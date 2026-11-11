# 🛑 RULES.md - QUY TẮC VIẾT CODE & NHỮNG ĐIỀU AI TUYỆT ĐỐI KHÔNG ĐƯỢC LÀM

**Dự án:** 3D Tropical Oceanarium Simulation (Mô Phỏng Thủy Cung Nhiệt Đới 3D Tương Tác)  
**Mục đích:** Đảm bảo toàn bộ mã nguồn tuân thủ 100% chuẩn học thuật của học phần Đồ Họa Máy Tính, chất lượng đồ họa đạt chuẩn hiện đại (như hình mẫu thủy cung và mô hình cá Giant Trevally từ Sketchfab), và phân định trách nhiệm rõ ràng cho nhóm 2 người.

---

## 1. ⚠️ NHỮNG ĐIỀU AI TUYỆT ĐỐI KHÔNG ĐƯỢC LÀM (STRICT PROHIBITIONS FOR AI)

Bất kỳ AI nào (hoặc thành viên trong nhóm) khi sinh code (vibe coding) hoặc sửa đổi dự án đều **PHẢI TUÂN THỦ TUYỆT ĐỐI** các điều cấm sau:

### 🚫 ĐIỀU CẤM 1: KHÔNG BAO GIỜ TẠO CÁ DẠNG HÌNH KHỐI TRẺ EM (NO PROCEDURAL TOY PRIMITIVES)
* **Tuyệt đối cấm:** Ghép vài khối cầu `Sphere`, khối nón `Cone`, khối hộp `Box` hoặc các tam giác phẳng dẹt không texture để làm "con cá". Đó là hình đồ chơi mẫu giáo, vi phạm nghiêm trọng yêu cầu đồ án chất lượng cao.
* **Quy chuẩn bắt buộc:** Phải nạp và sử dụng **100% mô hình 3D thực tế** (định dạng `.glb` hoặc `.obj` chất lượng cao từ Sketchfab) với đầy đủ cấu trúc giải phẫu sinh học: vảy ánh bạc kim loại (như cá Giant Trevally), vây lượn sóng mềm mại, mắt tròn, đuôi xẻ chữ V, thân thon dài tự nhiên.

### 🚫 ĐIỀU CẤM 2: KHÔNG ĐỂ MÀU NƯỚC TỐI ĐEN SÌ HOẶC U ÁM (NO MURKY BLACK VOID)
* **Tuyệt đối cấm:** Đặt màu nền `glClearColor(0.0, 0.0, 0.0, 1.0)` hay nền Three.js màu đen kịt như vực thẳm chết chóc.
* **Quy chuẩn bắt buộc:** Thủy cung phải tràn ngập ánh nắng nhiệt đới rực rỡ, sử dụng bảng màu ngọc bích sáng (**Bright Tropical Cyan `#068da9`** và **Sunlit Azure `#38bdf8`**). Luôn có hiệu ứng luồng tia nắng mặt trời (Volumetric God Rays) và lưới sóng nắng (Caustics) chiếu rọi từ trên nóc xuống.

### 🚫 ĐIỀU CẤM 3: KHÔNG DÙNG GIAO DIỆN CHATBOT HOẶC FORM CỦA AI (NO GENERIC AI UI)
* **Tuyệt đối cấm:** Tạo các ô nhập text trò chuyện, nút bấm bo tròn kiểu mạng xã hội hay giao diện AI trợ lý chung chung.
* **Quy chuẩn bắt buộc:** Giao diện là **HUD Kính Thủy Cung Bảo Tàng (Museum Glass HUD)**: Tối giản, thanh lịch, nền kính mờ Glassmorphism, hiển thị trực quan: Chế độ Camera hiện tại, FPS (60 FPS), hướng dẫn phím tắt `[SPACE]`, và các Kiosk thông tin sinh vật vát chéo 45 độ dọc đường hầm.

### 🚫 ĐIỀU CẤM 4: KHÔNG DÙNG CHUYỂN ĐỘNG THẲNG ĐUỘT, GIẬT CỤC (NO LINEAR SNAPPING)
* **Tuyệt đối cấm:** Đổi góc nhìn đột ngột (teleport) làm người xem chóng mặt, hoặc cho cá di chuyển giật khấc theo từng đoạn thẳng rời rạc.
* **Quy chuẩn bắt buộc:** Mọi đường bơi của cá phải bám sát **Đường cong Bézier 3D bậc 3** (slide giáo trình CG.5) mượt mà không góc nhọn; chuyển cảnh camera khi bấm `[SPACE]` phải dùng nội suy **LERP / SmoothStep / GSAP `power2.inOut`** kéo dài 2.5 giây êm ái.

