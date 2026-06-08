@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" x64
cd /d D:\SOLO-9\23-freeform-lens-tracer\build
cmake .. -G Ninja -DCMAKE_PREFIX_PATH=D:\Qt\6.8.3\msvc2022_64 -DCMAKE_BUILD_TYPE=Release
if %ERRORLEVEL% EQU 0 (
    ninja
)
