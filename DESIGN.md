# 🎨 DESIGN.md - THIẾT KẾ GIAO DIỆN, MÀU SẮC, KHÔNG GIAN & CHUYỂN ĐỘNG

**Dự án:** 3D Tropical Oceanarium Simulation (Mô Phỏng Thủy Cung Nhiệt Đới 3D Tương Tác)  
**Phong cách chủ đạo:** *Modern High-End Oceanarium & Biomimetic Marine Realism*  
**Mục tiêu mỹ thuật:** Tái hiện không gian thủy cung rực rỡ, ngập tràn ánh nắng và sắc xanh ngọc nhiệt đới, loại bỏ hoàn toàn cảm giác u ám, tăm tối hoặc đồ chơi hình khối thô sơ.

---

## 1. BẢNG MÀU CHUẨN HỆ THỐNG (COLOR PALETTE & DESIGN TOKENS)

Mọi hiệu ứng Shaders, ánh sáng môi trường trong WebGL và giao diện người dùng HUD đều phải tuân thủ nghiêm ngặt bảng mã màu sau:

| Tên Token Màu | Mã HEX | Giá trị GLSL `vec3(r, g, b)` | Mục đích sử dụng |
| :--- | :--- | :--- | :--- |
| **Tropical Sunlit Surface** | `#38bdf8` | `vec3(0.220, 0.741, 0.973)` | Ánh sáng mặt trời chiếu xuyên qua mặt nước, viền sáng trên lưng cá |
| **Radiant Marine Cyan** | `#068da9` | `vec3(0.024, 0.553, 0.663)` | Màu nước biển nhiệt đới chủ đạo, sương mù đại dương tầng giữa |
| **Deep Coral Turquoise** | `#044a63` | `vec3(0.016, 0.290, 0.388)` | Nền nước ở độ sâu rạn san hô, tạo chiều sâu thị giác |
| **Sun Caustics Highlight** | `#e0f2fe` | `vec3(0.878, 0.949, 0.996)` | Vệt nắng gợn sóng in trên sàn hầm kính và trên mai rùa, lưng cá đuối |
| **Walkway Dark Slate** | `#0f172a` | `vec3(0.059, 0.090, 0.165)` | Sàn đá mài cao cấp của hành lang hầm, có độ bóng phản chiếu (Glossy) |
| **Neon Cyan Guide LED** | `#22d3ee` | `vec3(0.133, 0.827, 0.933)` | Dải đèn LED phát quang dọc theo mép chân tường đường hầm |
| **Acrylic Glass Glare** | `#ffffff` | `vec4(1.0, 1.0, 1.0, 0.25)` | Vệt sáng phản quang Fresnel trên bề mặt kính vòm Acrylic |
| **Museum Glass HUD** | `#0a192f` | `rgba(10, 25, 47, 0.75)` | Nền bán trong suốt có hiệu ứng làm mờ (Backdrop Blur) của các Kiosk |

---

## 2. QUY CHUẨN TYPOGRAPHY (FONT CHỮ & CẤP ĐỘ NỘI DUNG)

Sử dụng bộ font chữ hiện đại, đậm chất bảo tàng khoa học công nghệ:

1. **Heading & Title (Tiêu đề lớn, tên loài sinh vật):**
   * **Font family:** `'Space Grotesk'`, sans-serif
   * **Kích thước:** `24px - 32px`, `font-weight: 700`
   * **Letter-spacing:** `1.5px` (Tạo nét hiện đại, công nghệ cao)
2. **Technical HUD & Coordinates (Chỉ số FPS, Tọa độ Camera, Góc nhìn):**
   * **Font family:** `'JetBrains Mono'`, monospace
   * **Kích thước:** `12px - 14px`, `font-weight: 500`
   * **Màu sắc:** `#38bdf8` (Xanh thiên thanh phát sáng nhẹ)
