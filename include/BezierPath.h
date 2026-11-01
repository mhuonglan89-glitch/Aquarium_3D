#pragma once
#include <glm/glm.hpp>
#include <cmath>

/**
 * @file BezierPath.h
 * @brief Thuật toán đường cong Bézier 3D bậc 3 dẫn đường đàn cá
 * @author Bạn A (Backend / Math & Bezier) - Môn Đồ Họa Máy Tính (Chương CG.5)
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
        return glm::normalize(d);
    }

    // Tính góc xoay đầu cá trên mặt phẳng nằm ngang XZ
    float GetFacingAngle(float t) const {
        glm::vec3 tangent = GetTangent(t);
        return atan2(tangent.x, tangent.z);
    }

    // Hàm ma trận phân cấp uốn vây đuôi cá theo thời gian: sin(t * omega) * amplitude
    static float CalculateTailWag(float time, float speed = 8.0f, float amplitudeDeg = 20.0f) {
        return sin(time * speed) * glm::radians(amplitudeDeg);
    }
};
