#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include <cmath>
#include "Shader.h"

/**
 * ==============================================================================================
 * 📌 FILE: include/Tunnel.h
 * 👤 PHỤ TRÁCH: BẠN A (Backend / Kỹ sư Toán học & Hình học Tham số)
 * 📚 KIẾN THỨC GIÁO TRÌNH ÁNH XẠ:
 *    - CHƯƠNG CG.5: ĐƯỜNG CONG VÀ MẶT CONG (Curves and Surfaces)
 *      -> Mặt cong tham số (Parametric Surface): Nửa hình trụ tròn nằm dọc trục Z
 *      -> Chia lưới đỉnh tam giác (Triangulation of Quad Mesh)
 *      -> Vector pháp tuyến giải tích xuyên tâm (Analytical Radial Normals)
 *    - CHƯƠNG CG.6: KỸ THUẬT ĐỒ HỌA NÂNG CAO (Advanced Techniques)
 *      -> Kính Acrylic trong suốt (Alpha Blending: glBlendFunc)
 *      -> Quản lý bộ đệm Z-Buffer khi vẽ kính (glDepthMask)
 * ==============================================================================================
 */

class Tunnel {
public:
    // ------------------------------------------------------------------------------------------
    // [CỤM 1: CÁC ĐỐI TƯỢNG QUẢN LÝ BỘ ĐỆM ĐỒ HỌA OPENGL (VAO & VBO)]
    // ------------------------------------------------------------------------------------------
    // VAO (Vertex Array Object): Lưu cấu hình định dạng thuộc tính đỉnh (Vertex Attributes)
    // VBO (Vertex Buffer Object): Chứa mảng dữ liệu tọa độ đỉnh thô đưa vào VRAM GPU
    unsigned int VAO_Glass, VBO_Glass; // Bộ đệm vòm kính Acrylic trong suốt
    unsigned int VAO_Floor, VBO_Floor; // Bộ đệm sàn đi bộ đá mài tối màu
    unsigned int VAO_Beams, VBO_Beams; // Bộ đệm các khung dầm thép chịu lực uốn cong
    unsigned int VAO_Leds,  VBO_Leds;  // Bộ đệm dải đèn LED cyan phát quang dẫn lối

    int glassVertexCount = 0; // Số lượng đỉnh vẽ vòm kính
    int floorVertexCount = 0; // Số lượng đỉnh vẽ sàn hầm
    int beamVertexCount  = 0; // Số lượng đỉnh vẽ dầm thép
    int ledVertexCount   = 0; // Số lượng đỉnh vẽ dải LED

    float Radius; // Bán kính vòm kính (R = 160.0f)
    float Length; // Chiều dài hầm thủy cung (L = 1600.0f)

    /**
     * @brief Constructor khởi tạo toàn bộ hình học hầm thủy cung bằng toán học giải tích
     */
    Tunnel(float radius = 160.0f, float length = 1600.0f, int segments = 36, int stepsZ = 32)
        : Radius(radius), Length(length) {
        
        // Bước 1: Sinh lưới vòm kính nửa hình trụ tham số
        generateGlassArch(radius, length, segments, stepsZ);

        // Bước 2: Sinh mặt phẳng sàn đi bộ bằng đá mài (khít với chân vòm kính bán kính R)
        generateFloor(radius, length, stepsZ);

        // Bước 3: Sinh các dầm thép chịu lực cách nhau mỗi 100 units
        generateSteelBeams(radius, length, segments, 100.0f);

        // Bước 4: Sinh 2 dải đèn LED cyan chạy dọc mép lối đi
        generateLedStrips(radius * 0.78f, length, stepsZ);
    }

    /**
     * @brief Destructor giải phóng bộ nhớ VRAM trên GPU tránh rò rỉ bộ nhớ
     */
    ~Tunnel() {
        glDeleteVertexArrays(1, &VAO_Glass); glDeleteBuffers(1, &VBO_Glass);
        glDeleteVertexArrays(1, &VAO_Floor); glDeleteBuffers(1, &VBO_Floor);
        glDeleteVertexArrays(1, &VAO_Beams); glDeleteBuffers(1, &VBO_Beams);
        glDeleteVertexArrays(1, &VAO_Leds);  glDeleteBuffers(1, &VBO_Leds);
    }

