const std = @import("std");

const c = @cImport({
    @cInclude("dcimgui.h");
});

// figure out how to share functions which return c types across modules
fn vec2(x: f32, y: f32) c.ImVec2 {
    var v = c.ImVec2{};
    v.x = x;
    v.y = y;
    return v;
}

pub const Console = struct {
    allocator: std.mem.Allocator,
    buffer: std.ArrayList([]const u8),
    input_buffer: [256]u8,
    scroll_to_bottom: bool,

    pub fn init(allocator: std.mem.Allocator) Console {
        return Console{
            .allocator = allocator,
            .buffer = std.ArrayList([]const u8).init(allocator),
            .input_buffer = std.mem.zeroes([256]u8),
            .scroll_to_bottom = false,
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
        self.scroll_to_bottom = true;
    }

    pub fn render(self: *Console) void {
        _ = c.ImGui_Begin("Console", null, 0);

        if (c.ImGui_Button("Clear")) {
            for (self.buffer.items) |item| {
                self.allocator.free(item);
            }
            self.buffer.clearRetainingCapacity();
        }

        c.ImGui_SameLine();

        const avail = c.ImGui_GetContentRegionAvail().x;
        c.ImGui_SetNextItemWidth(avail);

        if (c.ImGui_InputText("##input", &self.input_buffer, self.input_buffer.len, c.ImGuiInputTextFlags_EnterReturnsTrue)) {
            self.log("> {s}\n", .{self.input_buffer});
            self.input_buffer[0] = 0;
            self.scroll_to_bottom = true;
        }

        c.ImGui_Spacing();
        c.ImGui_Separator();
        c.ImGui_Spacing();

        _ = c.ImGui_BeginChild("logs", vec2(0, 0), c.ImGuiChildFlags_Borders, 0);

        var clipper = c.ImGuiListClipper{};
        c.ImGuiListClipper_Begin(&clipper, @as(c_int, @intCast(self.buffer.items.len)), 0);

        while (c.ImGuiListClipper_Step(&clipper)) {
            for (@as(usize, @intCast(clipper.DisplayStart))..@as(usize, @intCast(clipper.DisplayEnd))) |i| {
                const item = self.buffer.items[i];
                c.ImGui_Text(item.ptr);
            }
        }

        if (self.scroll_to_bottom) {
            c.ImGui_SetScrollHereY(1.0);
            self.scroll_to_bottom = false;
        }

        c.ImGuiListClipper_End(&clipper);

        _ = c.ImGui_EndChild();

        c.ImGui_End();
    }
};
