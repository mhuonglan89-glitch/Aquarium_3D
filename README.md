# 🌊 ĐỒ ÁN ĐỒ HỌA MÁY TÍNH: MÔ PHỎNG THỦY CUNG 3D TƯƠNG TÁC
**Học phần:** Đồ họa máy tính (Computer Graphics)  
**Quy mô:** Nhóm 2 sinh viên (Bạn A & Bạn B)  
**Nền tảng công nghệ:** C++11 / Modern OpenGL (GLAD, GLFW 3.4, GLM, stb_image) & WebGL 3D (Three.js / GSAP)  
**Thư mục dự án:** `C:\Exercise\DHMT\Aquarium_3D`

---

## 🚀 HƯỚNG DẪN KHỞI CHẠY NHANH (QUICK START)

### 1. Chạy Ứng Dụng C++ OpenGL Desktop (.exe)
* **Cách nhanh nhất:** Nhấp đúp chuột vào file `run.bat` (hoặc mở Terminal gõ `.\Aquarium_3D.exe`).
* **Biên dịch lại từ mã nguồn:** Nhấp đúp vào `build_mingw.bat` (sử dụng trình biên dịch MinGW g++ 64-bit đi kèm dự án).

### 2. Chạy Bản Mô Phỏng WebGL 3D Trực Tiếp Trên Trình Duyệt
* Nhấp đúp mở file `aquarium_tunnel_walkthrough.html` hoặc `index.html` trên bất kỳ trình duyệt nào (Chrome, Edge, Firefox).
* Chạy mượt mà 60 - 120 FPS, hỗ trợ cả khi **offline không có mạng** (đã tích hợp sẵn thư viện trong `assets/js/`).

---

## 🎮 KỊCH BẢN ĐIỀU KHIỂN & TRẢI NGHIỆM ĐỘT PHÁ

* **🚶 Chế độ 1: Khách Tham Quan Đường Hầm Vòm Kính (Tunnel Walkthrough)**
  * **W, A, S, D:** Bước đi thong thả dọc hành lang hầm kính (được khóa cao độ tự nhiên trên mặt sàn).
  * **Chuột:** Quay tự do 360° ngắm nhìn đàn cá bơi lội qua vòm kính trong suốt có dải LED xanh và dầm thép chịu lực.
* **🌊 Chế độ 2: Xuyên Kính Hòa Mình Vào Lòng Biển (100% Không Kính)**
  * **Phím [SPACE]:** Kích hoạt camera LERP lướt mượt mà tiến về vách kính và xuyên thẳng ra ngoài nước.
  * **Trải nghiệm mới:** Toàn bộ khung hầm và kính biến mất 100%! Người dùng lơ lửng tự do giữa làn nước xanh ngọc bích, bọt khí nổi li ti, tia nắng mặt trời rọi trực tiếp, bơi lội sát bên cá mập voi khổng lồ.
  * **Phím E / Q:** Bơi ngoi lên / Lặn sâu xuống (khi ở chế độ lặn đại dương).
  * **Phím [SPACE] lần nữa:** Camera lùi êm ái xuyên qua mặt kính trở lại sàn đi bộ trong đường hầm.
* **ESC:** Thoát chương trình.

---

## 👥 PHÂN CHIA TRÁCH NHIỆM CHO 2 THÀNH VIÊN

| Thành Viên | Vai Trò | Tệp Tin Phụ Trách | Trọng Tâm Kiến Thức Đồ Họa |
| :--- | :--- | :--- | :--- |
| **🧑‍💻 BẠN A** | Kỹ sư Engine & Toán Học Không Gian (Backend) | `include/Camera.h`<br>`include/Tunnel.h`<br>`include/BezierPath.h` | • Tham số hóa nửa hình trụ sinh vòm kính ($x=R\cos\theta, y=R\sin\theta$)<br>• Đường cong Bézier 3D bậc 3 và đạo hàm tiếp tuyến $\text{atan2}$ xoay đầu cá<br>• Camera 6-DOF và thuật toán nội suy LERP chuyển cảnh `glm::mix` |
| **🧑‍💻 BẠN B** | Kỹ sư Vật Liệu, Shaders & Tài Nguyên 3D (Frontend) | `include/Shader.h`<br>`include/Mesh.h`<br>`include/Model.h`<br>`include/ParticleSystem.h`<br>`assets/shaders/ocean_fog.*` | • Mô hình chiếu sáng Blinn-Phong và tia nắng nước Caustics<br>• Sương mù đại dương hàm mũ (Exponential Fog: $\text{fogFactor} = e^{-\text{density} \times d}$)<br>• Nạp mô hình 3D (.obj) và Texture 2K thật từ Sketchfab<br>• Hệ hạt bọt khí nổi (Particle System) & Alpha Blending kính |

