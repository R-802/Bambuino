# Bambuino Control System with Slint UI

This project is an ESP32-S3 based control system for temperature, pressure, and flow control, with a modern Slint-based user interface.

## Architecture

The system has a clear separation between frontend UI and backend functionality:

### Frontend UI (Slint)

The UI is built using [Slint](https://slint.dev/), a modern UI framework for embedded systems:

- **src/ui_manager/main_ui.slint**: Declarative UI definition using Slint language
- **src/ui_manager/generated/**: Contains generated C bindings from Slint files
- **src/display_driver.c**: Contains the Slint renderer implementation for ESP32

### Backend (Application Logic)

The application logic is separate from the UI:

- **src/main.c**: Main application entry point and PID control loops
- **src/sensor_manager/**: Sensor reading and data management
- **src/hardware/**: Hardware control for SSRs, dimmer, and other peripherals
- **src/pid_controller.c**: PID control implementation

### UI Manager (Bridge)

The UI Manager acts as a bridge between frontend and backend:

- **src/ui_manager/ui_manager.c**: Connects UI events to application logic
- **include/ui_manager/ui_manager.h**: Public API for UI management
