#ifndef BLINK_API_H
#define BLINK_API_H

#include "cri.h"
#include "lib/wren.h"
#include "lib/map.h"

#include "blink_graphics.h"

#define BLINK_VERSION "0.1.0"

typedef struct {
    bool error;
    char error_message[1024];
    uint8_t prev_keyboard_state[512];
    uint8_t prev_mouse_state[8];
    bool console;
    int argc;
    char **argv;
    int width, height;
    float scale;
    blink_color clear_color;
    blink_font *font;
    char title[256];
    map_int_t keymap;
    blink_image *screen;
    cri_window *window;
    WrenVM *vm;
    WrenHandle *color_class;
    WrenHandle *image_class;
    WrenHandle *game;
    WrenHandle *on_config;
    WrenHandle *on_init;
    WrenHandle *on_update;
    WrenHandle *on_draw;
    WrenHandle *on_active;
    WrenHandle *on_resize;
    WrenHandle *on_keyboard;
    WrenHandle *on_char_input;
    WrenHandle *on_mouse_button;
    WrenHandle *on_mouse_move;
    WrenHandle *on_mouse_scroll;
    WrenHandle *on_drop;
} blink_state;

//--------------------
// Graphics
//--------------------

void api_graphics_clip(WrenVM *vm);
void api_graphics_clear(WrenVM *vm);
void api_graphics_get(WrenVM *vm);
void api_graphics_set(WrenVM *vm);
void api_graphics_line(WrenVM *vm);
void api_graphics_fill(WrenVM *vm);
void api_graphics_rectangle(WrenVM *vm);
void api_graphics_fill_rectangle(WrenVM *vm);
void api_graphics_circle(WrenVM *vm);
void api_graphics_fill_circle(WrenVM *vm);
void api_graphics_blit(WrenVM *vm);
void api_graphics_blit_alpha(WrenVM *vm);
void api_graphics_blit_tint(WrenVM *vm);
void api_graphics_print(WrenVM *vm);
void api_graphics_print_font(WrenVM *vm);
void api_graphics_screenshot(WrenVM *vm);
void api_graphics_measure(WrenVM *vm);
void api_graphics_get_width(WrenVM *vm);
void api_graphics_get_height(WrenVM *vm);
void api_graphics_set_clear_color(WrenVM *vm);

void api_color_allocate(WrenVM *vm);
void api_color_new_rgba(WrenVM *vm);
void api_color_new_rgb(WrenVM *vm);
void api_color_get_index(WrenVM *vm);
void api_color_set_index(WrenVM *vm);

void api_image_allocate(WrenVM *vm);
void api_image_finalize(void *data);
void api_image_new_wh(WrenVM *vm);
void api_image_new_filename(WrenVM *vm);
void api_image_new_from_memory(WrenVM *vm);
void api_image_clip(WrenVM *vm);
void api_image_clear(WrenVM *vm);
void api_image_get(WrenVM *vm);
void api_image_set(WrenVM *vm);
void api_image_line(WrenVM *vm);
void api_image_fill(WrenVM *vm);
void api_image_rectangle(WrenVM *vm);
void api_image_fill_rectangle(WrenVM *vm);
void api_image_circle(WrenVM *vm);
void api_image_fill_circle(WrenVM *vm);
void api_image_blit(WrenVM *vm);
void api_image_blit_alpha(WrenVM *vm);
void api_image_blit_tint(WrenVM *vm);
void api_image_print(WrenVM *vm);
void api_image_print_font(WrenVM *vm);
void api_image_resize(WrenVM *vm);
void api_image_save(WrenVM *vm);
void api_image_save_to_memory(WrenVM *vm);
void api_image_get_width(WrenVM *vm);
void api_image_get_height(WrenVM *vm);

void api_font_allocate(WrenVM *vm);
void api_font_finalize(void *data);
void api_font_new(WrenVM *vm);
void api_font_new_from_memory(WrenVM *vm);
void api_font_measure(WrenVM *vm);

//--------------------
// Input
//--------------------

void api_keyboard_down(WrenVM *vm);
void api_keyboard_pressed(WrenVM *vm);

void api_mouse_down(WrenVM *vm);
void api_mouse_pressed(WrenVM *vm);
void api_mouse_get_x(WrenVM *vm);
void api_mouse_get_y(WrenVM *vm);
void api_mouse_get_scroll_x(WrenVM *vm);
void api_mouse_get_scroll_y(WrenVM *vm);

//--------------------
// System
//--------------------

void api_window_close(WrenVM *vm);
void api_window_get_active(WrenVM *vm);
void api_window_get_width(WrenVM *vm);
void api_window_get_height(WrenVM *vm);

void api_os_get_name(WrenVM *vm);
void api_os_get_blink_version(WrenVM *vm);
void api_os_get_args(WrenVM *vm);
void api_os_get_clipboard(WrenVM *vm);
void api_os_set_clipboard(WrenVM *vm);

void api_directory_exists(WrenVM *vm);
void api_directory_list(WrenVM *vm);

void api_file_exists(WrenVM *vm);
void api_file_size(WrenVM *vm);
void api_file_mod_time(WrenVM *vm);
void api_file_read(WrenVM *vm);
void api_file_write(WrenVM *vm);

#endif
