#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

// Header của BẠN A (Backend / Math & Camera)
#include "Camera.h"
#include "Tunnel.h"
#include "BezierPath.h"

// Header của BẠN B (Frontend / Shaders & Assets)
#include "Shader.h"
#include "Model.h"
#include "ParticleSystem.h"

// Cấu hình màn hình
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

// Khởi tạo Camera tại vị trí sàn hầm kính (Bạn A)
Camera camera(glm::vec3(0.0f, 35.0f, -200.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// Quản lý thời gian Delta Time
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Callbacks
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

int main() {
    // 1. Khởi tạo GLFW
    if (!glfwInit()) {
        std::cerr << "[ERROR] Khong the khoi tao GLFW!" << std::endl;
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "MOH HINH THUY CUNG 3D - INTERACTIVE 3D AQUARIUM", NULL, NULL);
    if (window == NULL) {
        std::cerr << "[ERROR] Khong the khoi tao cua so GLFW!" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // Ẩn con trỏ chuột và bắt sự kiện tự do
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // 2. Khởi tạo GLAD nạp các con trỏ hàm OpenGL
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "[ERROR] Khong the khoi tao GLAD!" << std::endl;
        return -1;
    }

    // Cấu hình trạng thái OpenGL toàn cục (Chương CG.4 & CG.6)
    glEnable(GL_DEPTH_TEST); // Z-Buffer khử mặt khuất
    glDepthFunc(GL_LESS);

    // 3. Khởi tạo Shaders & Tài nguyên 3D (Của BẠN B)
    Shader oceanShader("assets/shaders/ocean_fog.vs", "assets/shaders/ocean_fog.fs");
    std::cout << "[INFO::SHADER] Program ID: " << oceanShader.ID << std::endl;
    
    // Nạp Models và Texture thật
    Model whaleShark("assets/models/whale_shark.obj", glm::vec3(0.18f, 0.28f, 0.38f), "assets/textures/shark_diffuse.jpg");
    Model eagleRay("assets/models/spotted_eagle_ray.obj", glm::vec3(0.12f, 0.16f, 0.22f), "assets/textures/ray_texture.png");
    Model clownfish("assets/models/clownfish.obj", glm::vec3(0.95f, 0.45f, 0.08f), "assets/textures/clownfish.png");
    Model coralReef("assets/models/coral_reef.obj", glm::vec3(0.85f, 0.22f, 0.35f), "assets/textures/coral_base.png");
    ParticleSystem bubbles(100); // 100 hạt bọt khí nổi li ti

    // 4. Khởi tạo Vòm kính & Quỹ đạo Bézier 3D (Của BẠN A)
    Tunnel glassTunnel(160.0f, 1500.0f); // Bán kính vòm 160, dài 1500

    // Quỹ đạo bơi Bézier của cá mập voi (Lượn hình vòng cung lớn ngay phía trên và trước tầm nhìn)
    BezierPath sharkPath(
        glm::vec3(-160.0f,  65.0f,  -60.0f),
        glm::vec3( -30.0f,  95.0f,   30.0f),
        glm::vec3(  60.0f, 110.0f,  120.0f),
        glm::vec3( 180.0f,  85.0f,  220.0f)
    );

    // Quỹ đạo bơi Bézier của cá đuối (Lướt sát nóc vòm kính ngay phía trước người xem)
    BezierPath rayPath(
        glm::vec3(  60.0f, 130.0f,  -40.0f),
        glm::vec3(  10.0f, 140.0f,   30.0f),
        glm::vec3( -30.0f, 135.0f,  100.0f),
        glm::vec3(-100.0f, 125.0f,  180.0f)
    );

    std::cout << "========================================================\n";
    std::cout << "  3D AQUARIUM SIMULATION ENGINE DA KHOI DONG THANH CONG!\n";
    std::cout << "  - Phim W, A, S, D : Buoc di trong ham kinh\n";
    std::cout << "  - Chuot           : Xoay goc nhin 360 do\n";
    std::cout << "  - Phim [SPACE]    : Xuyen kinh hoa minh vao nuoc (100% khong kinh)\n";
    std::cout << "  - Phim E / Q      : Ngoi len / Lan xuong (khi lan bien)\n";
    std::cout << "  - Phim ESC        : Thoat chuong trinh\n";
    std::cout << "========================================================\n";

    // 5. VÒNG LẶP RENDER CHÍNH (RENDER LOOP)
    while (!glfwWindowShouldClose(window)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Xử lý sự kiện bàn phím (Bạn A: WASD & SPACE LERP)
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);
        camera.ProcessKeyboard(window, deltaTime);

        // Xóa màn hình với màu xanh ngọc đại dương Deep Marine (#041C2C)
        glClearColor(0.016f, 0.110f, 0.173f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        oceanShader.use();
        oceanShader.setFloat("time", currentFrame);
        oceanShader.setVec3("viewPos", camera.Position);

        // Thiết lập ma trận quan sát và phép chiếu phối cảnh (Chương CG.4)
        glm::mat4 projection = glm::perspective(glm::radians(camera.Fov), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 3500.0f);
        glm::mat4 view = camera.GetViewMatrix();
        oceanShader.setMat4("projection", projection);
        oceanShader.setMat4("view", view);

        // --------------------------------------------------------------------
        // BƯỚC 1: VẼ SINH VẬT BIỂN (Vật thể đặc bên ngoài vẽ trước)
        // --------------------------------------------------------------------
        // 1.1 Cá mập voi bơi theo đường cong Bézier 3D (Bạn A tính toán, Bạn B vẽ)
        static float tShark = 0.20f;
        tShark += deltaTime * 0.035f;
        if (tShark > 1.0f) tShark = 0.0f;
        glm::vec3 sharkPos = sharkPath.GetPoint(tShark);
        float sharkAngle = sharkPath.GetFacingAngle(tShark);
        float sharkPitch = sharkPath.GetPitchAngle(tShark);

        glm::mat4 modelShark = glm::mat4(1.0f);
        modelShark = glm::translate(modelShark, sharkPos);
        modelShark = glm::rotate(modelShark, sharkAngle, glm::vec3(0, 1, 0));
        modelShark = glm::rotate(modelShark, sharkPitch, glm::vec3(1, 0, 0));
        modelShark = glm::rotate(modelShark, glm::radians(-90.0f), glm::vec3(0, 1, 0));
        modelShark = glm::scale(modelShark, glm::vec3(25.0f));
        oceanShader.setMat4("model", modelShark);
        oceanShader.setBool("isFish", true);
        whaleShark.Draw(oceanShader);

        // 1.2 Cá đuối đốm lướt trên nóc vòm kính
        static float tRay = 0.35f;
        tRay += deltaTime * 0.045f;
        if (tRay > 1.0f) tRay = 0.0f;
        glm::vec3 rayPos = rayPath.GetPoint(tRay);
        float rayAngle = rayPath.GetFacingAngle(tRay);
        float rayPitch = rayPath.GetPitchAngle(tRay);

        glm::mat4 modelRay = glm::mat4(1.0f);
        modelRay = glm::translate(modelRay, rayPos);
        modelRay = glm::rotate(modelRay, rayAngle, glm::vec3(0, 1, 0));
        modelRay = glm::rotate(modelRay, rayPitch, glm::vec3(1, 0, 0));
        modelRay = glm::rotate(modelRay, glm::radians(-90.0f), glm::vec3(0, 1, 0));
        modelRay = glm::scale(modelRay, glm::vec3(22.0f));
        oceanShader.setMat4("model", modelRay);
        oceanShader.setBool("isFish", false);
        eagleRay.Draw(oceanShader);

        // 1.3 Rạn san hô cố định ở đáy cát hai bên vòm kính
        oceanShader.setBool("isFish", false);
        glm::mat4 modelCoral1 = glm::mat4(1.0f);
        modelCoral1 = glm::translate(modelCoral1, glm::vec3(145.0f, 0.0f, -120.0f));
        modelCoral1 = glm::scale(modelCoral1, glm::vec3(60.0f));
        oceanShader.setMat4("model", modelCoral1);
        coralReef.Draw(oceanShader);

        glm::mat4 modelCoral2 = glm::mat4(1.0f);
        modelCoral2 = glm::translate(modelCoral2, glm::vec3(-135.0f, 0.0f, -10.0f));
        modelCoral2 = glm::scale(modelCoral2, glm::vec3(55.0f));
        oceanShader.setMat4("model", modelCoral2);
        coralReef.Draw(oceanShader);

        // 1.4 Đàn cá hề bơi quanh rạn san hô hai bên hầm kính
        oceanShader.setBool("isFish", true);
        for (int i = 0; i < 6; ++i) {
            float phase = currentFrame * 1.8f + i * 1.05f;
            glm::vec3 cfPos = glm::vec3(
                145.0f + sin(phase) * 22.0f,
                22.0f + cos(phase * 0.9f) * 12.0f,
                -120.0f + cos(phase) * 22.0f
            );
            glm::mat4 modelCF = glm::mat4(1.0f);
            modelCF = glm::translate(modelCF, cfPos);
            modelCF = glm::rotate(modelCF, -phase + glm::radians(90.0f), glm::vec3(0, 1, 0));
            modelCF = glm::scale(modelCF, glm::vec3(35.0f));
            oceanShader.setMat4("model", modelCF);
            clownfish.Draw(oceanShader);
        }
        for (int i = 0; i < 5; ++i) {
            float phase = currentFrame * 1.6f + i * 1.25f;
            glm::vec3 cfPos = glm::vec3(
                -135.0f + sin(phase) * 20.0f,
                25.0f + sin(phase * 0.8f) * 10.0f,
                -60.0f + cos(phase) * 20.0f
            );
            glm::mat4 modelCF = glm::mat4(1.0f);
            modelCF = glm::translate(modelCF, cfPos);
            modelCF = glm::rotate(modelCF, -phase + glm::radians(90.0f), glm::vec3(0, 1, 0));
            modelCF = glm::scale(modelCF, glm::vec3(32.0f));
            oceanShader.setMat4("model", modelCF);
            clownfish.Draw(oceanShader);
        }

        // 1.5 Hệ hạt bọt khí nổi lên từ đáy biển (Bạn B)
        oceanShader.setBool("isFish", false);
        bubbles.UpdateAndRender(deltaTime, camera.Position, oceanShader);

        // --------------------------------------------------------------------
        // BƯỚC 2: VẼ HẦM KÍNH THỦY CUNG (Chỉ vẽ khi ở Chế độ 1 trong hầm)
        // Khi camera.isDiving == true (Bấm SPACE), ẩn hoàn toàn hầm kính (100% không kính)!
        // --------------------------------------------------------------------
        if (!camera.isDiving) {
            glm::mat4 modelTunnel = glm::mat4(1.0f);
            oceanShader.setMat4("model", modelTunnel);
            glassTunnel.Draw(oceanShader);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();

        static int frameCount = 0;
        frameCount++;
        if (frameCount == 15) {
            std::vector<unsigned char> pixels(SCR_WIDTH * SCR_HEIGHT * 3);
            glReadPixels(0, 0, SCR_WIDTH, SCR_HEIGHT, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());
            FILE* f = fopen("live_opengl_frame.ppm", "wb");
            if (f) {
                fprintf(f, "P6\n%d %d\n255\n", SCR_WIDTH, SCR_HEIGHT);
                for (int y = SCR_HEIGHT - 1; y >= 0; --y) {
                    fwrite(&pixels[y * SCR_WIDTH * 3], 1, SCR_WIDTH * 3, f);
                }
                fclose(f);
            }
        }
    }

    glfwTerminate();
    return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // Đảo ngược vì tọa độ Y từ trên xuống
    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    camera.Fov -= (float)yoffset;
    if (camera.Fov < 1.0f) camera.Fov = 1.0f;
    if (camera.Fov > 90.0f) camera.Fov = 90.0f;
}
