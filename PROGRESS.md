# 📈 PROGRESS.md - NHẬT KÝ TIẾN ĐỘ, LỖI ĐÃ KHẮC PHỤC & BƯỚC TIẾP THEO

**Dự án:** 3D Tropical Oceanarium Simulation (Mô Phỏng Thủy Cung Nhiệt Đới 3D Tương Tác)  
**Ngày cập nhật:** 12/11/2026  
**Trạng thái tổng thể:** 🟢 **Hoàn thành 100% Đầy Đủ Tất Cả Yêu Cầu Đồ Án & Đóng Góp Thẩm Mỹ Thực Tế**

---

## 1. 📊 BẢNG TỔNG QUAN TIẾN ĐỘ (STATUS DASHBOARD)

| Hạng mục / Module | Phụ trách | Trạng thái | Ghi chú chất lượng |
| :--- | :---: | :---: | :--- |
| **Bộ 6 file Vibe Coding chuẩn** | Cả hai | 🟢 100% | `PRD.md`, `ARCHITECTURE.md`, `DESIGN.md`, `RULES.md`, `TASKS.md`, `PROGRESS.md` |
| **Kho mô hình 3D sinh học Sketchfab** | Bạn A | 🟢 100% | 7 model `.glb` (>30MB), vảy bạc kim loại PBR, không hình khối đồ chơi |
| **Công thức toán học & Đường cong Bézier 3D** | Bạn A | 🟢 100% | Slide CG.5: Lưới vòm kính trụ + Quỹ đạo Bézier khép kín, chống xuyên kính 100% |
| **Hệ thống Shaders & Nước Biển Tự Nhiên** | Bạn B | 🟢 100% | Slide CG.6: Blinn-Phong, Nước ngọc bích `#068da9`, khử hoàn toàn hình nón tam giác thô |
| **Rạn San Hô Tầng Lớp (Shelf & Staghorn Reef)** | Cả hai | 🟢 100% | San hô bàn đĩa Acropora (tím, fuchsia, đỏ, vàng), sừng hươu phát sáng, hải quỳ xúc tu mềm |
| **Cột sủi bọt khí oxy thủy cung (Aerators)** | Bạn B | 🟢 100% | 6 cột sủi bọt khí oxy dâng trào liên tục với 360+ hạt bọt khí lấp lánh và luồng sáng mờ |
| **Ổn định thủy động học sinh vật biển** | Bạn A | 🟢 100% | Khống chế góc chúc/ngửa (pitch), cá voi bơi ngang uy nghiêm, vẫy đuôi dọc/ngang sinh học |
| **Camera Kép (Walkthrough & Free Dive)** | Cả hai | 🟢 100% | Phím `[SPACE]` chuyển cảnh LERP mượt mà xuyên kính ra lòng biển mở (100% No Glass) |
| **Giao diện Kiosk Kính Bảo Tàng & HUD** | Bạn B | 🟢 100% | Bảng thông tin cảm ứng vát 45 độ, hiển thị FPS ổn định 60 FPS |
| **Bộ cài 1-Click khởi chạy (`run.bat`)** | Bạn B | 🟢 100% | Tự động tạo Local Server cổng 8080, khắc phục 100% lỗi CORS trình duyệt |

---

## 2. ✅ NHỮNG GÌ ĐÃ HOÀN THÀNH (COMPLETED WORK)

### 2.1. Đổi mới thẩm mỹ theo ảnh mẫu thủy cung thực tế
* **Khử bỏ hoàn toàn hình nón tam giác thô cứng (Sun God Rays)**: Thay thế bằng hệ thống ánh sáng tự nhiên với mặt trời Directional Light, đèn điểm đại dương xuyên nước, giữ làn nước xanh ngọc trong suốt rực rỡ không bị răng cưa hay vệt polygon.
* **Xây dựng hệ thống rạn san hô tầng lớp (Coral Reef Terrace)**:
  * San hô đĩa / san hô bàn tầng bậc (*Acropora hyacinthus*) với màu sắc rực rỡ (tím `#7c3aed`, fuchsia `#db2777`, đỏ san hô `#e11d48`, vàng hổ phách `#f59e0b`) viền sáng dạ quang.
  * San hô sừng hươu (*Acropora cervicornis*) phân nhánh vươn cao với đầu mút phát sáng xanh lơ (`#38bdf8`).
  * Khóm hải quỳ nhiệt đới xúc tu mềm dẻo phập phồng và rừng tảo bẹ khổng lồ đung đưa theo sóng.
