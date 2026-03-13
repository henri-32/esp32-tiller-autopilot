# Autopilot Boot

STM32/Native-Projekt fuer eine adaptive Trimmregelung mit klar getrennten
Schichten fuer Sensorik, Entscheidungslogik und Aktorik.

## Relevante PlatformIO Environments

- `genericSTM32F411RE`: Embedded Target (STM32Cube)
- `native_csc`: CSC Unit-Tests (Unity)
- `sim`: Native Simulation
- `gui`: Native Lauf fuer System-Loop + UI-Stubs
- `gui_sim`: GUI mit Simulationsanbindung

## Kurzbefehle

- Tests: `pio test -e native_csc`
- Simulation bauen: `pio run -e sim`
- GUI bauen: `pio run -e gui`

## Architektur-Dokumente

- [Architektur](include/READMES/Architektur.md)
- [Steering Controller Config](include/READMES/SteeringControllerConfig.md)

## Third-Party

- ASCII 16x16 bitmap font in `include/ui/fonts.h` is rendered from DejaVu Sans Mono (`/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf`): https://dejavu-fonts.github.io/
- License: DejaVu Fonts License (Bitstream Vera derivative, permissive)