3. **Body Text & Biological Info (Nội dung thuyết minh sinh học Kiosk):**
   * **Font family:** `'Inter'`, -apple-system, sans-serif
   * **Kích thước:** `14px - 15px`, `font-weight: 400`, `line-height: 1.6`
   * **Màu sắc:** `#e2e8f0` (Trắng xám dịu mắt)

---

## 3. CÁC THÀNH PHẦN THỊ GIÁC (VISUAL COMPONENTS)

### 3.1. Hầm Kính Acrylic Đi Bộ (Walkway Glass Tunnel)
* **Hình học:** Nửa hình trụ tham số bán kính $R = 4.5m$, chiều dài $L = 60m$.
* **Vật liệu kính:** Truyền ánh sáng vật lý (Physical Transmission = 0.92, Roughness = 0.05, Index of Refraction IOR = 1.491).
* **Nẹp chịu lực (Steel Ribs):** Các khung vòm kim loại xám titan đặt cách nhau mỗi $4m$, tạo cảm giác kết cấu công trình chịu lực chân thực.
* **Sàn đi bộ (Polished Floor):** Bề mặt đá thẫm màu, phản chiếu ánh sáng caustics gợn sóng từ trên nóc kính đổ xuống.
* **Dải đèn LED chân tường:** Hai đường ánh sáng neon xanh ngọc chạy dọc mép sàn dẫn hướng người tham quan trong hầm.

### 3.2. Trụ Kiosk Cảm Ứng Thông Tin (Digital Museum Kiosks)
* **Vị trí:** Đặt dọc lối đi hầm kính tại các điểm ngắm cá đẹp.
* **Kiểu dáng:** Trụ kim loại sơn tĩnh điện vát góc 45 độ hướng về mắt khách tham quan.
* **Màn hình hiển thị:** Giao diện kính Glassmorphism mờ ảo:
  * Ảnh chụp thực tế của loài sinh vật.
  * Tên phổ thông & Tên khoa học La-tinh (vd: *Caranx ignobilis* - Giant Trevally).
  * Kích thước tối đa, phạm vi phân bố nhiệt đới, thức ăn chủ yếu.

### 3.3. Ánh Sáng Đại Dương & Sương Mù (Ocean Lighting & Fog)
* **Volumetric Sun God Rays (Tia nắng chiếu xiên):** Các dải tia sáng hình nón chiếu xiên từ mặt nước góc 65 độ xuống đáy cát san hô, có độ trong suốt và nhấp nhô mềm mại.
* **Ocean Caustics:** Mạng lưới gợn sóng ánh sáng chuyển động liên tục theo thời gian $t$, tính toán bằng thuật toán Voronoi / Perlin Noise trong Shader.
* **Exponential Fog:** Nền nước có sương mù giảm tầm nhìn theo hàm mũ $e^{-(\text{density} \cdot d)^2}$ với màu nước ngọc bích sáng, giúp cảnh quan có độ sâu thăm thẳm tự nhiên mà không bị tối đen.

---

## 4. TIÊU CHUẨN CHẤT LƯỢNG MÔ HÌNH 3D SINH VẬT BIỂN (SKETCHFAB STANDARD)

Bám sát 100% hình ảnh thực tế từ kho tài nguyên Sketchfab (ảnh mẫu cá Giant Trevally và Manta Ray của đề bài):

```
       CÁ KHẾ VÂY XANH (GIANT TREVALLY)           CÁ ĐUỐI MANTA (GIANT MANTA RAY)
       ┌───────────────────────────────┐          ┌──────────────────────────────┐
       │ - Vảy ánh bạc kim loại (PBR)  │          │ - Sải cánh rộng 6.5m         │
       │ - Lưng màu xanh xám điểm đốm  │          │ - Lưng đen tuyền vệt phát quang│
       │ - Vây ngực cử động linh hoạt  │          │ - Bụng trắng muốt tự nhiên   │
       │ - Đuôi hình chữ V xẻ sâu      │          │ - Lướt êm ái sát vòm kính    │
       └───────────────────────────────┘          └──────────────────────────────┘
```

