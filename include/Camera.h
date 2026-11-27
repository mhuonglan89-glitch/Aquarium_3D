#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <cmath>
#include <vector>

/**
 * ==============================================================================================
 * 📌 FILE: include/Camera.h
 * 👤 PHỤ TRÁCH: BẠN A (Backend / Kỹ sư Toán học & Điều khiển Camera)
 * 📚 KIẾN THỨC GIÁO TRÌNH ÁNH XẠ:
 *    - CHƯƠNG CG.4: HỆ TỌA ĐỘ VÀ CÁC PHÉP BIẾN ĐỔI QUAN SÁT (Viewing & Camera)
 *      -> Ma trận quan sát LookAt: glm::lookAt(Position, Position + Front, Up)
 *      -> Thuật toán nội suy tuyến tính LERP: glm::mix chuyển cảnh mượt mà
 *      -> Kiểm tra va chạm vùng đi bộ trên Mê cung đa nhánh & Đại sảnh 360°
 * ==============================================================================================
 */

class Camera {
public:
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;

    float Yaw;
    float Pitch;
    float MovementSpeed;
    float MouseSensitivity;
    float Fov;

    bool isDiving = false;
    bool cursorLocked = true;
    glm::vec3 targetPos;
    float targetFov;
    float lerpSpeed = 9.0f;

    Camera(glm::vec3 startPos = glm::vec3(0.0f, 38.0f, -520.0f)) {
        Position = startPos;
        targetPos = startPos;
        WorldUp = glm::vec3(0.0f, 1.0f, 0.0f);
        Yaw = 90.0f; // Nhìn thẳng về phía Đại Sảnh Trung Tâm (+Z)
        Pitch = 5.0f;
        Front = glm::vec3(0.0f, 0.0f, 1.0f);
        MovementSpeed = 240.0f;
        MouseSensitivity = 0.12f;
        Fov = 65.0f;
        targetFov = 65.0f;
        updateCameraVectors();
    }

    glm::mat4 GetViewMatrix() const {
        return glm::lookAt(Position, Position + Front, Up);
    }

    const char* GetZoneName() const {
        if (isDiving) return "LAN BIEN TU DO 6-DOF";
        float rHub = std::sqrt(Position.x * Position.x + Position.z * Position.z);
        if (rHub < 215.0f) return "DAI SANH QUAN SAT TRUNG TAM 360";
        if (Position.z < -200.0f && std::abs(Position.x) < 120.0f) return "LOI VAO CHINH (NHANH NAM)";
        if (Position.z > 200.0f && Position.x < 120.0f) return "HANH LANG BAC -> RE PHAI HEM CA MAP";
        if (Position.x >= 120.0f && Position.z > 550.0f) return "DAI QUAN SAT HEM CA MAP (DONG-BAC)";
        if (Position.x < -200.0f && Position.z > -120.0f) return "HANH LANG TAY -> RE TRAI VUON SAN HO";
        if (Position.x < -500.0f && Position.z <= -120.0f) return "DAI QUAN SAT VUON SAN HO (TAY-NAM)";
        if (Position.x > 200.0f && Position.z > -120.0f) return "HANH LANG DONG -> RE PHAI RUNG SUA";
        if (Position.x > 500.0f && Position.z <= -120.0f) return "DAI QUAN SAT RUNG SUA PHAT SANG (DONG-NAM)";
        return "ME CUNG THUY CUNG";
    }

    void ToggleDiveMode() {
        isDiving = !isDiving;
        lerpSpeed = 3.2f;

        if (isDiving) {
            // Bay vọt lên qua mái vòm kính ra vùng nước biển tự do phía trên mê cung
            targetPos = glm::vec3(Position.x, 185.0f, Position.z);
            targetFov = 82.0f;
        } else {
            // Lùi về đúng hành lang gần nhất trong mê cung
            targetPos = glm::vec3(Position.x, 38.0f, Position.z);
            clampToMazeWalkway(targetPos);
            targetFov = 65.0f;
        }
    }

