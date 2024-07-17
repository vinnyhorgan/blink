#include "cri.h"
#include "lib/wren.h"

#include "blink_graphics.h"
#include "blink_api.h"

#include "api.wren.h"

#include <stdio.h>
#include <stdlib.h>
#include <direct.h>

#ifdef _WIN32
#include <windows.h>
#endif

#define blink_min(a, b) ((a) < (b) ? (a) : (b))

// This file is a bit of a mess at the moment...

static void word_wrap(const char *input, int line_width, char *output) {
    int input_len = strlen(input);
    int output_index = 0;
    int line_start = 0;

    while (line_start < input_len) {
        int line_end = line_start + line_width;

        if (line_end >= input_len) {
            // If the remaining part of the string is shorter than the line width
            strcpy(output + output_index, input + line_start);
            output_index += strlen(input + line_start);
            break;
        }

        // Find the last space within the current line width
        int last_space = -1;
        for (int i = line_start; i < line_end; i++) {
            if (isspace(input[i])) {
                last_space = i;
            }
        }

        if (last_space == -1) {
            // No spaces found, force a line break
            strncpy(output + output_index, input + line_start, line_width);
            output_index += line_width;
            output[output_index++] = '\n';
            line_start += line_width;
        } else {
            // Break at the last space
            strncpy(output + output_index, input + line_start, last_space - line_start);
            output_index += last_space - line_start;
            output[output_index++] = '\n';
            line_start = last_space + 1;
        }

        // Skip any leading spaces at the start of the new line
        while (isspace(input[line_start])) {
            line_start++;
        }
    }

    output[output_index] = '\0'; // Null-terminate the output string
}

static void wren_write(WrenVM *vm, const char *text) {
    printf("%s", text);
}

static void wren_error(WrenVM *vm, WrenErrorType type, const char *module, int line, const char *message)
{
    blink_state *state = wrenGetUserData(vm);

    int max_width = state->width / 8;

    char formatted_message[1024];
    char wrapped_message[1024];

    switch (type) {
    case WREN_ERROR_COMPILE:
        state->error = true;
        blink_set_clip(state->screen, blink_new_rect(0, 0, -1, -1));
        snprintf(formatted_message, sizeof(formatted_message), "[%s line %d] %s", module, line, message);
        word_wrap(formatted_message, max_width, wrapped_message);
        snprintf(state->error_message, sizeof(state->error_message), "COMPILE ERROR :(\n\n%s", wrapped_message);
        break;
    case WREN_ERROR_RUNTIME:
        state->error = true;
        blink_set_clip(state->screen, blink_new_rect(0, 0, -1, -1));
        snprintf(formatted_message, sizeof(formatted_message), "%s", message);
        word_wrap(formatted_message, max_width, wrapped_message);
        snprintf(state->error_message, sizeof(state->error_message), "RUNTIME ERROR :(\n\n%s\n\nSTACK TRACE:\n\n", wrapped_message);
        break;
    case WREN_ERROR_STACK_TRACE:
        snprintf(formatted_message, sizeof(formatted_message), "[%s line %d] in %s", module, line, message);
        word_wrap(formatted_message, max_width, wrapped_message);
        snprintf(state->error_message, sizeof(state->error_message), "%s%s\n", state->error_message, wrapped_message);
        break;
    }
}

static WrenLoadModuleResult wren_load_module(WrenVM *vm, const char *name) {
    WrenLoadModuleResult result = { 0 };

    if (!strcmp(name, "meta") || !strcmp(name, "random"))
        return result;

    if (!strcmp(name, "blink")) {
        result.source = api_source;
        return result;
    }

    return result;
}

static WrenForeignClassMethods wren_bind_foreign_class(WrenVM *vm, const char *module, const char *class_name) {
    WrenForeignClassMethods methods = { 0 };

    if (!strcmp(class_name, "Image")) {
        methods.allocate = api_image_allocate;
        methods.finalize = api_image_finalize;
    } else if (!strcmp(class_name, "Color")) {
        methods.allocate = api_color_allocate;
    }

    return methods;
}

