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

enum {
    BLINK_SCALE2X = (1 << 0),
    BLINK_SCALE3X = (1 << 1),
    BLINK_SCALE4X = (1 << 2),
    BLINK_HIDECURSOR = (1 << 3),
    BLINK_FPSINF = (1 << 4)
};

typedef union { struct { uint8_t b, g, r, a; }; uint32_t w; } blink_Color;
typedef struct { int x, y, w, h; } blink_Rect;
typedef struct { blink_Color *pixels; int w, h; } blink_Image;

typedef struct {
    bool should_quit;
    bool hide_cursor;
    double step_time;
    double prev_time;
    blink_Image *screen;
    blink_Rect clip;
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

blink_Context *blink_create(const char *title, int width, int height, int flags);
void blink_destroy(blink_Context *ctx);
bool blink_update(blink_Context *ctx, double *dt);
void *blink_read_file(const char *filename, int *len);

blink_Image *blink_create_image(int width, int height);
blink_Image *blink_load_image_mem(void *data, int len);
blink_Image *blink_load_image_file(const char *filename);
void blink_destroy_image(blink_Image *img);

void blink_clear(blink_Context *ctx, blink_Color color);
void blink_set_clip(blink_Context *ctx, blink_Rect rect);
void blink_draw_point(blink_Context *ctx, int x, int y, blink_Color color);
void blink_draw_rect(blink_Context *ctx, blink_Rect rect, blink_Color color);
void blink_draw_line(blink_Context *ctx, int x1, int y1, int x2, int y2, blink_Color color);
void blink_draw_image(blink_Context *ctx, blink_Image *img, int x, int y);
void blink_draw_image2(blink_Context *ctx, blink_Image *img, int x, int y, blink_Rect src, blink_Color color);
void blink_draw_image3(blink_Context *ctx, blink_Image *img, blink_Rect dst, blink_Rect src, blink_Color mul_color, blink_Color add_color);

#endif
