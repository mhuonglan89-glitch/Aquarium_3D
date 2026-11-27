#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

/**
 * ==============================================================================================
 * 📌 FILE: include/Camera.h
 * 👤 PHỤ TRÁCH: BẠN A (Backend / Kỹ sư Toán học & Điều khiển Camera)
 * 📚 KIẾN THỨC GIÁO TRÌNH ÁNH XẠ:
 *    - CHƯƠNG CG.4: HỆ TỌA ĐỘ VÀ CÁC PHÉP BIẾN ĐỔI QUAN SÁT (Viewing & Camera)
 *      -> Ma trận quan sát LookAt: glm::lookAt(Position, Position + Front, Up)
 *      -> Góc xoay Euler 3 chiều (Pitch, Yaw)
 *      -> Thuật toán nội suy tuyến tính LERP: glm::mix chuyển cảnh mượt mà
 *    - TRẢI NGHIỆM ĐỘT PHÁ KÉP: Chế độ 1 (Đi bộ trong hầm) <-> Chế độ 2 (Hòa mình lòng biển)
 * ==============================================================================================
 */

class Camera {
public:
    // ------------------------------------------------------------------------------------------
    // [CỤM 1: CÁC VECTOR HỆ TRỤC TỌA ĐỘ CỤC BỘ CỦA CAMERA]
    // ------------------------------------------------------------------------------------------
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;

    // ------------------------------------------------------------------------------------------
    // [CỤM 2: CÁC GÓC XOAY EULER & THÔNG SỐ VẬN HÀNH]
    // ------------------------------------------------------------------------------------------
    float Yaw;
    float Pitch;

    float MovementSpeed;
    float MouseSensitivity;
    float Fov;

    // ------------------------------------------------------------------------------------------
    // [CỤM 3: CƠ CHẾ CHUYỂN CẢNH "XUYÊN KÍNH" BẰNG THUẬT TOÁN NỘI SUY LERP]
    // ------------------------------------------------------------------------------------------
    bool isDiving = false;   // false = Trong hầm kính; true = Đang lặn tự do ngoài biển
    bool cursorLocked = true; // Tab để khóa/mở khóa con trỏ chuột
    glm::vec3 targetPos;
    float targetFov;
    float lerpSpeed = 8.5f;  // Tốc độ nội suy mượt mà, phản hồi nhạy tức thì

    Camera(glm::vec3 startPos = glm::vec3(0.0f, 38.0f, -380.0f)) {
        Position = startPos;
        targetPos = startPos;
        WorldUp = glm::vec3(0.0f, 1.0f, 0.0f);
        Yaw = 90.0f; // Nhìn dọc theo đường hầm (+Z)
        Pitch = 8.0f;
        Front = glm::vec3(0.0f, 0.0f, 1.0f);
        MovementSpeed = 240.0f;
        MouseSensitivity = 0.12f;
        Fov = 65.0f;
        targetFov = 65.0f;
        updateCameraVectors();
    }

    // ------------------------------------------------------------------------------------------
    // [CỤM 4: MA TRẬN QUAN SÁT (VIEW MATRIX - CHƯƠNG CG.4)]
    // ------------------------------------------------------------------------------------------
    glm::mat4 GetViewMatrix() const {
        return glm::lookAt(Position, Position + Front, Up);
    }

    // ------------------------------------------------------------------------------------------
    // [CỤM 5: CHUYỂN ĐỔI GIỮA 2 CHẾ ĐỘ TRẢI NGHIỆM KHI NHẤN PHÍM [SPACE]]
    // ------------------------------------------------------------------------------------------
    void ToggleDiveMode() {
        isDiving = !isDiving;
        lerpSpeed = 3.2f; // Lướt êm ái điện ảnh khi xuyên qua lớp kính hầm

        if (isDiving) {
            // CHẾ ĐỘ 2: Lướt xuyên qua vách kính ra vùng biển mở phía trên đáy cát
            targetPos = glm::vec3(270.0f, 120.0f, glm::clamp(Position.z, -700.0f, 700.0f));
            targetFov = 82.0f;
        } else {
            // CHẾ ĐỘ 1: Lùi về giữa lối đi sàn hầm khô ráo
            targetPos = glm::vec3(0.0f, 38.0f, glm::clamp(Position.z, -680.0f, 680.0f));
            targetFov = 65.0f;
        }
    }