static WrenForeignMethodFn wren_bind_foreign_method(WrenVM *vm, const char *module, const char *class_name, bool is_static, const char *signature) {
    if (!strcmp(class_name, "Graphics")) {
        if (!strcmp(signature, "screenshot()"))
            return api_graphics_screenshot;
        if (!strcmp(signature, "measure(_)"))
            return api_graphics_measure;
        if (!strcmp(signature, "clear(_)"))
            return api_graphics_clear;
        if (!strcmp(signature, "clip(_,_,_,_)"))
            return api_graphics_clip;
        if (!strcmp(signature, "set(_,_,_)"))
            return api_graphics_set;
        if (!strcmp(signature, "get(_,_)"))
            return api_graphics_get;
        if (!strcmp(signature, "fill(_,_,_,_,_)"))
            return api_graphics_fill;
        if (!strcmp(signature, "line(_,_,_,_,_)"))
            return api_graphics_line;
        if (!strcmp(signature, "rectangle(_,_,_,_,_)"))
            return api_graphics_rectangle;
        if (!strcmp(signature, "fillRectangle(_,_,_,_,_)"))
            return api_graphics_fill_rectangle;
        if (!strcmp(signature, "circle(_,_,_,_)"))
            return api_graphics_circle;
        if (!strcmp(signature, "fillCircle(_,_,_,_)"))
            return api_graphics_fill_circle;
        if (!strcmp(signature, "blit(_,_,_,_,_,_,_)"))
            return api_graphics_blit;
        if (!strcmp(signature, "blitAlpha(_,_,_,_,_,_,_,_)"))
            return api_graphics_blit_alpha;
        if (!strcmp(signature, "blitTint(_,_,_,_,_,_,_,_)"))
            return api_graphics_blit_tint;
        if (!strcmp(signature, "print(_,_,_,_)"))
            return api_graphics_print;
        if (!strcmp(signature, "clearColor=(_)"))
            return api_graphics_set_clear_color;
    } else if (!strcmp(class_name, "Window")) {
        if (!strcmp(signature, "active"))
            return api_window_get_active;
        if (!strcmp(signature, "width"))
            return api_window_get_width;
        if (!strcmp(signature, "height"))
            return api_window_get_height;
        if (!strcmp(signature, "close()"))
            return api_window_close;
    } else if (!strcmp(class_name, "Directory")) {
        if (!strcmp(signature, "exists(_)"))
            return api_directory_exists;
        if (!strcmp(signature, "list(_)"))
            return api_directory_list;
    } else if (!strcmp(class_name, "File")) {
        if (!strcmp(signature, "exists(_)"))
            return api_file_exists;
        if (!strcmp(signature, "size(_)"))
            return api_file_size;
        if (!strcmp(signature, "modTime(_)"))
            return api_file_mod_time;
        if (!strcmp(signature, "read(_)"))
            return api_file_read;
        if (!strcmp(signature, "write(_,_)"))
            return api_file_write;
    } else if (!strcmp(class_name, "Mouse")) {
        if (!strcmp(signature, "down(_)"))
            return api_mouse_down;
        if (!strcmp(signature, "pressed(_)"))
            return api_mouse_pressed;
        if (!strcmp(signature, "x"))
            return api_mouse_get_x;
        if (!strcmp(signature, "y"))
            return api_mouse_get_y;
        if (!strcmp(signature, "scrollX"))
            return api_mouse_get_scroll_x;
        if (!strcmp(signature, "scrollY"))
            return api_mouse_get_scroll_y;
    } else if (!strcmp(class_name, "Keyboard")) {
        if (!strcmp(signature, "down(_)"))
            return api_keyboard_down;
        if (!strcmp(signature, "pressed(_)"))
            return api_keyboard_pressed;
    } else if (!strcmp(class_name, "OS")) {
        if (!strcmp(signature, "name"))
            return api_os_get_name;
        if (!strcmp(signature, "blinkVersion"))
            return api_os_get_blink_version;
        if (!strcmp(signature, "args"))
            return api_os_get_args;
    } else if (!strcmp(class_name, "Color")) {
        if (!strcmp(signature, "init new(_,_,_,_)"))
            return api_color_new_rgba;
        if (!strcmp(signature, "init new(_,_,_)"))
            return api_color_new_rgb;
        if (!strcmp(signature, "[_]"))
            return api_color_get_index;
        if (!strcmp(signature, "[_]=(_)"))
            return api_color_set_index;
    } else if (!strcmp(class_name, "Image")) {
        if (!strcmp(signature, "init new(_,_)"))
            return api_image_new_wh;
        if (!strcmp(signature, "init new(_)"))
            return api_image_new_filename;
        if (!strcmp(signature, "save(_)"))
            return api_image_save;
        if (!strcmp(signature, "clear(_)"))
            return api_image_clear;
        if (!strcmp(signature, "clip(_,_,_,_)"))
            return api_image_clip;
        if (!strcmp(signature, "set(_,_,_)"))
            return api_image_set;
        if (!strcmp(signature, "get(_,_)"))
            return api_image_get;
        if (!strcmp(signature, "fill(_,_,_,_,_)"))
            return api_image_fill;
        if (!strcmp(signature, "line(_,_,_,_,_)"))
            return api_image_line;
        if (!strcmp(signature, "rectangle(_,_,_,_,_)"))
            return api_image_rectangle;
        if (!strcmp(signature, "fillRectangle(_,_,_,_,_)"))
            return api_image_fill_rectangle;
        if (!strcmp(signature, "circle(_,_,_,_)"))
            return api_image_circle;
        if (!strcmp(signature, "fillCircle(_,_,_,_)"))
            return api_image_fill_circle;
        if (!strcmp(signature, "blit(_,_,_,_,_,_,_)"))
            return api_image_blit;
        if (!strcmp(signature, "blitAlpha(_,_,_,_,_,_,_,_)"))
            return api_image_blit_alpha;
        if (!strcmp(signature, "blitTint(_,_,_,_,_,_,_,_)"))
            return api_image_blit_tint;
        if (!strcmp(signature, "print(_,_,_,_)"))
            return api_image_print;
        if (!strcmp(signature, "width"))
            return api_image_get_width;
        if (!strcmp(signature, "height"))
            return api_image_get_height;
    }

    return NULL;
}

