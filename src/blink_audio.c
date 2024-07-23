#include "blink_audio.h"

#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "cri.h"

#define STB_VORBIS_HEADER_ONLY
#include "lib/stb_vorbis.c"

#define CLAMP(x, a, b) ((x) < (a) ? (a) : (x) > (b) ? (b) : (x))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

#define FX_BITS (12)
#define FX_UNIT (1 << FX_BITS)
#define FX_MASK (FX_UNIT - 1)
#define FX_FROM_FLOAT(f) ((f) * FX_UNIT)
#define FX_LERP(a, b, p) ((a) + ((((b) - (a)) * (p)) >> FX_BITS))

#define BUFFER_SIZE (512)
#define BUFFER_MASK (BUFFER_SIZE - 1)

struct ba_source {
    ba_source *next;
    int16_t buffer[BUFFER_SIZE];
    ba_event_handler handler;
    void *user_data;
    int samplerate;
    int length;
    int end;
    int state;
    int64_t position;
    int lgain, rgain;
    int rate;
    int nextfill;
    bool loop;
    bool rewind;
    bool active;
    double gain;
    double pan;
};

static struct {
    ba_source *sources;
    int32_t buffer[BUFFER_SIZE];
    int samplerate;
    int gain;
    pthread_mutex_t mutex;
} ba_state;

void ba_init(int samplerate) {
    ba_state.samplerate = samplerate;
    ba_state.sources = NULL;
    ba_state.gain = FX_UNIT;
}

void ba_set_master_gain(double gain) {
    ba_state.gain = FX_FROM_FLOAT(gain);
}

static void rewind_source(ba_source *source) {
    ba_event e;
    e.type = BA_EVENT_REWIND;
    e.user_data = source->user_data;
    source->handler(&e);
    source->position = 0;
    source->rewind = false;
    source->end = source->length;
    source->nextfill = 0;
}

static void fill_source_buffer(ba_source *source, int offset, int length) {
    ba_event e;
    e.type = BA_EVENT_SAMPLES;
    e.user_data = source->user_data;
    e.buffer = source->buffer + offset;
    e.length = length;
    source->handler(&e);
}

static void process_source(ba_source *source, int size) {
    int32_t *dst = ba_state.buffer;

    if (source->rewind)
        rewind_source(source);

    if (source->state != BA_STATE_PLAYING)
        return;

    while (size > 0) {
        int frame = source->position >> FX_BITS;

        if (frame + 3 >= source->nextfill) {
            fill_source_buffer(source, (source->nextfill * 2) & BUFFER_MASK, BUFFER_SIZE / 2);
            source->nextfill += BUFFER_SIZE / 4;
        }

        if (frame >= source->end) {
            source->end = frame + source->length;
            if (!source->loop) {
                source->state = BA_STATE_STOPPED;
                break;
            }
        }

        int n = MIN(source->nextfill - 2, source->end) - frame;
        int count = (n << FX_BITS) / source->rate;
        count = MAX(count, 1);
        count = MIN(count, size / 2);
        size -= count * 2;

        if (source->rate == FX_UNIT) {
            n = frame * 2;
            for (int i = 0; i < count; i++) {
                dst[0] += (source->buffer[(n) & BUFFER_MASK] * source->lgain) >> FX_BITS;
                dst[1] += (source->buffer[(n + 1) & BUFFER_MASK] * source->rgain) >> FX_BITS;
                n += 2;
                dst += 2;
            }
            source->position += count * FX_UNIT;
        } else {
            for (int i = 0; i < count; i++) {
                n = (source->position >> FX_BITS) * 2;
                int p = source->position & FX_MASK;
                int a = source->buffer[(n) & BUFFER_MASK];
                int b = source->buffer[(n + 2) & BUFFER_MASK];
                dst[0] += (FX_LERP(a, b, p) * source->lgain) >> FX_BITS;
                n++;
                a = source->buffer[(n) & BUFFER_MASK];
                b = source->buffer[(n + 2) & BUFFER_MASK];
                dst[1] += (FX_LERP(a, b, p) * source->rgain) >> FX_BITS;
                source->position += source->rate;
                dst += 2;
            }
        }
    }
}

void ba_process(int16_t *dst, int size) {
    while (size > BUFFER_SIZE) {
        ba_process(dst, BUFFER_SIZE);
        dst += BUFFER_SIZE;
        size -= BUFFER_SIZE;
    }

    memset(ba_state.buffer, 0, size * sizeof(ba_state.buffer[0]));

    pthread_mutex_lock(&ba_state.mutex);
    ba_source **s = &ba_state.sources;
    while (*s) {
        process_source(*s, size);

        if ((*s)->state != BA_STATE_PLAYING) {
            (*s)->active = false;
            *s = (*s)->next;
        } else {
            s = &(*s)->next;
        }
    }
    pthread_mutex_unlock(&ba_state.mutex);

    for (int i = 0; i < size; i++) {
        int x = (ba_state.buffer[i] * ba_state.gain) >> FX_BITS;
        dst[i] = CLAMP(x, -32768, 32767);
    }
}

