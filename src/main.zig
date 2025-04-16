const std = @import("std");
const font = @embedFile("jetbrainsmono.ttf");

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

fn errorCallback(errn: c_int, str: [*c]const u8) callconv(.C) void {
    std.log.err("glfw error '{}'': {s}", .{ errn, str });
}

fn draw(window: ?*c.GLFWwindow) void {
    c.cImGui_ImplOpenGL3_NewFrame();
    c.cImGui_ImplGlfw_NewFrame();
    c.ImGui_NewFrame();

    _ = c.ImGui_DockSpaceOverViewport();

    c.ImGui_ShowDemoWindow(null);

    c.ImGui_Render();

    c.glClear(c.GL_COLOR_BUFFER_BIT);

    c.cImGui_ImplOpenGL3_RenderDrawData(c.ImGui_GetDrawData());

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

pub fn main() !void {
    _ = c.glfwSetErrorCallback(errorCallback);

    if (c.glfwInit() != c.GLFW_TRUE) {
        return;
    }
    defer c.glfwTerminate();

    c.glfwWindowHint(c.GLFW_CONTEXT_VERSION_MAJOR, 3);
    c.glfwWindowHint(c.GLFW_CONTEXT_VERSION_MINOR, 0);

    c.glfwWindowHint(c.GLFW_VISIBLE, c.GLFW_FALSE);

    const window = c.glfwCreateWindow(960, 540, "blink", null, null);
    if (window == null) {
        return;
    }
    defer c.glfwDestroyWindow(window);

    const hwnd = c.glfwGetWin32Window(window);

    const dark: c.BOOL = c.TRUE;
    _ = c.DwmSetWindowAttribute(hwnd, 20, &dark, 4);

    c.glfwMakeContextCurrent(window);
    c.glfwSwapInterval(1);

    _ = c.glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

    _ = c.ImGui_CreateContext(null);
    defer c.ImGui_DestroyContext(null);

    const style = c.ImGui_GetStyle();

    style.*.Colors[c.ImGuiCol_Text] = color(0.0, 0.0, 0.0, 1.0);

    style.*.Colors[c.ImGuiCol_Text] = color(1.00, 1.00, 1.00, 1.00);
    style.*.Colors[c.ImGuiCol_TextDisabled] = color(0.50, 0.50, 0.50, 1.00);
    style.*.Colors[c.ImGuiCol_WindowBg] = color(0.06, 0.06, 0.06, 0.94);
    style.*.Colors[c.ImGuiCol_ChildBg] = color(1.00, 1.00, 1.00, 0.00);
    style.*.Colors[c.ImGuiCol_PopupBg] = color(0.08, 0.08, 0.08, 0.94);
    style.*.Colors[c.ImGuiCol_Border] = color(0.43, 0.43, 0.50, 0.50);
    style.*.Colors[c.ImGuiCol_BorderShadow] = color(0.00, 0.00, 0.00, 0.00);
    style.*.Colors[c.ImGuiCol_FrameBg] = color(0.20, 0.21, 0.22, 0.54);
    style.*.Colors[c.ImGuiCol_FrameBgHovered] = color(0.40, 0.40, 0.40, 0.40);
    style.*.Colors[c.ImGuiCol_FrameBgActive] = color(0.18, 0.18, 0.18, 0.67);
    style.*.Colors[c.ImGuiCol_TitleBg] = color(0.04, 0.04, 0.04, 1.00);
    style.*.Colors[c.ImGuiCol_TitleBgActive] = color(0.29, 0.29, 0.29, 1.00);
    style.*.Colors[c.ImGuiCol_TitleBgCollapsed] = color(0.00, 0.00, 0.00, 0.51);
    style.*.Colors[c.ImGuiCol_MenuBarBg] = color(0.14, 0.14, 0.14, 1.00);
    style.*.Colors[c.ImGuiCol_ScrollbarBg] = color(0.02, 0.02, 0.02, 0.53);
    style.*.Colors[c.ImGuiCol_ScrollbarGrab] = color(0.31, 0.31, 0.31, 1.00);
    style.*.Colors[c.ImGuiCol_ScrollbarGrabHovered] = color(0.41, 0.41, 0.41, 1.00);
    style.*.Colors[c.ImGuiCol_ScrollbarGrabActive] = color(0.51, 0.51, 0.51, 1.00);
    style.*.Colors[c.ImGuiCol_CheckMark] = color(0.94, 0.94, 0.94, 1.00);
    style.*.Colors[c.ImGuiCol_SliderGrab] = color(0.51, 0.51, 0.51, 1.00);
    style.*.Colors[c.ImGuiCol_SliderGrabActive] = color(0.86, 0.86, 0.86, 1.00);
    style.*.Colors[c.ImGuiCol_Button] = color(0.44, 0.44, 0.44, 0.40);
    style.*.Colors[c.ImGuiCol_ButtonHovered] = color(0.46, 0.47, 0.48, 1.00);
    style.*.Colors[c.ImGuiCol_ButtonActive] = color(0.42, 0.42, 0.42, 1.00);
    style.*.Colors[c.ImGuiCol_Header] = color(0.70, 0.70, 0.70, 0.31);
    style.*.Colors[c.ImGuiCol_HeaderHovered] = color(0.70, 0.70, 0.70, 0.80);
    style.*.Colors[c.ImGuiCol_HeaderActive] = color(0.48, 0.50, 0.52, 1.00);
    style.*.Colors[c.ImGuiCol_Separator] = color(0.43, 0.43, 0.50, 0.50);
    style.*.Colors[c.ImGuiCol_SeparatorHovered] = color(0.72, 0.72, 0.72, 0.78);
    style.*.Colors[c.ImGuiCol_SeparatorActive] = color(0.51, 0.51, 0.51, 1.00);
    style.*.Colors[c.ImGuiCol_ResizeGrip] = color(0.91, 0.91, 0.91, 0.25);
    style.*.Colors[c.ImGuiCol_ResizeGripHovered] = color(0.81, 0.81, 0.81, 0.67);
    style.*.Colors[c.ImGuiCol_ResizeGripActive] = color(0.46, 0.46, 0.46, 0.95);
    style.*.Colors[c.ImGuiCol_PlotLines] = color(0.61, 0.61, 0.61, 1.00);
    style.*.Colors[c.ImGuiCol_PlotLinesHovered] = color(1.00, 0.43, 0.35, 1.00);
    style.*.Colors[c.ImGuiCol_PlotHistogram] = color(0.73, 0.60, 0.15, 1.00);
    style.*.Colors[c.ImGuiCol_PlotHistogramHovered] = color(1.00, 0.60, 0.00, 1.00);
    style.*.Colors[c.ImGuiCol_TextSelectedBg] = color(0.87, 0.87, 0.87, 0.35);
    style.*.Colors[c.ImGuiCol_DragDropTarget] = color(1.00, 1.00, 0.00, 0.90);
    style.*.Colors[c.ImGuiCol_NavHighlight] = color(0.60, 0.60, 0.60, 1.00);
    style.*.Colors[c.ImGuiCol_NavWindowingHighlight] = color(1.00, 1.00, 1.00, 0.70);

    const io = c.ImGui_GetIO();
    io.*.ConfigFlags |= c.ImGuiConfigFlags_DockingEnable;

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
    var vm = Vm{};
    vm.init();

    // init finished show window and start main loop
    c.glfwShowWindow(window);

    while (c.glfwWindowShouldClose(window) != c.GLFW_TRUE) {
        draw(window);

        c.glfwPollEvents();
    }
}