    // ------------------------------------------------------------------------------------------
    // [CỤM 6: XỬ LÝ BÀN PHÍM TOÀN DIỆN (WASD + MŨI TÊN + E/Q LÊN XUỐNG + SHIFT TĂNG TỐC)]
    // ------------------------------------------------------------------------------------------
    void ProcessKeyboard(GLFWwindow* window, float deltaTime) {
        // Phục hồi dần tốc độ phản hồi nhanh sau khi hoàn tất hiệu ứng lướt SPACE
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

        // Hỗ trợ xoay góc nhìn bằng bàn phím (J/L/I/K) phòng trường hợp người dùng dùng Trackpad
        float keyTurnSpeed = 75.0f * deltaTime;
        if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS) ProcessMouseMovement(-keyTurnSpeed / MouseSensitivity, 0.0f);
        if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) ProcessMouseMovement( keyTurnSpeed / MouseSensitivity, 0.0f);
        if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS) ProcessMouseMovement(0.0f,  keyTurnSpeed / MouseSensitivity);
        if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS) ProcessMouseMovement(0.0f, -keyTurnSpeed / MouseSensitivity);

        if (!isDiving) {
            // --- CHẾ ĐỘ 1: ĐI BỘ TRONG HẦM KÍNH ---
            glm::vec3 walkFront = glm::vec3(Front.x, 0.0f, Front.z);
            if (glm::length(walkFront) < 0.001f) walkFront = glm::vec3(0.0f, 0.0f, 1.0f);
            walkFront = glm::normalize(walkFront);
            glm::vec3 walkRight = glm::normalize(glm::cross(walkFront, WorldUp));

            if (keyForward)  targetPos += walkFront * velocity;
            if (keyBackward) targetPos -= walkFront * velocity;
            if (keyLeft)     targetPos -= walkRight * velocity;
            if (keyRight)    targetPos += walkRight * velocity;
            if (keyUp)       targetPos.y += velocity * 0.65f; // Cho phép nâng tầm mắt ngắm cá trên nóc
            if (keyDown)     targetPos.y -= velocity * 0.65f; // Hạ tầm mắt xuống sát sàn LED

            targetPos.y = glm::clamp(targetPos.y, 16.0f, 105.0f);
            targetPos.x = glm::clamp(targetPos.x, -110.0f, 110.0f);
            targetPos.z = glm::clamp(targetPos.z, -760.0f, 760.0f);
        } else {
            // --- CHẾ ĐỘ 2: BƠI TỰ DO 6 HƯỚNG TRONG LÒNG BIỂN (6-DOF) ---
            if (keyForward)  targetPos += Front * velocity;
            if (keyBackward) targetPos -= Front * velocity;
            if (keyLeft)     targetPos -= Right * velocity;
            if (keyRight)    targetPos += Right * velocity;
            if (keyUp)       targetPos.y += velocity;
            if (keyDown)     targetPos.y -= velocity;

            targetPos.y = glm::clamp(targetPos.y, 14.0f, 500.0f);
        }

        // Nội suy LERP mượt mà tới tọa độ mục tiêu
        float alpha = glm::clamp(deltaTime * lerpSpeed, 0.0f, 1.0f);
        Position = glm::mix(Position, targetPos, alpha);
        Fov      = glm::mix(Fov,      targetFov, alpha);

        // Bắt phím SPACE (chống dội phím): Chuyển đổi chế độ Đi hầm <-> Lặn biển
        static bool spacePressedLast = false;
        bool spacePressedNow = (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS);
        if (spacePressedNow && !spacePressedLast) {
            ToggleDiveMode();
        }
        spacePressedLast = spacePressedNow;

        // Bắt phím TAB: Khóa / Mở khóa con trỏ chuột
        static bool tabPressedLast = false;
        bool tabPressedNow = (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS);
        if (tabPressedNow && !tabPressedLast) {
            cursorLocked = !cursorLocked;
            glfwSetInputMode(window, GLFW_CURSOR, cursorLocked ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
        }
        tabPressedLast = tabPressedNow;
    }

    // ------------------------------------------------------------------------------------------
    // [CỤM 7: XỬ LÝ XOAY CHUỘT 360 ĐỘ (EULER ANGLES)]
    // ------------------------------------------------------------------------------------------
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
    // ------------------------------------------------------------------------------------------
    // [CỤM 8: TÍNH TOÁN LẠI CÁC VECTOR PHƯƠNG HƯỚNG THEO LƯỢNG GIÁC 3D]
    // ------------------------------------------------------------------------------------------
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