    // ------------------------------------------------------------------------------------------
    // [CỤM 2: HÀM VẼ TOÀN BỘ CÔNG TRÌNH HẦM THỦY CUNG (DRAW CALL)]
    // ------------------------------------------------------------------------------------------
    /**
     * @brief Vẽ lần lượt các thành phần của hầm tuân thủ nghiêm ngặt thứ tự khử mặt khuất
     * @rule Bất biến trong đồ họa: Vẽ vật thể ĐẶC trước -> Vẽ vật thể TRONG SUỐT sau cùng!
     */
    void Draw(Shader& shader) {
        shader.setBool("useTexture", false);
        shader.setBool("isFish", false);

        // --- 1. VẼ SÀN HẦM ĐI BỘ (Vật thể đặc) ---
        shader.setVec3("objectColor", glm::vec3(0.08f, 0.12f, 0.18f)); // Màu đá mài Titanium Steel
        shader.setFloat("alphaVal", 1.0f);                              // Hoàn toàn không trong suốt
        glBindVertexArray(VAO_Floor);
        glDrawArrays(GL_TRIANGLES, 0, floorVertexCount);

        // --- 2. VẼ 2 DẢI ĐÈN LED NEON DẪN LỐI (Vật thể phát quang) ---
        shader.setVec3("objectColor", glm::vec3(0.047f, 0.910f, 0.855f)); // Sắc xanh Neon Cyan (#0CE8DA)
        shader.setFloat("alphaVal", 1.0f);
        glBindVertexArray(VAO_Leds);
        glDrawArrays(GL_TRIANGLES, 0, ledVertexCount);

        // --- 3. VẼ CÁC KHUNG DẦM THÉP CHỊU LỰC UỐN CONG ---
        shader.setVec3("objectColor", glm::vec3(0.18f, 0.22f, 0.30f)); // Khung kim loại xám sẫm
        shader.setFloat("alphaVal", 1.0f);
        glLineWidth(3.5f); // Đặt độ dày nét vẽ dầm thép
        glBindVertexArray(VAO_Beams);
        glDrawArrays(GL_LINES, 0, beamVertexCount);

        // --- 4. VẼ VÒM KÍNH ACRYLIC TRONG SUỐT (ALPHA BLENDING - CHƯƠNG CG.6) ---
        glEnable(GL_BLEND);                                // Bật chế độ hòa trộn màu Alpha
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // Công thức: C = C_src*alpha + C_dst*(1-alpha)
        glDepthMask(GL_FALSE);                             // TẠM DỪNG GHI ĐỘ SÂU để không che mất cá bơi phía sau kính!
        
        shader.setVec3("objectColor", glm::vec3(0.35f, 0.78f, 0.95f)); // Màu phản quang xanh lơ nhẹ của kính
        shader.setFloat("alphaVal", 0.18f);                             // Kính trong suốt cao (18% mờ) giúp nhìn rõ cá bên ngoài
        glBindVertexArray(VAO_Glass);
        glDrawArrays(GL_TRIANGLES, 0, glassVertexCount);

        glDepthMask(GL_TRUE);                              // KHÔI PHỤC LẠI GHI ĐỘ SÂU cho các lượt vẽ sau
        glBindVertexArray(0);                              // Hủy gán VAO
    }

private:
    // ------------------------------------------------------------------------------------------
    // [CỤM 3: TOÁN HỌC THAM SỐ SINH LƯỚI VÒM KÍNH NỬA HÌNH TRỤ (CHƯƠNG CG.5)]
    // ------------------------------------------------------------------------------------------
    /**
     * @brief Sinh lưới tam giác vòm kính bằng công thức tham số hình trụ
     * @formula x = R*cos(theta), y = R*sin(theta), z in [-L/2, L/2] với theta in [0, pi]
     * @explanation
     *   - segments (36): Chia góc nửa đường tròn thành 36 bước góc.
     *   - stepsZ (32): Chia chiều dài hầm thành 32 đốt hình học.
     *   - Mỗi ô lưới tứ giác gồm 4 điểm (v1, v2, v3, v4) được chia thành 2 tam giác OpenGL.
     */
    void generateGlassArch(float r, float l, int segs, int stepsZ) {
        std::vector<float> verts;
        float stepZ = l / stepsZ;

        for (int i = 0; i < segs; ++i) {
            float t1 = 3.14159265f * (float)i / segs;       // Góc theta bắt đầu của ô
            float t2 = 3.14159265f * (float)(i + 1) / segs; // Góc theta kết thúc của ô

            for (int j = 0; j < stepsZ; ++j) {
                float z1 = -l / 2.0f + j * stepZ;           // Tọa độ Z đầu đốt
                float z2 = z1 + stepZ;                      // Tọa độ Z cuối đốt

                // 4 đỉnh của 1 ô tứ giác tham số:
                glm::vec3 v1(r * cos(t1), r * sin(t1), z1); // Góc trên - trái
                glm::vec3 v2(r * cos(t2), r * sin(t2), z1); // Góc trên - phải
                glm::vec3 v3(r * cos(t1), r * sin(t1), z2); // Góc dưới - trái
                glm::vec3 v4(r * cos(t2), r * sin(t2), z2); // Góc dưới - phải

                // Tam giác 1: (v1, v2, v3)
                pushVertex(verts, v1, (float)i/segs,     (float)j/stepsZ);
                pushVertex(verts, v2, (float)(i+1)/segs, (float)j/stepsZ);
                pushVertex(verts, v3, (float)i/segs,     (float)(j+1)/stepsZ);

                // Tam giác 2: (v2, v4, v3)
                pushVertex(verts, v2, (float)(i+1)/segs, (float)j/stepsZ);
                pushVertex(verts, v4, (float)(i+1)/segs, (float)(j+1)/stepsZ);
                pushVertex(verts, v3, (float)i/segs,     (float)(j+1)/stepsZ);
            }
        }

        glassVertexCount = verts.size() / 8; // Mỗi đỉnh có 8 thuộc tính (3 pos, 3 norm, 2 uv)
        setupBuffer(VAO_Glass, VBO_Glass, verts);
    }

