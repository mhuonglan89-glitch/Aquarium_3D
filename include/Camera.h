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
    // [CỤM 1: CÁC VECTOR HỆ TRỤC TỌA ĐỘ CỤC BỘ CỦA CAMERA (CAMERA COORDINATE SYSTEM)]
    // ------------------------------------------------------------------------------------------
    glm::vec3 Position; // Vị trí hiện tại của Camera trong không gian thế giới (World Space)
    glm::vec3 Front;    // Vector hướng nhìn thẳng về phía trước (Z-axis của Camera)
    glm::vec3 Up;       // Vector hướng thẳng lên đỉnh đầu người quan sát (Y-axis của Camera)
    glm::vec3 Right;    // Vector hướng sang bên tay phải người quan sát (X-axis của Camera)
    glm::vec3 WorldUp;  // Vector quy ước hướng lên trời của toàn thế giới: vec3(0, 1, 0)

    // ------------------------------------------------------------------------------------------
    // [CỤM 2: CÁC GÓC XOAY EULER (EULER ANGLES) VÀ THÔNG SỐ VẬN HÀNH]
    // ------------------------------------------------------------------------------------------
    float Yaw;          // Góc xoay phương ngang quanh trục Y (trái/phải)
    float Pitch;        // Góc ngóc/chúi đầu phương dọc quanh trục X (lên/xuống)

    float MovementSpeed;    // Tốc độ di chuyển bước đi (units/giây)
    float MouseSensitivity; // Độ nhạy chuột khi xoay ngắm cảnh
    float Fov;              // Góc mở trường nhìn hiện tại (Field of View - tính bằng độ)

    // ------------------------------------------------------------------------------------------
    // [CỤM 3: CƠ CHẾ CHUYỂN CẢNH "XUYÊN KÍNH" BẰNG THUẬT TOÁN NỘI SUY LERP (CHƯƠNG CG.4)]
    // ------------------------------------------------------------------------------------------
    bool isDiving = false;  // Trạng thái: false = Trong hầm kính; true = Đang lặn tự do ngoài biển
    glm::vec3 targetPos;    // Tọa độ mục tiêu mà camera đang nội suy lướt tới
    float targetFov;        // Góc mở mục tiêu (60 độ trong hầm -> 85 độ góc rộng khi lặn biển)

    /**
     * @brief Constructor khởi tạo camera tại lối đi giữa sàn đường hầm
     */
    Camera(glm::vec3 startPos = glm::vec3(0.0f, 38.0f, -350.0f)) {
        Position = startPos;
        targetPos = startPos;
        WorldUp = glm::vec3(0.0f, 1.0f, 0.0f);
        Yaw = 90.0f; // Nhìn dọc theo đường hầm (hướng về phía +Z)
        Pitch = 8.0f; // Ngước nhẹ lên vòm kính để thấy ngay đàn cá bơi phía trên
        Front = glm::vec3(0.0f, 0.0f, 1.0f);
        MovementSpeed = 160.0f;
        MouseSensitivity = 0.1f;
        Fov = 65.0f;
        targetFov = 65.0f;
        updateCameraVectors();
    }

    // ------------------------------------------------------------------------------------------
    // [CỤM 4: MA TRẬN QUAN SÁT (VIEW MATRIX - CHƯƠNG CG.4)]
    // ------------------------------------------------------------------------------------------
    /**
     * @brief Tính toán ma trận quan sát View Matrix biến đổi tọa độ thế giới về hệ quy chiếu camera
     * @formula glm::lookAt(Vị_trí_mắt, Điểm_ngắm_tới, Hướng_lên_đỉnh_đầu)
     */
    glm::mat4 GetViewMatrix() const {
        return glm::lookAt(Position, Position + Front, Up);
    }

    // ------------------------------------------------------------------------------------------
    // [CỤM 5: CHUYỂN ĐỔI GIỮA 2 CHẾ ĐỘ TRẢI NGHIỆM KHI NHẤN PHÍM [SPACE]]
    // ------------------------------------------------------------------------------------------
    /**
     * @brief Kích hoạt chuyển cảnh: Lướt xuyên qua vách kính hoặc lùi về lại sàn hầm
     */
    void ToggleDiveMode() {
        isDiving = !isDiving; // Đảo cờ trạng thái

        if (isDiving) {
            // CHẾ ĐỘ 2: Đẩy tọa độ mục tiêu xuyên qua vách kính ra vùng nước mở phía trên đáy biển (Y > 0)
            targetPos = glm::vec3(260.0f, 110.0f, glm::clamp(Position.z, -700.0f, 700.0f));
            targetFov = 85.0f; // Mở rộng FOV điện ảnh choáng ngợp
        } else {
            // CHẾ ĐỘ 1: Lùi về giữa lối đi sàn hầm khô ráo (giới hạn trong chiều dài hầm)
            targetPos = glm::vec3(0.0f, 38.0f, glm::clamp(Position.z, -680.0f, 680.0f));
            targetFov = 65.0f; // Trả lại góc nhìn tiêu chuẩn trong đường hầm
        }
    }

    // ------------------------------------------------------------------------------------------
    // [CỤM 6: XỬ LÝ PHÍM WASD VỚI NỘI SUY TUYẾN TÍNH LERP (CHƯƠNG CG.4)]
    // ------------------------------------------------------------------------------------------
    /**
     * @brief Xử lý sự kiện bàn phím với chuyển động mượt mà
     * @formula P_hiện_tại = glm::mix(P_hiện_tại, P_đích, dt * 3.5)
     */
    void ProcessKeyboard(GLFWwindow* window, float deltaTime) {
        // Thuật toán LERP (Linear Interpolation) làm mượt bước lướt camera
        Position = glm::mix(Position, targetPos, deltaTime * 3.5f);
        Fov      = glm::mix(Fov,      targetFov, deltaTime * 3.5f);

        float velocity = MovementSpeed * deltaTime;

        if (!isDiving) {
            // --- CHẾ ĐỘ 1: BƯỚC ĐI TRONG HẦM KÍNH (Khóa cao độ Y = 35) ---
            glm::vec3 walkFront = glm::normalize(glm::vec3(Front.x, 0.0f, Front.z));
            glm::vec3 walkRight = glm::normalize(glm::vec3(Right.x, 0.0f, Right.z));

            if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) targetPos += walkFront * velocity; // Tiến
            if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) targetPos -= walkFront * velocity; // Lùi
            if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) targetPos -= walkRight * velocity; // Sang trái
            if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) targetPos += walkRight * velocity; // Sang phải

            // Giới hạn người đi bộ trên sàn hầm (Không lọt xuống đáy biển)
            targetPos.y = 35.0f;
            if (targetPos.x > 85.0f)  targetPos.x = 85.0f;
            if (targetPos.x < -85.0f) targetPos.x = -85.0f;
            if (targetPos.z > 680.0f) targetPos.z = 680.0f;
            if (targetPos.z < -680.0f) targetPos.z = -680.0f;
        } else {
            // --- CHẾ ĐỘ 2: BƠI TỰ DO 6 HƯỚNG TRONG LÒNG BIỂN (6-DOF) ---
            if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) targetPos += Front * velocity;
            if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) targetPos -= Front * velocity;
            if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) targetPos -= Right * velocity;
            if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) targetPos += Right * velocity;
            if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) targetPos.y += velocity; // Phím E: Bơi ngoi lên
            if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) targetPos.y -= velocity; // Phím Q: Lặn sâu xuống

            // Không để người lặn chui xuống dưới đáy cát (Y >= 15) hoặc bay ra khỏi mặt nước (Y <= 520)
            if (targetPos.y < 15.0f)  targetPos.y = 15.0f;
            if (targetPos.y > 520.0f) targetPos.y = 520.0f;
        }

        // Bắt sự kiện phím SPACE (Chống dội phím: chỉ kích hoạt 1 lần duy nhất mỗi lần bấm)
        static bool spacePressedLast = false;
        bool spacePressedNow = (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS);
        if (spacePressedNow && !spacePressedLast) {
            ToggleDiveMode();
        }
        spacePressedLast = spacePressedNow;
    }

    // ------------------------------------------------------------------------------------------
    // [CỤM 7: XỬ LÝ XOAY CHUỘT 360 ĐỘ (EULER ANGLES)]
    // ------------------------------------------------------------------------------------------
    /**
     * @brief Tính toán góc quay khi rê chuột và giới hạn góc Pitch để không bị lộn ngược đầu
     */
    void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true) {
        xoffset *= MouseSensitivity;
        yoffset *= MouseSensitivity;

        Yaw   += xoffset; // Xoay ngang
        Pitch += yoffset; // Ngóc lên / chúi xuống

        if (constrainPitch) {
            if (Pitch > 89.0f)  Pitch = 89.0f;  // Khóa góc ngước tối đa 89 độ
            if (Pitch < -89.0f) Pitch = -89.0f; // Khóa góc cúi tối đa -89 độ
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
        
        // Tính tích có hướng (Cross Product) để tìm hệ trục trực giao
        Right = glm::normalize(glm::cross(Front, WorldUp));
        Up    = glm::normalize(glm::cross(Right, Front));
    }
};
