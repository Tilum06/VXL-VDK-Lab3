# Lab 3 - Exercise 4: Modify Timer Parameters

Phạm vi: mục 5.5, trang 22 của tài liệu Lab 3 Buttons/Switches, theo yêu cầu
người dùng: thử một LED PA5 (RED_LED), sáng 250 ms/tắt 250 ms, độc lập chu kỳ
TIM2. Không triển khai Ex5-Ex10, không sửa .ioc hoặc .gitignore.
Các ghi chú EX3_REPORT.md và MODULE_NOTES.md mô tả các bước trước; báo cáo này
mô tả trạng thái sau khi CubeMX đã sinh thêm cấu hình TIM2 trong project.

## Kiểm tra source trước khi sửa

- Đã đọc các module Core/Src, Core/Inc, cấu hình clock, startup, build script,
  cấu hình project/.ioc và các phần HAL liên quan đến tick/timer/GPIO.
- `main.c` đã gọi `MX_TIM2_Init()` rồi `timer2_init()`. Hàm thứ hai đã start
  interrupt nhưng cũng cấu hình lại PSC/ARR thành 10 ms, ghi đè giá trị CubeMX.
- Có handle `htim2` trùng giữa main.c và timer.c; MSP trùng giữa file CubeMX
  và timer.c; hai định nghĩa `TIM2_IRQHandler()` trong stm32f1xx_it.c.
- Chỉ có một callback ứng dụng `HAL_TIM_PeriodElapsedCallback()` trong timer.c;
  nó chỉ tăng `timer2_irq_count`. Callback weak trong HAL là cơ chế override
  bình thường. Không có blink/traffic FSM đang chạy gây xung đột PA5.
- `HAL_InitTick()` mặc định cấu hình SysTick 1 kHz, `SysTick_Handler()` gọi
  `HAL_IncTick()`, `HAL_GetTick()` đọc uwTick. Không có override tick sang TIM2,
  không có lời gọi đổi tick frequency hoặc suspend tick trong ứng dụng.
- `HAL_RCC_ClockConfig()` cập nhật SysTick theo clock mới. Do đó tick HAL vẫn
  1 ms khi thay PSC/ARR TIM2; không cần thêm timer phần cứng.

## Thay đổi

| File | Nội dung |
| --- | --- |
| Src/main.c | Include traffic_light, gọi init và process trong USER CODE |
| Src/traffic_light.c | Blink PA5 bằng HAL_GetTick(), không blocking |
| Inc/traffic_light.h | Cập nhật prototype process(void) của module hiện có |
| Src/timer.c | Bỏ handle/MSP trùng và cấu hình lại timer; giữ start IRQ/callback |
| Inc/timer.h | Mô tả hợp đồng khởi tạo và tốc độ bộ đếm kiểm tra |
| Src/stm32f1xx_it.c | Bỏ ISR TIM2 trùng trong USER CODE, giữ ISR CubeMX |
| Inc/stm32f1xx_it.h | Bỏ prototype TIM2 lặp trong USER CODE |
| EX4_REPORT.md | Báo cáo và hướng dẫn kiểm tra Ex4 |

Giữ nguyên mapping, MX_GPIO_Init(), board_io_init(), pull-up nút, blanking
segment/enable, clock hệ thống và callback TIM2. Không thêm module C/H mới.
TIM2 vẫn có ngắt và bộ đếm debug Ex3, do CubeMX sở hữu handle/MSP/IRQ/PSC/ARR.

## Nguyên lý 2 Hz

`traffic_light_init()` đặt PA5 LOW và lưu HAL_GetTick(). Vòng lặp chính gọi
`traffic_light_process()` liên tục. Hàm lấy hiệu unsigned giữa tick hiện tại
và mốc cũ, chia cho 250 ms để tìm số nửa chu kỳ đã qua. Hàm tiến mốc theo số
nửa chu kỳ đầy đủ, giữ phần dư, chỉ toggle khi số nửa chu kỳ là lẻ.

