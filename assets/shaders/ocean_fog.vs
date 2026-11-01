#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float time;
uniform bool isFish;

void main() {
    vec3 pos = aPos;
    
    // Dao động uốn lượn vây đuôi cá tự nhiên (Chương CG.3 & CG.4)
    if (isFish && pos.z < -2.0) {
        float wag = sin(time * 5.5 - pos.z * 0.1) * 0.06 * (-pos.z);
        pos.x += wag;
    }

    FragPos = vec3(model * vec4(pos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;
    TexCoords = aTexCoords;
    gl_Position = projection * view * vec4(FragPos, 1.0);
}