### 🚫 ĐIỀU CẤM 5: KHÔNG DÙNG HÀM CŨ DEPRECATED TRONG OPENGL (NO FIXED-FUNCTION PIPELINE)
* **Tuyệt đối cấm:** Sử dụng các hàm thời OpenGL 1.x/2.x như `glBegin()`, `glEnd()`, `glVertex3f()`, `glMatrixMode()`, `glLoadIdentity()`.
* **Quy chuẩn bắt buộc:** 100% code C++ OpenGL phải viết trên chuẩn **OpenGL 3.3 Core Profile** hiện đại: Sử dụng Vertex Array Object (VAO), Vertex Buffer Object (VBO), Element Buffer Object (EBO) và Shaders GLSL tự viết (`.vs`, `.fs`).

### 🚫 ĐIỀU CẤM 6: KHÔNG SỬA ĐÈ VÀO MÃ NGUỒN CỦA NHAU (NO REPO COLLISION)
* **Quy tắc phân quyền:**
  * **Bạn A sở hữu:** Thư mục toán học & mô hình (`include/Camera.h`, `include/BezierPath.h`, `include/Tunnel.h`, `include/Mesh.h`, `include/Model.h`, `assets/models/`).
  * **Bạn B sở hữu:** Thư mục Shaders & Trình diễn Web/UI (`include/Shader.h`, `include/ParticleSystem.h`, `assets/shaders/`, `index.html`, `run.bat`).
  * **File dùng chung:** `src/main.cpp` chỉ được chỉnh sửa tại hàm `main()` khi ráp nối các module đã được kiểm thử độc lập.

---

## 2. QUY CHUẨN LẬP TRÌNH C++ / OPENGL (C++ CODING CONVENTIONS)

1. **Chuẩn ngôn ngữ:** C++17. Ưu tiên sử dụng `const ref`, `std::vector`, `std::string_view` để tối ưu hiệu năng.
2. **Quy tắc đặt tên:**
   * Tên Lớp (Class / Struct): `PascalCase` (vd: `Camera`, `BezierPath`, `ParticleSystem`).
   * Tên Phương thức / Hàm: `PascalCase` (vd: `GetViewMatrix()`, `Evaluate()`, `Render()`).
   * Tên Biến thành viên (Member variables): Tiền tố `m_` hoặc viết thường `camelCase` (vd: `m_position`, `m_yaw`, `m_pitch`).
   * Hằng số (Constants): Viết hoa toàn bộ với dấu gạch dưới `UPPER_SNAKE_CASE` (vd: `MAX_PARTICLES`, `TUNNEL_RADIUS`).
3. **Chú thích học thuật (Academic Documentation):**
   * Mọi công thức toán học dựng hình (Vòm trụ, Bézier 3D, Ma trận nhìn, Blinn-Phong) phải ghi rõ số hiệu chương giáo trình đồ họa máy tính (vd: `// Áp dụng công thức đường cong tham số Bézier bậc 3 theo slide CG.5`).
4. **Giải phóng tài nguyên (Resource Cleanup):**
   * Mọi VAO, VBO, Shaders và Textures sau khi dùng xong phải gọi lệnh hủy GPU tương ứng (`glDeleteBuffers`, `glDeleteVertexArrays`, `glDeleteProgram`).

---

## 3. QUY CHUẨN LẬP TRÌNH WEBGL / THREE.JS

1. **Nạp mô hình bất đồng bộ an toàn:**
   * Luôn sử dụng `THREE.GLTFLoader` kết hợp `THREE.LoadingManager` để theo dõi tiến độ nạp (Progress Callback).
   * Kiểm tra `gltf.scene` tồn tại trước khi thêm vào `scene.add()`, tự động tính toán lại ma trận bóng đổ `castShadow = true` và `receiveShadow = true` cho từng child mesh.
2. **Tối ưu vòng lặp Render (Performance Optimization):**
   * Không khởi tạo đối tượng mới (`new THREE.Vector3()`, `new THREE.Matrix4()`) bên trong hàm `animate()` để tránh kích hoạt trình gom rác (Garbage Collection) gây tụt khung hình.
   * Sử dụng các biến tạm tái sử dụng (Scratch variables) bên ngoài vòng lặp.
3. **Chạy qua Local Web Server:**
   * Tuyệt đối không mở file trực tiếp qua giao thức `file://` (sẽ bị trình duyệt chặn CORS khi nạp file mô hình `.glb`).
   * Luôn khởi chạy ứng dụng bằng script `run.bat` (sử dụng Python `http.server 8080`) để đảm bảo nạp tài nguyên 100% trơn tru.
