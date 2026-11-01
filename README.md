# 🌊 DỰ ÁN ĐỒ HỌA MÁY TÍNH: MÔ PHỎNG THỦY CUNG 3D TƯƠNG TÁC
**Vị trí dự án:** `C:\Exercise\DHMT\Aquarium_3D`  
**Học phần:** Đồ họa máy tính (Computer Graphics)  
**Nhóm:** 2 thành viên (Bạn A & Bạn B)

---

## 👥 PHÂN CHIA TRÁCH NHIỆM

* **🧑‍💻 BẠN A (Backend / Math & Camera):**
  * `include/Camera.h`: Camera 6 bậc tự do (WASD + Chuột), thuật toán LERP chuyển cảnh mượt mà khi bấm `[SPACE]`.
  * `include/Tunnel.h`: Hàm toán học tham số sinh vòm kính hình trụ bán nguyệt $x = R\cos\theta, y = R\sin\theta$ và sàn hầm.
  * `include/BezierPath.h`: Thuật toán đường cong Bézier 3D bậc 3 giải tích tính quỹ đạo cá bơi và vector tiếp tuyến xoay đầu cá.

* **🧑‍💻 BẠN B (Frontend / Assets & Shaders):**
  * `include/Shader.h`: Class nạp, biên dịch và kiểm soát lỗi Shader GLSL trên GPU.
  * `assets/shaders/ocean_fog.vs` & `assets/shaders/ocean_fog.fs`: Vertex & Fragment Shader chiếu sáng Blinn-Phong và sương mù nước biển sâu (Underwater Exponential Fog).
  * `include/Mesh.h` & `include/Model.h`: Nạp mô hình 3D thực tế (.obj) và Texture từ Sketchfab.
  * `include/ParticleSystem.h`: Hệ hạt bọt khí nổi lên từ đáy biển (Alpha Blending).

* **🤝 GHÉP NỐI CHUNG:**
  * `src/main.cpp`: Tích hợp toàn bộ hệ thống vào vòng lặp Render 60 FPS.
  * `index.html`: Bản chạy trực tiếp trên trình duyệt WebGL chuẩn Three.js / GSAP Motion Design để demo ngay mà không cần cài đặt compiler phức tạp!
