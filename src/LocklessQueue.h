/*
    LocklessQueue - minimal single-producer/single-consumer ring buffer

    The original arduino-pico BLE library got this from the RP2040 Arduino
    core, where it needed real cross-core safety (BTstack runs on the second
    core). Teensyduino has no equivalent, and doesn't need one: BTstackTeensy
    is fully polled from BLE::update() (see BLE.cpp), so there is never a
    concurrent reader/writer to guard against on Teensy -- this is a plain
    ring buffer with the same API surface, used as a drop-in replacement.
*/

#pragma once

#include <stddef.h>

template<typename T>
class LocklessQueue {
public:
    explicit LocklessQueue(size_t size) : _size(size + 1), _head(0), _tail(0) {
        _buff = new T[_size];
    }

    ~LocklessQueue() {
        delete[] _buff;
    }

    bool write(T v) {
        size_t next = (_head + 1) % _size;
        if (next == _tail) {
            return false; // full
        }
        _buff[_head] = v;
        _head = next;
        return true;
    }

    bool read(T *v) {
        if (_tail == _head) {
            return false; // empty
        }
        *v = _buff[_tail];
        _tail = (_tail + 1) % _size;
        return true;
    }

    bool peek(T *v) {
        if (_tail == _head) {
            return false; // empty
        }
        *v = _buff[_tail];
        return true;
    }

    int available() {
        return (int)((_head + _size - _tail) % _size);
    }

private:
    T *_buff;
    size_t _size;
    size_t _head;
    size_t _tail;
};
