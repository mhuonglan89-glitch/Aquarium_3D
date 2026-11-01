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
    // NGUỒN SÁNG MẶT TRỜI NHIỆT ĐỚI RỰC RỠ TRÊN ĐỈNH NƯỚC (Bright Tropical Sunlight)
    vec3 sunPos = vec3(0.0, 600.0, 0.0);
    vec3 sunColor = vec3(0.85, 0.96, 1.0);  // Ánh nắng trắng xanh rực rỡ
    vec3 waterAmbient = vec3(0.18, 0.48, 0.68); // Sắc xanh ngọc bích sáng (Bright Turquoise)
    vec3 norm = normalize(Normal);

    // 1. Ánh sáng môi trường tán xạ trong nước (High Ambient for Bright Ocean)
    vec3 ambient = 0.55 * waterAmbient;

    // 2. Khuếch tán ánh nắng mặt trời (Diffuse)
    vec3 lightDir = normalize(sunPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * sunColor * 0.75;

    // 3. Phản xạ gương lấp lánh trên lưng cá & mặt kính (Specular Blinn-Phong)
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(norm, halfwayDir), 0.0), 48.0);
    vec3 specular = 0.65 * spec * sunColor;

    // 4. Hiệu ứng gợn sóng khúc xạ ánh nắng (Dynamic Water Caustics)
    float caustic1 = sin(FragPos.x * 0.08 + time * 2.2) * cos(FragPos.z * 0.08 + time * 1.8);
    float caustic2 = sin(FragPos.x * 0.12 - time * 1.5) * cos(FragPos.z * 0.12 + time * 2.5);
    float causticIntensity = clamp((caustic1 + caustic2) * 0.5, 0.0, 1.0);
    vec3 causticColor = vec3(0.35, 0.85, 0.95) * causticIntensity * 0.45;

    // Màu cơ sở vật thể
    vec3 baseColor = objectColor;
    if (useTexture) {
        baseColor = texture(texture_diffuse1, TexCoords).rgb;
    }

    vec3 lightedColor = (ambient + diffuse + specular + causticColor) * baseColor;

    // 5. Sương mù nước biển nhiệt đới trong vắt (Crystalline Tropical Ocean Fog)
    float distance = length(viewPos - FragPos);
    float fogDensity = 0.00095; // Nước biển trong vắt nhìn xa (không bị tối đen)
    float fogFactor = exp(-fogDensity * distance);
    fogFactor = clamp(fogFactor, 0.0, 1.0);

    // Màu nước biển xanh ngọc bích rực rỡ (Cyan Turquoise: #068da9 / #0e7490)
    vec3 tropicalOceanCyan = vec3(0.04, 0.42, 0.58);
    vec3 finalColor = mix(tropicalOceanCyan, lightedColor, fogFactor);

    FragColor = vec4(finalColor, alphaVal);
}