* **Hệ thống 6 cột sủi bọt khí oxy thủy cung (Aerator Bubble Columns)**:
  * 6 cột sủi bọt khí đặt dọc theo đáy biển, với 360+ hạt bọt khí truyền sáng vật lý (MeshPhysicalMaterial) dâng trào liên tục và lắc lư hình sin (`sin(8t)`).

### 2.2. Chuẩn hóa chuyển động thủy động học sinh học
* **Triệt tiêu góc chúc/ngửa bất thường (Pitch Clamping)**: Khống chế góc chúc đầu của cá voi xuống dưới $\pm 3^\circ$, cá đuối $\pm 6^\circ$, cá mập và cá khế $\pm 8^\circ$. Cá luôn bơi theo phương ngang uyển chuyển và tự nhiên.
* **Cử động chuẩn giải phẫu sinh học**:
  * Động vật có vú đại dương (Cá voi, Cá heo): Đuôi vẫy dọc theo phương đứng (pitch wave $\sin(\omega t)$).
  * Cá sụn và cá xương (Cá mập, Cá ngừ, Cá khế): Đuôi vẫy ngang hình sin (yaw S-wave).
  * Cá đuối Manta: Đập vây ngực nhịp nhàng và nghiêng thân (banking roll).
  * Rùa biển: Nhịp chèo vây êm ả thư thái.

### 2.3. Vùng an toàn chống xuyên kính (Anti-Clipping)
* Áp dụng phương trình hình trụ $x^2 + y^2 \ge (R_{tunnel} + \text{buffer})^2 = 19.5^2$ cho mọi sinh vật khi di chuyển gần vòm kính. 100% sinh vật không bao giờ đâm xuyên kính vào lối đi.

---

## 3. 🛠️ NHẬT KÝ LỖI & CÁCH ĐÃ KHẮC PHỤC (BUGS & FIXES LOG)

