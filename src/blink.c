#include "blink.h"

#define STBI_NO_STDIO
#define STBI_NO_HDR
#define STBI_NO_LINEAR
#define STBI_ONLY_PNG
#define STB_IMAGE_IMPLEMENTATION
#include "lib/stb_image.h"

#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
#define DWMWA_USE_IMMERSIVE_DARK_MODE 20
#endif

#define blink_expect(x) if (!(x)) { blink_panic("assertion failure: %s", #x); }

static void blink_panic(const char *fmt, ...) {
    fprintf(stderr, "blink fatal error: ");
    va_list ap;
    va_start(ap, fmt); vfprintf(stderr, fmt, ap); va_end(ap);
    fprintf(stderr, "\n");
    exit(1);
}

static void *blink_alloc(int n) {
    void *res = calloc(1, n);
    if (!res) { blink_panic("out of memory"); }
    return res;
}

static blink_Rect blink_intersect_rects(blink_Rect a, blink_Rect b) {
    int x1 = blink_max(a.x, b.x);
    int y1 = blink_max(a.y, b.y);
    int x2 = blink_min(a.x + a.w, b.x + b.w);
    int y2 = blink_min(a.y + a.h, b.y + b.h);
    return (blink_Rect) { x1, y1, x2 - x1, y2 - y1 };
}

static inline blink_Color blink_blend_pixel(blink_Color dst, blink_Color src) {
    blink_Color res;
    res.w = (dst.w & 0xff00ff) + ((((src.w & 0xff00ff) - (dst.w & 0xff00ff)) * src.a) >> 8);
    res.g = dst.g + (((src.g - dst.g) * src.a) >> 8);
    res.a = dst.a;
    return res;
}


static inline blink_Color blink_blend_pixel2(blink_Color dst, blink_Color src, blink_Color clr) {
    src.a = (src.a * clr.a) >> 8;
    int ia = 0xff - src.a;
    dst.r = ((src.r * clr.r * src.a) >> 16) + ((dst.r * ia) >> 8);
    dst.g = ((src.g * clr.g * src.a) >> 16) + ((dst.g * ia) >> 8);
    dst.b = ((src.b * clr.b * src.a) >> 16) + ((dst.b * ia) >> 8);
    return dst;
}


static inline blink_Color blink_blend_pixel3(blink_Color dst, blink_Color src, blink_Color clr, blink_Color add) {
    src.r = blink_min(255, src.r + add.r);
    src.g = blink_min(255, src.g + add.g);
    src.b = blink_min(255, src.b + add.b);
    return blink_blend_pixel2(dst, src, clr);
}

static blink_Rect blink_get_adjusted_window_rect(blink_Context *ctx) {
    float src_ar = (float) ctx->screen->h / ctx->screen->w;
    float dst_ar = (float) ctx->height / ctx->width;
    int w, h;
    if (src_ar < dst_ar) {
        w = ctx->width; h = ceil(w * src_ar);
    } else {
        h = ctx->height; w = ceil(h / src_ar);
    }
    return blink_rect((ctx->width - w) / 2, (ctx->height - h) / 2, w, h);
}

static LRESULT CALLBACK blink_wndproc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
    blink_Context *ctx = (void*) GetProp(hwnd, "blink_Context");

    switch (message) {
    case WM_PAINT:
        BITMAPINFO bmi = {
            .bmiHeader.biSize = sizeof(BITMAPINFOHEADER),
            .bmiHeader.biBitCount = 32,
            .bmiHeader.biCompression = BI_RGB,
            .bmiHeader.biPlanes = 1,
            .bmiHeader.biWidth = ctx->screen->w,
            .bmiHeader.biHeight = -ctx->screen->h
        };

        blink_Rect wr = blink_get_adjusted_window_rect(ctx);

        StretchDIBits(ctx->hdc,
            wr.x, wr.y, wr.w, wr.h,
            0, 0, ctx->screen->w, ctx->screen->h,
            ctx->screen->pixels, &bmi, DIB_RGB_COLORS, SRCCOPY);

        ValidateRect(hwnd, 0);
        break;

    case WM_SETCURSOR:
        if (ctx->hide_cursor && LOWORD(lparam) == HTCLIENT) {
            SetCursor(0);
            break;
        }
        goto unhandled;

    case WM_SIZE:
        if (wparam != SIZE_MINIMIZED) {
            ctx->width = LOWORD(lparam);
            ctx->height = HIWORD(lparam);
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            HBRUSH brush = CreateSolidBrush(RGB(0, 0, 0));
            FillRect(hdc, &ps.rcPaint, brush);
            DeleteObject(brush);
            EndPaint(hwnd, &ps);
            RedrawWindow(ctx->hwnd, 0, 0, RDW_INVALIDATE | RDW_UPDATENOW);
        }
        break;

    case WM_QUIT:
    case WM_CLOSE:
        ctx->should_quit = true;
        break;

    default:
unhandled:
        return DefWindowProc(hwnd, message, wparam, lparam);
    }

    return 0;
}

