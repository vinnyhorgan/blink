#include "blink_graphics.h"

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define STBI_NO_STDIO
#define STBI_NO_HDR
#define STBI_NO_LINEAR
#define STBI_ONLY_PNG
#define STB_IMAGE_IMPLEMENTATION
#include "lib/stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "lib/stb_image_write.h"

#include "cri.h"

#define EXPAND(X) ((X) + ((X) > 0))

#define CLIP0(CX, X, X2, W) \
    if (X < CX) {           \
        int D = CX - X;     \
        W -= D;             \
        X2 += D;            \
        X += D;             \
    }

#define CLIP1(X, DW, W) \
    if (X + W > DW)     \
        W = DW - X;

#define CLIP()                      \
    CLIP0(dst->clip.x, dx, sx, w);  \
    CLIP0(dst->clip.y, dy, sy, h);  \
    CLIP0(0, sx, dx, w);            \
    CLIP0(0, sy, dy, h);            \
    CLIP1(dx, dst->clip.x + cw, w); \
    CLIP1(dy, dst->clip.y + ch, h); \
    CLIP1(sx, src->w, w);           \
    CLIP1(sy, src->h, h);           \
    if (w <= 0 || h <= 0)           \
    return

blink_image *blink_create_image(int w, int h) {
    blink_image *image = (blink_image*)calloc(1, sizeof(blink_image));
    image->w = w;
    image->h = h;
    image->clip = blink_new_rect(0, 0, -1, -1);
    image->pixels = (blink_color*)calloc(w * h, sizeof(blink_color));
    return image;
}

blink_image *blink_load_image_mem(void *data, int size) {
    int w, h;
    uint8_t *png = stbi_load_from_memory(data, size, &w, &h, NULL, 4);
    if (!png) { return NULL; }
    blink_image *image = blink_create_image(w, h);
    memcpy(image->pixels, png, w * h * 4);
    free(png);

    for (int i = 0; i < w * h; i++) {
        blink_color *p = &image->pixels[i];
        uint8_t t = p->r;
        p->r = p->b;
        p->b = t;
    }

    return image;
}

blink_image *blink_load_image_file(const char *filename) {
    int size;
    void *data = cri_read_file(filename, &size);
    if (!data) { return NULL; }
    blink_image *image = blink_load_image_mem(data, size);
    free(data);
    return image;
}

void blink_save_image(blink_image *image, const char *filename) {
    blink_color *image_data = (blink_color*)calloc(image->w * image->h, sizeof(blink_color));
    memcpy(image_data, image->pixels, image->w * image->h * 4);

    for (int i = 0; i < image->w * image->h; i++) {
        blink_color *p = &image_data[i];
        uint8_t t = p->r;
        p->r = p->b;
        p->b = t;
    }

    stbi_write_png(filename, image->w, image->h, 4, image_data, image->w * 4);
    free(image_data);
}

void blink_destroy_image(blink_image *image) {
    free(image->pixels);
    free(image);
}

static bool blink_check_column(blink_image *image, int x, int y, int h) {
    while (h > 0) {
        if (image->pixels[x + y * image->w].a) { return true; }
        y++; h--;
    }
    return false;
}

static blink_font *blink_load_font_from_image(blink_image *image) {
    if (!image) { return NULL; }
    blink_font *font = (blink_font*)calloc(1, sizeof(blink_font));
    font->image = image;

    for (int i = 0; i < 256; i++) {
        blink_glyph *g = &font->glyphs[i];
        blink_rect r = {
            (image->w / 16) * (i % 16),
            (image->h / 16) * (i / 16),
            image->w / 16,
            image->h / 16
        };

        for (int x = r.x + r.w - 1; x >= r.x; x--) {
            if (blink_check_column(font->image, x, r.y, r.h)) { break; }
            r.w--;
        }

        for (int x = r.x; x < r.x + r.w; x++) {
            if (blink_check_column(font->image, x, r.y, r.h)) { break; }
            r.x++;
            r.w--;
        }

        g->xadv = r.w + 1;
        g->rect = r;
    }

    font->glyphs[' '].rect = (blink_rect){0};
    font->glyphs[' '].xadv = font->glyphs['a'].xadv;

    return font;
}


blink_font *blink_load_font_mem(void *data, int size) {
    return blink_load_font_from_image(blink_load_image_mem(data, size));
}

blink_font *blink_load_font_file(const char *filename) {
    return blink_load_font_from_image(blink_load_image_file(filename));
}

void blink_destroy_font(blink_font *font) {
    blink_destroy_image(font->image);
    free(font);
}

int blink_text_width(blink_font *font, const char *text) {
    int x = 0;
    for (uint8_t *p = (void*)text; *p; p++) {
        x += font->glyphs[*p].xadv;
    }
    return x;
}

