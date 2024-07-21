#ifndef BLINK_AUDIO_H
#define BLINK_AUDIO_H

#include <stdbool.h>
#include <stdint.h>

typedef struct ba_source ba_source;

typedef struct {
    int type;
    void *user_data;
    int16_t *buffer;
    int length;
} ba_event;

typedef void (*ba_event_handler)(ba_event *e);

typedef struct {
    ba_event_handler handler;
    void *user_data;
    int samplerate;
    int length;
} ba_source_info;

enum {
    BA_STATE_STOPPED,
    BA_STATE_PLAYING,
    BA_STATE_PAUSED
};

enum {
    BA_EVENT_DESTROY,
    BA_EVENT_SAMPLES,
    BA_EVENT_REWIND
};

void ba_init(int samplerate);
void ba_set_master_gain(double gain);
void ba_process(int16_t *dst, int size);

ba_source *ba_new_source(const ba_source_info *info);
ba_source *ba_load_source_mem(void *data, int size);
ba_source *ba_load_source_file(const char *filename);
void ba_destroy_source(ba_source *source);
double ba_get_length(ba_source *source);
double ba_get_position(ba_source *source);
int ba_get_state(ba_source *source);
void ba_set_gain(ba_source *source, double gain);
void ba_set_pan(ba_source *source, double pan);
void ba_set_pitch(ba_source *source, double pitch);
void ba_set_loop(ba_source *source, bool loop);
void ba_play(ba_source *source);
void ba_pause(ba_source *source);
void ba_stop(ba_source *source);

#endif