    // ------------------------------------------------------------------------------------------
    // [CỤM 4: SINH HÌNH HỌC SÀN ĐI BỘ BẰNG PHẲNG DỌC THEO HÀNH LANG]
    // ------------------------------------------------------------------------------------------
    void generateFloor(float width, float l, int stepsZ) {
        std::vector<float> verts;
        float stepZ = l / stepsZ;

        for (int j = 0; j < stepsZ; ++j) {
            float z1 = -l / 2.0f + j * stepZ;
            float z2 = z1 + stepZ;

            glm::vec3 v1(-width, 0.0f, z1);
            glm::vec3 v2( width, 0.0f, z1);
            glm::vec3 v3(-width, 0.0f, z2);
            glm::vec3 v4( width, 0.0f, z2);

            pushFloorVertex(verts, v1); pushFloorVertex(verts, v2); pushFloorVertex(verts, v3);
            pushFloorVertex(verts, v2); pushFloorVertex(verts, v4); pushFloorVertex(verts, v3);
        }

        floorVertexCount = verts.size() / 8;
        setupBuffer(VAO_Floor, VBO_Floor, verts);
    }

    // ------------------------------------------------------------------------------------------
    // [CỤM 5: SINH CÁC DẦM THÉP CHỊU LỰC UỐN CONG NÂNG ĐỠ VÒM KÍNH]
    // ------------------------------------------------------------------------------------------
    void generateSteelBeams(float r, float l, int segs, float spacing) {
        std::vector<float> verts;
        int beamCount = static_cast<int>(l / spacing);
        float startZ = -l / 2.0f;

        for (int b = 0; b <= beamCount; ++b) {
            float z = startZ + b * spacing;
            for (int i = 0; i < segs; ++i) {
                float t1 = 3.14159265f * (float)i / segs;
                float t2 = 3.14159265f * (float)(i + 1) / segs;

                glm::vec3 p1(r * cos(t1), r * sin(t1), z);
                glm::vec3 p2(r * cos(t2), r * sin(t2), z);

                pushVertex(verts, p1, 0.0f, 0.0f);
                pushVertex(verts, p2, 1.0f, 0.0f);
            }
        }

        beamVertexCount = verts.size() / 8;
        setupBuffer(VAO_Beams, VBO_Beams, verts);
    }

