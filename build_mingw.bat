@echo off
echo ========================================================
echo   BIEN DICH DO AN THUY CUNG 3D BANG MINGW G++ (64-BIT)
echo   TICH HOP NGUOI A (Math, Camera, Tunnel) VA NGUOI B (Shaders, Models)
echo ========================================================

set GXX="C:\Program Files (x86)\Dev-Cpp\MinGW64\bin\g++.exe"
if not exist %GXX% (
    set GXX=g++.exe
)

echo Dang bien dich ma nguon...
%GXX% -std=c++11 -O2 -Iinclude src/main.cpp src/glad.c src/stb_image.cpp -Llib/mingw -lglfw3 -lopengl32 -lgdi32 -o Aquarium_OpenGL_Main.exe

if %ERRORLEVEL% EQU 0 (
    copy /Y Aquarium_OpenGL_Main.exe Aquarium_3D.exe >nul
    echo [THANH CONG] Da tao thanh cong file: Aquarium_OpenGL_Main.exe va Aquarium_3D.exe!
    echo Chay ung dung bang lenh: Aquarium_OpenGL_Main.exe
) else (
    echo [THAT BAI] Co loi trong qua trinh bien dich!
)
pause
