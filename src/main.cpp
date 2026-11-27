#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <cmath>
#include <ctime>
#include <cstdlib>

// Nạp các thư viện tự xây dựng của Thành viên A (Backend) và Thành viên B (Frontend)
#include "Shader.h"
#include "Camera.h"
#include "Model.h"
#include "BezierPath.h"
#include "Tunnel.h"
#include "ParticleSystem.h"

/**
 * ==============================================================================================
 * 📌 FILE: src/main.cpp (ĐIỂM CHẠM HỢP NHẤT TOÀN BỘ DỰ ÁN)
 * 🎓 MÔN HỌC: ĐỒ HỌA MÁY TÍNH (COMPUTER GRAPHICS)
 * 🏛️ ĐỀ TÀI: MÔ PHỎNG THỦY CUNG 3D TƯƠNG TÁC — TRẢI NGHIỆM ĐỘT PHÁ KÉP (DUAL-EXPERIENCE)
 * ==============================================================================================
 */

// ==============================================================================================
// 🎯 CỤM 1: CẤU HÌNH KÍCH THƯỚC CỬA SỔ & TRẠNG THÁI CAMERA TOÀN CỤC
// ==============================================================================================
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

Camera camera(glm::vec3(0.0f, 38.0f, -520.0f)); // Khởi tạo tại Lối vào chính (Nhánh Nam) nhìn về Đại Sảnh Trung Tâm
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

// ==============================================================================================
// 🎯 CỤM 2: HÀM SINH ĐÁY CÁT ĐẠI DƯƠNG NGOÀI HẦM KÍNH (SEABED WITH CAUSTICS)
// ==============================================================================================
Mesh createSeabedMesh() {
    std::vector<Vertex> verts;
    std::vector<unsigned int> inds;

    const int grid = 64;
    const float size = 4400.0f; // Phủ rộng [-2200, +2200]
    const float half = size * 0.5f;
    const float step = size / grid;

    auto getSandY = [](float x, float z) -> float {
        // Giữ đáy cát phẳng (-2.0f) dưới toàn bộ khu vực mê cung & đại sảnh, chỉ nhấp nhô cồn cát ở vùng biển xa
        float r = std::sqrt(x * x + z * z);
        bool nearCorridor = (std::abs(x) < 145.0f && z > -820.0f && z < 820.0f) ||
                            (std::abs(z) < 145.0f && x > -780.0f && x < 780.0f) ||
                            (std::abs(x + 640.0f) < 145.0f && z > -600.0f && z < 140.0f) ||
                            (std::abs(x - 640.0f) < 145.0f && z > -580.0f && z < 140.0f) ||
                            (std::abs(z - 680.0f) < 145.0f && x > -140.0f && x < 600.0f) ||
                            (r < 250.0f);
        if (nearCorridor) return -2.0f;
        float dune = sinf(x * 0.008f) * 9.0f + cosf(z * 0.007f) * 9.0f + sinf((x + z) * 0.015f) * 3.5f;
        return -2.0f + dune;
    };

    for (int iz = 0; iz <= grid; ++iz) {
        for (int ix = 0; ix <= grid; ++ix) {
            float x = -half + ix * step;
            float z = -half + iz * step;
            float y = getSandY(x, z);

            // Tính pháp tuyến bằng sai phân hữu hạn
            float hL = getSandY(x - 2.0f, z);
            float hR = getSandY(x + 2.0f, z);
            float hD = getSandY(x, z - 2.0f);
            float hU = getSandY(x, z + 2.0f);
            glm::vec3 normal = glm::normalize(glm::vec3(hL - hR, 4.0f, hD - hU));

            Vertex v;
            v.Position = glm::vec3(x, y, z);
            v.Normal = normal;
            v.TexCoords = glm::vec2(x * 0.01f, z * 0.01f);
            verts.push_back(v);
        }
    }

    for (int iz = 0; iz < grid; ++iz) {
        for (int ix = 0; ix < grid; ++ix) {
            unsigned int a = iz * (grid + 1) + ix;
            unsigned int b = a + 1;
            unsigned int c = a + (grid + 1);
            unsigned int d = c + 1;
            inds.push_back(a); inds.push_back(c); inds.push_back(b);
            inds.push_back(b); inds.push_back(c); inds.push_back(d);
        }
    }

    return Mesh(verts, inds);
}

