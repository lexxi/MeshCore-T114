# MeshCore T114 Pager

A focused MeshCore firmware for the **Heltec Mesh Node T114** with display and three-button navigation.

This repository is intentionally reduced to the parts needed for the T114 build used in this project. The goal is a small standalone MeshCore device that can be operated without a phone for quick/canned messages.

## Features

- Heltec Mesh Node T114 / nRF52840
- SX1262 LoRa radio
- ST7789 display
- MeshCore Companion firmware over BLE
- Canned / quick messages stored in LittleFS (`/canned.txt`)
- Default canned-message list created on first boot
- Three-button navigation
- GPS support retained
- EU/UK Narrow radio preset used by the project

Current radio settings:

```text
Frequency: 869.618 MHz
Bandwidth: 62.5 kHz
Spreading Factor: SF8
Coding Rate: 8
TX power: 22 dBm
```

## Three-button controls

The firmware uses three external buttons connected between the GPIO and GND. Internal pull-ups are enabled, so no external pull-up resistors are required.

```text
UP    -> GPIO9
DOWN  -> GPIO10
OK    -> GPIO16
GND   -> common ground
```

The buttons are used to navigate channels and canned messages and to confirm selections.

## Default canned messages

If `/canned.txt` does not exist on first boot, the firmware creates it with:

```text
Bin unterwegs
Alles OK
Bitte melden
Komme spaeter
Brauche Hilfe
```

The firmware currently supports up to 20 canned messages with up to 40 characters per message.

## Build

This project uses PlatformIO. The active environment is:

```text
Heltec_t114_companion_radio_ble_canned
```

On Windows with Python 3.13 and PlatformIO installed:

```powershell
py -3.13 -m platformio run `
  -e Heltec_t114_companion_radio_ble_canned
```

Flash the connected T114 with:

```powershell
py -3.13 -m platformio run `
  -e Heltec_t114_companion_radio_ble_canned `
  -t upload
```

## Repository scope

This is **not intended to replace the upstream MeshCore repository**. It is a specialized T114 fork kept deliberately small for this hardware project.

The upstream project contains support for many additional boards, roles, build environments and features that have intentionally been removed here.

## Upstream and acknowledgements

This project is based on **MeshCore** and would not exist without the work of the MeshCore developers and community:

- MeshCore upstream: https://github.com/meshcore-dev/MeshCore
- MeshCore documentation: https://docs.meshcore.io

The canned-message implementation used as the starting point for this T114 port comes from the `Bandit_canned_message` work by **gjelsoe**:

- https://github.com/gjelsoe/MeshCore/tree/Bandit_canned_message

Many thanks to the MeshCore developers, contributors and community, and especially to **gjelsoe** for the canned-message work that made this T114 adaptation possible.

This repository contains T114-specific changes including nRF52 LittleFS support, T114 build fixes, default canned messages, and the three-button navigation used by this hardware build.

## License

This project remains under the same **MIT License** as MeshCore. See [`license.txt`](license.txt).

Please also refer to the upstream MeshCore project for the original project history, contributors and documentation.
