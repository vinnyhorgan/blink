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

typedef struct {
    bool should_quit;
    blink_Image *screen;
    blink_Rect clip;
    int width, height;
    HWND hwnd;
    HDC hdc;
} blink_Context;

#define blink_rect(X, Y, W, H) ((blink_Rect) { (X), (Y), (W), (H) })
#define blink_rgba(R, G, B, A) ((blink_Color) { .r = (R), .g = (G), .b = (B), .a = (A) })
#define blink_rgb(R, G, B) kit_rgba(R, G, B, 0xff)

blink_Context *blink_create(const char *title, int width, int height);
void blink_destroy(blink_Context *ctx);
bool blink_update(blink_Context *ctx);

blink_Image *blink_create_image(int width, int height);
void blink_destroy_image(blink_Image *img);

#endif