    // ------------------------------------------------------------------------------------------
    // [CỤM 6: SINH 2 DẢI ĐÈN LED NEON DẪN LỐI CHẠY DỌC CHÂN MÉP SÀN]
    // ------------------------------------------------------------------------------------------
    void generateLedStrips(float edgeX, float l, int stepsZ) {
        std::vector<float> verts;
        float stepZ = l / stepsZ;
        float ledWidth = 4.0f; // Bề rộng dải LED phát sáng là 4 đơn vị

        for (int j = 0; j < stepsZ; ++j) {
            float z1 = -l / 2.0f + j * stepZ;
            float z2 = z1 + stepZ;

            // Dải LED bên trái sàn
            glm::vec3 l1(-edgeX, 0.2f, z1);
            glm::vec3 l2(-edgeX + ledWidth, 0.2f, z1);
            glm::vec3 l3(-edgeX, 0.2f, z2);
            glm::vec3 l4(-edgeX + ledWidth, 0.2f, z2);

            pushFloorVertex(verts, l1); pushFloorVertex(verts, l2); pushFloorVertex(verts, l3);
            pushFloorVertex(verts, l2); pushFloorVertex(verts, l4); pushFloorVertex(verts, l3);

            // Dải LED bên phải sàn
            glm::vec3 r1(edgeX - ledWidth, 0.2f, z1);
            glm::vec3 r2(edgeX, 0.2f, z1);
            glm::vec3 r3(edgeX - ledWidth, 0.2f, z2);
            glm::vec3 r4(edgeX, 0.2f, z2);

            pushFloorVertex(verts, r1); pushFloorVertex(verts, r2); pushFloorVertex(verts, r3);
            pushFloorVertex(verts, r2); pushFloorVertex(verts, r4); pushFloorVertex(verts, r3);
        }

        ledVertexCount = verts.size() / 8;
        setupBuffer(VAO_Leds, VBO_Leds, verts);
    }

    // ------------------------------------------------------------------------------------------
    // [CỤM 7: HÀM NẠP DỮ LIỆU ĐỈNH VÀO BỘ ĐỆM VRAM GPU (VAO / VBO)]
    // ------------------------------------------------------------------------------------------
    void setupBuffer(unsigned int& vao, unsigned int& vbo, const std::vector<float>& verts) {
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float), verts.data(), GL_STATIC_DRAW);

        // aPos (location = 0): 3 giá trị float tọa độ X, Y, Z
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        // aNormal (location = 1): 3 giá trị float vector pháp tuyến nx, ny, nz
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        // aTexCoords (location = 2): 2 giá trị float tọa độ vân bề mặt U, V
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);
        
        glBindVertexArray(0);
    }

    // Đẩy thuộc tính 1 đỉnh vòm kính: vị trí pos, vector pháp tuyến xuyên tâm norm, tọa độ texture (u,v)
    void pushVertex(std::vector<float>& buf, glm::vec3 pos, float u, float v) {
        buf.push_back(pos.x); buf.push_back(pos.y); buf.push_back(pos.z);
        // Pháp tuyến mặt trụ tròn: Vector xuyên tâm tính từ trục Z (0, 0, z) hướng ra điểm (x, y, z)
        glm::vec3 norm = glm::normalize(glm::vec3(pos.x, pos.y, 0.0f));
        buf.push_back(norm.x); buf.push_back(norm.y); buf.push_back(norm.z);
        buf.push_back(u); buf.push_back(v);
    }

    // Đẩy thuộc tính 1 đỉnh sàn: pháp tuyến luôn hướng thẳng lên trời (0, 1, 0)
    void pushFloorVertex(std::vector<float>& buf, glm::vec3 pos) {
        buf.push_back(pos.x); buf.push_back(pos.y); buf.push_back(pos.z);
        buf.push_back(0.0f); buf.push_back(1.0f); buf.push_back(0.0f);
        buf.push_back(pos.x * 0.05f); buf.push_back(pos.z * 0.05f);
    }
};
