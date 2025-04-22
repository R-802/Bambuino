# The Bambuino Project

**Bambuino** is an embedded espresso machine controller inspired by the [Gaggiuino Project](https://github.com/Zer0-bit/gaggiuino), reimagined to run natively on the **ESP32** platform. It features a modular architecture that cleanly separates a modern UI from robust backend logic, enabling precise, real-time control for coffee enthusiasts and tinkerers.

---

## 🔧 System Architecture

The system is built around the **ESP32** microcontroller and is structured into three main layers:

### 🖥️ Frontend UI — *Powered by [Slint](https://slint.dev/)*

A sleek, embedded-friendly user interface developed using **Slint**, a declarative UI toolkit:

- `src/ui_manager/main_ui.slint` — Slint UI definition
- `src/ui_manager/generated/` — Auto-generated C bindings from Slint files
- `src/display_driver.c` — Custom Slint renderer integration for ESP32 display output

### ⚙️ Backend — *Application Logic & Hardware Control*

This is where the real-time control and system logic lives:

- `src/main.c` — Main entry point, system setup, and PID control loops
- `src/sensor_manager/` — Modules for sensor input, filtering, and management
- `src/hardware/` — Direct control of hardware components (SSRs, dimmer, etc.)
- `src/pid_controller.c` — PID control algorithms for temperature and pressure regulation

### 🔄 UI Manager — *Bridge Between UI and Backend*

Handles interaction between the user interface and core logic:

- `src/ui_manager/ui_manager.c` — Manages UI events and syncs state
- `include/ui_manager/ui_manager.h` — Public API for external interaction with the UI layer

---

