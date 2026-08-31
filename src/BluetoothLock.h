/*
    BluetoothLock - RAII guard against BTstack processing racing the sketch

    On the Pico W, the original arduino-pico BLE library used this to guard
    calls into BTstack from the Arduino sketch's core against BTstack's own
    processing, which the Pico SDK runs asynchronously on the second core.

    BTstackTeensy has no such second core or background driver: BTstack is
    entirely polled, and only ever runs synchronously inside BLE::update()
    (see BLE.cpp), which the sketch's loop() calls directly. There is never
    a concurrent call into BTstack while the sketch is doing something else,
    so this guard has nothing to do here -- it's kept as a no-op so the rest
    of the library (written against the Pico version) doesn't need to change
    at every "BluetoothLock lock;" call site.
*/

#pragma once

struct BluetoothLock {
    BluetoothLock() {}
    ~BluetoothLock() {}
};