    void ProcessKeyboard(GLFWwindow* window, float deltaTime) {
        lerpSpeed = glm::mix(lerpSpeed, 9.5f, glm::clamp(deltaTime * 2.0f, 0.0f, 1.0f));

        float boost = (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
                       glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS) ? 2.4f : 1.0f;
        float velocity = MovementSpeed * boost * deltaTime;

        bool keyForward  = (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS);
        bool keyBackward = (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS);
        bool keyLeft     = (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS);
        bool keyRight    = (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS);
        bool keyUp       = (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_PAGE_UP) == GLFW_PRESS);
        bool keyDown     = (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_PAGE_DOWN) == GLFW_PRESS);

        float keyTurnSpeed = 75.0f * deltaTime;
        if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS) ProcessMouseMovement(-keyTurnSpeed / MouseSensitivity, 0.0f);
        if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) ProcessMouseMovement( keyTurnSpeed / MouseSensitivity, 0.0f);
        if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS) ProcessMouseMovement(0.0f,  keyTurnSpeed / MouseSensitivity);
        if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS) ProcessMouseMovement(0.0f, -keyTurnSpeed / MouseSensitivity);

        if (!isDiving) {
            glm::vec3 walkFront = glm::vec3(Front.x, 0.0f, Front.z);
            if (glm::length(walkFront) < 0.001f) walkFront = glm::vec3(0.0f, 0.0f, 1.0f);
            walkFront = glm::normalize(walkFront);
            glm::vec3 walkRight = glm::normalize(glm::cross(walkFront, WorldUp));

            if (keyForward)  targetPos += walkFront * velocity;
            if (keyBackward) targetPos -= walkFront * velocity;
            if (keyLeft)     targetPos -= walkRight * velocity;
            if (keyRight)    targetPos += walkRight * velocity;
            if (keyUp)       targetPos.y += velocity * 0.65f;
            if (keyDown)     targetPos.y -= velocity * 0.65f;

            targetPos.y = glm::clamp(targetPos.y, 16.0f, 95.0f);
            clampToMazeWalkway(targetPos);
        } else {
            if (keyForward)  targetPos += Front * velocity;
            if (keyBackward) targetPos -= Front * velocity;
            if (keyLeft)     targetPos -= Right * velocity;
            if (keyRight)    targetPos += Right * velocity;
            if (keyUp)       targetPos.y += velocity;
            if (keyDown)     targetPos.y -= velocity;

            targetPos.y = glm::clamp(targetPos.y, 14.0f, 500.0f);
        }

        float alpha = glm::clamp(deltaTime * lerpSpeed, 0.0f, 1.0f);
        Position = glm::mix(Position, targetPos, alpha);
        Fov      = glm::mix(Fov,      targetFov, alpha);

        static bool spacePressedLast = false;
        bool spacePressedNow = (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS);
        if (spacePressedNow && !spacePressedLast) ToggleDiveMode();
        spacePressedLast = spacePressedNow;

        static bool tabPressedLast = false;
        bool tabPressedNow = (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS);
        if (tabPressedNow && !tabPressedLast) {
            cursorLocked = !cursorLocked;
            glfwSetInputMode(window, GLFW_CURSOR, cursorLocked ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
        }
        tabPressedLast = tabPressedNow;
    }

    void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true) {
        xoffset *= MouseSensitivity;
        yoffset *= MouseSensitivity;
        Yaw   += xoffset;
        Pitch += yoffset;
        if (constrainPitch) {
            if (Pitch > 89.0f)  Pitch = 89.0f;
            if (Pitch < -89.0f) Pitch = -89.0f;
        }
        updateCameraVectors();
    }

private:
    // Kiểm tra và giữ người đi bộ nằm trong hành lang mê cung & Đại Sảnh Trung Tâm
    static void clampToMazeWalkway(glm::vec3& pos) {
        glm::vec2 p(pos.x, pos.z);

        // 1. Trong Đại Sảnh Trung Tâm (R = 195), đẩy nhẹ ra ngoài bục thông tin ở tâm (R = 36)
        float distHub = glm::length(p);
        if (distHub < 195.0f) {
            if (distHub < 36.0f) {
                glm::vec2 dir = (distHub > 0.01f) ? (p / distHub) : glm::vec2(0.0f, -1.0f);
                p = dir * 36.0f;
                pos.x = p.x; pos.z = p.y;
            }
            return;
        }

        // 2. Tập hợp các đoạn trục đường hầm mê cung (bán kính lối đi an toàn = 76)
        struct Seg { glm::vec2 a, b; };
        static const Seg segs[] = {
            { {   0.0f, -680.0f}, {   0.0f,  680.0f} }, // Trục Nam <-> Bắc
            { {   0.0f,  680.0f}, { 480.0f,  680.0f} }, // Nhánh Bắc rẽ phải (Hẻm Cá Mập)
            { {-640.0f,    0.0f}, { 640.0f,    0.0f} }, // Trục Tây <-> Đông
            { {-640.0f,    0.0f}, {-640.0f, -480.0f} }, // Nhánh Tây rẽ trái (Vườn San Hô)
            { { 640.0f,    0.0f}, { 640.0f, -460.0f} }  // Nhánh Đông rẽ phải (Rừng Sứa)
        };

        float minDist = 1e9f;
        glm::vec2 bestClosest = p;
        for (const auto& s : segs) {
            glm::vec2 ab = s.b - s.a;
            float t = glm::clamp(glm::dot(p - s.a, ab) / glm::dot(ab, ab), 0.0f, 1.0f);
            glm::vec2 q = s.a + ab * t;
            float d = glm::length(p - q);
            if (d < minDist) {
                minDist = d;
                bestClosest = q;
            }
        }

        const float walkHalfWidth = 76.0f;
        if (minDist > walkHalfWidth) {
            glm::vec2 clamped = bestClosest + ((p - bestClosest) / minDist) * walkHalfWidth;
            pos.x = clamped.x;
            pos.z = clamped.y;
        }
    }

    void updateCameraVectors() {
        glm::vec3 front;
        front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        front.y = sin(glm::radians(Pitch));
        front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        Front = glm::normalize(front);
        Right = glm::normalize(glm::cross(Front, WorldUp));
        Up    = glm::normalize(glm::cross(Right, Front));
    }
};