// Sinh mặt nước biển gợn sóng lấp lánh ánh nắng ở trên cao (Y = 530)
Mesh createWaterSurfaceMesh() {
    std::vector<Vertex> verts;
    std::vector<unsigned int> inds;
    const int grid = 32;
    const float size = 4400.0f;
    const float half = size * 0.5f;
    const float step = size / grid;

    for (int iz = 0; iz <= grid; ++iz) {
        for (int ix = 0; ix <= grid; ++ix) {
            float x = -half + ix * step;
            float z = -half + iz * step;
            Vertex v;
            v.Position = glm::vec3(x, 530.0f, z);
            v.Normal = glm::vec3(0.0f, -1.0f, 0.0f);
            v.TexCoords = glm::vec2(x * 0.01f, z * 0.01f);
            verts.push_back(v);
        }
    }
    for (int iz = 0; iz < grid; ++iz) {
        for (int ix = 0; ix < grid; ++ix) {
            unsigned int a = iz * (grid + 1) + ix;
            unsigned int b = a + 1;
            unsigned int c = a + (grid + 1);
            unsigned int d = c + 1;
            inds.push_back(a); inds.push_back(b); inds.push_back(c);
            inds.push_back(b); inds.push_back(d); inds.push_back(c);
        }
    }
    return Mesh(verts, inds);
}

// ==============================================================================================
// 🎯 CỤM 3: CẤU TRÚC ĐÀN CÁ BƠI TỰ DO ĐA DẠNG KHẮP ĐẠI DƯƠNG (FREE-ROAMING FISH AGENT)
// ==============================================================================================
struct RoamingFish {
    Model* model;
    glm::vec3 center;      // Tâm vùng tuần tra riêng
    float radiusX;         // Biên độ bơi rộng theo X
    float radiusZ;         // Biên độ bơi rộng theo Z
    float baseY;           // Độ cao bơi trung bình
    float ampY;            // Biên độ nhấp nhô lên xuống
    float freqX;           // Tần số dao động X
    float freqZ;           // Tần số dao động Z
    float speed;           // Tốc độ bơi
    float phase;           // Pha khởi đầu riêng (để không bơi cùng 1 chỗ)
    float scale;           // Kích thước thực tế
    float yawOffsetDeg;    // Góc hiệu chỉnh hướng đầu model (nếu file .glb/.obj xoay khác +Z)
    bool isFishShader;     // Bật hiệu ứng uốn đuôi trên GPU
};

// Hàm giữ sinh vật luôn bơi ngoài tất cả các nhánh mê cung & Đại Sảnh Trung Tâm
static glm::vec3 keepOutsideTunnel(glm::vec3 p) {
    float rHub = std::sqrt(p.x * p.x + p.z * p.z);
    if (rHub < 245.0f && p.y < 255.0f) {
        p.y = 260.0f;
    } else {
        bool inBranch = (std::abs(p.x) < 135.0f && p.z > -810.0f && p.z < 810.0f) ||
                        (std::abs(p.z) < 135.0f && p.x > -770.0f && p.x < 770.0f) ||
                        (std::abs(p.x + 640.0f) < 135.0f && p.z > -590.0f && p.z < 135.0f) ||
                        (std::abs(p.x - 640.0f) < 135.0f && p.z > -570.0f && p.z < 135.0f) ||
                        (std::abs(p.z - 680.0f) < 135.0f && p.x > -135.0f && p.x < 590.0f);
        if (inBranch && p.y < 140.0f) {
            p.y = 148.0f;
        }
    }
    if (p.y < 18.0f) p.y = 18.0f;
    return p;
}

static void drawSplineCreature(Shader& shader, Model& model, const ClosedSplinePath& path,
                               float t, float scale, float currentTime, float phase,
                               float yawOffsetDeg = 0.0f, float rollAmp = 0.0f, bool wagTail = true) {
    glm::vec3 pos = path.GetPoint(t);
    float yaw = path.GetFacingAngle(t);
    float pitch = path.GetPitchAngle(t);

    glm::mat4 m = glm::mat4(1.0f);
    m = glm::translate(m, pos);
    m = glm::rotate(m, yaw + glm::radians(yawOffsetDeg), glm::vec3(0.0f, 1.0f, 0.0f));
    m = glm::rotate(m, pitch, glm::vec3(1.0f, 0.0f, 0.0f));
    if (rollAmp > 0.0f) {
        m = glm::rotate(m, sinf(currentTime * 3.0f + phase) * rollAmp, glm::vec3(0.0f, 0.0f, 1.0f));
    }
    m = glm::scale(m, glm::vec3(scale));

    shader.setBool("isFish", wagTail);
    shader.setFloat("fishPhase", phase);
    shader.setMat4("model", m);
    model.Draw(shader);
}

