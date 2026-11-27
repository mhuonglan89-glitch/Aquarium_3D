#version 330 core
layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec4 aColor;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;
uniform float uTime;
uniform float uSwim;

out vec3 vWorldPosition;
out vec3 vWorldNormal;
out vec2 vTexCoord;
out vec4 vColor;

void main() {
    vec3 localPosition = aPosition;
    if (uSwim > 0.5 && uSwim < 1.5) {
        // Cá mập OBJ cũ có thân chạy theo trục X.
        float tailWeight = clamp((-aPosition.x - 0.10) / 1.55, 0.0, 1.0);
        localPosition.z += sin(uTime * 4.2 + aPosition.x * 3.0) * 0.085 * tailWeight;
        localPosition.y += sin(uTime * 2.1 + aPosition.x * 2.2) * 0.018 * tailWeight;
    } else if (uSwim > 3.5 && uSwim < 4.5) {
        // Cá đuối có hai cánh trải theo trục Z sau khi đổi sang hệ +X-forward.
        float wingDistance = abs(aPosition.z);
        float wingWeight = clamp((wingDistance - 0.10) * 1.7, 0.0, 1.0);
        float wingPhase = uTime * 2.2 + wingDistance * 3.4;
        localPosition.y += sin(wingPhase) * 0.10 * wingWeight;
        localPosition.x += cos(wingPhase) * 0.018 * wingWeight;
    } else if (uSwim > 1.5) {
        // Các GLB mới có thân theo trục Z; uSwim > 2.5 đảo phía đuôi.
        float tailSide = uSwim > 2.5 ? 1.0 : -1.0;
        float tailWeight = clamp((tailSide * aPosition.z - 0.035) * 8.0, 0.0, 1.0);
        localPosition.x += sin(uTime * 3.4 + aPosition.z * 8.0) * 0.020 * tailWeight;
        localPosition.y += sin(uTime * 1.8 + aPosition.z * 6.0) * 0.006 * tailWeight;
    }
    vec4 worldPosition = uModel * vec4(localPosition, 1.0);
    vWorldPosition = worldPosition.xyz;
    vWorldNormal = mat3(transpose(inverse(uModel))) * aNormal;
    vTexCoord = aTexCoord;
    vColor = aColor;
    gl_Position = uProjection * uView * worldPosition;
}
