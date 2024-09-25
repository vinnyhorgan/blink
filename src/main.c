#include <stdio.h>
#include "lib/libuv/include/uv.h"
#include "api/api.h"

static const char* get_platform(void) {
#if defined(_WIN32)
    return "windows";
#elif defined(__APPLE__)
    return "macos";
#elif defined(__linux__)
    return "linux";
#else
    return "unknown";
#endif
}

int main(int argc, char **argv) {
    argv = uv_setup_args(argc, argv);

    lua_State *L = luaL_newstate();
    luaL_openlibs(L);
    api_load_libs(L);

    lua_newtable(L);
    for (int i = 0; i < argc; i++) {
        lua_pushstring(L, argv[i]);
        lua_rawseti(L, -2, i + 1);
    }
    lua_setglobal(L, "ARGS");

    lua_pushstring(L, "1.0");
    lua_setglobal(L, "VERSION");

    lua_pushstring(L, get_platform());
    lua_setglobal(L, "PLATFORM");

    char exename[2048];
    size_t size = sizeof(exename);
    uv_exepath(exename, &size);
    lua_pushstring(L, exename);
    lua_setglobal(L, "EXEFILE");

    luaL_dostring(L,
        "xpcall(function()\n"
        "    require('main')\n"
        "end, function(err)\n"
        "    print('Error: ' .. tostring(err))\n"
        "    print(debug.traceback(nil, 2))\n"
        "    os.exit(1)\n"
        "end)");

    lua_close(L);

    return 0;
}
