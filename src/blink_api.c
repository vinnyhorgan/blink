#include "blink_api.h"

#define blink_abort_vm(vm, error) do {   \
        wrenSetSlotString(vm, 0, error); \
        wrenAbortFiber(vm, 0);           \
    } while (false);

#define blink_assert_type(vm, slot, type, field_name)                        \
    if (wrenGetSlotType(vm, slot) != WREN_TYPE_##type) {                     \
        blink_abort_vm(vm, "Expected " #field_name " to be of type " #type); \
        return;                                                              \
    }

#define blink_clamp(x, min, max) ((x) < (min) ? (min) : ((x) > (max) ? (max) : (x)))

//--------------------
// Graphics
//--------------------

void api_graphics_clip(WrenVM *vm) {
    blink_state *state = (blink_state*)wrenGetUserData(vm);
    blink_assert_type(vm, 1, NUM, "x");
    blink_assert_type(vm, 2, NUM, "y");
    blink_assert_type(vm, 3, NUM, "w");
    blink_assert_type(vm, 4, NUM, "h");
    int x = (int)wrenGetSlotDouble(vm, 1);
    int y = (int)wrenGetSlotDouble(vm, 2);
    int w = (int)wrenGetSlotDouble(vm, 3);
    int h = (int)wrenGetSlotDouble(vm, 4);
    blink_set_clip(state->screen, blink_new_rect(x, y, w, h));
}

void api_graphics_clear(WrenVM *vm) {
    blink_state *state = (blink_state*)wrenGetUserData(vm);
    blink_assert_type(vm, 1, FOREIGN, "color");
    blink_color *color = (blink_color*)wrenGetSlotForeign(vm, 1);
    blink_clear(state->screen, *color);
}

void api_graphics_get(WrenVM *vm) {
    blink_state *state = (blink_state*)wrenGetUserData(vm);
    blink_assert_type(vm, 1, NUM, "x");
    blink_assert_type(vm, 2, NUM, "y");
    int x = (int)wrenGetSlotDouble(vm, 1);
    int y = (int)wrenGetSlotDouble(vm, 2);
    wrenSetSlotHandle(vm, 1, state->color_class);
    blink_color *color = wrenSetSlotNewForeign(vm, 0, 1, sizeof(blink_color));
    *color = blink_get_pixel(state->screen, x, y);
}

void api_graphics_set(WrenVM *vm) {
    blink_state *state = (blink_state*)wrenGetUserData(vm);
    blink_assert_type(vm, 1, NUM, "x");
    blink_assert_type(vm, 2, NUM, "y");
    blink_assert_type(vm, 3, FOREIGN, "color");
    int x = (int)wrenGetSlotDouble(vm, 1);
    int y = (int)wrenGetSlotDouble(vm, 2);
    blink_color *color = (blink_color*)wrenGetSlotForeign(vm, 3);
    blink_set_pixel(state->screen, x, y, *color);
}

void api_graphics_line(WrenVM *vm) {
    blink_state *state = (blink_state*)wrenGetUserData(vm);
    blink_assert_type(vm, 1, NUM, "x0");
    blink_assert_type(vm, 2, NUM, "y0");
    blink_assert_type(vm, 3, NUM, "x1");
    blink_assert_type(vm, 4, NUM, "y1");
    blink_assert_type(vm, 5, FOREIGN, "color");
    int x0 = (int)wrenGetSlotDouble(vm, 1);
    int y0 = (int)wrenGetSlotDouble(vm, 2);
    int x1 = (int)wrenGetSlotDouble(vm, 3);
    int y1 = (int)wrenGetSlotDouble(vm, 4);
    blink_color *color = (blink_color*)wrenGetSlotForeign(vm, 5);
    blink_line(state->screen, x0, y0, x1, y1, *color);
}

void api_graphics_fill(WrenVM *vm) {
    blink_state *state = (blink_state*)wrenGetUserData(vm);
    blink_assert_type(vm, 1, NUM, "x");
    blink_assert_type(vm, 2, NUM, "y");
    blink_assert_type(vm, 3, NUM, "w");
    blink_assert_type(vm, 4, NUM, "h");
    blink_assert_type(vm, 5, FOREIGN, "color");
    int x = (int)wrenGetSlotDouble(vm, 1);
    int y = (int)wrenGetSlotDouble(vm, 2);
    int w = (int)wrenGetSlotDouble(vm, 3);
    int h = (int)wrenGetSlotDouble(vm, 4);
    blink_color *color = (blink_color*)wrenGetSlotForeign(vm, 5);
    blink_fill(state->screen, x, y, w, h, *color);
}

void api_graphics_rectangle(WrenVM *vm) {
    blink_state *state = (blink_state*)wrenGetUserData(vm);
    blink_assert_type(vm, 1, NUM, "x");
    blink_assert_type(vm, 2, NUM, "y");
    blink_assert_type(vm, 3, NUM, "w");
    blink_assert_type(vm, 4, NUM, "h");
    blink_assert_type(vm, 5, FOREIGN, "color");
    int x = (int)wrenGetSlotDouble(vm, 1);
    int y = (int)wrenGetSlotDouble(vm, 2);
    int w = (int)wrenGetSlotDouble(vm, 3);
    int h = (int)wrenGetSlotDouble(vm, 4);
    blink_color *color = (blink_color*)wrenGetSlotForeign(vm, 5);
    blink_rectangle(state->screen, x, y, w, h, *color);
}

void api_graphics_fill_rectangle(WrenVM *vm) {
    blink_state *state = (blink_state*)wrenGetUserData(vm);
    blink_assert_type(vm, 1, NUM, "x");
    blink_assert_type(vm, 2, NUM, "y");
    blink_assert_type(vm, 3, NUM, "w");
    blink_assert_type(vm, 4, NUM, "h");
    blink_assert_type(vm, 5, FOREIGN, "color");
    int x = (int)wrenGetSlotDouble(vm, 1);
    int y = (int)wrenGetSlotDouble(vm, 2);
    int w = (int)wrenGetSlotDouble(vm, 3);
    int h = (int)wrenGetSlotDouble(vm, 4);
    blink_color *color = (blink_color*)wrenGetSlotForeign(vm, 5);
    blink_fill_rectangle(state->screen, x, y, w, h, *color);
}

void api_graphics_circle(WrenVM *vm) {
    blink_state *state = (blink_state*)wrenGetUserData(vm);
    blink_assert_type(vm, 1, NUM, "x");
    blink_assert_type(vm, 2, NUM, "y");
    blink_assert_type(vm, 3, NUM, "r");
    blink_assert_type(vm, 4, FOREIGN, "color");
    int x = (int)wrenGetSlotDouble(vm, 1);
    int y = (int)wrenGetSlotDouble(vm, 2);
    int r = (int)wrenGetSlotDouble(vm, 3);
    blink_color *color = (blink_color*)wrenGetSlotForeign(vm, 4);
    blink_circle(state->screen, x, y, r, *color);
}

void api_graphics_fill_circle(WrenVM *vm) {
    blink_state *state = (blink_state*)wrenGetUserData(vm);
    blink_assert_type(vm, 1, NUM, "x");
    blink_assert_type(vm, 2, NUM, "y");
    blink_assert_type(vm, 3, NUM, "r");
    blink_assert_type(vm, 4, FOREIGN, "color");
    int x = (int)wrenGetSlotDouble(vm, 1);
    int y = (int)wrenGetSlotDouble(vm, 2);
    int r = (int)wrenGetSlotDouble(vm, 3);
    blink_color *color = (blink_color*)wrenGetSlotForeign(vm, 4);
    blink_fill_circle(state->screen, x, y, r, *color);
}

void api_graphics_blit(WrenVM *vm) {
    blink_state *state = (blink_state*)wrenGetUserData(vm);
    blink_assert_type(vm, 1, FOREIGN, "image");
    blink_assert_type(vm, 2, NUM, "dx");
    blink_assert_type(vm, 3, NUM, "dy");
    blink_assert_type(vm, 4, NUM, "sx");
    blink_assert_type(vm, 5, NUM, "sy");
    blink_assert_type(vm, 6, NUM, "w");
    blink_assert_type(vm, 7, NUM, "h");
    blink_image **image = (blink_image**)wrenGetSlotForeign(vm, 1);
    int dx = (int)wrenGetSlotDouble(vm, 2);
    int dy = (int)wrenGetSlotDouble(vm, 3);
    int sx = (int)wrenGetSlotDouble(vm, 4);
    int sy = (int)wrenGetSlotDouble(vm, 5);
    int w = (int)wrenGetSlotDouble(vm, 6);
    int h = (int)wrenGetSlotDouble(vm, 7);
    blink_blit(state->screen, *image, dx, dy, sx, sy, w, h);
}

void api_graphics_blit_alpha(WrenVM *vm) {
    blink_state *state = (blink_state*)wrenGetUserData(vm);
    blink_assert_type(vm, 1, FOREIGN, "image");
    blink_assert_type(vm, 2, NUM, "dx");
    blink_assert_type(vm, 3, NUM, "dy");
    blink_assert_type(vm, 4, NUM, "sx");
    blink_assert_type(vm, 5, NUM, "sy");
    blink_assert_type(vm, 6, NUM, "w");
    blink_assert_type(vm, 7, NUM, "h");
    blink_assert_type(vm, 8, NUM, "alpha");
    blink_image **image = (blink_image**)wrenGetSlotForeign(vm, 1);
    int dx = (int)wrenGetSlotDouble(vm, 2);
    int dy = (int)wrenGetSlotDouble(vm, 3);
    int sx = (int)wrenGetSlotDouble(vm, 4);
    int sy = (int)wrenGetSlotDouble(vm, 5);
    int w = (int)wrenGetSlotDouble(vm, 6);
    int h = (int)wrenGetSlotDouble(vm, 7);
    float alpha = (float)wrenGetSlotDouble(vm, 8);
    blink_blit_alpha(state->screen, *image, dx, dy, sx, sy, w, h, alpha);
}

void api_graphics_blit_tint(WrenVM *vm) {
    blink_state *state = (blink_state*)wrenGetUserData(vm);
    blink_assert_type(vm, 1, FOREIGN, "image");
    blink_assert_type(vm, 2, NUM, "dx");
    blink_assert_type(vm, 3, NUM, "dy");
    blink_assert_type(vm, 4, NUM, "sx");
    blink_assert_type(vm, 5, NUM, "sy");
    blink_assert_type(vm, 6, NUM, "w");
    blink_assert_type(vm, 7, NUM, "h");
    blink_assert_type(vm, 8, FOREIGN, "tint");
    blink_image **image = (blink_image**)wrenGetSlotForeign(vm, 1);
    int dx = (int)wrenGetSlotDouble(vm, 2);
    int dy = (int)wrenGetSlotDouble(vm, 3);
    int sx = (int)wrenGetSlotDouble(vm, 4);
    int sy = (int)wrenGetSlotDouble(vm, 5);
    int w = (int)wrenGetSlotDouble(vm, 6);
    int h = (int)wrenGetSlotDouble(vm, 7);
    blink_color *tint = (blink_color*)wrenGetSlotForeign(vm, 8);
    blink_blit_tint(state->screen, *image, dx, dy, sx, sy, w, h, *tint);
}

void api_graphics_print(WrenVM *vm) {
    blink_state *state = (blink_state*)wrenGetUserData(vm);
    blink_assert_type(vm, 1, STRING, "text");
    blink_assert_type(vm, 2, NUM, "x");
    blink_assert_type(vm, 3, NUM, "y");
    blink_assert_type(vm, 4, FOREIGN, "color");
    const char *text = (char*)wrenGetSlotString(vm, 1);
    int x = (int)wrenGetSlotDouble(vm, 2);
    int y = (int)wrenGetSlotDouble(vm, 3);
    blink_color *color = (blink_color*)wrenGetSlotForeign(vm, 4);
    blink_draw_text(state->screen, state->font, text, x, y, *color);
}

void api_graphics_screenshot(WrenVM *vm) {
    blink_state *state = (blink_state*)wrenGetUserData(vm);
    wrenSetSlotHandle(vm, 1, state->image_class);
    blink_image **image = wrenSetSlotNewForeign(vm, 0, 1, sizeof(blink_image*));
    *image = blink_create_image(state->width, state->height);
    memcpy((*image)->pixels, state->screen->pixels, state->width * state->height * 4);
}

void api_graphics_measure(WrenVM *vm) {
    blink_state *state = (blink_state*)wrenGetUserData(vm);
    blink_assert_type(vm, 1, STRING, "text");
    const char *text = wrenGetSlotString(vm, 1);
    wrenSetSlotDouble(vm, 0, blink_text_width(state->font, text));
}

void api_graphics_set_clear_color(WrenVM *vm) {
    blink_state *state = (blink_state*)wrenGetUserData(vm);
    blink_assert_type(vm, 1, FOREIGN, "color");
    blink_color *color = (blink_color*)wrenGetSlotForeign(vm, 1);
    state->clear_color = *color;
}

void api_color_allocate(WrenVM *vm) {
    wrenEnsureSlots(vm, 1);
    wrenSetSlotNewForeign(vm, 0, 0, sizeof(blink_color));
}

void api_color_new_rgba(WrenVM *vm) {
    blink_color *color = (blink_color*)wrenGetSlotForeign(vm, 0);
    blink_assert_type(vm, 1, NUM, "r");
    blink_assert_type(vm, 2, NUM, "g");
    blink_assert_type(vm, 3, NUM, "b");
    blink_assert_type(vm, 4, NUM, "a");
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
    blink_color *color = (blink_color*)wrenGetSlotForeign(vm, 0);
    blink_assert_type(vm, 1, NUM, "r");
    blink_assert_type(vm, 2, NUM, "g");
    blink_assert_type(vm, 3, NUM, "b");
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
    blink_assert_type(vm, 1, NUM, "index");
    int index = (int)wrenGetSlotDouble(vm, 1);

    if (index < 0 || index > 3) {
        blink_abort_vm(vm, "Invalid color index");
        return;
    }

    wrenSetSlotDouble(vm, 0, color[index]);
}

void api_color_set_index(WrenVM *vm) {
    uint8_t *color = (uint8_t*)wrenGetSlotForeign(vm, 0);
    blink_assert_type(vm, 1, NUM, "index");
    blink_assert_type(vm, 2, NUM, "value");
    int index = (int)wrenGetSlotDouble(vm, 1);
    int value = (int)wrenGetSlotDouble(vm, 2);

    if (index < 0 || index > 3) {
        blink_abort_vm(vm, "Invalid color index");
        return;
    }

    color[index] = value;
}

void api_image_allocate(WrenVM *vm) {
    wrenEnsureSlots(vm, 1);
    wrenSetSlotNewForeign(vm, 0, 0, sizeof(blink_image*));
}

void api_image_finalize(void *data) {
    blink_image **image = (blink_image**)data;
    blink_destroy_image(*image);
}

void api_image_new_wh(WrenVM *vm) {
    blink_image **image = (blink_image**)wrenGetSlotForeign(vm, 0);
    blink_assert_type(vm, 1, NUM, "w");
    blink_assert_type(vm, 2, NUM, "h");
    int w = (int)wrenGetSlotDouble(vm, 1);
    int h = (int)wrenGetSlotDouble(vm, 2);

    if (w <= 0 || h <= 0) {
        blink_abort_vm(vm, "Invalid image dimensions");
        return;
    }

    *image = blink_create_image(w, h);
}

void api_image_new_filename(WrenVM *vm) {
    blink_image **image = (blink_image**)wrenGetSlotForeign(vm, 0);
    blink_assert_type(vm, 1, STRING, "filename");
    const char *filename = wrenGetSlotString(vm, 1);
    blink_image *loaded_image = blink_load_image_file(filename);
    if (!loaded_image) {
        blink_abort_vm(vm, "Failed to load image");
        return;
    }

    *image = loaded_image;
}

void api_image_clip(WrenVM *vm) {
    blink_image **image = (blink_image**)wrenGetSlotForeign(vm, 0);
    blink_assert_type(vm, 1, NUM, "x");
    blink_assert_type(vm, 2, NUM, "y");
    blink_assert_type(vm, 3, NUM, "w");
    blink_assert_type(vm, 4, NUM, "h");
    int x = (int)wrenGetSlotDouble(vm, 1);
    int y = (int)wrenGetSlotDouble(vm, 2);
    int w = (int)wrenGetSlotDouble(vm, 3);
    int h = (int)wrenGetSlotDouble(vm, 4);
    blink_set_clip(*image, blink_new_rect(x, y, w, h));
}

void api_image_clear(WrenVM *vm) {
    blink_image **image = (blink_image**)wrenGetSlotForeign(vm, 0);
    blink_assert_type(vm, 1, FOREIGN, "color");
    blink_color *color = (blink_color*)wrenGetSlotForeign(vm, 1);
    blink_clear(*image, *color);
}

void api_image_get(WrenVM *vm) {
    blink_image **image = (blink_image**)wrenGetSlotForeign(vm, 0);
    blink_state *state = (blink_state*)wrenGetUserData(vm);
    blink_assert_type(vm, 1, NUM, "x");
    blink_assert_type(vm, 2, NUM, "y");
    int x = (int)wrenGetSlotDouble(vm, 1);
    int y = (int)wrenGetSlotDouble(vm, 2);
    wrenSetSlotHandle(vm, 1, state->color_class);
    blink_color *color = wrenSetSlotNewForeign(vm, 0, 1, sizeof(blink_color));
    *color = blink_get_pixel(*image, x, y);
}

void api_image_set(WrenVM *vm) {
    blink_image **image = (blink_image**)wrenGetSlotForeign(vm, 0);
    blink_assert_type(vm, 1, NUM, "x");
    blink_assert_type(vm, 2, NUM, "y");
    blink_assert_type(vm, 3, FOREIGN, "color");
    int x = (int)wrenGetSlotDouble(vm, 1);
    int y = (int)wrenGetSlotDouble(vm, 2);
    blink_color *color = (blink_color*)wrenGetSlotForeign(vm, 3);
    blink_set_pixel(*image, x, y, *color);
}

void api_image_line(WrenVM *vm) {
    blink_image **image = (blink_image**)wrenGetSlotForeign(vm, 0);
    blink_assert_type(vm, 1, NUM, "x0");
    blink_assert_type(vm, 2, NUM, "y0");
    blink_assert_type(vm, 3, NUM, "x1");
    blink_assert_type(vm, 4, NUM, "y1");
    blink_assert_type(vm, 5, FOREIGN, "color");
    int x0 = (int)wrenGetSlotDouble(vm, 1);
    int y0 = (int)wrenGetSlotDouble(vm, 2);
    int x1 = (int)wrenGetSlotDouble(vm, 3);
    int y1 = (int)wrenGetSlotDouble(vm, 4);
    blink_color *color = (blink_color*)wrenGetSlotForeign(vm, 5);
    blink_line(*image, x0, y0, x1, y1, *color);
}

void api_image_fill(WrenVM *vm) {
    blink_image **image = (blink_image**)wrenGetSlotForeign(vm, 0);
    blink_assert_type(vm, 1, NUM, "x");
    blink_assert_type(vm, 2, NUM, "y");
    blink_assert_type(vm, 3, NUM, "w");
    blink_assert_type(vm, 4, NUM, "h");
    blink_assert_type(vm, 5, FOREIGN, "color");
    int x = (int)wrenGetSlotDouble(vm, 1);
    int y = (int)wrenGetSlotDouble(vm, 2);
    int w = (int)wrenGetSlotDouble(vm, 3);
    int h = (int)wrenGetSlotDouble(vm, 4);
    blink_color *color = (blink_color*)wrenGetSlotForeign(vm, 5);
    blink_fill(*image, x, y, w, h, *color);
}

void api_image_rectangle(WrenVM *vm) {
    blink_image **image = (blink_image**)wrenGetSlotForeign(vm, 0);
    blink_assert_type(vm, 1, NUM, "x");
    blink_assert_type(vm, 2, NUM, "y");
    blink_assert_type(vm, 3, NUM, "w");
    blink_assert_type(vm, 4, NUM, "h");
    blink_assert_type(vm, 5, FOREIGN, "color");
    int x = (int)wrenGetSlotDouble(vm, 1);
    int y = (int)wrenGetSlotDouble(vm, 2);
    int w = (int)wrenGetSlotDouble(vm, 3);
    int h = (int)wrenGetSlotDouble(vm, 4);
    blink_color *color = (blink_color*)wrenGetSlotForeign(vm, 5);
    blink_rectangle(*image, x, y, w, h, *color);
}

void api_image_fill_rectangle(WrenVM *vm) {
    blink_image **image = (blink_image**)wrenGetSlotForeign(vm, 0);
    blink_assert_type(vm, 1, NUM, "x");
    blink_assert_type(vm, 2, NUM, "y");
    blink_assert_type(vm, 3, NUM, "w");
    blink_assert_type(vm, 4, NUM, "h");
    blink_assert_type(vm, 5, FOREIGN, "color");
    int x = (int)wrenGetSlotDouble(vm, 1);
    int y = (int)wrenGetSlotDouble(vm, 2);
    int w = (int)wrenGetSlotDouble(vm, 3);
    int h = (int)wrenGetSlotDouble(vm, 4);
    blink_color *color = (blink_color*)wrenGetSlotForeign(vm, 5);
    blink_fill_rectangle(*image, x, y, w, h, *color);
}

void api_image_circle(WrenVM *vm) {
    blink_image **image = (blink_image**)wrenGetSlotForeign(vm, 0);
    blink_assert_type(vm, 1, NUM, "x");
    blink_assert_type(vm, 2, NUM, "y");
    blink_assert_type(vm, 3, NUM, "r");
    blink_assert_type(vm, 4, FOREIGN, "color");
    int x = (int)wrenGetSlotDouble(vm, 1);
    int y = (int)wrenGetSlotDouble(vm, 2);
    int r = (int)wrenGetSlotDouble(vm, 3);
    blink_color *color = (blink_color*)wrenGetSlotForeign(vm, 4);
    blink_circle(*image, x, y, r, *color);
}

void api_image_fill_circle(WrenVM *vm) {
    blink_image **image = (blink_image**)wrenGetSlotForeign(vm, 0);
    blink_assert_type(vm, 1, NUM, "x");
    blink_assert_type(vm, 2, NUM, "y");
    blink_assert_type(vm, 3, NUM, "r");
    blink_assert_type(vm, 4, FOREIGN, "color");
    int x = (int)wrenGetSlotDouble(vm, 1);
    int y = (int)wrenGetSlotDouble(vm, 2);
    int r = (int)wrenGetSlotDouble(vm, 3);
    blink_color *color = (blink_color*)wrenGetSlotForeign(vm, 4);
    blink_fill_circle(*image, x, y, r, *color);
}

void api_image_blit(WrenVM *vm) {
    blink_image **image = (blink_image**)wrenGetSlotForeign(vm, 0);
    blink_assert_type(vm, 1, FOREIGN, "image");
    blink_assert_type(vm, 2, NUM, "dx");
    blink_assert_type(vm, 3, NUM, "dy");
    blink_assert_type(vm, 4, NUM, "sx");
    blink_assert_type(vm, 5, NUM, "sy");
    blink_assert_type(vm, 6, NUM, "w");
    blink_assert_type(vm, 7, NUM, "h");
    blink_image **src_image = (blink_image**)wrenGetSlotForeign(vm, 1);
    int dx = (int)wrenGetSlotDouble(vm, 2);
    int dy = (int)wrenGetSlotDouble(vm, 3);
    int sx = (int)wrenGetSlotDouble(vm, 4);
    int sy = (int)wrenGetSlotDouble(vm, 5);
    int w = (int)wrenGetSlotDouble(vm, 6);
    int h = (int)wrenGetSlotDouble(vm, 7);
    blink_blit(*image, *src_image, dx, dy, sx, sy, w, h);
}

void api_image_blit_alpha(WrenVM *vm) {
    blink_image **image = (blink_image**)wrenGetSlotForeign(vm, 0);
    blink_assert_type(vm, 1, FOREIGN, "image");
    blink_assert_type(vm, 2, NUM, "dx");
    blink_assert_type(vm, 3, NUM, "dy");
    blink_assert_type(vm, 4, NUM, "sx");
    blink_assert_type(vm, 5, NUM, "sy");
    blink_assert_type(vm, 6, NUM, "w");
    blink_assert_type(vm, 7, NUM, "h");
    blink_assert_type(vm, 8, NUM, "alpha");
    blink_image **src_image = (blink_image**)wrenGetSlotForeign(vm, 1);
    int dx = (int)wrenGetSlotDouble(vm, 2);
    int dy = (int)wrenGetSlotDouble(vm, 3);
    int sx = (int)wrenGetSlotDouble(vm, 4);
    int sy = (int)wrenGetSlotDouble(vm, 5);
    int w = (int)wrenGetSlotDouble(vm, 6);
    int h = (int)wrenGetSlotDouble(vm, 7);
    float alpha = (float)wrenGetSlotDouble(vm, 8);
    blink_blit_alpha(*image, *src_image, dx, dy, sx, sy, w, h, alpha);
}

void api_image_blit_tint(WrenVM *vm) {
    blink_image **image = (blink_image**)wrenGetSlotForeign(vm, 0);
    blink_assert_type(vm, 1, FOREIGN, "image");
    blink_assert_type(vm, 2, NUM, "dx");
    blink_assert_type(vm, 3, NUM, "dy");
    blink_assert_type(vm, 4, NUM, "sx");
    blink_assert_type(vm, 5, NUM, "sy");
    blink_assert_type(vm, 6, NUM, "w");
    blink_assert_type(vm, 7, NUM, "h");
    blink_assert_type(vm, 8, FOREIGN, "tint");
    blink_image **src_image = (blink_image**)wrenGetSlotForeign(vm, 1);
    int dx = (int)wrenGetSlotDouble(vm, 2);
    int dy = (int)wrenGetSlotDouble(vm, 3);
    int sx = (int)wrenGetSlotDouble(vm, 4);
    int sy = (int)wrenGetSlotDouble(vm, 5);
    int w = (int)wrenGetSlotDouble(vm, 6);
    int h = (int)wrenGetSlotDouble(vm, 7);
    blink_color *tint = (blink_color*)wrenGetSlotForeign(vm, 8);
    blink_blit_tint(*image, *src_image, dx, dy, sx, sy, w, h, *tint);
}

void api_image_print(WrenVM *vm) {
    blink_image **image = (blink_image**)wrenGetSlotForeign(vm, 0);
    blink_state *state = (blink_state*)wrenGetUserData(vm);
    blink_assert_type(vm, 1, STRING, "text");
    blink_assert_type(vm, 2, NUM, "x");
    blink_assert_type(vm, 3, NUM, "y");
    blink_assert_type(vm, 4, FOREIGN, "color");
    char *text = (char*)wrenGetSlotString(vm, 1);
    int x = (int)wrenGetSlotDouble(vm, 2);
    int y = (int)wrenGetSlotDouble(vm, 3);
    blink_color *color = (blink_color*)wrenGetSlotForeign(vm, 4);
    blink_draw_text(*image, state->font, text, x, y, *color);
}

void api_image_save(WrenVM *vm) {
    blink_image **image = (blink_image**)wrenGetSlotForeign(vm, 0);
    blink_assert_type(vm, 1, STRING, "filename");
    const char *filename = wrenGetSlotString(vm, 1);
    blink_save_image(*image, filename);
}

void api_image_get_width(WrenVM *vm) {
    blink_image **image = (blink_image**)wrenGetSlotForeign(vm, 0);
    wrenSetSlotDouble(vm, 0, (*image)->w);
}

void api_image_get_height(WrenVM *vm) {
    blink_image **image = (blink_image**)wrenGetSlotForeign(vm, 0);
    wrenSetSlotDouble(vm, 0, (*image)->h);
}

//--------------------
// Input
//--------------------

void api_keyboard_down(WrenVM *vm) {
    blink_state *state = (blink_state*)wrenGetUserData(vm);
    blink_assert_type(vm, 1, STRING, "key");
    const char *key_string = wrenGetSlotString(vm, 1);
    int *key = map_get(&state->keymap, key_string);
    if (!key) {
        blink_abort_vm(vm, "Invalid key");
        return;
    }

    wrenSetSlotBool(vm, 0, cri_get_keyboard_state(state->window)[*key]);
}

void api_keyboard_pressed(WrenVM *vm) {
    blink_state *state = (blink_state*)wrenGetUserData(vm);
    blink_assert_type(vm, 1, STRING, "key");
    const char *key_string = wrenGetSlotString(vm, 1);
    int *key = map_get(&state->keymap, key_string);
    if (!key) {
        blink_abort_vm(vm, "Invalid key");
        return;
    }

    wrenSetSlotBool(vm, 0, cri_get_keyboard_state(state->window)[*key] && !state->prev_keyboard_state[*key]);
}

void api_mouse_down(WrenVM *vm) {
    blink_state *state = (blink_state*)wrenGetUserData(vm);
    blink_assert_type(vm, 1, NUM, "button");
    int button = (int)wrenGetSlotDouble(vm, 1);
    wrenSetSlotBool(vm, 0, cri_get_mouse_state(state->window)[button]);
}

void api_mouse_pressed(WrenVM *vm) {
    blink_state *state = (blink_state*)wrenGetUserData(vm);
    blink_assert_type(vm, 1, NUM, "button");
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

void api_directory_exists(WrenVM *vm) {
    blink_assert_type(vm, 1, STRING, "path");
    const char *path = wrenGetSlotString(vm, 1);
    wrenSetSlotBool(vm, 0, cri_dir_exists(path));
}

void api_directory_list(WrenVM *vm) {
    blink_assert_type(vm, 1, STRING, "path");
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
    blink_assert_type(vm, 1, STRING, "path");
    const char *path = wrenGetSlotString(vm, 1);
    wrenSetSlotBool(vm, 0, cri_file_exists(path));
}

void api_file_size(WrenVM *vm) {
    blink_assert_type(vm, 1, STRING, "path");
    const char *path = wrenGetSlotString(vm, 1);
    wrenSetSlotDouble(vm, 0, cri_get_file_size(path));
}

void api_file_mod_time(WrenVM *vm) {
    blink_assert_type(vm, 1, STRING, "path");
    const char *path = wrenGetSlotString(vm, 1);
    wrenSetSlotDouble(vm, 0, cri_get_file_mod_time(path));
}

void api_file_read(WrenVM *vm) {
    blink_assert_type(vm, 1, STRING, "path");
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
    blink_assert_type(vm, 1, STRING, "path");
    blink_assert_type(vm, 2, STRING, "data");
    const char *path = wrenGetSlotString(vm, 1);
    int size;
    const char *data = wrenGetSlotBytes(vm, 2, &size);
    cri_write_file(path, data, size);
}