blink_Context *blink_create(const char *title, int width, int height, int flags) {
    blink_Context *ctx = blink_alloc(sizeof(blink_Context));

    ctx->screen = blink_create_image(width, height);
    ctx->clip = blink_rect(0, 0, width, height);
    ctx->hide_cursor = !!(flags & BLINK_HIDECURSOR);

    if (flags & BLINK_FPSINF) {
        ctx->step_time = 0;
    } else {
        ctx->step_time = 1.0 / 60.0;
    }

    RegisterClass(&(WNDCLASS) {
        .style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW,
        .lpfnWndProc = blink_wndproc,
        .hCursor = LoadCursor(0, IDC_ARROW),
        .lpszClassName = title
    });

    if (flags & BLINK_SCALE2X) { width *= 2; height *= 2; } else
    if (flags & BLINK_SCALE3X) { width *= 3; height *= 3; } else
    if (flags & BLINK_SCALE4X) { width *= 4; height *= 4; }

    RECT rect = { .right = width, .bottom = height };
    int style = WS_OVERLAPPEDWINDOW;
    AdjustWindowRect(&rect, style, 0);
    ctx->hwnd = CreateWindow(
        title, title, style,
        CW_USEDEFAULT, CW_USEDEFAULT,
        rect.right - rect.left, rect.bottom - rect.top,
        0, 0, 0, 0
    );
    SetProp(ctx->hwnd, "blink_Context", ctx);

    BOOL dark = TRUE;
    DwmSetWindowAttribute(ctx->hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &dark, sizeof(dark));

    ShowWindow(ctx->hwnd, SW_NORMAL);
    ctx->hdc = GetDC(ctx->hwnd);

    timeBeginPeriod(1);
    ctx->prev_time = clock() / 1000.0;

    return ctx;
}

void blink_destroy(blink_Context *ctx) {
    ReleaseDC(ctx->hwnd, ctx->hdc);
    DestroyWindow(ctx->hwnd);
    blink_destroy_image(ctx->screen);
    free(ctx);
}