// ==============================================================================================
// 🚀 HÀM CHÍNH (MAIN FUNCTION)
// ==============================================================================================
int main() {
    srand(static_cast<unsigned int>(time(nullptr)));

    // ------------------------------------------------------------------------------------------
    // 🟢 BƯỚC 1: KHỞI TẠO CỬA SỔ GLFW & OPENGL 3.3 CORE PROFILE (CHƯƠNG CG.2)
    // ------------------------------------------------------------------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(
        SCR_WIDTH, SCR_HEIGHT,
        "3D Tropical Oceanarium - Thuy Cung Nhiet Doi (Do An Do Hoa May Tinh)", NULL, NULL);
    if (window == NULL) {
        std::cerr << "[ERROR] Khong the khoi tao cua so GLFW!" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "[ERROR] Khong the khoi tao GLAD!" << std::endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // ------------------------------------------------------------------------------------------
    // 🟢 BƯỚC 2: KHỞI TẠO SHADERS, ĐÁY BIỂN & ĐA DẠNG SINH VẬT SKETCHFAB (BẠN B PHỤ TRÁCH)
    // ------------------------------------------------------------------------------------------
    Shader oceanShader("assets/shaders/ocean_fog.vs", "assets/shaders/ocean_fog.fs");
    Mesh seabedMesh = createSeabedMesh();
    Mesh waterSurfaceMesh = createWaterSurfaceMesh();

    // Nạp đầy đủ các sinh vật biển 3D từ Sketchfab (.obj & .glb kèm texture)
    // Tham số thứ 4 (alignRotDeg) chuẩn hóa trục đầu về +Z và lưng về +Y để 100% cá bơi xuôi chiều!
    Model whaleModel      ("assets/models/whale.glb",                     glm::vec3(0.22f, 0.34f, 0.48f));
    Model whaleSharkModel ("assets/models/whale_shark.obj",               glm::vec3(0.32f, 0.44f, 0.56f), "assets/textures/shark_diffuse.jpg", glm::vec3(0.0f, -90.0f, 0.0f));
    Model sharkModel      ("assets/models/shark.glb",                     glm::vec3(0.45f, 0.52f, 0.60f), "", glm::vec3(-90.0f, 0.0f, 0.0f));
    Model eagleRayModel   ("assets/models/spotted_eagle_ray.obj",         glm::vec3(0.22f, 0.30f, 0.42f), "assets/textures/ray_texture.png", glm::vec3(0.0f, -90.0f, 0.0f));
    Model mantaRayModel   ("assets/models/manta_ray.glb",                 glm::vec3(0.18f, 0.24f, 0.34f));
    Model dolphinModel    ("assets/models/dolphin.glb",                   glm::vec3(0.45f, 0.62f, 0.78f));
    Model turtleModel     ("assets/models/turtle.glb",                    glm::vec3(0.28f, 0.52f, 0.32f));
    Model clownfishModel  ("assets/models/clownfish.obj",                 glm::vec3(0.98f, 0.45f, 0.08f), "assets/textures/clownfish.png");
    Model blueTangModel   ("assets/models/real_fish/BlueTang.glb",        glm::vec3(0.12f, 0.52f, 0.95f));
    Model mahiMahiModel   ("assets/models/real_fish/MahiMahi.glb",        glm::vec3(0.25f, 0.85f, 0.55f));
    Model tunaModel       ("assets/models/tuna.glb",                      glm::vec3(0.35f, 0.55f, 0.80f));
    Model barramundiModel ("assets/models/real_fish/BarramundiFish.glb",  glm::vec3(0.75f, 0.78f, 0.82f));
    Model pufferfishModel ("assets/models/real_fish/Pufferfish.glb",      glm::vec3(0.95f, 0.82f, 0.25f));
    Model jellyfishModel  ("assets/models/real_fish/JellyfishBloom.glb",  glm::vec3(0.92f, 0.48f, 0.95f));
    Model seahorseModel   ("assets/models/real_fish/Seahorse.glb",        glm::vec3(0.98f, 0.65f, 0.18f));
    Model seadragonModel  ("assets/models/real_fish/LeafySeadragon.glb",  glm::vec3(0.85f, 0.55f, 0.22f));
    Model coralReefModel  ("assets/models/coral_reef.obj",                glm::vec3(0.92f, 0.35f, 0.45f), "assets/models/coral-reef_body_basecolor.jpg");

    ParticleSystem bubbles(260);

    // ------------------------------------------------------------------------------------------
    // 🟢 BƯỚC 3: HẦM KÍNH ACRYLIC & CÁC QUỸ ĐẠO TUẦN TRA KHÉP KÍN RỘNG KHẮP BIỂN (BẠN A)
    // ------------------------------------------------------------------------------------------
    Tunnel glassTunnel(110.0f, 1600.0f);

    // 3.1. Quỹ đạo Cá voi khổng lồ (Vòng tuần tra đại dương rộng lớn tầng cao)
    ClosedSplinePath whalePath({
        glm::vec3(-620.0f, 310.0f, -650.0f),
        glm::vec3(   0.0f, 340.0f, -850.0f),
        glm::vec3( 620.0f, 300.0f, -450.0f),
        glm::vec3( 580.0f, 270.0f,  550.0f),
        glm::vec3(   0.0f, 320.0f,  880.0f),
        glm::vec3(-600.0f, 290.0f,  450.0f)
    });

    // 3.2. Quỹ đạo Cá mập voi & Cá mập trắng (Lượn sát trên nóc hầm kính & 2 bên vách kính)
    ClosedSplinePath whaleSharkPath({
        glm::vec3(-340.0f, 140.0f, -550.0f),
        glm::vec3( -90.0f, 235.0f, -150.0f), // Vượt ngang nóc kính để khách trong hầm ngước thấy!
        glm::vec3( 320.0f, 130.0f,  250.0f),
        glm::vec3( 380.0f, 110.0f,  720.0f),
        glm::vec3(   0.0f, 245.0f,  520.0f),
        glm::vec3(-390.0f, 125.0f,   80.0f)
    });

    // 3.3. Quỹ đạo Số 8 Cá mập đại dương
    ClosedSplinePath sharkPatrolPath({
        glm::vec3( 360.0f, 115.0f, -620.0f),
        glm::vec3(  60.0f, 225.0f, -200.0f),
        glm::vec3(-350.0f, 120.0f,  320.0f),
        glm::vec3(-420.0f, 150.0f,  750.0f),
        glm::vec3( 120.0f, 230.0f,  380.0f),
        glm::vec3( 440.0f,  95.0f,  -80.0f)
    });

    // 3.4. Quỹ đạo Cá đuối đốm & Cá đuối Manta (Lướt uốn lượn quanh Đại Sảnh Trung Tâm & các nhánh mê cung)
    ClosedSplinePath rayPath({
        glm::vec3(-340.0f, 165.0f, -340.0f),
        glm::vec3(   0.0f, 285.0f,    0.0f), // Lướt ngang qua nóc vòm Đại Sảnh Trung Tâm!
        glm::vec3( 360.0f, 155.0f,  340.0f),
        glm::vec3( 320.0f, 185.0f,  460.0f),
        glm::vec3(-280.0f, 235.0f,  320.0f),
        glm::vec3(-380.0f, 150.0f, -240.0f)
    });

    // 3.5. Quỹ đạo Đàn Cá heo (Bơi theo cung tròn nhấp nhô quanh 4 góc đại dương)
    ClosedSplinePath dolphinPath({
        glm::vec3( 350.0f, 180.0f, -360.0f),
        glm::vec3( 360.0f, 250.0f,  350.0f),
        glm::vec3(   0.0f, 290.0f,  480.0f),
        glm::vec3(-360.0f, 210.0f,  380.0f),
        glm::vec3(-360.0f, 155.0f, -320.0f),
        glm::vec3(   0.0f, 275.0f, -450.0f)
    });

    // 3.6. Quỹ đạo Rùa biển (Bơi thong thả quanh 4 khu vườn san hô bên ngoài Đại Sảnh)
    ClosedSplinePath turtlePath({
        glm::vec3(-350.0f,  75.0f, -340.0f),
        glm::vec3(-380.0f,  95.0f,  340.0f),
        glm::vec3(   0.0f, 270.0f,  380.0f),
        glm::vec3( 360.0f,  85.0f,  340.0f),
        glm::vec3( 360.0f,  75.0f, -320.0f),
        glm::vec3(   0.0f, 265.0f, -380.0f)
    });

    // 3.7. Khởi tạo 52 sinh vật bơi tự do đa vùng (Không dồn vào 1 chỗ, phân bố khắp 3D)
    std::vector<RoamingFish> roamingSchool;
    auto addSchoolFish = [&](Model* m, glm::vec3 center, float rx, float rz, float by, float ay,
                             float fx, float fz, float spd, float sc, float yawOff, bool isF) {
        RoamingFish rf;
        rf.model = m;
        rf.center = center;
        rf.radiusX = rx; rf.radiusZ = rz;
        rf.baseY = by;   rf.ampY = ay;
        rf.freqX = fx;   rf.freqZ = fz;
        rf.speed = spd;
        rf.phase = static_cast<float>(rand() % 628) * 0.01f;
        rf.scale = sc;
        rf.yawOffsetDeg = yawOff;
        rf.isFishShader = isF;
        roamingSchool.push_back(rf);
    };

    // Đàn Cá hề (10 con) rải khắp nhiều vùng bên trái, bên phải & phía trên hầm
    for (int i = 0; i < 10; ++i) {
        float side = (i % 2 == 0) ? 1.0f : -1.0f;
        addSchoolFish(&clownfishModel,
                      glm::vec3(side * (260.0f + (i % 3) * 85.0f), 0.0f, -550.0f + i * 125.0f),
                      130.0f, 210.0f, 55.0f + (i % 4) * 28.0f, 22.0f,
                      0.7f, 0.5f, 0.45f + (i % 3) * 0.1f, 22.0f, 0.0f, true);
    }

    // Đàn Cá đuôi gai xanh BlueTang (10 con) bơi lượn vòng rộng qua nóc hầm & 2 bên
    for (int i = 0; i < 10; ++i) {
        addSchoolFish(&blueTangModel,
                      glm::vec3(((i % 2 == 0) ? -220.0f : 220.0f), 0.0f, -480.0f + i * 110.0f),
                      260.0f, 320.0f, 120.0f + (i % 3) * 45.0f, 35.0f,
                      0.55f, 0.42f, 0.52f + (i % 4) * 0.08f, 26.0f, 0.0f, true);
    }

    // Đàn Cá nheo Mahi-Mahi & Cá ngừ đại dương (12 con) bơi tầm xa tốc độ cao
    for (int i = 0; i < 6; ++i) {
        addSchoolFish(&mahiMahiModel,
                      glm::vec3((i % 2 == 0 ? 320.0f : -320.0f), 0.0f, -400.0f + i * 180.0f),
                      380.0f, 450.0f, 150.0f + i * 25.0f, 40.0f,
                      0.45f, 0.65f, 0.65f + i * 0.05f, 42.0f, 0.0f, true);
        addSchoolFish(&tunaModel,
                      glm::vec3((i % 2 == 0 ? -360.0f : 360.0f), 0.0f, 400.0f - i * 170.0f),
                      420.0f, 480.0f, 185.0f + i * 20.0f, 38.0f,
                      0.6f, 0.4f, 0.72f + i * 0.04f, 46.0f, 0.0f, true);
    }

    // Cá chẽm Barramundi & Cá nóc Pufferfish (10 con)
    for (int i = 0; i < 5; ++i) {
        addSchoolFish(&barramundiModel,
                      glm::vec3((i % 2 == 0 ? 290.0f : -290.0f), 0.0f, -350.0f + i * 190.0f),
                      220.0f, 280.0f, 95.0f + i * 22.0f, 25.0f,
                      0.5f, 0.6f, 0.48f, 44.0f, 0.0f, true);
        addSchoolFish(&pufferfishModel,
                      glm::vec3((i % 2 == 0 ? -245.0f : 245.0f), 0.0f, -500.0f + i * 240.0f),
                      120.0f, 160.0f, 65.0f + i * 18.0f, 18.0f,
                      0.4f, 0.5f, 0.32f, 24.0f, 0.0f, false);
    }

    // Đàn Sứa phát sáng (10 con) trôi bồng bềnh tầng trung-cao, Cá ngựa & Hải long lá quanh san hô
    for (int i = 0; i < 10; ++i) {
        float side = (i % 2 == 0) ? 1.0f : -1.0f;
        addSchoolFish(&jellyfishModel,
                      glm::vec3(side * (230.0f + (i % 3) * 110.0f), 0.0f, -600.0f + i * 135.0f),
                      75.0f, 95.0f, 145.0f + (i % 4) * 35.0f, 45.0f,
                      0.25f, 0.2f, 0.22f, 40.0f, 0.0f, false);
    }
    for (int i = 0; i < 4; ++i) {
        float side = (i % 2 == 0) ? 1.0f : -1.0f;
        addSchoolFish(&seahorseModel,
                      glm::vec3(side * (240.0f + i * 40.0f), 0.0f, -380.0f + i * 250.0f),
                      65.0f, 80.0f, 48.0f + i * 12.0f, 14.0f,
                      0.35f, 0.4f, 0.25f, 26.0f, 0.0f, false);
        addSchoolFish(&seadragonModel,
                      glm::vec3(-side * (250.0f + i * 35.0f), 0.0f, -260.0f + i * 220.0f),
                      80.0f, 100.0f, 58.0f + i * 15.0f, 16.0f,
                      0.4f, 0.35f, 0.28f, 30.0f, 0.0f, true);
    }

    // 3.8. Rải 16 cụm rạn san hô tại 4 vùng biển mở xung quanh Đại Sảnh & 4 cánh mê cung (không đè vào hầm)
    struct CoralSpot { glm::vec3 pos; float scale; float rotY; };
    std::vector<CoralSpot> coralSpots = {
        // Vùng Tây-Nam (Vườn San Hô bên ngoài nhánh Tây & Nam)
        { glm::vec3(-290.0f, 24.0f, -290.0f), 125.0f, 0.4f },
        { glm::vec3(-420.0f, 28.0f, -240.0f), 145.0f, 1.2f },
        { glm::vec3(-360.0f, 24.0f, -460.0f), 130.0f, 2.5f },
        { glm::vec3(-860.0f, 30.0f, -280.0f), 150.0f, 0.9f },
        // Vùng Đông-Nam (Rừng Sứa bên ngoài nhánh Đông & Nam)
        { glm::vec3( 290.0f, 24.0f, -290.0f), 125.0f, 1.5f },
        { glm::vec3( 420.0f, 28.0f, -240.0f), 145.0f, 2.8f },
        { glm::vec3( 360.0f, 24.0f, -460.0f), 130.0f, 0.3f },
        { glm::vec3( 860.0f, 28.0f, -240.0f), 145.0f, 1.9f },
        // Vùng Tây-Bắc (Nhìn từ Đại Sảnh & Hành lang Bắc/Tây)
        { glm::vec3(-290.0f, 24.0f,  290.0f), 130.0f, 2.2f },
        { glm::vec3(-440.0f, 28.0f,  260.0f), 145.0f, 0.8f },
        { glm::vec3(-340.0f, 26.0f,  480.0f), 135.0f, 1.1f },
        { glm::vec3(-560.0f, 28.0f,  420.0f), 140.0f, 2.9f },
        // Vùng Đông-Bắc (Hẻm Cá Mập bên ngoài nhánh Bắc & Đông)
        { glm::vec3( 290.0f, 24.0f,  290.0f), 130.0f, 1.7f },
        { glm::vec3( 430.0f, 28.0f,  360.0f), 145.0f, 2.0f },
        { glm::vec3( 250.0f, 22.0f,  480.0f), 120.0f, 0.6f },
        { glm::vec3( 660.0f, 28.0f,  420.0f), 140.0f, 3.1f }
    };

    std::cout << "========================================================\n";
    std::cout << "  3D TROPICAL AQUARIUM - THUY CUNG NHIET DOI LUNG LINH\n";
    std::cout << "  - W/A/S/D hoac Mui ten : Di chuyen toi lui, trai phai\n";
    std::cout << "  - E / Q (hoac R / F)   : Di len / Xuong (Ngoi len / Lan sau)\n";
    std::cout << "  - Left Shift           : Tang toc di chuyen / boi nhanh\n";
    std::cout << "  - Re Chuot hoac I/J/K/L: Xoay goc nhin 360 do\n";
    std::cout << "  - Phim [SPACE]         : Xuyen kinh ra bien / Ve ham kinh\n";
    std::cout << "  - Phim [TAB]           : Khoa / Mo khoa con tro chuot\n";
    std::cout << "  - Phim ESC             : Thoat chuong trinh\n";
    std::cout << "========================================================\n";

    float titleTimer = 0.0f;

    // ------------------------------------------------------------------------------------------
    // 🟢 BƯỚC 4: VÒNG LẶP RENDER CHÍNH 60 FPS (RENDER / GAME LOOP)
    // ------------------------------------------------------------------------------------------
    while (!glfwWindowShouldClose(window)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        if (deltaTime > 0.05f) deltaTime = 0.05f;
        lastFrame = currentFrame;

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);
        camera.ProcessKeyboard(window, deltaTime);

        // Cập nhật thanh tiêu đề hiển thị trạng thái & phím bấm trực quan
        titleTimer += deltaTime;
        if (titleTimer >= 0.25f) {
            titleTimer = 0.0f;
            std::ostringstream title;
            title << "[" << camera.GetZoneName() << "]"
                  << " | Pos(" << static_cast<int>(camera.Position.x) << ", "
                  << static_cast<int>(camera.Position.y) << ", "
                  << static_cast<int>(camera.Position.z) << ")"
                  << " | WASD/Mui ten: Di | E/Q: Len-Xuong | SPACE: Lan ra bien";
            glfwSetWindowTitle(window, title.str().c_str());
        }

        glClearColor(0.03f, 0.44f, 0.60f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        oceanShader.use();
        oceanShader.setFloat("time", currentFrame);
        oceanShader.setVec3("viewPos", camera.Position);

        glm::mat4 projection = glm::perspective(glm::radians(camera.Fov), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.5f, 4000.0f);
        glm::mat4 view = camera.GetViewMatrix();
        oceanShader.setMat4("projection", projection);
        oceanShader.setMat4("view", view);

        // ======================================================================================
        // 🐟 GIAI ĐOẠN 1: VẼ ĐÁY CÁT, RẠN SAN HÔ & TOÀN BỘ SINH VẬT ĐẶC (OPAQUE)
        // ======================================================================================
        // 1.1. Vẽ đáy cát đại dương có ánh nắng khúc xạ Caustics
        {
            glm::mat4 modelSand = glm::mat4(1.0f);
            oceanShader.setMat4("model", modelSand);
            oceanShader.setBool("isFish", false);
            oceanShader.setBool("useTexture", false);
            oceanShader.setFloat("causticStrength", 1.15f);
            oceanShader.setVec3("objectColor", glm::vec3(0.76f, 0.69f, 0.50f));
            oceanShader.setFloat("alphaVal", 1.0f);
            seabedMesh.Draw(oceanShader, 0, false, glm::vec3(0.76f, 0.69f, 0.50f));
        }

        // 1.2. Vẽ 16 cụm rạn san hô 2 bên đáy biển
        oceanShader.setBool("isFish", false);
        oceanShader.setFloat("causticStrength", 0.85f);
        for (const auto& c : coralSpots) {
            glm::mat4 modelCoral = glm::mat4(1.0f);
            modelCoral = glm::translate(modelCoral, c.pos);
            modelCoral = glm::rotate(modelCoral, c.rotY, glm::vec3(0.0f, 1.0f, 0.0f));
            modelCoral = glm::scale(modelCoral, glm::vec3(c.scale));
            oceanShader.setMat4("model", modelCoral);
            coralReefModel.Draw(oceanShader);
        }

        // 1.3. Vẽ các sinh vật khổng lồ tuần tra theo đường cong khép kín 3D (Catmull-Rom Spline)
        oceanShader.setFloat("causticStrength", 0.90f);
        float tWhale = fmodf(currentFrame * 0.018f, 1.0f);
        drawSplineCreature(oceanShader, whaleModel, whalePath, tWhale, 240.0f, currentFrame, 0.0f, 0.0f, 0.04f, true);

        float tWS1 = fmodf(currentFrame * 0.030f + 0.15f, 1.0f);
        float tWS2 = fmodf(currentFrame * 0.026f + 0.65f, 1.0f);
        drawSplineCreature(oceanShader, whaleSharkModel, whaleSharkPath, tWS1, 145.0f, currentFrame, 1.0f, 0.0f, 0.05f, true);
        drawSplineCreature(oceanShader, whaleSharkModel, whaleSharkPath, tWS2, 125.0f, currentFrame, 3.4f, 0.0f, 0.05f, true);

        float tSh1 = fmodf(currentFrame * 0.042f + 0.25f, 1.0f);
        float tSh2 = fmodf(currentFrame * 0.038f + 0.78f, 1.0f);
        drawSplineCreature(oceanShader, sharkModel, sharkPatrolPath, tSh1, 110.0f, currentFrame, 0.5f, 0.0f, 0.06f, true);
        drawSplineCreature(oceanShader, sharkModel, sharkPatrolPath, tSh2, 95.0f, currentFrame, 2.7f, 0.0f, 0.06f, true);

        // Cá đuối đốm & Cá đuối Manta vẫy cánh lượn quanh vòm kính
        for (int i = 0; i < 3; ++i) {
            float tR = fmodf(currentFrame * 0.036f + i * 0.33f, 1.0f);
            drawSplineCreature(oceanShader, eagleRayModel, rayPath, tR, 88.0f, currentFrame, i * 2.1f, 0.0f, 0.09f, true);
        }
        for (int i = 0; i < 2; ++i) {
            float tM = fmodf(currentFrame * 0.032f + 0.18f + i * 0.5f, 1.0f);
            drawSplineCreature(oceanShader, mantaRayModel, rayPath, tM, 95.0f, currentFrame, i * 3.0f, 0.0f, 0.08f, true);
        }

        // Đàn Cá heo bơi theo đội hình 3 con
        for (int i = 0; i < 3; ++i) {
            float tD = fmodf(currentFrame * 0.048f - i * 0.045f + 1.0f, 1.0f);
            drawSplineCreature(oceanShader, dolphinModel, dolphinPath, tD, 74.0f, currentFrame, i * 1.4f, 0.0f, 0.08f, true);
        }

        // 3 chú Rùa biển bơi thong thả dọc 2 bên vách kính
        for (int i = 0; i < 3; ++i) {
            float tTu = fmodf(currentFrame * 0.022f + i * 0.34f, 1.0f);
            drawSplineCreature(oceanShader, turtleModel, turtlePath, tTu, 52.0f, currentFrame, i * 2.0f, 0.0f, 0.04f, false);
        }

        // 1.4. Vẽ 52 sinh vật bơi tự do đa vùng (Clownfish, BlueTang, MahiMahi, Tuna, Barramundi, Puffer, Jellyfish, Seahorse)
        for (const auto& rf : roamingSchool) {
            float t = currentFrame * rf.speed + rf.phase;
            auto evalPos = [&](float tt) {
                glm::vec3 p(
                    rf.center.x + sinf(tt * rf.freqX) * rf.radiusX,
                    rf.baseY + sinf(tt * 0.9f + rf.phase) * rf.ampY,
                    rf.center.z + cosf(tt * rf.freqZ) * rf.radiusZ
                );
                return keepOutsideTunnel(p);
            };

            glm::vec3 pos = evalPos(t);
            glm::vec3 nextPos = evalPos(t + 0.03f);
            glm::vec3 dir = nextPos - pos;
            float yaw = (glm::length(dir) > 1e-5f) ? atan2f(dir.x, dir.z) : 0.0f;
            float horiz = std::sqrt(dir.x * dir.x + dir.z * dir.z);
            float pitch = (glm::length(dir) > 1e-5f) ? atan2f(-dir.y, horiz) : 0.0f;

            glm::mat4 m = glm::mat4(1.0f);
            m = glm::translate(m, pos);
            m = glm::rotate(m, yaw + glm::radians(rf.yawOffsetDeg), glm::vec3(0.0f, 1.0f, 0.0f));
            m = glm::rotate(m, pitch, glm::vec3(1.0f, 0.0f, 0.0f));
            m = glm::scale(m, glm::vec3(rf.scale));

            oceanShader.setBool("isFish", rf.isFishShader);
            oceanShader.setFloat("fishPhase", rf.phase);
            oceanShader.setMat4("model", m);
            rf.model->Draw(oceanShader);
        }

        // ======================================================================================
        // 🫧 GIAI ĐOẠN 2: MẶT NƯỚC BIỂN LẤP LÁNH & HỆ HẠT BỌT KHÍ BÁN TRONG SUỐT (CHƯƠNG CG.6)
        // ======================================================================================
        {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glDepthMask(GL_FALSE);
            glm::mat4 modelSurf = glm::mat4(1.0f);
            oceanShader.setMat4("model", modelSurf);
            oceanShader.setBool("isFish", false);
            oceanShader.setBool("useTexture", false);
            oceanShader.setFloat("causticStrength", 1.4f);
            oceanShader.setVec3("objectColor", glm::vec3(0.22f, 0.78f, 0.96f));
            oceanShader.setFloat("alphaVal", 0.55f);
            waterSurfaceMesh.Draw(oceanShader, 0, false, glm::vec3(0.22f, 0.78f, 0.96f));
            glDepthMask(GL_TRUE);
        }

        bubbles.UpdateAndRender(deltaTime, currentFrame, camera.Position, oceanShader);

        // ======================================================================================
        // 🏛️ GIAI ĐOẠN 3: CÔNG TRÌNH HẦM KÍNH THỦY CUNG (VẼ SAU CÙNG - ALPHA BLENDING)
        // ======================================================================================
        if (!camera.isDiving) {
            glm::mat4 modelTunnel = glm::mat4(1.0f);
            oceanShader.setMat4("model", modelTunnel);
            glassTunnel.Draw(oceanShader);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

// ==============================================================================================
// 🎯 CỤM 6: CÁC HÀM XỬ LÝ SỰ KIỆN CALLBACKS (EVENT HANDLERS)
// ==============================================================================================
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (!camera.cursorLocked && glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) != GLFW_PRESS) {
        firstMouse = true;
        return;
    }

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    camera.Fov -= (float)yoffset;
    if (camera.Fov < 1.0f)  camera.Fov = 1.0f;
    if (camera.Fov > 90.0f) camera.Fov = 90.0f;
}
