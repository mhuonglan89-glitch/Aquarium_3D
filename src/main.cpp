// ==============================================================================
// ĐỒ ÁN MÔ PHỎNG THỦY CUNG 3D TƯƠNG TÁC (COMPUTER GRAPHICS)
// TÍCH HỢP TOÀN DIỆN HAI PHÂN HỆ:
//
// [BẠN A] - BACKEND, HÌNH HỌC & CAMERA:
//   1. Hầm kính Acrylic bán nguyệt tham số (CG.5: Curves & Surfaces)
//   2. Dầm thép chịu lực Titanium Steel & Dải đèn LED neon xanh cyan (#0CE8DA)
//   3. Hệ thống Camera kép 6-DOF & Thuật toán LERP chuyển cảnh Xuyên Kính (SPACE)
//   4. Quỹ đạo bơi Bézier 3D bậc 3 (CG.5: B(t) & tiếp tuyến Tangent định hướng)
//
// [BẠN B] - SHADERS, VẬT LIỆU & TÀI NGUYÊN 3D:
//   1. Shaders: ocean_fog.vs, ocean_fog.fs (Blinn-Phong, Caustics, Suong mu bien sau)
//   2. Vertex Shader uốn vây uốn đuôi cá sinh động (uSwim: tail wave & wing flap)
//   3. Đầy đủ các sinh vật biển 3D: Cá mập voi, Cá đuối đốm, Cá hề, 3 rạn san hô,
//      đàn sứa phát sáng, cá chẽm Barramundi, cá nóc Pufferfish, cá nheo Mahi Mahi,
//      hải long lá Seadragon, cá ngựa Seahorse.
//   4. Hệ hạt bọt khí nổi li ti (ParticleSystem) từ san hô
//   5. Khử mặt khuất Z-Buffer, Alpha Blending kính Acrylic & nước biển
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
// Toán học Vector & Ma trận cơ bản (Chương CG.3 & CG.4)
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
// [BẠN A] Thuật toán Đường cong Bézier 3D bậc 3 (Chương CG.5)
// B(t) = (1-t)^3 * P0 + 3(1-t)^2 * t * P1 + 3(1-t) * t^2 * P2 + t^3 * P3
// -----------------------------------------------------------------------------
struct Bezier3D {
    Vec3 p0, p1, p2, p3;
    Bezier3D() {}
    Bezier3D(const Vec3& a, const Vec3& b, const Vec3& c, const Vec3& d)
        : p0(a), p1(b), p2(c), p3(d) {}

    Vec3 getPoint(float t) const {
        float u = 1.0f - t;
        float tt = t * t, uu = u * u;
        float uuu = uu * u, ttt = tt * t;
        return Vec3(
            uuu * p0.x + 3.0f * uu * t * p1.x + 3.0f * u * tt * p2.x + ttt * p3.x,
            uuu * p0.y + 3.0f * uu * t * p1.y + 3.0f * u * tt * p2.y + ttt * p3.y,
            uuu * p0.z + 3.0f * uu * t * p1.z + 3.0f * u * tt * p2.z + ttt * p3.z
        );
    }

    // Đạo hàm bậc 1 tính Vector tiếp tuyến (Tangent) để xoay đầu cá theo hướng bơi
    Vec3 getTangent(float t) const {
        float u = 1.0f - t;
        Vec3 d(
            3.0f * u * u * (p1.x - p0.x) + 6.0f * u * t * (p2.x - p1.x) + 3.0f * t * t * (p3.x - p2.x),
            3.0f * u * u * (p1.y - p0.y) + 6.0f * u * t * (p2.y - p1.y) + 3.0f * t * t * (p3.y - p2.y),
            3.0f * u * u * (p1.z - p0.z) + 6.0f * u * t * (p2.z - p1.z) + 3.0f * t * t * (p3.z - p2.z)
        );
        return normalize(d);
    }

    // Góc Yaw để đầu cá quay theo tiếp tuyến trên mặt phẳng ngang XZ
    float getFacingAngle(float t) const {
        Vec3 tan = getTangent(t);
        return std::atan2(-tan.z, tan.x);
    }

    // Góc Pitch để đầu cá nghiêng lên/xuống khi lặn
    float getPitchAngle(float t) const {
        Vec3 tan = getTangent(t);
        float horizLen = std::sqrt(tan.x * tan.x + tan.z * tan.z);
        return std::atan2(tan.y, horizLen);
    }
};

