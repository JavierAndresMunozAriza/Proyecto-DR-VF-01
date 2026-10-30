# Code Review Rules — DR-VF-01 firmware (C)

Test files (`*test*.c`) are exempt from the logging and magic-number rules.

## General
REJECT if:
- Hardcoded secrets or credentials
- Code duplication (violates DRY)
- Commented-out code blocks left in place
- Magic numbers for timing, pins, or thresholds → use a named `#define` or `enum`
- Return values ignored where a call can fail (allocation, driver, RTOS APIs)

## Architecture
REJECT if:
- `geofence.c` or `geofence.h` includes or calls ESP-IDF, Zephyr, or FreeRTOS APIs; the engine must stay pure C and testable on a host
- Task or FSM logic calls hardware or SDK APIs directly instead of going through the HAL or a driver
- A peripheral driver (LoRa, GNSS, IMU) does not expose the POSIX-style interface (`open`, `read`, `write`, `ioctl`, `test`)
- Main control flow is not an explicit FSM
- Non-`static` global state where a file-local variable or encapsulated struct would do
- Firmware logs through raw `printf` instead of the leveled log layer (INFO, DEBUG, WARNING, ERROR)

## ISR context and concurrency
REJECT if:
- Blocking calls, dynamic allocation, or logging inside an ISR
- Variables shared between an ISR and a task are not `volatile` or protected
- Resources shared between tasks are accessed without a mutex, semaphore, or queue

## Real-time and power
REJECT if:
- Busy-wait loops in periodic tasks without justification
- Blocking I/O or unbounded loops in the polygon check, which must take under 50 ms (RF-02)
- Polling or timers keep the MCU awake where the IMU wake interrupt would do

## Style
PREFER:
- `static` for file-local functions and variables
- Fixed-width types (`uint32_t`, `int16_t`) for hardware and timing values
