/**
 * ==============================================================================================
 * 🏛️ ĐỒ ÁN MÔN HỌC: ĐỒ HỌA MÁY TÍNH (COMPUTER GRAPHICS)
 * 🌊 ĐỀ TÀI: MÔ PHỎNG THỦY CUNG NHIỆT ĐỚI 3D TƯƠNG TÁC (3D TROPICAL OCEANARIUM)
 * 👥 NHÓM THỰC HIỆN:
 *    - BẠN A: Kỹ sư Backend / Toán học Không gian, Quỹ đạo Bézier 3D, Vòm kính & Camera LERP
 *    - BẠN B: Kỹ sư Frontend / Shaders GLSL, Chiếu sáng Blinn-Phong, Sương mù Fog & Mô hình 3D
 * ==============================================================================================
 */

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <vector>

// ==============================================================================================
// 📦 CÁC THƯ VIỆN MODULE PHÂN CÔNG GIỮA 2 THÀNH VIÊN
// ==============================================================================================
// Header do BẠN A phụ trách (Backend / Toán học & Không gian):
#include "Camera.h"     // Camera 6-DOF, điều khiển góc nhìn và nội suy LERP chuyển cảnh
#include "Tunnel.h"     // Sinh hình học vòm kính tham số hình trụ nửa đường tròn & dầm thép
#include "BezierPath.h" // Thuật toán đường cong Bézier 3D bậc 3 giải tích & vector tiếp tuyến

// Header do BẠN B phụ trách (Frontend / Shaders & Vật liệu):
#include "Shader.h"         // Quản lý biên dịch, liên kết và truyền Uniforms cho Shader GLSL
#include "Model.h"          // Nạp file mô hình 3D (.obj) từ Sketchfab và dán vân bề mặt Texture
#include "ParticleSystem.h" // Hệ hạt bọt khí nổi lơ lửng với kỹ thuật Billboard & Alpha Blending

// ==============================================================================================
// ⚙️ CỤM 1: KHỞI TẠO BIẾN TOÀN CỤC & CẤU HÌNH THỜI GIAN
// ==============================================================================================
const unsigned int SCR_WIDTH  = 1280; // Độ phân giải chiều ngang màn hình
const unsigned int SCR_HEIGHT = 720;  // Độ phân giải chiều dọc màn hình

// Khởi tạo Camera xuất phát tại cao độ mắt người đứng trên sàn hầm kính (y = 35.0f, z = 0.0f)
Camera camera(glm::vec3(0.0f, 35.0f, 0.0f));
float lastX = SCR_WIDTH / 2.0f;  // Vị trí con trỏ chuột ban đầu tại tâm màn hình
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;          // Cờ kiểm tra lần đầu rê chuột để tránh giật góc nhìn

// Quản lý thời gian trôi qua giữa 2 khung hình (Delta Time) để chuyển động vật lý đồng nhất
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Khai báo trước các hàm xử lý sự kiện (Callbacks)
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

