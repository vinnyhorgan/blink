#ifndef BLINK_H
#define BLINK_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>
#include <time.h>
#include <math.h>
#include <windows.h>
#include <windowsx.h>
#include <dwmapi.h>

typedef union { struct { uint8_t b, g, r, a; }; uint32_t w; } blink_Color;
typedef struct { int x, y, w, h; } blink_Rect;
typedef struct { blink_Color *pixels; int w, h; } blink_Image;
typedef struct { blink_Rect rect; int xadv; } blink_Glyph;
typedef struct { blink_Image *image; blink_Glyph glyphs[256]; } blink_Font;

typedef struct {
    bool should_quit;
    bool hide_cursor;
    int char_buf[32];
    uint8_t key_state[256];
    uint8_t mouse_state[16];
    struct { int x, y; } mouse_pos;
    struct { int x, y; } mouse_delta;
    float mouse_scroll;
    double step_time;
    double prev_time;
    blink_Image *screen;
    blink_Rect clip;
    blink_Font *font;
    int width, height;
    HWND hwnd;
    HDC hdc;
} blink_Context;

#define blink_max(a, b) ((a) > (b) ? (a) : (b))
#define blink_min(a, b) ((a) < (b) ? (a) : (b))
#define blink_lengthof(a) (sizeof(a) / sizeof((a)[0]))

#define blink_rect(X, Y, W, H) ((blink_Rect) { (X), (Y), (W), (H) })
#define blink_rgba(R, G, B, A) ((blink_Color) { .r = (R), .g = (G), .b = (B), .a = (A) })
#define blink_rgb(R, G, B) blink_rgba(R, G, B, 0xff)

#define BLINK_WHITE blink_rgb(0xff, 0xff, 0xff)
#define BLINK_BLACK blink_rgb(0, 0, 0)

blink_Context *blink_create(const char *title, int width, int height, int scale);
void blink_destroy(blink_Context *ctx);
bool blink_update(blink_Context *ctx, double *dt);
void blink_set_target_fps(blink_Context *ctx, int fps);
void blink_set_cursor_hidden(blink_Context *ctx, bool hidden);
void *blink_read_file(const char *filename, int *len);

blink_Image *blink_create_image(int width, int height);
blink_Image *blink_load_image_mem(void *data, int len);
blink_Image *blink_load_image_file(const char *filename);
void blink_destroy_image(blink_Image *img);

blink_Font *blink_load_font_mem(void *data, int len);
blink_Font *blink_load_font_file(const char *filename);
void blink_destroy_font(blink_Font *font);
int blink_text_width(blink_Font *font, const char *text);

int blink_get_char(blink_Context *ctx);
bool blink_key_down(blink_Context *ctx, int key);
bool blink_key_pressed(blink_Context *ctx, int key);
bool blink_key_released(blink_Context *ctx, int key);
void blink_mouse_pos(blink_Context *ctx, int *x, int *y);
void blink_mouse_delta(blink_Context *ctx, int *x, int *y);
bool blink_mouse_down(blink_Context *ctx, int button);
bool blink_mouse_pressed(blink_Context *ctx, int button);
bool blink_mouse_released(blink_Context *ctx, int button);
float blink_mouse_scroll(blink_Context *ctx);

void blink_clear(blink_Context *ctx, blink_Color color);
void blink_set_clip(blink_Context *ctx, blink_Rect rect);
void blink_draw_point(blink_Context *ctx, int x, int y, blink_Color color);
void blink_draw_rect(blink_Context *ctx, blink_Rect rect, blink_Color color);
void blink_draw_line(blink_Context *ctx, int x1, int y1, int x2, int y2, blink_Color color);
void blink_draw_image(blink_Context *ctx, blink_Image *img, int x, int y);
void blink_draw_image2(blink_Context *ctx, blink_Image *img, int x, int y, blink_Rect src, blink_Color color);
void blink_draw_image3(blink_Context *ctx, blink_Image *img, blink_Rect dst, blink_Rect src, blink_Color mul_color, blink_Color add_color);
int blink_draw_text(blink_Context *ctx, const char *text, int x, int y, blink_Color color);
int blink_draw_text2(blink_Context *ctx, blink_Font *font, const char *text, int x, int y, blink_Color color);

#endif
