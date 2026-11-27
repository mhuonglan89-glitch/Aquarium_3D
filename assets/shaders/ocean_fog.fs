#version 330 core

/**
 * ==============================================================================================
 * 📌 FILE: assets/shaders/ocean_fog.fs (FRAGMENT SHADER)
 * 👤 PHỤ TRÁCH: BẠN B (Frontend / Kỹ sư Chiếu Sáng GPU & Kỹ Xảo Đại Dương)
 * 📚 KIẾN THỨC GIÁO TRÌNH ÁNH XẠ:
 *    - CHƯƠNG CG.6: CÁC KỸ THUẬT ĐỒ HỌA NÂNG CAO (Advanced Graphics Techniques)
 *      1. Mô hình chiếu sáng Blinn-Phong:
 *         -> Ambient (Môi trường tán xạ dưới nước)
 *         -> Diffuse (Khuếch tán theo định luật Lambert)
 *         -> Specular (Phản xạ gương qua Vector nửa đường Halfway H)
 *      2. Hiệu ứng khúc xạ tia nắng gợn sóng (Dynamic Water Caustics)
 *      3. Sương mù nước biển hàm mũ (Underwater Exponential Fog: Beer-Lambert Law)
 *      4. Kính Acrylic trong suốt (Alpha Blending)
 * ==============================================================================================
 */

out vec4 FragColor; // Màu sắc cuối cùng xuất ra Framebuffer của pixel (r, g, b, a)

// ----------------------------------------------------------------------------------------------
// [CỤM 1: CÁC BIẾN ĐẦU VÀO NHẬN TỪ VERTEX SHADER SAU KHI NỘI SUY (VARYING INPUTS)]
// ----------------------------------------------------------------------------------------------
in vec3 FragPos;   // Tọa độ 3D của pixel trong Không gian Thế giới
in vec3 Normal;    // Vector pháp tuyến của pixel trong Không gian Thế giới
in vec2 TexCoords; // Tọa độ dán vân bề mặt (u, v)

// ----------------------------------------------------------------------------------------------
// [CỤM 2: CÁC BIẾN TOÀN CỤC UNIFORMS TRUYỀN TỪ C++]
// ----------------------------------------------------------------------------------------------
uniform sampler2D texture_diffuse1; // Bộ lấy mẫu Texture ảnh 2D
uniform bool useTexture;            // Cờ: true = Dùng ảnh texture; false = Dùng màu thuần
uniform vec3 objectColor;           // Màu cơ sở mặc định của vật thể
uniform vec3 viewPos;               // Tọa độ camera hiện tại trong không gian thế giới
uniform float alphaVal;             // Độ trong suốt (1.0 = Đục hoàn toàn; 0.28 = Kính trong suốt)
uniform float time;                 // Biến thời gian tính chu kỳ sóng tia nắng Caustics

