#ifndef BLINK_GRAPHICS_H
#define BLINK_GRAPHICS_H

#include <stdbool.h>
#include <stdint.h>

#define bg_rgba(R, G, B, A) ((bg_color){ .r = (R), .g = (G), .b = (B), .a = (A) })
#define bg_rgb(R, G, B) bg_rgba(R, G, B, 0xff)
#define bg_new_rect(X, Y, W, H) ((bg_rect){ (X), (Y), (W), (H) })

typedef struct {
    uint8_t b, g, r, a;
} bg_color;

typedef struct {
    int x, y, w, h;
} bg_rect;

typedef struct {
    int w, h;
    bg_rect clip;
    bg_color *pixels;
} bg_image;

typedef struct {
    bg_rect rect;
    int xadv;
} bg_glyph;

typedef struct {
    bg_image *image;
    bg_glyph glyphs[256];
} bg_font;

enum {
    BG_IMAGE_PNG,
    BG_IMAGE_JPG
};

bg_image *bg_new_image(int w, int h);
bg_image *bg_load_image_mem(void *data, int size);
bg_image *bg_load_image_file(const char *filename);
void bg_destroy_image(bg_image *image);
void *bg_save_image_mem(bg_image *image, int *size);
bool bg_save_image(bg_image *image, int type, const char *filename);
bg_image *bg_resize_image(bg_image *image, int w, int h);

bg_font *bg_load_font_mem(void *data, int size);
bg_font *bg_load_font_file(const char *filename);
void bg_destroy_font(bg_font *font);
int bg_text_width(bg_font *font, const char *text);

void bg_set_clip(bg_image *image, bg_rect rect);
void bg_clear(bg_image *image, bg_color color);
bg_color bg_get_pixel(bg_image *image, int x, int y);
void bg_set_pixel(bg_image *image, int x, int y, bg_color color);
void bg_line(bg_image *image, int x0, int y0, int x1, int y1, bg_color color);
void bg_fill(bg_image *image, int x, int y, int w, int h, bg_color color);
void bg_rectangle(bg_image *image, int x, int y, int w, int h, bg_color color);
void bg_fill_rectangle(bg_image *image, int x, int y, int w, int h, bg_color color);
void bg_circle(bg_image *image, int x0, int y0, int r, bg_color color);
void bg_fill_circle(bg_image *image, int x0, int y0, int r, bg_color color);
void bg_blit(bg_image *dst, bg_image *src, int dx, int dy, int sx, int sy, int w, int h);
void bg_blit_alpha(bg_image *dst, bg_image *src, int dx, int dy, int sx, int sy, int w, int h, float alpha);
void bg_blit_tint(bg_image *dst, bg_image *src, int dx, int dy, int sx, int sy, int w, int h, bg_color tint);
int bg_print(bg_image *image, bg_font *font, const char *text, int x, int y, bg_color color);

#endif
