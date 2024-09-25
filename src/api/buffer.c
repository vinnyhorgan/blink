#include "api.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define MAX_BUFFER_SIZE (1 << 30)

typedef struct {
    int size;
    uint8_t *data;
} Buffer;

static Buffer* new_buffer(lua_State *L, int size) {
    Buffer *buf = lua_newuserdata(L, sizeof(Buffer));
    luaL_setmetatable(L, API_TYPE_BUFFER);
    buf->size = size;
    buf->data = calloc(size, 1);
    if (!buf->data) { luaL_error(L, "out of memory"); }
    return buf;
}

static void check_offset(lua_State *L, Buffer *buf, int offset, int size) {
    if (offset < 0 || offset + size > buf->size) {
        luaL_error(L, "offset out of range");
    }
}

static int f_gc(lua_State *L) {
    Buffer *self = luaL_checkudata(L, 1, API_TYPE_BUFFER);
    if (self->data) { free(self->data); }
    return 0;
}

static int f_create(lua_State *L) {
    int size = luaL_checkinteger(L, 1);
    if (size < 0 || size > MAX_BUFFER_SIZE) { luaL_error(L, "size out of range"); }
    new_buffer(L, size);
    return 1;
}

static int f_from_string(lua_State *L) {
    size_t len;
    const char *str = luaL_checklstring(L, 1, &len);
    Buffer *self = new_buffer(L, len);
    memcpy(self->data, str, len);
    return 1;
}

static int f_to_string(lua_State *L) {
    Buffer *self = luaL_checkudata(L, 1, API_TYPE_BUFFER);
    lua_pushlstring(L, self->data, self->size);
    return 1;
}

static int f_get_size(lua_State *L) {
    Buffer *self = luaL_checkudata(L, 1, API_TYPE_BUFFER);
    lua_pushinteger(L, self->size);
    return 1;
}

#define READ_FUNC(name, type, push_func) \
static int f_read_##name(lua_State *L) { \
    Buffer *self = luaL_checkudata(L, 1, API_TYPE_BUFFER); \
    int offset = luaL_checkinteger(L, 2); \
    check_offset(L, self, offset, sizeof(type)); \
    type value; \
    memcpy(&value, self->data + offset, sizeof(type)); \
    push_func(L, value); \
    return 1; \
}

READ_FUNC(i8, int8_t, lua_pushinteger)
READ_FUNC(u8, uint8_t, lua_pushinteger)
READ_FUNC(i16, int16_t, lua_pushinteger)
READ_FUNC(u16, uint16_t, lua_pushinteger)
READ_FUNC(i32, int32_t, lua_pushinteger)
READ_FUNC(u32, uint32_t, lua_pushinteger)
READ_FUNC(f32, float, lua_pushnumber)
READ_FUNC(f64, double, lua_pushnumber)

#define WRITE_FUNC(name, type, check_func) \
static int f_write_##name(lua_State *L) { \
    Buffer *self = luaL_checkudata(L, 1, API_TYPE_BUFFER); \
    int offset = luaL_checkinteger(L, 2); \
    type value = check_func(L, 3); \
    check_offset(L, self, offset, sizeof(type)); \
    memcpy(self->data + offset, &value, sizeof(type)); \
    return 0; \
}

WRITE_FUNC(i8, int8_t, luaL_checkinteger)
WRITE_FUNC(u8, uint8_t, luaL_checkinteger)
WRITE_FUNC(i16, int16_t, luaL_checkinteger)
WRITE_FUNC(u16, uint16_t, luaL_checkinteger)
WRITE_FUNC(i32, int32_t, luaL_checkinteger)
WRITE_FUNC(u32, uint32_t, luaL_checkinteger)
WRITE_FUNC(f32, float, luaL_checknumber)
WRITE_FUNC(f64, double, luaL_checknumber)

static int f_read_string(lua_State *L) {
    Buffer *self = luaL_checkudata(L, 1, API_TYPE_BUFFER);
    int offset = luaL_checkinteger(L, 2);
    int size = luaL_checkinteger(L, 3);
    if (size < 0) { luaL_error(L, "size out of range"); }
    check_offset(L, self, offset, size);
    lua_pushlstring(L, self->data + offset, size);
    return 1;
}

static int f_write_string(lua_State *L) {
    Buffer *self = luaL_checkudata(L, 1, API_TYPE_BUFFER);
    int offset = luaL_checkinteger(L, 2);
    size_t len;
    const char *str = luaL_checklstring(L, 3, &len);
    int size = luaL_optinteger(L, 4, len);
    if (size < 0 || size > len) { luaL_error(L, "size out of range"); }
    check_offset(L, self, offset, size);
    memcpy(self->data + offset, str, size);
    return 0;
}

static int f_copy(lua_State *L) {
    Buffer *self = luaL_checkudata(L, 1, API_TYPE_BUFFER);
    int self_offset = luaL_checkinteger(L, 2);
    Buffer *other = luaL_checkudata(L, 3, API_TYPE_BUFFER);
    int other_offset = luaL_optinteger(L, 4, 0);
    int size = luaL_optinteger(L, 5, other->size - other_offset);
    if (size < 0) { luaL_error(L, "size out of range"); }
    check_offset(L, self, self_offset, size);
    check_offset(L, other, other_offset, size);
    memmove(self->data + self_offset, other->data + other_offset, size);
    return 0;
}

static int f_fill(lua_State *L) {
    Buffer *self = luaL_checkudata(L, 1, API_TYPE_BUFFER);
    int offset = luaL_checkinteger(L, 2);
    uint8_t value = luaL_checkinteger(L, 3);
    int size = luaL_optinteger(L, 4, self->size - offset);
    if (size < 0) { luaL_error(L, "size out of range"); }
    check_offset(L, self, offset, size);
    memset(self->data + offset, value, size);
    return 0;
}

static const luaL_Reg lib[] = {
    { "__gc", f_gc },
    { "create", f_create },
    { "from_string", f_from_string },
    { "to_string", f_to_string },
    { "get_size", f_get_size },
    { "read_i8", f_read_i8 },
    { "read_u8", f_read_u8 },
    { "read_i16", f_read_i16 },
    { "read_u16", f_read_u16 },
    { "read_i32", f_read_i32 },
    { "read_u32", f_read_u32 },
    { "read_f32", f_read_f32 },
    { "read_f64", f_read_f64 },
    { "write_i8", f_write_i8 },
    { "write_u8", f_write_u8 },
    { "write_i16", f_write_i16 },
    { "write_u16", f_write_u16 },
    { "write_i32", f_write_i32 },
    { "write_u32", f_write_u32 },
    { "write_f32", f_write_f32 },
    { "write_f64", f_write_f64 },
    { "read_string", f_read_string },
    { "write_string", f_write_string },
    { "copy", f_copy },
    { "fill", f_fill },
    { NULL, NULL }
};

int luaopen_buffer(lua_State *L) {
    luaL_newmetatable(L, API_TYPE_BUFFER);
    luaL_setfuncs(L, lib, 0);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    return 1;
}
