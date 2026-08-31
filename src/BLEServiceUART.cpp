/*
    BLEServiceUART - Implements simpleNordic BLE SPP with auto-flush timeout
    Copyright (c) 2026 Earle F. Philhower, III.  All rights reserved.

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/



#include <Arduino.h>
#include <btstack.h>
#include "BLEServiceUART.h"
#include <ble/att_db_util.h>
#include <btstack_run_loop.h>
#include <list>
#include "LocklessQueue.h"
#include "BluetoothLock.h"

BLEServiceUART::BLEServiceUART(int rxbuff, int txbuff)
    : BLEService(BLEUUID(SERVICE_UUID)) {
    _rx = new BLECharacteristic(BLEUUID(CHARACTERISTIC_UUID_RX), BLEWrite, "Teensy RX Port");
    _rx->setCallbacks(this);
    _tx = new BLECharacteristic(BLEUUID(CHARACTERISTIC_UUID_TX), BLERead | BLENotify, "Teensy TX Port");
    _tx->setCallbacks(this);
    addCharacteristic(_rx);
    addCharacteristic(_tx);
    _rxQueue = new LocklessQueue<uint8_t>(rxbuff);
    _txQueue = new LocklessQueue<uint8_t>(txbuff);
    _overflow = false;
    _txSize = txbuff;
    _flushPending = false;
    _flushTimeout = 0;
    _lastFlush = millis();
    btstack_run_loop_set_timer_handler(&_flushTimer, _flushTimerCB);
    btstack_run_loop_set_timer_context(&_flushTimer, this);
}

BLEServiceUART::~BLEServiceUART() {
    BluetoothLock b;
    delete _rx;
    delete _tx;
    delete _rxQueue;
    delete _txQueue;
    btstack_run_loop_remove_timer(&_flushTimer);
}

void BLEServiceUART::begin() {
}

void BLEServiceUART::begin(unsigned long baud) {
    (void)baud;
    begin();
}

void BLEServiceUART::begin(unsigned long baud, uint16_t cfg) {
    (void)baud;
    (void)cfg;
    begin();
}

void BLEServiceUART::end() {
    // No panic() on Teensyduino (unlike the Pico SDK) -- this really is
    // unsupported (there's no real UART hardware here to shut down), so
    // fail loudly instead of silently doing nothing.
    DEBUGBLE("BLEServiceUART::end() is unsupported\n");
    abort();
}

void BLEServiceUART::setAutoflush(uint32_t ms) {
    _flushTimeout = ms;
}

BLEServiceUART::operator bool() {
    if (con_handle != 0) {
        return true;
    } else {
        return false;
    }
}

int BLEServiceUART::read() {
    uint8_t ret;
    if (_rxQueue->read(&ret)) {
        return ret;
    } else {
        return -1;
    }
}

int BLEServiceUART::peek() {
    uint8_t ret;
    if (_rxQueue->peek(&ret)) {
        return ret;
    } else {
        return -1;
    }
}

int BLEServiceUART::available() {
    return _rxQueue->available();
}

bool BLEServiceUART::overflow() {
    BluetoothLock b;
    bool ovf = _overflow;
    _overflow = false;
    return ovf;
}

size_t BLEServiceUART::write(uint8_t c) {
    if (_txQueue->write(c)) {
        // We can just buffer it up for now, but set reminder alarm
        BluetoothLock b;
        if ((_flushTimeout > 0) && !_flushPending) {
            btstack_run_loop_set_timer(&_flushTimer, _flushTimeout);
            btstack_run_loop_add_timer(&_flushTimer);
            _flushPending = true;
        }
        return 1;
    }
    // Write buffer is full, update characteristic
    flush();
    return _txQueue->write(c);
}

void BLEServiceUART::flush() {
    btstack_run_loop_remove_timer(&_flushTimer);

    _flushPending = false;
    uint8_t b[_txSize];
    size_t len = 0;
    while (len < _txSize) {
        uint8_t r;
        if (!_txQueue->read(&r)) {
            break;
        }
        b[len++] = r;
    }
    _tx->setValue(b, len);
    _lastFlush = millis();
}

// The host has sent us data...
void BLEServiceUART::onWrite(BLECharacteristic *c) {
    if (c != _rx) {
        return;  // Shouldn't ever happen
    }
    auto len = c->valueLen();
    const char *data = (const char *)c->valueData();
    for (size_t off = 0; off < len; off++) {
        if (!_rxQueue->write(data[off])) {
            _overflow = true;
        }
    }
}