ba_source *ba_new_source(const ba_source_info *info) {
    ba_source *source = calloc(1, sizeof(ba_source));
    if (!source)
        return NULL;

    source->handler = info->handler;
    source->length = info->length;
    source->samplerate = info->samplerate;
    source->user_data = info->user_data;
    ba_set_gain(source, 1);
    ba_set_pan(source, 0);
    ba_set_pitch(source, 1);
    ba_set_loop(source, false);
    ba_stop(source);
    return source;
}

static bool ogg_init(ba_source_info *info, void *data, int size, bool ownsdata);
static bool wav_init(ba_source_info *info, void *data, int size, bool ownsdata);

static int check_header(void *data, int size, char *str, int offset) {
    int len = strlen(str);
    return (size >= offset + len) && !memcmp((char*)data + offset, str, len);
}

static ba_source *new_source_from_mem(void *data, int size, bool ownsdata) {
    ba_source_info info;

    if (check_header(data, size, "OggS", 0)) {
        if (!ogg_init(&info, data, size, ownsdata))
            return NULL;
        return ba_new_source(&info);
    }

    if (check_header(data, size, "WAVE", 8)) {
        if (!wav_init(&info, data, size, ownsdata))
            return NULL;
        return ba_new_source(&info);
    }

    return NULL;
}

ba_source *ba_load_source_file(const char *filename) {
    int size;
    void *data = cri_read_file(filename, &size);
    if (!data)
        return NULL;

    ba_source *source = new_source_from_mem(data, size, true);
    if (!source) {
        free(data);
        return NULL;
    }

    return source;
}

ba_source *ba_load_source_mem(void *data, int size) {
    return new_source_from_mem(data, size, false);
}

void ba_destroy_source(ba_source *source) {
    pthread_mutex_lock(&ba_state.mutex);
    if (source->active) {
        ba_source **s = &ba_state.sources;
        while (*s) {
            if (*s == source) {
                *s = source->next;
                break;
            }
        }
    }
    pthread_mutex_unlock(&ba_state.mutex);

    ba_event e;
    e.type = BA_EVENT_DESTROY;
    e.user_data = source->user_data;
    source->handler(&e);
    free(source);
}

double ba_get_length(ba_source *source) {
    return source->length / (double)source->samplerate;
}

double ba_get_position(ba_source *source) {
    return ((source->position >> FX_BITS) % source->length) / (double)source->samplerate;
}

int ba_get_state(ba_source *source) {
    return source->state;
}

static void recalc_source_gains(ba_source *source) {
    double pan = source->pan;
    double l = source->gain * (pan <= 0. ? 1. : 1. - pan);
    double r = source->gain * (pan >= 0. ? 1. : 1. + pan);
    source->lgain = FX_FROM_FLOAT(l);
    source->rgain = FX_FROM_FLOAT(r);
}

void ba_set_gain(ba_source *source, double gain) {
    source->gain = gain;
    recalc_source_gains(source);
}

void ba_set_pan(ba_source *source, double pan) {
    source->pan = CLAMP(pan, -1.0, 1.0);
    recalc_source_gains(source);
}

void ba_set_pitch(ba_source *source, double pitch) {
    double rate;
    if (pitch > 0.0) {
        rate = source->samplerate / (double)ba_state.samplerate * pitch;
    } else {
        rate = 0.001;
    }
    source->rate = FX_FROM_FLOAT(rate);
}

void ba_set_loop(ba_source *source, bool loop) {
    source->loop = loop;
}

void ba_play(ba_source *source) {
    pthread_mutex_lock(&ba_state.mutex);
    source->state = BA_STATE_PLAYING;
    if (!source->active) {
        source->active = true;
        source->next = ba_state.sources;
        ba_state.sources = source;
    }
    pthread_mutex_unlock(&ba_state.mutex);
}

void ba_pause(ba_source *source) {
    source->state = BA_STATE_PAUSED;
}

void ba_stop(ba_source *source) {
    source->state = BA_STATE_STOPPED;
    source->rewind = true;
}

//--------------------
// OGG
//--------------------

typedef struct {
    stb_vorbis *ogg;
    void *data;
} ba_ogg_stream;

static void ogg_handler(ba_event *e) {
    ba_ogg_stream *s = e->user_data;

    switch (e->type) {
    case BA_EVENT_DESTROY:
        stb_vorbis_close(s->ogg);
        free(s->data);
        free(s);
        break;

    case BA_EVENT_SAMPLES:
        int len = e->length;
        int16_t *buf = e->buffer;
fill:
        int n = stb_vorbis_get_samples_short_interleaved(s->ogg, 2, buf, len);
        n *= 2;

        if (len != n) {
            stb_vorbis_seek_start(s->ogg);
            buf += n;
            len -= n;
            goto fill;
        }
        break;

    case BA_EVENT_REWIND:
        stb_vorbis_seek_start(s->ogg);
        break;
    }
}