Vì vậy chu kỳ đầy đủ là 500 ms, tần số 2 Hz, duty cycle 50%. Không đếm ngắt
TIM2, không dùng HAL_Delay(), không chờ bận. Phép trừ uint32_t xử lý được một
lần tràn HAL tick, miễn foreground không bị ngừng suốt một vòng tick (~49,7 ngày).
Nếu foreground bị chặn lâu, hàm khôi phục đúng pha nhưng không thể tái tạo các
cạnh đã bỏ lỡ. Cần giữ vòng lặp chính chạy nhanh hơn đáng kể so với 250 ms;
độ phân giải danh định là 1 ms cộng độ trễ phục vụ vòng lặp/ngắt.

LOW 250 ms rồi HIGH 250 ms cho cùng thời gian sáng/tắt với cả cực tính active
HIGH và active LOW. Cực tính chỉ quyết định trạng thái sáng đầu tiên: active
HIGH bắt đầu tắt; active LOW bắt đầu sáng. Không suy ra cực tính LED giao thông
từ LED 7 đoạn. File Proteus Lab_3 đã lưu trên đĩa chỉ có khung sheet, không có
linh kiện STM32/LED để xác nhận dây nối; công cụ UI cũng không kích hoạt được
cửa sổ Proteus. Cần đối chiếu bản schematic thực tế khi chạy thủ công.

## Cấu hình TIM2 trong STM32CubeIDE

Clock hiện tại: HSI 8 MHz, không PLL; SYSCLK = HCLK = PCLK1 = 8 MHz;
APB1 chia 1 nên TIM2CLK = 8 MHz. Khi APB1 chia khác 1, clock timer bằng 2 PCLK1.

`T_update = (PSC + 1) * (ARR + 1) / TIM2CLK`

| Chu kỳ update | PSC (Prescaler) | ARR (Counter Period) | Tần số IRQ |
| --- | ---: | ---: | ---: |
| 1 ms | 799 | 9 | 1000 Hz |
| 10 ms (hiện tại) | 799 | 99 | 100 Hz |
| 100 ms | 799 | 999 | 10 Hz |

Counter chạy 10 kHz. Chỉ đổi Counter Period giữa 9/99/999; giữ Internal Clock,
UP, Clock Division DIV1 và TIM2 global interrupt enabled. Giữ SYS timebase là
SysTick và bật Keep User Code when re-generating. Bảng chỉ đúng với TIM2CLK
8 MHz; kiểm tra lại Clock Configuration nếu đổi clock.

Sau Generate Code, thứ tự khởi tạo vẫn phải là MX_GPIO_Init(), MX_TIM2_Init(),
board_io_init(), timer2_init(), traffic_light_init(). Không thêm một hàm start
TIM2 hoặc callback/ISR thứ hai. Các thay đổi main.c đều nằm trong USER CODE;
những bản trùng đã bị bỏ khỏi USER CODE nên lần regenerate tiếp không giữ lại.

## Kiểm tra đã thực hiện

- Full compile/link bằng ARM GCC 9.3.1 của STM32CubeIDE, Cortex-M3,
  STM32F103x6, header/driver/startup/linker script thật, `-Wall -Werror`: PASS.
- text = 7308 B, data = 20 B, bss = 1652 B (gồm dự phòng heap/stack);
  flash text + data = 7328 B, RAM data + bss = 1672 B.
- Tái sử dụng `tools/build_ex3.ps1`; tên script/output giữ nguyên. ELF, HEX,
  BIN và MAP mới ở `Core/.build-ex3/`, đều bị Git ignore.
- ELF nm xác nhận mỗi symbol chỉ có một định nghĩa: htim2, TIM2_IRQHandler,
  HAL_TIM_Base_MspInit, HAL_TIM_PeriodElapsedCallback và SysTick_Handler.
  HAL_InitTick/HAL_GetTick vẫn là weak implementation mặc định của HAL.