// -----------------------------------------------------------------------------
// Hình học cơ bản & [BẠN A] Hầm kính Thủy cung (Chương CG.5)
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
// [BẠN A] Sinh lưới Vòm Kính Acrylic & Kết cấu Hầm bằng Toán học tham số
// x = R*cos(theta), y = centerY + R*sin(theta), z in [zStart, zStart + length]
// -----------------------------------------------------------------------------
static void createTunnelGlass(Mesh& mesh, float radius, float length, float centerY, float zStart, int segs = 32, int stepsZ = 24) {
    std::vector<Vertex> verts;
    float stepZ = length / (float)stepsZ;

    for (int i = 0; i < segs; ++i) {
        float theta1 = PI * (float)i / (float)segs;
        float theta2 = PI * (float)(i + 1) / (float)segs;

        float x1 = radius * std::cos(theta1);
        float y1 = centerY + radius * std::sin(theta1);
        float x2 = radius * std::cos(theta2);
        float y2 = centerY + radius * std::sin(theta2);

        Vec3 n1(std::cos(theta1), std::sin(theta1), 0.0f);
        Vec3 n2(std::cos(theta2), std::sin(theta2), 0.0f);

        for (int j = 0; j < stepsZ; ++j) {
            float z1 = zStart + (float)j * stepZ;
            float z2 = z1 + stepZ;

            Vec3 v1(x1, y1, z1);
            Vec3 v2(x2, y2, z1);
            Vec3 v3(x1, y1, z2);
            Vec3 v4(x2, y2, z2);

            // Mặt ngoài
            addTriangle(verts, v1, v2, v3, n1);
            addTriangle(verts, v2, v4, v3, n2);

            // Mặt trong (nhìn từ trong hầm ra ngoài đại dương)
            Vec3 inN1 = multiply(n1, -1.0f);
            Vec3 inN2 = multiply(n2, -1.0f);
            addTriangle(verts, v1, v3, v2, inN1);
            addTriangle(verts, v2, v3, v4, inN2);
        }
    }
    mesh.upload(verts);
}

static void createTunnelFloor(Mesh& mesh, float width, float length, float y, float zStart, int stepsZ = 24) {
    std::vector<Vertex> verts;
    float stepZ = length / (float)stepsZ;
    Vec3 n(0.0f, 1.0f, 0.0f);

    for (int j = 0; j < stepsZ; ++j) {
        float z1 = zStart + (float)j * stepZ;
        float z2 = z1 + stepZ;

        Vec3 v1(-width, y, z1);
        Vec3 v2( width, y, z1);
        Vec3 v3(-width, y, z2);
        Vec3 v4( width, y, z2);

        addTriangle(verts, v1, v2, v3, n);
        addTriangle(verts, v2, v4, v3, n);
    }
    mesh.upload(verts);
}

static void createTunnelRibs(Mesh& mesh, float radius, float length, float centerY, float zStart, int segs = 32, float spacing = 1.25f, float ribThickness = 0.045f) {
    std::vector<Vertex> verts;
    int ribCount = (int)(length / spacing);

    for (int r = 0; r <= ribCount; ++r) {
        float ribZ = zStart + (float)r * spacing;
        float zA = ribZ - ribThickness;
        float zB = ribZ + ribThickness;

        for (int i = 0; i < segs; ++i) {
            float theta1 = PI * (float)i / (float)segs;
            float theta2 = PI * (float)(i + 1) / (float)segs;

            float rOuter = radius + 0.038f;
            float rInner = radius - 0.025f;

            float x1_out = rOuter * std::cos(theta1);
            float y1_out = centerY + rOuter * std::sin(theta1);
            float x2_out = rOuter * std::cos(theta2);
            float y2_out = centerY + rOuter * std::sin(theta2);

            float x1_in = rInner * std::cos(theta1);
            float y1_in = centerY + rInner * std::sin(theta1);
            float x2_in = rInner * std::cos(theta2);
            float y2_in = centerY + rInner * std::sin(theta2);

            Vec3 n1(std::cos(theta1), std::sin(theta1), 0.0f);
            Vec3 n2(std::cos(theta2), std::sin(theta2), 0.0f);

            // Mặt ngoài dầm thép
            addTriangle(verts, Vec3(x1_out, y1_out, zA), Vec3(x2_out, y2_out, zA), Vec3(x1_out, y1_out, zB), n1);
            addTriangle(verts, Vec3(x2_out, y2_out, zA), Vec3(x2_out, y2_out, zB), Vec3(x1_out, y1_out, zB), n2);

            // Mặt bên hướng -Z
            Vec3 sideN(0.0f, 0.0f, -1.0f);
            addTriangle(verts, Vec3(x1_in, y1_in, zA), Vec3(x1_out, y1_out, zA), Vec3(x2_in, y2_in, zA), sideN);
            addTriangle(verts, Vec3(x1_out, y1_out, zA), Vec3(x2_out, y2_out, zA), Vec3(x2_in, y2_in, zA), sideN);

            // Mặt bên hướng +Z
            Vec3 sideP(0.0f, 0.0f, 1.0f);
            addTriangle(verts, Vec3(x1_in, y1_in, zB), Vec3(x2_in, y2_in, zB), Vec3(x1_out, y1_out, zB), sideP);
            addTriangle(verts, Vec3(x1_out, y1_out, zB), Vec3(x2_in, y2_in, zB), Vec3(x2_out, y2_out, zB), sideP);
        }
    }
    mesh.upload(verts);
}

