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

    pub fn log(self: *Console, message: []const u8) void {
        const copy = self.allocator.alloc(u8, message.len) catch unreachable;
        @memcpy(copy, message);
        self.buffer.append(copy) catch unreachable;
    }

    pub fn render(self: *Console) void {
        if (c.ImGui_Begin("Console", null, 0)) {
            for (self.buffer.items) |item| {
                c.ImGui_Text(item.ptr);
            }
            c.ImGui_End();
        }
    }
};
