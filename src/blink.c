#include "cri.h"
#include "lib/wren.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#endif

#include "blink_api.h"
#include "blink_graphics.h"
#include "blink_audio.h"
#include "api.wren.h"

//--------------------
// Wren Configuration
//--------------------

static void word_wrap(const char *input, int line_width, char *output) {
    int input_len = strlen(input);
    int output_index = 0;
    int line_start = 0;

    while (line_start < input_len) {
        int line_end = line_start + line_width;

        if (line_end >= input_len) {
            strcpy(output + output_index, input + line_start);
            output_index += strlen(input + line_start);
            break;
        }

        int last_space = -1;
        for (int i = line_start; i < line_end; i++) {
            if (isspace(input[i])) {
                last_space = i;
            }
        }

        if (last_space == -1) {
            strncpy(output + output_index, input + line_start, line_width);
            output_index += line_width;
            output[output_index++] = '\n';
            line_start += line_width;
        } else {
            strncpy(output + output_index, input + line_start, last_space - line_start);
            output_index += last_space - line_start;
            output[output_index++] = '\n';
            line_start = last_space + 1;
        }

        while (isspace(input[line_start])) {
            line_start++;
        }
    }

    output[output_index] = '\0';
}

static void on_complete(WrenVM *vm, const char *name, WrenLoadModuleResult result) {
    if (result.source)
        free((void*)result.source);
}

static WrenLoadModuleResult wren_load_module(WrenVM *vm, const char *name) {
    WrenLoadModuleResult result = { 0 };

    if (!strcmp(name, "meta") || !strcmp(name, "random"))
        return result;

    if (!strcmp(name, "blink")) {
        result.source = api_source;
        return result;
    }

    char full_path[256];
    snprintf(full_path, sizeof(full_path), "%s.wren", name);

    result.source = cri_read_file(full_path, NULL);
    result.onComplete = on_complete;

    return result;
}

#define BIND_METHOD(s, m)      \
    if (!strcmp(signature, s)) \
        return m;

