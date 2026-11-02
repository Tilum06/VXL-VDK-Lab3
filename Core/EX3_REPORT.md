# Lab 3 — Ex3: GPIO and Timer

Phạm vi: hoàn thiện cấu hình GPIO runtime và TIM2, thêm .gitignore, build,
kiểm tra liên kết ngắt, commit và push riêng Ex3. Chưa triển khai Ex4–Ex10.

## GPIO

| Chân | Chức năng | Cấu hình sau khởi tạo |
| --- | --- | --- |
| PA2 | MODE | Input pull-up, nhấn LOW |
| PA3 | SET | Input pull-up, nhấn LOW |
| PA4 | SAVE | Input pull-up, nhấn LOW |
| PA5–PA7 | RED/YELLOW/GREEN tuyến 1 | Output push-pull, mapping giữ nguyên |
| PA8–PA10 | RED2/YELLOW2/GREEN2 tuyến 2 | Output push-pull, mapping giữ nguyên |
| PB0–PB6 | SEG_A–SEG_G | Output push-pull, active LOW, blank HIGH |
| PB7–PB10 | EN0–EN3 | Output push-pull, PNP active LOW, disable HIGH |

`board_io_init()` chạy ngay sau `MX_GPIO_Init()`: tắt cả bốn EN, blank segment,
rồi cấu hình lại ba input thành GPIO_PULLUP. Chưa quét hoặc hiển thị số.

Giữ nguyên `lab3.ioc`, clock hệ thống, pin mapping và code CubeMX ngoài vùng
USER CODE, ngoại trừ một dòng bắt buộc bật `HAL_TIM_MODULE_ENABLED` trong
`stm32f1xx_hal_conf.h` (file không có vùng USER CODE).

Vì giữ .ioc, giao diện CubeMX vẫn thể hiện input NOPULL và chưa có TIM2.
Cấu hình Ex3 thực tế được áp dụng bằng module ứng dụng. Khi Generate Code lại,
cần kiểm tra/bật lại dòng HAL TIM nếu bị CubeMX ghi đè; không thêm một bộ TIM2
do CubeMX sinh đồng thời với module này vì sẽ trùng handle/MSP/IRQ/callback.

## TIM2

- HSI = SYSCLK = HCLK = PCLK1 = 8 MHz, APB1 prescaler /1.
- Clock TIM2 = PCLK1 nếu APB1 /1; bằng 2 × PCLK1 nếu APB1 có chia.
- `timer2_init()` đọc PCLK1 và prescaler APB1 thực tế, tính PSC để counter 10 kHz.
- Với project hiện tại: PSC = 8,000,000 / 10,000 − 1 = **799**; ARR = **99**.
- Chu kỳ update = (799 + 1) × (99 + 1) / 8,000,000 = **0.010 s**.
- Clock source `TIM_CLOCKSOURCE_INTERNAL`, counter UP, clock division /1.
- Bật RCC TIM2, NVIC TIM2_IRQn priority 1/subpriority 0, global IRQ enabled.
- Xóa UIF và pending IRQ do bước init trước `HAL_TIM_Base_Start_IT(&htim2)`.
- `TIM2_IRQHandler()` gọi `HAL_TIM_IRQHandler(&htim2)`.
- Callback lọc TIM2 và chỉ tăng `volatile uint32_t timer2_irq_count`.
  Không đọc nút, không HAL_Delay, không blink, không accumulator Ex4.

## File thay đổi

- Thêm `Inc/board_io.h`, `Src/board_io.c`, `Inc/timer.h`, `Src/timer.c`.
- `Src/main.c`: include và hai lời gọi init trong USER CODE.
- `Src/stm32f1xx_it.c`, `Inc/stm32f1xx_it.h`: IRQ trong USER CODE.
- `Inc/stm32f1xx_hal_conf.h`: bật HAL TIM.
- `Src/input_reading.c`: cập nhật comment pull-up; giữ nguyên stub.
- Thêm `tools/build_ex3.ps1`: full build độc lập bằng toolchain ARM GCC.
- Thêm `.gitignore` ở root project, báo cáo này; cập nhật ghi chú bước khung.

## Build và kiểm tra

Đã full compile/link tất cả Core/Src, HAL driver và startup thật với linker
script STM32F103C6UX_FLASH.ld; ARM GCC 9.3.1, `-Wall -Werror`, không warning.
Kết quả: text 7044 B, data 20 B, bss 1644 B (bao gồm vùng heap/stack dự phòng).
Flash dùng text + data = 7064 B; RAM data + bss = 1664 B.

ELF, HEX, BIN và MAP nằm trong `Core/.build-ex3/`, không đưa vào Git.
Đã kiểm tra vector TIM2 trong BIN trỏ đến ISR Thumb `0x080003B1`, không trỏ
Default_Handler; map xác nhận callback và MSP lấy từ module timer của ứng dụng.

Build lại bằng PowerShell (chỉ định thư mục `tools/bin` của compiler):

```powershell
./Core/tools/build_ex3.ps1 -ToolchainBin 'path/to/ARM/toolchain/bin'
```

## Git

Repository GitHub lúc kiểm tra chưa có commit/branch; local ban đầu cũng chưa
có Git. Khởi tạo tại root STM32 project `lab3`, branch `ex3`, origin
`https://github.com/Tilum06/VXL-VDK-Lab3.git`. Commit đầu tiên chứa project
hiện có (gồm các stub của bước trước) và cấu hình Ex3, không tạo branch Ex1/Ex2.

`.gitignore` loại build, .settings/.metadata, backup/cache/temp, Proteus
workspace và log; giữ .ioc, .project/.cproject/.mxproject, source, startup,
linker script, Drivers và tài liệu. Repository mới chưa có build file trong
index, không cần git rm --cached và không xóa file làm việc.
Đã kiểm tra quy tắc ignore trên đường dẫn build/cache/backup và xác nhận
source, thiết kế .pdsprj/.dsn, thư viện Proteus .lib và báo cáo được giữ lại.

## Giới hạn và kiểm tra runtime tiếp theo

- Chưa chạy Proteus/phần cứng; build và vector check không chứng minh tần số
  ngắt đã đo được. Với clock mô phỏng 8 MHz, chạy 1 giây phải thấy bộ đếm tăng
  khoảng 100; kiểm tra PSC=799, ARR=99, CR1.CEN=1, DIER.UIE=1 và NVIC ISER.
- Kiểm tra PA2/3/4 khi thả HIGH, khi nhấn LOW; EN/SEG đều HIGH sau init.
- File thiết kế gốc tìm thấy tại `Proteus/Lab_3/lab3pdsprj.pdsprj` bên ngoài
  root Git STM32 hiện tại. Không sửa hoặc di chuyển thiết kế; chưa kiểm chứng
  đầy đủ dây nối/cực tính bằng mô phỏng. Mapping code theo yêu cầu người dùng.
- CubeMX hiện ghi LOW cho output trước khi board_io_init blank HIGH; có thể
  xuất hiện sáng thoáng lúc khởi động. Loại hoàn toàn xung này cần cập nhật
  mức khởi tạo CubeMX/.ioc ở bước được cho phép sau.
- MSP gốc tắt cả JTAG/SWD. Giữ cấu hình này; cần cân nhắc SWD nếu kiểm tra
  bằng debugger phần cứng. Không nhầm vấn đề debug với việc TIM2 chạy.
- HSI có sai số; chu kỳ 10 ms tính theo clock danh định, cần đo khi dùng mạch thật.
