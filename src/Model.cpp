#define TINYOBJLOADER_DISABLE_FAST_FLOAT
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#include "Model.h"
#include "cgltf.h"
#include "stb_image.h"

#include <algorithm>
#include <climits>
#include <cmath>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <cstring>
#include <vector>

struct ModelVec2 { float x; float y; };
struct ModelVec3 { float x; float y; float z; };
struct ModelVertex { float position[3]; float normal[3]; float texCoord[2]; float color[4]; };
struct ModelIndex { int position; int texCoord; int normal; };
struct ModelTriangle { ModelIndex corners[3]; };

static ModelVec3 subtractModel(const ModelVec3& a, const ModelVec3& b) {
    ModelVec3 result = {a.x-b.x, a.y-b.y, a.z-b.z};
    return result;
}

static ModelVec3 crossModel(const ModelVec3& a, const ModelVec3& b) {
    ModelVec3 result = {a.y*b.z-a.z*b.y, a.z*b.x-a.x*b.z, a.x*b.y-a.y*b.x};
    return result;
}

static ModelVec3 normalizeModel(const ModelVec3& value) {
    float length = std::sqrt(value.x*value.x + value.y*value.y + value.z*value.z);
    if (length < 0.00001f) {
        ModelVec3 up = {0.0f, 1.0f, 0.0f};
        return up;
    }
    ModelVec3 result = {value.x/length, value.y/length, value.z/length};
    return result;
}

static std::string directoryOf(const std::string& path) {
    std::string::size_type slash = path.find_last_of("/\\");
    if (slash == std::string::npos) return std::string();
    return path.substr(0, slash);
}

static std::string joinPath(const std::string& base, const std::string& child) {
    if (base.empty() || (!child.empty() && (child[0]=='/' || child[0]=='\\')) ||
        (child.size()>1 && child[1]==':')) return child;
    return base + "/" + child;
}

static GLuint loadMaterialTexture(const std::string& path) {
    if (path.empty()) return 0;
    int width = 0;
    int height = 0;
    int channels = 0;
    stbi_set_flip_vertically_on_load(1);
    unsigned char* pixels = stbi_load(path.c_str(), &width, &height, &channels, 4);
    if (pixels == NULL) {
        std::string fallback = std::string("D:/Aquarium_OpenGL/") + path;
        pixels = stbi_load(fallback.c_str(), &width, &height, &channels, 4);
    }
    if (pixels == NULL) {
        std::string filename = path;
        std::string::size_type pos = filename.find_last_of("/\\");
        if (pos != std::string::npos) filename = filename.substr(pos + 1);
        std::string alt1 = "assets/textures/" + filename;
        pixels = stbi_load(alt1.c_str(), &width, &height, &channels, 4);
        if (pixels == NULL) {
            std::string alt2 = "D:/Aquarium_OpenGL/assets/textures/" + filename;
            pixels = stbi_load(alt2.c_str(), &width, &height, &channels, 4);
        }
    }
    if (pixels == NULL) {
        std::cerr << "Cannot load MTL texture " << path << ": "
                  << stbi_failure_reason() << std::endl;
        return 0;
    }

    GLuint texture = 0;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
    stbi_image_free(pixels);
    return texture;
}

Model::Part::Part()
    : vao(0), vbo(0), vertexCount(0), diffuseTexture(0), opacity(1.0f) {
    diffuse[0] = 1.0f;
    diffuse[1] = 1.0f;
    diffuse[2] = 1.0f;
}

Model::Model() : parts() {}

Model::~Model() {
    destroy();
}

