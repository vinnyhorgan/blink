const std = @import("std");
const font = @embedFile("assets/jetbrainsmono.ttf");
const icon = @embedFile("assets/blink.png");

const Console = @import("console.zig").Console;
const Vm = @import("vm.zig").Vm;

const Reg = @import("vm.zig").Reg;
const Flag = @import("vm.zig").Flag;

const c = @cImport({
    @cInclude("dcimgui.h");
    @cInclude("dcimgui_impl_glfw.h");
    @cInclude("dcimgui_impl_opengl3.h");
    @cInclude("GLFW/glfw3.h");
    @cInclude("float.h");

    @cDefine("GLFW_EXPOSE_NATIVE_WIN32", {});
    @cInclude("GLFW/glfw3native.h");

    @cInclude("stb_image.h");

    // win specific
    @cInclude("dwmapi.h");
});

var global_console: Console = undefined;
var io: *c.ImGuiIO = undefined;
var vm: Vm = undefined;
var use_16_cols: bool = false;
var show_about = false;
var icon_tex: c.GLuint = 0;
var global_jump_to_pc: bool = false;

var running = false;
var cycles_per_frame: c_int = 1;

// palette (6 colors)
var white: c.ImVec4 = undefined;
var yellow: c.ImVec4 = undefined;
var light_pink: c.ImVec4 = undefined;
var dark_pink: c.ImVec4 = undefined;
var light_purple: c.ImVec4 = undefined;
var dark_purple: c.ImVec4 = undefined;

fn errorCallback(errn: c_int, str: [*c]const u8) callconv(.C) void {
    std.log.err("glfw error '{}'': {s}", .{ errn, str });
}

