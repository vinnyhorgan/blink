const std = @import("std");
const font = @embedFile("assets/jetbrainsmono.ttf");

const Console = @import("console.zig").Console;
const Vm = @import("vm.zig").Vm;

const c = @cImport({
    @cInclude("dcimgui.h");
    @cInclude("dcimgui_impl_glfw.h");
    @cInclude("dcimgui_impl_opengl3.h");
    @cInclude("GLFW/glfw3.h");
    @cInclude("float.h");

    @cDefine("GLFW_EXPOSE_NATIVE_WIN32", {});
    @cInclude("GLFW/glfw3native.h");

    // win specific
    @cInclude("dwmapi.h");
});

var global_console: Console = undefined;
var io: *c.ImGuiIO = undefined;

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
        c.ImGui_EndMenu();
    }

    c.ImGui_EndMainMenuBar();

    c.ImGui_ShowDemoWindow(null);

    global_console.render();

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

    const window = c.glfwCreateWindow(1280, 720, "blink", null, null);
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

    style.*.Colors[c.ImGuiCol_ResizeGrip] = color(0.00, 0.00, 0.00, 0.00);

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

    var vm = Vm.init(&global_console);

    vm.writeMem(0x3000, 0xFFFF);

    _ = vm.runCycle();

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
