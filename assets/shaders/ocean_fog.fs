#version 330 core

/**
 * ==============================================================================================
 * 📌 FILE: assets/shaders/ocean_fog.fs (FRAGMENT SHADER)
 * 👤 PHỤ TRÁCH: BẠN B (Frontend / Kỹ sư Chiếu Sáng GPU & Kỹ Xảo Đại Dương)
 * 📚 KIẾN THỨC GIÁO TRÌNH ÁNH XẠ:
 *    - CHƯƠNG CG.6: CÁC KỸ THUẬT ĐỒ HỌA NÂNG CAO (Advanced Graphics Techniques)
 *      1. Mô hình chiếu sáng Blinn-Phong (Ambient + Diffuse + Specular Halfway H)
 *      2. Hiệu ứng khúc xạ tia nắng gợn sóng (Dynamic Water Caustics)
 *      3. Phản quang viền Fresnel trên vòm kính Acrylic trong suốt
 *      4. Sương mù nước biển hàm mũ (Underwater Exponential Fog: Beer-Lambert Law)
 * ==============================================================================================
 */

out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform sampler2D texture_diffuse1;
uniform bool useTexture;
uniform vec3 objectColor;
uniform vec3 viewPos;
uniform float alphaVal;
uniform float time;
uniform float causticStrength; // 1.0 cho đáy biển/cá, 0.0 cho sàn khô trong hầm

void main() {
    // ------------------------------------------------------------------------------------------
    // [CỤM 1: NGUỒN SÁNG ĐỊNH HƯỚNG MẶT TRỜI NHIỆT ĐỚI (DIRECTIONAL SUNLIGHT)]
    // ------------------------------------------------------------------------------------------
    vec3 lightDir = normalize(vec3(0.22, 0.92, 0.32)); // Chiếu sáng đồng đều toàn bộ đại dương
    vec3 sunColor = vec3(0.92, 0.98, 1.0);
    vec3 waterAmbient = vec3(0.24, 0.56, 0.74);
    vec3 norm = normalize(Normal);

    // ------------------------------------------------------------------------------------------
    // [CỤM 2: MÔ HÌNH CHIẾU SÁNG BLINN-PHONG (AMBIENT + DIFFUSE + SPECULAR)]
    // ------------------------------------------------------------------------------------------
    vec3 ambient = 0.52 * waterAmbient;

    // Khuếch tán 2 mặt nhẹ giúp vây cá và san hô mỏng không bị tối đen hoàn toàn ở mặt dưới
    float diff = max(dot(norm, lightDir), 0.0);
    float backFill = max(dot(-norm, lightDir), 0.0) * 0.25;
    vec3 diffuse = (diff + backFill) * sunColor * 0.78;

    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(norm, halfwayDir), 0.0), 48.0);
    vec3 specular = 0.55 * spec * sunColor;

    // ------------------------------------------------------------------------------------------
    // [CỤM 3: GỢN SÓNG KHÚC XẠ ÁNH NẮNG NƯỚC ĐỘNG (DYNAMIC CAUSTICS)]
    // ------------------------------------------------------------------------------------------
    float c1 = sin(FragPos.x * 0.045 + time * 2.0) * cos(FragPos.z * 0.045 + time * 1.6);
    float c2 = sin(FragPos.x * 0.085 - time * 1.4) * cos(FragPos.z * 0.085 + time * 2.3);
    float causticIntensity = clamp((c1 + c2) * 0.5 + 0.15, 0.0, 1.0);
    vec3 causticColor = vec3(0.32, 0.85, 0.98) * causticIntensity * 0.42 * causticStrength;

    // ------------------------------------------------------------------------------------------
    // [CỤM 4: HÒA TRỘN MÀU VẬT THỂ & HIỆU ỨNG VIỀN KÍNH FRESNEL]
    // ------------------------------------------------------------------------------------------
    vec3 baseColor = objectColor;
    if (useTexture) {
        baseColor = texture(texture_diffuse1, TexCoords).rgb;
    }

    vec3 lightedColor = (ambient + diffuse + specular + causticColor) * baseColor;

    float outAlpha = alphaVal;
    if (alphaVal < 0.99) {
        // Phản quang viền kính cong Acrylic (Fresnel approximation)
        float fresnel = pow(1.0 - abs(dot(norm, viewDir)), 2.5);
        lightedColor += vec3(0.45, 0.88, 1.0) * fresnel * 0.55;
        outAlpha = clamp(alphaVal + fresnel * 0.25, 0.08, 0.65);
    }

    // ------------------------------------------------------------------------------------------
    // [CỤM 5: SƯƠNG MÙ NƯỚC BIỂN HÀM MŨ (UNDERWATER EXPONENTIAL FOG)]
    // ------------------------------------------------------------------------------------------
    float distance = length(viewPos - FragPos);
    float fogDensity = 0.00050;
    float fogFactor = clamp(exp(-fogDensity * distance), 0.0, 1.0);

    vec3 tropicalOceanCyan = vec3(0.03, 0.44, 0.60);
    vec3 finalColor = mix(tropicalOceanCyan, lightedColor, fogFactor);

    FragColor = vec4(finalColor, outAlpha);
}
