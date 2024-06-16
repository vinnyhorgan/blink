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
        return DefWindowProc(hwnd, message, wparam, lparam);
    }

    return 0;
}

blink_Context *blink_create(const char *title, int width, int height) {
    blink_Context *ctx = blink_alloc(sizeof(blink_Context));

    ctx->screen = blink_create_image(width, height);
    ctx->clip = blink_rect(0, 0, width, height);

    RegisterClass(&(WNDCLASS) {
        .style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW,
        .lpfnWndProc = blink_wndproc,
        .hCursor = LoadCursor(0, IDC_ARROW),
        .lpszClassName = title
    });

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