- Biên dịch chính Src/traffic_light.c với mock HAL bằng host GCC, kiểm tra
  10 giây tick 1 ms và sự kiện TIM2 giả lập ở 1/10/100 ms: mỗi trường hợp
  40 cạnh, cạnh mỗi 250 ms. Kiểm tra giữ phần dư, phục hồi pha khi gọi trễ,
  và tràn uint32_t: PASS. Đây là kiểm tra logic, không mô phỏng MCU/TIM2 thật.
- Kiểm tra source: không có lời gọi HAL_Delay() trong ứng dụng; bộ đếm ngắt
  chỉ phục vụ debug, không được logic blink đọc. `git diff --check`: PASS.
- Không sửa .ioc để build thử ba cấu hình; build firmware thực tế dùng
  PSC=799, ARR=99. Hai cấu hình còn lại cần Generate Code/build/chạy thủ công.
- Chưa chạy mô phỏng Proteus hoặc đo trên phần cứng.

Build lại từ root project:

```powershell
./Core/tools/build_ex3.ps1 -ToolchainBin 'path/to/ARM/toolchain/bin'
```

## Kiểm tra thủ công STM32CubeIDE/Proteus

1. Trong CubeIDE chọn lần lượt PSC/ARR theo bảng, Generate Code và Build.
   Giữ SysTick timebase 1 ms, TIM2 NVIC enabled; không chỉnh logic blink.
2. Nếu build từ CubeIDE, bảo đảm thư mục `.build-ex3` chứa object của script
   được exclude khỏi build/source discovery nếu IDE đưa nó vào danh sách input.
   Không sử dụng ELF cũ trong Debug khi chưa build lại.
3. Trong Proteus chọn firmware mới: ELF/HEX từ lần build tương ứng; clock MCU
   8 MHz. Đối chiếu PA5 nối đúng LED đỏ, điện trở hạn dòng và cực tính LED.
4. Nối Oscilloscope kênh A vào PA5, tham chiếu GND; DC coupling, khoảng
   100 ms/div, trigger theo cạnh ở giữa mức logic LOW/HIGH. Đo giữa hai cạnh
   lên liên tiếp: 500 ms. Đo độ rộng HIGH và LOW: mỗi khoảng 250 ms.
   Với active LOW, khoảng LOW chính là thời gian LED sáng.
5. Hoặc nối Logic Analyzer vào PA5, đặt bước lấy mẫu <=1 ms, capture >=2 s;
   đo khoảng cạnh liên tiếp 250 ms, chu kỳ 500 ms, duty 50%, tần số 2 Hz.
   Quan sát ít nhất 10 giây để phát hiện trôi pha.
6. Lặp lại cả ba cấu hình: dạng sóng PA5 phải giữ nguyên. Bộ đếm
   timer2_irq_count tăng khoảng 1000/100/10 mỗi giây tương ứng. Kiểm tra
   PSC/ARR và CR1.CEN/DIER.UIE nếu bộ đếm không thay đổi.
7. Xác nhận pull-up PA2/3/4 và trạng thái blank EN/SEG của Ex3 còn nguyên;
   các LED còn lại không có logic điều khiển mới.

## Git và giới hạn

Branch làm việc hiện tại `ex3`, origin `https://github.com/Tilum06/VXL-VDK-Lab3.git`.
Working tree ban đầu sạch. Commit Ex4 dùng message được yêu cầu:
`Complete Lab3 Ex4 - Timer Configuration`. Không tạo branch, rewrite lịch sử,
force push hoặc merge. Hash/kết quả push được báo trong phản hồi hoàn tất.

Các giới hạn còn lại là phép đo Proteus/phần cứng, cực tính/dây nối schematic
thực tế, kiểm tra regenerate và hai cấu hình TIM2 còn lại. Sai số HSI ảnh hưởng
thời gian thực trên phần cứng. MSP vẫn tắt JTAG/SWD theo cấu hình hiện có;
không thay đổi chức năng Ex3 để mở debugger trong Ex4.
