const std = @import("std");
const Op = @import("op.zig").Op;

const mem_size = 1 << 16;
const pc_start = 0x3000;

const Reg = enum(usize) {
    r0 = 0,
    r1,
    r2,
    r3,
    r4,
    r5,
    r6,
    r7,
    pc,
    cond,

    pub fn val(self: Reg) usize {
        return @intFromEnum(self);
    }
};

const Flag = enum(u8) {
    pos = 1 << 0,
    zero = 1 << 1,
    neg = 1 << 2,

    pub fn val(self: Flag) u8 {
        return @intFromEnum(self);
    }
};

const Trap = enum(u16) {
    getc = 0x20, // get character from keyboard, not echoed onto the terminal
    out = 0x21, // output a character
    puts = 0x22, // output a word string
    in = 0x23, // get character from keyboard, echoed onto the terminal
    putsp = 0x24, // output a byte string
    halt = 0x25, // halt the program
};

pub const Vm = struct {
    mem: [mem_size]u16 = undefined,
    reg: [Reg.cond.val() + 1]u16 = undefined,

    pub fn init(self: *Vm) void {
        std.debug.print("Initializing VM...\n", .{});
        self.reg[Reg.pc.val()] = pc_start;
    }
};
