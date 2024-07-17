@echo off

rem download mingw here: https://github.com/skeeto/w64devkit

gcc src/*.c cri/src/cri_common.c cri/src/cri_win.c -O2 -s -std=c99 -Icri/include -lgdi32 -lwinmm -ldwmapi -lole32 -o blink.exe -mwindows
