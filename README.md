BLETeensy -- BLE for Teensy 4.x + Murata Type 1YN (CYW43439)
Copyright (c) 2026 Earle F. Philhower, III.  All rights reserved.
LGPL licensed, see headers.

This library lets a Teensy 4.x board become a BLE server (i.e. the thing
your phone or PC connect to, like a BLE heart rate monitor) or a BLE client
(the thing that connects out to remote devices).

It's a port of Earle Philhower's `BLE` library from
[arduino-pico](http://github.com/earlephilhower/arduino-pico) -- originally
written against the Pico SDK's BTstack integration for the Pico W's onboard
CYW43439 -- retargeted to run on top of the
[BTstackTeensy](../BTstackTeensy) library instead, which provides the same
underlying BlueKitchen BTstack HCI/GAP/GATT stack for a Teensy 4.x + Murata
Type 1YN (CYW43439) module. The public `BLE`/`BLEServer`/`BLEClient`/
`BLECharacteristic`/... API is unchanged from the Pico version; it tries to
be relatively compatible with the ESP32 BLE and Arduino.cc ArduinoBLE
libraries, but isn't guaranteed to be so.

## Requirements

- A Teensy 4.x board (Teensy 4.1 or 4.0) wired to a Murata Type 1YN
  (CYW43439) module as documented in BTstackTeensy's README -- HCI UART on
  Serial8 (pins 33-36 by default) plus the BT_ON pin.
- The [BTstackTeensy](../BTstackTeensy) library installed alongside this one
  (`depends=BTstackTeensy` in `library.properties`); this library includes
  BTstackTeensy's raw BTstack headers (`btstack.h`, `hci.h`, `gap.h`,
  `ble/sm.h`, ...) directly rather than going through BTstackTeensy's own
  `BTstackTeensyManager`/`BTstackTeensy` wrapper classes, so both libraries
  can be installed side by side without any symbol clashes.

## Porting note: `BLE.update()`

On the Pico W, BTstack runs on the second core via the Pico SDK's
`async_context`, so the Arduino sketch's `loop()` never needs to service it.
Teensy 4.x has no such background driver -- BTstackTeensy polls its HCI UART
and BTstack's run loop from a function that must be called every iteration
of `loop()` (see `BTstackTeensy::loop()`). This port adds that same
requirement here as `BLE.update()`: **call `BLE.update()` once per `loop()`
iteration** (see the examples) or BLE processing -- including `BLE.begin()`
itself, which blocks until the local address is known -- will stall.

## Examples

- `Beacon`, `BLEClientDemo`, `CharacteristicOnWrite`, `CustomService`,
  `Scan`, `SerialBLE` -- ported from the original Pico examples; button/ID
  calls specific to the RP2040 (`BOOTSEL`, `pico_get_unique_board_id()`,
  `rp2040.getFreeHeap()`) were swapped for Teensy-portable equivalents.
# bleteensy
