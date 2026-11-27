#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <cmath>

/**
 * ==============================================================================================
 * 📌 FILE: include/BezierPath.h
 * 👤 PHỤ TRÁCH: BẠN A (Backend / Kỹ sư Toán học & Quỹ đạo Không gian)
 * 📚 KIẾN THỨC GIÁO TRÌNH ÁNH XẠ:
 *    - CHƯƠNG CG.5: ĐƯỜNG CONG VÀ MẶT CONG (Curves and Surfaces)
 *      -> Thuật toán đường cong tham số Cubic Bézier 3D (Đa thức Bernstein bậc 3)
 *      -> Đạo hàm tiếp tuyến bậc 1 (First Derivative / Tangent Vector)
 *    - CHƯƠNG CG.3: BIẾN ĐỔI HÌNH HỌC (3D Transformations)
 *      -> Góc xoay hướng bơi Yaw (atan2) và Pitch (góc ngóc/chúi đầu)
 * ==============================================================================================
 */

class BezierPath {
public:
    // ------------------------------------------------------------------------------------------
    // [CỤM 1: 4 ĐIỂM KIỂM SOÁT (CONTROL POINTS) CỦA ĐƯỜNG CONG BÉZIER BẬC 3]
    // ------------------------------------------------------------------------------------------
    // P0: Điểm xuất phát (Start point) tại t = 0
    // P1: Điểm điều khiển 1 (Uốn cong quỹ đạo về hướng điểm này)
    // P2: Điểm điều khiển 2 (Uốn cong quỹ đạo chuẩn bị tới đích)
    // P3: Điểm kết thúc (End point) tại t = 1
    glm::vec3 P0, P1, P2, P3;

    /**
     * @brief Constructor khởi tạo 4 điểm kiểm soát cho quỹ đạo bơi của từng loài cá
     */
    BezierPath(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3)
        : P0(p0), P1(p1), P2(p2), P3(p3) {}

    // ------------------------------------------------------------------------------------------
    // [CỤM 2: THUẬT TOÁN TÍNH TỌA ĐỘ VỊ TRÍ B(t) BẰNG ĐA THỨC BERNSTEIN BẬC 3]
    // ------------------------------------------------------------------------------------------
    /**
     * @brief Tính tọa độ 3D tại thời điểm tham số t (t chạy từ 0.0f đến 1.0f)
     * @formula B(t) = (1-t)³*P0 + 3(1-t)²*t*P1 + 3(1-t)*t²*P2 + t³*P3
     * @explanation
     *   - u = (1 - t)
     *   - u³*P0: Ảnh hưởng của điểm đầu P0 (giảm dần khi t tăng)
     *   - 3u²t*P1: Lực kéo uốn cong của điểm điều khiển P1
     *   - 3ut²*P2: Lực kéo uốn cong của điểm điều khiển P2
     *   - t³*P3: Ảnh hưởng của điểm đích P3 (tăng dần khi t về 1)
     */
    glm::vec3 GetPoint(float t) const {
        float u = 1.0f - t;
        float tt = t * t;       // t²
        float uu = u * u;       // (1 - t)²
        float uuu = uu * u;     // (1 - t)³
        float ttt = tt * t;     // t³

        glm::vec3 p = uuu * P0;             // Trọng số cho P0
        p += 3.0f * uu * t * P1;            // Trọng số cho P1
        p += 3.0f * u * tt * P2;            // Trọng số cho P2
        p += ttt * P3;                      // Trọng số cho P3
        return p;
    }

    // ------------------------------------------------------------------------------------------
    // [CỤM 3: ĐẠO HÀM BẬC 1 TÍNH VECTOR TIẾP TUYẾN CHỈ HƯỚNG BƠI (TANGENT VECTOR)]
    // ------------------------------------------------------------------------------------------
    /**
     * @brief Tính đạo hàm dB(t)/dt để tìm vector vận tốc / hướng di chuyển tức thời
     * @formula B'(t) = 3(1-t)²*(P1 - P0) + 6(1-t)*t*(P2 - P1) + 3*t²*(P3 - P2)
     * @why_important
     *   Nếu không có hàm này, con cá chỉ dịch chuyển vị trí nhưng thân cá sẽ đứng yên
     *   hoặc bị bơi giật cục/bơi lùi. Vector tiếp tuyến chỉ rõ hướng đầu cá cần phải quay vào!
     */
    glm::vec3 GetTangent(float t) const {
        float u = 1.0f - t;
        glm::vec3 d = 3.0f * u * u * (P1 - P0)
                    + 6.0f * u * t * (P2 - P1)
                    + 3.0f * t * t * (P3 - P2);
        
        // Chuẩn hóa vector về độ dài 1 (Unit vector) để tránh lỗi chia cho 0
        if (glm::length(d) < 0.0001f) 
            return glm::vec3(0.0f, 0.0f, 1.0f);
        return glm::normalize(d);
    }

