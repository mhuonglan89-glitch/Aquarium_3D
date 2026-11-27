Aquarium OpenGL Dev-C++ project
================================

Project folder: D:\Aquarium_OpenGL
IDE: Dev-C++ 5.11 with TDM-GCC 4.9.2 64-bit
Output: Aquarium_OpenGL_DemoControls_MemberB.exe

Open Aquarium_OpenGL_DevCpp_Starter.dev in Dev-C++ and press F9 (Compile & Run).
Makefile.win builds the integrated Member A and Member B source files. Close the
aquarium window before rebuilding the EXE.

Controls
--------
W/A/S/D: move through the aquarium
Mouse: look around
Q/E: move down/up
Space: smoothly pass through the front glass and return
F: toggle exponential underwater fog
C: toggle animated caustics
Escape: close

Member B work
-------------
The project includes Shader and Mesh classes, GLSL compile/link, Blinn-Phong
lighting, diffuse texture mapping, exponential underwater fog, animated
caustics, OBJ/MTL loading through TinyObjLoader and stb_image, and an 80-bubble
particle system. The model loader groups faces by material, triangulates faces,
loads each MTL diffuse map, and generates missing normals and planar UVs.

The scene loads four real marine/reef meshes: clownfish and coral reef from
Grab3D, plus whale shark and eagle ray from 3DAssets.dev. The assignment sheet
mentions Sketchfab; these bundled assets are from the alternate CC0 sources,
with source records, original downloads, and AI-origin notes in
assets/source_models. The two GLB models are converted to OBJ for Dev-C++ by
tools/convert_glb_to_obj.js. The C++ scene animates their motion; the whale
shark also gets a tail-swish vertex animation in the shader.

Clownfish and coral use their source MTL textures. The shark and eagle ray source
meshes have material colors but no image maps, so the project supplies 2K
fallback diffuse maps for those models. 2K fallback textures are also bundled
for all four species as needed.

Integrated project files
------------------------
src/main.cpp                         Render loop and A+B integration
src/Camera.cpp / include/Camera.h    First-person controls and SPACE LERP
src/Tunnel.cpp / include/Tunnel.h    Walkway, LED strips, steel arches, glass vault
src/BezierPath.cpp / include/BezierPath.h  Cubic Bezier animal paths
src/Shader.cpp / include/Shader.h    Shader compile/link and ownership
src/Model.cpp / include/Model.h      OBJ/MTL and texture loader
src/ParticleSystem.cpp               Rising bubble motion
include/Mesh.h                       VAO/VBO mesh interface
assets/shaders/ocean_fog.vs          Vertex shader and shark tail motion
assets/shaders/ocean_fog.fs          Lighting, texture, caustics and fog
assets/models/                       Four OBJ meshes and their MTL files/maps
assets/source_models/README.txt      Source and license record
assets/source_models/downloads/      Original ZIPs, extracted assets and GLBs
tools/convert_glb_to_obj.js          Offline GLB to OBJ converter
tools/generate_member_b_textures.cpp  2K fallback texture generator
