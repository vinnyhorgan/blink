#include "blink_api.h"

#include "blink_audio.h"

#include <string.h>

#define ABORT_VM(vm, error) do {         \
        wrenSetSlotString(vm, 0, error); \
        wrenAbortFiber(vm, 0);           \
    } while (false);

#define ASSERT_TYPE(vm, slot, type, field_name)                        \
    if (wrenGetSlotType(vm, slot) != WREN_TYPE_##type) {               \
        ABORT_VM(vm, "Expected " #field_name " to be of type " #type); \
        return;                                                        \
    }

//--------------------
// Graphics
//--------------------

void api_graphics_clip(WrenVM *vm) {
    blink_state *state = (blink_state*)wrenGetUserData(vm);
    ASSERT_TYPE(vm, 1, NUM, "x");
    ASSERT_TYPE(vm, 2, NUM, "y");
    ASSERT_TYPE(vm, 3, NUM, "w");
    ASSERT_TYPE(vm, 4, NUM, "h");
    int x = (int)wrenGetSlotDouble(vm, 1);
    int y = (int)wrenGetSlotDouble(vm, 2);
    int w = (int)wrenGetSlotDouble(vm, 3);
    int h = (int)wrenGetSlotDouble(vm, 4);
    bg_set_clip(state->screen, bg_new_rect(x, y, w, h));
}

void api_graphics_clear(WrenVM *vm) {
    blink_state *state = (blink_state*)wrenGetUserData(vm);
    ASSERT_TYPE(vm, 1, FOREIGN, "color");
    bg_color *color = (bg_color*)wrenGetSlotForeign(vm, 1);
    bg_clear(state->screen, *color);
}

void api_graphics_get(WrenVM *vm) {
    blink_state *state = (blink_state*)wrenGetUserData(vm);
    ASSERT_TYPE(vm, 1, NUM, "x");
    ASSERT_TYPE(vm, 2, NUM, "y");
    int x = (int)wrenGetSlotDouble(vm, 1);
    int y = (int)wrenGetSlotDouble(vm, 2);
    wrenSetSlotHandle(vm, 1, state->color_class);
    bg_color *color = wrenSetSlotNewForeign(vm, 0, 1, sizeof(bg_color));
    *color = bg_get_pixel(state->screen, x, y);
}

void api_graphics_set(WrenVM *vm) {
    blink_state *state = (blink_state*)wrenGetUserData(vm);
    ASSERT_TYPE(vm, 1, NUM, "x");
    ASSERT_TYPE(vm, 2, NUM, "y");
    ASSERT_TYPE(vm, 3, FOREIGN, "color");
    int x = (int)wrenGetSlotDouble(vm, 1);
    int y = (int)wrenGetSlotDouble(vm, 2);
    bg_color *color = (bg_color*)wrenGetSlotForeign(vm, 3);
    bg_set_pixel(state->screen, x, y, *color);
}

void api_graphics_line(WrenVM *vm) {
    blink_state *state = (blink_state*)wrenGetUserData(vm);
    ASSERT_TYPE(vm, 1, NUM, "x0");
    ASSERT_TYPE(vm, 2, NUM, "y0");
    ASSERT_TYPE(vm, 3, NUM, "x1");
    ASSERT_TYPE(vm, 4, NUM, "y1");
    ASSERT_TYPE(vm, 5, FOREIGN, "color");
    int x0 = (int)wrenGetSlotDouble(vm, 1);
    int y0 = (int)wrenGetSlotDouble(vm, 2);
    int x1 = (int)wrenGetSlotDouble(vm, 3);
    int y1 = (int)wrenGetSlotDouble(vm, 4);
    bg_color *color = (bg_color*)wrenGetSlotForeign(vm, 5);
    bg_line(state->screen, x0, y0, x1, y1, *color);
}

void api_graphics_fill(WrenVM *vm) {
    blink_state *state = (blink_state*)wrenGetUserData(vm);
    ASSERT_TYPE(vm, 1, NUM, "x");
    ASSERT_TYPE(vm, 2, NUM, "y");
    ASSERT_TYPE(vm, 3, NUM, "w");
    ASSERT_TYPE(vm, 4, NUM, "h");
    ASSERT_TYPE(vm, 5, FOREIGN, "color");
    int x = (int)wrenGetSlotDouble(vm, 1);
    int y = (int)wrenGetSlotDouble(vm, 2);
    int w = (int)wrenGetSlotDouble(vm, 3);
    int h = (int)wrenGetSlotDouble(vm, 4);
    bg_color *color = (bg_color*)wrenGetSlotForeign(vm, 5);
    bg_fill(state->screen, x, y, w, h, *color);
}

void api_graphics_rectangle(WrenVM *vm) {
    blink_state *state = (blink_state*)wrenGetUserData(vm);
    ASSERT_TYPE(vm, 1, NUM, "x");
    ASSERT_TYPE(vm, 2, NUM, "y");
    ASSERT_TYPE(vm, 3, NUM, "w");
    ASSERT_TYPE(vm, 4, NUM, "h");
    ASSERT_TYPE(vm, 5, FOREIGN, "color");
    int x = (int)wrenGetSlotDouble(vm, 1);
    int y = (int)wrenGetSlotDouble(vm, 2);
    int w = (int)wrenGetSlotDouble(vm, 3);
    int h = (int)wrenGetSlotDouble(vm, 4);
    bg_color *color = (bg_color*)wrenGetSlotForeign(vm, 5);
    bg_rectangle(state->screen, x, y, w, h, *color);
}

void api_graphics_fill_rectangle(WrenVM *vm) {
    blink_state *state = (blink_state*)wrenGetUserData(vm);
    ASSERT_TYPE(vm, 1, NUM, "x");
    ASSERT_TYPE(vm, 2, NUM, "y");
    ASSERT_TYPE(vm, 3, NUM, "w");
    ASSERT_TYPE(vm, 4, NUM, "h");
    ASSERT_TYPE(vm, 5, FOREIGN, "color");
    int x = (int)wrenGetSlotDouble(vm, 1);
    int y = (int)wrenGetSlotDouble(vm, 2);
    int w = (int)wrenGetSlotDouble(vm, 3);
    int h = (int)wrenGetSlotDouble(vm, 4);
    bg_color *color = (bg_color*)wrenGetSlotForeign(vm, 5);
    bg_fill_rectangle(state->screen, x, y, w, h, *color);
}

void api_graphics_circle(WrenVM *vm) {
    blink_state *state = (blink_state*)wrenGetUserData(vm);
    ASSERT_TYPE(vm, 1, NUM, "x");
    ASSERT_TYPE(vm, 2, NUM, "y");
    ASSERT_TYPE(vm, 3, NUM, "r");
    ASSERT_TYPE(vm, 4, FOREIGN, "color");
    int x = (int)wrenGetSlotDouble(vm, 1);
    int y = (int)wrenGetSlotDouble(vm, 2);
    int r = (int)wrenGetSlotDouble(vm, 3);
    bg_color *color = (bg_color*)wrenGetSlotForeign(vm, 4);
    bg_circle(state->screen, x, y, r, *color);
}

void api_graphics_fill_circle(WrenVM *vm) {
    blink_state *state = (blink_state*)wrenGetUserData(vm);
    ASSERT_TYPE(vm, 1, NUM, "x");
    ASSERT_TYPE(vm, 2, NUM, "y");
    ASSERT_TYPE(vm, 3, NUM, "r");
    ASSERT_TYPE(vm, 4, FOREIGN, "color");
    int x = (int)wrenGetSlotDouble(vm, 1);
    int y = (int)wrenGetSlotDouble(vm, 2);
    int r = (int)wrenGetSlotDouble(vm, 3);
    bg_color *color = (bg_color*)wrenGetSlotForeign(vm, 4);
    bg_fill_circle(state->screen, x, y, r, *color);
}

void api_graphics_blit(WrenVM *vm) {
    blink_state *state = (blink_state*)wrenGetUserData(vm);
    ASSERT_TYPE(vm, 1, FOREIGN, "image");
    ASSERT_TYPE(vm, 2, NUM, "dx");
    ASSERT_TYPE(vm, 3, NUM, "dy");
    ASSERT_TYPE(vm, 4, NUM, "sx");
    ASSERT_TYPE(vm, 5, NUM, "sy");
    ASSERT_TYPE(vm, 6, NUM, "w");
    ASSERT_TYPE(vm, 7, NUM, "h");
    bg_image **image = (bg_image**)wrenGetSlotForeign(vm, 1);
    int dx = (int)wrenGetSlotDouble(vm, 2);
    int dy = (int)wrenGetSlotDouble(vm, 3);
    int sx = (int)wrenGetSlotDouble(vm, 4);
    int sy = (int)wrenGetSlotDouble(vm, 5);
    int w = (int)wrenGetSlotDouble(vm, 6);
    int h = (int)wrenGetSlotDouble(vm, 7);
    bg_blit(state->screen, *image, dx, dy, sx, sy, w, h);
}

void api_graphics_blit_alpha(WrenVM *vm) {
    blink_state *state = (blink_state*)wrenGetUserData(vm);
    ASSERT_TYPE(vm, 1, FOREIGN, "image");
    ASSERT_TYPE(vm, 2, NUM, "dx");
    ASSERT_TYPE(vm, 3, NUM, "dy");
    ASSERT_TYPE(vm, 4, NUM, "sx");
    ASSERT_TYPE(vm, 5, NUM, "sy");
    ASSERT_TYPE(vm, 6, NUM, "w");
    ASSERT_TYPE(vm, 7, NUM, "h");
    ASSERT_TYPE(vm, 8, NUM, "alpha");
    bg_image **image = (bg_image**)wrenGetSlotForeign(vm, 1);
    int dx = (int)wrenGetSlotDouble(vm, 2);
    int dy = (int)wrenGetSlotDouble(vm, 3);
    int sx = (int)wrenGetSlotDouble(vm, 4);
    int sy = (int)wrenGetSlotDouble(vm, 5);
    int w = (int)wrenGetSlotDouble(vm, 6);
    int h = (int)wrenGetSlotDouble(vm, 7);
    float alpha = (float)wrenGetSlotDouble(vm, 8);
    bg_blit_alpha(state->screen, *image, dx, dy, sx, sy, w, h, alpha);
}

void api_graphics_blit_tint(WrenVM *vm) {
    blink_state *state = (blink_state*)wrenGetUserData(vm);
    ASSERT_TYPE(vm, 1, FOREIGN, "image");
    ASSERT_TYPE(vm, 2, NUM, "dx");
    ASSERT_TYPE(vm, 3, NUM, "dy");
    ASSERT_TYPE(vm, 4, NUM, "sx");
    ASSERT_TYPE(vm, 5, NUM, "sy");
    ASSERT_TYPE(vm, 6, NUM, "w");
    ASSERT_TYPE(vm, 7, NUM, "h");
    ASSERT_TYPE(vm, 8, FOREIGN, "tint");
    bg_image **image = (bg_image**)wrenGetSlotForeign(vm, 1);
    int dx = (int)wrenGetSlotDouble(vm, 2);
    int dy = (int)wrenGetSlotDouble(vm, 3);
    int sx = (int)wrenGetSlotDouble(vm, 4);
    int sy = (int)wrenGetSlotDouble(vm, 5);
    int w = (int)wrenGetSlotDouble(vm, 6);
    int h = (int)wrenGetSlotDouble(vm, 7);
    bg_color *tint = (bg_color*)wrenGetSlotForeign(vm, 8);
    bg_blit_tint(state->screen, *image, dx, dy, sx, sy, w, h, *tint);
}

void api_graphics_print(WrenVM *vm) {
    blink_state *state = (blink_state*)wrenGetUserData(vm);
    ASSERT_TYPE(vm, 1, STRING, "text");
    ASSERT_TYPE(vm, 2, NUM, "x");
    ASSERT_TYPE(vm, 3, NUM, "y");
    ASSERT_TYPE(vm, 4, FOREIGN, "color");
    const char *text = (char*)wrenGetSlotString(vm, 1);
    int x = (int)wrenGetSlotDouble(vm, 2);
    int y = (int)wrenGetSlotDouble(vm, 3);
    bg_color *color = (bg_color*)wrenGetSlotForeign(vm, 4);
    bg_print(state->screen, state->font, text, x, y, *color);
}

void api_graphics_print_font(WrenVM *vm) {
    blink_state *state = (blink_state*)wrenGetUserData(vm);
    ASSERT_TYPE(vm, 1, FOREIGN, "font");
    ASSERT_TYPE(vm, 2, STRING, "text");
    ASSERT_TYPE(vm, 3, NUM, "x");
    ASSERT_TYPE(vm, 4, NUM, "y");
    ASSERT_TYPE(vm, 5, FOREIGN, "color");
    bg_font **font = (bg_font**)wrenGetSlotForeign(vm, 1);
    const char *text = (char*)wrenGetSlotString(vm, 2);
    int x = (int)wrenGetSlotDouble(vm, 3);
    int y = (int)wrenGetSlotDouble(vm, 4);
    bg_color *color = (bg_color*)wrenGetSlotForeign(vm, 5);
    bg_print(state->screen, *font, text, x, y, *color);
}

void api_graphics_screenshot(WrenVM *vm) {
    blink_state *state = (blink_state*)wrenGetUserData(vm);
    wrenSetSlotHandle(vm, 1, state->image_class);
    bg_image **image = wrenSetSlotNewForeign(vm, 0, 1, sizeof(bg_image*));
    *image = bg_new_image(state->width, state->height);
    memcpy((*image)->pixels, state->screen->pixels, state->width * state->height * 4);
}

void api_graphics_measure(WrenVM *vm) {
    blink_state *state = (blink_state*)wrenGetUserData(vm);
    ASSERT_TYPE(vm, 1, STRING, "text");
    const char *text = wrenGetSlotString(vm, 1);
    wrenSetSlotDouble(vm, 0, bg_text_width(state->font, text));
}

void api_graphics_get_width(WrenVM *vm) {
    blink_state *state = (blink_state*)wrenGetUserData(vm);
    wrenSetSlotDouble(vm, 0, state->width);
}

void api_graphics_get_height(WrenVM *vm) {
    blink_state *state = (blink_state*)wrenGetUserData(vm);
    wrenSetSlotDouble(vm, 0, state->height);
}

void api_graphics_set_clear_color(WrenVM *vm) {
    blink_state *state = (blink_state*)wrenGetUserData(vm);
    ASSERT_TYPE(vm, 1, FOREIGN, "color");
    bg_color *color = (bg_color*)wrenGetSlotForeign(vm, 1);
    state->clear_color = *color;
}

void api_color_allocate(WrenVM *vm) {
    wrenEnsureSlots(vm, 1);
    wrenSetSlotNewForeign(vm, 0, 0, sizeof(bg_color));
}

void api_color_new_rgba(WrenVM *vm) {
    bg_color *color = (bg_color*)wrenGetSlotForeign(vm, 0);
    ASSERT_TYPE(vm, 1, NUM, "r");
    ASSERT_TYPE(vm, 2, NUM, "g");
    ASSERT_TYPE(vm, 3, NUM, "b");
    ASSERT_TYPE(vm, 4, NUM, "a");
    int r = (int)wrenGetSlotDouble(vm, 1);
    int g = (int)wrenGetSlotDouble(vm, 2);
    int b = (int)wrenGetSlotDouble(vm, 3);
    int a = (int)wrenGetSlotDouble(vm, 4);
    color->r = r;
    color->g = g;
    color->b = b;
    color->a = a;
}

void api_color_new_rgb(WrenVM *vm) {
    bg_color *color = (bg_color*)wrenGetSlotForeign(vm, 0);
    ASSERT_TYPE(vm, 1, NUM, "r");
    ASSERT_TYPE(vm, 2, NUM, "g");
    ASSERT_TYPE(vm, 3, NUM, "b");
    int r = (int)wrenGetSlotDouble(vm, 1);
    int g = (int)wrenGetSlotDouble(vm, 2);
    int b = (int)wrenGetSlotDouble(vm, 3);
    color->r = r;
    color->g = g;
    color->b = b;
    color->a = 255;
}

void api_color_get_index(WrenVM *vm) {
    uint8_t *color = (uint8_t*)wrenGetSlotForeign(vm, 0);
    ASSERT_TYPE(vm, 1, NUM, "index");
    int index = (int)wrenGetSlotDouble(vm, 1);

    if (index < 0 || index > 3) {
        ABORT_VM(vm, "Invalid color index");
        return;
    }

    wrenSetSlotDouble(vm, 0, color[index]);
}

void api_color_set_index(WrenVM *vm) {
    uint8_t *color = (uint8_t*)wrenGetSlotForeign(vm, 0);
    ASSERT_TYPE(vm, 1, NUM, "index");
    ASSERT_TYPE(vm, 2, NUM, "value");
    int index = (int)wrenGetSlotDouble(vm, 1);
    int value = (int)wrenGetSlotDouble(vm, 2);

    if (index < 0 || index > 3) {
        ABORT_VM(vm, "Invalid color index");
        return;
    }

    color[index] = value;
}

void api_image_allocate(WrenVM *vm) {
    wrenEnsureSlots(vm, 1);
    wrenSetSlotNewForeign(vm, 0, 0, sizeof(bg_image*));
}

void api_image_finalize(void *data) {
    bg_image **image = (bg_image**)data;
    bg_destroy_image(*image);
}

void api_image_new(WrenVM *vm) {
    bg_image **image = (bg_image**)wrenGetSlotForeign(vm, 0);
    ASSERT_TYPE(vm, 1, NUM, "w");
    ASSERT_TYPE(vm, 2, NUM, "h");
    int w = (int)wrenGetSlotDouble(vm, 1);
    int h = (int)wrenGetSlotDouble(vm, 2);

    if (w <= 0 || h <= 0) {
        ABORT_VM(vm, "Invalid image dimensions");
        return;
    }

    *image = bg_new_image(w, h);
}

void api_image_new_filename(WrenVM *vm) {
    bg_image **image = (bg_image**)wrenGetSlotForeign(vm, 0);
    ASSERT_TYPE(vm, 1, STRING, "filename");
    const char *filename = wrenGetSlotString(vm, 1);
    bg_image *loaded_image = bg_load_image_file(filename);
    if (!loaded_image) {
        ABORT_VM(vm, "Failed to load image");
        return;
    }

    *image = loaded_image;
}

void api_image_new_from_memory(WrenVM *vm) {
    bg_image **image = (bg_image**)wrenGetSlotForeign(vm, 0);
    ASSERT_TYPE(vm, 1, STRING, "data");
    int size;
    const char *data = wrenGetSlotBytes(vm, 1, &size);
    bg_image *loaded_image = bg_load_image_mem((void*)data, size);
    if (!loaded_image) {
        ABORT_VM(vm, "Failed to load image");
        return;
    }

    *image = loaded_image;
}

void api_image_clip(WrenVM *vm) {
    bg_image **image = (bg_image**)wrenGetSlotForeign(vm, 0);
    ASSERT_TYPE(vm, 1, NUM, "x");
    ASSERT_TYPE(vm, 2, NUM, "y");
    ASSERT_TYPE(vm, 3, NUM, "w");
    ASSERT_TYPE(vm, 4, NUM, "h");
    int x = (int)wrenGetSlotDouble(vm, 1);
    int y = (int)wrenGetSlotDouble(vm, 2);
    int w = (int)wrenGetSlotDouble(vm, 3);
    int h = (int)wrenGetSlotDouble(vm, 4);
    bg_set_clip(*image, bg_new_rect(x, y, w, h));
}

void api_image_clear(WrenVM *vm) {
    bg_image **image = (bg_image**)wrenGetSlotForeign(vm, 0);
    ASSERT_TYPE(vm, 1, FOREIGN, "color");
    bg_color *color = (bg_color*)wrenGetSlotForeign(vm, 1);
    bg_clear(*image, *color);
}

void api_image_get(WrenVM *vm) {
    bg_image **image = (bg_image**)wrenGetSlotForeign(vm, 0);
    blink_state *state = (blink_state*)wrenGetUserData(vm);
    ASSERT_TYPE(vm, 1, NUM, "x");
    ASSERT_TYPE(vm, 2, NUM, "y");
    int x = (int)wrenGetSlotDouble(vm, 1);
    int y = (int)wrenGetSlotDouble(vm, 2);
    wrenSetSlotHandle(vm, 1, state->color_class);
    bg_color *color = wrenSetSlotNewForeign(vm, 0, 1, sizeof(bg_color));
    *color = bg_get_pixel(*image, x, y);
}

void api_image_set(WrenVM *vm) {
    bg_image **image = (bg_image**)wrenGetSlotForeign(vm, 0);
    ASSERT_TYPE(vm, 1, NUM, "x");
    ASSERT_TYPE(vm, 2, NUM, "y");
    ASSERT_TYPE(vm, 3, FOREIGN, "color");
    int x = (int)wrenGetSlotDouble(vm, 1);
    int y = (int)wrenGetSlotDouble(vm, 2);
    bg_color *color = (bg_color*)wrenGetSlotForeign(vm, 3);
    bg_set_pixel(*image, x, y, *color);
}

void api_image_line(WrenVM *vm) {
    bg_image **image = (bg_image**)wrenGetSlotForeign(vm, 0);
    ASSERT_TYPE(vm, 1, NUM, "x0");
    ASSERT_TYPE(vm, 2, NUM, "y0");
    ASSERT_TYPE(vm, 3, NUM, "x1");
    ASSERT_TYPE(vm, 4, NUM, "y1");
    ASSERT_TYPE(vm, 5, FOREIGN, "color");
    int x0 = (int)wrenGetSlotDouble(vm, 1);
    int y0 = (int)wrenGetSlotDouble(vm, 2);
    int x1 = (int)wrenGetSlotDouble(vm, 3);
    int y1 = (int)wrenGetSlotDouble(vm, 4);
    bg_color *color = (bg_color*)wrenGetSlotForeign(vm, 5);
    bg_line(*image, x0, y0, x1, y1, *color);
}

void api_image_fill(WrenVM *vm) {
    bg_image **image = (bg_image**)wrenGetSlotForeign(vm, 0);
    ASSERT_TYPE(vm, 1, NUM, "x");
    ASSERT_TYPE(vm, 2, NUM, "y");
    ASSERT_TYPE(vm, 3, NUM, "w");
    ASSERT_TYPE(vm, 4, NUM, "h");
    ASSERT_TYPE(vm, 5, FOREIGN, "color");
    int x = (int)wrenGetSlotDouble(vm, 1);
    int y = (int)wrenGetSlotDouble(vm, 2);
    int w = (int)wrenGetSlotDouble(vm, 3);
    int h = (int)wrenGetSlotDouble(vm, 4);
    bg_color *color = (bg_color*)wrenGetSlotForeign(vm, 5);
    bg_fill(*image, x, y, w, h, *color);
}

void api_image_rectangle(WrenVM *vm) {
    bg_image **image = (bg_image**)wrenGetSlotForeign(vm, 0);
    ASSERT_TYPE(vm, 1, NUM, "x");
    ASSERT_TYPE(vm, 2, NUM, "y");
    ASSERT_TYPE(vm, 3, NUM, "w");
    ASSERT_TYPE(vm, 4, NUM, "h");
    ASSERT_TYPE(vm, 5, FOREIGN, "color");
    int x = (int)wrenGetSlotDouble(vm, 1);
    int y = (int)wrenGetSlotDouble(vm, 2);
    int w = (int)wrenGetSlotDouble(vm, 3);
    int h = (int)wrenGetSlotDouble(vm, 4);
    bg_color *color = (bg_color*)wrenGetSlotForeign(vm, 5);
    bg_rectangle(*image, x, y, w, h, *color);
}

void api_image_fill_rectangle(WrenVM *vm) {
    bg_image **image = (bg_image**)wrenGetSlotForeign(vm, 0);
    ASSERT_TYPE(vm, 1, NUM, "x");
    ASSERT_TYPE(vm, 2, NUM, "y");
    ASSERT_TYPE(vm, 3, NUM, "w");
    ASSERT_TYPE(vm, 4, NUM, "h");
    ASSERT_TYPE(vm, 5, FOREIGN, "color");
    int x = (int)wrenGetSlotDouble(vm, 1);
    int y = (int)wrenGetSlotDouble(vm, 2);
    int w = (int)wrenGetSlotDouble(vm, 3);
    int h = (int)wrenGetSlotDouble(vm, 4);
    bg_color *color = (bg_color*)wrenGetSlotForeign(vm, 5);
    bg_fill_rectangle(*image, x, y, w, h, *color);
}

void api_image_circle(WrenVM *vm) {
    bg_image **image = (bg_image**)wrenGetSlotForeign(vm, 0);
    ASSERT_TYPE(vm, 1, NUM, "x");
    ASSERT_TYPE(vm, 2, NUM, "y");
    ASSERT_TYPE(vm, 3, NUM, "r");
    ASSERT_TYPE(vm, 4, FOREIGN, "color");
    int x = (int)wrenGetSlotDouble(vm, 1);
    int y = (int)wrenGetSlotDouble(vm, 2);
    int r = (int)wrenGetSlotDouble(vm, 3);
    bg_color *color = (bg_color*)wrenGetSlotForeign(vm, 4);
    bg_circle(*image, x, y, r, *color);
}

void api_image_fill_circle(WrenVM *vm) {
    bg_image **image = (bg_image**)wrenGetSlotForeign(vm, 0);
    ASSERT_TYPE(vm, 1, NUM, "x");
    ASSERT_TYPE(vm, 2, NUM, "y");
    ASSERT_TYPE(vm, 3, NUM, "r");
    ASSERT_TYPE(vm, 4, FOREIGN, "color");
    int x = (int)wrenGetSlotDouble(vm, 1);
    int y = (int)wrenGetSlotDouble(vm, 2);
    int r = (int)wrenGetSlotDouble(vm, 3);
    bg_color *color = (bg_color*)wrenGetSlotForeign(vm, 4);
    bg_fill_circle(*image, x, y, r, *color);
}

void api_image_blit(WrenVM *vm) {
    bg_image **image = (bg_image**)wrenGetSlotForeign(vm, 0);
    ASSERT_TYPE(vm, 1, FOREIGN, "image");
    ASSERT_TYPE(vm, 2, NUM, "dx");
    ASSERT_TYPE(vm, 3, NUM, "dy");
    ASSERT_TYPE(vm, 4, NUM, "sx");
    ASSERT_TYPE(vm, 5, NUM, "sy");
    ASSERT_TYPE(vm, 6, NUM, "w");
    ASSERT_TYPE(vm, 7, NUM, "h");
    bg_image **src_image = (bg_image**)wrenGetSlotForeign(vm, 1);
    int dx = (int)wrenGetSlotDouble(vm, 2);
    int dy = (int)wrenGetSlotDouble(vm, 3);
    int sx = (int)wrenGetSlotDouble(vm, 4);
    int sy = (int)wrenGetSlotDouble(vm, 5);
    int w = (int)wrenGetSlotDouble(vm, 6);
    int h = (int)wrenGetSlotDouble(vm, 7);
    bg_blit(*image, *src_image, dx, dy, sx, sy, w, h);
}

void api_image_blit_alpha(WrenVM *vm) {
    bg_image **image = (bg_image**)wrenGetSlotForeign(vm, 0);
    ASSERT_TYPE(vm, 1, FOREIGN, "image");
    ASSERT_TYPE(vm, 2, NUM, "dx");
    ASSERT_TYPE(vm, 3, NUM, "dy");
    ASSERT_TYPE(vm, 4, NUM, "sx");
    ASSERT_TYPE(vm, 5, NUM, "sy");
    ASSERT_TYPE(vm, 6, NUM, "w");
    ASSERT_TYPE(vm, 7, NUM, "h");
    ASSERT_TYPE(vm, 8, NUM, "alpha");
    bg_image **src_image = (bg_image**)wrenGetSlotForeign(vm, 1);
    int dx = (int)wrenGetSlotDouble(vm, 2);
    int dy = (int)wrenGetSlotDouble(vm, 3);
    int sx = (int)wrenGetSlotDouble(vm, 4);
    int sy = (int)wrenGetSlotDouble(vm, 5);
    int w = (int)wrenGetSlotDouble(vm, 6);
    int h = (int)wrenGetSlotDouble(vm, 7);
    float alpha = (float)wrenGetSlotDouble(vm, 8);
    bg_blit_alpha(*image, *src_image, dx, dy, sx, sy, w, h, alpha);
}

void api_image_blit_tint(WrenVM *vm) {
    bg_image **image = (bg_image**)wrenGetSlotForeign(vm, 0);
    ASSERT_TYPE(vm, 1, FOREIGN, "image");
    ASSERT_TYPE(vm, 2, NUM, "dx");
    ASSERT_TYPE(vm, 3, NUM, "dy");
    ASSERT_TYPE(vm, 4, NUM, "sx");
    ASSERT_TYPE(vm, 5, NUM, "sy");
    ASSERT_TYPE(vm, 6, NUM, "w");
    ASSERT_TYPE(vm, 7, NUM, "h");
    ASSERT_TYPE(vm, 8, FOREIGN, "tint");
    bg_image **src_image = (bg_image**)wrenGetSlotForeign(vm, 1);
    int dx = (int)wrenGetSlotDouble(vm, 2);
    int dy = (int)wrenGetSlotDouble(vm, 3);
    int sx = (int)wrenGetSlotDouble(vm, 4);
    int sy = (int)wrenGetSlotDouble(vm, 5);
    int w = (int)wrenGetSlotDouble(vm, 6);
    int h = (int)wrenGetSlotDouble(vm, 7);
    bg_color *tint = (bg_color*)wrenGetSlotForeign(vm, 8);
    bg_blit_tint(*image, *src_image, dx, dy, sx, sy, w, h, *tint);
}

void api_image_print(WrenVM *vm) {
    bg_image **image = (bg_image**)wrenGetSlotForeign(vm, 0);
    blink_state *state = (blink_state*)wrenGetUserData(vm);
    ASSERT_TYPE(vm, 1, STRING, "text");
    ASSERT_TYPE(vm, 2, NUM, "x");
    ASSERT_TYPE(vm, 3, NUM, "y");
    ASSERT_TYPE(vm, 4, FOREIGN, "color");
    char *text = (char*)wrenGetSlotString(vm, 1);
    int x = (int)wrenGetSlotDouble(vm, 2);
    int y = (int)wrenGetSlotDouble(vm, 3);
    bg_color *color = (bg_color*)wrenGetSlotForeign(vm, 4);
    bg_print(*image, state->font, text, x, y, *color);
}

void api_image_print_font(WrenVM *vm) {
    bg_image **image = (bg_image**)wrenGetSlotForeign(vm, 0);
    ASSERT_TYPE(vm, 1, FOREIGN, "font");
    ASSERT_TYPE(vm, 2, STRING, "text");
    ASSERT_TYPE(vm, 3, NUM, "x");
    ASSERT_TYPE(vm, 4, NUM, "y");
    ASSERT_TYPE(vm, 5, FOREIGN, "color");
    bg_font **font = (bg_font**)wrenGetSlotForeign(vm, 1);
    const char *text = (char*)wrenGetSlotString(vm, 2);
    int x = (int)wrenGetSlotDouble(vm, 3);
    int y = (int)wrenGetSlotDouble(vm, 4);
    bg_color *color = (bg_color*)wrenGetSlotForeign(vm, 5);
    bg_print(*image, *font, text, x, y, *color);
}

void api_image_resize(WrenVM *vm) {
    bg_image **image = (bg_image**)wrenGetSlotForeign(vm, 0);
    ASSERT_TYPE(vm, 1, NUM, "w");
    ASSERT_TYPE(vm, 2, NUM, "h");
    int w = (int)wrenGetSlotDouble(vm, 1);
    int h = (int)wrenGetSlotDouble(vm, 2);

    if (w <= 0 || h <= 0) {
        ABORT_VM(vm, "Invalid image dimensions");
        return;
    }

    bg_image *resized_image = bg_resize_image(*image, w, h);
    bg_destroy_image(*image);
    *image = resized_image;
}

void api_image_save(WrenVM *vm) {
    bg_image **image = (bg_image**)wrenGetSlotForeign(vm, 0);
    ASSERT_TYPE(vm, 1, STRING, "type");
    ASSERT_TYPE(vm, 2, STRING, "filename");
    const char *type = wrenGetSlotString(vm, 1);
    const char *filename = wrenGetSlotString(vm, 2);

    if (!strcmp(type, "png")) {
        wrenSetSlotBool(vm, 0, bg_save_image(*image, BG_IMAGE_PNG, filename));
    } else if (!strcmp(type, "jpg")) {
        wrenSetSlotBool(vm, 0, bg_save_image(*image, BG_IMAGE_JPG, filename));
    } else {
        ABORT_VM(vm, "Invalid image type (png or jpg expected)");
    }
}

void api_image_save_to_memory(WrenVM *vm) {
    bg_image **image = (bg_image**)wrenGetSlotForeign(vm, 0);
    int size;
    void *data = bg_save_image_mem(*image, &size);

    if (data) {
        wrenSetSlotBytes(vm, 0, data, size);
    } else {
        wrenSetSlotNull(vm, 0);
    }
}

void api_image_get_width(WrenVM *vm) {
    bg_image **image = (bg_image**)wrenGetSlotForeign(vm, 0);
    wrenSetSlotDouble(vm, 0, (*image)->w);
}

void api_image_get_height(WrenVM *vm) {
    bg_image **image = (bg_image**)wrenGetSlotForeign(vm, 0);
    wrenSetSlotDouble(vm, 0, (*image)->h);
}

void api_font_allocate(WrenVM *vm) {
    wrenEnsureSlots(vm, 1);
    wrenSetSlotNewForeign(vm, 0, 0, sizeof(bg_font*));
}

void api_font_finalize(void *data) {
    bg_font **font = (bg_font**)data;
    bg_destroy_font(*font);
}

void api_font_new(WrenVM *vm) {
    bg_font **font = (bg_font**)wrenGetSlotForeign(vm, 0);
    ASSERT_TYPE(vm, 1, STRING, "filename");
    const char *filename = wrenGetSlotString(vm, 1);
    bg_font *loaded_font = bg_load_font_file(filename);
    if (!loaded_font) {
        ABORT_VM(vm, "Failed to load font");
        return;
    }

    *font = loaded_font;
}

void api_font_new_from_memory(WrenVM *vm) {
    bg_font **font = (bg_font**)wrenGetSlotForeign(vm, 0);
    ASSERT_TYPE(vm, 1, STRING, "data");
    int size;
    const char *data = wrenGetSlotBytes(vm, 1, &size);
    bg_font *loaded_font = bg_load_font_mem((void*)data, size);
    if (!loaded_font) {
        ABORT_VM(vm, "Failed to load font");
        return;
    }

    *font = loaded_font;
}

void api_font_measure(WrenVM *vm) {
    bg_font **font = (bg_font**)wrenGetSlotForeign(vm, 0);
    ASSERT_TYPE(vm, 1, STRING, "text");
    const char *text = wrenGetSlotString(vm, 1);
    wrenSetSlotDouble(vm, 0, bg_text_width(*font, text));
}

//--------------------
// Audio
//--------------------

void api_source_allocate(WrenVM *vm) {
    wrenEnsureSlots(vm, 1);
    wrenSetSlotNewForeign(vm, 0, 0, sizeof(ba_source*));
}

void api_source_finalize(void *data) {
    ba_source **source = (ba_source**)data;
    ba_destroy_source(*source);
}

void api_source_new(WrenVM *vm) {
    ba_source **source = (ba_source**)wrenGetSlotForeign(vm, 0);
    ASSERT_TYPE(vm, 1, STRING, "filename");
    const char *filename = wrenGetSlotString(vm, 1);
    ba_source *loaded_source = ba_load_source_file(filename);
    if (!loaded_source) {
        ABORT_VM(vm, "Failed to load source");
        return;
    }

    *source = loaded_source;
}

typedef struct {
    blink_sound sound;
    int idx;
} sound_stream;

#define SOUND_PROCESS_LOOP(X) \
    while (n--) {           \
        X                   \
        dst += 2;           \
        s->idx++;           \
    }

static void sound_stream_handler(ba_event *e) {
    sound_stream *s = e->user_data;

    switch (e->type)
    {
    case BA_EVENT_DESTROY:
        free(s->sound.data);
        free(s);
        break;

    case BA_EVENT_SAMPLES:
        int16_t *dst = e->buffer;
        int len = e->length / 2;
fill:
        int n = MIN(len, s->sound.length - s->idx);
        len -= n;

        if (s->sound.bitdepth == 16 && s->sound.channels == 1) {
            SOUND_PROCESS_LOOP({
                dst[0] = dst[1] = ((int16_t*)s->sound.data)[s->idx];
            });
        } else if (s->sound.bitdepth == 16 && s->sound.channels == 2) {
            SOUND_PROCESS_LOOP({
                int x = s->idx * 2;
                dst[0] = ((int16_t*)s->sound.data)[x];
                dst[1] = ((int16_t*)s->sound.data)[x + 1];
            });
        } else if (s->sound.bitdepth == 8 && s->sound.channels == 1) {
            SOUND_PROCESS_LOOP({
                dst[0] = dst[1] = (((uint8_t*)s->sound.data)[s->idx] - 128) << 8;
            });
        } else if (s->sound.bitdepth == 8 && s->sound.channels == 2) {
            SOUND_PROCESS_LOOP({
                int x = s->idx * 2;
                dst[0] = (((uint8_t*)s->sound.data)[x] - 128) << 8;
                dst[1] = (((uint8_t*)s->sound.data)[x + 1] - 128) << 8;
            });
        }

        if (len > 0) {
            s->idx = 0;
            goto fill;
        }
        break;

    case BA_EVENT_REWIND:
        s->idx = 0;
        break;
    }
}

void api_source_new_from_sound(WrenVM *vm) {
    ba_source **source = (ba_source**)wrenGetSlotForeign(vm, 0);
    ASSERT_TYPE(vm, 1, FOREIGN, "sound");
    blink_sound *sound = (blink_sound*)wrenGetSlotForeign(vm, 1);

    ba_source_info info;

    sound_stream *stream = calloc(1, sizeof(sound_stream));
    if (!stream) {
        ABORT_VM(vm, "Failed to allocate stream");
        return;
    }

    blink_sound copy;
    copy.data = malloc(sound->length * (sound->bitdepth / 8) * sound->channels);
    if (!copy.data) {
        ABORT_VM(vm, "Failed to allocate sound data");
        return;
    }

    memcpy(copy.data, sound->data, sound->length * (sound->bitdepth / 8) * sound->channels);
    copy.bitdepth = sound->bitdepth;
    copy.samplerate = sound->samplerate;
    copy.channels = sound->channels;
    copy.length = sound->length;

    stream->sound = copy;
    stream->idx = 0;

    info.user_data = stream;
    info.handler = sound_stream_handler;
    info.samplerate = sound->samplerate;
    info.length = sound->length;

    *source = ba_new_source(&info);
}

void api_source_play(WrenVM *vm) {
    ba_source **source = (ba_source**)wrenGetSlotForeign(vm, 0);
    ba_play(*source);
}

void api_source_pause(WrenVM *vm) {
    ba_source **source = (ba_source**)wrenGetSlotForeign(vm, 0);
    ba_pause(*source);
}

void api_source_stop(WrenVM *vm) {
    ba_source **source = (ba_source**)wrenGetSlotForeign(vm, 0);
    ba_stop(*source);
}

void api_source_get_length(WrenVM *vm) {
    ba_source **source = (ba_source**)wrenGetSlotForeign(vm, 0);
    wrenSetSlotDouble(vm, 0, ba_get_length(*source));
}

void api_source_get_position(WrenVM *vm) {
    ba_source **source = (ba_source**)wrenGetSlotForeign(vm, 0);
    wrenSetSlotDouble(vm, 0, ba_get_position(*source));
}

void api_source_get_state(WrenVM *vm) {
    ba_source **source = (ba_source**)wrenGetSlotForeign(vm, 0);
    int state = ba_get_state(*source);

    switch (state) {
    case BA_STATE_STOPPED:
        wrenSetSlotString(vm, 0, "stopped");
        break;
    case BA_STATE_PLAYING:
        wrenSetSlotString(vm, 0, "playing");
        break;
    case BA_STATE_PAUSED:
        wrenSetSlotString(vm, 0, "paused");
        break;
    }
}

void api_source_set_gain(WrenVM *vm) {
    ba_source **source = (ba_source**)wrenGetSlotForeign(vm, 0);
    ASSERT_TYPE(vm, 1, NUM, "gain");
    float gain = (float)wrenGetSlotDouble(vm, 1);
    ba_set_gain(*source, gain);
}

void api_source_set_pan(WrenVM *vm) {
    ba_source **source = (ba_source**)wrenGetSlotForeign(vm, 0);
    ASSERT_TYPE(vm, 1, NUM, "pan");
    float pan = (float)wrenGetSlotDouble(vm, 1);
    ba_set_pan(*source, pan);
}

void api_source_set_pitch(WrenVM *vm) {
    ba_source **source = (ba_source**)wrenGetSlotForeign(vm, 0);
    ASSERT_TYPE(vm, 1, NUM, "pitch");
    float pitch = (float)wrenGetSlotDouble(vm, 1);
    ba_set_pitch(*source, pitch);
}

void api_source_set_loop(WrenVM *vm) {
    ba_source **source = (ba_source**)wrenGetSlotForeign(vm, 0);
    ASSERT_TYPE(vm, 1, BOOL, "loop");
    bool loop = wrenGetSlotBool(vm, 1);
    ba_set_loop(*source, loop);
}

void api_sound_allocate(WrenVM *vm) {
    wrenEnsureSlots(vm, 1);
    wrenSetSlotNewForeign(vm, 0, 0, sizeof(blink_sound));
}

void api_sound_finalize(void *data) {
    blink_sound *sound = (blink_sound*)data;
    free(sound->data);
}

void api_sound_new(WrenVM *vm) {
    blink_sound *sound = (blink_sound*)wrenGetSlotForeign(vm, 0);
    ASSERT_TYPE(vm, 1, NUM, "samples");
    ASSERT_TYPE(vm, 2, NUM, "sampleRate");
    ASSERT_TYPE(vm, 3, NUM, "bitDepth");
    ASSERT_TYPE(vm, 4, NUM, "channels");

    int samples = (int)wrenGetSlotDouble(vm, 1);
    int sample_rate = (int)wrenGetSlotDouble(vm, 2);
    int bit_depth = (int)wrenGetSlotDouble(vm, 3);
    int channels = (int)wrenGetSlotDouble(vm, 4);

    if (samples <= 0 || sample_rate <= 0) {
        ABORT_VM(vm, "Invalid sample data");
        return;
    }

    if (bit_depth != 8 && bit_depth != 16) {
        ABORT_VM(vm, "Invalid bit depth");
        return;
    }

    if (channels != 1 && channels != 2) {
        ABORT_VM(vm, "Invalid number of channels");
        return;
    }

    sound->data = malloc(samples * (bit_depth / 8) * channels);
    if (!sound->data) {
        ABORT_VM(vm, "Failed to allocate sound data");
        return;
    }

    sound->bitdepth = bit_depth;
    sound->samplerate = sample_rate;
    sound->channels = channels;
    sound->length = samples;
}

static char *find_subchunk(char *data, int len, char *id, int *size) {
    int idlen = strlen(id);
    char *p = data + 12;
next:
    *size = *((uint32_t*)(p + 4));
    if (memcmp(p, id, idlen)) {
        p += 8 + *size;
        if (p > data + len) return NULL;
        goto next;
    }
    return p + 8;
}

static bool read_wav(blink_sound *sound, void *data, int size) {
    char *p = data;
    memset(sound, 0, sizeof(blink_sound));

    if (memcmp(p, "RIFF", 4) || memcmp(p + 8, "WAVE", 4))
        return false;

    int sz;
    p = find_subchunk(data, size, "fmt", &sz);
    if (!p)
        return false;

    int format = *((uint16_t*)(p));
    int channels = *((uint16_t*)(p + 2));
    int samplerate = *((uint32_t*)(p + 4));
    int bitdepth = *((uint16_t*)(p + 14));
    if (format != 1)
        return false;

    if (channels == 0 || samplerate == 0 || bitdepth == 0)
        return false;

    p = find_subchunk(data, size, "data", &sz);
    if (!p)
        return false;

    sound->data = malloc(sz);
    if (!sound->data)
        return false;

    memcpy(sound->data, p, sz);

    sound->samplerate = samplerate;
    sound->channels = channels;
    sound->length = (sz / (bitdepth / 8)) / channels;
    sound->bitdepth = bitdepth;

    return true;
}

void api_sound_new_filename(WrenVM *vm) {
    blink_sound *sound = (blink_sound*)wrenGetSlotForeign(vm, 0);
    ASSERT_TYPE(vm, 1, STRING, "filename");
    const char *filename = wrenGetSlotString(vm, 1);
    int size;
    void *data = cri_read_file(filename, &size);
    if (!data) {
        ABORT_VM(vm, "Failed to read file");
        return;
    }

    if (!read_wav(sound, data, size)) {
        free(data);
        ABORT_VM(vm, "Failed to read wav data");
        return;
    }

    free(data);
}

void api_sound_new_from_memory(WrenVM *vm) {
    blink_sound *sound = (blink_sound*)wrenGetSlotForeign(vm, 0);
    ASSERT_TYPE(vm, 1, STRING, "data");
    int size;
    const char *data = wrenGetSlotBytes(vm, 1, &size);
    if (!read_wav(sound, (void*)data, size)) {
        ABORT_VM(vm, "Failed to read wav data");
        return;
    }
}

void api_sound_get_sample(WrenVM *vm) {
    blink_sound *sound = (blink_sound*)wrenGetSlotForeign(vm, 0);
    ASSERT_TYPE(vm, 1, NUM, "index");
    int index = (int)wrenGetSlotDouble(vm, 1);

    if (index < 0 || index >= sound->length * sound->channels) {
        ABORT_VM(vm, "Invalid sample index");
        return;
    }

    float value;

    if (sound->bitdepth == 8) {
        uint8_t *data = (uint8_t*)sound->data;
        value = (data[index] - 128) / 128.0f;
    } else if (sound->bitdepth == 16) {
        int16_t *data = (int16_t*)sound->data;
        value = data[index] / 32767.0f;
    }

    wrenSetSlotDouble(vm, 0, value);
}

void api_sound_set_sample(WrenVM *vm) {
    blink_sound *sound = (blink_sound*)wrenGetSlotForeign(vm, 0);
    ASSERT_TYPE(vm, 1, NUM, "index");
    ASSERT_TYPE(vm, 2, NUM, "value");
    int index = (int)wrenGetSlotDouble(vm, 1);
    float value = (float)wrenGetSlotDouble(vm, 2);

    if (index < 0 || index >= sound->length * sound->channels) {
        ABORT_VM(vm, "Invalid sample index");
        return;
    }

    if (sound->bitdepth == 8) {
        uint8_t *data = (uint8_t*)sound->data;
        data[index] = (uint8_t)(value * 128 + 128);
    } else if (sound->bitdepth == 16) {
        int16_t *data = (int16_t*)sound->data;
        data[index] = (int16_t)(value * 32767);
    }
}

static void *write_wav(blink_sound *sound, int *size) {
    int data_size = sound->length * sound->channels * sound->bitdepth / 8;
    int wav_size = 44 + data_size;

    uint8_t *wav_data = (uint8_t*)malloc(wav_size);
    if (!wav_data)
        return NULL;

    memcpy(wav_data, "RIFF", 4);
    *(uint32_t*)(wav_data + 4) = data_size + 36;
    memcpy(wav_data + 8, "WAVE", 4);

    memcpy(wav_data + 12, "fmt ", 4);
    *(uint32_t*)(wav_data + 16) = 16;
    *(uint16_t*)(wav_data + 20) = 1;
    *(uint16_t*)(wav_data + 22) = sound->channels;
    *(uint32_t*)(wav_data + 24) = sound->samplerate;
    *(uint32_t*)(wav_data + 28) = sound->samplerate * sound->channels * sound->bitdepth / 8;
    *(uint16_t*)(wav_data + 32) = sound->channels * sound->bitdepth / 8;
    *(uint16_t*)(wav_data + 34) = sound->bitdepth;

    memcpy(wav_data + 36, "data", 4);
    *(uint32_t *)(wav_data + 40) = data_size;
    memcpy(wav_data + 44, sound->data, data_size);

    *size = wav_size;

    return wav_data;
}

void api_sound_save(WrenVM *vm) {
    blink_sound *sound = (blink_sound*)wrenGetSlotForeign(vm, 0);
    ASSERT_TYPE(vm, 1, STRING, "filename");
    const char *filename = wrenGetSlotString(vm, 1);
    int size;
    void *data = write_wav(sound, &size);
    if (!data) {
        ABORT_VM(vm, "Failed to write wav data");
        return;
    }

    wrenSetSlotBool(vm, 0, cri_write_file(filename, data, size));
    free(data);
}

void api_sound_save_to_memory(WrenVM *vm) {
    blink_sound *sound = (blink_sound*)wrenGetSlotForeign(vm, 0);
    int size;
    void *data = write_wav(sound, &size);
    if (data) {
        wrenSetSlotBytes(vm, 0, data, size);
    } else {
        wrenSetSlotNull(vm, 0);
    }

    free(data);
}

void api_sound_get_bit_depth(WrenVM *vm) {
    blink_sound *sound = (blink_sound*)wrenGetSlotForeign(vm, 0);
    wrenSetSlotDouble(vm, 0, sound->bitdepth);
}

void api_sound_get_sample_rate(WrenVM *vm) {
    blink_sound *sound = (blink_sound*)wrenGetSlotForeign(vm, 0);
    wrenSetSlotDouble(vm, 0, sound->samplerate);
}

void api_sound_get_channels(WrenVM *vm) {
    blink_sound *sound = (blink_sound*)wrenGetSlotForeign(vm, 0);
    wrenSetSlotDouble(vm, 0, sound->channels);
}

void api_sound_get_length(WrenVM *vm) {
    blink_sound *sound = (blink_sound*)wrenGetSlotForeign(vm, 0);
    wrenSetSlotDouble(vm, 0, sound->length);
}

//--------------------
// Input
//--------------------

void api_keyboard_down(WrenVM *vm) {
    blink_state *state = (blink_state*)wrenGetUserData(vm);
    ASSERT_TYPE(vm, 1, STRING, "key");
    const char *key_string = wrenGetSlotString(vm, 1);
    int *key = map_get(&state->keymap, key_string);
    if (!key) {
        ABORT_VM(vm, "Invalid key");
        return;
    }

    wrenSetSlotBool(vm, 0, cri_get_keyboard_state(state->window)[*key]);
}

void api_keyboard_pressed(WrenVM *vm) {
    blink_state *state = (blink_state*)wrenGetUserData(vm);
    ASSERT_TYPE(vm, 1, STRING, "key");
    const char *key_string = wrenGetSlotString(vm, 1);
    int *key = map_get(&state->keymap, key_string);
    if (!key) {
        ABORT_VM(vm, "Invalid key");
        return;
    }

    wrenSetSlotBool(vm, 0, cri_get_keyboard_state(state->window)[*key] && !state->prev_keyboard_state[*key]);
}

void api_mouse_down(WrenVM *vm) {
    blink_state *state = (blink_state*)wrenGetUserData(vm);
    ASSERT_TYPE(vm, 1, NUM, "button");
    int button = (int)wrenGetSlotDouble(vm, 1);
    wrenSetSlotBool(vm, 0, cri_get_mouse_state(state->window)[button]);
}

void api_mouse_pressed(WrenVM *vm) {
    blink_state *state = (blink_state*)wrenGetUserData(vm);
    ASSERT_TYPE(vm, 1, NUM, "button");
    int button = (int)wrenGetSlotDouble(vm, 1);
    wrenSetSlotBool(vm, 0, cri_get_mouse_state(state->window)[button] && !state->prev_mouse_state[button]);
}

void api_mouse_get_x(WrenVM *vm) {
    blink_state *state = (blink_state*)wrenGetUserData(vm);
    int x = cri_get_mouse_x(state->window);
    int virtual_x = (x - (cri_get_window_width(state->window) - state->width * state->scale) / 2) / state->scale;
    wrenSetSlotDouble(vm, 0, virtual_x);
}

void api_mouse_get_y(WrenVM *vm) {
    blink_state *state = (blink_state*)wrenGetUserData(vm);
    int y = cri_get_mouse_y(state->window);
    int virtual_y = (y - (cri_get_window_height(state->window) - state->height * state->scale) / 2) / state->scale;
    wrenSetSlotDouble(vm, 0, virtual_y);
}

void api_mouse_get_scroll_x(WrenVM *vm) {
    blink_state *state = (blink_state*)wrenGetUserData(vm);
    wrenSetSlotDouble(vm, 0, cri_get_mouse_scroll_x(state->window));
}

void api_mouse_get_scroll_y(WrenVM *vm) {
    blink_state *state = (blink_state*)wrenGetUserData(vm);
    wrenSetSlotDouble(vm, 0, cri_get_mouse_scroll_y(state->window));
}

//--------------------
// System
//--------------------

void api_window_close(WrenVM *vm) {
    blink_state *state = (blink_state*)wrenGetUserData(vm);
    cri_close(state->window);
}

void api_window_get_active(WrenVM *vm) {
    blink_state *state = (blink_state*)wrenGetUserData(vm);
    wrenEnsureSlots(vm, 1);
    wrenSetSlotBool(vm, 0, cri_is_window_active(state->window));
}

void api_window_get_width(WrenVM *vm) {
    blink_state *state = (blink_state*)wrenGetUserData(vm);
    wrenEnsureSlots(vm, 1);
    wrenSetSlotDouble(vm, 0, cri_get_window_width(state->window));
}

void api_window_get_height(WrenVM *vm) {
    blink_state *state = (blink_state*)wrenGetUserData(vm);
    wrenEnsureSlots(vm, 1);
    wrenSetSlotDouble(vm, 0, cri_get_window_height(state->window));
}

void api_os_get_name(WrenVM *vm) {
    wrenEnsureSlots(vm, 1);

#ifdef _WIN32
    wrenSetSlotString(vm, 0, "windows");
#else
    wrenSetSlotString(vm, 0, "linux");
#endif
}

void api_os_get_blink_version(WrenVM *vm) {
    wrenEnsureSlots(vm, 1);
    wrenSetSlotString(vm, 0, BLINK_VERSION);
}

void api_os_get_args(WrenVM *vm) {
    blink_state *state = (blink_state*)wrenGetUserData(vm);
    wrenEnsureSlots(vm, 2);
    wrenSetSlotNewList(vm, 0);

    for (int i = 0; i < state->argc; i++) {
        wrenSetSlotString(vm, 1, state->argv[i]);
        wrenInsertInList(vm, 0, i, 1);
    }
}

void api_os_get_clipboard(WrenVM *vm) {
    const char *clipboard = cri_get_clipboard_text();
    if (clipboard) {
        wrenEnsureSlots(vm, 1);
        wrenSetSlotString(vm, 0, clipboard);
        free((void*)clipboard);
    }
}

void api_os_set_clipboard(WrenVM *vm) {
    ASSERT_TYPE(vm, 1, STRING, "text");
    const char *text = wrenGetSlotString(vm, 1);
    cri_set_clipboard_text(text);
}

void api_directory_exists(WrenVM *vm) {
    ASSERT_TYPE(vm, 1, STRING, "path");
    const char *path = wrenGetSlotString(vm, 1);
    wrenSetSlotBool(vm, 0, cri_dir_exists(path));
}

void api_directory_list(WrenVM *vm) {
    ASSERT_TYPE(vm, 1, STRING, "path");
    const char *path = wrenGetSlotString(vm, 1);
    wrenEnsureSlots(vm, 2);
    wrenSetSlotNewList(vm, 0);

    int count;
    char **files = cri_get_dir_files(path, &count);

    if (files) {
        for (int i = 0; i < count; i++) {
            wrenSetSlotString(vm, 1, files[i]);
            wrenInsertInList(vm, 0, -1, 1);
        }
        cri_free_dir_files(files, count);
    }
}

void api_file_exists(WrenVM *vm) {
    ASSERT_TYPE(vm, 1, STRING, "path");
    const char *path = wrenGetSlotString(vm, 1);
    wrenSetSlotBool(vm, 0, cri_file_exists(path));
}

void api_file_size(WrenVM *vm) {
    ASSERT_TYPE(vm, 1, STRING, "path");
    const char *path = wrenGetSlotString(vm, 1);
    wrenSetSlotDouble(vm, 0, cri_get_file_size(path));
}

void api_file_mod_time(WrenVM *vm) {
    ASSERT_TYPE(vm, 1, STRING, "path");
    const char *path = wrenGetSlotString(vm, 1);
    wrenSetSlotDouble(vm, 0, cri_get_file_mod_time(path));
}

void api_file_read(WrenVM *vm) {
    ASSERT_TYPE(vm, 1, STRING, "path");
    const char *path = wrenGetSlotString(vm, 1);
    int size;
    void *data = cri_read_file(path, &size);
    if (data) {
        wrenSetSlotBytes(vm, 0, data, size);
    } else {
        wrenSetSlotNull(vm, 0);
    }
}

void api_file_write(WrenVM *vm) {
    ASSERT_TYPE(vm, 1, STRING, "path");
    ASSERT_TYPE(vm, 2, STRING, "data");
    const char *path = wrenGetSlotString(vm, 1);
    int size;
    const char *data = wrenGetSlotBytes(vm, 2, &size);
    wrenSetSlotBool(vm, 0, cri_write_file(path, data, size));
}

void api_request_allocate(WrenVM *vm) {
    wrenEnsureSlots(vm, 1);
    wrenSetSlotNewForeign(vm, 0, 0, sizeof(blink_request));
}

void api_request_finalize(void *data) {
    blink_request *request = (blink_request*)data;
    naettFree(request->req);
    naettClose(request->res);
}

void api_request_new(WrenVM *vm) {
    blink_request *request = (blink_request*)wrenGetSlotForeign(vm, 0);
    ASSERT_TYPE(vm, 1, STRING, "url");
    const char *url = wrenGetSlotString(vm, 1);
    request->req = naettRequest(url, naettMethod("GET"), naettHeader("accept", "*/*"));
}

void api_request_make(WrenVM *vm) {
    blink_request *request = (blink_request*)wrenGetSlotForeign(vm, 0);
    request->res = naettMake(request->req);
}

void api_request_get_complete(WrenVM *vm) {
    blink_request *request = (blink_request*)wrenGetSlotForeign(vm, 0);
    wrenSetSlotBool(vm, 0, naettComplete(request->res));
}

void api_request_get_status(WrenVM *vm) {
    blink_request *request = (blink_request*)wrenGetSlotForeign(vm, 0);
    wrenSetSlotDouble(vm, 0, naettGetStatus(request->res));
}

void api_request_get_body(WrenVM *vm) {
    blink_request *request = (blink_request*)wrenGetSlotForeign(vm, 0);
    int size;
    const char *data = naettGetBody(request->res, &size);
    wrenSetSlotBytes(vm, 0, data, size);
}
