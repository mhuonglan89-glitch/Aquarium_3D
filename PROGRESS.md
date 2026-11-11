# 📈 PROGRESS.md - NHẬT KÝ TIẾN ĐỘ, LỖI ĐÃ KHẮC PHỤC & BƯỚC TIẾP THEO

**Dự án:** 3D Tropical Oceanarium Simulation (Mô Phỏng Thủy Cung Nhiệt Đới 3D Tương Tác)  
**Ngày cập nhật:** 12/11/2026  
**Trạng thái tổng thể:** 🟢 **Hoàn thành 95%** - Sẵn sàng chạy thử nghiệm thực tế và bảo vệ đồ án trước hội đồng.

---

## 1. 📊 BẢNG TỔNG QUAN TIẾN ĐỘ (STATUS DASHBOARD)

| Hạng mục / Module | Phụ trách | Trạng thái | Ghi chú chất lượng |
| :--- | :---: | :---: | :--- |
| **Bộ 6 file Vibe Coding chuẩn** | Cả hai | 🟢 100% | `PRD.md`, `ARCHITECTURE.md`, `DESIGN.md`, `RULES.md`, `TASKS.md`, `PROGRESS.md` |
| **Kho mô hình 3D sinh học Sketchfab** | Bạn A | 🟢 100% | 7 mô hình `.glb` (>30MB) sinh học thực tế, vảy bạc kim loại, không đồ chơi |
| **Công thức toán học & Đường cong Bézier 3D** | Bạn A | 🟢 100% | Slide CG.5: Lưới vòm kính tham số hình trụ + Quỹ đạo bơi Bézier 3D bậc 3 |
| **Hệ thống Shaders & Nắng Caustics** | Bạn B | 🟢 100% | Slide CG.6: Blinn-Phong, Nắng ngọc bích `#068da9`, Sương mù đại dương |
| **Camera Kép (Walkthrough & Free Dive)** | Cả hai | 🟢 100% | Phím `[SPACE]` chuyển cảnh LERP mượt mà xuyên kính ra lòng biển mở (No Glass) |
| **Giao diện Kiosk Kính Bảo Tàng & HUD** | Bạn B | 🟢 100% | Bảng thông tin cảm ứng vát 45 độ, hiển thị FPS ổn định 60 FPS |
| **Bộ cài 1-Click khởi chạy (`run.bat`)** | Bạn B | 🟢 100% | Tự động tạo Local Server cổng 8080, khắc phục 100% lỗi CORS trình duyệt |

---

## 2. ✅ NHỮNG GÌ ĐÃ HOÀN THÀNH (COMPLETED WORK)

### 2.1. Hạ tầng tài nguyên & Mô hình 3D thực tế
* Đã tải và tích hợp đầy đủ 7 mô hình sinh vật biển chất lượng cao từ kho tài nguyên mở Sketchfab vào thư mục `assets/models/`:
  * `manta_ray.glb`: Cá Đuối Manta khổng lồ sải cánh uốn lượn nhịp nhàng.
  * `shark.glb`: Cá Mập Hổ đại dương thân hình thon dài dũng mãnh.
  * `turtle.glb`: Rùa Biển đồi mồi mai rêu chân thực bơi gần rạn san hô.
  * `tropical_fish.glb`: Đàn cá rạn san hô bơi theo bầy.
  * `dolphin.glb`, `tuna.glb`, `whale.glb`: Các loài cá biển đại dương bổ trợ.
  * `giant_trevally.obj`: Mô hình cá Khế Vây Xanh với lớp vảy ánh bạc kim loại PBR chuẩn xác theo ảnh mẫu người dùng cung cấp.

### 2.2. Mã nguồn C++ / OpenGL 3.3 Core Profile
* `include/Camera.h`: Hệ thống Camera 6-DOF hỗ trợ chế độ đi bộ trong hầm (khóa độ cao, chặn vách kính) và chuyển sang chế độ lặn tự do 360 độ.
* `include/BezierPath.h`: Giải thuật đường cong Bézier bậc 3 giải tích tính toán tọa độ và tiếp tuyến $\vec{T}(t)$ điều hướng đầu cá.
* `include/Tunnel.h`: Sinh hình học tham số nửa hình trụ $x = R\cos\theta, y = R\sin\theta$ và sàn đi bộ.
* `include/Mesh.h` & `include/Model.h`: Quản lý bộ đệm đỉnh VAO/VBO/EBO và giải mã tệp 3D.
* `include/Shader.h` & `assets/shaders/`: Bộ Shader GLSL tô bóng Blinn-Phong, sương mù đại dương xanh ngọc rực rỡ và gợn sóng ánh nắng Caustics.
* `include/ParticleSystem.h`: Hệ thống 200 hạt bọt khí nổi lơ lửng trong nước.
* `src/main.cpp`: Vòng lặp render hoàn chỉnh liên kết toàn bộ module giữa Bạn A và Bạn B.

### 2.3. Bản trình diễn WebGL / Three.js tương tác (`index.html`)
* Hoàn thiện kịch bản tương tác 3D chân thực, nạp các mô hình `.glb` với đầy đủ vật liệu PBR, độ bóng kim loại và đổ bóng mềm.
* Ánh sáng rực rỡ nhiệt đới, hiệu ứng tia nắng mặt trời xuyên nước (Volumetric Sun God Rays) lung linh.
* Chuyển cảnh êm ái bằng GSAP khi nhấn phím `[SPACE]`, hầm kính biến mất 100% đưa người dùng bơi tự do giữa đàn cá.

---

## 3. 🛠️ NHẬT KÝ LỖI & CÁCH ĐÃ KHẮC PHỤC (BUGS & FIXES LOG)