bool blink_update(blink_Context *ctx, double *dt) {
    RedrawWindow(ctx->hwnd, 0, 0, RDW_INVALIDATE | RDW_UPDATENOW);

    double now = clock() / 1000.0;
    double wait = (ctx->prev_time + ctx->step_time) - now;
    double prev = ctx->prev_time;
    if (wait > 0) {
        Sleep(wait * 1000);
        ctx->prev_time += ctx->step_time;
    } else {
        ctx->prev_time = now;
    }
    if (dt) { *dt = ctx->prev_time - prev; }

    MSG msg;
    while (PeekMessage(&msg, ctx->hwnd, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return !ctx->should_quit;
}

void *blink_read_file(const char *filename, int *len) {
    FILE *fp = fopen(filename, "rb");
    if (!fp) { return NULL; }
    fseek(fp, 0, SEEK_END);
    int n = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    char *buf = blink_alloc(n + 1);
    fread(buf, 1, n, fp);
    fclose(fp);
    if (len) { *len = n; }
    return buf;
}

blink_Image *blink_create_image(int width, int height) {
    blink_expect(width > 0 && height > 0);
    blink_Image *img = blink_alloc(sizeof(blink_Image) + width * height * sizeof(blink_Color));
    img->pixels = (void*) (img + 1);
    img->w = width;
    img->h = height;
    return img;
}

blink_Image *blink_load_image_mem(void *data, int len) {
    int x, y;
    unsigned char *png = stbi_load_from_memory(data, len, &x, &y, NULL, 4);
    if (!png) { return NULL; }
    blink_Image *img = blink_create_image(x, y);
    memcpy(img->pixels, png, x * y * 4);
    free(png);

    for (int i = 0; i < x * y; i++) {
        blink_Color *p = &img->pixels[i];
        uint8_t t = p->r;
        p->r = p->b;
        p->b = t;
    }

    return img;
}

blink_Image *blink_load_image_file(const char *filename) {
    int len;
    void *data = blink_read_file(filename, &len);
    if (!data) { return NULL; }
    blink_Image *res = blink_load_image_mem(data, len);
    free(data);
    return res;
}

void blink_destroy_image(blink_Image *img) {
    free(img);
}

void blink_clear(blink_Context *ctx, blink_Color color) {
    blink_draw_rect(ctx, blink_rect(0, 0, 0xffffff, 0xffffff), color);
}

void blink_set_clip(blink_Context *ctx, blink_Rect rect) {
    blink_Rect screen_rect = blink_rect(0, 0, ctx->screen->w, ctx->screen->h);
    ctx->clip = blink_intersect_rects(rect, screen_rect);
}

void blink_draw_point(blink_Context *ctx, int x, int y, blink_Color color) {
    if (color.a == 0) { return; }
    blink_Rect r = ctx->clip;
    if (x < r.x || y < r.y || x >= r.x + r.w || y >= r.y + r.h ) { return; }
    blink_Color *dst = &ctx->screen->pixels[x + y * ctx->screen->w];
    *dst = blink_blend_pixel(*dst, color);
}

void blink_draw_rect(blink_Context *ctx, blink_Rect rect, blink_Color color) {
    if (color.a == 0) { return; }
    rect = blink_intersect_rects(rect, ctx->clip);
    blink_Color *d = &ctx->screen->pixels[rect.x + rect.y * ctx->screen->w];
    int dr = ctx->screen->w - rect.w;
    for (int y = 0; y < rect.h; y++) {
        for (int x = 0; x < rect.w; x++) {
            *d = blink_blend_pixel(*d, color);
            d++;
        }
        d += dr;
    }
}

void blink_draw_line(blink_Context *ctx, int x1, int y1, int x2, int y2, blink_Color color) {
    int dx = abs(x2-x1);
    int sx = x1 < x2 ? 1 : -1;
    int dy = -abs(y2 - y1);
    int sy = y1 < y2 ? 1 : -1;
    int err = dx + dy;
    for (;;) {
        blink_draw_point(ctx, x1, y1, color);
        if (x1 == x2 && y1 == y2) { break; }
        int e2 = err << 1;
        if (e2 >= dy) { err += dy; x1 += sx; }
        if (e2 <= dx) { err += dx; y1 += sy; }
    }
}

void blink_draw_image(blink_Context *ctx, blink_Image *img, int x, int y) {
    blink_Rect dst = blink_rect(x, y, img->w, img->h);
    blink_Rect src = blink_rect(0, 0, img->w, img->h);
    blink_draw_image3(ctx, img, dst, src, BLINK_WHITE, BLINK_BLACK);
}

void blink_draw_image2(blink_Context *ctx, blink_Image *img, int x, int y, blink_Rect src, blink_Color color) {
    blink_Rect dst = blink_rect(x, y, abs(src.w), abs(src.h));
    blink_draw_image3(ctx, img, dst, src, color, BLINK_BLACK);
}

void blink_draw_image3(blink_Context *ctx, blink_Image *img, blink_Rect dst, blink_Rect src, blink_Color mul_color, blink_Color add_color) {
    if (!src.w || !src.w || !dst.w || !dst.h) { return; }

    int cx1 = ctx->clip.x;
    int cy1 = ctx->clip.y;
    int cx2 = cx1 + ctx->clip.w;
    int cy2 = cy1 + ctx->clip.h;
    int stepx = (src.w << 10) / dst.w;
    int stepy = (src.h << 10) / dst.h;
    int sy = src.y << 10;

    int dy = dst.y;
    if (dy < cy1) { sy += (cy1 - dy) * stepy; dy = cy1; }
    int ey = blink_min(cy2, dst.y + dst.h);

    int blend_fn = 1;
    if (mul_color.w != 0xffffffff) { blend_fn = 2; }
    if ((add_color.w & 0xffffff00) != 0xffffff00) { blend_fn = 3; }

    for (; dy < ey; dy++) {
        if (dy >= cy1 && dy < cy2) {
            int sx = src.x << 10;
            blink_Color *srow = &img->pixels[(sy >> 10) * img->w];
            blink_Color *drow = &ctx->screen->pixels[dy * ctx->screen->w];

            int dx = dst.x;
            if (dx < cx1) { sx += (cx1 - dx) * stepx; dx = cx1; }
            int ex = blink_min(cx2, dst.x + dst.w);

            for (; dx < ex; dx++) {
                blink_Color *s = &srow[sx >> 10];
                blink_Color *d = &drow[dx];
                switch (blend_fn) {
                case 1: *d = blink_blend_pixel(*d, *s); break;
                case 2: *d = blink_blend_pixel2(*d, *s, mul_color); break;
                case 3: *d = blink_blend_pixel3(*d, *s, mul_color, add_color); break;
                }
                sx += stepx;
            }
        }
        sy += stepy;
    }
}
