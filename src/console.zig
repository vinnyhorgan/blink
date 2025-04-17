const std = @import("std");

const c = @cImport({
    @cInclude("dcimgui.h");
});

pub const Console = struct {
    allocator: std.mem.Allocator,
    buffer: std.ArrayList([]const u8),

    pub fn init(allocator: std.mem.Allocator) Console {
        return Console{
            .allocator = allocator,
            .buffer = std.ArrayList([]const u8).init(allocator),
        };
    }

    pub fn deinit(self: *Console) void {
        for (self.buffer.items) |item| {
            self.allocator.free(item);
        }
        self.buffer.deinit();
    }

    pub fn log(self: *Console, comptime fmt: []const u8, args: anytype) void {
        const copy = std.fmt.allocPrint(self.allocator, fmt, args) catch unreachable;
        self.buffer.append(copy) catch unreachable;
    }

    pub fn render(self: *Console) void {
        _ = c.ImGui_Begin("Console", null, 0);
        for (self.buffer.items) |item| {
            c.ImGui_Text(item.ptr);
        }
        c.ImGui_End();
    }
};