static WrenForeignMethodFn wren_bind_foreign_method(WrenVM *vm, const char *module, const char *class_name, bool is_static, const char *signature) {
    if (!strcmp(class_name, "Graphics")) {
        BIND_METHOD("clip(_,_,_,_)", api_graphics_clip);
        BIND_METHOD("clear(_)", api_graphics_clear);
        BIND_METHOD("get(_,_)", api_graphics_get);
        BIND_METHOD("set(_,_,_)", api_graphics_set);
        BIND_METHOD("line(_,_,_,_,_)", api_graphics_line);
        BIND_METHOD("fill(_,_,_,_,_)", api_graphics_fill);
        BIND_METHOD("rectangle(_,_,_,_,_)", api_graphics_rectangle);
        BIND_METHOD("fillRectangle(_,_,_,_,_)", api_graphics_fill_rectangle);
        BIND_METHOD("circle(_,_,_,_)", api_graphics_circle);
        BIND_METHOD("fillCircle(_,_,_,_)", api_graphics_fill_circle);
        BIND_METHOD("blit(_,_,_,_,_,_,_)", api_graphics_blit);
        BIND_METHOD("blitAlpha(_,_,_,_,_,_,_,_)", api_graphics_blit_alpha);
        BIND_METHOD("blitTint(_,_,_,_,_,_,_,_)", api_graphics_blit_tint);
        BIND_METHOD("print(_,_,_,_)", api_graphics_print);
        BIND_METHOD("print(_,_,_,_,_)", api_graphics_print_font);
        BIND_METHOD("screenshot()", api_graphics_screenshot);
        BIND_METHOD("measure(_)", api_graphics_measure);
        BIND_METHOD("width", api_graphics_get_width);
        BIND_METHOD("height", api_graphics_get_height);
        BIND_METHOD("clearColor=(_)", api_graphics_set_clear_color);
    } else if (!strcmp(class_name, "Color")) {
        BIND_METHOD("init new(_,_,_,_)", api_color_new_rgba);
        BIND_METHOD("init new(_,_,_)", api_color_new_rgb);
        BIND_METHOD("[_]", api_color_get_index);
        BIND_METHOD("[_]=(_)", api_color_set_index);
    } else if (!strcmp(class_name, "Image")) {
        BIND_METHOD("init new(_,_)", api_image_new);
        BIND_METHOD("init new(_)", api_image_new_filename);
        BIND_METHOD("init fromMemory(_)", api_image_new_from_memory);
        BIND_METHOD("clip(_,_,_,_)", api_image_clip);
        BIND_METHOD("clear(_)", api_image_clear);
        BIND_METHOD("get(_,_)", api_image_get);
        BIND_METHOD("set(_,_,_)", api_image_set);
        BIND_METHOD("line(_,_,_,_,_)", api_image_line);
        BIND_METHOD("fill(_,_,_,_,_)", api_image_fill);
        BIND_METHOD("rectangle(_,_,_,_,_)", api_image_rectangle);
        BIND_METHOD("fillRectangle(_,_,_,_,_)", api_image_fill_rectangle);
        BIND_METHOD("circle(_,_,_,_)", api_image_circle);
        BIND_METHOD("fillCircle(_,_,_,_)", api_image_fill_circle);
        BIND_METHOD("blit(_,_,_,_,_,_,_)", api_image_blit);
        BIND_METHOD("blitAlpha(_,_,_,_,_,_,_,_)", api_image_blit_alpha);
        BIND_METHOD("blitTint(_,_,_,_,_,_,_,_)", api_image_blit_tint);
        BIND_METHOD("print(_,_,_,_)", api_image_print);
        BIND_METHOD("print(_,_,_,_,_)", api_image_print_font);
        BIND_METHOD("resize(_,_)", api_image_resize);
        BIND_METHOD("save(_,_)", api_image_save);
        BIND_METHOD("saveToMemory()", api_image_save_to_memory);
        BIND_METHOD("width", api_image_get_width);
        BIND_METHOD("height", api_image_get_height);
    } else if (!strcmp(class_name, "Font")) {
        BIND_METHOD("init new(_)", api_font_new);
        BIND_METHOD("init fromMemory(_)", api_font_new_from_memory);
        BIND_METHOD("measure(_)", api_font_measure);
    } else if (!strcmp(class_name, "Source")) {
        BIND_METHOD("init new(_)", api_source_new);
        BIND_METHOD("init fromSound(_)", api_source_new_from_sound);
        BIND_METHOD("play()", api_source_play);
        BIND_METHOD("pause()", api_source_pause);
        BIND_METHOD("stop()", api_source_stop);
        BIND_METHOD("length", api_source_get_length);
        BIND_METHOD("position", api_source_get_position);
        BIND_METHOD("state", api_source_get_state);
        BIND_METHOD("gain=(_)", api_source_set_gain);
        BIND_METHOD("pan=(_)", api_source_set_pan);
        BIND_METHOD("pitch=(_)", api_source_set_pitch);
        BIND_METHOD("loop=(_)", api_source_set_loop);
    } else if (!strcmp(class_name, "Sound")) {
        BIND_METHOD("init new(_,_,_,_)", api_sound_new);
        BIND_METHOD("init new(_)", api_sound_new_filename);
        BIND_METHOD("init fromMemory(_)", api_sound_new_from_memory);
        BIND_METHOD("getSample(_)", api_sound_get_sample);
        BIND_METHOD("setSample(_,_)", api_sound_set_sample);
        BIND_METHOD("save(_)", api_sound_save);
        BIND_METHOD("saveToMemory()", api_sound_save_to_memory);
        BIND_METHOD("bitDepth", api_sound_get_bit_depth);
        BIND_METHOD("sampleRate", api_sound_get_sample_rate);
        BIND_METHOD("channels", api_sound_get_channels);
        BIND_METHOD("length", api_sound_get_length);
    } else if (!strcmp(class_name, "Keyboard")) {
        BIND_METHOD("down(_)", api_keyboard_down);
        BIND_METHOD("pressed(_)", api_keyboard_pressed);
    } else if (!strcmp(class_name, "Mouse")) {
        BIND_METHOD("down(_)", api_mouse_down);
        BIND_METHOD("pressed(_)", api_mouse_pressed);
        BIND_METHOD("x", api_mouse_get_x);
        BIND_METHOD("y", api_mouse_get_y);
        BIND_METHOD("scrollX", api_mouse_get_scroll_x);
        BIND_METHOD("scrollY", api_mouse_get_scroll_y);
    } else if (!strcmp(class_name, "Window")) {
        BIND_METHOD("close()", api_window_close);
        BIND_METHOD("active", api_window_get_active);
        BIND_METHOD("width", api_window_get_width);
        BIND_METHOD("height", api_window_get_height);
    } else if (!strcmp(class_name, "OS")) {
        BIND_METHOD("name", api_os_get_name);
        BIND_METHOD("blinkVersion", api_os_get_blink_version);
        BIND_METHOD("args", api_os_get_args);
        BIND_METHOD("clipboard", api_os_get_clipboard);
        BIND_METHOD("clipboard=(_)", api_os_set_clipboard);
    } else if (!strcmp(class_name, "Directory")) {
        BIND_METHOD("exists(_)", api_directory_exists);
        BIND_METHOD("list(_)", api_directory_list);
    } else if (!strcmp(class_name, "File")) {
        BIND_METHOD("exists(_)", api_file_exists);
        BIND_METHOD("size(_)", api_file_size);
        BIND_METHOD("modTime(_)", api_file_mod_time);
        BIND_METHOD("read(_)", api_file_read);
        BIND_METHOD("write(_,_)", api_file_write);
    } else if (!strcmp(class_name, "Request")) {
        BIND_METHOD("init new(_)", api_request_new);
        BIND_METHOD("make()", api_request_make);
        BIND_METHOD("complete", api_request_get_complete);
        BIND_METHOD("status", api_request_get_status);
        BIND_METHOD("body", api_request_get_body);
    }

    return NULL;
}

static WrenForeignClassMethods wren_bind_foreign_class(WrenVM *vm, const char *module, const char *class_name) {
    WrenForeignClassMethods methods = { 0 };

    if (!strcmp(class_name, "Color")) {
        methods.allocate = api_color_allocate;
    } else if (!strcmp(class_name, "Image")) {
        methods.allocate = api_image_allocate;
        methods.finalize = api_image_finalize;
    } else if (!strcmp(class_name, "Font")) {
        methods.allocate = api_font_allocate;
        methods.finalize = api_font_finalize;
    } else if (!strcmp(class_name, "Source")) {
        methods.allocate = api_source_allocate;
        methods.finalize = api_source_finalize;
    } else if (!strcmp(class_name, "Sound")) {
        methods.allocate = api_sound_allocate;
        methods.finalize = api_sound_finalize;
    } else if (!strcmp(class_name, "Request")) {
        methods.allocate = api_request_allocate;
        methods.finalize = api_request_finalize;
    }

    return methods;
}

static void wren_write(WrenVM *vm, const char *text) {
    printf("%s", text);
}

