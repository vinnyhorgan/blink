const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    const exe_mod = b.createModule(.{
        .root_source_file = b.path("src/main.zig"),
        .target = target,
        .optimize = optimize,
        .link_libc = true,
        .link_libcpp = true,
    });

    const exe = b.addExecutable(.{
        .name = "blink",
        .root_module = exe_mod,
    });

    exe.addCSourceFiles(.{
        .files = &[_][]const u8{
            // common
            "vendor/glfw/src/context.c",
            "vendor/glfw/src/init.c",
            "vendor/glfw/src/input.c",
            "vendor/glfw/src/monitor.c",
            "vendor/glfw/src/platform.c",
            "vendor/glfw/src/vulkan.c",
            "vendor/glfw/src/window.c",
            "vendor/glfw/src/egl_context.c",
            "vendor/glfw/src/osmesa_context.c",
            "vendor/glfw/src/null_init.c",
            "vendor/glfw/src/null_monitor.c",
            "vendor/glfw/src/null_window.c",
            "vendor/glfw/src/null_joystick.c",

            "vendor/glfw/src/win32_module.c",
            "vendor/glfw/src/win32_time.c",
            "vendor/glfw/src/win32_thread.c",

            "vendor/glfw/src/win32_init.c",
            "vendor/glfw/src/win32_joystick.c",
            "vendor/glfw/src/win32_monitor.c",
            "vendor/glfw/src/win32_window.c",
            "vendor/glfw/src/wgl_context.c",

            // linux specific right now
            //"vendor/glfw/src/posix_module.c",
            //"vendor/glfw/src/posix_time.c",
            //"vendor/glfw/src/posix_thread.c",
            //"vendor/glfw/src/linux_joystick.c",
            //"vendor/glfw/src/posix_poll.c",

            //"vendor/glfw/src/wl_init.c",
            //"vendor/glfw/src/wl_monitor.c",
            //"vendor/glfw/src/wl_window.c",
        },
        .flags = &[_][]const u8{
            "-D_GLFW_WIN32",

            //"-D_GLFW_WAYLAND",
        },
    });

    exe.addCSourceFiles(.{
        .files = &[_][]const u8{
            "vendor/imgui/dcimgui_internal.cpp",
            "vendor/imgui/dcimgui.cpp",
            "vendor/imgui/imgui_demo.cpp",
            "vendor/imgui/imgui_draw.cpp",
            "vendor/imgui/imgui_tables.cpp",
            "vendor/imgui/imgui_widgets.cpp",
            "vendor/imgui/imgui.cpp",
            "vendor/imgui/backends/dcimgui_impl_glfw.cpp",
            "vendor/imgui/backends/dcimgui_impl_opengl3.cpp",
            "vendor/imgui/backends/imgui_impl_glfw.cpp",
            "vendor/imgui/backends/imgui_impl_opengl3.cpp",
        },
    });

    exe.addIncludePath(b.path("vendor/glfw/include"));
    exe.addIncludePath(b.path("vendor/glfw/wl"));
    exe.addIncludePath(b.path("vendor/imgui"));
    exe.addIncludePath(b.path("vendor/imgui/backends"));

    exe.linkSystemLibrary("gdi32");
    exe.linkSystemLibrary("opengl32");
    exe.linkSystemLibrary("dwmapi");

    //exe.linkSystemLibrary("GL");

    exe.subsystem = .Windows;

    b.installArtifact(exe);

    const run_cmd = b.addRunArtifact(exe);

    run_cmd.step.dependOn(b.getInstallStep());

    if (b.args) |args| {
        run_cmd.addArgs(args);
    }

    const run_step = b.step("run", "run blink");
    run_step.dependOn(&run_cmd.step);
}
