@echo off
echo ========================================================
echo   BIEN DICH DO AN THUY CUNG 3D BANG MINGW G++ (64-BIT)
echo ========================================================

set GXX="C:\Program Files (x86)\Dev-Cpp\MinGW64\bin\g++.exe"
set GCC="C:\Program Files (x86)\Dev-Cpp\MinGW64\bin\gcc.exe"
if not exist %GXX% set GXX=g++.exe
if not exist %GCC% set GCC=gcc.exe

echo Dang bien dich va lien ket Aquarium_3D.exe...
%GXX% -std=c++11 -O2 -Iinclude src/main.cpp src/glad.c src/stb_image.cpp -Llib -Llib/mingw -lglfw3 -lopengl32 -lgdi32 -luser32 -o Aquarium_3D.exe

if %ERRORLEVEL% EQU 0 (
    copy /Y Aquarium_3D.exe Aquarium_OpenGL_Main.exe >nul
    echo [THANH CONG] Da tao thanh cong file: Aquarium_3D.exe va Aquarium_OpenGL_Main.exe!
    echo Chay ung dung bang lenh: Aquarium_3D.exe
) else (
    echo [THAT BAI] Co loi trong qua trinh bien dich!
)
