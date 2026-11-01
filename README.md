# MeshCore T114 Pager

A focused MeshCore firmware for the **Heltec Mesh Node T114** with display and three-button navigation.

Current project firmware version: **v0.1.0**.

This repository is intentionally reduced to the parts needed for the T114 build used in this project. The goal is a small standalone MeshCore device that can be operated without a phone for quick/canned messages.

## Features

- Heltec Mesh Node T114 / nRF52840
- SX1262 LoRa radio
- ST7789 display with UTF-8 support for German umlauts and Latin-1 characters
- MeshCore Companion firmware over BLE
- USB CDC companion access in parallel with BLE
- Canned / quick messages stored in LittleFS (`/canned.txt`)
- Default canned-message list created on first boot
- Three-button navigation
- Standalone channel-message history on the display (last 20 messages, RAM in v0.1.0)
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

The buttons are used to navigate channels, the received-message history and canned messages, and to confirm selections.

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

For this T114 bootloader, the reliable update path is UF2 rather than a serial/nrfutil upload.

Build the firmware first:

```powershell
py -3.13 -m platformio run `
  -e Heltec_t114_companion_radio_ble_canned
```

Then generate the UF2 image:

```powershell
py -3.13 -m platformio run `
  -e Heltec_t114_companion_radio_ble_canned `
  -t create_uf2
```

The UF2 file is created here:

```text
.pio\build\Heltec_t114_companion_radio_ble_canned\firmware.uf2
```

When the T114 is in its UF2 bootloader, Windows exposes a removable drive named `HT-n5262`. Copy `firmware.uf2` to that drive. The bootloader may reboot the board as soon as the image has been accepted, so the drive can disappear immediately after or during the apparent end of the copy operation.

USB companion access after flashing:

```powershell
meshcore-cli -s COM4 -i
```

Replace `COM4` with the actual USB serial port. BLE remains available at the same time.

### Enter the bootloader over USB

After this firmware has been flashed once, the enclosure no longer needs to be opened for normal firmware updates.

Reboot the running T114 into the nRF52840 UF2/DFU bootloader with:

```powershell
py tools\bootloader.py COM4
```

Replace `COM4` with the T114's current application COM port.

The helper sends the T114-specific companion command and the firmware then calls the Adafruit nRF52 core's `enterUf2Dfu()` reset path. The application COM port disappears during the reset. In UF2 mode, Windows normally exposes the removable drive:

```text
HT-n5262
```

Then copy:

```text
.pio\build\Heltec_t114_companion_radio_ble_canned\firmware.uf2
```

to that drive.

If the software bootloader entry is not yet available because an older firmware is installed, the hardware fallback is to enter the T114 bootloader manually. On the tested HT-n5262 bootloader, a successful UF2 bootloader reports:

```text
Model: HT-n5262
Board-ID: HT-n5262
SoftDevice: S140 6.1.1
```

The bootloader can also display `BLEOTA`; that mode by itself does not provide a normal application serial port.

### UTF-8 and umlauts on the ST7789

MeshCore messages arrive as UTF-8. The bundled Arial bitmap fonts on this firmware are indexed with single-byte Latin-1 code points, so the ST7789 path performs one explicit UTF-8 -> Latin-1 conversion before drawing.

The ST7789 driver intentionally bypasses the generic `translateUTF8ToBlocks()` CP437 conversion used by other display paths. Without that override, German umlauts are corrupted before they reach the font renderer.

The tested characters include:

```text
Ä Ö Ü ä ö ü ß
```

Example test string:

```text
ÄÖÜ äöü ß Grüße aus Kärnten
```

### Manage canned messages over USB

The T114 Pager adds a small companion-protocol extension for maintaining `/canned.txt`.
The helper client is included in this repository:

```powershell
py tools\canned_cli.py COM4 list
py tools\canned_cli.py COM4 add "Bin in 10 Minuten da"
py tools\canned_cli.py COM4 del 2
py tools\canned_cli.py COM4 reload
```

The list supports up to 20 messages, with up to 40 UTF-8 bytes per message. Changes are saved to LittleFS immediately.

Canned messages can include live GPS placeholders which are expanded only when the message is sent:

```text
{GPS}  -> latitude,longitude
{LAT}  -> latitude
{LON}  -> longitude
```

Example:

```text
Brauche Hilfe, Position {GPS}
```

If a message contains a GPS placeholder but the T114 does not currently have a valid GPS fix, the send is cancelled and the display shows `No GPS fix`. Coordinates are formatted with six decimal places. The stored template still uses the normal 40-byte limit; the expanded outgoing message may be up to 120 bytes.

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

This repository contains T114-specific changes including nRF52 LittleFS support, T114 build fixes, default canned messages, three-button navigation, simultaneous BLE/USB companion access, software entry into the UF2 bootloader, and ST7789 UTF-8/umlaut handling used by this hardware build.

## License

This project remains under the same **MIT License** as MeshCore. See [`license.txt`](license.txt).

Please also refer to the upstream MeshCore project for the original project history, contributors and documentation.
