# 📋 TASKS.md - DANH SÁCH CÔNG VIỆC CHI TIẾT (GRANULAR STEP-BY-STEP CHECKLIST)

**Dự án:** 3D Tropical Oceanarium Simulation (Mô Phỏng Thủy Cung Nhiệt Đới 3D Tương Tác)  
**Phân công nhiệm vụ:**  
* 🔵 **Bạn A (Backend / Math / Geometry):** Quản lý Camera 6-DOF, Đường cong Bézier 3D, Lưới vòm kính tham số, Nạp mô hình 3D Sketchfab.  
* 🟢 **Bạn B (Frontend / Shaders / VFX):** Quản lý Hệ thống Shaders GLSL, Ánh sáng sương mù đại dương, Hiệu ứng bọt khí, Giao diện WebGL / HUD Kiosk.

---

## 📌 MILESTONE 0: KHỞI TẠO DỰ ÁN & THU THẬP TÀI NGUYÊN 3D CHÂN THỰC
> **Mục tiêu:** Chuẩn bị hạ tầng mã nguồn, thư viện C++/OpenGL và tải toàn bộ mô hình 3D sinh học thực tế từ Sketchfab (chuẩn cá Giant Trevally, Manta Ray, Shark,...).

- [x] **Task 0.1 [Chung]:** Khởi tạo cấu trúc thư mục chuẩn tại `C:\Exercise\DHMT\Aquarium_3D\` theo quy định.
  - *Tiêu chí nghiệm thu:* Có đầy đủ các thư mục `include/`, `src/`, `assets/models/`, `assets/shaders/`.
  - *Kiểm tra:* Lệnh PowerShell `Get-ChildItem -Directory`.
- [x] **Task 0.2 [Bạn A]:** Thu thập các mô hình 3D sinh vật biển thực tế định dạng `.glb` và `.obj` từ Sketchfab.
  - *Tiêu chí nghiệm thu:* Đã tải thành công 7 sinh vật biển kích thước > 30MB vào `assets/models/`:
    - `manta_ray.glb` (2.0 MB)
    - `shark.glb` (7.07 MB)
    - `turtle.glb` (3.63 MB)
    - `tropical_fish.glb` (10.12 MB)
    - `dolphin.glb` (3.19 MB)
    - `tuna.glb` (190 KB)
    - `whale.glb` (3.78 MB)
    - Mô hình cá Khế Vây Xanh (`giant_trevally.obj`).
  - *Kiểm tra:* Lệnh `Get-ChildItem assets/models/` hiển thị đầy đủ các file dung lượng chuẩn.
- [x] **Task 0.3 [Bạn B]:** Tạo script tự động khởi chạy môi trường kiểm thử WebGL không lo lỗi CORS (`run.bat`).
  - *Tiêu chí nghiệm thu:* Nhấp đúp chuột vào `run.bat` tự bật Python Local Server cổng 8080 và mở trình duyệt.

---

## 📌 MILESTONE 1: DỰNG KẾT CẤU HÌNH HỌC THAM SỐ & TOÁN HỌC ĐƯỜNG CONG (BẠN A)
> **Mục tiêu:** Xây dựng vòm kính thủy cung theo công thức tham số hình trụ và lập trình giải thuật đường cong Bézier 3D theo đúng giáo trình CG.

- [x] **Task 1.1 [Bạn A]:** Viết lớp `Tunnel.h` tự sinh lưới đỉnh (Mesh) vòm kính Acrylic nửa hình trụ:
  - *Công thức toán học:* $x = R \cos\theta, y = R \sin\theta, z = z$ với $\theta \in [0, \pi]$, $R = 4.5m$, $L = 60m$.
  - *Tiêu chí nghiệm thu:* Sinh đủ mảng đỉnh Position, Normal, TexCoord và mảng chỉ số Indices vẽ bằng `GL_TRIANGLES`.
- [x] **Task 1.2 [Bạn A]:** Viết thuật toán sinh mặt sàn hành lang đi bộ (`floorMesh`) bằng phẳng tại độ cao $y = 0.0$.
  - *Tiêu chí nghiệm thu:* Mặt phẳng rộng $9m$, dài $60m$, pháp tuyến hướng thẳng đứng $(0, 1, 0)$.
- [x] **Task 1.3 [Bạn A]:** Lập trình lớp giải thuật đường cong tham số Bézier 3D bậc 3 (`BezierPath.h`):
  - *Công thức:* $B(t) = (1-t)^3 P_0 + 3(1-t)^2 t P_1 + 3(1-t)t^2 P_2 + t^3 P_3$.
  - *Hàm tiếp tuyến:* $\vec{T}(t) = \frac{dB(t)}{dt}$ dùng để xoay đầu cá theo hướng bơi tự nhiên.
  - *Tiêu chí nghiệm thu:* Cá bơi theo một vòng tròn khép kín mềm mại, không bị giật hoặc đứt quãng.
- [x] **Task 1.4 [Bạn A]:** Viết bộ đọc file mô hình 3D (`Mesh.h`, `Model.h`) kết nối GPU qua VAO/VBO/EBO.
  - *Tiêu chí nghiệm thu:* Nạp đỉnh, pháp tuyến và tọa độ vân bề mặt của các mô hình sinh học chuẩn xác.

---

## 📌 MILESTONE 2: HỆ THỐNG SHADERS ÁNH SÁNG & SƯƠNG MÙ ĐẠI DƯƠNG RỰC RỠ (BẠN B)
> **Mục tiêu:** Viết Shaders GLSL tái hiện màu nước ngọc bích sáng lung linh, vệt nắng caustics và hiệu ứng sương mù đại dương theo chuẩn CG.6.

- [x] **Task 2.1 [Bạn B]:** Viết Vertex Shader đại dương (`ocean_fog.vs`):
  - *Chức năng:* Nhân các ma trận phép chiếu $P$, nhìn $V$, biến đổi mô hình $M$ và truyền tọa độ không gian thế giới vào Fragment Shader.
- [x] **Task 2.2 [Bạn B]:** Viết Fragment Shader đại dương (`ocean_fog.fs`):
  - *Thuật toán Blinn-Phong:* Tính toán thành phần Ambient, Diffuse (Lambert) và Specular với Halfway Vector $\vec{H} = \text{normalize}(\vec{L} + \vec{V})$.
  - *Màu nước biển nhiệt đới:* Sử dụng màu ngọc bích `#068da9` và xanh thiên thanh `#38bdf8`, xóa sổ màu đen sì.
  - *Sương mù đại dương:* Tính hệ số mờ theo khoảng cách $f = e^{-(\text{density} \cdot d)^2}$.
  - *Nắng Caustics:* Tính gợn sóng khúc xạ ánh sáng theo biến thời gian `u_time`.
  - *Tiêu chí nghiệm thu:* Nền nước bừng sáng, lung linh, tạo chiều sâu thị giác rõ nét.
