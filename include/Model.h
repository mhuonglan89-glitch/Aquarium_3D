#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cctype>
#include <cmath>
#include <stb_image.h>
#include "cgltf.h"
#include "Mesh.h"
#include "Shader.h"

/**
 * ==============================================================================================
 * 📌 FILE: include/Model.h
 * 👤 PHỤ TRÁCH: BẠN B (Frontend / Kỹ sư Nạp Mô Hình 3D & Dán Vân Bề Mặt)
 * 📚 KIẾN THỨC GIÁO TRÌNH ÁNH XẠ:
 *    - CHƯƠNG CG.2 & CG.6: MÔ HÌNH HÓA VÀ DÁN VÂN BỀ MẶT (Texture Mapping & 3D Formats)
 *      -> Đọc và phân tích định dạng Wavefront (.obj) và glTF/GLB (.glb) từ Sketchfab
 *      -> Thuật toán Fan Triangulation (chia đa giác n-gon thành tam giác OpenGL)
 *      -> Chuẩn hóa kích thước tự động (Auto-Normalization về [-0.5, 0.5])
 *      -> Tính vector pháp tuyến phẳng (Flat Normals via Cross Product)
 *      -> Nạp ảnh kết cấu Texture (JPG/PNG/Embedded GLB) qua thư viện stb_image.h
 *      -> Dựng Mesh dự phòng hình học (Fallback Procedural Fish Mesh)
 * ==============================================================================================
 */

class Model {
public:
    std::vector<Mesh> meshes;        // Danh sách các Mesh cấu thành mô hình
    glm::vec3 defaultColor;          // Màu cơ sở mặc định của vật thể
    unsigned int textureID = 0;      // ID của Texture chính trên GPU
    bool hasTexture = false;         // Cờ kiểm tra: Vật thể có dùng ảnh Texture ngoài hay không
    glm::vec3 normalizedSize = glm::vec3(1.0f); // Tỉ lệ kích thước sau khi chuẩn hóa
    glm::vec3 alignRotationDeg = glm::vec3(0.0f); // Góc xoay hiệu chỉnh trục đầu (+Z) & lưng (+Y)

