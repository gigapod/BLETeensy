// Simple debug option

#pragma once

#if !defined(DEBUG_BLETEENSY_PORT) || !defined(DEBUG_BLETEENSY)
#define DEBUGBLE(...) do { } while(0)
#else
#define DEBUGBLE(fmt, ...) do { DEBUG_BLETEENSY_PORT.printf(fmt, ## __VA_ARGS__); DEBUG_BLETEENSY_PORT.flush(); } while (0)
#endif