    // ------------------------------------------------------------------------------------------
    // [CỤM 4: TÍNH GÓC XOAY ĐẦU CÁ THEO HƯỚNG BƠI TRÊN MẶT PHẲNG XZ (YAW ANGLE)]
    // ------------------------------------------------------------------------------------------
    /**
     * @brief Tính góc xoay quanh trục Y (Yaw) bằng hàm arctan2(tangent.x, tangent.z)
     * @return float Góc radian dùng cho hàm glm::rotate(model, angle, vec3(0, 1, 0))
     * @tip Khi thầy hỏi: "Làm sao cá biết quay đầu đúng hướng đường cong?",
     *      trả lời: "Dạ em lấy đạo hàm tiếp tuyến B'(t) rồi tính atan2(Tx, Tz) ạ!"
     */
    float GetFacingAngle(float t) const {
        glm::vec3 tangent = GetTangent(t);
        return atan2(tangent.x, tangent.z);
    }

    // ------------------------------------------------------------------------------------------
    // [CỤM 5: TÍNH GÓC NGHIÊNG ĐẦU CÁ KHI BƠI LẶN LÊN / XUỐNG (PITCH ANGLE)]
    // ------------------------------------------------------------------------------------------
    /**
     * @brief Tính góc nghiêng quanh trục X (Pitch) khi cá bơi ngóc lên mặt nước hoặc lặn xuống đáy
     * @return float Góc radian dùng cho hàm glm::rotate(model, pitch, vec3(1, 0, 0))
     */
    float GetPitchAngle(float t) const {
        glm::vec3 tangent = GetTangent(t);
        float horizLen = std::sqrt(tangent.x * tangent.x + tangent.z * tangent.z);
        return atan2(tangent.y, horizLen);
    }

    // ------------------------------------------------------------------------------------------
    // [CỤM 6: THUẬT TOÁN MA TRẬN PHÂN CẤP VẪY ĐUÔI CÁ THEO HÀM SIN (HIERARCHICAL WIGGLE)]
    // ------------------------------------------------------------------------------------------
    /**
     * @brief Tính góc dao động uốn lượn vây đuôi cá theo thời gian
     * @formula theta_tail = sin(time * speed) * amplitudeDeg
     * @note Thuộc Chương CG.3: Phép biến đổi ma trận phân cấp (Thân cá -> Vây đuôi)
     */
    static float CalculateTailWag(float time, float speed = 6.0f, float amplitudeDeg = 18.0f) {
        return sin(time * speed) * glm::radians(amplitudeDeg);
    }
};

// ----------------------------------------------------------------------------------------------
// [CỤM 7: ĐƯỜNG CONG KHÉP KÍN CATMULL-ROM SPLINE 3D (CHƯƠNG CG.5 - QUỸ ĐẠO TUẦN TRA VÔ HẠN)]
// ----------------------------------------------------------------------------------------------
/**
 * @brief Cho phép cá bơi tuần tra khép kín qua N điểm kiểm soát P0 -> P1 -> ... -> Pn -> P0
 *        mà không bị giật cục khi quay lại điểm xuất phát (đảm bảo liên tục C1 tại điểm nối).
 */
class ClosedSplinePath {
public:
    std::vector<glm::vec3> points;

    ClosedSplinePath(const std::vector<glm::vec3>& pts) : points(pts) {}

    glm::vec3 GetPoint(float t) const {
        if (points.size() < 4) return glm::vec3(0.0f);
        t = t - std::floor(t); // Giữ t trong [0, 1)
        int n = static_cast<int>(points.size());
        float scaled = t * n;
        int idx = static_cast<int>(scaled) % n;
        float u = scaled - std::floor(scaled);

        const glm::vec3& p0 = points[(idx - 1 + n) % n];
        const glm::vec3& p1 = points[idx];
        const glm::vec3& p2 = points[(idx + 1) % n];
        const glm::vec3& p3 = points[(idx + 2) % n];

        float u2 = u * u;
        float u3 = u2 * u;

        return 0.5f * ((2.0f * p1) +
                       (-p0 + p2) * u +
                       (2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3) * u2 +
                       (-p0 + 3.0f * p1 - 3.0f * p2 + p3) * u3);
    }

    glm::vec3 GetTangent(float t) const {
        if (points.size() < 4) return glm::vec3(0.0f, 0.0f, 1.0f);
        t = t - std::floor(t);
        int n = static_cast<int>(points.size());
        float scaled = t * n;
        int idx = static_cast<int>(scaled) % n;
        float u = scaled - std::floor(scaled);

        const glm::vec3& p0 = points[(idx - 1 + n) % n];
        const glm::vec3& p1 = points[idx];
        const glm::vec3& p2 = points[(idx + 1) % n];
        const glm::vec3& p3 = points[(idx + 2) % n];

        float u2 = u * u;
        glm::vec3 d = 0.5f * ((-p0 + p2) +
                              2.0f * (2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3) * u +
                              3.0f * (-p0 + 3.0f * p1 - 3.0f * p2 + p3) * u2);

        if (glm::length(d) < 1e-5f) return glm::vec3(0.0f, 0.0f, 1.0f);
        return glm::normalize(d);
    }

    float GetFacingAngle(float t) const {
        glm::vec3 tangent = GetTangent(t);
        return atan2(tangent.x, tangent.z);
    }

    float GetPitchAngle(float t) const {
        glm::vec3 tangent = GetTangent(t);
        float horizLen = std::sqrt(tangent.x * tangent.x + tangent.z * tangent.z);
        return atan2(-tangent.y, horizLen);
    }
};