static void createTunnelLeds(Mesh& mesh, float edgeX, float length, float y, float zStart, float ledWidth = 0.07f, int stepsZ = 24) {
    std::vector<Vertex> verts;
    float stepZ = length / (float)stepsZ;
    Vec3 n(0.0f, 1.0f, 0.0f);
    float ledY = y + 0.012f;

    for (int j = 0; j < stepsZ; ++j) {
        float z1 = zStart + (float)j * stepZ;
        float z2 = z1 + stepZ;

        // Dải LED bên trái
        Vec3 l1(-edgeX, ledY, z1);
        Vec3 l2(-edgeX + ledWidth, ledY, z1);
        Vec3 l3(-edgeX, ledY, z2);
        Vec3 l4(-edgeX + ledWidth, ledY, z2);
        addTriangle(verts, l1, l2, l3, n);
        addTriangle(verts, l2, l4, l3, n);

        // Dải LED bên phải
        Vec3 r1(edgeX - ledWidth, ledY, z1);
        Vec3 r2(edgeX, ledY, z1);
        Vec3 r3(edgeX - ledWidth, ledY, z2);
        Vec3 r4(edgeX, ledY, z2);
        addTriangle(verts, r1, r2, r3, n);
        addTriangle(verts, r2, r4, r3, n);
    }
    mesh.upload(verts);
}

