#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform sampler2D texture_diffuse1;
uniform bool useTexture;
uniform vec3 objectColor;
uniform vec3 viewPos;
uniform float alphaVal; // Độ trong suốt cho kính vòm (Alpha Blending)
uniform float time;     // Dùng cho hiệu ứng tia nắng Caustics

void main() {
    // Nguồn sáng mặt trời rọi từ trên đỉnh mặt nước xuống (Chương CG.6 Blinn-Phong)
    vec3 lightPos = vec3(0.0, 500.0, 0.0);
    vec3 lightColor = vec3(0.55, 0.85, 1.0); // Sắc xanh ngọc đại dương
    vec3 norm = normalize(Normal);

    // 1. Thành phần Môi trường (Ambient)
    vec3 ambient = 0.25 * lightColor;

    // 2. Thành phần Khuếch tán (Diffuse - Lambert)
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    // 3. Thành phần Phản xạ gương (Specular - Blinn-Phong)
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(norm, halfwayDir), 0.0), 32.0);
    vec3 specular = 0.35 * spec * lightColor;

    // Hiệu ứng tia nắng nước Caustics gợn sóng nhẹ
    float caustics = sin(FragPos.x * 0.05 + time * 1.5) * cos(FragPos.z * 0.05 + time * 1.5) * 0.08;
    diffuse += vec3(max(0.0, caustics));

    // Lấy màu cơ sở
    vec3 baseColor = objectColor;
    if (useTexture) {
        baseColor = texture(texture_diffuse1, TexCoords).rgb;
    }

    vec3 lightedColor = (ambient + diffuse + specular) * baseColor;

    // 4. Kỹ thuật Sương Mù Nước Biển Sâu (Underwater Exponential Fog - Chương CG.6)
    float distance = length(viewPos - FragPos);
    float fogDensity = 0.0018; // Mật độ độ đục của nước
    float fogFactor = exp(-fogDensity * distance);
    fogFactor = clamp(fogFactor, 0.0, 1.0);

    // Bảng màu đại dương sâu chuẩn hệ thống: #041C2C
    vec3 deepOceanColor = vec3(0.016, 0.110, 0.173);
    vec3 finalColor = mix(deepOceanColor, lightedColor, fogFactor);

    FragColor = vec4(finalColor, alphaVal);
}
