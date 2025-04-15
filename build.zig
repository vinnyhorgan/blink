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

    b.installArtifact(exe);

    exe.addCSourceFiles(.{
        .files = &[_][]const u8{
            "imgui/dcimgui_internal.cpp",
            "imgui/dcimgui.cpp",
            "imgui/imgui_demo.cpp",
            "imgui/imgui_draw.cpp",
            "imgui/imgui_tables.cpp",
            "imgui/imgui_widgets.cpp",
            "imgui/imgui.cpp",
            "imgui/backends/dcimgui_impl_glfw.cpp",
            "imgui/backends/dcimgui_impl_opengl3.cpp",
            "imgui/backends/imgui_impl_glfw.cpp",
            "imgui/backends/imgui_impl_opengl3.cpp",
        },
    });

    exe.addIncludePath(b.path("imgui"));
    exe.addIncludePath(b.path("imgui/backends"));
    exe.linkSystemLibrary("glfw");
    exe.linkSystemLibrary("GL");

    const run_cmd = b.addRunArtifact(exe);

    run_cmd.step.dependOn(b.getInstallStep());

    if (b.args) |args| {
        run_cmd.addArgs(args);
    }

    const run_step = b.step("run", "run blink");
    run_step.dependOn(&run_cmd.step);
}
