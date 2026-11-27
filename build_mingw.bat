@echo off
echo ========================================================
echo   BIEN DICH DO AN THUY CUNG 3D BANG MINGW G++ (64-BIT)
echo   [NHIEM VU NGUOI B: SHADERS, MODELS 3D, TEXTURES, PARTICLES]
echo ========================================================

set GXX="C:\Program Files (x86)\Dev-Cpp\MinGW64\bin\g++.exe"
set GCC="C:\Program Files (x86)\Dev-Cpp\MinGW64\bin\gcc.exe"
if not exist %GXX% set GXX=g++.exe
if not exist %GCC% set GCC=gcc.exe

echo Dang bien dich cac module cua Nguoi B...
%GCC% -O2 -c src/cgltf.c -o src/cgltf.o -Iinclude -Iinclude/glad
%GCC% -O2 -c src/glad.c -o src/glad.o -Iinclude -Iinclude/glad
%GCC% -O2 -c src/stb_image.c -o src/stb_image.o -Iinclude -Iinclude/glad
%GXX% -std=c++11 -O2 -c src/ParticleSystem.cpp -o src/ParticleSystem.o -Iinclude -Iinclude/glad
%GXX% -std=c++11 -O2 -c src/Model.cpp -o src/Model.o -Iinclude -Iinclude/glad
%GXX% -std=c++11 -O2 -c src/Shader.cpp -o src/Shader.o -Iinclude -Iinclude/glad
%GXX% -std=c++11 -O2 -c src/main.cpp -o src/main.o -Iinclude -Iinclude/glad

echo Dang lien ket file chay...
%GXX% src/main.o src/cgltf.o src/glad.o src/ParticleSystem.o src/Model.o src/Shader.o src/stb_image.o -L"C:\Program Files (x86)\Dev-Cpp\MinGW64\lib" -L"C:\Program Files (x86)\Dev-Cpp\MinGW64\x86_64-w64-mingw32\lib" -static-libgcc -Llib -Llib/mingw -lglfw3 -lgdi32 -luser32 -lopengl32 -o Aquarium_OpenGL_Main.exe

if %ERRORLEVEL% EQU 0 (
    copy /Y Aquarium_OpenGL_Main.exe Aquarium_3D.exe >nul
    echo [THANH CONG] Da tao thanh cong file: Aquarium_OpenGL_Main.exe va Aquarium_3D.exe!
    echo Chay ung dung bang lenh: Aquarium_OpenGL_Main.exe
) else (
    echo [THAT BAI] Co loi trong qua trinh bien dich!
)
pause