bool Model::loadOBJ(const char* path) {
    destroy();

    std::string objPath(path ? path : "");
    std::ifstream probe(objPath.c_str(), std::ios::binary);
    if (!probe.good()) {
        objPath = std::string("D:/Aquarium_OpenGL/") + objPath;
    } else {
        probe.close();
    }

    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string objError;
    std::string materialDirectory = directoryOf(objPath);
    if (!materialDirectory.empty()) materialDirectory += "/";

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &objError,
                          objPath.c_str(), materialDirectory.c_str(), true)) {
        std::cerr << "TinyObjLoader could not read " << objPath << std::endl;
        if (!objError.empty()) std::cerr << objError << std::endl;
        return false;
    }
    if (!objError.empty()) std::cerr << "OBJ warning: " << objError << std::endl;

    const size_t positionCount = attrib.vertices.size() / 3;
    if (positionCount == 0) return false;

    std::map<int, std::vector<ModelTriangle> > trianglesByMaterial;
    std::vector<ModelVec3> generatedNormals(positionCount);
    size_t shapeIndex;
    for (shapeIndex = 0; shapeIndex < shapes.size(); ++shapeIndex) {
        const tinyobj::mesh_t& mesh = shapes[shapeIndex].mesh;
        size_t indexOffset = 0;
        size_t faceIndex;
        for (faceIndex = 0; faceIndex < mesh.num_face_vertices.size(); ++faceIndex) {
            size_t faceVertexCount = (size_t)mesh.num_face_vertices[faceIndex];
            int materialId = faceIndex < mesh.material_ids.size()
                           ? mesh.material_ids[faceIndex] : -1;
            if (faceVertexCount >= 3 && indexOffset + faceVertexCount <= mesh.indices.size()) {
                size_t fan;
                for (fan = 1; fan + 1 < faceVertexCount; ++fan) {
                    const tinyobj::index_t source[3] = {
                        mesh.indices[indexOffset],
                        mesh.indices[indexOffset + fan],
                        mesh.indices[indexOffset + fan + 1]
                    };
                    ModelTriangle triangle;
                    bool valid = true;
                    ModelVec3 points[3];
                    int corner;
                    for (corner = 0; corner < 3; ++corner) {
                        if (source[corner].vertex_index < 0 ||
                            (size_t)source[corner].vertex_index >= positionCount) {
                            valid = false;
                            break;
                        }
                        triangle.corners[corner].position = source[corner].vertex_index;
                        triangle.corners[corner].texCoord = source[corner].texcoord_index;
                        triangle.corners[corner].normal = source[corner].normal_index;
                        size_t base = (size_t)source[corner].vertex_index * 3;
                        points[corner].x = (float)attrib.vertices[base];
                        points[corner].y = (float)attrib.vertices[base + 1];
                        points[corner].z = (float)attrib.vertices[base + 2];
                    }
                    if (!valid) continue;

                    ModelVec3 faceNormal = crossModel(
                        subtractModel(points[1], points[0]),
                        subtractModel(points[2], points[0]));
                    for (corner = 0; corner < 3; ++corner) {
                        size_t p = (size_t)triangle.corners[corner].position;
                        generatedNormals[p].x += faceNormal.x;
                        generatedNormals[p].y += faceNormal.y;
                        generatedNormals[p].z += faceNormal.z;
                    }
                    trianglesByMaterial[materialId].push_back(triangle);
                }
            }
            indexOffset += faceVertexCount;
        }
    }
    if (trianglesByMaterial.empty()) return false;

    ModelVec3 minimumPosition = {
        (float)attrib.vertices[0], (float)attrib.vertices[1], (float)attrib.vertices[2]
    };
    ModelVec3 maximumPosition = minimumPosition;
    size_t p;
    for (p = 1; p < positionCount; ++p) {
        size_t base = p * 3;
        minimumPosition.x = std::min(minimumPosition.x, (float)attrib.vertices[base]);
        minimumPosition.y = std::min(minimumPosition.y, (float)attrib.vertices[base + 1]);
        minimumPosition.z = std::min(minimumPosition.z, (float)attrib.vertices[base + 2]);
        maximumPosition.x = std::max(maximumPosition.x, (float)attrib.vertices[base]);
        maximumPosition.y = std::max(maximumPosition.y, (float)attrib.vertices[base + 1]);
        maximumPosition.z = std::max(maximumPosition.z, (float)attrib.vertices[base + 2]);
    }
    float rangeX = maximumPosition.x - minimumPosition.x;
    float rangeY = maximumPosition.y - minimumPosition.y;
    if (rangeX < 0.0001f) rangeX = 1.0f;
    if (rangeY < 0.0001f) rangeY = 1.0f;

    std::map<int, std::vector<ModelTriangle> >::const_iterator group;
    for (group = trianglesByMaterial.begin(); group != trianglesByMaterial.end(); ++group) {
        const int materialId = group->first;
        const std::vector<ModelTriangle>& triangles = group->second;
        std::vector<ModelVertex> vertices;
        vertices.reserve(triangles.size() * 3);
        size_t triangleIndex;
        for (triangleIndex = 0; triangleIndex < triangles.size(); ++triangleIndex) {
            int corner;
            for (corner = 0; corner < 3; ++corner) {
                const ModelIndex& index = triangles[triangleIndex].corners[corner];
                size_t base = (size_t)index.position * 3;
                ModelVertex vertex;
                vertex.position[0] = (float)attrib.vertices[base];
                vertex.position[1] = (float)attrib.vertices[base + 1];
                vertex.position[2] = (float)attrib.vertices[base + 2];

                ModelVec3 normal = normalizeModel(generatedNormals[(size_t)index.position]);
                if (index.normal >= 0 && (size_t)(index.normal + 1) * 3 <= attrib.normals.size()) {
                    size_t n = (size_t)index.normal * 3;
                    ModelVec3 supplied = {
                        (float)attrib.normals[n],
                        (float)attrib.normals[n + 1],
                        (float)attrib.normals[n + 2]
                    };
                    normal = normalizeModel(supplied);
                }
                vertex.normal[0] = normal.x;
                vertex.normal[1] = normal.y;
                vertex.normal[2] = normal.z;

                if (index.texCoord >= 0 && (size_t)(index.texCoord + 1) * 2 <= attrib.texcoords.size()) {
                    size_t uv = (size_t)index.texCoord * 2;
                    vertex.texCoord[0] = (float)attrib.texcoords[uv];
                    vertex.texCoord[1] = (float)attrib.texcoords[uv + 1];
                } else {
                    vertex.texCoord[0] = (vertex.position[0] - minimumPosition.x) / rangeX;
                    vertex.texCoord[1] = (vertex.position[1] - minimumPosition.y) / rangeY;
                }
                vertex.color[0] = 1.0f;
                vertex.color[1] = 1.0f;
                vertex.color[2] = 1.0f;
                vertex.color[3] = 1.0f;
                vertices.push_back(vertex);
            }
        }

        if (vertices.empty()) continue;
        Part part;
        part.vertexCount = (GLsizei)vertices.size();
        if (materialId >= 0 && (size_t)materialId < materials.size()) {
            const tinyobj::material_t& material = materials[(size_t)materialId];
            part.diffuse[0] = (float)material.diffuse[0];
            part.diffuse[1] = (float)material.diffuse[1];
            part.diffuse[2] = (float)material.diffuse[2];
            part.opacity = std::max(0.0f, std::min(1.0f, (float)material.dissolve));
            if (!material.diffuse_texname.empty()) {
                std::string texturePath = joinPath(directoryOf(objPath), material.diffuse_texname);
                part.diffuseTexture = loadMaterialTexture(texturePath);
            }
        }

        glGenVertexArrays(1, &part.vao);
        glGenBuffers(1, &part.vbo);
        glBindVertexArray(part.vao);
        glBindBuffer(GL_ARRAY_BUFFER, part.vbo);
        glBufferData(GL_ARRAY_BUFFER,
                     (GLsizeiptr)(vertices.size() * sizeof(ModelVertex)),
                     &vertices[0], GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(ModelVertex), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(ModelVertex),
                              (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(ModelVertex),
                              (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(ModelVertex),
                              (void*)(8 * sizeof(float)));
        glEnableVertexAttribArray(3);
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        parts.push_back(part);
    }

    return !parts.empty();
}

static GLuint uploadGlbImage(const unsigned char* encoded, size_t encodedSize) {
    if (encoded == NULL || encodedSize == 0 || encodedSize > (size_t)INT_MAX) return 0;
    int width = 0;
    int height = 0;
    int channels = 0;
    // glTF texture coordinates use the image's top-left origin; keep stb's
    // decoded row order so UVs line up with standard GLB assets.
    stbi_set_flip_vertically_on_load(0);
    unsigned char* pixels = stbi_load_from_memory(encoded, (int)encodedSize,
                                                   &width, &height, &channels, 4);
    if (pixels == NULL) {
        std::cerr << "Cannot decode embedded GLB texture: "
                  << stbi_failure_reason() << std::endl;
        return 0;
    }

    GLuint texture = 0;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    stbi_image_free(pixels);
    return texture;
}

static GLuint loadGlbTexture(const cgltf_image* image, const std::string& modelDirectory) {
    if (image == NULL) return 0;
    if (image->buffer_view != NULL) {
        const uint8_t* bytes = cgltf_buffer_view_data(image->buffer_view);
        return uploadGlbImage(bytes, (size_t)image->buffer_view->size);
    }
    if (image->uri != NULL && image->uri[0] != '\0' &&
        std::strncmp(image->uri, "data:", 5) != 0) {
        return loadMaterialTexture(joinPath(modelDirectory, image->uri));
    }
    return 0;
}

static ModelVec3 transformGlbPoint(const cgltf_float* m, const ModelVec3& p) {
    ModelVec3 result;
    result.x = m[0] * p.x + m[4] * p.y + m[8] * p.z + m[12];
    result.y = m[1] * p.x + m[5] * p.y + m[9] * p.z + m[13];
    result.z = m[2] * p.x + m[6] * p.y + m[10] * p.z + m[14];
    return result;
}

static ModelVec3 transformGlbNormal(const cgltf_float* m, const ModelVec3& n) {
    const float a00 = m[0], a01 = m[4], a02 = m[8];
    const float a10 = m[1], a11 = m[5], a12 = m[9];
    const float a20 = m[2], a21 = m[6], a22 = m[10];
    const float det = a00 * (a11 * a22 - a12 * a21)
                  - a01 * (a10 * a22 - a12 * a20)
                  + a02 * (a10 * a21 - a11 * a20);
    if (std::fabs(det) < 0.000001f) return normalizeModel(n);
    const float invDet = 1.0f / det;
    ModelVec3 result;
    result.x = ((a11 * a22 - a12 * a21) * n.x + (a12 * a20 - a10 * a22) * n.y + (a10 * a21 - a11 * a20) * n.z) * invDet;
    result.y = ((a02 * a21 - a01 * a22) * n.x + (a00 * a22 - a02 * a20) * n.y + (a01 * a20 - a00 * a21) * n.z) * invDet;
    result.z = ((a01 * a12 - a02 * a11) * n.x + (a02 * a10 - a00 * a12) * n.y + (a00 * a11 - a01 * a10) * n.z) * invDet;
    return normalizeModel(result);
}

bool Model::loadGLB(const char* path) {
    destroy();

    std::string glbPath(path ? path : "");
    std::ifstream probe(glbPath.c_str(), std::ios::binary);
    if (!probe.good()) {
        glbPath = std::string("D:/Aquarium_OpenGL/") + glbPath;
    } else {
        probe.close();
    }

    cgltf_options options = {};
    cgltf_data* data = NULL;
    cgltf_result result = cgltf_parse_file(&options, glbPath.c_str(), &data);
    if (result != cgltf_result_success || data == NULL) {
        std::cerr << "Could not parse GLB " << glbPath << std::endl;
        if (data != NULL) cgltf_free(data);
        return false;
    }
    result = cgltf_load_buffers(&options, data, glbPath.c_str());
    if (result != cgltf_result_success) {
        std::cerr << "Could not read GLB buffers " << glbPath << std::endl;
        cgltf_free(data);
        return false;
    }

    for (cgltf_size nodeIndex = 0; nodeIndex < data->nodes_count; ++nodeIndex) {
        const cgltf_node* node = &data->nodes[nodeIndex];
        if (node->mesh == NULL) continue;

        cgltf_float world[16];
        cgltf_node_transform_world(node, world);
        for (cgltf_size primitiveIndex = 0; primitiveIndex < node->mesh->primitives_count; ++primitiveIndex) {
            const cgltf_primitive& primitive = node->mesh->primitives[primitiveIndex];
            if (primitive.type != cgltf_primitive_type_triangles) continue;

            const cgltf_accessor* positions = cgltf_find_accessor(&primitive, cgltf_attribute_type_position, 0);
            const cgltf_accessor* normals = cgltf_find_accessor(&primitive, cgltf_attribute_type_normal, 0);
            const cgltf_accessor* texcoords = cgltf_find_accessor(&primitive, cgltf_attribute_type_texcoord, 0);
            const cgltf_accessor* colors = cgltf_find_accessor(&primitive, cgltf_attribute_type_color, 0);
            if (positions == NULL || positions->type != cgltf_type_vec3) continue;

            const cgltf_size elementCount = primitive.indices ? primitive.indices->count : positions->count;
            if (elementCount < 3) continue;
            std::vector<ModelVertex> vertices;
            vertices.reserve((size_t)elementCount);

            for (cgltf_size i = 0; i + 2 < elementCount; i += 3) {
                ModelVec3 points[3];
                cgltf_size sourceIndices[3];
                bool validTriangle = true;
                for (int corner = 0; corner < 3; ++corner) {
                    sourceIndices[corner] = primitive.indices
                        ? cgltf_accessor_read_index(primitive.indices, i + (cgltf_size)corner)
                        : i + (cgltf_size)corner;
                    if (sourceIndices[corner] >= positions->count) {
                        validTriangle = false;
                        break;
                    }
                    cgltf_float p[3] = {0.0f, 0.0f, 0.0f};
                    if (!cgltf_accessor_read_float(positions, sourceIndices[corner], p, 3)) {
                        validTriangle = false;
                        break;
                    }
                    ModelVec3 sourcePoint = {p[0], p[1], p[2]};
                    points[corner] = transformGlbPoint(world, sourcePoint);
                }
                if (!validTriangle) continue;

                ModelVec3 faceNormal = normalizeModel(crossModel(
                    subtractModel(points[1], points[0]),
                    subtractModel(points[2], points[0])));
                for (int corner = 0; corner < 3; ++corner) {
                    ModelVertex vertex;
                    vertex.position[0] = points[corner].x;
                    vertex.position[1] = points[corner].y;
                    vertex.position[2] = points[corner].z;

                    ModelVec3 normal = faceNormal;
                    if (normals != NULL) {
                        cgltf_float n[3] = {0.0f, 0.0f, 0.0f};
                        if (cgltf_accessor_read_float(normals, sourceIndices[corner], n, 3)) {
                            ModelVec3 sourceNormal = {n[0], n[1], n[2]};
                            normal = transformGlbNormal(world, sourceNormal);
                        }
                    }
                    vertex.normal[0] = normal.x;
                    vertex.normal[1] = normal.y;
                    vertex.normal[2] = normal.z;

                    cgltf_float uv[2] = {0.0f, 0.0f};
                    if (texcoords != NULL && sourceIndices[corner] < texcoords->count)
                        cgltf_accessor_read_float(texcoords, sourceIndices[corner], uv, 2);
                    vertex.texCoord[0] = uv[0];
                    vertex.texCoord[1] = uv[1];
                    cgltf_float color[4] = {1.0f, 1.0f, 1.0f, 1.0f};
                    if (colors != NULL && sourceIndices[corner] < colors->count)
                        cgltf_accessor_read_float(colors, sourceIndices[corner], color, 4);
                    vertex.color[0] = color[0];
                    vertex.color[1] = color[1];
                    vertex.color[2] = color[2];
                    vertex.color[3] = color[3];
                    vertices.push_back(vertex);
                }
            }

            if (vertices.empty()) continue;
            Part part;
            part.vertexCount = (GLsizei)vertices.size();
            if (primitive.material != NULL && primitive.material->has_pbr_metallic_roughness) {
                const cgltf_float* color = primitive.material->pbr_metallic_roughness.base_color_factor;
                part.diffuse[0] = color[0];
                part.diffuse[1] = color[1];
                part.diffuse[2] = color[2];
                part.opacity = std::max(0.0f, std::min(1.0f, color[3]));
                const cgltf_texture_view& baseColorTexture =
                    primitive.material->pbr_metallic_roughness.base_color_texture;
                if (baseColorTexture.texture != NULL)
                    part.diffuseTexture = loadGlbTexture(baseColorTexture.texture->image,
                                                          directoryOf(glbPath));
            }

            glGenVertexArrays(1, &part.vao);
            glGenBuffers(1, &part.vbo);
            glBindVertexArray(part.vao);
            glBindBuffer(GL_ARRAY_BUFFER, part.vbo);
            glBufferData(GL_ARRAY_BUFFER,
                         (GLsizeiptr)(vertices.size() * sizeof(ModelVertex)),
                         &vertices[0], GL_STATIC_DRAW);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(ModelVertex), (void*)0);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(ModelVertex),
                                  (void*)(3 * sizeof(float)));
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(ModelVertex),
                                  (void*)(6 * sizeof(float)));
            glEnableVertexAttribArray(2);
            glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(ModelVertex),
                                  (void*)(8 * sizeof(float)));
            glEnableVertexAttribArray(3);
            glBindVertexArray(0);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            parts.push_back(part);
        }
    }

    cgltf_free(data);
    if (parts.empty()) {
        std::cerr << "GLB has no triangle meshes: " << glbPath << std::endl;
        return false;
    }
    return true;
}
void Model::draw(GLint tintUniform, GLint textureUniform,
                 GLint useTextureUniform, GLuint fallbackTexture,
                 float tintR, float tintG, float tintB, float tintA) const {
    glActiveTexture(GL_TEXTURE0);
    glUniform1i(textureUniform, 0);
    size_t i;
    for (i = 0; i < parts.size(); ++i) {
        const Part& part = parts[i];
        GLuint texture = part.diffuseTexture != 0 ? part.diffuseTexture : fallbackTexture;
        glUniform4f(tintUniform,
                    tintR * part.diffuse[0],
                    tintG * part.diffuse[1],
                    tintB * part.diffuse[2],
                    tintA * part.opacity);
        glUniform1f(useTextureUniform, texture != 0 ? 1.0f : 0.0f);
        glBindTexture(GL_TEXTURE_2D, texture);
        glBindVertexArray(part.vao);
        glDrawArrays(GL_TRIANGLES, 0, part.vertexCount);
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Model::destroy() {
    size_t i;
    for (i = 0; i < parts.size(); ++i) {
        if (parts[i].diffuseTexture != 0) glDeleteTextures(1, &parts[i].diffuseTexture);
        if (parts[i].vbo != 0) glDeleteBuffers(1, &parts[i].vbo);
        if (parts[i].vao != 0) glDeleteVertexArrays(1, &parts[i].vao);
    }
    parts.clear();
}
