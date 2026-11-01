cmake_minimum_required(VERSION 3.10)
project(Aquarium_3D C CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Thu muc Header
include_directories(include)

# Danh sach ma nguon
set(SRC_FILES
    src/main.cpp
    src/glad.c
    src/stb_image.cpp
)

add_executable(Aquarium_3D ${SRC_FILES})

# Lien ket thu vien
if (MSVC)
    target_link_libraries(Aquarium_3D 
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/msvc/glfw3.lib 
        opengl32
    )
else()
    target_link_libraries(Aquarium_3D 
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/mingw/libglfw3.a 
        opengl32 
        gdi32
    )
endif()