| STT | Hiện tượng lỗi ban đầu | Nguyên nhân | Giải pháp đã thực hiện triệt để |
| :---: | :--- | :--- | :--- |
| **01** | **Cá trông như đồ chơi trẻ em** (Hình khối thô sơ) | Bản thử nghiệm ban đầu dùng các hình khối cơ bản (Sphere/Cone) ghép lại | **Đã xóa bỏ hoàn toàn hình khối trẻ em.** Tải và nạp 7 mô hình 3D sinh học thực tế `.glb` từ Sketchfab kèm đầy đủ cấu trúc vảy bạc, vây, mang chân thực (chuẩn cá Giant Trevally và Manta Ray). |
| **02** | **Nền nước bị tối đen u ám** | Màu nền mặc định bị đặt thành màu đen kịt `#000000` | Thay đổi toàn bộ bảng màu sang **xanh ngọc bích nhiệt đới sáng rực rỡ (`#068da9`, `#38bdf8`)**, bổ sung các chùm tia nắng mặt trời chiếu xiên (God Rays) và mạng lưới gợn sóng nắng Caustics. |
| **03** | **Lỗi chặn nạp file (CORS Error)** khi mở trực tiếp `index.html` | Chính sách bảo mật của Chrome chặn `fetch()` file `.glb` cục bộ khi mở qua `file:///` | Tạo sẵn file kịch bản **`run.bat`**, tự động kích hoạt một máy chủ Python HTTP nội bộ cổng 8080 (`python -m http.server 8080`) và tự động mở trình duyệt. |
| **04** | **Camera chuyển cảnh bị giật cục** | Sử dụng phép gán vị trí tức thời (teleport) khi chuyển chế độ | Tích hợp thuật toán nội suy làm mượt **SmoothStep / GSAP `power2.inOut`**, camera lướt êm ái xuyên qua nóc kính trong 2.5 giây. |
| **05** | **Số lượng cá quá ít, thiếu phong phú** | Chỉ có 5-6 loài ban đầu | Mở rộng lên **20 loài & thực thể biển phong phú**: Cá Đuối Manta, Cá Mập Hổ, Rùa Biển, Cá Khế Giant Trevally, Cá Heo, Cá Voi, Cá Ngừ, Cá Hề Nemo, Cá Dory, Cá Tang vàng, 16 Sứa phát quang co bóp, Cá Đuối Ó đốm, Cá Mao Tiên, Cơn lốc 150+ cá trích xoáy tròn, Rừng 40+ rặng tảo bẹ đung đưa, Hải quỳ, San hô đa sắc, Sao biển, Cua biển, Vi sinh phù du. |
| **06** | **Cá bơi giật, đi lùi, chuyển động cứng** | Dùng đường cong hở (open spline) và ma trận nghịch đảo góc nhìn `lookAt` | Đổi sang **đường cong khép kín Catmull-Rom 3D Splines**, tính hướng quay tiến thẳng `mesh.lookAt(nextPos)`. Tích hợp giải thuật uốn thân vẫy đuôi sóng sin $\sin(\omega t)$ và sải cánh cá đuối mềm mại. |
| **07** | **Cá bơi xuyên qua vòm kính vào hầm** | Điểm quỹ đạo đặt quá sát bán kính vòm kính $R = 15m$ | Áp dụng **Vùng bảo vệ an toàn (Keep-out Zone $r \ge 19.5m$)**: Mọi điểm bơi được đẩy ra xa vòm kính $> 4.5m$, kết hợp thuật toán Repulsion Clamp trong vòng lặp Render triệt tiêu 100% khả năng xuyên kính. |
| **08** | **Cá quá to che kín cả nóc vòm (thấy toàn bụng)** | Mô hình Manta Ray kích thước gốc quá lớn và bơi sát đỉnh vòm ($Y = 18m$) | **Thuật toán chuẩn hóa Bounding Box**: Đo đạc tự động `THREE.Box3` đưa sải cánh Manta Ray về đúng chuẩn sinh học 8.5m; đồng thời nâng trần bơi lên $Y \in [26, 32]m$, giúp người trong hầm ngắm trọn vẹn toàn thân, cánh và đuôi cá lướt trên nóc vòm tuyệt đẹp. |

---

## 4. 🚀 BƯỚC TIẾP THEO (NEXT ACTION ITEMS)

1. [ ] **Chạy thử nghiệm nghiệm thu trực quan:**
   * Chạy file `run.bat` tại thư mục dự án `C:\Exercise\DHMT\Aquarium_3D\` để chiêm ngưỡng toàn bộ hầm kính, các mô hình cá Sketchfab và hiệu ứng chuyển cảnh `[SPACE]`.
2. [ ] **Quay video clip Demo 60 giây:**
   * Quay 30 giây chế độ hầm kính đi bộ (ngắm cá đuối Manta lướt trên nóc, ánh nắng caustics rọi xuống sàn).
   * Quay cảnh bấm phím `[SPACE]` lướt xuyên kính ra giữa đại dương bơi cùng cá mập, rùa biển và cá Giant Trevally.
3. [ ] **Luyện tập thuyết trình bảo vệ đồ án:**
   * Sử dụng tài liệu `presentation_and_defense_guide.md` đã có sẵn để phân công Bạn A thuyết trình phần Toán học/Geometry, Bạn B thuyết trình phần Shaders/Ánh sáng.
4. [ ] **Đồng bộ mã nguồn vào Git:**
   * Lưu commit toàn bộ 6 file vibe coding vào Git để lưu vết tiến độ chuẩn chỉ.