fn draw(window: ?*c.GLFWwindow) void {
    c.cImGui_ImplOpenGL3_NewFrame();
    c.cImGui_ImplGlfw_NewFrame();
    c.ImGui_NewFrame();

    _ = c.ImGui_DockSpaceOverViewport();

    _ = c.ImGui_BeginMainMenuBar();

    if (c.ImGui_BeginMenu("File")) {
        c.ImGui_EndMenu();
    }

    if (c.ImGui_BeginMenu("Edit")) {
        c.ImGui_EndMenu();
    }

    if (c.ImGui_BeginMenu("View")) {
        c.ImGui_EndMenu();
    }

    if (c.ImGui_BeginMenu("Help")) {
        if (c.ImGui_MenuItem("About")) {
            show_about = true;
        }

        c.ImGui_EndMenu();
    }

    c.ImGui_EndMainMenuBar();

    //c.ImGui_ShowDemoWindow(null);

    global_console.render();

    // registers
    _ = c.ImGui_Begin("VM", null, 0);
    c.ImGui_Text("Registers");
    c.ImGui_Spacing();
    c.ImGui_Separator();
    c.ImGui_Spacing();
    c.ImGui_Text("R0: 0x%04X", vm.reg[Reg.r0.val()]);
    c.ImGui_SameLine();
    c.ImGui_Text("R1: 0x%04X", vm.reg[Reg.r1.val()]);
    c.ImGui_Text("R2: 0x%04X", vm.reg[Reg.r2.val()]);
    c.ImGui_SameLine();
    c.ImGui_Text("R3: 0x%04X", vm.reg[Reg.r3.val()]);
    c.ImGui_Text("R4: 0x%04X", vm.reg[Reg.r4.val()]);
    c.ImGui_SameLine();
    c.ImGui_Text("R5: 0x%04X", vm.reg[Reg.r5.val()]);
    c.ImGui_Text("R6: 0x%04X", vm.reg[Reg.r6.val()]);
    c.ImGui_SameLine();
    c.ImGui_Text("R7: 0x%04X", vm.reg[Reg.r7.val()]);

    c.ImGui_Spacing();
    c.ImGui_Separator();
    c.ImGui_Spacing();

    c.ImGui_Text("PC: 0x%04X", vm.reg[Reg.pc.val()]);
    c.ImGui_SameLine();

    if (vm.reg[Reg.cond.val()] & Flag.pos.val() != 0) {
        c.ImGui_Text("COND: positive");
    } else if (vm.reg[Reg.cond.val()] & Flag.zero.val() != 0) {
        c.ImGui_Text("COND: zero");
    } else if (vm.reg[Reg.cond.val()] & Flag.neg.val() != 0) {
        c.ImGui_Text("COND: negative");
    } else {
        c.ImGui_Text("COND: ???");
    }

    c.ImGui_Spacing();
    c.ImGui_Separator();
    c.ImGui_Spacing();

    const size = c.ImGui_GetContentRegionAvail().x;
    if (c.ImGui_ButtonEx("Step", vec2(size, 0))) {
        _ = vm.runCycle(true);
    }

    if (running) {
        c.ImGui_PushStyleColorImVec4(c.ImGuiCol_Button, color(0.93, 0.26, 0.4, 0.5));
        c.ImGui_PushStyleColorImVec4(c.ImGuiCol_ButtonHovered, color(0.93, 0.26, 0.4, 1.0));
        c.ImGui_PushStyleColorImVec4(c.ImGuiCol_Text, color(0.0, 0.0, 0.0, 1.0));
        if (c.ImGui_ButtonEx("Stop", vec2(size, 0))) {
            running = false;
        }
        c.ImGui_PopStyleColor();
        c.ImGui_PopStyleColor();
        c.ImGui_PopStyleColor();
    } else {
        c.ImGui_PushStyleColorImVec4(c.ImGuiCol_Button, color(0.95, 0.83, 0.67, 0.5));
        c.ImGui_PushStyleColorImVec4(c.ImGuiCol_ButtonHovered, yellow);
        c.ImGui_PushStyleColorImVec4(c.ImGuiCol_Text, color(0.0, 0.0, 0.0, 1.0));
        if (c.ImGui_ButtonEx("Run", vec2(size, 0))) {
            running = true;
        }
        c.ImGui_PopStyleColor();
        c.ImGui_PopStyleColor();
        c.ImGui_PopStyleColor();
    }

    _ = c.ImGui_SliderInt("Speed (CPF)", &cycles_per_frame, 1, 50);

    c.ImGui_End();

    // mem viewer
    _ = c.ImGui_Begin("Memory", null, 0);

    _ = c.ImGui_Checkbox("16 Columns", &use_16_cols);

    c.ImGui_SameLine();

    var should_jump = false;
    if (c.ImGui_Button("Jump to PC")) {
        should_jump = true;
    }

    c.ImGui_Spacing();
    c.ImGui_Separator();
    c.ImGui_Spacing();

    _ = c.ImGui_BeginChild("mem", vec2(0, 0), c.ImGuiChildFlags_Borders, c.ImGuiWindowFlags_HorizontalScrollbar);

    const cols: u32 = if (use_16_cols) 16 else 8;

    const pc_index = vm.reg[Reg.pc.val()];
    const pc_row = pc_index / cols;

    if (should_jump or global_jump_to_pc) {
        const line_height = c.ImGui_GetTextLineHeightWithSpacing();
        c.ImGui_SetScrollY(@as(f32, @floatFromInt(pc_row)) * line_height);

        global_jump_to_pc = false;
    }

    const rows = (vm.mem.len + cols - 1) / cols;
    var clipper = c.ImGuiListClipper{};
    c.ImGuiListClipper_Begin(&clipper, @as(c_int, @intCast(rows)), -1.0);

    while (c.ImGuiListClipper_Step(&clipper)) {
        for (@as(usize, @intCast(clipper.DisplayStart))..@as(usize, @intCast(clipper.DisplayEnd))) |row| {
            const base = row * cols;

            if (base >= vm.mem.len) {
                break;
            }

            c.ImGui_Text("0x%04X: ", base);
            c.ImGui_SameLine();

            for (0..cols) |col| {
                const i = base + col;
                if (i >= vm.mem.len) {
                    break;
                }

                const val = vm.mem[i];
                const is_pc = i == vm.reg[Reg.pc.val()];

                if (is_pc) {
                    c.ImGui_PushStyleColorImVec4(c.ImGuiCol_Text, color(0.0, 1.0, 0.0, 1.0));
                } else if (val == 0) {
                    c.ImGui_PushStyleColorImVec4(c.ImGuiCol_Text, color(0.5, 0.5, 0.5, 1.0));
                }

                c.ImGui_Text("%04X", val);

                if (is_pc or val == 0) {
                    c.ImGui_PopStyleColor();
                }

                if (col < cols - 1) {
                    c.ImGui_SameLine();
                }
            }
        }
    }

    c.ImGuiListClipper_End(&clipper);

    _ = c.ImGui_EndChild();

    c.ImGui_End();

    // about window

    if (show_about) {
        c.ImGui_OpenPopup("About", 0);
    }

    if (c.ImGui_BeginPopupModal("About", &show_about, c.ImGuiWindowFlags_AlwaysAutoResize)) {
        const window_size = c.ImGui_GetWindowSize().x;
        const image_x = (window_size - 128) / 2.0;
        c.ImGui_SetCursorPosX(image_x);
        c.ImGui_Image(icon_tex, vec2(128, 128));

        c.ImGui_Spacing();

        const text = "Blink IDE alpha build";
        c.ImGui_SetCursorPosX((window_size - c.ImGui_CalcTextSize(text).x) / 2.0);
        c.ImGui_Text(text);

        const text2 = "Made by Vinny Horgan with <3";
        c.ImGui_SetCursorPosX((window_size - c.ImGui_CalcTextSize(text2).x) / 2.0);
        c.ImGui_Text(text2);

        c.ImGui_Spacing();

        const link = "https://github.com/vinnyhorgan/blink";
        c.ImGui_SetCursorPosX((window_size - c.ImGui_CalcTextSize(link).x) / 2.0);

        c.ImGui_PushStyleColorImVec4(c.ImGuiCol_Text, light_pink);

        if (c.ImGui_Selectable(link)) {
            _ = c.system("explorer \"https://github.com/vinnyhorgan/blink\"");
        }

        c.ImGui_PopStyleColor();

        c.ImGui_Spacing();
        c.ImGui_Separator();
        c.ImGui_Spacing();

        const button_size = c.ImGui_GetContentRegionAvail().x / 2.0;
        c.ImGui_SetCursorPosX((window_size - button_size) / 2.0);
        if (c.ImGui_ButtonEx("Close", vec2(button_size, 0))) {
            show_about = false;
        }

        c.ImGui_EndPopup();
    }

    c.ImGui_Render();

    c.glClear(c.GL_COLOR_BUFFER_BIT);

    c.cImGui_ImplOpenGL3_RenderDrawData(c.ImGui_GetDrawData());

    if ((io.ConfigFlags & c.ImGuiConfigFlags_ViewportsEnable) != 0) {
        const backup = c.glfwGetCurrentContext();
        c.ImGui_UpdatePlatformWindows();
        c.ImGui_RenderPlatformWindowsDefault();
        c.glfwMakeContextCurrent(backup);
    }

    c.glfwSwapBuffers(window);
}