// -----------------------------------------------------------------------------
// [BẠN B] Nạp Textures & Thiết lập Uniforms Shaders
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
                          float caustics = 0.0f, float emission = 0.0f) {
    glVertexAttrib4f(3, 1.0f, 1.0f, 1.0f, 1.0f);
    glUniformMatrix4fv(u.model, 1, GL_FALSE, mat.v);
    glUniform4f(u.tint, r, g, b, a);
    glUniform1f(u.pattern, 0.0f);
    glUniform1f(u.caustics, caustics);
    glUniform1f(u.useTexture, 0.0f);
    glUniform1f(u.swim, 0.0f);
    glUniform1f(u.emission, emission);
    glBindVertexArray(mesh.vao);
    glDrawArrays(GL_TRIANGLES, 0, mesh.vertexCount);
    glBindVertexArray(0);
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
// [BẠN A] Hệ thống Camera kép & Thuật toán LERP chuyển cảnh (Chương CG.4)
// - Chế độ 1: Bước đi trên sàn hầm kính (Mode 1 - Tunnel Walkthrough)
// - Chế độ 2: Lướt xuyên kính lặn tự do 6-DOF (Mode 2 - Ocean Dive)
// - Bấm SPACE: Kích hoạt LERP P(t) = (1-t)P1 + t*P2 mượt mà
// -----------------------------------------------------------------------------
static bool isDiving = false;
static Vec3 camPos(0.0f, -0.25f, 7.8f);
static Vec3 targetCamPos(0.0f, -0.25f, 7.8f);
static float camFov = 50.0f;
static float targetFov = 50.0f;

static float camYaw = -90.0f, camPitch = -3.0f;
static float lastX = 500.0f, lastY = 350.0f;
static bool firstMouse = true;
static bool spaceWasDown = false;
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

    // Phím SPACE: Chuyển đổi giữa Chế độ 1 (Trong hầm) và Chế độ 2 (Lặn biển)
    bool spaceDown = (glfwGetKey(win, GLFW_KEY_SPACE) == GLFW_PRESS);
    if (spaceDown && !spaceWasDown) {
        isDiving = !isDiving;
        if (isDiving) {
            targetCamPos = Vec3(0.0f, 0.40f, 3.2f); // Lướt xuyên kính ra ngoài đại dương
            targetFov = 68.0f;                      // Mở rộng góc mở FOV điện ảnh
            std::cout << "[CAMERA] >> CHE DO 2: LAN BIEN TU DO (100% Khong kinh - WASD + E/Q)" << std::endl;
        } else {
            targetCamPos = Vec3(0.0f, -0.25f, 7.8f); // Thu về giữa hành lang hầm kính
            targetFov = 50.0f;
            std::cout << "[CAMERA] >> CHE DO 1: BUOC DI TRONG HAM KINH THUY CUNG (WASD)" << std::endl;
        }
    }
    spaceWasDown = spaceDown;

    // Phím F: Bật/Tắt Sương mù đại dương sâu
    bool fDown = glfwGetKey(win, GLFW_KEY_F) == GLFW_PRESS;
    if (fDown && !fogWasDown) {
        fogEnabled = !fogEnabled;
        std::cout << "[TOGGLE] Suong mu bien sau: " << (fogEnabled ? "BAT" : "TAT") << std::endl;
    }
    fogWasDown = fDown;

    // Phím C: Bật/Tắt Khúc xạ tia nắng đáy biển (Caustics)
    bool cDown = glfwGetKey(win, GLFW_KEY_C) == GLFW_PRESS;
    if (cDown && !causticsWasDown) {
        causticsEnabled = !causticsEnabled;
        std::cout << "[TOGGLE] Khuc xa tia nang (Caustics): " << (causticsEnabled ? "BAT" : "TAT") << std::endl;
    }
    causticsWasDown = cDown;

    // Thuật toán LERP mượt mà cho vị trí và góc mở FOV
    camPos = add(camPos, multiply(subtract(targetCamPos, camPos), dt * 3.5f));
    camFov += (targetFov - camFov) * dt * 3.5f;

    Vec3 front = getCamFront();
    Vec3 hFront = normalize(Vec3(front.x, 0.0f, front.z));
    Vec3 right = normalize(cross(hFront, Vec3(0, 1, 0)));
    float speed = 3.5f * dt;

    if (!isDiving) {
        // Chế độ 1: Bước đi trên sàn hầm kính
        if (glfwGetKey(win, GLFW_KEY_W) == GLFW_PRESS) targetCamPos = add(targetCamPos, multiply(hFront, speed));
        if (glfwGetKey(win, GLFW_KEY_S) == GLFW_PRESS) targetCamPos = subtract(targetCamPos, multiply(hFront, speed));
        if (glfwGetKey(win, GLFW_KEY_A) == GLFW_PRESS) targetCamPos = subtract(targetCamPos, multiply(right, speed));
        if (glfwGetKey(win, GLFW_KEY_D) == GLFW_PRESS) targetCamPos = add(targetCamPos, multiply(right, speed));

        // Giữ tầm mắt cố định trên sàn hầm và giới hạn trong vòm kính
        targetCamPos.y = -0.25f;
        if (targetCamPos.x < -1.25f) targetCamPos.x = -1.25f;
        if (targetCamPos.x >  1.25f) targetCamPos.x =  1.25f;
        if (targetCamPos.z <  4.50f) targetCamPos.z =  4.50f;
        if (targetCamPos.z >  9.00f) targetCamPos.z =  9.00f;
    } else {
        // Chế độ 2: Bơi lội 6-DOF tự do trong lòng đại dương
        if (glfwGetKey(win, GLFW_KEY_W) == GLFW_PRESS) targetCamPos = add(targetCamPos, multiply(front, speed));
        if (glfwGetKey(win, GLFW_KEY_S) == GLFW_PRESS) targetCamPos = subtract(targetCamPos, multiply(front, speed));
        if (glfwGetKey(win, GLFW_KEY_A) == GLFW_PRESS) targetCamPos = subtract(targetCamPos, multiply(right, speed));
        if (glfwGetKey(win, GLFW_KEY_D) == GLFW_PRESS) targetCamPos = add(targetCamPos, multiply(right, speed));
        if (glfwGetKey(win, GLFW_KEY_E) == GLFW_PRESS) targetCamPos.y += speed; // Bơi ngoi lên
        if (glfwGetKey(win, GLFW_KEY_Q) == GLFW_PRESS) targetCamPos.y -= speed; // Lặn xuống sâu

        // Giới hạn trong không gian đại dương mô phỏng
        if (targetCamPos.y < -2.20f) targetCamPos.y = -2.20f;
        if (targetCamPos.y >  2.40f) targetCamPos.y =  2.40f;
        if (targetCamPos.x < -6.50f) targetCamPos.x = -6.50f;
        if (targetCamPos.x >  6.50f) targetCamPos.x =  6.50f;
        if (targetCamPos.z < -6.00f) targetCamPos.z = -6.00f;
        if (targetCamPos.z >  5.00f) targetCamPos.z =  5.00f;
    }
}