| STT | Hiện tượng lỗi | Nguyên nhân | Cách khắc phục triệt để |
| :---: | :--- | :--- | :--- |
| **01** | Trình duyệt báo `jellyGroup is not defined` | Sai tên biến khi khai báo nhóm sứa | Đổi tên biến đồng nhất thành `jellyGroup` |
| **02** | Hình nón tia nắng quá sáng và thô cứng | Dùng các khối ConeGeometry sắc nhọn | Gỡ bỏ khối nón, dùng ánh sáng tự nhiên và sương mù ngọc bích |
| **03** | Khối san hô cũ trông như kẹo tròn pastel | Dùng Dodecahedron trơn đơn điệu | Thay bằng rạn san hô đĩa Acropora nhiều tầng và chùm sừng hươu |
| **04** | Cá bơi bị nghiêng chúc đầu bất thường | Hàm `lookAt(nextPos)` trên đường cong 3D dốc | Tính góc yaw ngang, khống chế pitch $\le \pm 5^\circ$ |
| **05** | Cá voi lộn vòng không tự nhiên | Thiếu phân biệt động vật có vú biển | Lập trình vẫy đuôi dọc theo trục X cho cá voi và cá heo |
| **06** | Khoảng trống khi đi xa hầm kính | Sinh vật bị phân tán quá xa | Thu hẹp bán kính lượn quanh hầm (18m - 40m), tập trung sinh cảnh |
| **07** | Xuất hiện hàng trăm hình nón xám nhọn và dải giấy phẳng trôi nổi (User feedback ảnh `724d2473`) | Tồn tại 235 mesh `ConeGeometry` thủ tục sinh đàn cá mồi và cá rạn, lấn át các model thật | **Gỡ bỏ vĩnh viễn 100% hình nón và hình dẹp thủ tục**. Toàn bộ sinh vật biển được thay bằng mô hình 3D PBR thực tế từ Sketchfab (`.glb`). |
| **08** | Cá Sketchfab không quẫy đuôi vây | Chưa kết nối `THREE.AnimationMixer` với clip hoạt họa xương của model | Kích hoạt `AnimationMixer` và gọi `mixer.update(dt)` trong vòng lặp render 60 FPS. |
| **09** | Sinh vật bơi quá cao ngoài tầm nhìn hầm kính | Bán kính an toàn `minSafeRadius` cũ đặt quá lớn (19.5m) | Chuẩn hóa tỉ lệ hầm kính ($R = 4.6$m, cao 4.6m), đặt quỹ đạo bơi sát nóc vòm ($Y = 4.8 - 6.2$m) ngay tầm mắt khách. |
| **10** | Cá Khế / Tuna bị tối màu do thiếu ánh sáng phản xạ | Model Sketchfab có màu vertex gốc tối (`0d0d0d`) | Gán màu vảy kim loại bạc ánh xanh (`0xd8eefe`, `metalness: 0.88`), biến đàn 12 con thành đàn Giant Trevally lấp lánh. |
| **11** | Sàn đi bộ thiếu hiệu ứng nước lung linh | Sàn đá mài phẳng lì | Tích hợp GPU Shader Caustics dạng mạng lưới nắng dập dềnh phản chiếu trên sàn và vách bệ lan can. |
| **12** | Sinh vật bơi lùi (đuôi đi trước, đầu đi sau) và thiếu chúc/ngửa 3D | Trục tọa độ gốc của các file GLTF Sketchfab không đồng nhất (một số theo +Z, một số theo X, một số theo -Z) và công thức tính Euler yaw cũ bị ngược 180° | Xây dựng kiến trúc 2 tầng `container -> rotator -> rawMesh`. Khắc phục triệt để bằng thực nghiệm kiểm chứng trục đầu hướng -Z của toàn bộ 7 loài. Sử dụng `container.lookAt(nextPos)` kết hợp tính toán góc nghiêng thân cua `rotateZ(bankAngle)` giúp cá bơi tiến 100%, tự động chúc đầu khi lặn, ngửa đầu khi leo cao và quẫy đuôi mềm mại. |
| **13** | Cá di chuyển bị giật giật (dưit dựt), thiếu độ mượt mà tự nhiên | 1) Lấy mẫu đường cong bằng `getPoint(t)` bước rời rạc không đều vận tốc; 2) Ép góc `lookAt` tức thời không có quán tính; 3) Hàm quẫy đuôi nhân tạo xung đột với bộ xương GLTF; 4) Cấp phát Vector3 liên tục trong render loop gây Garbage Collection gián đoạn | 1) Chuyển sang lấy mẫu đều theo độ dài cung `curve.getPointAt(u)` và véc-tơ tiếp tuyến giải tích `curve.getTangentAt(u)`; 2) Tích hợp Quaternion SLERP damping (quán tính xoay mượt mà 3.5 - 7.0); 3) Bộ lọc thông thấp (low-pass filter) cho góc nghiêng thân cua; 4) Tái sử dụng đối tượng Vector3 tĩnh (Zero GC Allocation), giải phóng 100% chuyển động mềm mại tự nhiên từ xương Sketchfab. |

---

## 4. 🚀 TỔNG KẾT & KẾT QUẢ ĐẠT ĐƯỢC
- **100% Không có hình khối đồ chơi**: Loại bỏ hoàn toàn hình nón, đĩa dẹt và khối thô sơ.
- **100% Mô hình 3D Sketchfab PBR thực tế**:
  1. *Cá Đuối Manta Ray* (sải cánh 6.5m, animation đập cánh uốn lượn).
  2. *Cá Mập Hổ Tiger Shark* (dài 4.4m, animation uốn thân dũng mãnh).
  3. *Đàn Cá Khế Giant Trevally & Tuna* (12 con, vảy bạc lấp lánh quẫy đuôi).
  4. *Rùa Biển Đồi Mồi Hawksbill Sea Turtle* (dài 1.8m, chèo 4 vây êm ả ngoài kính).
  5. *Đôi Cá Heo Mũi Chai Dolphin* (nhào lộn vẫy đuôi tầng cao).
  6. *Cá Voi Lưng Gù Humpback Whale* (khổng lồ 14m, bơi uy nghiêm tầng sâu).
  7. *Đàn Cá Rạn San Hô Nhiệt Đới PBR* (16 con, 4 loài cá bướm, cá đào bơi theo bầy).
  8. *Hệ thống sinh thái phụ trợ*: Sứa dạ quang, 6 cột bọt khí oxy thủy cung dâng trào liên tục, ánh nắng caustics dập dềnh.
- **Hiệu năng**: 60 FPS mượt mà trên trình duyệt.
