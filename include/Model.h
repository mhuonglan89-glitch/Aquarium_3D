#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include "Mesh.h"
#include "Shader.h"

/**
 * @file Model.h
 * @brief Đọc và nạp các mô hình 3D thực tế định dạng .obj (Cá mập, cá đuối, san hô)
 * @author Bạn B (Frontend / Assets & Models) - Môn Đồ Họa Máy Tính (Chương CG.6)
 */
class Model {
public:
    std::vector<Mesh> meshes;
    glm::vec3 defaultColor;

    Model(const std::string& path, glm::vec3 color = glm::vec3(0.3f, 0.6f, 0.8f)) {
        defaultColor = color;
        loadOBJ(path);
    }

    void Draw(Shader& shader) {
        shader.setVec3("objectColor", defaultColor);
        shader.setBool("useTexture", false);
        shader.setFloat("alphaVal", 1.0f);
        for (unsigned int i = 0; i < meshes.size(); i++) {
            meshes[i].Draw(shader);
        }
    }

private:
    void loadOBJ(const std::string& path) {
        std::vector<glm::vec3> temp_positions;
        std::vector<glm::vec3> temp_normals;
        std::vector<glm::vec2> temp_texcoords;
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;

        std::ifstream file(path);
        if (!file.is_open()) {
            std::cerr << "[ERROR::MODEL] Khong the mo file .obj tai: " << path << std::endl;
            // Tạo mesh dự phòng hình thoi nếu không có file .obj
            createFallbackFishMesh();
            return;
        }

        std::string line;
        while (std::getline(file, line)) {
            if (line.substr(0, 2) == "v ") {
                std::istringstream s(line.substr(2));
                glm::vec3 v;
                s >> v.x >> v.y >> v.z;
                temp_positions.push_back(v);
            }
            else if (line.substr(0, 3) == "vn ") {
                std::istringstream s(line.substr(3));
                glm::vec3 vn;
                s >> vn.x >> vn.y >> vn.z;
                temp_normals.push_back(vn);
            }
            else if (line.substr(0, 3) == "vt ") {
                std::istringstream s(line.substr(3));
                glm::vec2 vt;
                s >> vt.x >> vt.y;
                temp_texcoords.push_back(vt);
            }
            else if (line.substr(0, 2) == "f ") {
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
                    if (vnIdx > 0 && vnIdx <= temp_normals.size())
                        vertex.Normal = temp_normals[vnIdx - 1];
                    else
                        vertex.Normal = glm::vec3(0.0f, 1.0f, 0.0f);
                    if (vtIdx > 0 && vtIdx <= temp_texcoords.size())
                        vertex.TexCoords = temp_texcoords[vtIdx - 1];
                    else
                        vertex.TexCoords = glm::vec2(0.0f, 0.0f);

                    vertices.push_back(vertex);
                    faceIndices.push_back(vertices.size() - 1);
                }

                // Triangulate n-gon (Fan triangulation)
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
            meshes.push_back(Mesh(vertices, indices));
            std::cout << "[INFO::MODEL] Da nap thanh cong: " << path 
                      << " (" << vertices.size() << " dinh)" << std::endl;
        }
    }

    // Mesh dự phòng hình cá mập thuôn gọn nếu máy chưa có file .obj
    void createFallbackFishMesh() {
        std::vector<Vertex> verts;
        std::vector<unsigned int> inds;

        // Điểm đầu mũi cá, thân, đuôi, vây lưng
        glm::vec3 nose(0, 0, 15);
        glm::vec3 bodyL(-6, 0, 0), bodyR(6, 0, 0), bodyT(0, 5, 0), bodyB(0, -5, 0);
        glm::vec3 tail(0, 0, -20);
        glm::vec3 finTop(0, 10, -5);

        // Tạo khung hình học cơ bản
        auto addTri = [&](glm::vec3 p1, glm::vec3 p2, glm::vec3 p3) {
            glm::vec3 n = glm::normalize(glm::cross(p2 - p1, p3 - p1));
            verts.push_back({p1, n, {0,0}});
            verts.push_back({p2, n, {1,0}});
            verts.push_back({p3, n, {0.5,1}});
        };

        addTri(nose, bodyR, bodyT);
        addTri(nose, bodyT, bodyL);
        addTri(nose, bodyL, bodyB);
        addTri(nose, bodyB, bodyR);

        addTri(tail, bodyT, bodyR);
        addTri(tail, bodyL, bodyT);
        addTri(tail, bodyB, bodyL);
        addTri(tail, bodyR, bodyB);

        // Vây lưng
        addTri(bodyT, finTop, tail);

        for (unsigned int i = 0; i < verts.size(); i++) inds.push_back(i);
        meshes.push_back(Mesh(verts, inds));
    }
};