1. **Cá Khế Vây Xanh / Giant Trevally (`giant_trevally`):**
   * Bề mặt vảy có tính phản xạ kim loại cao (Metallic = 0.65, Roughness = 0.3), lấp lánh khi có tia nắng chiếu vào.
   * Cử động vẫy đuôi nhịp nhàng tần số $1.2\text{ Hz}$.
2. **Cá Đuối Manta (`manta_ray.glb`):**
   * Mô hình có sải cánh cong mềm mại; đỉnh vây uốn lượn hình sin lan truyền từ thân ra chóp vây.
   * Có vân da tự nhiên và đốm nhận dạng ở mặt bụng.
3. **Cá Mập Hổ (`shark.glb`):**
   * Thân hình ngư lôi kinh điển, có các sọc vằn xám mờ trên thân.
   * Bơi tuần tra chậm rãi, đầu hơi lắc nhẹ theo phương ngang theo góc tiếp tuyến của đường bơi.
4. **Rùa Biển Đồi Mồi (`turtle.glb`):**
   * Mai rùa có hoa văn lồi lõm chân thực.
   * Hai vây trước đập so le như cánh chim dưới nước.

---

## 5. THIẾT KẾ CHUYỂN ĐỘNG & ĐIỀU HƯỚNG (MOTION DESIGN SPECIFICATIONS)

### 5.1. Chuyển Động Chuyển Cảnh "Xuyên Kính" (Seamless Dive Transition)
* **Kích hoạt:** Người dùng nhấn phím **`[SPACE]`**.
* **Đường đi Camera:** Từ vị trí mắt người tham quan trong hầm $(0, 1.8, z)$ bay vút lên cao theo đường cong Hermite/SmoothStep, xuyên qua đỉnh vòm kính $(0, 5.2, z)$ ra ngoài lòng biển $(0, 9.0, z - 8.0)$.
* **Hiệu ứng biến mất của hầm kính:**
  * Thuật toán: Giảm dần độ mờ đục `opacity` của vật liệu kính từ $1.0 \to 0.0$ trong vòng $0.8$ giây đầu.
  * Khi ra đến lòng biển, toàn bộ mô hình hầm kính, khung sắt, sàn bê tông được chuyển sang trạng thái `visible = false` (Không còn kính chắn 100%).
* **Easing Function:** GSAP `power2.inOut` trong $2.5$ giây, triệt tiêu mọi cảm giác giật khấc.

### 5.2. Chuyển Động Bơi Theo Đường Cong Bézier 3D
* Đàn cá bơi tuần hoàn theo quỹ đạo kín bậc 3 được tạo bởi 4 điểm điều khiển $P_0, P_1, P_2, P_3$.
* Hướng quay của đầu cá được tính tự động dựa trên vector tiếp tuyến $\vec{T}(t) = \frac{dB(t)}{dt}$:
  $$\vec{Forward} = \text{normalize}(\vec{T}(t))$$
  $$\vec{Right} = \text{normalize}(\vec{Forward} \times \vec{Up})$$
  $$\vec{TrueUp} = \vec{Right} \times \vec{Forward}$$
* Khi cá rẽ sóng, thân cá hơi nghiêng góc Roll từ $5^\circ - 15^\circ$ theo bán kính cong của đường đi.

### 5.3. Hệ Thống Hạt Bọt Khí (Rising Bubble Particles)
* Hạt xuất phát từ mang cá hoặc các kẽ nứt san hô dưới đáy.
* Vận tốc bay lên: $v_y = 1.2 + 0.5 \cdot \text{rand}()$.
* Dao động ngang: $x(t) = x_0 + 0.1 \cdot \sin(3t)$, $z(t) = z_0 + 0.1 \cdot \cos(3t)$ mô phỏng lực cản chất lưu của nước.