static bool ogg_init(ba_source_info *info, void *data, int size, bool ownsdata) {
    stb_vorbis *ogg = stb_vorbis_open_memory(data, size, NULL, NULL);
    if (!ogg)
        return false;

    ba_ogg_stream *stream = calloc(1, sizeof(ba_ogg_stream));
    if (!stream) {
        stb_vorbis_close(ogg);
        return false;
    }

    stream->ogg = ogg;
    if (ownsdata)
        stream->data = data;

    stb_vorbis_info ogginfo = stb_vorbis_get_info(ogg);

    info->user_data = stream;
    info->handler = ogg_handler;
    info->samplerate = ogginfo.sample_rate;
    info->length = stb_vorbis_stream_length_in_samples(ogg);

    return true;
}

//--------------------
// WAVE
//--------------------

typedef struct {
    void *data;
    int bitdepth;
    int samplerate;
    int channels;
    int length;
} ba_wav;

typedef struct {
    ba_wav wav;
    void *data;
    int idx;
} ba_wav_stream;

static char *find_subchunk(char *data, int len, char *id, int *size) {
    int idlen = strlen(id);
    char *p = data + 12;
next:
    *size = *((uint32_t*)(p + 4));
    if (memcmp(p, id, idlen)) {
        p += 8 + *size;
        if (p > data + len) return NULL;
        goto next;
    }
    return p + 8;
}

static bool read_wav(ba_wav *w, void *data, int size) {
    char *p = data;
    memset(w, 0, sizeof(ba_wav));

    if (memcmp(p, "RIFF", 4) || memcmp(p + 8, "WAVE", 4))
        return false;

    int sz;
    p = find_subchunk(data, size, "fmt", &sz);
    if (!p)
        return false;

    int format = *((uint16_t*)(p));
    int channels = *((uint16_t*)(p + 2));
    int samplerate = *((uint32_t*)(p + 4));
    int bitdepth = *((uint16_t*)(p + 14));
    if (format != 1)
        return false;

    if (channels == 0 || samplerate == 0 || bitdepth == 0)
        return false;

    p = find_subchunk(data, size, "data", &sz);
    if (!p)
        return false;

    w->data = (void*)p;
    w->samplerate = samplerate;
    w->channels = channels;
    w->length = (sz / (bitdepth / 8)) / channels;
    w->bitdepth = bitdepth;

    return true;
}

#define WAV_PROCESS_LOOP(X) \
    while (n--) {           \
        X                   \
        dst += 2;           \
        s->idx++;           \
    }

static void wav_handler(ba_event *e) {
    ba_wav_stream *s = e->user_data;

    switch (e->type) {
    case BA_EVENT_DESTROY:
        free(s->data);
        free(s);
        break;

    case BA_EVENT_SAMPLES:
        int16_t *dst = e->buffer;
        int len = e->length / 2;
fill:
        int n = MIN(len, s->wav.length - s->idx);
        len -= n;

        if (s->wav.bitdepth == 16 && s->wav.channels == 1) {
            WAV_PROCESS_LOOP({
                dst[0] = dst[1] = ((int16_t*)s->wav.data)[s->idx];
            });
        } else if (s->wav.bitdepth == 16 && s->wav.channels == 2) {
            WAV_PROCESS_LOOP({
                int x = s->idx * 2;
                dst[0] = ((int16_t*)s->wav.data)[x];
                dst[1] = ((int16_t*)s->wav.data)[x + 1];
            });
        } else if (s->wav.bitdepth == 8 && s->wav.channels == 1) {
            WAV_PROCESS_LOOP({
                dst[0] = dst[1] = (((uint8_t*)s->wav.data)[s->idx] - 128) << 8;
            });
        } else if (s->wav.bitdepth == 8 && s->wav.channels == 2) {
            WAV_PROCESS_LOOP({
                int x = s->idx * 2;
                dst[0] = (((uint8_t*)s->wav.data)[x] - 128) << 8;
                dst[1] = (((uint8_t*)s->wav.data)[x + 1] - 128) << 8;
            });
        }

        if (len > 0) {
            s->idx = 0;
            goto fill;
        }
        break;

    case BA_EVENT_REWIND:
        s->idx = 0;
        break;
    }
}

static bool wav_init(ba_source_info *info, void *data, int size, bool ownsdata) {
    ba_wav wav;
    if (!read_wav(&wav, data, size))
        return false;

    if (wav.channels > 2 || (wav.bitdepth != 16 && wav.bitdepth != 8))
        return false;

    ba_wav_stream *stream = calloc(1, sizeof(ba_wav_stream));
    if (!stream)
        return false;

    stream->wav = wav;

    if (ownsdata)
        stream->data = data;

    stream->idx = 0;

    info->user_data = stream;
    info->handler = wav_handler;
    info->samplerate = wav.samplerate;
    info->length = wav.length;

    return true;
}
