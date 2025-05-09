const std = @import("std");

const Console = @import("console.zig").Console;

const mem_size = 1 << 16;
const pc_start = 0x3000;

pub const Reg = enum(u8) {
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

    pub fn val(self: Reg) u8 {
        return @intFromEnum(self);
    }
};

pub const Flag = enum(u8) {
    pos = 1 << 0,
    zero = 1 << 1,
    neg = 1 << 2,

    pub fn val(self: Flag) u8 {
        return @intFromEnum(self);
    }
};

const Op = enum(u8) {
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
};

const Trap = enum(u16) {
    getc = 0x20, // get character from keyboard, not echoed onto the terminal
    out = 0x21, // output a character
    puts = 0x22, // output a word string
    in = 0x23, // get character from keyboard, echoed onto the terminal
    putsp = 0x24, // output a byte string
    halt = 0x25, // halt the program
};

fn signExtend(val: u16, comptime bit_count: u16) u16 {
    const sign_bit = 1 << (bit_count - 1);
    if (val & sign_bit != 0) {
        return val | @as(u16, @truncate((0xFFFF << bit_count)));
    }
    return val;
}

pub const Vm = struct {
    mem: [mem_size]u16,
    reg: [Reg.cond.val() + 1]u16,
    console: *Console,
    exit: bool,

    pub fn init(console: *Console) Vm {
        console.log("Initializing VM...\n", .{});

        var self: Vm = Vm{
            .mem = [_]u16{0} ** mem_size,
            .reg = [_]u16{0} ** (Reg.cond.val() + 1),
            .console = console,
            .exit = false,
        };

        self.reg[Reg.pc.val()] = pc_start;
        self.reg[Reg.cond.val()] = Flag.zero.val();

        return self;
    }

    pub fn loadRom(self: *Vm, path: []const u8) !void {
        const file = try std.fs.cwd().openFile(path, .{});
        defer file.close();

        const reader = file.reader();

        var origin = try reader.readInt(u16, std.builtin.Endian.little);
        origin = @byteSwap(origin);

        const max_read = @as(u32, @intCast(mem_size)) - @as(u32, @intCast(origin));

        var mem_idx = origin;
        while (mem_idx < max_read) {
            const word = reader.readInt(u16, std.builtin.Endian.little) catch |err| {
                switch (err) {
                    error.EndOfStream => break,
                    else => return err,
                }
            };
            self.mem[mem_idx] = @byteSwap(word);
            mem_idx += 1;
        }
    }

    pub fn runCycle(self: *Vm, log: bool) bool {
        if (log) {
            self.console.log("Start cycle...\n", .{});
        }

        const instruction = self.readMem(self.reg[Reg.pc.val()]);

        self.reg[Reg.pc.val()] += 1;

        const op: Op = @enumFromInt(instruction >> 12);

        if (log) {
            self.console.log("Executing opcode: {s}\n", .{@tagName(op)});
        }

        switch (op) {
            Op.LEA => self.opLEA(instruction),
            Op.TRAP => self.opTRAP(instruction),

            Op.BR => self.opBR(instruction),
            Op.ADD => self.opADD(instruction),
            else => {
                if (log) {
                    self.console.log("unknown opcode: {s}, stopping...\n", .{@tagName(op)});
                }
                return false;
            },
        }

        if (self.exit) {
            self.exit = false;
            return false;
        }

        return true;
    }

    pub fn readMem(self: *Vm, addr: u16) u16 {
        return self.mem[addr];
    }

    pub fn writeMem(self: *Vm, addr: u16, value: u16) void {
        self.mem[addr] = value;
    }

    pub fn updateFlags(self: *Vm, reg: Reg) void {
        if (self.reg[reg.val()] == 0) {
            self.reg[Reg.cond.val()] = Flag.zero.val();
        } else if ((self.reg[reg.val()] >> 15) == 1) {
            self.reg[Reg.cond.val()] = Flag.neg.val();
        } else {
            self.reg[Reg.cond.val()] = Flag.pos.val();
        }
    }

    // instructions

    fn opLEA(self: *Vm, instruction: u16) void {
        const dr = (instruction >> 9) & 0x7;
        const pc_offset = signExtend(instruction & 0x1FF, 9);
        const addr, _ = @addWithOverflow(self.reg[Reg.pc.val()], pc_offset);
        self.reg[dr] = addr;
        self.updateFlags(@enumFromInt(dr));
    }

    fn opTRAP(self: *Vm, instruction: u16) void {
        self.reg[Reg.r7.val()] = self.reg[Reg.pc.val()];

        const trap_code: Trap = @enumFromInt(instruction & 0xFF);
        switch (trap_code) {
            Trap.puts => {
                var addr = self.reg[Reg.r0.val()];
                var c: u8 = @truncate(self.readMem(addr));

                var buf: [256]u8 = undefined;
                var i: usize = 0;

                while (c != 0) {
                    buf[i] = c;
                    i += 1;
                    addr += 1;
                    c = @truncate(self.readMem(addr));
                }
                buf[i] = 0;

                self.console.log("{s}", .{buf});
            },
            Trap.halt => {
                self.console.log("Halting...\n", .{});
                self.exit = true;
            },
            Trap.getc => {},
            Trap.out => {},
            Trap.in => {},
            Trap.putsp => {},
        }
    }

    fn opBR(self: *Vm, instruction: u16) void {
        _ = self;
        _ = instruction;
    }

    fn opADD(self: *Vm, instruction: u16) void {
        const dr = (instruction >> 9) & 0x7;
        const sr1 = (instruction >> 6) & 0x7;

        const immediate = (instruction >> 5) & 1;
        if (immediate == 1) {
            const imm5 = signExtend(instruction & 0x1F, 5);
            self.reg[dr], _ = @addWithOverflow(self.reg[sr1], imm5);
        } else {
            const sr2 = instruction & 0x7;
            self.reg[dr], _ = @addWithOverflow(self.reg[sr1], self.reg[sr2]);
        }

        self.updateFlags(@enumFromInt(dr));
    }
};
