# -*- coding: utf-8 -*-
"""
Tạo file PowerPoint (.pptx) chuẩn 16:9 với:
1. TÔNG NỀN SÁNG (Light Modern Academic Theme) chuyên nghiệp, tương phản cao trên máy chiếu
2. CỠ CHỮ TO LÊN RÕ RÀNG (Tiêu đề 26pt, Thẻ 17pt, Nội dung 14pt)
3. ẢNH CHỤP 3D ĐÚNG CHIỀU 100% (Không lật) và CODE CARDS SÁNG ĐỒNG BỘ
4. ĐẦY ĐỦ LỜI THOẠI (Speaker Notes) cho từng slide
"""

import os
from pptx import Presentation
from pptx.util import Inches, Pt
from pptx.enum.text import PP_ALIGN
from pptx.dml.color import RGBColor
from pptx.enum.shapes import MSO_SHAPE

MEDIA_DIR = "D:\\Aquarium_OpenGL\\slide_assets"

def create_deck():
    prs = Presentation()
    prs.slide_width = Inches(13.333)
    prs.slide_height = Inches(7.5)

    blank_layout = prs.slide_layouts[6]

    # Bảng màu TÔNG SÁNG chuyên nghiệp (Light Modern Academic)
    BG_LIGHT      = RGBColor(248, 250, 252) # Light Slate #F8FAFC
    CARD_WHITE    = RGBColor(255, 255, 255) # Pure White #FFFFFF
    BORDER_CARD   = RGBColor(226, 232, 240) # Slate 200 #E2E8F0
    TEXT_MAIN     = RGBColor(15, 23, 42)    # Slate 900 (Đen than đậm, cực rõ)
    TEXT_BODY     = RGBColor(30, 41, 59)    # Slate 800 (Dễ đọc trên nền sáng)
    TEXT_MUTED    = RGBColor(100, 116, 139) # Slate 500
    ACCENT_BLUE   = RGBColor(2, 132, 199)   # Sky Blue #0284C7
    COLOR_A       = RGBColor(2, 132, 199)   # Xanh dương cho Bạn A
    COLOR_B       = RGBColor(5, 150, 105)   # Xanh ngọc cho Bạn B

    def set_slide_background(slide):
        bg = slide.shapes.add_shape(MSO_SHAPE.RECTANGLE, 0, 0, prs.slide_width, prs.slide_height)
        bg.fill.solid()
        bg.fill.fore_color.rgb = BG_LIGHT
        bg.line.fill.background()
        return bg

    def add_header(slide, title_text, category_text, presenter_badge=None, badge_color=COLOR_A):
        # Category label
        cat_box = slide.shapes.add_textbox(Inches(0.8), Inches(0.35), Inches(9.5), Inches(0.35))
        tf_cat = cat_box.text_frame
        p_cat = tf_cat.paragraphs[0]
        p_cat.text = category_text.upper()
        p_cat.font.size = Pt(12)
        p_cat.font.bold = True
        p_cat.font.color.rgb = ACCENT_BLUE

        # Title
        title_box = slide.shapes.add_textbox(Inches(0.8), Inches(0.68), Inches(9.5), Inches(0.9))
        tf_title = title_box.text_frame
        p_title = tf_title.paragraphs[0]
        p_title.text = title_text
        p_title.font.size = Pt(25) # Chữ to rõ
        p_title.font.bold = True
        p_title.font.color.rgb = TEXT_MAIN

        # Presenter Badge
        if presenter_badge:
            badge = slide.shapes.add_shape(MSO_SHAPE.ROUNDED_RECTANGLE, Inches(10.5), Inches(0.50), Inches(2.0), Inches(0.48))
            badge.fill.solid()
            badge.fill.fore_color.rgb = badge_color
            badge.line.fill.background()
            p_badge = badge.text_frame.paragraphs[0]
            p_badge.text = presenter_badge
            p_badge.font.size = Pt(12)
            p_badge.font.bold = True
            p_badge.font.color.rgb = RGBColor(255, 255, 255)
            p_badge.alignment = PP_ALIGN.CENTER

    def add_card(slide, left, top, width, height, title, items, border_color=None):
        card = slide.shapes.add_shape(MSO_SHAPE.ROUNDED_RECTANGLE, left, top, width, height)
        card.fill.solid()
        card.fill.fore_color.rgb = CARD_WHITE
        card.line.color.rgb = border_color if border_color else BORDER_CARD
        card.line.width = Pt(2 if border_color else 1.2)

        tf = card.text_frame
        tf.word_wrap = True
        tf.margin_left = Inches(0.3)
        tf.margin_right = Inches(0.3)
        tf.margin_top = Inches(0.25)
        tf.margin_bottom = Inches(0.25)

        p_t = tf.paragraphs[0]
        p_t.text = title
        p_t.font.size = Pt(16.5) # Tiêu đề thẻ to rõ
        p_t.font.bold = True
        p_t.font.color.rgb = border_color if border_color else ACCENT_BLUE
        p_t.space_after = Pt(10)

        for item in items:
            p = tf.add_paragraph()
            p.text = "• " + item
            p.font.size = Pt(13.5) # Nội dung chữ to lên
            p.font.color.rgb = TEXT_BODY
            p.space_after = Pt(6)

    def add_image_safe(slide, img_path, left, top, width, height, caption=None):
        if os.path.exists(img_path):
            pic = slide.shapes.add_picture(img_path, left, top, width, height)
            if caption:
                cap_box = slide.shapes.add_textbox(left, top + height + Inches(0.04), width, Inches(0.32))
                tf_cap = cap_box.text_frame
                p_cap = tf_cap.paragraphs[0]
                p_cap.text = caption
                p_cap.font.size = Pt(10.5)
                p_cap.font.bold = True
                p_cap.font.color.rgb = ACCENT_BLUE
                p_cap.alignment = PP_ALIGN.CENTER
            return pic
        return None

    # =========================================================================
    # SLIDE 1: TIÊU ĐỀ
    # =========================================================================
    s1 = prs.slides.add_slide(blank_layout)
    set_slide_background(s1)

    tbox = s1.shapes.add_textbox(Inches(0.8), Inches(0.55), Inches(11.5), Inches(1.9))
    tf1 = tbox.text_frame
    p1 = tf1.paragraphs[0]
    p1.text = "ĐỒ ÁN MÔN ĐỒ HỌA MÁY TÍNH"
    p1.font.size = Pt(14)
    p1.font.bold = True
    p1.font.color.rgb = ACCENT_BLUE

    p2 = tf1.add_paragraph()
    p2.text = "MÔ PHỎNG THỦY CUNG 3D TƯƠNG TÁC"
    p2.font.size = Pt(33)
    p2.font.bold = True
    p2.font.color.rgb = TEXT_MAIN
    p2.space_before = Pt(4)

    p3 = tf1.add_paragraph()
    p3.text = "Modern OpenGL C++11 & GLSL Shaders | V-Sync 60 FPS | Phân hệ 2 Thành viên"
    p3.font.size = Pt(14.5)
    p3.font.color.rgb = TEXT_MUTED

    add_card(s1, Inches(0.8), Inches(2.6), Inches(5.8), Inches(2.1),
             "BẠN A — Kỹ sư Toán học & Hình học (Backend)",
             [
                 "Hầm kính bán nguyệt tham số (CG.5 Mặt cong tham số)",
                 "Quỹ đạo bơi Bézier 3D bậc 3 & Tiếp tuyến Tangent (CG.5)",
                 "Hệ thống Camera kép 6-DOF & Thuật toán LERP Xuyên Kính (CG.4)",
                 "Sàn hành lang tối màu, dầm thép chịu lực & dải LED cyan neon"
             ], border_color=COLOR_A)

    add_card(s1, Inches(0.8), Inches(4.9), Inches(5.8), Inches(2.1),
             "BẠN B — Kỹ sư Vật liệu & Shaders (Frontend)",
             [
                 "Mô hình chiếu sáng Blinn-Phong & Texture Mapping 2K (CG.6)",
                 "Hiệu ứng khúc xạ tia nắng Caustics & Sương mù biển sâu (CG.6)",
                 "Vertex Shader uốn vây uốn đuôi cá thời gian thực (uSwim)",
                 "Pipeline 8 loài cá 3D (OBJ/GLB) & Hệ hạt bọt khí nổi li ti"
             ], border_color=COLOR_B)

    add_image_safe(s1, os.path.join(MEDIA_DIR, "shot_dive.png"),
                   Inches(6.9), Inches(2.6), Inches(5.6), Inches(4.3),
                   "Giao diện Mô phỏng Thủy cung 3D thời gian thực (Modern OpenGL)")

    s1.notes_slide.notes_text_frame.text = (
        "[LỜI THOẠI BẠN A]: Kính chào Thầy Cô và các bạn. Hôm nay, nhóm chúng em xin đại diện trình bày "
        "đồ án môn Đồ Họa Máy Tính với đề tài: 'Mô phỏng Thủy cung 3D Tương tác' trên nền tảng Modern OpenGL C++ và GLSL. "
        "Nhóm gồm 2 thành viên: Em là Bạn A phụ trách phần Toán học, Hình học tham số và Camera; "
        "Bạn B phụ trách Shaders, Vật liệu chiếu sáng và Tài nguyên 3D."
    )

    # =========================================================================
    # SLIDE 2: KIẾN TRÚC TỔNG QUAN
    # =========================================================================
    s2 = prs.slides.add_slide(blank_layout)
    set_slide_background(s2)
    add_header(s2, "Kiến trúc Đồ họa Tổng quan (Graphics Pipeline Flow)", "Hệ Thống", "BẠN A", COLOR_A)

    add_card(s2, Inches(0.8), Inches(1.8), Inches(5.8), Inches(5.1),
             "Phân Tách Trách Nhiệm CPU & GPU Shader",
             [
                 "1. CPU (C++ Engine) — Bạn A & Bạn B phối hợp:",
                 "   - Quản lý ma trận biến đổi Model, View, Projection (CG.3/4)",
                 "   - Nội suy giải tích đường cong Bézier 3D cho đàn cá lớn",
                 "   - Xử lý bàn phím (WASD, E/Q, SPACE) & Chuột 360 độ",
                 "   - Tải mô hình 3D đa định dạng: OBJ (TinyOBJ) & GLB (cgltf)",
                 "2. GPU (GLSL Shaders) — Bạn B phát triển:",
                 "   - ocean_fog.vs: Biến dạng tọa độ uSwim song song trên GPU",
                 "   - ocean_fog.fs: Blinn-Phong, khúc xạ Caustics, sương mù Fog",
                 "3. Đạt chuẩn 60 FPS cố định (V-Sync), khử giật lag tuyệt đối"
             ])

    add_image_safe(s2, os.path.join(MEDIA_DIR, "shot_tunnel.png"),
                   Inches(6.9), Inches(1.8), Inches(5.6), Inches(4.8),
                   "Góc nhìn từ bên trong Hầm Kính Acrylic nhìn ra đại dương mở")

    s2.notes_slide.notes_text_frame.text = (
        "[LỜI THOẠI BẠN A]: Kiến trúc hệ thống được chia làm hai nửa phối hợp chặt chẽ. Trên CPU, chúng em quản lý "
        "toàn bộ logic ma trận quan sát, toán học Bézier và Camera. Trên GPU, Shader đảm nhận toàn bộ việc uốn vây uốn đuôi "
        "và đổ bóng nước biển. Nhờ tối ưu hóa tính toán song song, chương trình đạt 60 FPS mượt mà."
    )

    # =========================================================================
    # SLIDE 3: BẠN A - HÌNH HỌC THAM SỐ VÒM KÍNH
    # =========================================================================
    s3 = prs.slides.add_slide(blank_layout)
    set_slide_background(s3)
    add_header(s3, "Hình học Tham số: Vòm Kính Acrylic & Kết Cấu Hầm", "Toán Học & Hình Học (CG.5)", "BẠN A", COLOR_A)

    add_card(s3, Inches(0.8), Inches(1.8), Inches(5.8), Inches(5.1),
             "Toán học Mặt cong Tham số (CG.5) & Kết cấu Hầm",
             [
                 "1. Phương trình vòm kính nửa hình trụ:",
                 "   x = R * cos(theta), y = y0 + R * sin(theta), z ∈ [zStart, zEnd]",
                 "   theta ∈ [0, π], Bán kính R = 2.4m, Chiều dài L = 5.2m",
                 "2. Lưới tam giác 2 lớp (Double-sided):",
                 "   Nhân đôi pháp tuyến (-N) mặt trong giúp quan sát từ cả 2 phía không bị Culling",
                 "3. Dầm thép Titanium Steel: Vòng cung chịu lực cách đều Δz = 1.25m",
                 "4. Dải LED neon xanh cyan (#0CE8DA): Chạy dọc 2 bên mép sàn",
                 "   Hệ số phát sáng uEmission = 2.8 giúp dải đèn rực rỡ trong nước"
             ], border_color=COLOR_A)

    add_image_safe(s3, os.path.join(MEDIA_DIR, "shot_tunnel.png"),
                   Inches(6.9), Inches(1.8), Inches(5.6), Inches(2.45),
                   "Ảnh giao diện thực tế: Hầm kính Acrylic và Dải đèn LED dẫn lối")

    add_image_safe(s3, os.path.join(MEDIA_DIR, "code_tunnel_math.png"),
                   Inches(6.9), Inches(4.5), Inches(5.6), Inches(2.4),
                   "Ảnh mã nguồn: Hàm sinh vòm kính tham số 2 lớp pháp tuyến")

    s3.notes_slide.notes_text_frame.text = (
        "[LỜI THOẠI BẠN A]: Em đã áp dụng lý thuyết Mặt cong tham số trong Chương CG.5 để sinh lưới vòm kính hình nửa trụ "
        "từ phương trình toán học thuần túy. Để người đứng trong hầm nhìn ra thấy kính mà bơi ngoài nhìn vào cũng thấy, "
        "em sinh lưới 2 lớp đối xứng pháp tuyến. Hầm còn có hệ thống dầm thép chịu lực uốn cong và dải đèn LED cyan neon "
        "phát sáng dọc mép sàn với hệ số emission cao."
    )

    # =========================================================================
    # SLIDE 4: BẠN A - QUỸ ĐẠO BÉZIER 3D
    # =========================================================================
    s4 = prs.slides.add_slide(blank_layout)
    set_slide_background(s4)
    add_header(s4, "Quỹ đạo Bézier 3D Bậc 3 & Định hướng Tiếp tuyến", "Toán Học & Hình Học (CG.5)", "BẠN A", COLOR_A)

    add_card(s4, Inches(0.8), Inches(1.8), Inches(5.8), Inches(5.1),
             "Giải tích Bézier Bậc 3 & Tự động Xoay đầu Cá",
             [
                 "1. Công thức đa thức Bernstein bậc 3:",
                 "   B(t) = (1-t)³*P0 + 3(1-t)²*t*P1 + 3(1-t)*t²*P2 + t³*P3",
                 "   Lộ trình 4 điểm mốc khép kín quanh rạn san hô",
                 "2. Đạo hàm bậc 1 tính Vector Tiếp tuyến (Tangent):",
                 "   B'(t) = 3(1-t)²(P1-P0) + 6(1-t)t(P2-P1) + 3t²(P3-P2)",
                 "3. Tính góc xoay Euler từ tiếp tuyến T = (Tx, Ty, Tz):",
                 "   - Góc Yaw (Mặt phẳng ngang XZ): theta_yaw = atan2(-Tz, Tx)",
                 "   - Góc Pitch (Nghiêng lên/xuống): theta_pitch = atan2(Ty, |T_xz|)",
                 "4. Kết quả: Cá bơi lượn mượt mà C1/G1, đầu cá luôn đón hướng vận tốc"
             ], border_color=COLOR_A)

    add_image_safe(s4, os.path.join(MEDIA_DIR, "shot_shark_bezier.png"),
                   Inches(6.9), Inches(1.8), Inches(5.6), Inches(2.45),
                   "Ảnh giao diện thực tế: Cá Mập Voi lượn theo đường cong Bézier 3D")

    add_image_safe(s4, os.path.join(MEDIA_DIR, "code_bezier_math.png"),
                   Inches(6.9), Inches(4.5), Inches(5.6), Inches(2.4),
                   "Ảnh mã nguồn: Hàm tính tiếp tuyến Tangent và góc xoay Yaw/Pitch")

    s4.notes_slide.notes_text_frame.text = (
        "[LỜI THOẠI BẠN A]: Để đàn cá lớn bơi tự nhiên, em áp dụng Đường cong Bézier 3D bậc 3. "
        "Điểm đặc sắc là em đã giải tích đạo hàm bậc 1 của đường cong để tìm Vector tiếp tuyến B'(t). "
        "Từ đó, tính ra góc Yaw để xoay đầu cá theo hướng bơi và góc Pitch để cá chúi đầu xuống hoặc ngóc lên khi đổi độ sâu, "
        "giúp chuyển động đạt độ chân thực tuyệt đối."
    )

    # =========================================================================
    # SLIDE 5: BẠN A - CAMERA KÉP & LERP XUYÊN KÍNH
    # =========================================================================
    s5 = prs.slides.add_slide(blank_layout)
    set_slide_background(s5)
    add_header(s5, "Hệ thống Camera kép 6-DOF & Thuật toán LERP Xuyên Kính", "Quan Sát & Tương Tác (CG.4)", "BẠN A", COLOR_A)

    add_card(s5, Inches(0.8), Inches(1.8), Inches(5.8), Inches(5.1),
             "Nội suy Tuyến tính LERP & Chuyển đổi 2 Chế độ",
             [
                 "1. Thuật toán LERP mượt mà theo DeltaTime:",
                 "   P(t) = mix(P_current, P_target, dt * 3.5)",
                 "   FOV(t) = mix(FOV_current, FOV_target, dt * 3.5)",
                 "2. Bấm phím [SPACE] kích hoạt LERP Xuyên Kính:",
                 "   - Chế độ 1: Bước đi trên sàn hầm kính (y = -0.25m, FOV = 50°)",
                 "   - Chế độ 2: Lướt xuyên kính lặn tự do 6-DOF (FOV = 68°)",
                 "   Khi nhấn SPACE: Camera lướt qua kính, hầm kính ẩn hoàn toàn (100% không kính), nhường chỗ cho đại dương mở",
                 "3. Điều khiển 6 Bậc tự do (6-DOF): W, A, S, D bơi đa hướng,",
                 "   phím E bơi ngoi lên, phím Q lặn xuống sâu, chuột xoay 360°"
             ], border_color=COLOR_A)

    add_image_safe(s5, os.path.join(MEDIA_DIR, "shot_dive.png"),
                   Inches(6.9), Inches(1.8), Inches(5.6), Inches(2.45),
                   "Ảnh giao diện thực tế: Chế độ Lặn Biển 100% không kính sau khi bấm SPACE")

    add_image_safe(s5, os.path.join(MEDIA_DIR, "code_camera_lerp.png"),
                   Inches(6.9), Inches(4.5), Inches(5.6), Inches(2.4),
                   "Ảnh mã nguồn: Thuật toán LERP nội suy vị trí và phím SPACE chuyển đổi")

    s5.notes_slide.notes_text_frame.text = (
        "[LỜI THOẠI BẠN A]: Tính năng tương tác độc đáo nhất là Camera kép với hiệu ứng LERP Xuyên Kính. "
        "Ban đầu khách tham quan đi bộ trong hầm. Khi bấm phím SPACE, thuật toán LERP đẩy camera lướt xuyên qua kính "
        "ra đại dương bao la, hầm kính lập tức ẩn đi đem lại cảm giác hòa mình 100% vào làn nước, "
        "mở khóa đầy đủ 6 bậc tự do: bơi lên bằng E, lặn xuống bằng Q. Nhấn SPACE lần nữa sẽ lùi về hầm kính. "
        "Sau đây em xin nhường lời cho Bạn B trình bày về Shaders và Tài nguyên 3D."
    )

    # =========================================================================
    # SLIDE 6: BẠN B - CHIẾU SÁNG BLINN-PHONG
    # =========================================================================
    s6 = prs.slides.add_slide(blank_layout)
    set_slide_background(s6)
    add_header(s6, "Mô hình Chiếu sáng Blinn-Phong & Vật liệu Nước", "Vật Liệu & Shaders (CG.6)", "BẠN B", COLOR_B)

    add_card(s6, Inches(0.8), Inches(1.8), Inches(5.8), Inches(5.1),
             "Mô hình Blinn-Phong trên Fragment Shader",
             [
                 "1. Công thức 3 thành phần phản xạ ánh sáng (CG.6):",
                 "   I = I_ambient + I_diffuse + I_specular",
                 "   - Ambient: Ánh sáng môi trường xanh ngọc đại dương",
                 "   - Diffuse (Lambert): max(N • L, 0) đón hướng tia nắng rọi",
                 "   - Specular (Blinn-Phong): pow(max(N • H, 0), shininess)",
                 "2. Cải tiến Halfway Vector H = normalize(L + V):",
                 "   Thay thế vector phản xạ R của Phong cổ điển, điểm sáng vảy cá chân thực và tiết kiệm chu kỳ GPU",
                 "3. Texture Mapping 2K: Da cá mập voi đốm trắng, thân cá đuối, san hô",
                 "4. Kênh phát sáng uEmission = 2.8 cho dải đèn LED và đàn sứa"
             ], border_color=COLOR_B)

    add_image_safe(s6, os.path.join(MEDIA_DIR, "shot_shark_bezier.png"),
                   Inches(6.9), Inches(1.8), Inches(5.6), Inches(2.45),
                   "Ảnh giao diện thực tế: Điểm sáng gương Specular trên lưng cá đón nắng")

    add_image_safe(s6, os.path.join(MEDIA_DIR, "code_blinn_phong.png"),
                   Inches(6.9), Inches(4.5), Inches(5.6), Inches(2.4),
                   "Ảnh mã nguồn: Blinn-Phong Specular và công thức sương mù Fog")

    s6.notes_slide.notes_text_frame.text = (
        "[LỜI THOẠI BẠN B]: Kính thưa Thầy Cô, sau phần khung xương của Bạn A, em thổi hồn vào không gian bằng hệ thống Shaders. "
        "Em áp dụng mô hình Blinn-Phong trên Fragment Shader với Halfway Vector H, giúp điểm sáng bóng trên vảy cá "
        "khi đón ánh nắng vừa chân thực, vừa tối ưu hiệu năng tính toán. Tất cả sinh vật đều được bọc Texture 2K chân thực, "
        "kết hợp kênh Emission phát sáng cho đèn LED và sứa biển."
    )

    # =========================================================================
    # SLIDE 7: BẠN B - CAUSTICS & SƯƠNG MÙ BIỂN SÂU
    # =========================================================================
    s7 = prs.slides.add_slide(blank_layout)
    set_slide_background(s7)
    add_header(s7, "Hiệu ứng Quang học: Caustics & Sương Mù Biển Sâu", "Quang Học Đại Dương (CG.6)", "BẠN B", COLOR_B)

    add_card(s7, Inches(0.8), Inches(1.8), Inches(5.8), Inches(5.1),
             "Khúc xạ Tia nắng Đáy biển (Caustics) & Fog",
             [
                 "1. Khúc xạ tia nắng mặt trời (Caustics) trên đáy cát:",
                 "   Giao thoa 2 hàm sóng lượng giác theo thời gian:",
                 "   waterUV = vWorldPosition.xz * 1.35",
                 "   rippleA = sin(waterUV.x * 3.1 + uTime * 0.75 + ...)",
                 "   rippleB = cos(waterUV.y * 3.7 - uTime * 0.62 + ...)",
                 "   caustic = 1.0 + 0.65 * pow(max(0.5 + 0.5*A*B, 0.0), 10.0)",
                 "2. Sương mù đại dương sâu (Exponential Ocean Fog):",
                 "   fogFactor = exp(-uFogDensity * distanceToCamera)",
                 "   finalColor = mix(uFogColor, litColor, clamp(fogFactor, 0, 1))",
                 "   Màu biển sâu Deep Marine (#095273) tạo chiều sâu điện ảnh",
                 "3. Tương tác bật/tắt tức thì: Phím F (Fog), Phím C (Caustics)"
             ], border_color=COLOR_B)

    add_image_safe(s7, os.path.join(MEDIA_DIR, "shot_dive.png"),
                   Inches(6.9), Inches(1.8), Inches(5.6), Inches(2.45),
                   "Ảnh giao diện thực tế: Sương mù đại dương sâu và vệt nắng Caustics trên cát")

    add_image_safe(s7, os.path.join(MEDIA_DIR, "shot_fog_comparison.png"),
                   Inches(6.9), Inches(4.5), Inches(5.6), Inches(2.4),
                   "Ảnh thực nghiệm so sánh: Khi bấm phím F tắt sương mù biển sâu")

    s7.notes_slide.notes_text_frame.text = (
        "[LỜI THOẠI BẠN B]: Để tái hiện làn nước biển chân thực, em tích hợp hai hiệu ứng quang học: "
        "Thứ nhất là Khúc xạ tia nắng Caustics trên cát bằng phương trình giao thoa sóng sin, tạo các vệt sáng uốn lượn lấp lánh. "
        "Thứ hai là Sương mù biển sâu Exponential Fog theo hàm mũ e^(-density * d), càng nhìn ra xa nước càng xanh đục huyền ảo. "
        "Người dùng có thể nhấn phím F để bật/tắt sương mù hoặc phím C để tắt bật Caustics trực tiếp khi demo."
    )

    # =========================================================================
    # SLIDE 8: BẠN B - VERTEX SHADER UỐN ĐUÔI CÁ
    # =========================================================================
    s8 = prs.slides.add_slide(blank_layout)
    set_slide_background(s8)
    add_header(s8, "Đồ họa Chuyển động: Vertex Shader Uốn Vây Đuôi", "Biến Dạng Đỉnh GPU (CG.6)", "BẠN B", COLOR_B)

    add_card(s8, Inches(0.8), Inches(1.8), Inches(5.8), Inches(5.1),
             "Biến dạng Hình học Thời gian thực (uSwim)",
             [
                 "1. Biến dạng Đuôi Cá Mập Voi (uSwim = 1.0):",
                 "   - Hàm trọng số khoảng cách từ đầu về đuôi:",
                 "     tailWeight = clamp((-aPosition.x - 0.10) / 1.55, 0.0, 1.0)",
                 "   - Sóng quẫy đuôi ngang trục Z:",
                 "     localPosition.z += sin(uTime * 4.2 + aPosition.x * 3.0) * 0.085 * w",
                 "2. Biến dạng Vẫy cánh Cá Đuối Đốm (uSwim = 4.0):",
                 "   - Hàm trọng số khoảng cách sải cánh ra 2 đầu:",
                 "     wingWeight = clamp((abs(aPosition.z) - 0.10) * 1.7, 0.0, 1.0)",
                 "   - Sóng đập cánh đứng trục Y:",
                 "     localPosition.y += sin(uTime * 2.2 + abs(z) * 3.4) * 0.10 * w",
                 "3. Ưu điểm: GPU xử lý song song 15.000 đỉnh cùng lúc, 0% CPU"
             ], border_color=COLOR_B)

    add_image_safe(s8, os.path.join(MEDIA_DIR, "shot_shark_bezier.png"),
                   Inches(6.9), Inches(1.8), Inches(5.6), Inches(2.45),
                   "Ảnh giao diện thực tế: Đuôi cá mập uốn lượn và cá đuối vỗ cánh")

    add_image_safe(s8, os.path.join(MEDIA_DIR, "code_uswim_shader.png"),
                   Inches(6.9), Inches(4.5), Inches(5.6), Inches(2.4),
                   "Ảnh mã nguồn: Thuật toán uSwim trong Vertex Shader ocean_fog.vs")

    s8.notes_slide.notes_text_frame.text = (
        "[LỜI THOẠI BẠN B]: Thay vì dùng khung xương phức tạp làm nặng máy, em lập trình biến dạng sinh học trực tiếp trên Vertex Shader. "
        "Với Cá Mập Voi, đỉnh càng gần đuôi thì biên độ sóng sin theo trục ngang Z càng lớn tạo cú quẫy đuôi đẩy nước. "
        "Với Cá Đuối, đỉnh càng xa về hai đầu cánh thì biên độ đập cánh theo trục đứng Y càng cao. "
        "Toàn bộ gần 15.000 đỉnh được GPU tính toán song song tức thì, hoàn toàn không tốn CPU."
    )

    # =========================================================================
    # SLIDE 9: BẠN B - TÀI NGUYÊN 3D & HỆ HẠT
    # =========================================================================
    s9 = prs.slides.add_slide(blank_layout)
    set_slide_background(s9)
    add_header(s9, "Pipeline Tài nguyên 3D & Hệ Hạt Bọt Khí (Particles)", "Mô Hình Hóa & Hệ Hạt (CG.2/6)", "BẠN B", COLOR_B)

    add_card(s9, Inches(0.8), Inches(1.8), Inches(5.8), Inches(5.1),
             "Pipeline 3D Đa Định dạng & Hệ Hạt Bọt Khí",
             [
                 "1. Tải mô hình 3D đa định dạng:",
                 "   - TinyOBJ: Cá mập voi, Cá đuối, Cá hề, 3 khối rạn San hô",
                 "   - Khronos cgltf: Cá chẽm Barramundi, Cá nóc Pufferfish, Mahi Mahi, Hải long lá, Cá ngựa, Đàn sứa phát sáng",
                 "2. Hệ hạt Particle System (80 bọt khí nổi li ti):",
                 "   Nổi liên tục từ hốc san hô, dao động tán xạ ngang ngẫu nhiên",
                 "3. Quản lý Z-Buffer & Alpha Blending (Chống rách hình Z-fighting):",
                 "   - Bước 1: Vẽ vật thể đặc với glDepthMask(GL_TRUE)",
                 "   - Bước 2: Bật Alpha Blending (glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA))",
                 "   - Bước 3: Khóa ghi độ sâu glDepthMask(GL_FALSE) vẽ bọt khí & kính"
             ], border_color=COLOR_B)

    add_image_safe(s9, os.path.join(MEDIA_DIR, "shot_corals_bubbles.png"),
                   Inches(6.9), Inches(1.8), Inches(5.6), Inches(2.45),
                   "Ảnh giao diện thực tế: 3 Khối San hô, Cá hề và Hệ hạt bọt khí nổi")

    add_image_safe(s9, os.path.join(MEDIA_DIR, "code_particles_zbuffer.png"),
                   Inches(6.9), Inches(4.5), Inches(5.6), Inches(2.4),
                   "Ảnh mã nguồn: Quản lý thứ tự vẽ Z-Buffer và Alpha Blending")

    s9.notes_slide.notes_text_frame.text = (
        "[LỜI THOẠI BẠN B]: Về tài nguyên, hệ thống hỗ trợ cả OBJ và GLB Binary với 8 loài cá và đàn sứa phát sáng. "
        "Em lập trình một hệ hạt 80 bọt khí oxy nổi liên tục từ san hô. Để các vật thể trong suốt như bọt khí, mặt nước và vòm kính "
        "nhìn xuyên thấu qua nhau không bị lỗi Z-fighting, em tuân thủ nghiêm ngặt quy trình quản lý Z-Buffer: vẽ vật thể đặc trước, "
        "sau đó khóa ghi depth mask rồi mới vẽ vật thể trong suốt. Nhờ đó hình ảnh nhìn qua kính trong veo và sắc nét."
    )

    # =========================================================================
    # SLIDE 10: KỊCH BẢN LIVE DEMO
    # =========================================================================
    s10 = prs.slides.add_slide(blank_layout)
    set_slide_background(s10)
    add_header(s10, "Kịch bản Trình diễn Trực tiếp (Live Demonstration)", "Thực Nghiệm", "PHỐI HỢP CẢ 2 BẠN", ACCENT_BLUE)

    add_card(s10, Inches(0.8), Inches(1.8), Inches(3.7), Inches(5.1),
             "Pha 1: Trải nghiệm Hầm Kính (Bạn A)",
             [
                 "Khởi động chương trình mượt mà 60 FPS",
                 "Bạn A điều khiển W, A, S, D đi dọc hành lang hầm",
                 "Chỉ rõ vòm kính Acrylic, dầm thép và dải đèn LED cyan neon dẫn lối",
                 "Xoay chuột 360° nhìn cá bơi ngoài vòm kính"
             ])

    add_card(s10, Inches(4.8), Inches(1.8), Inches(3.7), Inches(5.1),
             "Pha 2: LERP Xuyên Kính (Bạn A)",
             [
                 "Bạn A bấm phím [SPACE]:",
                 "Camera lướt xuyên qua kính vòm ra biển sâu",
                 "Hầm kính biến mất (100% không kính), FOV mở rộng từ 50° lên 68°",
                 "Trình diễn lặn 6-DOF: E bơi lên mặt nước, Q lặn sâu xuống chạm rạn san hô"
             ])

    add_card(s10, Inches(8.8), Inches(1.8), Inches(3.7), Inches(5.1),
             "Pha 3: Soi Chuyển động & Shaders (Bạn B)",
             [
                 "Bạn B thuyết minh chi tiết:",
                 "  - Cá mập voi uốn đuôi bơi đường cong Bézier 3D",
                 "  - Cá đuối đốm vỗ cánh mềm mại",
                 "  - Đàn sứa phát sáng & 80 bọt khí nổi",
                 "Tương tác Shaders: Bấm F tắt/bật Fog, bấm C tắt/bật Caustics trên cát"
             ])

    s10.notes_slide.notes_text_frame.text = (
        "[LỜI THOẠI PHỐI HỢP DEMO]: "
        "Bạn A: 'Sau đây chúng em xin demo trực tiếp chương trình. Như Thầy Cô thấy, hiện tại em đang đứng trên sàn hành lang hầm kính "
        "với dải đèn LED cyan neon dẫn lối. Nhìn lên trên qua kính vòm trong suốt là đàn cá đang bơi.' "
        "Bạn A: (Bấm phím SPACE) 'Bây giờ em bấm SPACE. Thuật toán LERP đưa camera lướt xuyên qua kính ra lòng biển sâu, "
        "hầm kính biến mất nhường chỗ cho đại dương tự do 6-DOF.' "
        "Bạn B: 'Ở góc nhìn lặn này, Thầy Cô quan sát thấy Cá Mập bơi theo quỹ đạo Bézier của Bạn A kết hợp uốn đuôi shader của em. "
        "Em xin bấm phím F để tắt bật sương mù và phím C để kiểm tra khúc xạ tia nắng Caustics trên đáy cát.'"
    )

    # =========================================================================
    # SLIDE 11: KẾT LUẬN & HỎI ĐÁP
    # =========================================================================
    s11 = prs.slides.add_slide(blank_layout)
    set_slide_background(s11)
    add_header(s11, "Kết luận, Đánh giá & Định hướng Phát triển", "Tổng Kết", "CẢ 2 BẠN", ACCENT_BLUE)

    add_card(s11, Inches(0.8), Inches(1.8), Inches(5.8), Inches(5.1),
             "Kết quả Đạt được & Bài học Kinh nghiệm",
             [
                 "1. Hiện thực hóa trọn vẹn 4 chương trọng tâm ĐHMT:",
                 "   - CG.3: Biến đổi hình học phân cấp & ma trận Model/View/Proj",
                 "   - CG.4: Hệ thống Camera 6-DOF & Thuật toán LERP chuyển cảnh",
                 "   - CG.5: Mặt cong tham số (Vòm kính) & Đường cong Bézier 3D",
                 "   - CG.6: Chiếu sáng Blinn-Phong, Texture 2K, Fog, Caustics, uSwim",
                 "2. Hiệu năng ấn tượng: 60 FPS cố định, hoạt động mượt mà",
                 "3. Phân công ăn khớp 100% giữa hai phân hệ của Bạn A và Bạn B",
                 "4. Hướng mở rộng: Shadow Mapping và thuật toán bầy đàn Boids"
             ])

    add_image_safe(s11, os.path.join(MEDIA_DIR, "shot_dive.png"),
                   Inches(6.9), Inches(1.8), Inches(5.6), Inches(4.8),
                   "Tổng kết Đồ án: Thủy cung 3D Tương tác hoàn chỉnh (Modern OpenGL C++)")

    s11.notes_slide.notes_text_frame.text = (
        "[LỜI THOẠI BẠN B KẾT THÚC]: Kính thưa Thầy Cô, qua đồ án này, nhóm chúng em đã làm chủ trọn vẹn "
        "đường ống đồ họa Modern OpenGL: từ toán học biến đổi, mặt cong tham số, camera tương tác cho đến kỹ thuật "
        "lập trình shader thời gian thực trên GPU. Chúng em xin chân thành cảm ơn Thầy Cô đã tận tình giảng dạy. "
        "Nhóm em rất mong nhận được những góp ý và câu hỏi phản biện của Thầy Cô ạ!"
    )

    output_path = "D:\\Aquarium_OpenGL\\Thuy_Cung_3D_Thuyet_Trinh.pptx"
    prs.save(output_path)
    print(f"[OK] Da tao thanh cong file PowerPoint TONG SANG, CHU TO, ANH DUNG CHIEU: {output_path}")

if __name__ == "__main__":
    create_deck()
