# -*- coding: utf-8 -*-
"""
Chuyển đổi ảnh chụp BMP sang PNG (KHÔNG LẬT - đúng chiều gốc 100%)
và sinh các ảnh Code Cards sáng sủa, chữ to rõ nét cho Slide thuyết trình tông sáng.
"""

import os
from PIL import Image, ImageDraw, ImageFont

MEDIA_DIR = "D:\\Aquarium_OpenGL\\slide_assets"
os.makedirs(MEDIA_DIR, exist_ok=True)

# 1. Chuyển đổi các ảnh BMP sang PNG đúng chiều chuẩn (không flip)
bmp_files = [
    "shot_tunnel.bmp",
    "shot_dive.bmp",
    "shot_shark_bezier.bmp",
    "shot_corals_bubbles.bmp",
    "shot_fog_comparison.bmp"
]

for bmp in bmp_files:
    if os.path.exists(bmp):
        png_name = bmp.replace(".bmp", ".png")
        png_path = os.path.join(MEDIA_DIR, png_name)
        img = Image.open(bmp)
        # Lưu trực tiếp đúng chiều tự nhiên của OpenGL BMP
        img.save(png_path, "PNG")
        print(f"[OK] Da luu dung chieu (khong lat): {png_path}")

# 2. Sinh các ảnh Code Cards tông sáng (Light Academic Theme), chữ to rõ nét
def create_light_code_image(filename, title, code_lines, badge_text, badge_color):
    width, height = 960, 540
    # Nền sáng tinh tế (Light Slate #F8FAFC)
    img = Image.new("RGBA", (width, height), (248, 250, 252, 255))
    draw = ImageDraw.Draw(img)

    # Viền card và thanh tiêu đề sáng
    draw.rounded_rectangle([0, 0, width - 1, height - 1], radius=16, outline=(203, 213, 225, 255), width=2)
    draw.rectangle([0, 0, width, 52], fill=(241, 245, 249, 255))
    draw.line([(0, 52), (width, 52)], fill=(203, 213, 225, 255), width=2)

    # 3 nút phong cách window
    draw.ellipse([20, 20, 32, 32], fill=(239, 68, 68))
    draw.ellipse([42, 20, 54, 32], fill=(245, 158, 11))
    draw.ellipse([64, 20, 76, 32], fill=(16, 185, 129))

    try:
        font_title = ImageFont.truetype("arialbd.ttf", 19)
        font_badge = ImageFont.truetype("arialbd.ttf", 13)
        font_code = ImageFont.truetype("consola.ttf", 17)
    except:
        font_title = font_badge = font_code = ImageFont.load_default()

    # Title chữ tối tương phản cao
    draw.text((95, 15), title, font=font_title, fill=(15, 23, 42))

    # Badge
    badge_w = 125
    draw.rounded_rectangle([width - badge_w - 20, 12, width - 20, 40], radius=6, fill=badge_color)
    draw.text((width - badge_w - 8, 17), badge_text, font=font_badge, fill=(255, 255, 255))

    # Code lines
    y = 74
    line_num = 1
    for line in code_lines:
        # Số dòng xám nhạt
        draw.text((22, y), f"{line_num:2d}", font=font_code, fill=(148, 163, 184))
        # Text
        color = (30, 41, 59) # Slate 800
        s = line.strip()
        if s.startswith("//") or s.startswith("/*"):
            color = (22, 163, 74)   # Comment xanh lá đậm
        elif s.startswith("uniform") or s.startswith("vec3") or s.startswith("float") or s.startswith("Mat4") or s.startswith("struct"):
            color = (2, 132, 199)   # Keyword xanh dương Sky Blue
        elif "sin(" in s or "cos(" in s or "clamp(" in s or "mix(" in s or "atan2" in s or "pow(" in s:
            color = (217, 119, 6)   # Hàm Amber/Cam
        elif s.startswith("return") or s.startswith("if") or s.startswith("else") or s.startswith("for"):
            color = (192, 38, 211)  # Control tím đậm

        draw.text((60, y), line, font=font_code, fill=color)
        y += 28
        line_num += 1

    out_path = os.path.join(MEDIA_DIR, filename)
    img.save(out_path, "PNG")
    print(f"[OK] Da tao Code Card sang: {out_path}")
    return out_path

# Tạo 6 ảnh code minh họa tông sáng
create_light_code_image(
    "code_tunnel_math.png",
    "createTunnelGlass() — Sinh vòm kính Acrylic tham số",
    [
        "// [BẠN A] Sinh vòm kính hình nửa trụ: x = R*cos(theta), y = y0 + R*sin(theta)",
        "for (int i = 0; i < segs; ++i) {",
        "    float t1 = PI * (float)i / segs, t2 = PI * (float)(i + 1) / segs;",
        "    float x1 = radius * cos(t1), y1 = centerY + radius * sin(t1);",
        "    float x2 = radius * cos(t2), y2 = centerY + radius * sin(t2);",
        "    Vec3 n1(cos(t1), sin(t1), 0.0f), n2(cos(t2), sin(t2), 0.0f);",
        "    for (int j = 0; j < stepsZ; ++j) {",
        "        // Lưới 2 lớp: mặt ngoài (+N) và mặt trong (-N) chống Back-face Culling",
        "        addTriangle(verts, v1, v2, v3, n1);",
        "        addTriangle(verts, v1, v3, v2, multiply(n1, -1.0f));",
        "    }",
        "}"
    ],
    "BẠN A (CG.5)", (2, 132, 199)
)

