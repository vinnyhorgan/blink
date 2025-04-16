const std = @import("std");

pub const Op = enum(u8) {
    BR = 0, // branch
    ADD, // add
    LD, // load
    ST, // store
    JSR, // jump register
    AND, // bitwise and
    LDR, // load register
    STR, // store register
    RTI, // unused
    NOT, // bitwise not
    LDI, // load indirect
    STI, // store indirect
    JMP, // jump
    RES, // reserved (unused)
    LEA, // load effective address
    TRAP, // execute trap

    pub fn val(self: Op) u16 {
        return @intFromEnum(self);
    }

    pub fn print(self: Op) void {
        switch (self) {
            Op.BR => std.debug.print("BR ({X})\n", .{self.val()}),
            Op.ADD => std.debug.print("ADD ({X})\n", .{self.val()}),
            Op.LD => std.debug.print("LD ({X})\n", .{self.val()}),
            Op.ST => std.debug.print("ST ({X})\n", .{self.val()}),
            Op.JSR => std.debug.print("JSR ({X})\n", .{self.val()}),
            Op.AND => std.debug.print("AND ({X})\n", .{self.val()}),
            Op.LDR => std.debug.print("LDR ({X})\n", .{self.val()}),
            Op.STR => std.debug.print("STR ({X})\n", .{self.val()}),
            Op.RTI => std.debug.print("RTI ({X})\n", .{self.val()}),
            Op.NOT => std.debug.print("NOT ({X})\n", .{self.val()}),
            Op.LDI => std.debug.print("LDI ({X})\n", .{self.val()}),
            Op.STI => std.debug.print("STI ({X})\n", .{self.val()}),
            Op.JMP => std.debug.print("JMP ({X})\n", .{self.val()}),
            Op.RES => std.debug.print("RES ({X})\n", .{self.val()}),
            Op.LEA => std.debug.print("LEA ({X})\n", .{self.val()}),
            Op.TRAP => std.debug.print("TRAP ({X})\n", .{self.val()}),
            else => std.debug.print("unknown ({X})\n"),
        }
    }
};
