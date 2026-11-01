#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

/**
 * @file Camera.h
 * @brief Hệ thống Camera 6 bậc tự do & Thuật toán LERP chuyển cảnh Xuyên Kính
 * @author Bạn A (Backend / Math & Camera) - Môn Đồ Họa Máy Tính (Chương CG.4: 3D Transformations & Projections)
 */
class Camera {
public:
    // Tọa độ & vector hệ trục quan sát
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;

    // Góc quay Euler (Pitch, Yaw)
    float Yaw;
    float Pitch;

    // Tùy chỉnh vận tốc và độ nhạy
    float MovementSpeed;
    float MouseSensitivity;
    float Fov;

    // Cơ chế chuyển cảnh "Xuyên Kính" (Chương CG.4: LERP P(t) = (1-t)P1 + t*P2)
    bool isDiving = false;
    glm::vec3 targetPos;
    float targetFov;

    Camera(glm::vec3 startPos = glm::vec3(0.0f, 35.0f, -200.0f)) {
        Position = startPos;
        targetPos = startPos;
        WorldUp = glm::vec3(0.0f, 1.0f, 0.0f);
        Yaw = 90.0f; // Nhìn dọc theo đường hầm (hướng +Z)
        Pitch = 0.0f;
        Front = glm::vec3(0.0f, 0.0f, 1.0f);
        MovementSpeed = 140.0f;
        MouseSensitivity = 0.1f;
        Fov = 60.0f;
        targetFov = 60.0f;
        updateCameraVectors();
    }

    // Tính ma trận quan sát (View Matrix - Chương CG.4)
    glm::mat4 GetViewMatrix() const {
        return glm::lookAt(Position, Position + Front, Up);
    }

    // Chuyển đổi giữa 2 chế độ khi nhấn phím SPACE
    void ToggleDiveMode() {
        isDiving = !isDiving;
        if (isDiving) {
            // Chế độ 2: Lướt xuyên kính ra ngoài lòng nước đại dương sâu
            targetPos = glm::vec3(240.0f, -60.0f, Position.z);
            targetFov = 85.0f; // Mở rộng góc mở FOV toàn cảnh điện ảnh
        } else {
            // Chế độ 1: Lùi về giữa hành lang đường hầm khô ráo
            targetPos = glm::vec3(0.0f, 35.0f, Position.z);
            targetFov = 60.0f; // Góc mở chuẩn hầm kính
        }
    }

    // Xử lý di chuyển bàn phím với thuật toán nội suy LERP mượt mà
    void ProcessKeyboard(GLFWwindow* window, float deltaTime) {
        // Thuật toán LERP (Linear Interpolation): P_now = mix(P_now, P_target, dt * 3.5)
        Position = glm::mix(Position, targetPos, deltaTime * 3.5f);
        Fov = glm::mix(Fov, targetFov, deltaTime * 3.5f);

        float velocity = MovementSpeed * deltaTime;

        if (!isDiving) {
            // Chế độ 1: Bước đi trên sàn hầm (Khóa độ cao Y = 35)
            glm::vec3 walkFront = glm::normalize(glm::vec3(Front.x, 0.0f, Front.z));
            glm::vec3 walkRight = glm::normalize(glm::vec3(Right.x, 0.0f, Right.z));

            if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) targetPos += walkFront * velocity;
            if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) targetPos -= walkFront * velocity;
            if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) targetPos -= walkRight * velocity;
            if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) targetPos += walkRight * velocity;

            // Giữ chân trên sàn hầm và giới hạn trong vòm kính
            targetPos.y = 35.0f;
            if (targetPos.x > 85.0f)  targetPos.x = 85.0f;
            if (targetPos.x < -85.0f) targetPos.x = -85.0f;
            if (targetPos.z > 680.0f) targetPos.z = 680.0f;
            if (targetPos.z < -680.0f) targetPos.z = -680.0f;
        } else {
            // Chế độ 2: Bơi lội tự do 6 hướng trong lòng đại dương
            if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) targetPos += Front * velocity;
            if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) targetPos -= Front * velocity;
            if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) targetPos -= Right * velocity;
            if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) targetPos += Right * velocity;
            if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) targetPos.y += velocity; // Bơi ngoi lên
            if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) targetPos.y -= velocity; // Lặn xuống sâu
        }

        // Bắt sự kiện phím SPACE (Chuyển đổi 1 lần mỗi lần nhấn - chống dội phím)
        static bool spacePressedLast = false;
        bool spacePressedNow = (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS);
        if (spacePressedNow && !spacePressedLast) {
            ToggleDiveMode();
        }
        spacePressedLast = spacePressedNow;
    }

    // Xử lý xoay chuột 360 độ
    void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true) {
        xoffset *= MouseSensitivity;
        yoffset *= MouseSensitivity;

        Yaw += xoffset;
        Pitch += yoffset;

        if (constrainPitch) {
            if (Pitch > 89.0f) Pitch = 89.0f;
            if (Pitch < -89.0f) Pitch = -89.0f;
        }

        updateCameraVectors();
    }

private:
    void updateCameraVectors() {
        glm::vec3 front;
        front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        front.y = sin(glm::radians(Pitch));
        front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        Front = glm::normalize(front);
        Right = glm::normalize(glm::cross(Front, WorldUp));
        Up = glm::normalize(glm::cross(Right, Front));
    }
};
