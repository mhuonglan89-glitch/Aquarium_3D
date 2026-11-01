#pragma once
#include <glm/glm.hpp>
#include <cmath>

/**
 * @file BezierPath.h
 * @brief Thuật toán đường cong Bézier 3D bậc 3 dẫn đường đàn cá & Ma trận phân cấp
 * @author Bạn A (Backend / Math & Bezier) - Môn Đồ Họa Máy Tính (Chương CG.5: Curves and Surfaces & CG.3: Hierarchy)
 */
class BezierPath {
public:
    glm::vec3 P0, P1, P2, P3;

    BezierPath(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3)
        : P0(p0), P1(p1), P2(p2), P3(p3) {}

    // Công thức giải tích đường cong Bézier bậc 3:
    // B(t) = (1-t)^3 * P0 + 3(1-t)^2 * t * P1 + 3(1-t) * t^2 * P2 + t^3 * P3
    glm::vec3 GetPoint(float t) const {
        float u = 1.0f - t;
        float tt = t * t;
        float uu = u * u;
        float uuu = uu * u;
        float ttt = tt * t;

        glm::vec3 p = uuu * P0;
        p += 3.0f * uu * t * P1;
        p += 3.0f * u * tt * P2;
        p += ttt * P3;
        return p;
    }

    // Đạo hàm bậc 1 tính Vector tiếp tuyến (Tangent) để xoay đầu cá theo hướng bơi
    // B'(t) = 3(1-t)^2(P1 - P0) + 6(1-t)t(P2 - P1) + 3t^2(P3 - P2)
    glm::vec3 GetTangent(float t) const {
        float u = 1.0f - t;
        glm::vec3 d = 3.0f * u * u * (P1 - P0)
                    + 6.0f * u * t * (P2 - P1)
                    + 3.0f * t * t * (P3 - P2);
        if (glm::length(d) < 0.0001f) return glm::vec3(0.0f, 0.0f, 1.0f);
        return glm::normalize(d);
    }

    // Tính góc xoay đầu cá trên mặt phẳng nằm ngang XZ (Yaw)
    float GetFacingAngle(float t) const {
        glm::vec3 tangent = GetTangent(t);
        return atan2(tangent.x, tangent.z);
    }

    // Tính góc nghiêng đầu cá khi bơi lặn lên/xuống (Pitch)
    float GetPitchAngle(float t) const {
        glm::vec3 tangent = GetTangent(t);
        float horizLen = std::sqrt(tangent.x * tangent.x + tangent.z * tangent.z);
        return atan2(tangent.y, horizLen);
    }

    // Hàm ma trận phân cấp uốn vây đuôi cá theo thời gian: sin(t * omega) * amplitude (Chương CG.3)
    static float CalculateTailWag(float time, float speed = 6.0f, float amplitudeDeg = 18.0f) {
        return sin(time * speed) * glm::radians(amplitudeDeg);
    }
};
