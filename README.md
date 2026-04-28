# Deck-12
### axustechnologiesslovakia-boop

Deck-12 is a 12-key macro interface for Spotify control and system automation.

## Architecture
- **/App**: Node.js/Electron runtime for media control and macros.
- **/Firmware**: C++ source for Xiao RP2040 and OLED integration.
- **/CAD**: Parametric chassis design files.
- **/Case**: Bill of Materials and assembly documentation.

## Setup
1. Flash `/Firmware` to the microcontroller.
2. In `/App`, run `npm install`.
3. Configure `config.json` with Spotify API credentials.
4. Launch via `npm start`.

## License
MIT
