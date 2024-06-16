@echo off

rem download compiler here: https://github.com/skeeto/w64devkit/

gcc src/*.c src/lib/*.c -o blink.exe -std=c99 -lgdi32 -luser32 -lwinmm -ldwmapi -O3 -s -mwindows
