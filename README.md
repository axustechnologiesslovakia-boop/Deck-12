# Deck-12
### axustechnologiesslovakia-boop

Deck-12 is a high-performance, 12-key mechanical macro interface featuring integrated OLED telemetry. The project focuses on a compact, ergonomic form factor for streamlined media and system control.

---

## Hardware Specifications

- **Controller**: Seeed Studio Xiao RP2040 (Dual-core ARM Cortex M0+)
- **Display**: 0.91" I2C OLED (128x32 resolution)
- **Input**: 12x Mechanical switches in a 4x3 matrix configuration
- **Connectivity**: USB-C high-speed serial interface
- **Logic Level**: 3.3V native

## Project Structure

- **Firmware/**: C++ source code optimized for the RP2040. Implements debounced switch polling and an I2C buffer for real-time display updates.
- **CAD/**: Parametric design files for the chassis. Optimized for tight tolerances, hardware clearance, and structural rigidity.
- **Case/**: Manufacturing documentation, including a standardized Bill of Materials (BOM) and assembly schematics.

## Engineering Details

### Firmware Logic
The firmware utilizes a non-blocking polling architecture to ensure low-latency input detection. The OLED display is updated via the I2C bus only when a state change is detected to reduce CPU overhead and minimize electromagnetic interference.

### Mechanical Design
The CAD files provide a precision-engineered enclosure designed to house the Xiao RP2040 and the OLED module securely. The design features:
- Integrated mounting points for the RP2040.
- Flush-mount window for the 0.91" OLED.
- Internal cable management channels.

## Bill of Materials (BOM) Summary

| Component | Description | Quantity |
| :--- | :--- | :--- |
| Xiao RP2040 | Main Microcontroller | 1 |
| 0.91" OLED | I2C Display Module | 1 |
| Mechanical Switches | Gateron/Cherry Compatible | 12 |
| Custom Chassis | 3D Printed / CNC Machined | 1 |
| 1N4148 | Signal Diodes (Matrix) | 12 |

## License

Distributed under the MIT License. See `LICENSE` for more information.
<img width="1199" height="695" alt="CAD" src="https://github.com/user-attachments/assets/a0a04b06-f18c-475b-b588-2b577d40063d" />
<img width="1107" height="841" alt="Schematic" src="https://github.com/user-attachments/assets/3d387923-0d16-48a9-8aca-dd90bc732d1a" />
<img width="641" height="605" alt="PCB" src="https://github.com/user-attachments/assets/2dee5711-7b75-4801-b87d-7502addadd6e" />
