#include "Shader.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

static std::string readShaderFile(const char* path) {
    std::ifstream file(path);
    if (!file) {
        file.clear();
        std::string fallback = std::string("D:/Aquarium_OpenGL/") + path;
        file.open(fallback.c_str());
    }
    if (!file) {
        std::cerr << "Cannot open shader file: " << path << std::endl;
        return std::string();
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

static GLuint compileShader(GLenum type, const std::string& source,
                            const char* label) {
    const char* sourceText = source.c_str();
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &sourceText, NULL);
    glCompileShader(shader);
    GLint success = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (success == GL_FALSE) {
        char log[2048] = {0};
        glGetShaderInfoLog(shader, sizeof(log), NULL, log);
        std::cerr << label << " shader compile error:" << std::endl
                  << log << std::endl;
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

Shader::Shader() : programId(0) {}

Shader::~Shader() {
    destroy();
}

bool Shader::load(const char* vertexPath, const char* fragmentPath) {
    destroy();
    std::string vertexSource = readShaderFile(vertexPath);
    std::string fragmentSource = readShaderFile(fragmentPath);
    if (vertexSource.empty() || fragmentSource.empty()) return false;

    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexSource, "Vertex");
    if (vertexShader == 0) return false;
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentSource, "Fragment");
    if (fragmentShader == 0) {
        glDeleteShader(vertexShader);
        return false;
    }

    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    GLint success = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (success == GL_FALSE) {
        char log[2048] = {0};
        glGetProgramInfoLog(program, sizeof(log), NULL, log);
        std::cerr << "Shader program link error:" << std::endl << log << std::endl;
        glDeleteProgram(program);
        program = 0;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    programId = program;
    return programId != 0;
}

void Shader::use() const {
    glUseProgram(programId);
}

GLuint Shader::id() const {
    return programId;
}

void Shader::destroy() {
    if (programId != 0) glDeleteProgram(programId);
    programId = 0;
}
