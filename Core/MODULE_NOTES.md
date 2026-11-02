# Bước 0: khung module Lab 3

> Đây là ghi chép của bước tạo khung trước Ex3. Trạng thái mới nhất xem
> [EX3_REPORT.md](EX3_REPORT.md); các mô tả TIM2 chưa bật/GPIO_NOPULL bên dưới
> ghi lại trạng thái ban đầu, không phải cấu hình runtime sau Ex3.

Đã đọc toàn bộ 24 trang `VXL_VDK_Lab_3_Buttons-Switches.pdf`.
Yêu cầu của người dùng quyết định mapping và phạm vi thực hiện. Ví dụ PB0 làm
nút, PA0–PA7 làm output ở trang 13–14 không áp dụng cho project này.
Các yêu cầu video, báo cáo và nộp BKeL của Ex10 là nội dung bài lab, chưa phải
thao tác được thực hiện trong bước tạo khung.

## Thay đổi trong bước này

Chỉ thêm bốn cặp file `.c/.h` trong `Src` và `Inc`:

| Module | Trách nhiệm dự kiến |
| --- | --- |
| input_reading | Đọc MODE/SET/SAVE, chống dội, trạng thái nhấn và giữ nút |
| input_processing | FSM mode, sự kiện nhấn, giá trị đang chỉnh và SAVE |
| output_display | Buffer bốn digit và quét LED 7 đoạn |
| traffic_light | Pha giao thông, đếm giây, nhấp nháy màu đang chọn |

Các hàm hiện là stub: không đọc/ghi GPIO, không tạo ngắt, không chạy FSM.
Hai hàm truy vấn nút luôn trả về 0. Chưa gọi các module từ main.c.
Enum, hằng số và prototype là giao diện chuẩn bị cho các bước tiếp theo;
chưa có exercise chức năng nào được coi là hoàn thành.

## Kết quả kiểm tra project hiện tại

- MCU: STM32F103C6Ux; HSI và bus hiện chạy 8 MHz, không PLL.
- PA2 MODE, PA3 SET, PA4 SAVE: input, GPIO_NOPULL.
- PA5/6/7: đỏ/vàng/xanh tuyến 1; PA8/9/10: đỏ/vàng/xanh tuyến 2.
- PB0–PB6: SEG_A–SEG_G; PB7–PB10: EN0–EN3. Mapping đã đúng.
- GPIO output push-pull, tốc độ LOW; mức khởi tạo output là LOW.
- Chưa có TIM2 init/IRQ/callback; HAL_TIM_MODULE_ENABLED đang bị comment.
- MSP hiện tắt cả JTAG và SWD; PB3/PB4 đã được giải phóng cho segment.
- .cproject có Core là source entry và Core/Inc là include path.
- Không tìm thấy AGENTS.md tại các cấp cha và các thư mục module.
- Thư mục project hiện không phải Git repository.

Không sửa .ioc, main.c, main.h, HAL config, MSP, interrupt handlers hoặc
cấu hình build. Không thay đổi chân hay cấu hình GPIO.

## Các điểm tích hợp cần xử lý theo exercise

1. Ex1: vẽ FSM 4 mode và các pha giao thông. MODE chuyển 1 → 2 → 3 → 4 → 1;
   mode 2/3/4 chỉnh đỏ/vàng/xanh; tách giá trị nháp khỏi giá trị đã lưu.
2. Ex2: đối chiếu schematic Proteus với mapping hiện có. Nút active LOW dự kiến
   cần điện trở kéo lên ngoài vì GPIO_NOPULL. PDF yêu cầu 12 LED vật lý;
   6 tín hiệu hiện có cần điều khiển các hướng đối diện tương ứng trong sơ đồ.
3. Ex3: bổ sung TIM2 10 ms bằng code ứng dụng và điểm nối tối thiểu trong vùng
   USER CODE nếu cần; phải giữ nguyên .ioc và phần CubeMX sinh. Với clock
   hiện tại 8 MHz, PSC=799 và ARR=99 là một phương án 10 ms, cần kiểm tra lại
   clock thực tế khi triển khai. HAL TIM đang tắt nên chưa thể gọi API TIM
   trực tiếp mà không xử lý phần phụ thuộc này.
4. Ex4: dùng INPUT_READING_TIMER_PERIOD_MS làm hằng chu kỳ danh định khi tích hợp
   timer; truyền thời gian thực đã trôi vào traffic_light_process. Tích lũy ms,
   trừ từng mốc 250 ms và giữ phần dư để nhấp nháy 2 Hz (chu kỳ đầy đủ 500 ms).
   Tick 100 ms gây lượng tử hóa thời điểm cạnh; muốn cạnh đúng 250 ms cần
   timebase đủ mịn. Không dùng số lần ngắt cố định hoặc xóa phần dư.
5. Ex5: chống dội cả ba nút; MODE dùng cạnh nhấn để mỗi lần nhấn chỉ đổi một
   mode. Thời gian debounce/giữ nút tính bằng ms; ISR chỉ làm việc ngắn,
   không HAL_Delay. Nếu tick tăng đến 100 ms thì không thể bảo đảm đáp ứng
   nút 50 ms như phần lý thuyết; giữ lịch lấy mẫu nút 10 ms khi cần.
6. Ex6: hiển thị mode và giá trị; nhấp nháy đồng thời màu được chọn ở hai tuyến.
   Dự kiến quét độc lập 1 ms/digit, khung 4 ms (250 Hz mỗi digit), không phụ
   thuộc chu kỳ TIM2 của Ex4. Phải tắt mọi EN trước khi đổi segment rồi mới
   bật một EN để tránh bóng mờ; buffer cần công bố nhất quán nếu dùng ISR.
7. Ex7: SET chỉnh đỏ 1–99, SAVE xác nhận; chốt quy tắc khi tăng quá 99.
8. Ex8: áp dụng chỉnh/lưu cho vàng.
9. Ex9: áp dụng chỉnh/lưu cho xanh.
10. Ex10: tích hợp chạy giao thông, kiểm tra tính nhất quán các thời lượng,
    đếm ngược và chuyển mode; sau đó chuẩn bị kiểm thử/video/báo cáo.

Common Anode + PNP high-side theo yêu cầu: EN LOW bật digit, SEG LOW bật đoạn.
Khi triển khai output_display_init phải đưa EN HIGH trước, rồi SEG HIGH.
Mức LOW ban đầu do CubeMX sinh có thể làm sáng đồng thời các digit trước bước
blanking này; khung hiện chưa can thiệp hành vi khởi động. Cực tính LED giao
thông cần đối chiếu schematic, không suy ra từ cực tính LED 7 đoạn.

## Kiểm tra khung

Đã biên dịch cả bốn module thành object bằng arm-none-eabi-gcc 9.3.1 của
STM32CubeIDE, Cortex-M3, STM32F103x6, USE_HAL_DRIVER và header HAL/CMSIS thực
của project, với -std=c11 -Wall -Wextra -Werror. Đã liên kết relocatable
bốn object thành công. File kiểm tra nằm ngoài project.

Đây chưa phải full firmware build/link hoặc kiểm thử trên Proteus/phần cứng.
