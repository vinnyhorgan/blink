#ifndef BLINK_GRAPHICS_H
#define BLINK_GRAPHICS_H

#include <stdint.h>

#define blink_rgba(R, G, B, A) ((blink_color) { .r = (R), .g = (G), .b = (B), .a = (A) })
#define blink_rgb(R, G, B) blink_rgba(R, G, B, 0xff)
#define blink_new_rect(X, Y, W, H) ((blink_rect) { (X), (Y), (W), (H) })

typedef struct {
    uint8_t b, g, r, a;
} blink_color;

typedef struct {
    int x, y, w, h;
} blink_rect;

typedef struct {
    int w, h;
    blink_rect clip;
    blink_color *pixels;
} blink_image;

typedef struct {
    blink_rect rect;
    int xadv;
} blink_glyph;

typedef struct {
    blink_image *image;
    blink_glyph glyphs[256];
} blink_font;

blink_image *blink_create_image(int w, int h);
blink_image *blink_load_image_mem(void *data, int size);
blink_image *blink_load_image_file(const char *filename);
void blink_save_image(blink_image *image, const char *type, const char *filename);
void *blink_save_image_mem(blink_image *image, int *size);
void blink_destroy_image(blink_image *image);

blink_font *blink_load_font_mem(void *data, int size);
blink_font *blink_load_font_file(const char *filename);
void blink_destroy_font(blink_font *font);
int blink_text_width(blink_font *font, const char *text);

void blink_clear(blink_image *image, blink_color color);
void blink_set_clip(blink_image *image, blink_rect rect);
void blink_set_pixel(blink_image *image, int x, int y, blink_color color);
blink_color blink_get_pixel(blink_image *image, int x, int y);
void blink_fill(blink_image *image, int x, int y, int w, int h, blink_color color);
void blink_line(blink_image *image, int x0, int y0, int x1, int y1, blink_color color);
void blink_rectangle(blink_image *image, int x, int y, int w, int h, blink_color color);
void blink_fill_rectangle(blink_image *image, int x, int y, int w, int h, blink_color color);
void blink_circle(blink_image *image, int x0, int y0, int r, blink_color color);
void blink_fill_circle(blink_image *image, int x0, int y0, int r, blink_color color);
void blink_blit(blink_image *dst, blink_image *src, int dx, int dy, int sx, int sy, int w, int h);
void blink_blit_alpha(blink_image *dst, blink_image *src, int dx, int dy, int sx, int sy, int w, int h, float alpha);
void blink_blit_tint(blink_image *dst, blink_image *src, int dx, int dy, int sx, int sy, int w, int h, blink_color tint);
int blink_draw_text(blink_image *image, blink_font *font, const char *text, int x, int y, blink_color color);

#endif
