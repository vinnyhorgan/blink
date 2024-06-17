#include "blink.h"

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

    return ctx;
}

void blink_destroy(blink_Context *ctx) {
    ReleaseDC(ctx->hwnd, ctx->hdc);
    DestroyWindow(ctx->hwnd);
    blink_destroy_image(ctx->screen);
    free(ctx);
}

bool blink_update(blink_Context *ctx) {
    RedrawWindow(ctx->hwnd, 0, 0, RDW_INVALIDATE | RDW_UPDATENOW);

    MSG msg;
    while (PeekMessage(&msg, ctx->hwnd, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return !ctx->should_quit;
}

blink_Image *blink_create_image(int width, int height) {
    blink_expect(width > 0 && height > 0);
    blink_Image *img = blink_alloc(sizeof(blink_Image) + width * height * sizeof(blink_Color));
    img->pixels = (void*) (img + 1);
    img->w = width;
    img->h = height;
    return img;
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
