// ==============================================================================
// ĐỒ ÁN MÔ PHỎNG THỦY CUNG 3D TƯƠNG TÁC (COMPUTER GRAPHICS)
// PHẦN DEMO ĐỘC LẬP DÀNH CHO BẠN B: KỸ SƯ VẬT LIỆU, SHADERS & TÀI NGUYÊN 3D
//
// Chỉ sử dụng các thành phần của Bạn B:
// 1. Shaders: ocean_fog.vs, ocean_fog.fs (Blinn-Phong, Fog, Caustics, Alpha Blending)
// 2. Models 3D: whale_shark.obj, spotted_eagle_ray.obj, clownfish.obj, coral_reef.obj
// 3. Textures: shark_diffuse.jpg, ray_texture.png, coral_base.png, clownfish.png
// 4. Hệ hạt: ParticleSystem (80 bọt khí nổi từ san hô)
// 5. Thư viện đọc: tiny_obj_loader.h, stb_image.h, Mesh.h, Shader.h
// ==============================================================================

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include "ParticleSystem.h"
#include "Model.h"
#include "Mesh.h"
#include "Shader.h"
#include "stb_image.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

static const float PI = 3.14159265358979323846f;

// -----------------------------------------------------------------------------
// Toán học Vector & Ma trận cơ bản (phục vụ render độc lập cho Bạn B)
// -----------------------------------------------------------------------------
struct Vec3 {
    float x, y, z;
    Vec3() : x(0.0f), y(0.0f), z(0.0f) {}
    Vec3(float px, float py, float pz) : x(px), y(py), z(pz) {}
};

