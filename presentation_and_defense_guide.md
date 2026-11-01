# 🎓 HƯỚNG DẪN THUYẾT TRÌNH & BẢO VỆ ĐỒ ÁN: MÔ PHỎNG THỦY CUNG 3D TƯƠNG TÁC
**Học phần:** Đồ họa máy tính (Computer Graphics)  
**Nhóm:** 2 sinh viên (Bạn A & Bạn B)  
**Công nghệ:** C++ / Modern OpenGL (GLAD, GLFW, GLM, stb_image) & WebGL 3D (Three.js / GSAP)

---

## 📑 MỤC LỤC
1. [Kịch Bản Thuyết Trình 5 Phút Chuẩn Mực](#1-kịch-bản-thuyết-trình-5-phút-chuẩn-mực)
2. [Bộ Câu Hỏi Vấn Đáp Dành Cho Bạn A (Backend / Toán & Camera)](#2-bộ-câu-hỏi-vấn-đáp-dành-cho-bạn-a)
3. [Bộ Câu Hỏi Vấn Đáp Dành Cho Bạn B (Frontend / Shaders & Vật Liệu)](#3-bộ-câu-hỏi-vấn-đáp-dành-cho-bạn-b)
4. [Ánh Xạ Chuẩn Lý Thuyết Từng Chương Vào Mã Nguồn](#4-ánh-xạ-chuẩn-lý-thuyết-từng-chương-vào-mã-nguồn)
5. [Hướng Dẫn Thao Tác Chạy Demo Trực Tiếp](#5-hướng-dẫn-thao-tác-chạy-demo-trực-tiếp)

---

## 1. KỊCH BẢN THUYẾT TRÌNH 5 PHÚT CHUẨN MỰC

* **Phút 0 - 1: Giới thiệu & Triết lý dự án (Cả 2 bạn)**
  * "Kính thưa Thầy/Cô, nhóm em phát triển đề tài **Mô phỏng Thủy cung 3D Tương tác** dựa trên kiến trúc phân tách rõ nét giữa **Backend Toán học - Không gian** và **Frontend Shaders - Vật liệu đồ họa**. Sản phẩm mang lại trải nghiệm đột phá kép: Đi dạo trong hầm kính và bấm phím **[SPACE]** để lướt xuyên kính hòa mình 100% vào làn nước đại dương cùng đàn cá khổng lồ."
* **Phút 1 - 2.5: Bạn A trình bày Backend Toán Học**
  * Trình chiếu thuật toán sinh vòm kính tham số hình trụ nửa đường tròn ($x = R\cos\theta, y = R\sin\theta$).
  * Trình chiếu công thức giải tích Bézier bậc 3 ($B(t)$) và vector tiếp tuyến đạo hàm bậc 1 ($B'(t)$) để dẫn đường và xoay đầu cá tự nhiên.
  * Trình chiếu cơ chế nội suy Camera LERP chuyển đổi mượt mà giữa hầm và biển.
* **Phút 2.5 - 4: Bạn B trình bày Shaders, Vật Liệu & Hiệu Ứng**
  * Giải thích Shader sương mù nước biển hàm mũ $\text{fogFactor} = e^{-\text{density} \times d}$ hòa trộn vào màu Deep Marine `#041C2C`.
  * Trình chiếu mô hình chiếu sáng Blinn-Phong và gợn sóng khúc xạ tia nắng Caustics.
  * Trình chiếu hệ hạt bọt khí nổi li ti và kỹ thuật Alpha Blending kính trong suốt.
* **Phút 4 - 5: Demo tương tác và kết luận (Cả 2 bạn)**
  * Chạy file `Aquarium_3D.exe` (hoặc mở `aquarium_tunnel_walkthrough.html`), dùng WASD đi bộ, nhấn SPACE lướt xuyên kính, xoay chuột 360 độ ngắm cá bơi.

---

## 2. BỘ CÂU HỎI VẤN ĐÁP DÀNH CHO BẠN A

### ❓ Câu 1: Em hãy viết phương trình toán học dùng để sinh lưới đỉnh vòm kính hầm?
* **Trả lời:**
  * Vòm kính hầm thủy cung là một mặt cong tham số (Parametric Surface - Chương CG.5) hình nửa mặt trụ tròn nằm dọc trục $Z$.
  * Phương trình tham số:
    $$\begin{cases} x = R \cdot \cos\theta \\ y = R \cdot \sin\theta \\ z \in [-L/2, L/2] \end{cases}$$
  * Trong đó:
    * $R = 160$ là bán kính vòm kính.
    * $\theta \in [0, \pi]$ quét từ mép sàn bên này qua đỉnh vòm sang mép sàn bên kia.
    * $z$ chạy dọc theo chiều dài đường hầm $L = 1500$.
  * Cứ mỗi ô lưới gồm 4 đỉnh $(v_1, v_2, v_3, v_4)$, nhóm em chia thành 2 tam giác để nạp vào VBO vẽ bằng `GL_TRIANGLES`.
  * Vector pháp tuyến (Normal) tại mỗi đỉnh được tính giải tích: $\vec{N} = \text{normalize}(x, y, 0)$ hướng ra ngoài mặt cong.

### ❓ Câu 2: Thuật toán Bézier 3D dẫn đường cho cá bơi được tính như thế nào? Làm sao xoay đầu cá theo hướng bơi?
* **Trả lời:**
  * Nhóm em sử dụng **đường cong Bézier bậc 3** (Cubic Bézier Curve - Chương CG.5) xác định bởi 4 điểm kiểm soát $P_0, P_1, P_2, P_3$.
  * Công thức giải tích vị trí:
    $$B(t) = (1-t)^3 P_0 + 3(1-t)^2 t P_1 + 3(1-t) t^2 P_2 + t^3 P_3 \quad (t \in [0, 1])$$
  * Để cá không bị bơi thụt lùi hay xoay ngang, nhóm lấy **đạo hàm bậc 1** của hàm Bézier để tìm vector tiếp tuyến $\vec{T}(t) = B'(t)$:
    $$B'(t) = 3(1-t)^2(P_1 - P_0) + 6(1-t)t(P_2 - P_1) + 3t^2(P_3 - P_2)$$
  * Góc xoay đầu cá trên mặt phẳng nằm ngang (Yaw) được tính bằng hàm hàm lượng giác:
    $$\text{angle} = \text{atan2}(T_x, T_z)$$
  * Nhờ đó, tại bất kỳ thời điểm $t$ nào, đầu cá luôn tự động hướng chuẩn xác theo tiếp tuyến quỹ đạo bơi.

### ❓ Câu 3: Thuật toán chuyển cảnh "Xuyên Kính" khi nhấn phím SPACE hoạt động ra sao?
* **Trả lời:**
  * Nhóm áp dụng phép **nội suy tuyến tính LERP** (Linear Interpolation - Chương CG.4):
    $$P(t) = (1 - \alpha) P_{\text{hiện tại}} + \alpha P_{\text{đích}}$$
  * Trong mã nguồn OpenGL:
    ```cpp
    Position = glm::mix(Position, targetPos, deltaTime * 3.5f);
    Fov = glm::mix(Fov, targetFov, deltaTime * 3.5f);
    ```
  * Khi bấm `[SPACE]`, cờ `isDiving` được bật:
    * `targetPos` được đẩy từ trong lòng hầm ra tọa độ ngoài nước $(x=240, y=-60)$.
    * Góc mở ống kính `Fov` mở rộng từ $60^\circ \to 85^\circ$ tạo chiều sâu đại dương điện ảnh.
    * Trong `main.cpp`, khi `isDiving == true`, hệ thống ngắt lệnh vẽ `glassTunnel.Draw()`, khiến vách kính và sàn hầm biến mất 100%, người dùng chuyển sang trạng thái bơi lội tự do 6 hướng.

---

## 3. BỘ CÂU HỎI VẤN ĐÁP DÀNH CHO BẠN B

### ❓ Câu 1: Em hãy trình bày mô hình chiếu sáng Blinn-Phong trong Shader của dự án?
* **Trả lời:**
  * Trong file `assets/shaders/ocean_fog.fs`, nhóm áp dụng mô hình chiếu sáng **Blinn-Phong** (Chương CG.6) gồm 3 thành phần:
    1. **Ambient (Môi trường):** $I_a = k_a \times \text{lightColor}$ (hệ số 0.28 mô phỏng ánh sáng tán xạ trong nước).
    2. **Diffuse (Khuếch tán Lambert):** $I_d = k_d \times \max(\vec{N} \cdot \vec{L}, 0.0) \times \text{lightColor}$, nguồn sáng đặt tại đỉnh mặt nước $(0, 600, 0)$ chiếu xuống.
    3. **Specular (Phản xạ gương Blinn-Phong):** Khác với Phong cổ điển dùng vector phản xạ $\vec{R}$, Blinn-Phong sử dụng **vector nửa đường (Halfway vector)**:
       $$\vec{H} = \frac{\vec{L} + \vec{V}}{\|\vec{L} + \vec{V}\|}, \quad I_s = k_s \times (\max(\vec{N} \cdot \vec{H}, 0.0))^{32}$$
       Cách này tính toán nhanh hơn và cho đốm sáng bóng mượt tự nhiên trên da cá và vách kính.

### ❓ Câu 2: Công thức sương mù nước biển sâu (Underwater Fog) được cài đặt như thế nào?
* **Trả lời:**
  * Nhóm cài đặt mô hình **Sương mù hàm mũ (Exponential Fog - Chương CG.6)**:
    $$\text{fogFactor} = e^{-\text{density} \times \text{distance}}$$
  * Trong đó:
    * `distance` là khoảng cách từ camera đến điểm ảnh: `length(viewPos - FragPos)`.
    * `density = 0.0016` là mật độ hạt lơ lửng trong nước biển.
  * Sau đó, màu cuối cùng được nội suy giữa màu gốc và màu nước xanh thẳm Deep Marine (`#041C2C`):
    ```glsl
    vec3 deepOceanColor = vec3(0.016, 0.110, 0.173);
    vec3 finalColor = mix(deepOceanColor, lightedColor, fogFactor);
    ```
  * Nhờ công thức này, vật thể càng ở xa sẽ càng chìm dần vào làn nước xanh huyền ảo.

### ❓ Câu 3: Kỹ thuật Alpha Blending cho vòm kính trong suốt và bọt khí đòi hỏi quy tắc vẽ gì đặc biệt?
* **Trả lời:**
  * Khi xử lý độ trong suốt (Alpha Blending - Chương CG.6), OpenGL sử dụng hàm:
    ```cpp
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    ```
  * **Quy tắc vàng về thứ tự vẽ (Draw Order):**
    * Phải vẽ tất cả các vật thể **đặc (Opaque)** trước: Cá mập, cá đuối, san hô, sàn hầm.
    * Sau đó mới vẽ các vật thể **trong suốt (Transparent)** sau cùng: Vòm kính Acrylic và bọt khí.
    * Khi vẽ vòm kính, ta tạm thời tắt ghi Z-Buffer bằng `glDepthMask(GL_FALSE)` rồi khôi phục `glDepthMask(GL_TRUE)` để tránh việc kính trong suốt ghi đè độ sâu làm che mất các vật thể bơi phía sau.

---

## 4. ÁNH XẠ CHUẨN LÝ THUYẾT TỪNG CHƯƠNG VÀO MÃ NGUỒN

| Chương Slide | Lý Thuyết Trọng Tâm | Tệp Mã Nguồn Cài Đặt |
| :--- | :--- | :--- |
| **CG.2** | Render Loop, Buffer VAO/VBO/EBO, Shader Compile | `Shader.h`, `Mesh.h`, `main.cpp` |
| **CG.3** | Phép biến đổi ma trận (TRS), Ma trận phân cấp vẫy đuôi cá | `BezierPath.h`, `ocean_fog.vs`, `main.cpp` |
| **CG.4** | Camera LookAt, Perspective Projection, Khử mặt khuất Z-Buffer, LERP | `Camera.h`, `main.cpp` |
| **CG.5** | Đường cong Bézier bậc 3 giải tích, Mặt cong tham số hình trụ nửa đường tròn | `BezierPath.h`, `Tunnel.h` |
| **CG.6** | Chiếu sáng Blinn-Phong, Texture Mapping, Alpha Blending kính, Underwater Fog, Particle System | `ocean_fog.fs`, `Model.h`, `ParticleSystem.h` |

---

## 5. HƯỚNG DẪN THAO TÁC CHẠY DEMO TRỰC TIẾP

### Cách 1: Chạy ứng dụng C++ OpenGL Desktop
1. Nhấp đúp vào file `run.bat` (hoặc mở PowerShell gõ `.\Aquarium_3D.exe`).
2. Nếu cần biên dịch lại từ mã nguồn: nhấp đúp vào `build_mingw.bat`.
3. Phím điều khiển:
   * **W, A, S, D:** Bước đi trong hầm kính.
   * **Chuột:** Quay góc nhìn 360°.
   * **SPACE:** Lướt xuyên kính ra ngoài lòng nước (100% không kính), hoặc bấm lại để quay về hầm.
   * **E / Q:** Ngoi lên / Lặn xuống (khi đang ở chế độ lặn đại dương).
   * **ESC:** Thoát chương trình.

### Cách 2: Chạy bản mô phỏng WebGL trực tiếp trên trình duyệt
1. Nhấp đúp mở file `aquarium_tunnel_walkthrough.html` (hoặc `index.html`) trên bất kỳ trình duyệt nào (Chrome, Edge, Firefox).
2. Trải nghiệm đồ họa 3D Three.js với âm thanh vòm Web Audio, các hiệu ứng Caustics, sương mù đại dương và bảng điều khiển Museum HUD đầy đủ.

