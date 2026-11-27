#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

/**
 * ==============================================================================================
 * 📌 FILE: include/Shader.h
 * 👤 PHỤ TRÁCH: BẠN B (Frontend / Kỹ sư Shaders & Hiệu Ứng Đồ Họa)
 * 📚 KIẾN THỨC GIÁO TRÌNH ÁNH XẠ:
 *    - CHƯƠNG CG.2: LẬP TRÌNH ĐỒ HỌA TRONG OPENGL (Programming CG in OpenGL)
 *      -> Đọc mã nguồn GLSL từ tệp văn bản (.vs và .fs)
 *      -> Biên dịch Shader (Vertex & Fragment) trên card đồ họa GPU
 *      -> Liên kết Shader Program (Linker)
 *      -> Bắt lỗi cú pháp (Syntax check / Compile Errors)
 *    - TRUYỀN DỮ LIỆU TỪ CPU SANG GPU: Các biến toàn cục Uniforms (Ma trận MVP, ánh sáng, màu sắc)
 * ==============================================================================================
 */

class Shader {
public:
    unsigned int ID; // Định danh ID của Shader Program trên GPU

    // ------------------------------------------------------------------------------------------
    // [CỤM 1: CONSTRUCTOR ĐỌC TỆP, BIÊN DỊCH VÀ LIÊN KẾT SHADER TRÊN GPU]
    // ------------------------------------------------------------------------------------------
    Shader(const char* vertexPath, const char* fragmentPath) {
        // 1. Đọc nội dung tệp mã nguồn Vertex và Fragment Shader từ ổ cứng
        std::string vertexCode;
        std::string fragmentCode;
        std::ifstream vShaderFile;
        std::ifstream fShaderFile;

        vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

        try {
            vShaderFile.open(vertexPath);
            fShaderFile.open(fragmentPath);
            std::stringstream vShaderStream, fShaderStream;
            vShaderStream << vShaderFile.rdbuf();
            fShaderStream << fShaderFile.rdbuf();
            vShaderFile.close();
            fShaderFile.close();
            vertexCode = vShaderStream.str();
            fragmentCode = fShaderStream.str();
        }
        catch (std::ifstream::failure& e) {
            std::cerr << "[ERROR::SHADER] Khong the doc file Shader tai: " 
                      << vertexPath << " hoac " << fragmentPath << std::endl;
        }

        const char* vShaderCode = vertexCode.c_str();
        const char* fShaderCode = fragmentCode.c_str();

        // 2. Biên dịch Vertex Shader trên GPU
        unsigned int vertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex, 1, &vShaderCode, NULL);
        glCompileShader(vertex);
        checkCompileErrors(vertex, "VERTEX"); // Kiểm tra lỗi biên dịch đỉnh

        // 3. Biên dịch Fragment Shader trên GPU
        unsigned int fragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment, 1, &fShaderCode, NULL);
        glCompileShader(fragment);
        checkCompileErrors(fragment, "FRAGMENT"); // Kiểm tra lỗi biên dịch điểm ảnh

        // 4. Tạo Shader Program và liên kết 2 shader thành chương trình hoàn chỉnh
        ID = glCreateProgram();
        glAttachShader(ID, vertex);
        glAttachShader(ID, fragment);
        glLinkProgram(ID);
        checkCompileErrors(ID, "PROGRAM"); // Kiểm tra lỗi liên kết chương trình

        // 5. Sau khi đã liên kết xong vào Program, giải phóng 2 shader trung gian để tiết kiệm VRAM
        glDeleteShader(vertex);
        glDeleteShader(fragment);
    }

    // ------------------------------------------------------------------------------------------
    // [CỤM 2: KÍCH HOẠT SHADER ĐỂ VẼ (USE PROGRAM)]
    // ------------------------------------------------------------------------------------------
    void use() const { 
        glUseProgram(ID); 
    }

    // ------------------------------------------------------------------------------------------
    // [CỤM 3: CÁC HÀM TIỆN ÍCH TRUYỀN BIẾN UNIFORM TỪ C++ SANG GPU]
    // ------------------------------------------------------------------------------------------
    void setBool(const std::string& name, bool value) const {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
    }
    void setInt(const std::string& name, int value) const {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
    }
    void setFloat(const std::string& name, float value) const {
        glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
    }
    void setVec3(const std::string& name, const glm::vec3& value) const {
        glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
    }
    void setVec4(const std::string& name, const glm::vec4& value) const {
        glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
    }
    void setMat4(const std::string& name, const glm::mat4& mat) const {
        glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }

private:
    // ------------------------------------------------------------------------------------------
    // [CỤM 4: BẮT VÀ IN CHI TIẾT LỖI CÚ PHÁP GLSL KHI BIÊN DỊCH TRÊN GPU]
    // ------------------------------------------------------------------------------------------
    void checkCompileErrors(unsigned int shader, std::string type) {
        int success;
        char infoLog[1024];
        if (type != "PROGRAM") {
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if (!success) {
                glGetShaderInfoLog(shader, 1024, NULL, infoLog);
                std::cerr << "[ERROR::SHADER_COMPILATION_ERROR of type: " << type << "]\n" 
                          << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
            }
        } else {
            glGetProgramiv(shader, GL_LINK_STATUS, &success);
            if (!success) {
                glGetProgramInfoLog(shader, 1024, NULL, infoLog);
                std::cerr << "[ERROR::PROGRAM_LINKING_ERROR of type: " << type << "]\n" 
                          << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
            }
        }
    }
};