static Vec3 add(const Vec3& a, const Vec3& b) { return Vec3(a.x + b.x, a.y + b.y, a.z + b.z); }
static Vec3 subtract(const Vec3& a, const Vec3& b) { return Vec3(a.x - b.x, a.y - b.y, a.z - b.z); }
static Vec3 multiply(const Vec3& a, float val) { return Vec3(a.x * val, a.y * val, a.z * val); }
static float dot(const Vec3& a, const Vec3& b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
static Vec3 cross(const Vec3& a, const Vec3& b) {
    return Vec3(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
}
static Vec3 normalize(const Vec3& v) {
    float len = std::sqrt(dot(v, v));
    if (len < 0.00001f) return Vec3(0, 0, 0);
    return multiply(v, 1.0f / len);
}

struct Mat4 { float v[16]; };

static Mat4 identityMatrix() {
    Mat4 m;
    for (int i = 0; i < 16; ++i) m.v[i] = 0.0f;
    m.v[0] = m.v[5] = m.v[10] = m.v[15] = 1.0f;
    return m;
}

static Mat4 multiply(const Mat4& a, const Mat4& b) {
    Mat4 res;
    for (int c = 0; c < 4; ++c) {
        for (int r = 0; r < 4; ++r) {
            res.v[c * 4 + r] = 0.0f;
            for (int k = 0; k < 4; ++k) res.v[c * 4 + r] += a.v[k * 4 + r] * b.v[c * 4 + k];
        }
    }
    return res;
}

static Mat4 translation(const Vec3& p) {
    Mat4 m = identityMatrix();
    m.v[12] = p.x; m.v[13] = p.y; m.v[14] = p.z;
    return m;
}

static Mat4 scaling(const Vec3& s) {
    Mat4 m = identityMatrix();
    m.v[0] = s.x; m.v[5] = s.y; m.v[10] = s.z;
    return m;
}

static Mat4 rotationY(float a) {
    Mat4 m = identityMatrix();
    float c = std::cos(a), s = std::sin(a);
    m.v[0] = c; m.v[2] = -s; m.v[8] = s; m.v[10] = c;
    return m;
}

static Mat4 rotationZ(float a) {
    Mat4 m = identityMatrix();
    float c = std::cos(a), s = std::sin(a);
    m.v[0] = c; m.v[1] = s; m.v[4] = -s; m.v[5] = c;
    return m;
}

static Mat4 rotationX(float a) {
    Mat4 m = identityMatrix();
    float c = std::cos(a), s = std::sin(a);
    m.v[5] = c; m.v[6] = s; m.v[9] = -s; m.v[10] = c;
    return m;
}

static Mat4 perspective(float fovRad, float aspect, float nearP, float farP) {
    Mat4 m;
    for (int i = 0; i < 16; ++i) m.v[i] = 0.0f;
    float tanHalf = std::tan(fovRad * 0.5f);
    if (aspect <= 0.0f) aspect = 1.0f;
    m.v[0] = 1.0f / (aspect * tanHalf);
    m.v[5] = 1.0f / tanHalf;
    m.v[10] = -(farP + nearP) / (farP - nearP);
    m.v[11] = -1.0f;
    m.v[14] = -(2.0f * farP * nearP) / (farP - nearP);
    return m;
}

static Mat4 lookAt(const Vec3& eye, const Vec3& target, const Vec3& up) {
    Vec3 f = normalize(subtract(target, eye));
    Vec3 r = normalize(cross(f, up));
    Vec3 u = cross(r, f);
    Mat4 m = identityMatrix();
    m.v[0] = r.x;  m.v[4] = r.y;  m.v[8] = r.z;   m.v[12] = -dot(r, eye);
    m.v[1] = u.x;  m.v[5] = u.y;  m.v[9] = u.z;   m.v[13] = -dot(u, eye);
    m.v[2] = -f.x; m.v[6] = -f.y; m.v[10] = -f.z; m.v[14] = dot(f, eye);
    return m;
}

static Mat4 modelMatrix(const Vec3& pos, const Vec3& sz, float rotY = 0.0f, float rotZ = 0.0f) {
    Mat4 m = translation(pos);
    if (rotY != 0.0f) m = multiply(m, rotationY(rotY));
    if (rotZ != 0.0f) m = multiply(m, rotationZ(rotZ));
    m = multiply(m, scaling(sz));
    return m;
}

// -----------------------------------------------------------------------------
// Hình học cơ bản (Khối hộp đáy biển & Hình cầu bọt khí)
// -----------------------------------------------------------------------------
static Vertex makeVertex(const Vec3& p, const Vec3& n, float u = 0.0f, float v = 0.0f) {
    Vertex vert;
    vert.position[0] = p.x; vert.position[1] = p.y; vert.position[2] = p.z;
    vert.normal[0] = n.x;   vert.normal[1] = n.y;   vert.normal[2] = n.z;
    vert.texCoord[0] = u;   vert.texCoord[1] = v;
    return vert;
}

static void addTriangle(std::vector<Vertex>& verts, const Vec3& a, const Vec3& b, const Vec3& c, const Vec3& n) {
    verts.push_back(makeVertex(a, n));
    verts.push_back(makeVertex(b, n));
    verts.push_back(makeVertex(c, n));
}

static void addQuad(std::vector<Vertex>& verts, const Vec3& a, const Vec3& b, const Vec3& c, const Vec3& d, const Vec3& n) {
    addTriangle(verts, a, b, c, n);
    addTriangle(verts, a, c, d, n);
}

static void createGlassQuad(Mesh& mesh) {
    std::vector<Vertex> verts;
    addQuad(verts, Vec3(-0.5f, -0.5f, 0.0f), Vec3(0.5f, -0.5f, 0.0f),
            Vec3(0.5f, 0.5f, 0.0f), Vec3(-0.5f, 0.5f, 0.0f), Vec3(0.0f, 0.0f, 1.0f));
    mesh.upload(verts);
}
static void createCube(Mesh& mesh) {
    std::vector<Vertex> verts;
    const float h = 0.5f;
    addQuad(verts, Vec3(-h,-h,h), Vec3(h,-h,h), Vec3(h,h,h), Vec3(-h,h,h), Vec3(0,0,1));
    addQuad(verts, Vec3(h,-h,-h), Vec3(-h,-h,-h), Vec3(-h,h,-h), Vec3(h,h,-h), Vec3(0,0,-1));
    addQuad(verts, Vec3(h,-h,h), Vec3(h,-h,-h), Vec3(h,h,-h), Vec3(h,h,h), Vec3(1,0,0));
    addQuad(verts, Vec3(-h,-h,-h), Vec3(-h,-h,h), Vec3(-h,h,h), Vec3(-h,h,-h), Vec3(-1,0,0));
    addQuad(verts, Vec3(-h,h,h), Vec3(h,h,h), Vec3(h,h,-h), Vec3(-h,h,-h), Vec3(0,1,0));
    addQuad(verts, Vec3(-h,-h,-h), Vec3(h,-h,-h), Vec3(h,-h,h), Vec3(-h,-h,h), Vec3(0,-1,0));
    mesh.upload(verts);
}

static void createSphere(Mesh& mesh) {
    std::vector<Vertex> verts;
    const int rings = 16, segments = 24;
    for (int i = 0; i < rings; ++i) {
        float t0 = PI * (float)i / (float)rings;
        float t1 = PI * (float)(i + 1) / (float)rings;
        for (int j = 0; j < segments; ++j) {
            float p0 = 2.0f * PI * (float)j / (float)segments;
            float p1 = 2.0f * PI * (float)(j + 1) / (float)segments;

            Vec3 a(std::sin(t0)*std::cos(p0), std::cos(t0), std::sin(t0)*std::sin(p0));
            Vec3 b(std::sin(t1)*std::cos(p0), std::cos(t1), std::sin(t1)*std::sin(p0));
            Vec3 c(std::sin(t1)*std::cos(p1), std::cos(t1), std::sin(t1)*std::sin(p1));
            Vec3 d(std::sin(t0)*std::cos(p1), std::cos(t0), std::sin(t0)*std::sin(p1));

            addTriangle(verts, a, b, c, a);
            addTriangle(verts, a, c, d, a);
        }
    }
    mesh.upload(verts);
}

// -----------------------------------------------------------------------------
static GLuint loadTextureImage(const char* path) {
    int w = 0, h = 0, ch = 0;
    stbi_set_flip_vertically_on_load(1);
    unsigned char* data = stbi_load(path, &w, &h, &ch, 4);
    if (!data) {
        std::string fallback = std::string("D:/Aquarium_OpenGL/") + path;
        data = stbi_load(fallback.c_str(), &w, &h, &ch, 4);
    }
    if (!data) {
        std::string fname = path;
        std::string::size_type p = fname.find_last_of("/\\");
        if (p != std::string::npos) fname = fname.substr(p + 1);
        std::string alt = "assets/textures/" + fname;
        data = stbi_load(alt.c_str(), &w, &h, &ch, 4);
    }
    if (!data) {
        std::cerr << "Khong the nap texture: " << path << std::endl;
        return 0;
    }

    GLuint tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
    stbi_image_free(data);
    return tex;
}

struct ShaderUniforms {
    GLint model, view, projection;
    GLint tint, viewPosition, fogColor, fogDensity, emission;
    GLint time, pattern, caustics, texture, useTexture, swim;
};

static ShaderUniforms getUniforms(GLuint prog) {
    ShaderUniforms u;
    u.model        = glGetUniformLocation(prog, "uModel");
    u.view         = glGetUniformLocation(prog, "uView");
    u.projection   = glGetUniformLocation(prog, "uProjection");
    u.tint         = glGetUniformLocation(prog, "uTint");
    u.viewPosition = glGetUniformLocation(prog, "uViewPosition");
    u.fogColor     = glGetUniformLocation(prog, "uFogColor");
    u.fogDensity   = glGetUniformLocation(prog, "uFogDensity");
    u.emission     = glGetUniformLocation(prog, "uEmission");
    u.time         = glGetUniformLocation(prog, "uTime");
    u.pattern      = glGetUniformLocation(prog, "uPattern");
    u.caustics     = glGetUniformLocation(prog, "uCaustics");
    u.texture      = glGetUniformLocation(prog, "uTexture");
    u.useTexture   = glGetUniformLocation(prog, "uUseTexture");
    u.swim         = glGetUniformLocation(prog, "uSwim");
    return u;
}

static void drawMeshShape(const Mesh& mesh, const ShaderUniforms& u,
                          const Mat4& mat, float r, float g, float b, float a,
                          float caustics = 0.0f) {
    glVertexAttrib4f(3, 1.0f, 1.0f, 1.0f, 1.0f);
    glUniformMatrix4fv(u.model, 1, GL_FALSE, mat.v);
    glUniform4f(u.tint, r, g, b, a);
    glUniform1f(u.pattern, 0.0f);
    glUniform1f(u.caustics, caustics);
    glUniform1f(u.useTexture, 0.0f);
    glUniform1f(u.swim, 0.0f);
    glUniform1f(u.emission, 0.0f);
    glBindVertexArray(mesh.vao);
    glDrawArrays(GL_TRIANGLES, 0, mesh.vertexCount);
}

static void drawObjModel(const Model& model, const ShaderUniforms& u,
                         const Mat4& mat, GLuint tex,
                         float r = 1.0f, float g = 1.0f, float b = 1.0f, float a = 1.0f,
                         float swimMode = 0.0f, float emission = 0.0f) {
    glUniformMatrix4fv(u.model, 1, GL_FALSE, mat.v);
    glUniform1f(u.pattern, 0.0f);
    glUniform1f(u.caustics, 0.0f);
    glUniform1f(u.swim, swimMode);
    glUniform1f(u.emission, emission);
    model.draw(u.tint, u.texture, u.useTexture, tex, r, g, b, a);
}

// -----------------------------------------------------------------------------
// Điều khiển Camera tự do (Độc lập, tiện cho Bạn B kiểm tra mô hình & shader)
// -----------------------------------------------------------------------------
static Vec3 camPos(0.0f, 0.2f, 8.5f);
static float camYaw = -90.0f, camPitch = -4.0f;
static float lastX = 500.0f, lastY = 350.0f;
static bool firstMouse = true;
static bool fogEnabled = true;
static bool causticsEnabled = true;
static bool fogWasDown = false;
static bool causticsWasDown = false;

static Vec3 getCamFront() {
    float yRad = camYaw * PI / 180.0f;
    float pRad = camPitch * PI / 180.0f;
    return normalize(Vec3(std::cos(yRad) * std::cos(pRad),
                          std::sin(pRad),
                          std::sin(yRad) * std::cos(pRad)));
}

static void mouseCallback(GLFWwindow* w, double x, double y) {
    (void)w;
    if (firstMouse) { lastX = (float)x; lastY = (float)y; firstMouse = false; return; }
    float xOff = (float)x - lastX;
    float yOff = lastY - (float)y;
    lastX = (float)x; lastY = (float)y;
    camYaw += xOff * 0.10f;
    camPitch += yOff * 0.10f;
    if (camPitch > 85.0f) camPitch = 85.0f;
    if (camPitch < -85.0f) camPitch = -85.0f;
}

static void processInput(GLFWwindow* win, float dt) {
    if (glfwGetKey(win, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(win, GLFW_TRUE);

    bool fDown = glfwGetKey(win, GLFW_KEY_F) == GLFW_PRESS;
    if (fDown && !fogWasDown) fogEnabled = !fogEnabled;
    fogWasDown = fDown;

    bool cDown = glfwGetKey(win, GLFW_KEY_C) == GLFW_PRESS;
    if (cDown && !causticsWasDown) causticsEnabled = !causticsEnabled;
    causticsWasDown = cDown;

    Vec3 front = getCamFront();
    Vec3 hFront = normalize(Vec3(front.x, 0.0f, front.z));
    Vec3 right = normalize(cross(hFront, Vec3(0, 1, 0)));
    float speed = 3.5f * dt;

    if (glfwGetKey(win, GLFW_KEY_W) == GLFW_PRESS) camPos = add(camPos, multiply(hFront, speed));
    if (glfwGetKey(win, GLFW_KEY_S) == GLFW_PRESS) camPos = subtract(camPos, multiply(hFront, speed));
    if (glfwGetKey(win, GLFW_KEY_A) == GLFW_PRESS) camPos = subtract(camPos, multiply(right, speed));
    if (glfwGetKey(win, GLFW_KEY_D) == GLFW_PRESS) camPos = add(camPos, multiply(right, speed));
    if (glfwGetKey(win, GLFW_KEY_E) == GLFW_PRESS) camPos.y += speed;
    if (glfwGetKey(win, GLFW_KEY_Q) == GLFW_PRESS) camPos.y -= speed;
}

// -----------------------------------------------------------------------------
// Hàm MAIN (Demo độc lập toàn diện của Bạn B)
// -----------------------------------------------------------------------------
int main() {
    if (!glfwInit()) {
        std::cerr << "Khong the khoi tao GLFW." << std::endl;
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(
        1000, 700,
        "Aquarium 3D - [Nhiem vu Nguoi B: Shaders, Models 3D, Textures, Particles] | WASD, Chuot, F, C",
        NULL, NULL);
    if (!window) {
        std::cerr << "Khong the tao cua so GLFW." << std::endl;
        glfwTerminate();
        return 1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    glfwSetCursorPosCallback(window, mouseCallback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (gladLoadGL(glfwGetProcAddress) == 0) {
        std::cerr << "Khong the nap GLAD." << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // 1. Lập trình Shaders Biển Sâu (Nhiệm vụ 2 của Bạn B)
    Shader shader;
    if (!shader.load("assets/shaders/ocean_fog.vs", "assets/shaders/ocean_fog.fs")) {
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }
    GLuint prog = shader.id();
    ShaderUniforms uniforms = getUniforms(prog);

    Mesh cube, sphere, glassQuad;
    createGlassQuad(glassQuad);
    createCube(cube);
    createSphere(sphere);

    // 2. Hệ hạt Bọt Khí (Nhiệm vụ 3 của Bạn B)
    ParticleSystem particles;

    // 3. Nạp Model 3D thực tế (Nhiệm vụ 1 của Bạn B)
    Model objShark, objFish, objCoral, objRay;
    bool sharkLoaded = objShark.loadOBJ("assets/models/whale_shark.obj");
    bool fishLoaded  = objFish.loadOBJ("assets/models/clownfish.obj");
    bool coralLoaded = objCoral.loadOBJ("assets/models/coral_reef.obj");
    bool rayLoaded = objRay.loadOBJ("assets/models/spotted_eagle_ray.obj");

    if (sharkLoaded) std::cout << "[OK] Nap thanh cong: whale_shark.obj" << std::endl;
    if (fishLoaded)  std::cout << "[OK] Nap thanh cong: clownfish.obj" << std::endl;
    if (coralLoaded) std::cout << "[OK] Nap thanh cong: coral_reef.obj" << std::endl;
    if (rayLoaded) std::cout << "[OK] Nap thanh cong: spotted_eagle_ray.obj" << std::endl;
    const int downloadedFishCount = 5;
    Model downloadedFish[5];
    const char* speciesNames[5] = {
        "Barramundi", "Pufferfish", "Mahi Mahi", "Leafy Seadragon", "Seahorse"
    };
    const char* speciesPaths[5] = {
        "assets/models/real_fish/BarramundiFish.glb",
        "assets/models/real_fish/Pufferfish.glb",
        "assets/models/real_fish/MahiMahi.glb",
        "assets/models/real_fish/LeafySeadragon.glb",
        "assets/models/real_fish/Seahorse.glb"
    };
    bool speciesLoaded[5] = {false, false, false, false, false};
    for (int i = 0; i < downloadedFishCount; ++i) {
        speciesLoaded[i] = downloadedFish[i].loadGLB(speciesPaths[i]);
        if (speciesLoaded[i])
            std::cout << "[OK] Nap model ca tai ve: " << speciesNames[i] << ".glb" << std::endl;
    }
    Model jellyfishBloom;
    bool jellyfishLoaded = jellyfishBloom.loadGLB("assets/models/real_fish/JellyfishBloom.glb");
    if (jellyfishLoaded)
        std::cout << "[OK] Nap dan sua tai ve: JellyfishBloom.glb" << std::endl;

    // Nạp Textures 2K chuẩn (.jpg, .png)
    GLuint sharkTex = loadTextureImage("assets/textures/shark_diffuse.jpg");
    GLuint coralTex = loadTextureImage("assets/models/coral-reef_body_basecolor.jpg");
    GLuint fishTex  = loadTextureImage("assets/textures/clownfish_2k.ppm");

    double prevTime = glfwGetTime();

    while (!glfwWindowShouldClose(window)) {
        double curTime = glfwGetTime();
        float dt = (float)(curTime - prevTime);
        prevTime = curTime;
        if (dt > 0.05f) dt = 0.05f;

        processInput(window, dt);

        int fbW = 0, fbH = 0;
        glfwGetFramebufferSize(window, &fbW, &fbH);
        glViewport(0, 0, fbW, fbH);

        // Màu nước xanh biển sâu thẳm
        Vec3 fogCol(0.035f, 0.32f, 0.45f);
        glClearColor(fogCol.x, fogCol.y, fogCol.z, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        Mat4 view = lookAt(camPos, add(camPos, getCamFront()), Vec3(0, 1, 0));
        Mat4 proj = perspective(48.0f * PI / 180.0f,
                                fbH > 0 ? (float)fbW / (float)fbH : 1.0f,
                                0.1f, 80.0f);

        glUseProgram(prog);
        glUniformMatrix4fv(uniforms.view, 1, GL_FALSE, view.v);
        glUniformMatrix4fv(uniforms.projection, 1, GL_FALSE, proj.v);
        glUniform3f(uniforms.viewPosition, camPos.x, camPos.y, camPos.z);
        glUniform3f(uniforms.fogColor, fogCol.x, fogCol.y, fogCol.z);
        glUniform1f(uniforms.fogDensity, fogEnabled ? 0.045f : 0.0f);
        glUniform1f(uniforms.time, (float)curTime);

        // 1. Đáy cát biển (Caustics khúc xạ tia nắng rọi qua nước)
        Mat4 seabed = modelMatrix(Vec3(0.0f, -2.55f, 0.5f), Vec3(22.0f, 0.35f, 18.0f));
        drawMeshShape(cube, uniforms, seabed, 0.45f, 0.35f, 0.22f, 1.0f, causticsEnabled ? 0.50f : 0.0f);

        // 2. Rạn đá và San hô 3D của Bạn B (coral_reef.obj)
        drawMeshShape(sphere, uniforms, modelMatrix(Vec3(-4.0f, -2.28f, -2.2f), Vec3(1.0f, 0.34f, 0.78f)), 0.30f, 0.28f, 0.24f, 1.0f);
        drawMeshShape(sphere, uniforms, modelMatrix(Vec3( 3.8f, -2.28f, -1.5f), Vec3(1.05f, 0.36f, 0.82f)), 0.32f, 0.30f, 0.26f, 1.0f);
        drawMeshShape(sphere, uniforms, modelMatrix(Vec3( 1.0f, -2.28f,  2.2f), Vec3(0.90f, 0.32f, 0.70f)), 0.34f, 0.31f, 0.25f, 1.0f);

        if (coralLoaded) {
            Mat4 cL = modelMatrix(Vec3(-4.0f, -2.35f, -2.2f), Vec3(4.80f, 4.80f, 4.80f), 0.25f);
            drawObjModel(objCoral, uniforms, cL, coralTex, 1.0f, 1.0f, 1.0f, 1.0f);

            Mat4 cR = modelMatrix(Vec3(3.8f, -2.35f, -1.5f), Vec3(4.40f, 4.40f, 4.40f), -0.55f);
            drawObjModel(objCoral, uniforms, cR, coralTex, 0.95f, 1.0f, 0.95f, 1.0f);

            Mat4 cM = modelMatrix(Vec3(1.0f, -2.35f, 2.2f), Vec3(3.80f, 3.80f, 3.80f), 0.70f);
            drawObjModel(objCoral, uniforms, cM, coralTex, 0.90f, 0.95f, 1.0f, 1.0f);
        }

        // 3. Cá Mập Voi khổng lồ 3D (whale_shark.obj) + Shader uốn đuôi uSwim
        float t = (float)curTime;
        if (sharkLoaded) {
            float sPhase = t * 0.24f;
            float sx = 0.0f + 2.25f * std::sin(sPhase);
            float sz = -2.85f + 0.18f * std::sin(sPhase * 0.7f);
            float sy = 1.50f + 0.12f * std::sin(t * 0.70f);
            bool sLeft = std::cos(sPhase) < 0.0f;

            Mat4 sMat = modelMatrix(Vec3(sx, sy, sz), Vec3(0.95f, 0.95f, 0.95f), sLeft ? PI : 0.0f);
            drawObjModel(objShark, uniforms, sMat, sharkTex, 1.0f, 1.0f, 1.0f, 1.0f, /*swim=*/true);
        }

        // 3b. Cá đuối đại bàng dùng model và texture 2K trong MTL; lượn ngang, vẫy cánh.
        if (rayLoaded) {
            float rayPhase = t * 0.20f;
            float rayX = 0.30f + 1.90f * std::sin(rayPhase);
            float rayY = 0.48f + 0.10f * std::sin(t * 0.58f);
            float rayZ = -4.80f + 0.10f * std::sin(rayPhase * 0.65f);
            // Xoay chéo mặt về phía kính để thấy rõ hai cánh đốm của cá đuối.
            float rayYaw = 1.20f + 0.18f * std::sin(rayPhase);
            Mat4 rayMat = modelMatrix(Vec3(rayX, rayY, rayZ),
                                      Vec3(0.90f, 0.90f, 0.90f), rayYaw);
            drawObjModel(objRay, uniforms, rayMat, 0, 1.0f, 1.0f, 1.0f, 1.0f,
                         /*swimMode=*/4.0f);
        }
        // 4. Cá hề mô hình OBJ có sẵn trong dự án
        if (fishLoaded) {
            float fPhase = t * 0.55f;
            float fx = 2.7f * std::sin(fPhase);
            float fy = -1.42f + 0.10f * std::sin(t * 1.1f);
            float fz = 2.15f;
            float fishHeading = std::cos(fPhase) >= 0.0f ? PI * 0.5f : -PI * 0.5f;
            Mat4 fMat = modelMatrix(Vec3(fx, fy, fz), Vec3(10.0f, 10.0f, 10.0f), fishHeading);
            drawObjModel(objFish, uniforms, fMat, fishTex, 1.0f, 1.0f, 1.0f, 1.0f);
        }

        // 5. Cá tải về có texture: barramundi và cá nóc.
        // Cá bơi theo làn ngang trong vùng nhìn rõ của camera.
        struct FishTrack {
            int species;
            float centerX, y, z, scale, speed, phase, range, forwardYaw, tailDirection;
        };
        const FishTrack fishTracks[] = {
            // Khronos Barramundi GLB có trục dọc Z và node quay 180 độ.
            {0, -2.20f, 0.48f, -1.35f, 2.55f, 0.31f, 0.20f, 1.05f, -PI * 0.5f, 1.0f},
            {1, 0.00f, -0.30f, 0.15f, 3.10f, 0.39f, 2.20f, 1.00f, PI * 0.5f, -1.0f},
            // Cá mahi mahi bơi ngang ở tầng giữa, tách khỏi đường cá hề.
            {2, 2.20f, 0.28f, 2.25f, 1.30f, 0.31f, 1.10f, 0.82f, PI * 0.5f, -1.0f},
            // Hải long lá và cá ngựa giữ dáng tự nhiên, chỉ trôi nhẹ theo làn.
            {3, -2.85f, -0.30f, -3.55f, 4.60f, 0.24f, 2.80f, 0.50f, PI * 0.5f, 0.0f},
            {4, 2.85f, -0.92f, -2.45f, 4.80f, 0.20f, 5.10f, 0.42f, PI * 0.5f, 0.0f}
        };
        const int fishTrackCount = (int)(sizeof(fishTracks) / sizeof(fishTracks[0]));
        for (int i = 0; i < fishTrackCount; ++i) {
            const FishTrack& track = fishTracks[i];
            if (!speciesLoaded[track.species]) continue;
            float phase = t * track.speed + track.phase;
            float x = track.centerX + track.range * std::sin(phase);
            float y = track.y + 0.09f * std::sin(t * 1.25f + track.phase);
            float heading = track.forwardYaw + (std::cos(phase) >= 0.0f ? 0.0f : PI);
            Mat4 fishMat = modelMatrix(Vec3(x, y, track.z),
                                       Vec3(track.scale, track.scale, track.scale), heading);
            float swimMode = track.tailDirection == 0.0f ? 0.0f
                           : (track.tailDirection > 0.0f ? 3.0f : 2.0f);
            float brightness = track.species == 0 ? 1.65f
                             : (track.species == 2 ? 1.18f
                             : (track.species == 3 ? 1.35f
                             : (track.species == 4 ? 1.18f : 1.0f)));
            float tintR = brightness, tintG = brightness, tintB = brightness;
            if (track.species == 2) {
                // Ánh vàng nhẹ giúp thân mahi mahi dễ nhận ra trong nước xanh.
                tintR = 1.35f;
                tintG = 1.20f;
                tintB = 0.95f;
            }
            drawObjModel(downloadedFish[track.species], uniforms, fishMat,
                         0, tintR, tintG, tintB, 1.0f, swimMode);
        }

        // Một cụm năm con sứa trôi chậm ở tầng nước phía trước rạn.
        if (jellyfishLoaded) {
            float jellyX = -2.15f + 0.30f * std::sin(t * 0.22f);
            float jellyY = 1.48f + 0.10f * std::sin(t * 0.65f);
            float jellyYaw = 0.12f * std::sin(t * 0.30f);
            Mat4 jellyMat = modelMatrix(Vec3(jellyX, jellyY, 0.75f),
                                        Vec3(0.50f, 0.50f, 0.50f), jellyYaw);
            // Ánh xanh trắng và tự phát sáng nhẹ để sứa nổi trên nền nước.
            drawObjModel(jellyfishBloom, uniforms, jellyMat, 0,
                         1.30f, 1.30f, 1.35f, 1.0f, 0.0f, 1.10f);
        }
        // 7. Hệ hạt Bọt Khí (ParticleSystem - 80 hạt nổi lên từ san hô)
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthMask(GL_FALSE);
        for (int i = 0; i < particles.count(); ++i) {
            float px, py, pz, prad;
            particles.getParticle(i, t, px, py, pz, prad);
            Mat4 bubble = modelMatrix(Vec3(px, py, pz), Vec3(prad, prad, prad));
            drawMeshShape(sphere, uniforms, bubble, 0.62f, 0.88f, 0.96f, 0.55f);
        }
        // 8. Bề mặt nước và kính acrylic mỏng để minh họa Alpha Blending.
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthMask(GL_FALSE);
        Mat4 waterMat = multiply(translation(Vec3(0.0f, 2.55f, 0.0f)),
                                 multiply(rotationX(-PI * 0.5f), scaling(Vec3(12.0f, 12.0f, 1.0f))));
        drawMeshShape(glassQuad, uniforms, waterMat, 0.18f, 0.62f, 0.74f, 0.13f);
        Mat4 glassMat = modelMatrix(Vec3(0.0f, -0.05f, 4.45f), Vec3(12.0f, 5.80f, 1.0f));
        drawMeshShape(glassQuad, uniforms, glassMat, 0.24f, 0.76f, 0.88f, 0.16f);
        glDepthMask(GL_TRUE);
        glBindVertexArray(0);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Giải phóng tài nguyên Bạn B
    cube.destroy();
    sphere.destroy();
    glassQuad.destroy();
    objShark.destroy();
    objFish.destroy();
    objCoral.destroy();
    objRay.destroy();
    for (int i = 0; i < downloadedFishCount; ++i) downloadedFish[i].destroy();
    jellyfishBloom.destroy();
    if (sharkTex) glDeleteTextures(1, &sharkTex);
    if (coralTex) glDeleteTextures(1, &coralTex);
    if (fishTex)  glDeleteTextures(1, &fishTex);
    shader.destroy();

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
