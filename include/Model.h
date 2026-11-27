#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <stb_image.h>
#include "Mesh.h"
#include "Shader.h"

/**
 * ==============================================================================================
 * 📌 FILE: include/Model.h
 * 👤 PHỤ TRÁCH: BẠN B (Frontend / Kỹ sư Nạp Mô Hình 3D & Dán Vân Bề Mặt)
 * 📚 KIẾN THỨC GIÁO TRÌNH ÁNH XẠ:
 *    - CHƯƠNG CG.2 & CG.6: MÔ HÌNH HÓA VÀ DÁN VÂN BỀ MẶT (Texture Mapping & 3D Formats)
 *      -> Đọc và phân tích định dạng Wavefront (.obj) chuẩn công nghiệp đồ họa
 *      -> Thuật toán Fan Triangulation (chia đa giác n-gon thành tam giác OpenGL)
 *      -> Tính vector pháp tuyến phẳng (Flat Normals via Cross Product)
 *      -> Nạp ảnh kết cấu Texture (JPG/PNG) qua thư viện stb_image.h
 *      -> Dựng Mesh dự phòng hình học (Fallback Procedural Fish Mesh)
 * ==============================================================================================
 */

class Model {
public:
    std::vector<Mesh> meshes; // Danh sách các Mesh cấu thành mô hình
    glm::vec3 defaultColor;   // Màu cơ sở mặc định của vật thể
    unsigned int textureID = 0; // ID của Texture trên GPU
    bool hasTexture = false;  // Cờ kiểm tra: Vật thể có dùng ảnh Texture hay không

    // ------------------------------------------------------------------------------------------
    // [CỤM 1: CONSTRUCTOR NẠP FILE MÔ HÌNH .OBJ VÀ FILE TEXTURE ĐI KÈM]
    // ------------------------------------------------------------------------------------------
    Model(const std::string& objPath, glm::vec3 color = glm::vec3(0.3f, 0.6f, 0.8f), const std::string& texPath = "") {
        defaultColor = color;
        loadOBJ(objPath);          // 1. Phân tích hình học .obj
        if (!texPath.empty()) {
            loadTexture(texPath);  // 2. Nạp ảnh vân bề mặt nếu có
        }
    }

    ~Model() {
        if (hasTexture && textureID != 0) {
            glDeleteTextures(1, &textureID); // Giải phóng bộ nhớ VRAM của Texture
        }
    }

    // ------------------------------------------------------------------------------------------
    // [CỤM 2: HÀM VẼ TOÀN BỘ CÁC MESH VỚI ÁNH XẠ VẬT LIỆU]
    // ------------------------------------------------------------------------------------------
    void Draw(Shader& shader) {
        shader.setVec3("objectColor", defaultColor);
        shader.setFloat("alphaVal", 1.0f); // Vật thể đặc có độ đục 100%

        if (hasTexture) {
            shader.setBool("useTexture", true);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, textureID);
            shader.setInt("texture_diffuse1", 0);
        } else {
            shader.setBool("useTexture", false);
        }

        // Vẽ lần lượt tất cả các mesh của mô hình
        for (unsigned int i = 0; i < meshes.size(); i++) {
            meshes[i].Draw(shader);
        }

        if (hasTexture) {
            glBindTexture(GL_TEXTURE_2D, 0); // Hủy gán texture sau khi vẽ xong
        }
    }

