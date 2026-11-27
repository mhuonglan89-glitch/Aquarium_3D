#ifndef AQUARIUM_MODEL_H
#define AQUARIUM_MODEL_H

#include <glad/gl.h>
#include <vector>

class Model {
public:
    Model();
    ~Model();

    Model(const Model&) = delete;
    Model& operator=(const Model&) = delete;

    bool loadOBJ(const char* path);
    bool loadGLB(const char* path);
    void draw(GLint tintUniform, GLint textureUniform,
              GLint useTextureUniform, GLuint fallbackTexture,
              float tintR, float tintG, float tintB, float tintA) const;
    void destroy();

private:
    struct Part {
        GLuint vao;
        GLuint vbo;
        GLsizei vertexCount;
        GLuint diffuseTexture;
        float diffuse[3];
        float opacity;

        Part();
    };

    std::vector<Part> parts;
};

#endif
