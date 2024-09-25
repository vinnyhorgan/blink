@echo off

rem download mingw here: https://github.com/skeeto/w64devkit

gcc src/*.c src/api/*.c src/lib/libuv/src/*.c src/lib/libuv/src/win/*.c^
    src/lib/minilua/*.c src/lib/miniz/*.c src/lib/monocypher/*.c^
    -O3 -s -std=c99 -fno-strict-aliasing -Isrc/lib/libuv/include -Isrc/lib/libuv/src^
    -ladvapi32 -liphlpapi -luserenv -lws2_32 -ldbghelp -lole32^
    -o blink.exe