---

## 📚 ÁNH XẠ CHUẨN 100% KIẾN THỨC CÁC CHƯƠNG HỌC

1. **Chương CG.2 (Programming CG in OpenGL):**
   * Khởi tạo Window và quản lý vòng lặp Render 60 FPS bằng GLFW.
   * Nạp hàm mở rộng OpenGL Core Profile 3.3 bằng GLAD (`gladLoadGLLoader`).
   * Quản lý mảng đỉnh đối tượng bằng VAO, VBO, EBO (`Mesh.h`, `Tunnel.h`, `ParticleSystem.h`).
   * Đọc, biên dịch và bắt lỗi Shader GLSL thời gian thực (`Shader.h`).
2. **Chương CG.3 & CG.4 (3D Graphics & Transformations):**
   * Phép biến đổi ma trận phân cấp (Hierarchy): thân cá kết hợp vây đuôi dao động $\sin(\omega t)$.
   * Hệ thống Camera LookAt 3D (`glm::lookAt`) kết hợp phép chiếu phối cảnh (`glm::perspective`).
   * Bật Z-Buffer khử mặt khuất (`GL_DEPTH_TEST`).
   * Thuật toán nội suy LERP: $P(t) = (1-t)P_1 + t P_2$ cho chuyển động xuyên kính.
3. **Chương CG.5 (Curves and Surfaces):**
   * Đường cong Bézier 3D bậc 3 giải tích: $B(t) = (1-t)^3 P_0 + 3(1-t)^2 t P_1 + 3(1-t) t^2 P_2 + t^3 P_3$.
   * Đạo hàm bậc 1 tìm vector tiếp tuyến $\vec{T}(t) = B'(t)$ để quay đầu cá theo tiếp tuyến quỹ đạo bơi.
   * Phương trình mặt cong tham số hình trụ nửa đường tròn sinh lưới đỉnh vòm kính hầm.
4. **Chương CG.6 (Advanced Graphics Techniques):**
   * Mô hình chiếu sáng Blinn-Phong: Ambient, Diffuse (Lambert), Specular (Halfway vector $\vec{H}$).
   * Dán vân bề mặt (Texture Mapping): Nạp texture da cá mập, cá đuối, rạn san hô (`stb_image.h`).
   * Kỹ thuật trong suốt (Alpha Blending): `glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)`.
   * Underwater Exponential Fog: $\text{fogFactor} = e^{-\text{density} \times d}$ hòa trộn cùng màu Deep Marine `#041C2C`.
   * Hệ hạt (Particle System): 100 hạt bọt khí bay từ đáy lên mặt nước.

---

## 🎨 BẢNG MÃ MÀU HỆ THỐNG (UNIFIED DESIGN SYSTEM)

* **Abyss Void (`#010A14`):** Nền biển sâu `vec3(0.004, 0.039, 0.078)`.
* **Deep Marine (`#041C2C`):** Màu sương mù nước biển `vec3(0.016, 0.110, 0.173)`.
* **Bioluminescent Cyan (`#0CE8DA`):** Tia sáng Caustics & dải LED dẫn lối `vec3(0.047, 0.910, 0.855)`.
* **Sunlit Azure (`#38BDF8`):** Vệt lóa vòm kính & phản xạ lưng cá `vec3(0.220, 0.741, 0.973)`.
* **Coral Sunset (`#F43F5E`):** Cụm rạn san hô điểm nhấn `vec3(0.957, 0.247, 0.369)`.
* **Titanium Steel (`#1E293B`):** Dầm thép chịu lực và sàn gạch hầm `vec3(0.118, 0.161, 0.231)`.

---

## 📖 BẢO VỆ ĐỒ ÁN & THUYẾT TRÌNH

Vui lòng tham khảo tệp **`presentation_and_defense_guide.md`** để xem:
* Kịch bản thuyết trình mẫu 5 phút ấn tượng nhất.
* Trọn bộ câu hỏi vấn đáp lý thuyết & mã nguồn dành riêng cho Bạn A và Bạn B.
* Bảng tra cứu "Bắt bệnh lỗi nhanh" khi chạy thực tế.