static void wren_error(WrenVM *vm, WrenErrorType type, const char *module, int line, const char *message) {
    blink_state *state = wrenGetUserData(vm);

    int max_width = (state->width - 20) / 8;

    char formatted_message[1024];
    char wrapped_message[1024];

    switch (type) {
    case WREN_ERROR_COMPILE:
        state->error = true;

        if (state->screen)
            bg_set_clip(state->screen, bg_new_rect(0, 0, -1, -1));

        ba_set_master_gain(0);

        snprintf(formatted_message, sizeof(formatted_message), "[%s line %d] %s", module, line, message);
        word_wrap(formatted_message, max_width, wrapped_message);
        snprintf(state->error_message, sizeof(state->error_message), "COMPILE ERROR :(\n\n%s", wrapped_message);
        break;
    case WREN_ERROR_RUNTIME:
        state->error = true;

        if (state->screen)
            bg_set_clip(state->screen, bg_new_rect(0, 0, -1, -1));

        ba_set_master_gain(0);

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

//--------------------
// Cri Callbacks
//--------------------

static void on_active(cri_window *window, bool is_active) {
    blink_state *state = (blink_state*)cri_get_user_data(window);

    if (!state->error) {
        wrenSetSlotHandle(state->vm, 0, state->game);
        wrenSetSlotBool(state->vm, 1, is_active);
        wrenCall(state->vm, state->on_active);
    }
}

static void on_resize(cri_window *window, int width, int height) {
    blink_state *state = (blink_state*)cri_get_user_data(window);

    state->scale = MIN((float)width / state->width, (float)height / state->height);

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

static bool on_close(cri_window *window) {
    blink_state *state = (blink_state*)cri_get_user_data(window);

    if (!state->error) {
        wrenSetSlotHandle(state->vm, 0, state->game);
        wrenCall(state->vm, state->on_close);
        return wrenGetSlotBool(state->vm, 0);
    }

    return true;
}

static void on_keyboard(cri_window *window, cri_key key, cri_mod_key mod, bool is_pressed) {
    blink_state *state = (blink_state*)cri_get_user_data(window);

    if (!state->error) {
        wrenSetSlotHandle(state->vm, 0, state->game);

        switch (key) {
        case KB_KEY_UNKNOWN: wrenSetSlotString(state->vm, 1, "unknown"); break;
        case KB_KEY_SPACE: wrenSetSlotString(state->vm, 1, "space"); break;
        case KB_KEY_APOSTROPHE: wrenSetSlotString(state->vm, 1, "apostrophe"); break;
        case KB_KEY_COMMA: wrenSetSlotString(state->vm, 1, "comma"); break;
        case KB_KEY_MINUS: wrenSetSlotString(state->vm, 1, "minus"); break;
        case KB_KEY_PERIOD: wrenSetSlotString(state->vm, 1, "period"); break;
        case KB_KEY_SLASH: wrenSetSlotString(state->vm, 1, "slash"); break;
        case KB_KEY_0: wrenSetSlotString(state->vm, 1, "0"); break;
        case KB_KEY_1: wrenSetSlotString(state->vm, 1, "1"); break;
        case KB_KEY_2: wrenSetSlotString(state->vm, 1, "2"); break;
        case KB_KEY_3: wrenSetSlotString(state->vm, 1, "3"); break;
        case KB_KEY_4: wrenSetSlotString(state->vm, 1, "4"); break;
        case KB_KEY_5: wrenSetSlotString(state->vm, 1, "5"); break;
        case KB_KEY_6: wrenSetSlotString(state->vm, 1, "6"); break;
        case KB_KEY_7: wrenSetSlotString(state->vm, 1, "7"); break;
        case KB_KEY_8: wrenSetSlotString(state->vm, 1, "8"); break;
        case KB_KEY_9: wrenSetSlotString(state->vm, 1, "9"); break;
        case KB_KEY_SEMICOLON: wrenSetSlotString(state->vm, 1, "semicolon"); break;
        case KB_KEY_EQUAL: wrenSetSlotString(state->vm, 1, "equal"); break;
        case KB_KEY_A: wrenSetSlotString(state->vm, 1, "a"); break;
        case KB_KEY_B: wrenSetSlotString(state->vm, 1, "b"); break;
        case KB_KEY_C: wrenSetSlotString(state->vm, 1, "c"); break;
        case KB_KEY_D: wrenSetSlotString(state->vm, 1, "d"); break;
        case KB_KEY_E: wrenSetSlotString(state->vm, 1, "e"); break;
        case KB_KEY_F: wrenSetSlotString(state->vm, 1, "f"); break;
        case KB_KEY_G: wrenSetSlotString(state->vm, 1, "g"); break;
        case KB_KEY_H: wrenSetSlotString(state->vm, 1, "h"); break;
        case KB_KEY_I: wrenSetSlotString(state->vm, 1, "i"); break;
        case KB_KEY_J: wrenSetSlotString(state->vm, 1, "j"); break;
        case KB_KEY_K: wrenSetSlotString(state->vm, 1, "k"); break;
        case KB_KEY_L: wrenSetSlotString(state->vm, 1, "l"); break;
        case KB_KEY_M: wrenSetSlotString(state->vm, 1, "m"); break;
        case KB_KEY_N: wrenSetSlotString(state->vm, 1, "n"); break;
        case KB_KEY_O: wrenSetSlotString(state->vm, 1, "o"); break;
        case KB_KEY_P: wrenSetSlotString(state->vm, 1, "p"); break;
        case KB_KEY_Q: wrenSetSlotString(state->vm, 1, "q"); break;
        case KB_KEY_R: wrenSetSlotString(state->vm, 1, "r"); break;
        case KB_KEY_S: wrenSetSlotString(state->vm, 1, "s"); break;
        case KB_KEY_T: wrenSetSlotString(state->vm, 1, "t"); break;
        case KB_KEY_U: wrenSetSlotString(state->vm, 1, "u"); break;
        case KB_KEY_V: wrenSetSlotString(state->vm, 1, "v"); break;
        case KB_KEY_W: wrenSetSlotString(state->vm, 1, "w"); break;
        case KB_KEY_X: wrenSetSlotString(state->vm, 1, "x"); break;
        case KB_KEY_Y: wrenSetSlotString(state->vm, 1, "y"); break;
        case KB_KEY_Z: wrenSetSlotString(state->vm, 1, "z"); break;
        case KB_KEY_LEFT_BRACKET: wrenSetSlotString(state->vm, 1, "left_bracket"); break;
        case KB_KEY_BACKSLASH: wrenSetSlotString(state->vm, 1, "backslash"); break;
        case KB_KEY_RIGHT_BRACKET: wrenSetSlotString(state->vm, 1, "right_bracket"); break;
        case KB_KEY_GRAVE_ACCENT: wrenSetSlotString(state->vm, 1, "grave_accent"); break;
        case KB_KEY_WORLD_1: wrenSetSlotString(state->vm, 1, "world_1"); break;
        case KB_KEY_WORLD_2: wrenSetSlotString(state->vm, 1, "world_2"); break;
        case KB_KEY_ESCAPE: wrenSetSlotString(state->vm, 1, "escape"); break;
        case KB_KEY_ENTER: wrenSetSlotString(state->vm, 1, "enter"); break;
        case KB_KEY_TAB: wrenSetSlotString(state->vm, 1, "tab"); break;
        case KB_KEY_BACKSPACE: wrenSetSlotString(state->vm, 1, "backspace"); break;
        case KB_KEY_INSERT: wrenSetSlotString(state->vm, 1, "insert"); break;
        case KB_KEY_DELETE: wrenSetSlotString(state->vm, 1, "delete"); break;
        case KB_KEY_RIGHT: wrenSetSlotString(state->vm, 1, "right"); break;
        case KB_KEY_LEFT: wrenSetSlotString(state->vm, 1, "left"); break;
        case KB_KEY_DOWN: wrenSetSlotString(state->vm, 1, "down"); break;
        case KB_KEY_UP: wrenSetSlotString(state->vm, 1, "up"); break;
        case KB_KEY_PAGE_UP: wrenSetSlotString(state->vm, 1, "page_up"); break;
        case KB_KEY_PAGE_DOWN: wrenSetSlotString(state->vm, 1, "page_down"); break;
        case KB_KEY_HOME: wrenSetSlotString(state->vm, 1, "home"); break;
        case KB_KEY_END: wrenSetSlotString(state->vm, 1, "end"); break;
        case KB_KEY_CAPS_LOCK: wrenSetSlotString(state->vm, 1, "caps_lock"); break;
        case KB_KEY_SCROLL_LOCK: wrenSetSlotString(state->vm, 1, "scroll_lock"); break;
        case KB_KEY_NUM_LOCK: wrenSetSlotString(state->vm, 1, "num_lock"); break;
        case KB_KEY_PRINT_SCREEN: wrenSetSlotString(state->vm, 1, "print_screen"); break;
        case KB_KEY_PAUSE: wrenSetSlotString(state->vm, 1, "pause"); break;
        case KB_KEY_F1: wrenSetSlotString(state->vm, 1, "f1"); break;
        case KB_KEY_F2: wrenSetSlotString(state->vm, 1, "f2"); break;
        case KB_KEY_F3: wrenSetSlotString(state->vm, 1, "f3"); break;
        case KB_KEY_F4: wrenSetSlotString(state->vm, 1, "f4"); break;
        case KB_KEY_F5: wrenSetSlotString(state->vm, 1, "f5"); break;
        case KB_KEY_F6: wrenSetSlotString(state->vm, 1, "f6"); break;
        case KB_KEY_F7: wrenSetSlotString(state->vm, 1, "f7"); break;
        case KB_KEY_F8: wrenSetSlotString(state->vm, 1, "f8"); break;
        case KB_KEY_F9: wrenSetSlotString(state->vm, 1, "f9"); break;
        case KB_KEY_F10: wrenSetSlotString(state->vm, 1, "f10"); break;
        case KB_KEY_F11: wrenSetSlotString(state->vm, 1, "f11"); break;
        case KB_KEY_F12: wrenSetSlotString(state->vm, 1, "f12"); break;
        case KB_KEY_F13: wrenSetSlotString(state->vm, 1, "f13"); break;
        case KB_KEY_F14: wrenSetSlotString(state->vm, 1, "f14"); break;
        case KB_KEY_F15: wrenSetSlotString(state->vm, 1, "f15"); break;
        case KB_KEY_F16: wrenSetSlotString(state->vm, 1, "f16"); break;
        case KB_KEY_F17: wrenSetSlotString(state->vm, 1, "f17"); break;
        case KB_KEY_F18: wrenSetSlotString(state->vm, 1, "f18"); break;
        case KB_KEY_F19: wrenSetSlotString(state->vm, 1, "f19"); break;
        case KB_KEY_F20: wrenSetSlotString(state->vm, 1, "f20"); break;
        case KB_KEY_F21: wrenSetSlotString(state->vm, 1, "f21"); break;
        case KB_KEY_F22: wrenSetSlotString(state->vm, 1, "f22"); break;
        case KB_KEY_F23: wrenSetSlotString(state->vm, 1, "f23"); break;
        case KB_KEY_F24: wrenSetSlotString(state->vm, 1, "f24"); break;
        case KB_KEY_F25: wrenSetSlotString(state->vm, 1, "f25"); break;
        case KB_KEY_KP_0: wrenSetSlotString(state->vm, 1, "kp0"); break;
        case KB_KEY_KP_1: wrenSetSlotString(state->vm, 1, "kp1"); break;
        case KB_KEY_KP_2: wrenSetSlotString(state->vm, 1, "kp2"); break;
        case KB_KEY_KP_3: wrenSetSlotString(state->vm, 1, "kp3"); break;
        case KB_KEY_KP_4: wrenSetSlotString(state->vm, 1, "kp4"); break;
        case KB_KEY_KP_5: wrenSetSlotString(state->vm, 1, "kp5"); break;
        case KB_KEY_KP_6: wrenSetSlotString(state->vm, 1, "kp6"); break;
        case KB_KEY_KP_7: wrenSetSlotString(state->vm, 1, "kp7"); break;
        case KB_KEY_KP_8: wrenSetSlotString(state->vm, 1, "kp8"); break;
        case KB_KEY_KP_9: wrenSetSlotString(state->vm, 1, "kp9"); break;
        case KB_KEY_KP_DECIMAL: wrenSetSlotString(state->vm, 1, "decimal"); break;
        case KB_KEY_KP_DIVIDE: wrenSetSlotString(state->vm, 1, "divide"); break;
        case KB_KEY_KP_MULTIPLY: wrenSetSlotString(state->vm, 1, "multiply"); break;
        case KB_KEY_KP_SUBTRACT: wrenSetSlotString(state->vm, 1, "subtract"); break;
        case KB_KEY_KP_ADD: wrenSetSlotString(state->vm, 1, "add"); break;
        case KB_KEY_KP_ENTER: wrenSetSlotString(state->vm, 1, "enter"); break;
        case KB_KEY_KP_EQUAL: wrenSetSlotString(state->vm, 1, "equal"); break;
        case KB_KEY_LEFT_SHIFT: wrenSetSlotString(state->vm, 1, "left_shift"); break;
        case KB_KEY_LEFT_CONTROL: wrenSetSlotString(state->vm, 1, "left_control"); break;
        case KB_KEY_LEFT_ALT: wrenSetSlotString(state->vm, 1, "left_alt"); break;
        case KB_KEY_LEFT_SUPER: wrenSetSlotString(state->vm, 1, "left_super"); break;
        case KB_KEY_RIGHT_SHIFT: wrenSetSlotString(state->vm, 1, "right_shift"); break;
        case KB_KEY_RIGHT_CONTROL: wrenSetSlotString(state->vm, 1, "right_control"); break;
        case KB_KEY_RIGHT_ALT: wrenSetSlotString(state->vm, 1, "right_alt"); break;
        case KB_KEY_RIGHT_SUPER: wrenSetSlotString(state->vm, 1, "right_super"); break;
        case KB_KEY_MENU: wrenSetSlotString(state->vm, 1, "menu"); break;
        }

        wrenSetSlotBool(state->vm, 2, is_pressed);
        wrenCall(state->vm, state->on_keyboard);
    }
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

static void on_drop(cri_window *window, int count, const char **paths) {
    blink_state *state = (blink_state*)cri_get_user_data(window);

    if (!state->error) {
        wrenSetSlotHandle(state->vm, 0, state->game);
        wrenSetSlotNewList(state->vm, 1);

        for (int i = 0; i < count; i++) {
            wrenSetSlotString(state->vm, 2, paths[i]);
            wrenInsertInList(state->vm, 1, i, 2);
        }

        wrenCall(state->vm, state->on_drop);
    }
}

static void on_audio(float *buffer, int frames, int channels, void *user_data) {
    int16_t *data = (int16_t*)malloc(frames * channels * sizeof(int16_t));
    ba_process(data, frames * channels);

    for (int i = 0; i < frames * channels; i++) {
        buffer[i] = (float)data[i] / 32767.0f;
    }
}

static void *default_font_data;
static int default_font_size;

int main(int argc, char **argv) {
    blink_state state;
    memset(&state, 0, sizeof(blink_state));

    state.argc = argc;
    state.argv = argv;
    state.width = 320;
    state.height = 240;
    state.scale = 2.0f;
    state.clear_color = bg_rgb(0, 0, 0);
    state.font = bg_load_font_mem(default_font_data, default_font_size);
    strcpy(state.title, "Blink");

    map_init(&state.keymap);
    map_set(&state.keymap, "unknown", KB_KEY_UNKNOWN);
    map_set(&state.keymap, "space", KB_KEY_SPACE);
    map_set(&state.keymap, "apostrophe", KB_KEY_APOSTROPHE);
    map_set(&state.keymap, "comma", KB_KEY_COMMA);
    map_set(&state.keymap, "minus", KB_KEY_MINUS);
    map_set(&state.keymap, "period", KB_KEY_PERIOD);
    map_set(&state.keymap, "slash", KB_KEY_SLASH);
    map_set(&state.keymap, "0", KB_KEY_0);
    map_set(&state.keymap, "1", KB_KEY_1);
    map_set(&state.keymap, "2", KB_KEY_2);
    map_set(&state.keymap, "3", KB_KEY_3);
    map_set(&state.keymap, "4", KB_KEY_4);
    map_set(&state.keymap, "5", KB_KEY_5);
    map_set(&state.keymap, "6", KB_KEY_6);
    map_set(&state.keymap, "7", KB_KEY_7);
    map_set(&state.keymap, "8", KB_KEY_8);
    map_set(&state.keymap, "9", KB_KEY_9);
    map_set(&state.keymap, "semicolon", KB_KEY_SEMICOLON);
    map_set(&state.keymap, "equal", KB_KEY_EQUAL);
    map_set(&state.keymap, "a", KB_KEY_A);
    map_set(&state.keymap, "b", KB_KEY_B);
    map_set(&state.keymap, "c", KB_KEY_C);
    map_set(&state.keymap, "d", KB_KEY_D);
    map_set(&state.keymap, "e", KB_KEY_E);
    map_set(&state.keymap, "f", KB_KEY_F);
    map_set(&state.keymap, "g", KB_KEY_G);
    map_set(&state.keymap, "h", KB_KEY_H);
    map_set(&state.keymap, "i", KB_KEY_I);
    map_set(&state.keymap, "j", KB_KEY_J);
    map_set(&state.keymap, "k", KB_KEY_K);
    map_set(&state.keymap, "l", KB_KEY_L);
    map_set(&state.keymap, "m", KB_KEY_M);
    map_set(&state.keymap, "n", KB_KEY_N);
    map_set(&state.keymap, "o", KB_KEY_O);
    map_set(&state.keymap, "p", KB_KEY_P);
    map_set(&state.keymap, "q", KB_KEY_Q);
    map_set(&state.keymap, "r", KB_KEY_R);
    map_set(&state.keymap, "s", KB_KEY_S);
    map_set(&state.keymap, "t", KB_KEY_T);
    map_set(&state.keymap, "u", KB_KEY_U);
    map_set(&state.keymap, "v", KB_KEY_V);
    map_set(&state.keymap, "w", KB_KEY_W);
    map_set(&state.keymap, "x", KB_KEY_X);
    map_set(&state.keymap, "y", KB_KEY_Y);
    map_set(&state.keymap, "z", KB_KEY_Z);
    map_set(&state.keymap, "left_bracket", KB_KEY_LEFT_BRACKET);
    map_set(&state.keymap, "backslash", KB_KEY_BACKSLASH);
    map_set(&state.keymap, "right_bracket", KB_KEY_RIGHT_BRACKET);
    map_set(&state.keymap, "grave_accent", KB_KEY_GRAVE_ACCENT);
    map_set(&state.keymap, "world_1", KB_KEY_WORLD_1);
    map_set(&state.keymap, "world_2", KB_KEY_WORLD_2);
    map_set(&state.keymap, "escape", KB_KEY_ESCAPE);
    map_set(&state.keymap, "enter", KB_KEY_ENTER);
    map_set(&state.keymap, "tab", KB_KEY_TAB);
    map_set(&state.keymap, "backspace", KB_KEY_BACKSPACE);
    map_set(&state.keymap, "insert", KB_KEY_INSERT);
    map_set(&state.keymap, "delete", KB_KEY_DELETE);
    map_set(&state.keymap, "right", KB_KEY_RIGHT);
    map_set(&state.keymap, "left", KB_KEY_LEFT);
    map_set(&state.keymap, "down", KB_KEY_DOWN);
    map_set(&state.keymap, "up", KB_KEY_UP);
    map_set(&state.keymap, "page_up", KB_KEY_PAGE_UP);
    map_set(&state.keymap, "page_down", KB_KEY_PAGE_DOWN);
    map_set(&state.keymap, "home", KB_KEY_HOME);
    map_set(&state.keymap, "end", KB_KEY_END);
    map_set(&state.keymap, "caps_lock", KB_KEY_CAPS_LOCK);
    map_set(&state.keymap, "scroll_lock", KB_KEY_SCROLL_LOCK);
    map_set(&state.keymap, "num_lock", KB_KEY_NUM_LOCK);
    map_set(&state.keymap, "print_screen", KB_KEY_PRINT_SCREEN);
    map_set(&state.keymap, "pause", KB_KEY_PAUSE);
    map_set(&state.keymap, "f1", KB_KEY_F1);
    map_set(&state.keymap, "f2", KB_KEY_F2);
    map_set(&state.keymap, "f3", KB_KEY_F3);
    map_set(&state.keymap, "f4", KB_KEY_F4);
    map_set(&state.keymap, "f5", KB_KEY_F5);
    map_set(&state.keymap, "f6", KB_KEY_F6);
    map_set(&state.keymap, "f7", KB_KEY_F7);
    map_set(&state.keymap, "f8", KB_KEY_F8);
    map_set(&state.keymap, "f9", KB_KEY_F9);
    map_set(&state.keymap, "f10", KB_KEY_F10);
    map_set(&state.keymap, "f11", KB_KEY_F11);
    map_set(&state.keymap, "f12", KB_KEY_F12);
    map_set(&state.keymap, "f13", KB_KEY_F13);
    map_set(&state.keymap, "f14", KB_KEY_F14);
    map_set(&state.keymap, "f15", KB_KEY_F15);
    map_set(&state.keymap, "f16", KB_KEY_F16);
    map_set(&state.keymap, "f17", KB_KEY_F17);
    map_set(&state.keymap, "f18", KB_KEY_F18);
    map_set(&state.keymap, "f19", KB_KEY_F19);
    map_set(&state.keymap, "f20", KB_KEY_F20);
    map_set(&state.keymap, "f21", KB_KEY_F21);
    map_set(&state.keymap, "f22", KB_KEY_F22);
    map_set(&state.keymap, "f23", KB_KEY_F23);
    map_set(&state.keymap, "f24", KB_KEY_F24);
    map_set(&state.keymap, "f25", KB_KEY_F25);
    map_set(&state.keymap, "kp0", KB_KEY_KP_0);
    map_set(&state.keymap, "kp1", KB_KEY_KP_1);
    map_set(&state.keymap, "kp2", KB_KEY_KP_2);
    map_set(&state.keymap, "kp3", KB_KEY_KP_3);
    map_set(&state.keymap, "kp4", KB_KEY_KP_4);
    map_set(&state.keymap, "kp5", KB_KEY_KP_5);
    map_set(&state.keymap, "kp6", KB_KEY_KP_6);
    map_set(&state.keymap, "kp7", KB_KEY_KP_7);
    map_set(&state.keymap, "kp8", KB_KEY_KP_8);
    map_set(&state.keymap, "kp9", KB_KEY_KP_9);
    map_set(&state.keymap, "decimal", KB_KEY_KP_DECIMAL);
    map_set(&state.keymap, "divide", KB_KEY_KP_DIVIDE);
    map_set(&state.keymap, "multiply", KB_KEY_KP_MULTIPLY);
    map_set(&state.keymap, "subtract", KB_KEY_KP_SUBTRACT);
    map_set(&state.keymap, "add", KB_KEY_KP_ADD);
    map_set(&state.keymap, "enter", KB_KEY_KP_ENTER);
    map_set(&state.keymap, "equal", KB_KEY_KP_EQUAL);
    map_set(&state.keymap, "left_shift", KB_KEY_LEFT_SHIFT);
    map_set(&state.keymap, "left_control", KB_KEY_LEFT_CONTROL);
    map_set(&state.keymap, "left_alt", KB_KEY_LEFT_ALT);
    map_set(&state.keymap, "left_super", KB_KEY_LEFT_SUPER);
    map_set(&state.keymap, "right_shift", KB_KEY_RIGHT_SHIFT);
    map_set(&state.keymap, "right_control", KB_KEY_RIGHT_CONTROL);
    map_set(&state.keymap, "right_alt", KB_KEY_RIGHT_ALT);
    map_set(&state.keymap, "right_super", KB_KEY_RIGHT_SUPER);
    map_set(&state.keymap, "menu", KB_KEY_MENU);

    WrenConfiguration config;
    wrenInitConfiguration(&config);
    config.loadModuleFn = wren_load_module;
    config.bindForeignMethodFn = wren_bind_foreign_method;
    config.bindForeignClassFn = wren_bind_foreign_class;
    config.writeFn = wren_write;
    config.errorFn = wren_error;

    WrenVM *vm = wrenNewVM(&config);
    wrenSetUserData(vm, &state);

    state.vm = vm;

    wrenInterpret(vm, "blink", api_source);
    wrenEnsureSlots(vm, 1);
    wrenGetVariable(vm, "blink", "Color", 0);
    state.color_class = wrenGetSlotHandle(vm, 0);
    wrenGetVariable(vm, "blink", "Image", 0);
    state.image_class = wrenGetSlotHandle(vm, 0);

    int flags = 0;

    cri_chdir(argv[1]);
    char *source = cri_read_file("main.wren", NULL);
    if (!source) {
        state.error = true;
        strcpy(state.error_message, "Failed to read source file");
        goto error;
    }

    WrenInterpretResult res = wrenInterpret(vm, argv[1], source);
    free(source);
    if (res != WREN_RESULT_SUCCESS)
        goto error;

    if (!wrenHasVariable(vm, argv[1], "main")) {
        state.error = true;
        strcpy(state.error_message, "Failed to find Game class instance");
        goto error;
    }

    wrenEnsureSlots(vm, 1);
    wrenGetVariable(vm, argv[1], "main", 0);
    if (wrenGetSlotType(vm, 0) != WREN_TYPE_UNKNOWN) {
        state.error = true;
        strcpy(state.error_message, "Game class is of an invalid type");
        goto error;
    }

    state.game = wrenGetSlotHandle(vm, 0);
    state.on_config = wrenMakeCallHandle(vm, "config(_)");
    state.on_init = wrenMakeCallHandle(vm, "init()");
    state.on_update = wrenMakeCallHandle(vm, "update(_)");
    state.on_draw = wrenMakeCallHandle(vm, "draw()");
    state.on_active = wrenMakeCallHandle(vm, "active(_)");
    state.on_resize = wrenMakeCallHandle(vm, "resize(_,_)");
    state.on_close = wrenMakeCallHandle(vm, "close()");
    state.on_keyboard = wrenMakeCallHandle(vm, "keyboard(_,_)");
    state.on_char_input = wrenMakeCallHandle(vm, "input(_)");
    state.on_mouse_button = wrenMakeCallHandle(vm, "mouseButton(_,_)");
    state.on_mouse_move = wrenMakeCallHandle(vm, "mouseMove(_,_)");
    state.on_mouse_scroll = wrenMakeCallHandle(vm, "mouseScroll(_,_)");
    state.on_drop = wrenMakeCallHandle(vm, "drop(_)");

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
    res = wrenCall(vm, state.on_config);
    if (res != WREN_RESULT_SUCCESS)
        goto error;

    wrenEnsureSlots(vm, 4);

#ifdef _WIN32
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
    }
#endif

    wrenSetSlotString(vm, 2, "resizable");
    wrenGetMapValue(vm, 1, 2, 3);
    if (wrenGetSlotType(vm, 3) == WREN_TYPE_BOOL) {
        if (wrenGetSlotBool(vm, 3))
            flags |= FLAG_RESIZABLE;
    }

    wrenSetSlotString(vm, 2, "hideCursor");
    wrenGetMapValue(vm, 1, 2, 3);
    if (wrenGetSlotType(vm, 3) == WREN_TYPE_BOOL) {
        if (wrenGetSlotBool(vm, 3))
            flags |= FLAG_HIDECURSOR;
    }

    wrenSetSlotString(vm, 2, "title");
    wrenGetMapValue(vm, 1, 2, 3);
    if (wrenGetSlotType(vm, 3) == WREN_TYPE_STRING)
        strcpy(state.title, wrenGetSlotString(vm, 3));

    wrenSetSlotString(vm, 2, "targetFps");
    wrenGetMapValue(vm, 1, 2, 3);
    if (wrenGetSlotType(vm, 3) == WREN_TYPE_NUM) {
        int target_fps = (int)wrenGetSlotDouble(vm, 3);
        if (target_fps >= 0)
            cri_set_target_fps(target_fps);
    }

    wrenSetSlotString(vm, 2, "width");
    wrenGetMapValue(vm, 1, 2, 3);
    if (wrenGetSlotType(vm, 3) == WREN_TYPE_NUM) {
        int width = (int)wrenGetSlotDouble(vm, 3);
        if (width > 0)
            state.width = width;
    }

    wrenSetSlotString(vm, 2, "height");
    wrenGetMapValue(vm, 1, 2, 3);
    if (wrenGetSlotType(vm, 3) == WREN_TYPE_NUM) {
        int height = (int)wrenGetSlotDouble(vm, 3);
        if (height > 0)
            state.height = height;
    }

    wrenSetSlotString(vm, 2, "scale");
    wrenGetMapValue(vm, 1, 2, 3);
    if (wrenGetSlotType(vm, 3) == WREN_TYPE_NUM) {
        float scale = (float)wrenGetSlotDouble(vm, 3);
        if (scale >= 1) {
            state.scale = scale;
        }
    }

    naettInit(NULL);
    cri_open_audio(44100, 2, on_audio, NULL);
    ba_init(cri_get_audio_sample_rate());

    wrenSetSlotHandle(vm, 0, state.game);
    wrenCall(vm, state.on_init);

error:
    state.screen = bg_new_image(state.width, state.height);
    state.window = cri_open(state.title, state.width * state.scale, state.height * state.scale, flags);

    cri_set_user_data(state.window, &state);

    cri_set_active_cb(state.window, on_active);
    cri_set_resize_cb(state.window, on_resize);
    cri_set_close_cb(state.window, on_close);
    cri_set_keyboard_cb(state.window, on_keyboard);
    cri_set_char_input_cb(state.window, on_char_input);
    cri_set_mouse_button_cb(state.window, on_mouse_button);
    cri_set_mouse_move_cb(state.window, on_mouse_move);
    cri_set_mouse_scroll_cb(state.window, on_mouse_scroll);
    cri_set_drop_cb(state.window, on_drop);

    cri_timer *timer = cri_timer_create();
    double dt = 0;

    do {
        if (state.error) {
            bg_clear(state.screen, bg_rgb(255, 0, 77));
            bg_print(state.screen, state.font, state.error_message, 10, 10, bg_rgb(255, 241, 232));
        } else {
            cri_timer_now(timer);

            wrenSetSlotHandle(vm, 0, state.game);
            wrenSetSlotDouble(vm, 1, dt);
            res = wrenCall(vm, state.on_update);
            if (res == WREN_RESULT_SUCCESS) {
                bg_clear(state.screen, state.clear_color);

                wrenSetSlotHandle(vm, 0, state.game);
                wrenCall(vm, state.on_draw);
            }
        }

        memcpy(state.prev_keyboard_state, cri_get_keyboard_state(state.window), 512);
        memcpy(state.prev_mouse_state, cri_get_mouse_state(state.window), 8);

        int r = cri_update(state.window, state.screen->pixels, state.screen->w, state.screen->h);
        if (r != 0) break;

        dt = cri_timer_dt(timer);
    } while (cri_wait_sync(state.window));

    cri_close_audio();

#ifdef _WIN32
    if (state.console)
        FreeConsole();
#endif

    map_deinit(&state.keymap);
    bg_destroy_font(state.font);
    bg_destroy_image(state.screen);

    wrenReleaseHandle(vm, state.color_class);
    wrenReleaseHandle(vm, state.image_class);

    if (state.game)
        wrenReleaseHandle(vm, state.game);
    if (state.on_config)
        wrenReleaseHandle(vm, state.on_config);
    if (state.on_init)
        wrenReleaseHandle(vm, state.on_init);
    if (state.on_update)
        wrenReleaseHandle(vm, state.on_update);
    if (state.on_draw)
        wrenReleaseHandle(vm, state.on_draw);
    if (state.on_active)
        wrenReleaseHandle(vm, state.on_active);
    if (state.on_resize)
        wrenReleaseHandle(vm, state.on_resize);
    if (state.on_close)
        wrenReleaseHandle(vm, state.on_close);
    if (state.on_keyboard)
        wrenReleaseHandle(vm, state.on_keyboard);
    if (state.on_char_input)
        wrenReleaseHandle(vm, state.on_char_input);
    if (state.on_mouse_button)
        wrenReleaseHandle(vm, state.on_mouse_button);
    if (state.on_mouse_move)
        wrenReleaseHandle(vm, state.on_mouse_move);
    if (state.on_mouse_scroll)
        wrenReleaseHandle(vm, state.on_mouse_scroll);

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
