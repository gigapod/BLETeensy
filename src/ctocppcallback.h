/*
    ctocppcallback.h - Turn a bound C++ member function into a plain C
    function pointer BTstack's C API can take as a callback.

    The original arduino-pico BLE library got this from the RP2040 Arduino
    core. Teensyduino has no equivalent, so this is a local, minimal
    reimplementation of the same trick: define CCALLBACKNAME to a unique
    name, #include this file, then use CCALLBACKNAME<Signature, __COUNTER__>
    at each call site needing a trampoline (see BLE.cpp and BLECB.h for the
    macros built on top of this).

    Each (name, __COUNTER__) pair gets its own static std::function slot and
    static trampoline function, so distinct macro call sites don't collide.
    A single call site only supports one outstanding bound callback at a
    time -- re-invoking the macro before the previous callback fires
    overwrites the slot. That's fine for every call site in this library:
    BTstackTeensy's btstack_config.h caps outstanding GATT client operations
    at one (MAX_NR_GATT_CLIENTS), and the singleton-style dispatchers (e.g.
    BLEClass::packetHandler) only ever register once, at BLE.begin()/
    BLE.startAdvertising().
*/

#pragma once

#include <functional>

#ifndef CCALLBACKNAME
#error "Define CCALLBACKNAME to a unique name before including ctocppcallback.h"
#endif

template<typename T, int uid>
class CCALLBACKNAME;

template<typename Ret, typename... Args, int uid>
class CCALLBACKNAME<Ret(Args...), uid> {
public:
    static std::function<Ret(Args...)> func;

    static Ret callback(Args... args) {
        return func(args...);
    }
};

template<typename Ret, typename... Args, int uid>
std::function<Ret(Args...)> CCALLBACKNAME<Ret(Args...), uid>::func;