void blink_clear(blink_image *image, blink_color color) {
    int count = image->w * image->h;
    int n;
    for (n = 0; n < count; n++)
        image->pixels[n] = color;
}

void blink_set_clip(blink_image *image, blink_rect rect) {
    image->clip = rect;
}

void blink_set_pixel(blink_image *image, int x, int y, blink_color color) {
    int xa, i, a;

    int cx = image->clip.x;
    int cy = image->clip.y;
    int cw = image->clip.w >= 0 ? image->clip.w : image->w;
    int ch = image->clip.h >= 0 ? image->clip.h : image->h;

    if (x >= cx && y >= cy && x < cx + cw && y < cy + ch) {
        xa = EXPAND(color.a);
        a = xa * xa;
        i = y * image->w + x;

        image->pixels[i].r += (uint8_t)((color.r - image->pixels[i].r) * a >> 16);
        image->pixels[i].g += (uint8_t)((color.g - image->pixels[i].g) * a >> 16);
        image->pixels[i].b += (uint8_t)((color.b - image->pixels[i].b) * a >> 16);
        image->pixels[i].a += (uint8_t)((color.a - image->pixels[i].a) * a >> 16);
    }
}

blink_color blink_get_pixel(blink_image *image, int x, int y) {
    blink_color empty = blink_rgba(0, 0, 0, 0);
    if (x >= 0 && y >= 0 && x < image->w && y < image->h)
        return image->pixels[y * image->w + x];
    return empty;
}

void blink_fill(blink_image *image, int x, int y, int w, int h, blink_color color) {
    blink_color *td;
    int dt, i;

    if (x < 0) {
        w += x;
        x = 0;
    }
    if (y < 0) {
        h += y;
        y = 0;
    }
    if (x + w > image->w)
        w = image->w - x;
    if (y + h > image->h)
        h = image->h - y;
    if (w <= 0 || h <= 0)
        return;

    td = &image->pixels[y * image->w + x];
    dt = image->w;

    do {
        for (i = 0; i < w; i++)
            td[i] = color;
        td += dt;
    } while (--h);
}

void blink_line(blink_image *image, int x0, int y0, int x1, int y1, blink_color color) {
    int sx, sy, dx, dy, err, e2;
    dx = abs(x1 - x0);
    dy = abs(y1 - y0);

    if (x0 < x1)
        sx = 1;
    else
        sx = -1;
    if (y0 < y1)
        sy = 1;
    else
        sy = -1;
    err = dx - dy;

    do {
        blink_set_pixel(image, x0, y0, color);
        e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
    } while (x0 != x1 || y0 != y1);
}

void blink_rectangle(blink_image *image, int x, int y, int w, int h, blink_color color) {
    int x1, y1;
    if (w <= 0 || h <= 0)
        return;

    if (w == 1) {
        blink_line(image, x, y, x, y + h, color);
    } else if (h == 1) {
        blink_line(image, x, y, x + w, y, color);
    } else {
        x1 = x + w - 1;
        y1 = y + h - 1;
        blink_line(image, x, y, x1, y, color);
        blink_line(image, x1, y, x1, y1, color);
        blink_line(image, x1, y1, x, y1, color);
        blink_line(image, x, y1, x, y, color);
    }
}

void blink_fill_rectangle(blink_image *image, int x, int y, int w, int h, blink_color color) {
    x += 1;
    y += 1;
    w -= 2;
    h -= 2;

    int cx = image->clip.x;
    int cy = image->clip.y;
    int cw = image->clip.w >= 0 ? image->clip.w : image->w;
    int ch = image->clip.h >= 0 ? image->clip.h : image->h;

    if (x < cx) {
        w += (x - cx);
        x = cx;
    }
    if (y < cy) {
        h += (y - cy);
        y = cy;
    }
    if (x + w > cx + cw)
        w -= (x + w) - (cx + cw);
    if (y + h > cy + ch)
        h -= (y + h) - (cy + ch);
    if (w <= 0 || h <= 0)
        return;

    blink_color *td = &image->pixels[y * image->w + x];
    int dt = image->w;
    int xa = EXPAND(color.a);
    int a = xa * xa;

    do {
        for (int i = 0; i < w; i++) {
            td[i].r += (uint8_t)((color.r - td[i].r) * a >> 16);
            td[i].g += (uint8_t)((color.g - td[i].g) * a >> 16);
            td[i].b += (uint8_t)((color.b - td[i].b) * a >> 16);
            td[i].a += (uint8_t)((color.a - td[i].a) * a >> 16);
        }
        td += dt;
    } while (--h);
}

