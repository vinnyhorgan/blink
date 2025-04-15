const std = @import("std");
const font = @embedFile("jetbrainsmono.ttf");

const c = @cImport({
    @cInclude("dcimgui.h");
    @cInclude("dcimgui_impl_glfw.h");
    @cInclude("dcimgui_impl_opengl3.h");
    @cInclude("GLFW/glfw3.h");
    @cInclude("float.h");
});

fn errorCallback(errn: c_int, str: [*c]const u8) callconv(.C) void {
    std.log.err("glfw error '{}'': {s}", .{ errn, str });
}

pub fn main() !void {
    _ = c.glfwSetErrorCallback(errorCallback);

    if (c.glfwInit() != c.GLFW_TRUE) {
        return;
    }
    defer c.glfwTerminate();

    c.glfwWindowHint(c.GLFW_CONTEXT_VERSION_MAJOR, 3);
    c.glfwWindowHint(c.GLFW_CONTEXT_VERSION_MINOR, 0);

    const window = c.glfwCreateWindow(960, 540, "blink", null, null);
    if (window == null) {
        return;
    }
    defer c.glfwDestroyWindow(window);

    c.glfwMakeContextCurrent(window);
    c.glfwSwapInterval(1);

    _ = c.ImGui_CreateContext(null);
    defer c.ImGui_DestroyContext(null);

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

    while (c.glfwWindowShouldClose(window) != c.GLFW_TRUE) {
        c.cImGui_ImplOpenGL3_NewFrame();
        c.cImGui_ImplGlfw_NewFrame();
        c.ImGui_NewFrame();

        _ = c.ImGui_DockSpaceOverViewport();

        c.ImGui_ShowDemoWindow(null);

        c.ImGui_Render();

        var width: c_int = 0;
        var height: c_int = 0;
        c.glfwGetFramebufferSize(window, &width, &height);

        c.glViewport(0, 0, width, height);
        c.glClear(c.GL_COLOR_BUFFER_BIT);

        c.cImGui_ImplOpenGL3_RenderDrawData(c.ImGui_GetDrawData());

        c.glfwSwapBuffers(window);
        c.glfwPollEvents();
    }
}