- [x] **Task 2.3 [Bạn B]:** Lập trình hệ thống hạt bọt khí đại dương (`ParticleSystem.h`):
  - *Tiêu chí nghiệm thu:* 200 hạt bọt khí chuyển động nổi dần lên mặt nước và lơ lửng quanh rạn san hô.

---

## 📌 MILESTONE 3: HỆ THỐNG CAMERA KÉP & CHUYỂN CẢNH "XUYÊN KÍNH" (BẠN A + BẠN B)
> **Mục tiêu:** Xây dựng trải nghiệm độc đáo 2 chế độ: Đi bộ trong hầm kính và Lướt tự do ngoài lòng biển mở không còn kính chắn.

- [x] **Task 3.1 [Bạn A]:** Lập trình lớp `Camera.h` hỗ trợ 6 bậc tự do:
  - *Chế độ 1 (Walkthrough):* Khóa độ cao tại tầm mắt người đi bộ $y = 1.8$, giới hạn không gian không cho đi xuyên thủng vách kính.
  - *Chế độ 2 (Free Dive):* Tự do bay lượn 360 độ trong nước bằng các phím WASD + Chuột.
- [x] **Task 3.2 [Bạn B]:** Lập trình cơ chế chuyển cảnh mượt mà bằng phím bấm **`[SPACE]`**:
  - *Giải thuật:* Nội suy tuyến tính LERP vị trí và góc nhìn camera từ hầm kính bay vụt lên qua nóc vòm ra lòng biển.
  - *Hiệu ứng ẩn kính:* Khi đã ra ngoài lòng biển, chuyển trạng thái mô hình hầm kính `visible = false` (hoàn toàn không còn kính chắn).
  - *Tiêu chí nghiệm thu:* Quá trình chuyển cảnh diễn ra êm dịu trong 2.5 giây không rung giật.

---

## 📌 MILESTONE 4: TÍCH HỢP MÔ HÌNH SKETCHFAB & KIOSK BẢO TÀNG (BẠN A + BẠN B)
> **Mục tiêu:** Đưa các mô hình cá thực tế vào bơi trong bể và xây dựng các Kiosk tra cứu thông tin sinh học dọc lối đi.

- [x] **Task 4.1 [Bạn A + Bạn B]:** Đưa các sinh vật biển thực tế vào khung cảnh mô phỏng:
  - Cá Đuối Manta khổng lồ sải cánh bay lượn sát trên nóc vòm kính.
  - Cá Mập Hổ và Cá Khế Vây Xanh tuần tra vùng nước mở.
  - Rùa Biển bơi đập vây nhẹ nhàng gần rạn san hô dưới đáy.
  - *Tiêu chí nghiệm thu:* Cá cử động vẫy đuôi, uốn lượn tự nhiên theo quỹ đạo Bézier, không còn bất kỳ hình khối đồ chơi nào.
- [x] **Task 4.2 [Bạn B]:** Thiết kế giao diện Kiosk Kính Cảm Ứng (Interactive Digital Kiosk):
  - Bảng thông tin đặt nghiêng 45 độ hiển thị tên loài cá, đặc tính sinh học, thói quen săn mồi.
  - Tích hợp thanh HUD tối giản hiển thị chỉ số FPS (60 FPS) và phím tắt điều khiển.

---

## 📌 MILESTONE 5: TỐI ƯU HIỆU NĂNG 60 FPS & ĐÓNG GÓI BẢO VỆ ĐỒ ÁN (CHUNG)
> **Mục tiêu:** Tinh chỉnh chuyển động, kiểm tra trên các máy cấu hình khác nhau, chuẩn bị slide và kịch bản bảo vệ trước hội đồng.

- [x] **Task 5.1 [Chung]:** Kiểm thử hiệu năng đạt chuẩn **60 FPS** ở độ phân giải Full HD.
  - *Tiêu chí nghiệm thu:* Không tụt khung hình khi cả đàn cá và hệ thống bọt khí cùng hoạt động.
- [x] **Task 5.2 [Chung]:** Hoàn thiện bộ 6 file vibe coding (`PRD.md`, `ARCHITECTURE.md`, `DESIGN.md`, `RULES.md`, `TASKS.md`, `PROGRESS.md`).
- [x] **Task 5.3 [Chung]:** Đóng gói Git Commit đầy đủ mã nguồn và tài sản 3D sẵn sàng cho buổi thuyết trình.
