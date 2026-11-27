#ifndef AQUARIUM_MESH_H
#define AQUARIUM_MESH_H

#include <glad/gl.h>
#include <vector>

struct Vertex {
    float position[3];
    float normal[3];
    float texCoord[2];
};

struct Mesh {
    GLuint vao;
    GLuint vbo;
    GLsizei vertexCount;

    Mesh() : vao(0), vbo(0), vertexCount(0) {}

    void upload(const std::vector<Vertex>& vertices) {
        vertexCount = (GLsizei)vertices.size();
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER,
                     (GLsizeiptr)(vertices.size() * sizeof(Vertex)),
                     vertices.empty() ? NULL : &vertices[0],
                     GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                              (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                              (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glBindVertexArray(0);
    }

    void destroy() {
        if (vbo != 0) glDeleteBuffers(1, &vbo);
        if (vao != 0) glDeleteVertexArrays(1, &vao);
        vao = 0;
        vbo = 0;
        vertexCount = 0;
    }
};

#endif
