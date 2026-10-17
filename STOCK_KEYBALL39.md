# Yowkees Keyball39 with nice!nano v2 and the original PMW3360

Configuration: right central, left peripheral, SSD1306 128x32 OLED, no TRRS cable.
This branch adapts the parent configuration for the original wired Keyball39
PCB. The parent's PMW3610 configuration is not appropriate for this hardware.

## Wiring

- Matrix rows: Pro Micro 21, 20, 19, 18; columns: 4, 5, 6, 7, 8, 9.
- PMW3360 SCK: Pro Micro 15 / P1.13.
- PMW3360 MOSI: Pro Micro 16 / P0.10.
- PMW3360 MISO: Pro Micro 14 / P1.11.
- PMW3360 CS: Pro Micro 10 / P0.09.
- No MOTION wire: use polling every 8000 microseconds.
- SSD1306 128x32 OLED at I2C address 0x3c; SDA P0.17, SCL P0.20.

Original sensor schematic:
https://github.com/Yowkees/keyball/blob/main/keyball46/ball_design_data/Trackball_circuit.pdf
Original keyboard schematic:
https://github.com/Yowkees/keyball/blob/main/keyball39/design_data/Circuit_of_Keyball39_Ballside_rev2.pdf

The published sensor circuit uses a TPS73601 regulator for 1.9 V and TXS0102
level translators. Their specified input ranges permit a 3.3 V board supply.
This supports trying the original sensor without adding a 5 V boost circuit;
successful operation of the user's specific hardware still needs verification.
Do not feed 5 V directly into nice!nano GPIO pins.

## Firmware and controls

ZMK is pinned to v0.3 commit edf5c0814fd3ea202e43aad2d68fd32e882a518c.
The MIT-licensed PMW3360 driver is vendored with polling fixes; see its
LOCAL_CHANGES.md for provenance. The sensor starts at 500 CPI without SROM,
matching the stock QMK default. Right central deep sleep is disabled because
this driver does not reinitialize the sensor after a power cycle. Battery life
must be evaluated on the actual hardware.

The parent's QWERTY keymap is retained. Hold the left Space thumb key for the
mouse layer; J is left click, L is right click, and semicolon is middle click.
Hold the left thumb key next to Space for scrolling. Hold the Escape thumb key
for slow pointer movement. Hold right Backspace for symbols and BLE controls:
left A clears the selected BLE bond, S selects profile 0, D profile 1, F profile 2.

ZMK Studio over USB can edit the keymap. This branch does not implement DYA's
PMW3610-specific sensor RPC interface, so its trackball tuning UI is not supported.
Sensor CPI and input scaling are set in keyball39_right.overlay and can be
changed by rebuilding.

Flash settings_reset on BOTH halves first, then their respective left/right
firmware. Reboot both halves and pair the PC with Keyball39. Runtime keymap
changes will also be erased by settings_reset.
