#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include <string>

/**
 * ==============================================================================================
 * 📌 FILE: include/Mesh.h
 * 👤 PHỤ TRÁCH: BẠN B (Frontend / Kỹ sư Cấu trúc Dữ liệu Đỉnh & Buffers)
 * 📚 KIẾN THỨC GIÁO TRÌNH ÁNH XẠ:
 *    - CHƯƠNG CG.2: CƠ CHẾ QUẢN LÝ BỘ ĐỆM ĐỒ HỌA TRONG OPENGL (Buffers Architecture)
 *      -> Cấu trúc đỉnh (Vertex): Position, Normal, TexCoords
 *      -> VAO (Vertex Array Object)
 *      -> VBO (Vertex Buffer Object) & EBO (Element Buffer Object / Index Buffer)
 *      -> Lệnh vẽ tối ưu glDrawElements vs glDrawArrays
 * ==============================================================================================
 */

// ----------------------------------------------------------------------------------------------
// [CỤM 1: CẤU TRÚC ĐỈNH CHUẨN 3D (INTERLEAVED VERTEX STRUCT)]
// ----------------------------------------------------------------------------------------------
struct Vertex {
    glm::vec3 Position;  // 3 floats: Tọa độ không gian 3 chiều (x, y, z)
    glm::vec3 Normal;    // 3 floats: Vector pháp tuyến phục vụ chiếu sáng Blinn-Phong (nx, ny, nz)
    glm::vec2 TexCoords; // 2 floats: Tọa độ dán ảnh vân bề mặt UV (u, v)
};

class Mesh {
public:
    std::vector<Vertex>       vertices; // Mảng danh sách các đỉnh của vật thể
    std::vector<unsigned int> indices;  // Mảng chỉ số đỉnh (Index Buffer) ghép thành các mặt tam giác
    unsigned int VAO;                   // ID quản lý cấu hình đỉnh của Mesh

    // ------------------------------------------------------------------------------------------
    // [CỤM 2: CONSTRUCTOR KHỞI TẠO VÀ CẤU HÌNH BỘ ĐỆM ĐỒ HỌA TRÊN GPU]
    // ------------------------------------------------------------------------------------------
    Mesh(std::vector<Vertex> verts, std::vector<unsigned int> inds) {
        this->vertices = verts;
        this->indices = inds;
        setupMesh(); // Gửi dữ liệu lên GPU
    }

    // ------------------------------------------------------------------------------------------
    // [CỤM 3: HÀM VẼ MESH (DRAW CALL ĐỒ HỌA)]
    // ------------------------------------------------------------------------------------------
    /**
     * @brief Kích hoạt VAO và gửi lệnh vẽ tới GPU
     * @explanation Nếu có mảng chỉ số EBO -> dùng glDrawElements (tiết kiệm bộ nhớ đỉnh).
     *              Nếu không có EBO -> dùng glDrawArrays.
     */
    void Draw(class Shader& shader) const {
        glBindVertexArray(VAO);
        if (!indices.empty()) {
            glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0);
        } else {
            glDrawArrays(GL_TRIANGLES, 0, static_cast<unsigned int>(vertices.size()));
        }
        glBindVertexArray(0); // Hủy gán VAO sau khi vẽ xong
    }

private:
    unsigned int VBO, EBO; // Bộ đệm đỉnh và bộ đệm chỉ số

    // ------------------------------------------------------------------------------------------
    // [CỤM 4: CẤU HÌNH ĐỊNH DẠNG ĐỈNH (VERTEX ATTRIBUTE POINTERS - CHƯƠNG CG.2)]
    // ------------------------------------------------------------------------------------------
    void setupMesh() {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

        // Nếu có danh sách chỉ số đỉnh EBO:
        if (!indices.empty()) {
            glGenBuffers(1, &EBO);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
        }

        // --- Thuộc tính 0: Vị trí đỉnh aPos (layout location = 0) ---
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

        // --- Thuộc tính 1: Vector pháp tuyến aNormal (layout location = 1) ---
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));

        // --- Thuộc tính 2: Tọa độ Texture aTexCoords (layout location = 2) ---
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));

        glBindVertexArray(0);
    }
};