void blink_circle(blink_image *image, int x0, int y0, int r, blink_color color) {
    int E = 1 - r;
    int dx = 0;
    int dy = -2 * r;
    int x = 0;
    int y = r;

    blink_set_pixel(image, x0, y0 + r, color);
    blink_set_pixel(image, x0, y0 - r, color);
    blink_set_pixel(image, x0 + r, y0, color);
    blink_set_pixel(image, x0 - r, y0, color);

    while (x < y - 1) {
        x++;

        if (E >= 0) {
            y--;
            dy += 2;
            E += dy;
        }

        dx += 2;
        E += dx + 1;

        blink_set_pixel(image, x0 + x, y0 + y, color);
        blink_set_pixel(image, x0 - x, y0 + y, color);
        blink_set_pixel(image, x0 + x, y0 - y, color);
        blink_set_pixel(image, x0 - x, y0 - y, color);

        if (x != y) {
            blink_set_pixel(image, x0 + y, y0 + x, color);
            blink_set_pixel(image, x0 - y, y0 + x, color);
            blink_set_pixel(image, x0 + y, y0 - x, color);
            blink_set_pixel(image, x0 - y, y0 - x, color);
        }
    }
}

void blink_fill_circle(blink_image *image, int x0, int y0, int r, blink_color color) {
    if (r <= 0)
        return;

    int E = 1 - r;
    int dx = 0;
    int dy = -2 * r;
    int x = 0;
    int y = r;

    blink_line(image, x0 - r + 1, y0, x0 + r, y0, color);

    while (x < y - 1) {
        x++;

        if (E >= 0) {
            y--;
            dy += 2;
            E += dy;
            blink_line(image, x0 - x + 1, y0 + y, x0 + x, y0 + y, color);
            blink_line(image, x0 - x + 1, y0 - y, x0 + x, y0 - y, color);
        }

        dx += 2;
        E += dx + 1;

        if (x != y) {
            blink_line(image, x0 - y + 1, y0 + x, x0 + y, y0 + x, color);
            blink_line(image, x0 - y + 1, y0 - x, x0 + y, y0 - x, color);
        }
    }
}

void blink_blit(blink_image *dst, blink_image *src, int dx, int dy, int sx, int sy, int w, int h) {
    int cw = dst->clip.w >= 0 ? dst->clip.w : dst->w;
    int ch = dst->clip.h >= 0 ? dst->clip.h : dst->h;

    CLIP();

    blink_color *ts = &src->pixels[sy * src->w + sx];
    blink_color *td = &dst->pixels[dy * dst->w + dx];
    int st = src->w;
    int dt = dst->w;
    do {
        memcpy(td, ts, w * sizeof(blink_color));
        ts += st;
        td += dt;
    } while (--h);
}

void blink_blit_alpha(blink_image *dst, blink_image *src, int dx, int dy, int sx, int sy, int w, int h, float alpha) {
    alpha = (alpha < 0) ? 0 : (alpha > 1 ? 1 : alpha);
    blink_blit_tint(dst, src, dx, dy, sx, sy, w, h, blink_rgba(0xff, 0xff, 0xff, (uint8_t)(alpha * 255)));
}

void blink_blit_tint(blink_image *dst, blink_image *src, int dx, int dy, int sx, int sy, int w, int h, blink_color tint) {
    int cw = dst->clip.w >= 0 ? dst->clip.w : dst->w;
    int ch = dst->clip.h >= 0 ? dst->clip.h : dst->h;

    CLIP();

    int xr = EXPAND(tint.r);
    int xg = EXPAND(tint.g);
    int xb = EXPAND(tint.b);
    int xa = EXPAND(tint.a);

    blink_color *ts = &src->pixels[sy * src->w + sx];
    blink_color *td = &dst->pixels[dy * dst->w + dx];
    int st = src->w;
    int dt = dst->w;
    do {
        for (int x = 0; x < w; x++) {
            unsigned r = (xr * ts[x].r) >> 8;
            unsigned g = (xg * ts[x].g) >> 8;
            unsigned b = (xb * ts[x].b) >> 8;
            unsigned a = xa * EXPAND(ts[x].a);
            td[x].r += (uint8_t)((r - td[x].r) * a >> 16);
            td[x].g += (uint8_t)((g - td[x].g) * a >> 16);
            td[x].b += (uint8_t)((b - td[x].b) * a >> 16);
            td[x].a += (uint8_t)((ts[x].a - td[x].a) * a >> 16);
        }
        ts += st;
        td += dt;
    } while (--h);
}

int blink_draw_text(blink_image *image, blink_font *font, const char *text, int x, int y, blink_color color) {
    int start_x = x;
    int line_height = 10;

    for (uint8_t *p = (void*)text; *p; p++) {
        if (*p == '\n') {
            x = start_x;
            y += line_height;
            continue;
        }

        blink_glyph g = font->glyphs[*p];
        blink_blit_tint(image, font->image, x, y, g.rect.x, g.rect.y, g.rect.w, g.rect.h, color);
        x += g.xadv;
    }

    return x;
}