// -----------------------------------------------------------------------------
// HÀM MAIN: TỔNG HỢP VÀ KHỞI CHẠY ĐỒ ÁN
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
        1200, 750,
        "MOH HINH THUY CUNG 3D - INTERACTIVE 3D AQUARIUM [Ban A: Tunnel & Camera | Ban B: Models & Shaders]",
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

    // 1. Biên dịch Shader của Bạn B (ocean_fog.vs, ocean_fog.fs)
    Shader shader;
    if (!shader.load("assets/shaders/ocean_fog.vs", "assets/shaders/ocean_fog.fs")) {
        std::cerr << "Loi bien dich Shaders Bien Sau." << std::endl;
        return 1;
    }
    GLuint prog = shader.id();
    ShaderUniforms uniforms = getUniforms(prog);

    // 2. Tạo hình học môi trường & [BẠN A] Hầm Kính Thủy Cung
    Mesh cube, sphere, glassQuad;
    createCube(cube);
    createSphere(sphere);
    createGlassQuad(glassQuad);

    // [BẠN A] Khởi tạo Vòm kính hầm Acrylic, dầm thép, dải LED và sàn đi bộ
    Mesh tunnelGlass, tunnelFloor, tunnelRibs, tunnelLeds;
    float tunnelRadius = 2.40f;
    float tunnelLength = 5.20f;
    float tunnelCenterY = -1.25f;
    float tunnelZStart = 4.20f;
    createTunnelGlass(tunnelGlass, tunnelRadius, tunnelLength, tunnelCenterY, tunnelZStart);
    createTunnelFloor(tunnelFloor, 1.70f, tunnelLength, tunnelCenterY, tunnelZStart);
    createTunnelRibs(tunnelRibs, tunnelRadius, tunnelLength, tunnelCenterY, tunnelZStart);
    createTunnelLeds(tunnelLeds, 1.66f, tunnelLength, tunnelCenterY, tunnelZStart);

    // [BẠN A] Quỹ đạo Bézier 3D bậc 3 lượn vòng cung cho Cá mập voi
    static Bezier3D sharkCurveA(
        Vec3( 3.8f, 1.45f, -2.6f),
        Vec3( 1.5f, 1.85f,  1.2f),
        Vec3(-1.8f, 1.70f,  1.4f),
        Vec3(-3.8f, 1.35f, -2.2f)
    );
    static Bezier3D sharkCurveB(
        Vec3(-3.8f, 1.35f, -2.2f),
        Vec3(-2.2f, 1.15f, -4.6f),
        Vec3( 2.2f, 1.25f, -4.4f),
        Vec3( 3.8f, 1.45f, -2.6f)
    );

    // [BẠN A] Quỹ đạo Bézier 3D bậc 3 cho Cá đuối đại bàng lướt qua rạn san hô
    static Bezier3D rayCurveA(
        Vec3(-2.8f, 0.52f, -4.6f),
        Vec3(-1.0f, 0.68f, -2.2f),
        Vec3( 1.2f, 0.65f, -2.4f),
        Vec3( 2.8f, 0.48f, -4.8f)
    );
    static Bezier3D rayCurveB(
        Vec3( 2.8f, 0.48f, -4.8f),
        Vec3( 1.5f, 0.38f, -5.6f),
        Vec3(-1.5f, 0.40f, -5.5f),
        Vec3(-2.8f, 0.52f, -4.6f)
    );

    // Hệ hạt bọt khí của Bạn B (80 bọt khí nổi từ san hô)
    ParticleSystem particles;

    // 3. Nạp Model 3D thực tế của Bạn B
    Model objShark, objFish, objCoral, objRay;
    bool sharkLoaded = objShark.loadOBJ("assets/models/whale_shark.obj");
    bool fishLoaded  = objFish.loadOBJ("assets/models/clownfish.obj");
    bool coralLoaded = objCoral.loadOBJ("assets/models/coral_reef.obj");
    bool rayLoaded   = objRay.loadOBJ("assets/models/spotted_eagle_ray.obj");

    if (sharkLoaded) std::cout << "[OK] Nap model OBJ: whale_shark.obj" << std::endl;
    if (fishLoaded)  std::cout << "[OK] Nap model OBJ: clownfish.obj" << std::endl;
    if (coralLoaded) std::cout << "[OK] Nap model OBJ: coral_reef.obj" << std::endl;
    if (rayLoaded)   std::cout << "[OK] Nap model OBJ: spotted_eagle_ray.obj" << std::endl;

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
            std::cout << "[OK] Nap model GLB: " << speciesNames[i] << ".glb" << std::endl;
    }
    Model jellyfishBloom;
    bool jellyfishLoaded = jellyfishBloom.loadGLB("assets/models/real_fish/JellyfishBloom.glb");
    if (jellyfishLoaded)
        std::cout << "[OK] Nap dan sua GLB: JellyfishBloom.glb" << std::endl;

    // Nạp Textures 2K chuẩn
    GLuint sharkTex = loadTextureImage("assets/textures/shark_diffuse.jpg");
    GLuint coralTex = loadTextureImage("assets/models/coral-reef_body_basecolor.jpg");
    GLuint fishTex  = loadTextureImage("assets/textures/clownfish_2k.ppm");

    std::cout << "\n======================================================================\n";
    std::cout << "  3D AQUARIUM SIMULATION ENGINE DA KHOI DONG THANH CONG!\n";
    std::cout << "  TICH HOP TOAN DIEN BAN A & BAN B:\n";
    std::cout << "  - BAN A: Ham kinh Acrylic, Dan den LED, Camera LERP & Quy dao Bezier\n";
    std::cout << "  - BAN B: 8 Loai Ca 3D, San ho, Shaders uon duoi, Caustics & Suong mu\n";
    std::cout << "----------------------------------------------------------------------\n";
    std::cout << "  - Phim W, A, S, D : Buoc di trong ham kinh (Mode 1) / Boi 6-DOF (Mode 2)\n";
    std::cout << "  - Chuot           : Xoay goc nhin 360 do\n";
    std::cout << "  - Phim [SPACE]    : Chuyen che do Xuyen Kinh mượt mà (LERP)\n";
    std::cout << "  - Phim E / Q      : Ngoi len / Lan xuong (khi Lan bien)\n";
    std::cout << "  - Phim F          : Bat/Tat Suong mu bien sau (Fog)\n";
    std::cout << "  - Phim C          : Bat/Tat Khuc xa tia nang (Caustics)\n";
    std::cout << "  - Phim ESC        : Thoat chuong trinh\n";
    std::cout << "======================================================================\n\n";

    double prevTime = glfwGetTime();

    // -------------------------------------------------------------------------
    // VÒNG LẶP RENDER CHÍNH (RENDER LOOP)
    // -------------------------------------------------------------------------
    while (!glfwWindowShouldClose(window)) {
        double curTime = glfwGetTime();
        float dt = (float)(curTime - prevTime);
        prevTime = curTime;
        if (dt > 0.05f) dt = 0.05f;

        processInput(window, dt);

        int fbW = 0, fbH = 0;
        glfwGetFramebufferSize(window, &fbW, &fbH);
        glViewport(0, 0, fbW, fbH);

        // Màu nước xanh đại dương sâu thẳm
        Vec3 fogCol(0.035f, 0.32f, 0.45f);
        glClearColor(fogCol.x, fogCol.y, fogCol.z, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Thiết lập ma trận Camera View & Perspective Projection
        Mat4 view = lookAt(camPos, add(camPos, getCamFront()), Vec3(0, 1, 0));
        Mat4 proj = perspective(camFov * PI / 180.0f,
                                fbH > 0 ? (float)fbW / (float)fbH : 1.0f,
                                0.1f, 80.0f);

        glUseProgram(prog);
        glUniformMatrix4fv(uniforms.view, 1, GL_FALSE, view.v);
        glUniformMatrix4fv(uniforms.projection, 1, GL_FALSE, proj.v);
        glUniform3f(uniforms.viewPosition, camPos.x, camPos.y, camPos.z);
        glUniform3f(uniforms.fogColor, fogCol.x, fogCol.y, fogCol.z);
        glUniform1f(uniforms.fogDensity, fogEnabled ? 0.045f : 0.0f);
        glUniform1f(uniforms.time, (float)curTime);

        float t = (float)curTime;

        // ---------------------------------------------------------------------
        // 1. Đáy cát biển (Caustics khúc xạ tia nắng rọi qua nước)
        // ---------------------------------------------------------------------
        Mat4 seabed = modelMatrix(Vec3(0.0f, -2.55f, 0.5f), Vec3(22.0f, 0.35f, 18.0f));
        drawMeshShape(cube, uniforms, seabed, 0.45f, 0.35f, 0.22f, 1.0f, causticsEnabled ? 0.50f : 0.0f);

        // ---------------------------------------------------------------------
        // 2. Rạn đá và San hô 3D của Bạn B (coral_reef.obj)
        // ---------------------------------------------------------------------
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

        // ---------------------------------------------------------------------
        // 3. Cá Mập Voi bơi theo Quỹ đạo Bézier 3D của Bạn A + Shader uốn đuôi của Bạn B
        // ---------------------------------------------------------------------
        if (sharkLoaded) {
            float sharkT = std::fmod(t * 0.040f, 1.0f);
            Vec3 sPos;
            float sFacing = 0.0f;
            float sPitch = 0.0f;
            if (sharkT < 0.5f) {
                float u = sharkT * 2.0f;
                sPos = sharkCurveA.getPoint(u);
                sFacing = sharkCurveA.getFacingAngle(u);
                sPitch = sharkCurveA.getPitchAngle(u);
            } else {
                float u = (sharkT - 0.5f) * 2.0f;
                sPos = sharkCurveB.getPoint(u);
                sFacing = sharkCurveB.getFacingAngle(u);
                sPitch = sharkCurveB.getPitchAngle(u);
            }

            Mat4 sMat = translation(sPos);
            sMat = multiply(sMat, rotationY(sFacing));
            sMat = multiply(sMat, rotationZ(sPitch));
            sMat = multiply(sMat, scaling(Vec3(0.95f, 0.95f, 0.95f)));
            drawObjModel(objShark, uniforms, sMat, sharkTex, 1.0f, 1.0f, 1.0f, 1.0f, /*swim=*/1.0f);
        }

        // ---------------------------------------------------------------------
        // 4. Cá đuối đại bàng lướt theo Quỹ đạo Bézier 3D + Shader vẫy cánh của Bạn B
        // ---------------------------------------------------------------------
        if (rayLoaded) {
            float rayT = std::fmod(t * 0.048f, 1.0f);
            Vec3 rPos;
            float rFacing = 0.0f;
            if (rayT < 0.5f) {
                float u = rayT * 2.0f;
                rPos = rayCurveA.getPoint(u);
                rFacing = rayCurveA.getFacingAngle(u);
            } else {
                float u = (rayT - 0.5f) * 2.0f;
                rPos = rayCurveB.getPoint(u);
                rFacing = rayCurveB.getFacingAngle(u);
            }

            Mat4 rMat = translation(rPos);
            rMat = multiply(rMat, rotationY(rFacing));
            rMat = multiply(rMat, scaling(Vec3(0.90f, 0.90f, 0.90f)));
            drawObjModel(objRay, uniforms, rMat, 0, 1.0f, 1.0f, 1.0f, 1.0f, /*swimMode=*/4.0f);
        }

        // ---------------------------------------------------------------------
        // 5. Đàn cá hề bơi quanh rạn san hô
        // ---------------------------------------------------------------------
        if (fishLoaded) {
            float fPhase = t * 0.55f;
            float fx = 2.7f * std::sin(fPhase);
            float fy = -1.42f + 0.10f * std::sin(t * 1.1f);
            float fz = 2.15f;
            float fishHeading = std::cos(fPhase) >= 0.0f ? PI * 0.5f : -PI * 0.5f;
            Mat4 fMat = modelMatrix(Vec3(fx, fy, fz), Vec3(10.0f, 10.0f, 10.0f), fishHeading);
            drawObjModel(objFish, uniforms, fMat, fishTex, 1.0f, 1.0f, 1.0f, 1.0f);
        }

        // ---------------------------------------------------------------------
        // 6. Các loài cá GLB tải về (Barramundi, Cá nóc, Mahi Mahi, Seadragon, Seahorse)
        // ---------------------------------------------------------------------
        struct FishTrack {
            int species;
            float centerX, y, z, scale, speed, phase, range, forwardYaw, tailDirection;
        };
        const FishTrack fishTracks[] = {
            {0, -2.20f, 0.48f, -1.35f, 2.55f, 0.31f, 0.20f, 1.05f, -PI * 0.5f, 1.0f},
            {1,  0.00f,-0.30f,  0.15f, 3.10f, 0.39f, 2.20f, 1.00f,  PI * 0.5f,-1.0f},
            {2,  2.20f, 0.28f,  2.25f, 1.30f, 0.31f, 1.10f, 0.82f,  PI * 0.5f,-1.0f},
            {3, -2.85f,-0.30f, -3.55f, 4.60f, 0.24f, 2.80f, 0.50f,  PI * 0.5f, 0.0f},
            {4,  2.85f,-0.92f, -2.45f, 4.80f, 0.20f, 5.10f, 0.42f,  PI * 0.5f, 0.0f}
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
                tintR = 1.35f; tintG = 1.20f; tintB = 0.95f;
            }
            drawObjModel(downloadedFish[track.species], uniforms, fishMat,
                         0, tintR, tintG, tintB, 1.0f, swimMode);
        }

        // Đàn sứa phát sáng trôi dạt tự nhiên
        if (jellyfishLoaded) {
            float jellyX = -2.15f + 0.30f * std::sin(t * 0.22f);
            float jellyY = 1.48f + 0.10f * std::sin(t * 0.65f);
            float jellyYaw = 0.12f * std::sin(t * 0.30f);
            Mat4 jellyMat = modelMatrix(Vec3(jellyX, jellyY, 0.75f),
                                        Vec3(0.50f, 0.50f, 0.50f), jellyYaw);
            drawObjModel(jellyfishBloom, uniforms, jellyMat, 0,
                         1.30f, 1.30f, 1.35f, 1.0f, 0.0f, 1.10f);
        }

        // ---------------------------------------------------------------------
        // 7. Hệ hạt Bọt Khí của Bạn B (80 hạt nổi lên từ san hô)
        // ---------------------------------------------------------------------
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthMask(GL_FALSE);
        for (int i = 0; i < particles.count(); ++i) {
            float px, py, pz, prad;
            particles.getParticle(i, t, px, py, pz, prad);
            Mat4 bubble = modelMatrix(Vec3(px, py, pz), Vec3(prad, prad, prad));
            drawMeshShape(sphere, uniforms, bubble, 0.62f, 0.88f, 0.96f, 0.55f);
        }

        // ---------------------------------------------------------------------
        // 8. Bề mặt nước biển lấp lánh phía trên
        // ---------------------------------------------------------------------
        Mat4 waterMat = multiply(translation(Vec3(0.0f, 2.55f, 0.0f)),
                                 multiply(rotationX(-PI * 0.5f), scaling(Vec3(12.0f, 12.0f, 1.0f))));
        drawMeshShape(glassQuad, uniforms, waterMat, 0.18f, 0.62f, 0.74f, 0.13f);

        // ---------------------------------------------------------------------
        // 9. [BẠN A] HẦM KÍNH THỦY CUNG (TUNNEL WALKWAY & DOME)
        // Vẽ sàn tối màu, dải đèn LED cyan neon (#0CE8DA) phát sáng, dầm thép chịu lực,
        // và vòm kính Acrylic trong suốt (Chỉ hiển thị khi ở Chế độ 1 trong hầm kính)
        // ---------------------------------------------------------------------
        if (!isDiving || camPos.z > 4.2f) {
            Mat4 idMat = identityMatrix();

            // 9.1 Sàn hầm tối màu (Titanium Steel #1E293B)
            glDepthMask(GL_TRUE);
            drawMeshShape(tunnelFloor, uniforms, idMat, 0.12f, 0.16f, 0.22f, 1.0f);

            // 9.2 Dải đèn LED neon dẫn lối dọc mép sàn (Bioluminescent Cyan #0CE8DA)
            drawMeshShape(tunnelLeds, uniforms, idMat, 0.05f, 0.91f, 0.86f, 1.0f, 0.0f, /*emission=*/2.8f);

            // 9.3 Dầm thép chịu lực uốn cong đỡ vòm kính
            drawMeshShape(tunnelRibs, uniforms, idMat, 0.20f, 0.25f, 0.32f, 1.0f);

            // 9.4 Vòm kính Acrylic trong suốt (Alpha Blending, khử z-buffer conflict)
            glDepthMask(GL_FALSE);
            drawMeshShape(tunnelGlass, uniforms, idMat, 0.20f, 0.65f, 0.85f, 0.26f);
        }

        glDepthMask(GL_TRUE);
        glBindVertexArray(0);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Giải phóng tài nguyên
    cube.destroy();
    sphere.destroy();
    glassQuad.destroy();
    tunnelGlass.destroy();
    tunnelFloor.destroy();
    tunnelRibs.destroy();
    tunnelLeds.destroy();
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