private:
    // ------------------------------------------------------------------------------------------
    // [CỤM 3: BỘ PHÂN TÍCH ĐỊNH DẠNG TỆP WAVEFRONT OBJ TỰ XÂY DỰNG]
    // ------------------------------------------------------------------------------------------
    /**
     * @brief Đọc từng dòng của file văn bản .obj:
     *   - 'v ': Tọa độ vị trí đỉnh (Geometric Vertex)
     *   - 'vn': Vector pháp tuyến đỉnh (Vertex Normal)
     *   - 'vt': Tọa độ vân bề mặt (Texture Coordinates)
     *   - 'f ': Chỉ số ghép mặt đa giác (Face format: v/vt/vn)
     */
    void loadOBJ(const std::string& path) {
        std::vector<glm::vec3> temp_positions;
        std::vector<glm::vec3> temp_normals;
        std::vector<glm::vec2> temp_texcoords;
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;

        std::ifstream file(path);
        if (!file.is_open()) {
            std::cerr << "[WARN::MODEL] Khong the mo file .obj tai: " << path << ". Khoi tao Mesh mac dinh." << std::endl;
            createFallbackFishMesh();
            return;
        }

        std::string line;
        while (std::getline(file, line)) {
            // 1. Phân tích tọa độ đỉnh vị trí (v x y z)
            if (line.substr(0, 2) == "v ") {
                std::istringstream s(line.substr(2));
                glm::vec3 v;
                s >> v.x >> v.y >> v.z;
                temp_positions.push_back(v);
            }
            // 2. Phân tích vector pháp tuyến (vn nx ny nz)
            else if (line.substr(0, 3) == "vn ") {
                std::istringstream s(line.substr(3));
                glm::vec3 vn;
                s >> vn.x >> vn.y >> vn.z;
                temp_normals.push_back(vn);
            }
            // 3. Phân tích tọa độ dán vân bề mặt (vt u v)
            else if (line.substr(0, 3) == "vt ") {
                std::istringstream s(line.substr(3));
                glm::vec2 vt;
                s >> vt.x >> vt.y;
                temp_texcoords.push_back(vt);
            }
            // 4. Phân tích mặt đa giác (f v1/vt1/vn1 v2/vt2/vn2 v3/vt3/vn3 ...)
            else if (line.substr(0, 2) == "f ") {
                std::istringstream s(line.substr(2));
                std::string vString;
                std::vector<unsigned int> faceIndices;

                while (s >> vString) {
                    unsigned int vIdx = 0, vtIdx = 0, vnIdx = 0;
                    std::replace(vString.begin(), vString.end(), '/', ' '); // Thay ký tự '/' bằng dấu cách
                    std::istringstream vStream(vString);

                    vStream >> vIdx;
                    if (vStream >> vtIdx) {
                        vStream >> vnIdx;
                    }

                    Vertex vertex;
                    // Lấy vị trí đỉnh (lưu ý chỉ số trong file .obj bắt đầu từ 1)
                    if (vIdx > 0 && vIdx <= temp_positions.size())
                        vertex.Position = temp_positions[vIdx - 1];
                    else
                        vertex.Position = glm::vec3(0.0f);

                    // Lấy vector pháp tuyến
                    if (vnIdx > 0 && vnIdx <= temp_normals.size())
                        vertex.Normal = temp_normals[vnIdx - 1];
                    else
                        vertex.Normal = glm::vec3(0.0f, 1.0f, 0.0f);

                    // Lấy tọa độ UV
                    if (vtIdx > 0 && vtIdx <= temp_texcoords.size())
                        vertex.TexCoords = temp_texcoords[vtIdx - 1];
                    else
                        vertex.TexCoords = glm::vec2(vertex.Position.x * 0.05f, vertex.Position.z * 0.05f);

                    vertices.push_back(vertex);
                    faceIndices.push_back(static_cast<unsigned int>(vertices.size() - 1));
                }

                // --------------------------------------------------------------------------------------
                // [CỤM 4: THUẬT TOÁN FAN TRIANGULATION CHIA ĐA GIÁC N-GON THÀNH TAM GIÁC OPENGL]
                // --------------------------------------------------------------------------------------
                // Với đa giác có k đỉnh (v0, v1, v2, ... vk-1), chia thành các tam giác:
                // (v0, v1, v2), (v0, v2, v3), ..., (v0, vk-2, vk-1)
                for (size_t k = 1; k + 1 < faceIndices.size(); ++k) {
                    indices.push_back(faceIndices[0]);
                    indices.push_back(faceIndices[k]);
                    indices.push_back(faceIndices[k + 1]);
                }
            }
        }
        file.close();

        if (vertices.empty()) {
            createFallbackFishMesh();
        } else {
            // Tự tính vector pháp tuyến nếu model tải về không có thuộc tính 'vn'
            if (temp_normals.empty()) {
                computeFlatNormals(vertices, indices);
            }
            meshes.push_back(Mesh(vertices, indices));
            std::cout << "[INFO::MODEL] Da nap thanh cong: " << path 
                      << " (" << vertices.size() << " dinh, " << (indices.size()/3) << " tam giac)" << std::endl;
        }
    }

    // ------------------------------------------------------------------------------------------
    // [CỤM 5: NẠP VÀ CẤU HÌNH TEXTURE BẰNG THƯ VIỆN STB_IMAGE (CHƯƠNG CG.6)]
    // ------------------------------------------------------------------------------------------
    void loadTexture(const std::string& path) {
        int width, height, nrChannels;
        stbi_set_flip_vertically_on_load(true); // Lật trục Y vì tọa độ ảnh khác hệ trục OpenGL
        unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);
        if (data) {
            GLenum format = GL_RGB;
            if (nrChannels == 1) format = GL_RED;
            else if (nrChannels == 3) format = GL_RGB;
            else if (nrChannels == 4) format = GL_RGBA;

            glGenTextures(1, &textureID);
            glBindTexture(GL_TEXTURE_2D, textureID);
            glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D); // Tự động sinh chuỗi Mipmap thu nhỏ nhiều cấp độ

            // Cấu hình lặp ảnh (Wrap mode) và lọc ảnh mượt mà (Bilinear Filtering)
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            stbi_image_free(data);
            hasTexture = true;
            std::cout << "[INFO::TEXTURE] Da nap texture: " << path 
                      << " (" << width << "x" << height << ", " << nrChannels << " channels)" << std::endl;
        } else {
            std::cerr << "[WARN::TEXTURE] Khong the doc file texture tai: " << path << std::endl;
        }
    }

    // ------------------------------------------------------------------------------------------
    // [CỤM 6: THUẬT TOÁN TÍNH VECTOR PHÁP TUYẾN BẰNG TÍCH CÓ HƯỚNG (CROSS PRODUCT)]
    // ------------------------------------------------------------------------------------------
    /**
     * @brief Tính vector pháp tuyến đơn vị vuông góc với mặt tam giác (v1, v2, v3)
     * @formula N = normalize((v2 - v1) x (v3 - v1))
     */
    void computeFlatNormals(std::vector<Vertex>& verts, const std::vector<unsigned int>& inds) {
        for (size_t i = 0; i + 2 < inds.size(); i += 3) {
            unsigned int i1 = inds[i];
            unsigned int i2 = inds[i + 1];
            unsigned int i3 = inds[i + 2];
            glm::vec3 v1 = verts[i1].Position;
            glm::vec3 v2 = verts[i2].Position;
            glm::vec3 v3 = verts[i3].Position;
            glm::vec3 normal = glm::normalize(glm::cross(v2 - v1, v3 - v1));
            verts[i1].Normal = normal;
            verts[i2].Normal = normal;
            verts[i3].Normal = normal;
        }
    }

    // ------------------------------------------------------------------------------------------
    // [CỤM 7: HÌNH HỌC DỰ PHÒNG TOÁN HỌC TRÁNH CRASH (FALLBACK MESH)]
    // ------------------------------------------------------------------------------------------
    /**
     * @brief Nếu máy giảng viên thiếu file .obj, hàm này tự dựng một chú cá thuôn gọn
     *        với đầy đủ đầu, thân, đuôi và vây lưng bằng các tam giác đối xứng.
     */
    void createFallbackFishMesh() {
        std::vector<Vertex> verts;
        std::vector<unsigned int> inds;

        glm::vec3 nose(0, 0, 15);
        glm::vec3 bodyL(-6, 0, 0), bodyR(6, 0, 0), bodyT(0, 5, 0), bodyB(0, -5, 0);
        glm::vec3 tail(0, 0, -20);
        glm::vec3 finTop(0, 10, -5);

        auto addTri = [&](glm::vec3 p1, glm::vec3 p2, glm::vec3 p3) {
            glm::vec3 n = glm::normalize(glm::cross(p2 - p1, p3 - p1));
            verts.push_back({p1, n, {0.0f, 0.0f}});
            verts.push_back({p2, n, {1.0f, 0.0f}});
            verts.push_back({p3, n, {0.5f, 1.0f}});
        };

        addTri(nose, bodyR, bodyT); addTri(nose, bodyT, bodyL);
        addTri(nose, bodyL, bodyB); addTri(nose, bodyB, bodyR);
        addTri(tail, bodyT, bodyR); addTri(tail, bodyL, bodyT);
        addTri(tail, bodyB, bodyL); addTri(tail, bodyR, bodyB);
        addTri(bodyT, finTop, tail); // Vây lưng

        for (unsigned int i = 0; i < verts.size(); i++) inds.push_back(i);
        meshes.push_back(Mesh(verts, inds));
    }
};