void main() {
    // ------------------------------------------------------------------------------------------
    // [CỤM 3: ĐẶC TẢ NGUỒN SÁNG MẶT TRỜI NHIỆT ĐỚI VÀ MÔI TRƯỜNG BIỂN SÂU]
    // ------------------------------------------------------------------------------------------
    vec3 sunPos = vec3(0.0, 600.0, 0.0);        // Nguồn sáng mặt trời đặt trên đỉnh cao mặt nước
    vec3 sunColor = vec3(0.85, 0.96, 1.0);       // Ánh nắng trắng xanh rực rỡ nhiệt đới
    vec3 waterAmbient = vec3(0.18, 0.48, 0.68);  // Sắc xanh ngọc bích tán xạ của nước (Turquoise)
    vec3 norm = normalize(Normal);               // Chuẩn hóa vector pháp tuyến về độ dài đơn vị 1.0

    // ------------------------------------------------------------------------------------------
    // [CỤM 4: THÀNH PHẦN 1 — ÁNH SÁNG MÔI TRƯỜNG TÁN XẠ (AMBIENT LIGHTING)]
    // ------------------------------------------------------------------------------------------
    // Mô phỏng ánh sáng xanh khuếch tán đa hướng len lỏi trong lòng đại dương
    vec3 ambient = 0.55 * waterAmbient;

    // ------------------------------------------------------------------------------------------
    // [CỤM 5: THÀNH PHẦN 2 — ÁNH SÁNG KHUẾCH TÁN LAMBERT (DIFFUSE LIGHTING)]
    // ------------------------------------------------------------------------------------------
    // Cường độ tỉ lệ thuận với cosin góc tạo bởi pháp tuyến và hướng tia sáng: max(dot(N, L), 0.0)
    vec3 lightDir = normalize(sunPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * sunColor * 0.75;

    // ------------------------------------------------------------------------------------------
    // [CỤM 6: THÀNH PHẦN 3 — PHẢN XẠ GƯƠNG BLINN-PHONG (SPECULAR LIGHTING)]
    // ------------------------------------------------------------------------------------------
    // Sử dụng Vector nửa đường: H = normalize(L + V)
    // Ưu điểm vượt trội so với Phong: Tính toán nhanh hơn và vệt sáng hình elip mượt mà trên vảy cá
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(norm, halfwayDir), 0.0), 48.0); // Độ bóng cao (Shininess = 48)
    vec3 specular = 0.65 * spec * sunColor;

    // ------------------------------------------------------------------------------------------
    // [CỤM 7: THÀNH PHẦN 4 — GỢN SÓNG KHÚC XẠ ÁNH NẮNG NƯỚC ĐỘNG (DYNAMIC CAUSTICS)]
    // ------------------------------------------------------------------------------------------
    // Mô phỏng các dải tia nắng nhảy múa in bóng trên lưng cá và đáy cát bằng giao thoa sóng sin/cos
    float caustic1 = sin(FragPos.x * 0.08 + time * 2.2) * cos(FragPos.z * 0.08 + time * 1.8);
    float caustic2 = sin(FragPos.x * 0.12 - time * 1.5) * cos(FragPos.z * 0.12 + time * 2.5);
    float causticIntensity = clamp((caustic1 + caustic2) * 0.5, 0.0, 1.0);
    vec3 causticColor = vec3(0.35, 0.85, 0.95) * causticIntensity * 0.45;

    // ------------------------------------------------------------------------------------------
    // [CỤM 8: HÒA TRỘN MÀU CƠ SỞ VỚI TỔNG CHIẾU SÁNG (LIGHTING EQUATION)]
    // ------------------------------------------------------------------------------------------
    vec3 baseColor = objectColor;
    if (useTexture) {
        baseColor = texture(texture_diffuse1, TexCoords).rgb; // Lấy màu từ ảnh Texture
    }

    // Tổng hợp ánh sáng theo công thức Blinn-Phong: (Ambient + Diffuse + Specular + Caustics) * BaseColor
    vec3 lightedColor = (ambient + diffuse + specular + causticColor) * baseColor;

    // ------------------------------------------------------------------------------------------
    // [CỤM 9: THÀNH PHẦN 5 — SƯƠNG MÙ NƯỚC BIỂN HÀM MŨ (UNDERWATER EXPONENTIAL FOG)]
    // ------------------------------------------------------------------------------------------
    // Định luật hấp thụ quang học: fogFactor = exp(-density * distance)
    // distance: Khoảng cách từ mắt nhìn (Camera) tới điểm ảnh
    float distance = length(viewPos - FragPos);
    float fogDensity = 0.00052; // Mật độ hạt lơ lửng trong nước biển nhiệt đới trong xanh (nhìn xa rõ cá)
    float fogFactor = exp(-fogDensity * distance);
    fogFactor = clamp(fogFactor, 0.0, 1.0);

    // Bảng màu đại dương chuẩn hệ thống: Màu xanh ngọc nhiệt đới (#045a78)
    vec3 tropicalOceanCyan = vec3(0.03, 0.44, 0.60);
    
    // Hàm mix(A, B, t) = A*(1-t) + B*t: Càng ở xa, màu sắc càng hòa tan vào màu xanh nước biển
    vec3 finalColor = mix(tropicalOceanCyan, lightedColor, fogFactor);

    // Xuất màu cuối cùng kèm độ trong suốt alphaVal phục vụ Alpha Blending vòm kính
    FragColor = vec4(finalColor, alphaVal);
}
