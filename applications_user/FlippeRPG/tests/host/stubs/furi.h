// Host-side stand-in for the Flipper's furi.h — just enough for the game logic to compile on a PC.
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
static inline uint32_t furi_get_tick(void) { return 0; }