static void on_active(cri_window *window, bool is_active) {
    blink_state *state = (blink_state*)cri_get_user_data(window);

    if (!state->error) {
        wrenSetSlotHandle(state->vm, 0, state->game);
        wrenSetSlotBool(state->vm, 1, is_active);
        wrenCall(state->vm, state->on_mouse_move);
    }
}

static void on_resize(cri_window *window, int width, int height) {
    blink_state *state = (blink_state*)cri_get_user_data(window);

    state->scale = blink_min((float)width / state->width, (float)height / state->height);

    float new_width = state->width * state->scale;
    float new_height = state->height * state->scale;

    float viewport_x = (width - new_width) / 2;
    float viewport_y = (height - new_height) / 2;

    cri_set_viewport(window, viewport_x, viewport_y, new_width, new_height);

    if (!state->error) {
        wrenSetSlotHandle(state->vm, 0, state->game);
        wrenSetSlotDouble(state->vm, 1, width);
        wrenSetSlotDouble(state->vm, 2, height);
        wrenCall(state->vm, state->on_resize);
    }
}

static void on_keyboard(cri_window *window, cri_key key, cri_mod_key mod, bool is_pressed) {
    blink_state *state = (blink_state*)cri_get_user_data(window);
}

static void codepoint_to_string(unsigned int codepoint, char *buffer) {
    if (codepoint <= 0x7F) {
        buffer[0] = (char)codepoint;
        buffer[1] = '\0';
    } else if (codepoint <= 0x7FF) {
        buffer[0] = (char)(0xC0 | (codepoint >> 6));
        buffer[1] = (char)(0x80 | (codepoint & 0x3F));
        buffer[2] = '\0';
    } else if (codepoint <= 0xFFFF) {
        buffer[0] = (char)(0xE0 | (codepoint >> 12));
        buffer[1] = (char)(0x80 | ((codepoint >> 6) & 0x3F));
        buffer[2] = (char)(0x80 | (codepoint & 0x3F));
        buffer[3] = '\0';
    } else if (codepoint <= 0x10FFFF) {
        buffer[0] = (char)(0xF0 | (codepoint >> 18));
        buffer[1] = (char)(0x80 | ((codepoint >> 12) & 0x3F));
        buffer[2] = (char)(0x80 | ((codepoint >> 6) & 0x3F));
        buffer[3] = (char)(0x80 | (codepoint & 0x3F));
        buffer[4] = '\0';
    } else {
        buffer[0] = '\0';
    }
}

