#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <vector>

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
Camera camera(glm::vec3(0.0f, 35.0f, 0.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

int main() {
    // 1. Khởi tạo GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "3D Tropical Oceanarium - Thuy Cung Nhiet Doi Lung Linh", NULL, NULL);
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

    // 2. Khởi tạo GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "[ERROR] Khong the khoi tao GLAD!" << std::endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // 3. Khởi tạo Shaders & Hệ Sinh Thái Biển Đa Dạng (Bạn B)
    Shader oceanShader("assets/shaders/ocean_fog.vs", "assets/shaders/ocean_fog.fs");

    // Nạp đầy đủ các loài sinh vật từ bộ sưu tập Sketchfab:
    Model mantaRay("assets/models/spotted_eagle_ray.obj", glm::vec3(0.12f, 0.18f, 0.25f)); // Cá đuối Manta khổng lồ
    Model tigerShark("assets/models/whale_shark.obj", glm::vec3(0.35f, 0.42f, 0.48f));     // Cá mập hổ / cá mập voi
    Model seaTurtle("assets/models/sea_turtle.obj", glm::vec3(0.18f, 0.52f, 0.28f));       // Rùa biển bơi lội
    Model clownfish("assets/models/clownfish.obj", glm::vec3(0.98f, 0.45f, 0.08f));        // Cá hề Nemo cam rực rỡ
    Model yellowTang("assets/models/clownfish.obj", glm::vec3(0.96f, 0.85f, 0.12f));       // Cá đuôi gai vàng (Yellow Tang)
    Model blueSurgeon("assets/models/clownfish.obj", glm::vec3(0.12f, 0.55f, 0.92f));      // Cá bắp nẻ xanh (Dory)
    Model coralReef("assets/models/coral_reef.obj", glm::vec3(0.92f, 0.28f, 0.42f));       // Rạn san hô sặc sỡ

    ParticleSystem bubbles(100); // 100 hạt bọt khí lấp lánh

    // 4. Khởi tạo Đường Hầm Kính & Các Quỹ Đạo Bézier 3D (Bạn A)
    Tunnel glassTunnel(160.0f, 1600.0f);

    // Quỹ đạo cá đuối khổng lồ lướt trên vòm kính
    BezierPath mantaPath(
        glm::vec3(-220.0f, -190.0f, -400.0f),
        glm::vec3(  80.0f, -220.0f,  120.0f),
        glm::vec3( 160.0f, -180.0f,  450.0f),
        glm::vec3( 280.0f, -200.0f,  900.0f)
    );

    // Quỹ đạo cá mập tuần tra tầng giữa
    BezierPath sharkPath(
        glm::vec3(-480.0f, -80.0f,  250.0f),
        glm::vec3(-120.0f, -130.0f, -450.0f),
        glm::vec3( 280.0f, -90.0f,  -250.0f),
        glm::vec3( 520.0f, -40.0f,   350.0f)
    );

    // Quỹ đạo rùa biển bơi thong thả bên hông kính
    BezierPath turtlePath(
        glm::vec3( 260.0f, -60.0f, -350.0f),
        glm::vec3( 180.0f, -90.0f,  100.0f),
        glm::vec3( 220.0f, -50.0f,  450.0f),
        glm::vec3( 190.0f, -80.0f,  800.0f)
    );

    std::cout << "========================================================\n";
    std::cout << "  3D TROPICAL AQUARIUM - THUY CUNG NHIET DOI LUNG LINH\n";
    std::cout << "  - Nuoc bien: Xanh ngoc lam ruc ro (Bright Tropical Cyan)\n";
    std::cout << "  - Sinh vat: Ca duoi Manta, Ca map ho, Rua bien, Ca nhiet doi\n";
    std::cout << "  - Phim [SPACE]: Xuyen kinh hoa minh vao nuoc (100% khong kinh)\n";
    std::cout << "========================================================\n";

    // 5. VÒNG LẶP RENDER CHÍNH (RENDER LOOP)
    while (!glfwWindowShouldClose(window)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);
        camera.ProcessKeyboard(window, deltaTime);

        // NỀN SÁNG RỰC RỠ: Màu xanh ngọc bích nhiệt đới (Tropical Cyan Ocean #057091)
        glClearColor(0.02f, 0.44f, 0.58f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        oceanShader.use();
        oceanShader.setFloat("time", currentFrame);
        oceanShader.setVec3("viewPos", camera.Position);

        glm::mat4 projection = glm::perspective(glm::radians(camera.Fov), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 3500.0f);
        glm::mat4 view = camera.GetViewMatrix();
        oceanShader.setMat4("projection", projection);
        oceanShader.setMat4("view", view);

        // --------------------------------------------------------------------
        // BƯỚC 1: VẼ SINH VẬT BIỂN ĐA DẠNG (Vật thể đặc vẽ trước)
        // --------------------------------------------------------------------

        // 1.1 Cá đuối Manta khổng lồ lướt trên vòm kính (sải cánh rộng)
        static float tManta = 0.45f;
        tManta += deltaTime * 0.038f;
        if (tManta > 1.0f) tManta = 0.0f;
        glm::vec3 mantaPos = mantaPath.GetPoint(tManta);
        float mantaAngle = mantaPath.GetFacingAngle(tManta);

        glm::mat4 modelManta = glm::mat4(1.0f);
        modelManta = glm::translate(modelManta, mantaPos);
        modelManta = glm::rotate(modelManta, mantaAngle, glm::vec3(0, 1, 0));
        // Thêm chuyển động đập cánh nhẹ theo hàm sin
        float wingPitch = sin(currentFrame * 4.5f) * 0.08f;
        modelManta = glm::rotate(modelManta, wingPitch, glm::vec3(1, 0, 0));
        modelManta = glm::scale(modelManta, glm::vec3(2.2f));
        oceanShader.setMat4("model", modelManta);
        mantaRay.Draw(oceanShader);

        // 1.2 Cá mập hổ dũng mãnh bơi tuần tra
        static float tShark = 0.12f;
        tShark += deltaTime * 0.032f;
        if (tShark > 1.0f) tShark = 0.0f;
        glm::vec3 sharkPos = sharkPath.GetPoint(tShark);
        float sharkAngle = sharkPath.GetFacingAngle(tShark);

        glm::mat4 modelShark = glm::mat4(1.0f);
        modelShark = glm::translate(modelShark, sharkPos);
        modelShark = glm::rotate(modelShark, sharkAngle, glm::vec3(0, 1, 0));
        modelShark = glm::scale(modelShark, glm::vec3(1.9f));
        oceanShader.setMat4("model", modelShark);
        tigerShark.Draw(oceanShader);

        // 1.3 Rùa biển bơi thong thả
        static float tTurtle = 0.7f;
        tTurtle += deltaTime * 0.025f;
        if (tTurtle > 1.0f) tTurtle = 0.0f;
        glm::vec3 turtlePos = turtlePath.GetPoint(tTurtle);
        float turtleAngle = turtlePath.GetFacingAngle(tTurtle);

        glm::mat4 modelTurtle = glm::mat4(1.0f);
        modelTurtle = glm::translate(modelTurtle, turtlePos);
        modelTurtle = glm::rotate(modelTurtle, turtleAngle, glm::vec3(0, 1, 0));
        modelTurtle = glm::scale(modelTurtle, glm::vec3(1.3f));
        oceanShader.setMat4("model", modelTurtle);
        seaTurtle.Draw(oceanShader);

        // 1.4 Các rạn san hô phong phú trải dọc 2 bên hông hầm kính
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

        // 1.5 Đàn cá nhiệt đới đông đảo (Cá hề Nemo, Cá gai vàng Tang, Cá nẻ xanh Dory)
        for (int i = 0; i < 18; ++i) {
            float phase = currentFrame * 1.8f + i * 0.7f;
            float side = (i % 2 == 0) ? 1.0f : -1.0f;
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

            if (i % 3 == 0) clownfish.Draw(oceanShader);
            else if (i % 3 == 1) yellowTang.Draw(oceanShader);
            else blueSurgeon.Draw(oceanShader);
        }

        // 1.6 Hệ hạt bọt khí lấp lánh nổi lên từ rạn san hô
        bubbles.UpdateAndRender(deltaTime, camera.Position, oceanShader);

        // --------------------------------------------------------------------
        // BƯỚC 2: VẼ ĐƯỜNG HẦM KÍNH (Chỉ vẽ khi ở trong hầm)
        // Khi camera.isDiving == true (Bấm SPACE), ẩn hầm kính -> 100% không kính!
        // --------------------------------------------------------------------
        if (!camera.isDiving) {
            glm::mat4 modelTunnel = glm::mat4(1.0f);
            oceanShader.setMat4("model", modelTunnel);
            oceanShader.setVec3("objectColor", glm::vec3(0.18f, 0.24f, 0.32f)); // Màu khung dầm thép
            oceanShader.setFloat("alphaVal", 0.28f);                             // Kính trong suốt lấp lánh
            glassTunnel.Draw(oceanShader);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
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
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    camera.Fov -= (float)yoffset;
    if (camera.Fov < 1.0f) camera.Fov = 1.0f;
    if (camera.Fov > 90.0f) camera.Fov = 90.0f;
}
