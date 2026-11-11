# 📄 PRD.md - TÀI LIỆU YÊU CẦU SẢN PHẨM (PRODUCT REQUIREMENTS DOCUMENT)
**Dự án:** 3D Tropical Oceanarium Simulation (Mô Phỏng Thủy Cung Nhiệt Đới 3D Tương Tác)  
**Học phần:** Đồ Họa Máy Tính (Computer Graphics)  
**Nhóm thực hiện:** 2 sinh viên (Bạn A: Backend/Math - Bạn B: Frontend/Shaders)

---

## 1. TỔNG QUAN DỰ ÁN (EXECUTIVE SUMMARY)
Ứng dụng đồ họa máy tính 3D thời gian thực (Real-time 3D Graphics) tái hiện không gian thủy cung nhiệt đới hiện đại chuẩn quốc tế. Sản phẩm cung cấp trải nghiệm kép (Dual-Mode):
1. **Chế độ 1 (Tham quan hầm kính):** Khách đi bộ trong đường hầm vòm kính Acrylic trong suốt, ngắm nhìn đàn sinh vật biển bơi lội ngay trên đỉnh đầu.
2. **Chế độ 2 (Hòa mình vào lòng biển):** Khi bấm `[SPACE]`, camera lướt êm ái xuyên qua mặt kính, đưa người xem hòa mình 100% vào làn nước đại dương rực rỡ nắng, bơi tự do cùng đàn cá mà **hoàn toàn không còn kính chắn**.

---

## 2. ĐỐI TƯỢNG NGƯỜI DÙNG & MỤC ĐÍCH SỬ DỤNG
* **Hội đồng chấm thi / Giảng viên bộ môn:** Đánh giá việc áp dụng 100% chuẩn xác các thuật toán đồ họa trong giáo trình (Ma trận phân cấp CG.4, Camera LERP CG.4, Đường cong Bézier 3D CG.5, Mặt cong tham số CG.5, Shaders Blinn-Phong/Fog/Caustics CG.6).
* **Khách tham quan / Người dùng trải nghiệm:** Khám phá thế giới đại dương lung linh, ngắm nhìn các loài cá 3D sinh học chân thực (như Giant Trevally, Manta Ray, Tiger Shark, Sea Turtle tải từ Sketchfab), thưởng thức nghệ thuật chuyển động Motion Design mượt mà 60 FPS.

---

## 3. CÁC TÍNH NĂNG CỐT LÕI (CORE FEATURES)

### 3.1. Hệ Sinh Thái Biển Chân Thực 100% (Real 3D Marine Life)
* **YÊU CẦU BẮT BUỘC:** Không sử dụng các khối hình học cơ bản (hình cầu, hình nón) để giả lập cá. Phải nạp 100% mô hình 3D thực tế chất lượng cao từ Sketchfab kèm đầy đủ cấu trúc giải phẫu sinh học (vây, mang, mắt, vảy ánh bạc như cá Giant Trevally):
  * **Cá Đuối Manta Khổng Lồ (`manta_ray.glb`):** Sải cánh rộng, lưng có vệt sáng xanh Specular, bơi lướt trên nóc vòm kính.
  * **Cá Mập Hổ (`shark.glb`):** Thân hình thon dài, vây lưng nhọn, bơi tuần tra tầng giữa.
  * **Rùa Biển (`turtle.glb`):** Mai đốm rêu chân thực, 2 vây bơi trước đập nước nhịp nhàng.
  * **Cá Khế Vây Xanh / Cá Vẫu (`giant_trevally.glb`):** Vảy ánh kim lấp lánh phản chiếu ánh nắng.
  * **Đàn cá nhiệt đới (`tropical_fish.glb`):** Hàng chục cá hề Nemo, cá gai vàng Tang bơi theo bầy quanh rạn san hô.

### 3.2. Không Gian Kiến Trúc Thủy Cung Hiện Đại
* **Vòm kính Acrylic:** Tự sinh bằng công thức toán tham số nửa hình trụ, chất liệu kính trong suốt có vệt phản xạ gương (Fresnel Specular Glare).
* **Hành lang đi bộ:** Sàn gạch tối màu có gợn sóng ánh nắng (Caustics) in bóng và dải đèn LED xanh ngọc sáng chói dẫn lối 2 bên mép sàn.
* **Trụ Kiosk điện tử:** Bảng thông tin cảm ứng kỹ thuật số dọc lối đi giới thiệu chi tiết sinh học từng loài cá.

### 3.3. Cơ Chế Chuyển Cảnh "Xuyên Kính" (Seamless Dive Transition)
* Nhấn phím **`[SPACE]`**: Camera sử dụng thuật toán nội suy tuyến tính (LERP) lướt êm ái xuyên qua vách kính ra lòng nước.
* Toàn bộ hầm kính được ẩn đi 100%, chuyển sang chế độ lặn tự do 360°, bọt khí bay ngay trước mắt dưới ánh nắng chói lòa.

---

## 4. TIÊU CHÍ PHI CHỨC NĂNG (NON-FUNCTIONAL REQUIREMENTS)
* **Độ mượt mà:** Đảm bảo duy trì ổn định **60 FPS** ở độ phân giải Full HD (1920x1080).
* **Thẩm mỹ:** Không sử dụng màu đen sì u ám; nền nước phải là màu **xanh ngọc bích nhiệt đới sáng rực rỡ (Bright Tropical Cyan `#068da9`)** với các luồng tia nắng mặt trời (Volumetric Sun Rays) chiếu sáng đáy biển.
* **Độ tin cậy:** Khởi chạy độc lập không phụ thuộc kết nối mạng khi thuyết trình bảo vệ.