fn framebufferSizeCallback(window: ?*c.GLFWwindow, width: c_int, height: c_int) callconv(.C) void {
    c.glViewport(0, 0, width, height);
    draw(window);
}

fn color(r: f32, g: f32, b: f32, a: f32) c.ImVec4 {
    var v = c.ImVec4{};
    v.x = r;
    v.y = g;
    v.z = b;
    v.w = a;
    return v;
}

fn colorFromHex(hex: []const u8, a: f32) !c.ImVec4 {
    const r = try std.fmt.parseInt(u8, hex[1..3], 16);
    const g = try std.fmt.parseInt(u8, hex[3..5], 16);
    const b = try std.fmt.parseInt(u8, hex[5..7], 16);

    return color(@as(f32, @floatFromInt(r)) / 255.0, @as(f32, @floatFromInt(g)) / 255.0, @as(f32, @floatFromInt(b)) / 255.0, a);
}

fn vec2(x: f32, y: f32) c.ImVec2 {
    var v = c.ImVec2{};
    v.x = x;
    v.y = y;
    return v;
}

pub fn main() !void {
    _ = c.glfwSetErrorCallback(errorCallback);

    if (c.glfwInit() != c.GLFW_TRUE) {
        return;
    }
    defer c.glfwTerminate();

    c.glfwWindowHint(c.GLFW_CONTEXT_VERSION_MAJOR, 3);
    c.glfwWindowHint(c.GLFW_CONTEXT_VERSION_MINOR, 0);

    c.glfwWindowHint(c.GLFW_VISIBLE, c.GLFW_FALSE);

    const window = c.glfwCreateWindow(1280, 720, "Blink IDE - by Vinny Horgan - Alpha build", null, null);
    if (window == null) {
        return;
    }
    defer c.glfwDestroyWindow(window);

    const hwnd = c.glfwGetWin32Window(window);

    const dark: c.BOOL = c.TRUE;
    _ = c.DwmSetWindowAttribute(hwnd, 20, &dark, 4);

    c.glfwMakeContextCurrent(window);
    c.glfwSwapInterval(0);

    _ = c.glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

    _ = c.ImGui_CreateContext(null);
    defer c.ImGui_DestroyContext(null);

    const style = c.ImGui_GetStyle();

    // final style
    style.*.WindowMenuButtonPosition = c.ImGuiDir_None;
    style.*.FramePadding = vec2(6.0, 6.0);
    style.*.ScrollbarRounding = 0.0;
    style.*.TabRounding = 0.0;
    style.*.TabBarBorderSize = 0.0;
    style.*.ChildRounding = 4.0;
    style.*.FrameRounding = 4.0;
    style.*.PopupRounding = 4.0;
    style.*.ScrollbarRounding = 4.0;
    style.*.GrabRounding = 2.0;

    // palette (6 colors)
    white = try colorFromHex("#fbf5ef", 1.0);
    yellow = try colorFromHex("#f2d3ab", 1.0);
    light_pink = try colorFromHex("#c69fa5", 1.0);
    dark_pink = try colorFromHex("#8b6d9c", 1.0);
    light_purple = try colorFromHex("#494d7e", 1.0);
    dark_purple = try colorFromHex("#272744", 1.0);

    _ = light_pink;

    style.*.Colors[c.ImGuiCol_ResizeGrip] = color(0.00, 0.00, 0.00, 0.00);

    style.*.Colors[c.ImGuiCol_Text] = white;
    style.*.Colors[c.ImGuiCol_WindowBg] = dark_purple;
    style.*.Colors[c.ImGuiCol_TitleBg] = light_purple;
    style.*.Colors[c.ImGuiCol_TabDimmedSelected] = try colorFromHex("#272744", 0.5);
    style.*.Colors[c.ImGuiCol_TabHovered] = dark_purple;
    style.*.Colors[c.ImGuiCol_MenuBarBg] = dark_purple;
    style.*.Colors[c.ImGuiCol_DockingEmptyBg] = dark_pink;
    style.*.Colors[c.ImGuiCol_DockingPreview] = dark_purple;
    style.*.Colors[c.ImGuiCol_TitleBgActive] = light_purple;
    style.*.Colors[c.ImGuiCol_TabSelected] = dark_purple;
    style.*.Colors[c.ImGuiCol_TabSelectedOverline] = dark_pink;
    style.*.Colors[c.ImGuiCol_Header] = try colorFromHex("#494d7e", 0.5);
    style.*.Colors[c.ImGuiCol_HeaderHovered] = light_purple;
    style.*.Colors[c.ImGuiCol_Button] = try colorFromHex("#494d7e", 0.5);
    style.*.Colors[c.ImGuiCol_ButtonHovered] = light_purple;
    style.*.Colors[c.ImGuiCol_ButtonActive] = dark_pink;
    style.*.Colors[c.ImGuiCol_SliderGrab] = light_purple;
    style.*.Colors[c.ImGuiCol_SliderGrabActive] = dark_pink;
    style.*.Colors[c.ImGuiCol_FrameBg] = try colorFromHex("#494d7e", 0.5);
    style.*.Colors[c.ImGuiCol_FrameBgHovered] = try colorFromHex("#494d7e", 0.5);
    style.*.Colors[c.ImGuiCol_FrameBgActive] = try colorFromHex("#494d7e", 0.5);
    style.*.Colors[c.ImGuiCol_CheckMark] = white;
    style.*.Colors[c.ImGuiCol_SeparatorHovered] = yellow;
    style.*.Colors[c.ImGuiCol_SeparatorActive] = yellow;
    style.*.Colors[c.ImGuiCol_ResizeGripHovered] = yellow;
    style.*.Colors[c.ImGuiCol_ResizeGripActive] = yellow;

    io = c.ImGui_GetIO();
    io.*.ConfigFlags |= c.ImGuiConfigFlags_DockingEnable;
    io.*.ConfigFlags |= c.ImGuiConfigFlags_ViewportsEnable;

    io.*.ConfigWindowsMoveFromTitleBarOnly = true;
    io.*.ConfigDockingAlwaysTabBar = true;

    var font_cfg = c.ImFontConfig{
        .FontDataOwnedByAtlas = false,
        .GlyphMaxAdvanceX = c.FLT_MAX,
        .RasterizerMultiply = 1.0,
        .RasterizerDensity = 1.0,
        .OversampleH = 2,
        .OversampleV = 2,
    };

    _ = c.ImFontAtlas_AddFontFromMemoryTTF(
        io.*.Fonts,
        @constCast(font),
        font.len,
        18.0,
        &font_cfg,
        null,
    );

    _ = c.cImGui_ImplGlfw_InitForOpenGL(window, true);
    defer c.cImGui_ImplGlfw_Shutdown();

    _ = c.cImGui_ImplOpenGL3_InitEx("#version 130");
    defer c.cImGui_ImplOpenGL3_Shutdown();

    // setup vm
    const allocator = std.heap.page_allocator;
    global_console = Console.init(allocator);
    defer global_console.deinit();

    vm = Vm.init(&global_console);

    global_jump_to_pc = true;

    // load icon

    var width: c_int = 0;
    var height: c_int = 0;
    const image_data = c.stbi_load_from_memory(icon, icon.len, &width, &height, null, 4);
    if (image_data == null) {
        return;
    }

    defer c.stbi_image_free(image_data);

    c.glGenTextures(1, &icon_tex);
    c.glBindTexture(c.GL_TEXTURE_2D, icon_tex);

    c.glTexParameteri(c.GL_TEXTURE_2D, c.GL_TEXTURE_MIN_FILTER, c.GL_LINEAR);
    c.glTexParameteri(c.GL_TEXTURE_2D, c.GL_TEXTURE_MAG_FILTER, c.GL_LINEAR);

    c.glTexImage2D(c.GL_TEXTURE_2D, 0, c.GL_RGBA, width, height, 0, c.GL_RGBA, c.GL_UNSIGNED_BYTE, image_data);

    // init finished show window and start main loop
    c.glfwShowWindow(window);

    var current_time: f64 = undefined;
    var previous_time: f64 = undefined;
    var update_time: f64 = undefined;
    var draw_time: f64 = undefined;
    var frame_time: f64 = undefined;

    const target_time: f64 = 1.0 / 60.0;

    _ = c.timeBeginPeriod(1);
    defer _ = c.timeEndPeriod(1);

    previous_time = c.glfwGetTime();

    while (c.glfwWindowShouldClose(window) != c.GLFW_TRUE) {
        if (running) {
            for (0..@as(usize, @intCast(cycles_per_frame))) |_| {
                _ = vm.runCycle(false);
            }
        }

        current_time = c.glfwGetTime();
        update_time = current_time - previous_time;
        previous_time = current_time;

        draw(window);

        current_time = c.glfwGetTime();
        draw_time = current_time - previous_time;
        previous_time = current_time;

        frame_time = update_time + draw_time;

        if (frame_time < target_time) {
            const porcodio = (target_time - frame_time) * std.time.ns_per_s;

            std.time.sleep(@intFromFloat(porcodio));

            current_time = c.glfwGetTime();
            const wait_time = current_time - previous_time;
            previous_time = current_time;

            frame_time += wait_time;
        }

        if (c.glfwGetWindowAttrib(window, c.GLFW_ICONIFIED) == c.GLFW_TRUE) {
            c.glfwWaitEvents();
            previous_time = c.glfwGetTime();
        } else {
            c.glfwPollEvents();
        }
    }
}
