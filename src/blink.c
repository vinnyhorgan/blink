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

enum {
    BLINK_INPUT_DOWN = (1 << 0),
    BLINK_INPUT_PRESSED = (1 << 1),
    BLINK_INPUT_RELEASED = (1 << 2)
};

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

static bool blink_check_input_flag(uint8_t *t, uint32_t idx, uint32_t cap, int flag) {
    if (idx > cap) { return false; }
    return t[idx] & flag ? true : false;
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

static bool blink_check_column(blink_Image *img, int x, int y, int h) {
    while (h > 0) {
        if (img->pixels[x + y * img->w].a) { return true; }
        y++; h--;
    }
    return false;
}

static blink_Font *blink_load_font_from_image(blink_Image *img) {
    if (!img) { return NULL; }
    blink_Font *font = blink_alloc(sizeof(blink_Font));
    font->image = img;

    for (int i = 0; i < 256; i++) {
        blink_Glyph *g = &font->glyphs[i];
        blink_Rect r = {
            (img->w / 16) * (i % 16),
            (img->h / 16) * (i / 16),
            img->w / 16,
            img->h / 16
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

    font->glyphs[' '].rect = (blink_Rect) {0};
    font->glyphs[' '].xadv = font->glyphs['a'].xadv;

    return font;
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

    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
        if (lparam & (1 << 30)) { break; }
        ctx->key_state[(uint8_t) wparam] = BLINK_INPUT_DOWN | BLINK_INPUT_PRESSED;
        break;

    case WM_KEYUP:
    case WM_SYSKEYUP:
        ctx->key_state[(uint8_t) wparam] &= ~BLINK_INPUT_DOWN;
        ctx->key_state[(uint8_t) wparam] |= BLINK_INPUT_RELEASED;
        break;

    case WM_CHAR:
        if (wparam < 32) { break; }
        for (int i = 0; i < blink_lengthof(ctx->char_buf); i++) {
            if (ctx->char_buf[i]) { continue; }
            ctx->char_buf[i] = wparam;
            break;
        }
        break;

    case WM_LBUTTONDOWN: case WM_LBUTTONUP:
    case WM_RBUTTONDOWN: case WM_RBUTTONUP:
    case WM_MBUTTONDOWN: case WM_MBUTTONUP:
        int button = (message == WM_LBUTTONDOWN || message == WM_LBUTTONUP) ? 1 :
                     (message == WM_RBUTTONDOWN || message == WM_RBUTTONUP) ? 2 : 3;
        if (message == WM_LBUTTONDOWN || message == WM_RBUTTONDOWN || message == WM_MBUTTONDOWN) {
            SetCapture(hwnd);
            ctx->mouse_state[button] = BLINK_INPUT_DOWN | BLINK_INPUT_PRESSED;
        } else {
            ReleaseCapture();
            ctx->mouse_state[button] &= ~BLINK_INPUT_DOWN;
            ctx->mouse_state[button] |= BLINK_INPUT_RELEASED;
        }

    case WM_MOUSEMOVE:
        wr = blink_get_adjusted_window_rect(ctx);
        int prevx = ctx->mouse_pos.x;
        int prevy = ctx->mouse_pos.y;
        ctx->mouse_pos.x = (GET_X_LPARAM(lparam) - wr.x) * ctx->screen->w / wr.w;
        ctx->mouse_pos.y = (GET_Y_LPARAM(lparam) - wr.y) * ctx->screen->h / wr.h;
        ctx->mouse_delta.x += ctx->mouse_pos.x - prevx;
        ctx->mouse_delta.y += ctx->mouse_pos.y - prevy;
        break;

    case WM_MOUSEWHEEL:
        ctx->mouse_scroll = (float) GET_WHEEL_DELTA_WPARAM(wparam) / WHEEL_DELTA;
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
unhandled:
        return DefWindowProc(hwnd, message, wparam, lparam);
    }

    return 0;
}

static void *blink_font_data;
static int blink_font_size;

blink_Context *blink_create(const char *title, int width, int height, int scale) {
    blink_Context *ctx = blink_alloc(sizeof(blink_Context));

    ctx->screen = blink_create_image(width, height);
    ctx->clip = blink_rect(0, 0, width, height);

    RegisterClass(&(WNDCLASS) {
        .style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW,
        .lpfnWndProc = blink_wndproc,
        .hCursor = LoadCursor(0, IDC_ARROW),
        .lpszClassName = title
    });

    width *= scale;
    height *= scale;

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

    ctx->font = blink_load_font_mem(blink_font_data, blink_font_size);

    return ctx;
}

void blink_destroy(blink_Context *ctx) {
    ReleaseDC(ctx->hwnd, ctx->hdc);
    DestroyWindow(ctx->hwnd);
    blink_destroy_image(ctx->screen);
    blink_destroy_font(ctx->font);
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

    memset(ctx->char_buf, 0, sizeof(ctx->char_buf));
    for (int i = 0; i < sizeof(ctx->key_state); i++) {
        ctx->key_state[i] &= ~(BLINK_INPUT_PRESSED | BLINK_INPUT_RELEASED);
    }
    for (int i = 0; i < sizeof(ctx->mouse_state); i++) {
        ctx->mouse_state[i] &= ~(BLINK_INPUT_PRESSED | BLINK_INPUT_RELEASED);
    }
    ctx->mouse_scroll = 0;

    MSG msg;
    while (PeekMessage(&msg, ctx->hwnd, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return !ctx->should_quit;
}

void blink_set_target_fps(blink_Context *ctx, int fps) {
    if (fps < 1) {
        ctx->step_time = 0;
    } else {
        ctx->step_time = 1.0 / (double)fps;
    }
}

void blink_set_cursor_hidden(blink_Context *ctx, bool hidden) {
    ctx->hide_cursor = hidden;
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

blink_Font *blink_load_font_mem(void *data, int len) {
    return blink_load_font_from_image(blink_load_image_mem(data, len));
}

blink_Font *blink_load_font_file(const char *filename) {
    return blink_load_font_from_image(blink_load_image_file(filename));
}

void blink_destroy_font(blink_Font *font) {
    free(font->image);
    free(font);
}

int blink_text_width(blink_Font *font, const char *text) {
    int x = 0;
    for (uint8_t *p = (void*) text; *p; p++) {
        x += font->glyphs[*p].xadv;
    }
    return x;
}

int blink_get_char(blink_Context *ctx) {
    for (int i = 0; i < blink_lengthof(ctx->char_buf); i++) {
        if (!ctx->char_buf[i]) { continue; }
        int res = ctx->char_buf[i];
        ctx->char_buf[i] = 0;
        return res;
    }
    return 0;
}

bool blink_key_down(blink_Context *ctx, int key) {
    return blink_check_input_flag(ctx->key_state, key, sizeof(ctx->key_state), BLINK_INPUT_DOWN);
}

bool blink_key_pressed(blink_Context *ctx, int key) {
    return blink_check_input_flag(ctx->key_state, key, sizeof(ctx->key_state), BLINK_INPUT_PRESSED);
}

bool blink_key_released(blink_Context *ctx, int key) {
    return blink_check_input_flag(ctx->key_state, key, sizeof(ctx->key_state), BLINK_INPUT_RELEASED);
}

void blink_mouse_pos(blink_Context *ctx, int *x, int *y) {
    if (x) { *x = ctx->mouse_pos.x; }
    if (y) { *y = ctx->mouse_pos.y; }
}

void blink_mouse_delta(blink_Context *ctx, int *x, int *y) {
    if (x) { *x = ctx->mouse_delta.x; }
    if (y) { *y = ctx->mouse_delta.y; }
}

bool blink_mouse_down(blink_Context *ctx, int button) {
    return blink_check_input_flag(ctx->mouse_state, button, sizeof(ctx->mouse_state), BLINK_INPUT_DOWN);
}

bool blink_mouse_pressed(blink_Context *ctx, int button) {
    return blink_check_input_flag(ctx->mouse_state, button, sizeof(ctx->mouse_state), BLINK_INPUT_PRESSED);
}

bool blink_mouse_released(blink_Context *ctx, int button) {
    return blink_check_input_flag(ctx->mouse_state, button, sizeof(ctx->mouse_state), BLINK_INPUT_RELEASED);
}

float blink_mouse_scroll(blink_Context *ctx) {
    return ctx->mouse_scroll;
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

int blink_draw_text(blink_Context *ctx, const char *text, int x, int y, blink_Color color) {
    return blink_draw_text2(ctx, ctx->font, text, x, y, color);
}

int blink_draw_text2(blink_Context *ctx, blink_Font *font, const char *text, int x, int y, blink_Color color) {
    for (uint8_t *p = (void*) text; *p; p++) {
        blink_Glyph g = font->glyphs[*p];
        blink_draw_image2(ctx, font->image, x, y, g.rect, color);
        x += g.xadv;
    }
    return x;
}

static char blink_font[] = {
    0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00,
    0x0d, 0x49, 0x48, 0x44, 0x52, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00,
    0x00, 0x80, 0x01, 0x03, 0x00, 0x00, 0x00, 0xf9, 0xf0, 0xf3, 0x88,
    0x00, 0x00, 0x00, 0x06, 0x50, 0x4c, 0x54, 0x45, 0x00, 0x00, 0x00,
    0xff, 0xff, 0xff, 0xa5, 0xd9, 0x9f, 0xdd, 0x00, 0x00, 0x00, 0x01,
    0x74, 0x52, 0x4e, 0x53, 0x00, 0x40, 0xe6, 0xd8, 0x66, 0x00, 0x00,
    0x03, 0xb6, 0x49, 0x44, 0x41, 0x54, 0x48, 0xc7, 0xed, 0x55, 0xbd,
    0x8a, 0x1c, 0x47, 0x10, 0x2e, 0x3b, 0xb8, 0xa8, 0x91, 0x2f, 0x6c,
    0xd0, 0xa2, 0xc4, 0xb1, 0xa1, 0xf1, 0xd9, 0xb8, 0x31, 0xc3, 0x19,
    0x14, 0xf9, 0x31, 0x1a, 0x59, 0xb4, 0x14, 0x34, 0xf6, 0x46, 0x43,
    0x81, 0x9b, 0x39, 0x3f, 0x80, 0x1f, 0xc0, 0x0e, 0x15, 0xf8, 0x0d,
    0x1c, 0xd8, 0x81, 0xa1, 0xe1, 0xa0, 0xa3, 0xe2, 0x9c, 0x09, 0x83,
    0x96, 0x63, 0xa3, 0x8d, 0x8c, 0xd8, 0xe8, 0x98, 0xa0, 0x19, 0xf9,
    0xab, 0x9e, 0xe3, 0xe4, 0xb3, 0xfc, 0x08, 0xaa, 0xdd, 0xd9, 0x9e,
    0xf9, 0xb6, 0xba, 0x7e, 0xbe, 0xae, 0xaa, 0x21, 0x7a, 0x2f, 0xf7,
    0xc5, 0xa6, 0xe4, 0xc8, 0x07, 0x1b, 0xfa, 0xd3, 0x09, 0xd1, 0x90,
    0x52, 0xae, 0x29, 0x38, 0x17, 0x1d, 0x00, 0x03, 0x80, 0x96, 0x22,
    0xbe, 0x04, 0x3b, 0x28, 0x60, 0xf5, 0x9b, 0xd8, 0x8e, 0x14, 0xec,
    0x9b, 0x46, 0x8d, 0x9c, 0x02, 0x8b, 0x71, 0x3b, 0xba, 0xd5, 0xe8,
    0x96, 0xd2, 0x1c, 0x85, 0xd4, 0x86, 0x23, 0x57, 0xfa, 0x16, 0x57,
    0x47, 0x52, 0x2f, 0x00, 0x7e, 0xb8, 0x17, 0x41, 0xd7, 0xcf, 0x8e,
    0x79, 0xd3, 0x7c, 0x63, 0x56, 0xe5, 0xc0, 0x54, 0xb7, 0x22, 0x43,
    0x09, 0x22, 0xe2, 0xd4, 0x86, 0xd0, 0x95, 0x33, 0x26, 0xcd, 0xc5,
    0x28, 0x10, 0x9a, 0x35, 0x74, 0xed, 0xbc, 0x17, 0x33, 0x5b, 0xce,
    0x44, 0x85, 0x8c, 0xa5, 0x1b, 0x17, 0xcc, 0x62, 0xc4, 0x89, 0x51,
    0xa3, 0xd6, 0xd1, 0xc1, 0x89, 0xa8, 0xbe, 0x58, 0xd8, 0x68, 0x88,
    0x2d, 0xb7, 0xc6, 0x8f, 0x98, 0x1d, 0x6f, 0x9d, 0x83, 0x17, 0xf7,
    0x4e, 0x18, 0x88, 0xa3, 0x0d, 0xf3, 0xb2, 0x0c, 0xc2, 0x8f, 0x0e,
    0xc7, 0x5a, 0x3d, 0x55, 0x8e, 0x31, 0x3d, 0x79, 0x12, 0xc5, 0x99,
    0x18, 0x5e, 0x1f, 0x12, 0x5d, 0x4b, 0x2c, 0xf1, 0xf9, 0xf3, 0x02,
    0x20, 0x85, 0xe5, 0xa6, 0x02, 0xc8, 0x25, 0x32, 0x97, 0xe6, 0x0c,
    0x87, 0xe5, 0x1a, 0x40, 0xeb, 0x1a, 0x57, 0x70, 0x9c, 0x9e, 0xbc,
    0xbc, 0xaa, 0x54, 0x44, 0x6d, 0x04, 0xd8, 0xc0, 0x4d, 0xad, 0x89,
    0x58, 0xd4, 0xcb, 0xf1, 0x5c, 0x98, 0x0f, 0x8b, 0x7a, 0x79, 0x47,
    0x1a, 0xe3, 0x23, 0x52, 0xab, 0x2c, 0x5c, 0xf8, 0x94, 0x28, 0x4a,
    0x94, 0x5f, 0x3b, 0x50, 0x43, 0xb0, 0xbe, 0x03, 0x7b, 0xe4, 0x57,
    0x93, 0xfc, 0x18, 0x9c, 0x4d, 0x08, 0x5d, 0xf2, 0x16, 0xc0, 0x4b,
    0xcf, 0x36, 0x58, 0x5b, 0x91, 0xd1, 0x2e, 0x6d, 0x00, 0x2c, 0xde,
    0x7d, 0x1a, 0x40, 0x0f, 0x00, 0xd6, 0x10, 0xf8, 0x75, 0x72, 0x31,
    0x9c, 0x28, 0x70, 0xdc, 0x1c, 0x98, 0x9b, 0xab, 0x95, 0x17, 0xfe,
    0x90, 0xdf, 0x8d, 0xe2, 0x0d, 0x4e, 0x6b, 0x4f, 0x1b, 0xf2, 0xb4,
    0x77, 0x66, 0xbf, 0x55, 0xc8, 0x81, 0x28, 0x43, 0x49, 0xe9, 0x5a,
    0xf9, 0xb3, 0x1c, 0x18, 0x89, 0x8f, 0x69, 0x6b, 0xa2, 0x93, 0x19,
    0x56, 0x0c, 0x22, 0x91, 0xa3, 0x8c, 0x20, 0xc8, 0x2d, 0x22, 0x88,
    0x2c, 0x16, 0x69, 0x41, 0x22, 0x08, 0x5a, 0x01, 0xe4, 0x22, 0x25,
    0x64, 0xe8, 0x27, 0xf7, 0x52, 0x81, 0x71, 0xc7, 0x23, 0x1f, 0xcd,
    0x81, 0xe5, 0xc0, 0x55, 0xee, 0x22, 0x99, 0x89, 0xfe, 0x15, 0xd5,
    0xe9, 0xba, 0x6c, 0xec, 0x7e, 0x5c, 0xef, 0x6e, 0xcf, 0xcf, 0x59,
    0x14, 0x1f, 0xed, 0xc6, 0x5d, 0xce, 0x9d, 0xa0, 0x06, 0x80, 0x94,
    0xa0, 0xb1, 0x28, 0x1f, 0x49, 0x7e, 0xd2, 0x1c, 0x3a, 0x63, 0xdc,
    0xf9, 0x40, 0x05, 0x75, 0x8d, 0x9c, 0x83, 0xf9, 0x4c, 0x78, 0x49,
    0xf9, 0xe9, 0x0a, 0x04, 0x73, 0x9c, 0xed, 0xe8, 0x52, 0x35, 0x0d,
    0x5e, 0x94, 0xa0, 0x47, 0x6f, 0x23, 0x79, 0x2f, 0xff, 0x3b, 0x3f,
    0x50, 0x8b, 0x62, 0xcf, 0x65, 0x18, 0xf0, 0x30, 0x4c, 0x0a, 0x24,
    0x00, 0xdf, 0xd0, 0xe3, 0x74, 0x46, 0xf4, 0x58, 0x55, 0xa6, 0xa7,
    0x95, 0xa1, 0xf5, 0x73, 0x8a, 0x44, 0xbf, 0xeb, 0x96, 0x72, 0xcc,
    0x0d, 0x25, 0xf4, 0xe2, 0x5c, 0x1a, 0xfd, 0xd2, 0x81, 0x90, 0xb4,
    0x84, 0x5f, 0x50, 0x34, 0xab, 0xc6, 0x74, 0xc8, 0x4d, 0xb7, 0xd0,
    0x99, 0x59, 0x35, 0x6c, 0xab, 0xce, 0xca, 0x6a, 0xaf, 0xff, 0x58,
    0x3d, 0x7e, 0x26, 0xf5, 0xaa, 0x3f, 0xde, 0x7a, 0x6f, 0xe9, 0x02,
    0x45, 0xe2, 0xe9, 0xf2, 0xb2, 0x38, 0x4a, 0xd6, 0x18, 0x47, 0xaf,
    0xa0, 0x95, 0x50, 0x13, 0x1f, 0x63, 0x22, 0x4d, 0xd6, 0x52, 0x04,
    0x70, 0x9a, 0x50, 0x7d, 0x15, 0x43, 0xcf, 0x3a, 0x43, 0xf1, 0x7b,
    0x4b, 0xde, 0x9f, 0xfd, 0x71, 0xfd, 0x09, 0x0a, 0xda, 0x0e, 0x9e,
    0xe2, 0x43, 0x6d, 0x8c, 0xf8, 0xc5, 0xd9, 0xab, 0xb2, 0x26, 0x98,
    0x1f, 0xaa, 0x2f, 0xf9, 0x2e, 0x7e, 0x8e, 0x3a, 0x9d, 0xb4, 0x48,
    0x1e, 0xea, 0x54, 0xa4, 0x3f, 0xe5, 0xd9, 0x6d, 0x6d, 0x96, 0xb5,
    0x4a, 0xcb, 0x47, 0x1f, 0x10, 0xed, 0x1f, 0xf8, 0xb1, 0xba, 0x73,
    0xde, 0x6f, 0x5c, 0xc2, 0x25, 0x3a, 0x0a, 0x77, 0x1e, 0x31, 0x10,
    0x7d, 0x4d, 0xfd, 0xf2, 0xe8, 0x5d, 0x30, 0x54, 0x1a, 0x04, 0xd3,
    0x8e, 0x09, 0xf3, 0x36, 0xa3, 0x01, 0x25, 0x40, 0x9c, 0x63, 0x47,
    0x15, 0x82, 0x36, 0x61, 0x15, 0xa7, 0xd3, 0x6f, 0x81, 0xd4, 0x26,
    0x76, 0xd5, 0x70, 0xab, 0x46, 0x95, 0x2b, 0xb3, 0xda, 0xe0, 0xbb,
    0x1e, 0x79, 0xdb, 0x2c, 0xad, 0x6d, 0x1f, 0x0c, 0xe3, 0x25, 0x7d,
    0x89, 0x45, 0x1e, 0x1c, 0x91, 0x91, 0x66, 0xb5, 0xc3, 0xb4, 0xef,
    0xb7, 0x18, 0xb0, 0x37, 0x62, 0x41, 0xc8, 0xe0, 0xaf, 0x62, 0x44,
    0x3f, 0x66, 0xa1, 0x9b, 0xbf, 0x87, 0x61, 0xc8, 0xf1, 0xf4, 0x3a,
    0x46, 0x74, 0x4a, 0x9c, 0x29, 0x36, 0xdc, 0xd5, 0xe8, 0x6f, 0x14,
    0x18, 0x30, 0xcf, 0xe3, 0x0e, 0x1a, 0x15, 0x2f, 0x08, 0x05, 0x6c,
    0x9e, 0xa9, 0xa9, 0x8d, 0x6c, 0xe9, 0x37, 0xe0, 0x98, 0x3a, 0xe5,
    0x3f, 0xb5, 0x71, 0x2c, 0xb4, 0xdf, 0x4c, 0xa3, 0xa0, 0xb5, 0xb0,
    0xca, 0xde, 0x67, 0xe5, 0xe3, 0x72, 0xb7, 0x36, 0xee, 0x25, 0x2e,
    0x4c, 0x29, 0xe6, 0x01, 0x19, 0x5c, 0xb0, 0xae, 0xdb, 0xad, 0xdf,
    0x92, 0x31, 0x27, 0x06, 0x52, 0x04, 0x53, 0xa2, 0x37, 0x6a, 0xce,
    0xe7, 0x39, 0xe7, 0x0b, 0x30, 0x36, 0xb5, 0x0e, 0xe8, 0x3f, 0x10,
    0x2e, 0x25, 0x94, 0x0e, 0x4c, 0xd3, 0x57, 0xd3, 0x34, 0x5d, 0x98,
    0xd5, 0xd6, 0x70, 0x47, 0xc3, 0xdb, 0xc1, 0xfa, 0x97, 0xb2, 0x33,
    0xea, 0x49, 0xde, 0x92, 0xb4, 0x9d, 0xf7, 0x1b, 0xd9, 0x89, 0x3d,
    0xc1, 0x22, 0x1b, 0x92, 0xae, 0x71, 0x5b, 0x4c, 0xeb, 0xdb, 0xc0,
    0xcc, 0xfd, 0x04, 0xa6, 0x6f, 0xd5, 0x17, 0x26, 0x1a, 0x06, 0x6c,
    0x17, 0x9a, 0xfa, 0x02, 0x3e, 0xd6, 0x67, 0xb1, 0x63, 0x57, 0xc8,
    0x19, 0x73, 0xbd, 0x6f, 0xb1, 0x03, 0xa2, 0x99, 0x4c, 0x30, 0xf7,
    0x5f, 0x2e, 0x33, 0xcd, 0xff, 0x00, 0xf0, 0x33, 0x22, 0x99, 0xe0,
    0x7e, 0x00, 0x87, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4e, 0x44,
    0xae, 0x42, 0x60, 0x82
};

static void *blink_font_data = blink_font;
static int blink_font_size = sizeof(blink_font);