    // ------------------------------------------------------------------------------------------
    // [CỤM 1: CONSTRUCTOR TỰ NHẬN DIỆN ĐỊNH DẠNG .OBJ / .GLB VÀ NẠP TEXTURE]
    // ------------------------------------------------------------------------------------------
    Model(const std::string& modelPath, glm::vec3 color = glm::vec3(0.3f, 0.6f, 0.8f),
          const std::string& texPath = "", glm::vec3 alignRotDeg = glm::vec3(0.0f))
        : defaultColor(color), alignRotationDeg(alignRotDeg) {
        if (!texPath.empty()) {
            loadTexture(texPath);
        }

        std::string lowerPath = modelPath;
        std::transform(lowerPath.begin(), lowerPath.end(), lowerPath.begin(),
                       [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

        if (lowerPath.size() >= 4 &&
            (lowerPath.substr(lowerPath.size() - 4) == ".glb" ||
             (lowerPath.size() >= 5 && lowerPath.substr(lowerPath.size() - 5) == ".gltf"))) {
            loadGLTF(modelPath);
        } else {
            loadOBJ(modelPath);
        }
    }

    ~Model() {
        if (hasTexture && textureID != 0) {
            glDeleteTextures(1, &textureID);
        }
    }

    // ------------------------------------------------------------------------------------------
    // [CỤM 2: HÀM VẼ TOÀN BỘ CÁC MESH VỚI ÁNH XẠ VẬT LIỆU]
    // ------------------------------------------------------------------------------------------
    void Draw(Shader& shader) {
        shader.setFloat("alphaVal", 1.0f); // Vật thể đặc có độ đục 100%

        for (unsigned int i = 0; i < meshes.size(); i++) {
            meshes[i].Draw(shader, textureID, hasTexture, defaultColor);
        }

        glBindTexture(GL_TEXTURE_2D, 0);
    }

private:
    struct RawMeshData {
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
        unsigned int textureID = 0;
        bool hasTexture = false;
        glm::vec3 materialColor = glm::vec3(-1.0f);
    };

    // ------------------------------------------------------------------------------------------
    // [CỤM 3: CHUẨN HÓA KÍCH THƯỚC MÔ HÌNH VỀ ĐƠN VỊ CHUẨN (AUTO-NORMALIZATION)]
    // ------------------------------------------------------------------------------------------
    /**
     * @brief Do các model tải từ Sketchfab có tỉ lệ xuất file rất khác nhau (có con rộng 0.06,
     *        có con rộng 300), hàm này tự động dời tâm về gốc (0,0,0) và co dãn chiều dài nhất về 1.0f.
     *        Nhờ vậy, trong main.cpp ta chỉ cần truyền kích thước thực tế mong muốn (vd: 110.0f).
     */
    void finalizeAndNormalizeMeshes(std::vector<RawMeshData>& rawMeshes, const std::string& path) {
        if (rawMeshes.empty()) {
            createFallbackFishMesh();
            return;
        }

        glm::mat4 alignMat = glm::mat4(1.0f);
        alignMat = glm::rotate(alignMat, glm::radians(alignRotationDeg.y), glm::vec3(0.0f, 1.0f, 0.0f));
        alignMat = glm::rotate(alignMat, glm::radians(alignRotationDeg.x), glm::vec3(1.0f, 0.0f, 0.0f));
        alignMat = glm::rotate(alignMat, glm::radians(alignRotationDeg.z), glm::vec3(0.0f, 0.0f, 1.0f));
        glm::mat3 alignNormMat = glm::mat3(alignMat);

        for (auto& rm : rawMeshes) {
            for (auto& v : rm.vertices) {
                v.Position = glm::vec3(alignMat * glm::vec4(v.Position, 1.0f));
                v.Normal = glm::normalize(alignNormMat * v.Normal);
            }
        }

        glm::vec3 minPt(1e9f), maxPt(-1e9f);
        size_t totalVerts = 0, totalTris = 0;

        for (const auto& rm : rawMeshes) {
            for (const auto& v : rm.vertices) {
                minPt = glm::min(minPt, v.Position);
                maxPt = glm::max(maxPt, v.Position);
            }
            totalVerts += rm.vertices.size();
            totalTris += rm.indices.size() / 3;
        }

        if (totalVerts == 0) {
            createFallbackFishMesh();
            return;
        }

        glm::vec3 center = (minPt + maxPt) * 0.5f;
        glm::vec3 extent = maxPt - minPt;
        float maxExtent = std::max(extent.x, std::max(extent.y, extent.z));
        if (maxExtent < 1e-6f) maxExtent = 1.0f;

        normalizedSize = extent / maxExtent;

        for (auto& rm : rawMeshes) {
            for (auto& v : rm.vertices) {
                v.Position = (v.Position - center) / maxExtent;
            }
            meshes.push_back(Mesh(std::move(rm.vertices), std::move(rm.indices),
                                  rm.textureID, rm.hasTexture, rm.materialColor));
        }

        std::cout << "[INFO::MODEL] Da nap thanh cong: " << path
                  << " (" << meshes.size() << " meshes, " << totalVerts << " dinh, "
                  << totalTris << " tam giac)" << std::endl;
    }

    // ------------------------------------------------------------------------------------------
    // [CỤM 4: BỘ PHÂN TÍCH MÔ HÌNH GLTF / GLB TỪ SKETCHFAB (SỬ DỤNG CGLTF)]
    // ------------------------------------------------------------------------------------------
    void loadGLTF(const std::string& path) {
        cgltf_options options = {};
        cgltf_data* data = nullptr;

        cgltf_result res = cgltf_parse_file(&options, path.c_str(), &data);
        if (res != cgltf_result_success) {
            std::cerr << "[WARN::MODEL] Khong the doc file GLB/GLTF tai: " << path << std::endl;
            createFallbackFishMesh();
            return;
        }

        if (cgltf_load_buffers(&options, data, path.c_str()) != cgltf_result_success) {
            std::cerr << "[WARN::MODEL] Khong the nap buffer GLB/GLTF tai: " << path << std::endl;
            cgltf_free(data);
            createFallbackFishMesh();
            return;
        }

        std::vector<RawMeshData> rawMeshes;
        std::string baseDir = "";
        size_t slashPos = path.find_last_of("/\\");
        if (slashPos != std::string::npos) baseDir = path.substr(0, slashPos + 1);

        // Duyệt tất cả các Node trong Scene để áp dụng đúng ma trận biến đổi cục bộ của Sketchfab
        for (cgltf_size ni = 0; ni < data->nodes_count; ++ni) {
            const cgltf_node* node = &data->nodes[ni];
            if (!node->mesh) continue;

            cgltf_float worldMatArr[16];
            cgltf_node_transform_world(node, worldMatArr);
            glm::mat4 nodeMat = glm::make_mat4(worldMatArr);
            glm::mat3 normalMat = glm::mat3(glm::transpose(glm::inverse(nodeMat)));

            const cgltf_mesh* mesh = node->mesh;
            for (cgltf_size pi = 0; pi < mesh->primitives_count; ++pi) {
                const cgltf_primitive* prim = &mesh->primitives[pi];
                if (prim->type != cgltf_primitive_type_triangles) continue;

                const cgltf_accessor* posAcc = nullptr;
                const cgltf_accessor* normAcc = nullptr;
                const cgltf_accessor* uvAcc = nullptr;

                for (cgltf_size ai = 0; ai < prim->attributes_count; ++ai) {
                    const cgltf_attribute& attr = prim->attributes[ai];
                    if (attr.type == cgltf_attribute_type_position) posAcc = attr.data;
                    else if (attr.type == cgltf_attribute_type_normal) normAcc = attr.data;
                    else if (attr.type == cgltf_attribute_type_texcoord && attr.index == 0) uvAcc = attr.data;
                }

                if (!posAcc || posAcc->count == 0) continue;

                RawMeshData rm;
                rm.vertices.resize(posAcc->count);

                for (cgltf_size vi = 0; vi < posAcc->count; ++vi) {
                    float p[3] = {0.0f, 0.0f, 0.0f};
                    cgltf_accessor_read_float(posAcc, vi, p, 3);
                    glm::vec4 worldPos = nodeMat * glm::vec4(p[0], p[1], p[2], 1.0f);
                    rm.vertices[vi].Position = glm::vec3(worldPos);

                    if (normAcc) {
                        float n[3] = {0.0f, 1.0f, 0.0f};
                        cgltf_accessor_read_float(normAcc, vi, n, 3);
                        glm::vec3 wn = normalMat * glm::vec3(n[0], n[1], n[2]);
                        rm.vertices[vi].Normal = (glm::length(wn) > 1e-6f) ? glm::normalize(wn) : glm::vec3(0.0f, 1.0f, 0.0f);
                    } else {
                        rm.vertices[vi].Normal = glm::vec3(0.0f, 1.0f, 0.0f);
                    }

                    if (uvAcc) {
                        float uv[2] = {0.0f, 0.0f};
                        cgltf_accessor_read_float(uvAcc, vi, uv, 2);
                        rm.vertices[vi].TexCoords = glm::vec2(uv[0], uv[1]);
                    } else {
                        rm.vertices[vi].TexCoords = glm::vec2(0.0f, 0.0f);
                    }
                }

                if (prim->indices) {
                    rm.indices.resize(prim->indices->count);
                    for (cgltf_size ii = 0; ii < prim->indices->count; ++ii) {
                        rm.indices[ii] = static_cast<unsigned int>(cgltf_accessor_read_index(prim->indices, ii));
                    }
                } else {
                    rm.indices.resize(posAcc->count);
                    for (cgltf_size ii = 0; ii < posAcc->count; ++ii) {
                        rm.indices[ii] = static_cast<unsigned int>(ii);
                    }
                }

                if (!normAcc) {
                    computeFlatNormals(rm.vertices, rm.indices);
                }

                // Nạp vật liệu & Texture nhúng bên trong file GLB
                if (prim->material) {
                    const cgltf_material* mat = prim->material;
                    if (mat->has_pbr_metallic_roughness) {
                        const auto& pbr = mat->pbr_metallic_roughness;
                        rm.materialColor = glm::vec3(pbr.base_color_factor[0],
                                                     pbr.base_color_factor[1],
                                                     pbr.base_color_factor[2]);

                        if (pbr.base_color_texture.texture && pbr.base_color_texture.texture->image) {
                            const cgltf_image* img = pbr.base_color_texture.texture->image;
                            if (img->buffer_view && img->buffer_view->buffer && img->buffer_view->buffer->data) {
                                const unsigned char* rawBytes =
                                    static_cast<const unsigned char*>(img->buffer_view->buffer->data) + img->buffer_view->offset;
                                rm.textureID = loadTextureFromMemory(rawBytes, static_cast<int>(img->buffer_view->size));
                                rm.hasTexture = (rm.textureID != 0);
                            } else if (img->uri && !std::string(img->uri).empty()) {
                                rm.textureID = loadTextureFromFile(baseDir + img->uri, false);
                                rm.hasTexture = (rm.textureID != 0);
                            }
                        }
                    }
                }

                rawMeshes.push_back(std::move(rm));
            }
        }

        cgltf_free(data);
        finalizeAndNormalizeMeshes(rawMeshes, path);
    }

    // ------------------------------------------------------------------------------------------
    // [CỤM 5: BỘ PHÂN TÍCH ĐỊNH DẠNG TỆP WAVEFRONT OBJ TỰ XÂY DỰNG]
    // ------------------------------------------------------------------------------------------
    void loadOBJ(const std::string& path) {
        std::vector<glm::vec3> temp_positions;
        std::vector<glm::vec3> temp_normals;
        std::vector<glm::vec2> temp_texcoords;
        RawMeshData rm;

        std::ifstream file(path);
        if (!file.is_open()) {
            std::cerr << "[WARN::MODEL] Khong the mo file .obj tai: " << path << ". Khoi tao Mesh mac dinh." << std::endl;
            createFallbackFishMesh();
            return;
        }

        std::string line;
        while (std::getline(file, line)) {
            if (line.size() < 2) continue;
            if (line.substr(0, 2) == "v ") {
                std::istringstream s(line.substr(2));
                glm::vec3 v;
                s >> v.x >> v.y >> v.z;
                temp_positions.push_back(v);
            } else if (line.size() >= 3 && line.substr(0, 3) == "vn ") {
                std::istringstream s(line.substr(3));
                glm::vec3 vn;
                s >> vn.x >> vn.y >> vn.z;
                temp_normals.push_back(vn);
            } else if (line.size() >= 3 && line.substr(0, 3) == "vt ") {
                std::istringstream s(line.substr(3));
                glm::vec2 vt;
                s >> vt.x >> vt.y;
                temp_texcoords.push_back(vt);
            } else if (line.substr(0, 2) == "f ") {
                std::istringstream s(line.substr(2));
                std::string vString;
                std::vector<unsigned int> faceIndices;

                while (s >> vString) {
                    unsigned int vIdx = 0, vtIdx = 0, vnIdx = 0;
                    std::replace(vString.begin(), vString.end(), '/', ' ');
                    std::istringstream vStream(vString);

                    vStream >> vIdx;
                    if (vStream >> vtIdx) {
                        vStream >> vnIdx;
                    }

                    Vertex vertex;
                    if (vIdx > 0 && vIdx <= temp_positions.size())
                        vertex.Position = temp_positions[vIdx - 1];
                    else
                        vertex.Position = glm::vec3(0.0f);

                    if (vnIdx > 0 && vnIdx <= temp_normals.size())
                        vertex.Normal = temp_normals[vnIdx - 1];
                    else
                        vertex.Normal = glm::vec3(0.0f, 1.0f, 0.0f);

                    if (vtIdx > 0 && vtIdx <= temp_texcoords.size())
                        vertex.TexCoords = temp_texcoords[vtIdx - 1];
                    else
                        vertex.TexCoords = glm::vec2(vertex.Position.x * 0.05f, vertex.Position.z * 0.05f);

                    rm.vertices.push_back(vertex);
                    faceIndices.push_back(static_cast<unsigned int>(rm.vertices.size() - 1));
                }

                // Fan Triangulation
                for (size_t k = 1; k + 1 < faceIndices.size(); ++k) {
                    rm.indices.push_back(faceIndices[0]);
                    rm.indices.push_back(faceIndices[k]);
                    rm.indices.push_back(faceIndices[k + 1]);
                }
            }
        }
        file.close();

        if (!rm.vertices.empty() && temp_normals.empty()) {
            computeFlatNormals(rm.vertices, rm.indices);
        }

        std::vector<RawMeshData> rawMeshes;
        if (!rm.vertices.empty()) rawMeshes.push_back(std::move(rm));
        finalizeAndNormalizeMeshes(rawMeshes, path);
    }

    // ------------------------------------------------------------------------------------------
    // [CỤM 6: NẠP TEXTURE TỪ FILE HOẶC BỘ NHỚ RAM (GLB EMBEDDED)]
    // ------------------------------------------------------------------------------------------
    unsigned int uploadTextureData(unsigned char* data, int width, int height, int nrChannels) {
        if (!data) return 0;
        GLenum format = GL_RGB;
        if (nrChannels == 1) format = GL_RED;
        else if (nrChannels == 3) format = GL_RGB;
        else if (nrChannels == 4) format = GL_RGBA;

        unsigned int tex = 0;
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        return tex;
    }

    unsigned int loadTextureFromFile(const std::string& path, bool flipY = true) {
        int width = 0, height = 0, nrChannels = 0;
        stbi_set_flip_vertically_on_load(flipY ? 1 : 0);
        unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);
        if (data) {
            unsigned int tex = uploadTextureData(data, width, height, nrChannels);
            stbi_image_free(data);
            std::cout << "[INFO::TEXTURE] Da nap texture: " << path
                      << " (" << width << "x" << height << ", " << nrChannels << " channels)" << std::endl;
            return tex;
        }
        std::cerr << "[WARN::TEXTURE] Khong the doc file texture tai: " << path << std::endl;
        return 0;
    }

    unsigned int loadTextureFromMemory(const unsigned char* bytes, int byteLen) {
        int width = 0, height = 0, nrChannels = 0;
        stbi_set_flip_vertically_on_load(0); // GLTF UV gốc từ trên xuống dưới
        unsigned char* data = stbi_load_from_memory(bytes, byteLen, &width, &height, &nrChannels, 0);
        if (data) {
            unsigned int tex = uploadTextureData(data, width, height, nrChannels);
            stbi_image_free(data);
            return tex;
        }
        return 0;
    }

    void loadTexture(const std::string& path) {
        textureID = loadTextureFromFile(path, true);
        hasTexture = (textureID != 0);
    }

    // ------------------------------------------------------------------------------------------
    // [CỤM 7: THUẬT TOÁN TÍNH VECTOR PHÁP TUYẾN AN TOÀN (TRÁNH LỖI CHIA 0)]
    // ------------------------------------------------------------------------------------------
    void computeFlatNormals(std::vector<Vertex>& verts, const std::vector<unsigned int>& inds) {
        for (size_t i = 0; i + 2 < inds.size(); i += 3) {
            unsigned int i1 = inds[i];
            unsigned int i2 = inds[i + 1];
            unsigned int i3 = inds[i + 2];
            glm::vec3 v1 = verts[i1].Position;
            glm::vec3 v2 = verts[i2].Position;
            glm::vec3 v3 = verts[i3].Position;
            glm::vec3 c = glm::cross(v2 - v1, v3 - v1);
            glm::vec3 normal = (glm::length(c) > 1e-6f) ? glm::normalize(c) : glm::vec3(0.0f, 1.0f, 0.0f);
            verts[i1].Normal = normal;
            verts[i2].Normal = normal;
            verts[i3].Normal = normal;
        }
    }

    // ------------------------------------------------------------------------------------------
    // [CỤM 8: HÌNH HỌC DỰ PHÒNG TOÁN HỌC TRÁNH CRASH (FALLBACK MESH)]
    // ------------------------------------------------------------------------------------------
    void createFallbackFishMesh() {
        std::vector<Vertex> verts;
        std::vector<unsigned int> inds;

        glm::vec3 nose(0, 0, 0.5f);
        glm::vec3 bodyL(-0.18f, 0, 0), bodyR(0.18f, 0, 0), bodyT(0, 0.16f, 0), bodyB(0, -0.16f, 0);
        glm::vec3 tail(0, 0, -0.5f);
        glm::vec3 finTop(0, 0.28f, -0.15f);

        auto addTri = [&](glm::vec3 p1, glm::vec3 p2, glm::vec3 p3) {
            glm::vec3 c = glm::cross(p2 - p1, p3 - p1);
            glm::vec3 n = (glm::length(c) > 1e-6f) ? glm::normalize(c) : glm::vec3(0.0f, 1.0f, 0.0f);
            verts.push_back({p1, n, {0.0f, 0.0f}});
            verts.push_back({p2, n, {1.0f, 0.0f}});
            verts.push_back({p3, n, {0.5f, 1.0f}});
        };

        addTri(nose, bodyR, bodyT); addTri(nose, bodyT, bodyL);
        addTri(nose, bodyL, bodyB); addTri(nose, bodyB, bodyR);
        addTri(tail, bodyT, bodyR); addTri(tail, bodyL, bodyT);
        addTri(tail, bodyB, bodyL); addTri(tail, bodyR, bodyB);
        addTri(bodyT, finTop, tail);

        for (unsigned int i = 0; i < verts.size(); i++) inds.push_back(i);
        meshes.push_back(Mesh(verts, inds));
    }
};