create_light_code_image(
    "code_bezier_math.png",
    "Bezier3D & getFacingAngle() — Tiếp tuyến định hướng cá",
    [
        "// [BẠN A] Đạo hàm bậc 1 tính Vector Tiếp tuyến (Tangent):",
        "Vec3 getTangent(float t) const {",
        "    float u = 1.0f - t;",
        "    return normalize(3.0f*u*u*(p1 - p0) + 6.0f*u*t*(p2 - p1) + 3.0f*t*t*(p3 - p2));",
        "}",
        "",
        "// Tự động xoay đầu cá theo tiếp tuyến quỹ đạo bơi",
        "float getFacingAngle(float t) const {",
        "    Vec3 T = getTangent(t);",
        "    return atan2(-T.z, T.x); // Xoay Yaw trên mặt phẳng ngang XZ",
        "}",
        "float getPitchAngle(float t) const { return atan2(T.y, sqrt(T.x*T.x + T.z*T.z)); }"
    ],
    "BẠN A (CG.5)", (2, 132, 199)
)

create_light_code_image(
    "code_camera_lerp.png",
    "Camera LERP & SPACE Toggle — Chuyển cảnh Xuyên Kính",
    [
        "// [BẠN A] Thuật toán LERP nội suy vị trí mượt mà không bị giật",
        "camPos = add(camPos, multiply(subtract(targetCamPos, camPos), dt * 3.5f));",
        "camFov += (targetFov - camFov) * dt * 3.5f;",
        "",
        "if (spacePressed) {",
        "    isDiving = !isDiving;",
        "    if (isDiving) {",
        "        targetCamPos = Vec3(0.0f, 0.40f, 3.2f); // Lướt xuyên kính ra ngoài",
        "        targetFov = 68.0f;                      // Mở rộng FOV điện ảnh",
        "    } else {",
        "        targetCamPos = Vec3(0.0f, -0.25f, 7.8f); // Thu về sàn hầm kính",
        "    }",
        "}"
    ],
    "BẠN A (CG.4)", (2, 132, 199)
)

create_light_code_image(
    "code_uswim_shader.png",
    "ocean_fog.vs — Vertex Shader uốn đuôi & vỗ cánh cá",
    [
        "// [BẠN B] Biến dạng đỉnh thời gian thực uSwim ngay trên GPU",
        "if (uSwim > 0.5 && uSwim < 1.5) {",
        "    // Cá mập voi: đỉnh càng gần đuôi thì quẫy ngang Z càng mạnh",
        "    float tailWeight = clamp((-aPosition.x - 0.10) / 1.55, 0.0, 1.0);",
        "    localPosition.z += sin(uTime * 4.2 + aPosition.x * 3.0) * 0.085 * tailWeight;",
        "    localPosition.y += sin(uTime * 2.1 + aPosition.x * 2.2) * 0.018 * tailWeight;",
        "} else if (uSwim > 3.5 && uSwim < 4.5) {",
        "    // Cá đuối đốm: vỗ cánh theo phương thẳng đứng Y",
        "    float wingWeight = clamp((abs(aPosition.z) - 0.10) * 1.7, 0.0, 1.0);",
        "    localPosition.y += sin(uTime * 2.2 + abs(aPosition.z) * 3.4) * 0.10 * wingWeight;",
        "}"
    ],
    "BẠN B (CG.6)", (5, 150, 105)
)

create_light_code_image(
    "code_blinn_phong.png",
    "ocean_fog.fs — Blinn-Phong Specular & Caustics & Fog",
    [
        "// [BẠN B] Halfway Vector H = normalize(L + V)",
        "vec3 halfDirection = normalize(lightDirection + viewDirection);",
        "float spec = pow(max(dot(normal, halfDirection), 0.0), 42.0);",
        "vec3 specularColor = vec3(0.55, 0.78, 0.92) * spec * 0.38;",
        "",
        "// Khúc xạ Caustics trên cát bằng giao thoa sóng sin",
        "float caustic = 1.0 + uCaustics * 0.65 * pow(max(causticCell, 0.0), 10.0);",
        "",
        "// Sương mù biển sâu Exponential Ocean Fog: fog = e^(-density * d)",
        "float fogFactor = exp(-uFogDensity * length(uViewPosition - vWorldPosition));",
        "FragColor = vec4(mix(uFogColor, litColor, clamp(fogFactor, 0.0, 1.0)), alpha);"
    ],
    "BẠN B (CG.6)", (5, 150, 105)
)

create_light_code_image(
    "code_particles_zbuffer.png",
    "ParticleSystem & Z-Buffer Alpha Blending",
    [
        "// [BẠN B] Quy trình quản lý Z-Buffer vẽ kính và bọt khí trong suốt",
        "// 1. Vẽ vật thể đặc (Cát, San hô, Cá) với ghi độ sâu Z-Buffer bình thường",
        "glDepthMask(GL_TRUE);",
        "",
        "// 2. Kích hoạt Alpha Blending và tạm KHÓA ghi độ sâu để chống lỗi Z-fighting",
        "glEnable(GL_BLEND);",
        "glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);",
        "glDepthMask(GL_FALSE);",
        "",
        "// 3. Vẽ 80 hạt bọt khí nổi li ti & Vòm kính Acrylic trong suốt",
        "for (int i = 0; i < 80; ++i) drawMeshShape(sphere, ... bubble, alpha = 0.55);",
        "drawMeshShape(tunnelGlass, ... alpha = 0.26);",
        "glDepthMask(GL_TRUE); // Khôi phục ghi độ sâu"
    ],
    "BẠN B (CG.6)", (5, 150, 105)
)

print("[OK] Hoan tat sinh toan bo Media anh chup dung chieu va Code Cards sang!")