static void on_char_input(cri_window *window, unsigned int code) {
    blink_state *state = (blink_state*)cri_get_user_data(window);

    if (!state->error) {
        wrenSetSlotHandle(state->vm, 0, state->game);
        char codepoint[5];
        codepoint_to_string(code, codepoint);
        wrenSetSlotString(state->vm, 1, codepoint);
        wrenCall(state->vm, state->on_char_input);
    }
}

static void on_mouse_button(cri_window *window, cri_mouse_button button, cri_mod_key mod, bool is_pressed) {
    blink_state *state = (blink_state*)cri_get_user_data(window);

    if (!state->error) {
        wrenSetSlotHandle(state->vm, 0, state->game);
        wrenSetSlotDouble(state->vm, 1, button);
        wrenSetSlotBool(state->vm, 2, is_pressed);
        wrenCall(state->vm, state->on_mouse_button);
    }
}

static void on_mouse_move(cri_window *window, int x, int y) {
    blink_state *state = (blink_state*)cri_get_user_data(window);

    if (!state->error) {
        wrenSetSlotHandle(state->vm, 0, state->game);
        wrenSetSlotDouble(state->vm, 1, x);
        wrenSetSlotDouble(state->vm, 2, y);
        wrenCall(state->vm, state->on_mouse_move);
    }
}

static void on_mouse_scroll(cri_window *window, cri_mod_key mod, float dx, float dy) {
    blink_state *state = (blink_state*)cri_get_user_data(window);

    if (!state->error) {
        wrenSetSlotHandle(state->vm, 0, state->game);
        wrenSetSlotDouble(state->vm, 1, dx);
        wrenSetSlotDouble(state->vm, 2, dy);
        wrenCall(state->vm, state->on_mouse_scroll);
    }
}

static void *default_font_data;
static int default_font_size;

