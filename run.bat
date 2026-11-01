@echo off
if not exist "Aquarium_3D.exe" (
    echo [CHUA CO FILE THUC THI] Dang bien dich truoc khi chay...
    call build_mingw.bat
)
echo Dang khoi dong Mo Phong Thuy Cung 3D...
start "" Aquarium_3D.exe

