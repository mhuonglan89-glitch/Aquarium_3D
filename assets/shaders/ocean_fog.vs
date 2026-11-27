#version 330 core

/**
 * ==============================================================================================
 * 📌 FILE: assets/shaders/ocean_fog.vs (VERTEX SHADER)
 * 👤 PHỤ TRÁCH: BẠN B (Frontend / Kỹ sư Lập trình Pipeline Đồ Họa GLSL)
 * 📚 KIẾN THỨC GIÁO TRÌNH ÁNH XẠ:
 *    - CHƯƠNG CG.2 & CG.4: BIẾN ĐỔI KHÔNG GIAN TỌA ĐỘ TRONG PIPELINE
 *      -> Model Space -> World Space -> View Space -> Clip Space
 *      -> Ma trận MVP: gl_Position = projection * view * model * vec4(aPos, 1.0)
 *      -> Ma trận biến đổi vector pháp tuyến: Normal Matrix = transpose(inverse(model))
 *    - CHƯƠNG CG.3: DAO ĐỘNG UỐN LƯỢN VÂY ĐUÔI CÁ THEO HÀM SIN (Hierarchical Fish Motion)
 * ==============================================================================================
 */

// ----------------------------------------------------------------------------------------------
// [CỤM 1: CÁC THUỘC TÍNH ĐỈNH ĐẦU VÀO TỪ VBO (VERTEX ATTRIBUTES)]
// ----------------------------------------------------------------------------------------------
layout (location = 0) in vec3 aPos;       // Vị trí đỉnh cục bộ (x, y, z)
layout (location = 1) in vec3 aNormal;    // Vector pháp tuyến đỉnh cục bộ (nx, ny, nz)
layout (location = 2) in vec2 aTexCoords; // Tọa độ vân bề mặt (u, v)

// ----------------------------------------------------------------------------------------------
// [CỤM 2: DỮ LIỆU ĐẦU RA TRUYỀN SANG GIAI ĐOẠN RASTERIZATION & FRAGMENT SHADER]
// ----------------------------------------------------------------------------------------------
out vec3 FragPos;   // Tọa độ đỉnh trong không gian thế giới (World Space)
out vec3 Normal;    // Vector pháp tuyến đã biến đổi sang không gian thế giới
out vec2 TexCoords; // Tọa độ Texture truyền tiếp cho Fragment Shader

// ----------------------------------------------------------------------------------------------
// [CỤM 3: CÁC BIẾN TOÀN CỤC UNIFORMS NHẬN TỪ CPU (C++)]
// ----------------------------------------------------------------------------------------------
uniform mat4 model;      // Ma trận mô hình: Tịnh tiến, Xoay, Co dãn (TRS)
uniform mat4 view;       // Ma trận quan sát Camera (LookAt)
uniform mat4 projection; // Ma trận chiếu phối cảnh (Perspective Projection)
uniform float time;      // Biến thời gian thực để tạo dao động sinh học
uniform bool isFish;     // Cờ: true = Áp dụng hiệu ứng uốn đuôi cá; false = Vật thể tĩnh

void main() {
    vec3 pos = aPos;
    
    // ------------------------------------------------------------------------------------------
    // [CỤM 4: HIỆU ỨNG DAO ĐỘNG UỐN ĐUÔI CÁ THEO HÀM SIN (CHƯƠNG CG.3)]
    // ------------------------------------------------------------------------------------------
    // Nếu là loài cá và phần thân nằm về phía đuôi (trục z < -2.0), tạo dao động lắc lư theo trục X
    if (isFish && pos.z < -2.0) {
        float wag = sin(time * 5.5 - pos.z * 0.1) * 0.06 * (-pos.z);
        pos.x += wag; // Uốn cong thân cá sang trái/phải
    }

    // ------------------------------------------------------------------------------------------
    // [CỤM 5: CHUỖI BIẾN ĐỔI MA TRẬN KHÔNG GIAN (MVP TRANSFORMATION)]
    // ------------------------------------------------------------------------------------------
    // 1. Tính tọa độ điểm ảnh trong Thế giới: FragPos = Model * vec4(pos, 1.0)
    FragPos = vec3(model * vec4(pos, 1.0));

    // 2. Tính Vector pháp tuyến trong Thế giới bằng Ma trận chuyển vị nghịch đảo (Normal Matrix)
    // Giúp bảo toàn tính trực giao góc 90 độ khi vật thể bị co dãn không đồng đều (Non-uniform scale)
    Normal = mat3(transpose(inverse(model))) * aNormal;

    // 3. Chuyển tiếp tọa độ UV
    TexCoords = aTexCoords;

    // 4. Tọa độ đỉnh cuối cùng trong Không gian Cắt đồng nhất (Clip Space):
    gl_Position = projection * view * vec4(FragPos, 1.0);
}