int main(int argc, char **argv) {
    blink_state state;
    memset(&state, 0, sizeof(state));

    state.argc = argc;
    state.argv = argv;

    state.error = false;
    state.error_message[0] = '\0';

    state.clear_color = blink_rgb(255, 255, 255);
    state.font = blink_load_font_mem(default_font_data, default_font_size);
    state.scale = 2.0f;
    state.width = 320;
    state.height = 240;

    map_init(&state.keymap);

    map_set(&state.keymap, "space", KB_KEY_SPACE);

    strcpy(state.title, "Blink");

    WrenConfiguration config;
    wrenInitConfiguration(&config);

    config.writeFn = wren_write;
    config.errorFn = wren_error;
    config.loadModuleFn = wren_load_module;
    config.bindForeignMethodFn = wren_bind_foreign_method;
    config.bindForeignClassFn = wren_bind_foreign_class;

    WrenVM *vm = wrenNewVM(&config);

    state.vm = vm;

    wrenInterpret(vm, "blink", api_source);

    wrenEnsureSlots(vm, 1);

    wrenGetVariable(vm, "blink", "Color", 0);
    state.color_class = wrenGetSlotHandle(vm, 0);

    wrenGetVariable(vm, "blink", "Image", 0);
    state.image_class = wrenGetSlotHandle(vm, 0);

    wrenSetUserData(vm, &state);

    WrenHandle *conf, *init, *update, *draw;

    chdir(argv[1]);

    int flags = 0;

    char *source = cri_read_file("main.wren", NULL);
    if (!source) {
        state.error = true;
        strcpy(state.error_message, "Failed to read source file");
    } else {
        WrenInterpretResult res = wrenInterpret(vm, argv[1], source);
        if (res == WREN_RESULT_SUCCESS) {
            wrenEnsureSlots(vm, 1);
            wrenGetVariable(vm, argv[1], "Game", 0);
            state.game = wrenGetSlotHandle(vm, 0);
            conf = wrenMakeCallHandle(vm, "config(_)");
            init = wrenMakeCallHandle(vm, "init()");
            update = wrenMakeCallHandle(vm, "update(_)");
            draw = wrenMakeCallHandle(vm, "draw()");
            state.on_active = wrenMakeCallHandle(vm, "active(_)");
            state.on_resize = wrenMakeCallHandle(vm, "resize(_,_)");
            state.on_keyboard = wrenMakeCallHandle(vm, "keyboard(_,_)");
            state.on_char_input = wrenMakeCallHandle(vm, "input(_)");
            state.on_mouse_button = wrenMakeCallHandle(vm, "mouseButton(_,_)");
            state.on_mouse_move = wrenMakeCallHandle(vm, "mouseMove(_,_)");
            state.on_mouse_scroll = wrenMakeCallHandle(vm, "mouseScroll(_,_)");

            wrenEnsureSlots(vm, 4);

            wrenSetSlotNewMap(vm, 1);

            wrenSetSlotString(vm, 2, "console");
            wrenSetSlotBool(vm, 3, false);
            wrenSetMapValue(vm, 1, 2, 3);

            wrenSetSlotString(vm, 2, "resizable");
            wrenSetSlotBool(vm, 3, true);
            wrenSetMapValue(vm, 1, 2, 3);

            wrenSetSlotString(vm, 2, "hideCursor");
            wrenSetSlotBool(vm, 3, false);
            wrenSetMapValue(vm, 1, 2, 3);

            wrenSetSlotString(vm, 2, "title");
            wrenSetSlotString(vm, 3, "Blink");
            wrenSetMapValue(vm, 1, 2, 3);

            wrenSetSlotString(vm, 2, "console");
            wrenSetSlotBool(vm, 3, false);
            wrenSetMapValue(vm, 1, 2, 3);

            wrenSetSlotString(vm, 2, "targetFps");
            wrenSetSlotDouble(vm, 3, 60);
            wrenSetMapValue(vm, 1, 2, 3);

            wrenSetSlotString(vm, 2, "width");
            wrenSetSlotDouble(vm, 3, 320);
            wrenSetMapValue(vm, 1, 2, 3);

            wrenSetSlotString(vm, 2, "height");
            wrenSetSlotDouble(vm, 3, 240);
            wrenSetMapValue(vm, 1, 2, 3);

            wrenSetSlotString(vm, 2, "scale");
            wrenSetSlotDouble(vm, 3, 2);
            wrenSetMapValue(vm, 1, 2, 3);

            wrenSetSlotHandle(vm, 0, state.game);
            WrenInterpretResult res = wrenCall(vm, conf);

            if (res == WREN_RESULT_SUCCESS) {
                wrenEnsureSlots(vm, 4);

                wrenSetSlotString(vm, 2, "console");
                wrenGetMapValue(vm, 1, 2, 3);
                if (wrenGetSlotType(vm, 3) == WREN_TYPE_BOOL) {
                    state.console = wrenGetSlotBool(vm, 3);
                    if (state.console) {
                        if (AllocConsole()) {
                            freopen("CONOUT$", "w", stdout);
                            freopen("CONOUT$", "w", stderr);
                            freopen("CONIN$", "r", stdin);
                        }
                    }
                } else {
                    state.console = false;
                }

                wrenSetSlotString(vm, 2, "resizable");
                wrenGetMapValue(vm, 1, 2, 3);
                if (wrenGetSlotType(vm, 3) == WREN_TYPE_BOOL) {
                    bool resizable = wrenGetSlotBool(vm, 3);
                    if (resizable) {
                        flags |= FLAG_RESIZABLE;
                    }
                }

                wrenSetSlotString(vm, 2, "hideCursor");
                wrenGetMapValue(vm, 1, 2, 3);
                if (wrenGetSlotType(vm, 3) == WREN_TYPE_BOOL) {
                    bool hide_cursor = wrenGetSlotBool(vm, 3);
                    if (hide_cursor) {
                        flags |= FLAG_HIDECURSOR;
                    }
                }

                wrenSetSlotString(vm, 2, "title");
                wrenGetMapValue(vm, 1, 2, 3);
                if (wrenGetSlotType(vm, 3) == WREN_TYPE_STRING) {
                    const char *title = wrenGetSlotString(vm, 3);
                    strcpy(state.title, title);
                } else {
                    strcpy(state.title, "Blink");
                }

                wrenSetSlotString(vm, 2, "targetFps");
                wrenGetMapValue(vm, 1, 2, 3);
                if (wrenGetSlotType(vm, 3) == WREN_TYPE_NUM) {
                    cri_set_target_fps((int)wrenGetSlotDouble(vm, 3));
                }

                wrenSetSlotString(vm, 2, "width");
                wrenGetMapValue(vm, 1, 2, 3);
                if (wrenGetSlotType(vm, 3) == WREN_TYPE_NUM) {
                    state.width = (int)wrenGetSlotDouble(vm, 3);
                } else {
                    state.width = 320;
                }

                wrenSetSlotString(vm, 2, "height");
                wrenGetMapValue(vm, 1, 2, 3);
                if (wrenGetSlotType(vm, 3) == WREN_TYPE_NUM) {
                    state.height = (int)wrenGetSlotDouble(vm, 3);
                } else {
                    state.height = 240;
                }

                wrenSetSlotString(vm, 2, "scale");
                wrenGetMapValue(vm, 1, 2, 3);
                if (wrenGetSlotType(vm, 3) == WREN_TYPE_NUM) {
                    state.scale = (int)wrenGetSlotDouble(vm, 3);
                } else {
                    state.scale = 2;
                }

                wrenSetSlotHandle(vm, 0, state.game);
                wrenCall(vm, init);
            }
        }

        free(source);
    }

    state.window = cri_open(state.title, state.width * state.scale, state.height * state.scale, flags);
    if (!state.window)
        return 1;

    state.screen = blink_create_image(state.width, state.height);

    cri_set_active_cb(state.window, on_active);
    cri_set_resize_cb(state.window, on_resize);
    cri_set_keyboard_cb(state.window, on_keyboard);
    cri_set_char_input_cb(state.window, on_char_input);
    cri_set_mouse_button_cb(state.window, on_mouse_button);
    cri_set_mouse_move_cb(state.window, on_mouse_move);
    cri_set_mouse_scroll_cb(state.window, on_mouse_scroll);

    cri_set_user_data(state.window, &state);

    cri_timer *timer = cri_timer_create();
    double dt = 0;

    do {
        if (state.error) {
            blink_clear(state.screen, blink_rgb(255, 130, 170));
            blink_draw_text(state.screen, state.font, state.error_message, 10, 10, blink_rgb(255, 241, 232));
        } else {
            cri_timer_now(timer);

            wrenSetSlotHandle(vm, 0, state.game);
            wrenSetSlotDouble(vm, 1, dt);
            WrenInterpretResult res = wrenCall(vm, update);
            if (res == WREN_RESULT_SUCCESS) {
                blink_clear(state.screen, state.clear_color);

                wrenSetSlotHandle(vm, 0, state.game);
                wrenCall(vm, draw);
            }
        }

        memcpy(state.prev_keyboard_state, cri_get_keyboard_state(state.window), 512);
        memcpy(state.prev_mouse_state, cri_get_mouse_state(state.window), 8);

        int r = cri_update(state.window, state.screen->pixels, state.screen->w, state.screen->h);
        if (r != 0) break;

        dt = cri_timer_dt(timer);
    } while (cri_wait_sync(state.window));

    map_deinit(&state.keymap);

    if (state.console)
        FreeConsole();

    wrenFreeVM(vm);

    return 0;
}

static char default_font[] = {
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

static void *default_font_data = default_font;
static int default_font_size = sizeof(default_font);