// ==============================================================================================
// 🚀 HÀM CHÍNH (MAIN FUNCTION): ĐIỂM CHẠM TÍCH HỢP TOÀN BỘ DỰ ÁN
// ==============================================================================================
int main() {
    // ------------------------------------------------------------------------------------------
    // 🟢 BƯỚC 1: KHỞI TẠO CỬA SỔ BẰNG THƯ VIỆN GLFW (CHƯƠNG CG.2)
    // ------------------------------------------------------------------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // Sử dụng OpenGL phiên bản 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // Dùng Core Profile chuẩn hiện đại

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, 
        "3D Tropical Oceanarium - Thuy Cung Nhiet Doi Lung Linh (Do An DHMT)", NULL, NULL);
    if (window == NULL) {
        std::cerr << "[ERROR] Khong the khoi tao cua so GLFW!" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window); // Gán ngữ cảnh OpenGL hiện tại cho cửa sổ này
    
    // Đăng ký các hàm lắng nghe sự kiện
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // Ẩn con trỏ chuột và bắt sự kiện tự do 360 độ kiểu game góc nhìn thứ nhất (FPS)
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // ------------------------------------------------------------------------------------------
    // 🟢 BƯỚC 2: NẠP CÁC CON TRỎ HÀM OPENGL BẰNG THƯ VIỆN GLAD (CHƯƠNG CG.2)
    // ------------------------------------------------------------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "[ERROR] Khong the khoi tao GLAD!" << std::endl;
        return -1;
    }

    // Cấu hình khử mặt khuất bằng Z-Buffer (Chương CG.4)
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS); // Pixel nào có tọa độ Z nhỏ hơn (gần mắt hơn) sẽ được hiển thị

    // ------------------------------------------------------------------------------------------
    // 🟢 BƯỚC 3: KHỞI TẠO SHADERS & HỆ SINH THÁI BIỂN ĐA DẠNG (BẠN B PHỤ TRÁCH)
    // ------------------------------------------------------------------------------------------
    // 3.1. Nạp và biên dịch Shader chiếu sáng Blinn-Phong và sương mù nước biển hàm mũ
    Shader oceanShader("assets/shaders/ocean_fog.vs", "assets/shaders/ocean_fog.fs");

    // 3.2. Nạp các mô hình 3D thực tế (.obj) từ Sketchfab kèm màu sắc đặc trưng của từng loài:
    Model mantaRay("assets/models/spotted_eagle_ray.obj", glm::vec3(0.12f, 0.18f, 0.25f)); // Cá đuối Manta khổng lồ
    Model tigerShark("assets/models/whale_shark.obj", glm::vec3(0.35f, 0.42f, 0.48f));     // Cá mập hổ tuần tra
    Model seaTurtle("assets/models/sea_turtle.obj", glm::vec3(0.18f, 0.52f, 0.28f));       // Rùa biển đồi mồi
    Model clownfish("assets/models/clownfish.obj", glm::vec3(0.98f, 0.45f, 0.08f));        // Cá hề Nemo cam rực rỡ
    Model yellowTang("assets/models/clownfish.obj", glm::vec3(0.96f, 0.85f, 0.12f));       // Cá đuôi gai vàng (Yellow Tang)
    Model blueSurgeon("assets/models/clownfish.obj", glm::vec3(0.12f, 0.55f, 0.92f));      // Cá bắp nẻ xanh (Dory)
    Model coralReef("assets/models/coral_reef.obj", glm::vec3(0.92f, 0.28f, 0.42f));       // Cụm rạn san hô sặc sỡ

    // 3.3. Khởi tạo hệ thống 100 hạt bọt khí nổi lơ lửng từ đáy rạn san hô
    ParticleSystem bubbles(100);

    // ------------------------------------------------------------------------------------------
    // 🟢 BƯỚC 4: KHỞI TẠO CÔNG TRÌNH HẦM KÍNH & QUỸ ĐẠO BÉZIER 3D (BẠN A PHỤ TRÁCH)
    // ------------------------------------------------------------------------------------------
    // 4.1. Sinh vòm kính Acrylic bán kính R = 160, chiều dài hầm L = 1600 bằng toán tham số nửa hình trụ
    Tunnel glassTunnel(160.0f, 1600.0f);

    // 4.2. Quỹ đạo bơi Bézier 3D của Cá đuối Manta (Lướt êm đềm ngay sát nóc vòm kính)
    BezierPath mantaPath(
        glm::vec3(-220.0f, -190.0f, -400.0f), // P0: Bắt đầu ngoài khơi xa
        glm::vec3(  80.0f, -220.0f,  120.0f), // P1: Điểm uốn kéo sát đỉnh kính hầm
        glm::vec3( 160.0f, -180.0f,  450.0f), // P2: Điểm uốn lượn dọc chiều dài hầm
        glm::vec3( 280.0f, -200.0f,  900.0f)  // P3: Kết thúc lặn sâu về rạn san hô
    );

    // 4.3. Quỹ đạo bơi Bézier 3D của Cá mập hổ (Tuần tra tầng nước giữa bao quanh hầm)
    BezierPath sharkPath(
        glm::vec3(-480.0f, -80.0f,  250.0f),
        glm::vec3(-120.0f, -130.0f, -450.0f),
        glm::vec3( 280.0f, -90.0f,  -250.0f),
        glm::vec3( 520.0f, -40.0f,   350.0f)
    );

    // 4.4. Quỹ đạo bơi Bézier 3D của Rùa biển (Bơi thong thả bên hông vách kính)
    BezierPath turtlePath(
        glm::vec3( 260.0f, -60.0f, -350.0f),
        glm::vec3( 180.0f, -90.0f,  100.0f),
        glm::vec3( 220.0f, -50.0f,  450.0f),
        glm::vec3( 190.0f, -80.0f,  800.0f)
    );

    // In thông tin hướng dẫn ra màn hình console
    std::cout << "========================================================\n";
    std::cout << "  3D TROPICAL AQUARIUM - THUY CUNG NHIET DOI LUNG LINH\n";
    std::cout << "  - Nuoc bien: Xanh ngoc lam ruc ro (Bright Tropical Cyan)\n";
    std::cout << "  - Sinh vat: Ca duoi Manta, Ca map ho, Rua bien, Ca nhiet doi\n";
    std::cout << "  - Phim W, A, S, D : Buoc di trong ham kinh / Boi ngoai bien\n";
    std::cout << "  - Re Chuot        : Xoay goc nhin toan canh 360 do\n";
    std::cout << "  - Phim [SPACE]    : Xuyen kinh hoa minh vao nuoc (100% khong kinh)\n";
    std::cout << "  - Phim E / Q      : Ngoi len / Lan sau (khi boi ngoai bien)\n";
    std::cout << "  - Phim ESC        : Thoat chuong trinh\n";
    std::cout << "========================================================\n";

    // ------------------------------------------------------------------------------------------
    // 🟢 BƯỚC 5: VÒNG LẶP RENDER CHÍNH 60 FPS (RENDER / GAME LOOP)
    // ------------------------------------------------------------------------------------------
    while (!glfwWindowShouldClose(window)) {
        // --- 5.1. Tính toán Delta Time ---
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // --- 5.2. Xử lý sự kiện bàn phím (Bạn A phụ trách: WASD & LERP phím SPACE) ---
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);
        camera.ProcessKeyboard(window, deltaTime);

        // --- 5.3. Xóa màn hình với màu xanh ngọc bích nhiệt đới rực rỡ (Tropical Cyan #057091) ---
        glClearColor(0.02f, 0.44f, 0.58f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Xóa cả bộ đệm màu và bộ đệm độ sâu

        // --- 5.4. Kích hoạt Shader và truyền các biến môi trường toàn cục (Uniforms) ---
        oceanShader.use();
        oceanShader.setFloat("time", currentFrame);
        oceanShader.setVec3("viewPos", camera.Position);

        // Thiết lập ma trận chiếu phối cảnh (Perspective) và ma trận quan sát (LookAt Camera) (Chương CG.4)
        glm::mat4 projection = glm::perspective(glm::radians(camera.Fov), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 3500.0f);
        glm::mat4 view = camera.GetViewMatrix();
        oceanShader.setMat4("projection", projection);
        oceanShader.setMat4("view", view);

        // ======================================================================================
        // 🐟 GIAI ĐOẠN VẼ 1: CÁC VẬT THỂ ĐẶC (OPAQUE OBJECTS) — PHẢI VẼ TRƯỚC!
        // ======================================================================================

        // --------------------------------------------------------------------------------------
        // 1.1. VẼ CÁ ĐUỐI MANTA BƠI THEO BÉZIER 3D & ĐẬP CÁNH
        // --------------------------------------------------------------------------------------
        static float tManta = 0.45f;
        tManta += deltaTime * 0.038f; // Tham số t tăng dần theo thời gian
        if (tManta > 1.0f) tManta = 0.0f; // Lặp lại chu kỳ bơi
        glm::vec3 mantaPos = mantaPath.GetPoint(tManta);        // Tọa độ Bézier giải tích B(t)
        float mantaAngle   = mantaPath.GetFacingAngle(tManta);  // Góc xoay tiếp tuyến B'(t)

        glm::mat4 modelManta = glm::mat4(1.0f);
        modelManta = glm::translate(modelManta, mantaPos);                    // Tịnh tiến tới vị trí
        modelManta = glm::rotate(modelManta, mantaAngle, glm::vec3(0, 1, 0)); // Xoay đầu theo tiếp tuyến
        // Chuyển động vẫy cánh mềm mại theo hàm sin (Chương CG.3 Ma trận phân cấp)
        float wingPitch = sin(currentFrame * 4.5f) * 0.08f;
        modelManta = glm::rotate(modelManta, wingPitch, glm::vec3(1, 0, 0));
        modelManta = glm::scale(modelManta, glm::vec3(2.2f));                 // Co dãn kích thước to lớn
        oceanShader.setMat4("model", modelManta);
        mantaRay.Draw(oceanShader);

        // --------------------------------------------------------------------------------------
        // 1.2. VẼ CÁ MẬP HỔ BƠI TUẦN TRA QUANH HẦM KÍNH
        // --------------------------------------------------------------------------------------
        static float tShark = 0.12f;
        tShark += deltaTime * 0.032f;
        if (tShark > 1.0f) tShark = 0.0f;
        glm::vec3 sharkPos = sharkPath.GetPoint(tShark);
        float sharkAngle   = sharkPath.GetFacingAngle(tShark);

        glm::mat4 modelShark = glm::mat4(1.0f);
        modelShark = glm::translate(modelShark, sharkPos);
        modelShark = glm::rotate(modelShark, sharkAngle, glm::vec3(0, 1, 0));
        modelShark = glm::scale(modelShark, glm::vec3(1.9f));
        oceanShader.setMat4("model", modelShark);
        tigerShark.Draw(oceanShader);

        // --------------------------------------------------------------------------------------
        // 1.3. VẼ RÙA BIỂN BƠI THONG THẢ BÊN HÔNG KÍNH
        // --------------------------------------------------------------------------------------
        static float tTurtle = 0.7f;
        tTurtle += deltaTime * 0.025f;
        if (tTurtle > 1.0f) tTurtle = 0.0f;
        glm::vec3 turtlePos = turtlePath.GetPoint(tTurtle);
        float turtleAngle   = turtlePath.GetFacingAngle(tTurtle);

        glm::mat4 modelTurtle = glm::mat4(1.0f);
        modelTurtle = glm::translate(modelTurtle, turtlePos);
        modelTurtle = glm::rotate(modelTurtle, turtleAngle, glm::vec3(0, 1, 0));
        modelTurtle = glm::scale(modelTurtle, glm::vec3(1.3f));
        oceanShader.setMat4("model", modelTurtle);
        seaTurtle.Draw(oceanShader);

        // --------------------------------------------------------------------------------------
        // 1.4. VẼ CÁC CỤM RẠN SAN HÔ PHONG PHÚ Ở ĐÁY CÁT 2 BÊN HÔNG HẦM
        // --------------------------------------------------------------------------------------
        std::vector<glm::vec3> coralPositions = {
            glm::vec3(-240.0f, -120.0f, -350.0f),
            glm::vec3(-260.0f, -110.0f,  100.0f),
            glm::vec3(-240.0f, -130.0f,  500.0f),
            glm::vec3( 250.0f, -115.0f, -200.0f),
            glm::vec3( 270.0f, -125.0f,  250.0f),
            glm::vec3( 240.0f, -110.0f,  650.0f)
        };
        for (const auto& cPos : coralPositions) {
            glm::mat4 modelCoral = glm::mat4(1.0f);
            modelCoral = glm::translate(modelCoral, cPos);
            modelCoral = glm::scale(modelCoral, glm::vec3(2.8f));
            oceanShader.setMat4("model", modelCoral);
            coralReef.Draw(oceanShader);
        }

        // --------------------------------------------------------------------------------------
        // 1.5. VẼ ĐÀN CÁ NHIỆT ĐỚI ĐÔNG ĐẢO (CÁ HỀ NEMO, CÁ GAI VÀNG TANG, CÁ DORY)
        // --------------------------------------------------------------------------------------
        for (int i = 0; i < 18; ++i) {
            float phase = currentFrame * 1.8f + i * 0.7f;
            float side = (i % 2 == 0) ? 1.0f : -1.0f; // Bơi đối xứng 2 bên rạn san hô
            glm::vec3 fishPos = glm::vec3(
                side * (210.0f + sin(phase * 0.9f) * 40.0f),
                -80.0f + cos(phase * 0.6f) * 20.0f,
                (i * 75.0f - 550.0f) + cos(phase) * 35.0f
            );
            glm::mat4 modelFish = glm::mat4(1.0f);
            modelFish = glm::translate(modelFish, fishPos);
            modelFish = glm::rotate(modelFish, -phase, glm::vec3(0, 1, 0));
            modelFish = glm::scale(modelFish, glm::vec3(0.55f));
            oceanShader.setMat4("model", modelFish);

            // Xen kẽ các loài cá để tạo sự phong phú màu sắc:
            if (i % 3 == 0) clownfish.Draw(oceanShader);
            else if (i % 3 == 1) yellowTang.Draw(oceanShader);
            else blueSurgeon.Draw(oceanShader);
        }

        // ======================================================================================
        // 🫧 GIAI ĐOẠN VẼ 2: HỆ HẠT BỌT KHÍ NỔI BÁN TRONG SUỐT (CHƯƠNG CG.6)
        // ======================================================================================
        bubbles.UpdateAndRender(deltaTime, camera.Position, oceanShader);

        // ======================================================================================
        // 🏛️ GIAI ĐOẠN VẼ 3: CÔNG TRÌNH HẦM KÍNH THỦY CUNG (VẼ SAU CÙNG)
        // ======================================================================================
        // QUY TẮC CHUYỂN CẢNH ĐỘT PHÁ:
        // - Khi camera.isDiving == false: Người dùng đang ở trong hầm kính -> Vẽ đầy đủ hầm kính!
        // - Khi camera.isDiving == true (Bấm SPACE): Người dùng đã bơi ra ngoài biển -> ẨN HẦM KÍNH 100%!
        if (!camera.isDiving) {
            glm::mat4 modelTunnel = glm::mat4(1.0f);
            oceanShader.setMat4("model", modelTunnel);
            oceanShader.setVec3("objectColor", glm::vec3(0.18f, 0.24f, 0.32f)); // Màu khung dầm thép
            oceanShader.setFloat("alphaVal", 0.28f);                             // Độ trong suốt kính Acrylic
            glassTunnel.Draw(oceanShader);
        }

        // --- 5.7. Hoán đổi bộ đệm kép (Double Buffering) và xử lý sự kiện ngoại vi ---
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Giải phóng bộ nhớ và dọn dẹp thư viện GLFW khi đóng cửa sổ
    glfwTerminate();
    return 0;
}

// ==============================================================================================
// 🎯 CỤM 6: CÁC HÀM XỬ LÝ SỰ KIỆN CALLBACKS (EVENT HANDLERS)
// ==============================================================================================
// Thay đổi kích thước khung nhìn OpenGL khi người dùng co giãn cửa sổ
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

// Bắt tọa độ rê chuột để quay góc nhìn Camera 360 độ (Euler Pitch & Yaw)
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // Đảo ngược vì hệ tọa độ cửa sổ Y chạy từ trên xuống dưới
    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// Bắt sự kiện cuộn bánh xe chuột để phóng to / thu nhỏ góc mở ống kính (FOV Zoom)
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    camera.Fov -= (float)yoffset;
    if (camera.Fov < 1.0f)  camera.Fov = 1.0f;
    if (camera.Fov > 90.0f) camera.Fov = 90.0f;
}
