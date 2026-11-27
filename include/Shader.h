#ifndef AQUARIUM_SHADER_H
#define AQUARIUM_SHADER_H

#include <glad/gl.h>

class Shader {
public:
    Shader();
    ~Shader();

    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;

    bool load(const char* vertexPath, const char* fragmentPath);
    void use() const;
    GLuint id() const;
    void destroy();

private:
    GLuint programId;
};

#endif
