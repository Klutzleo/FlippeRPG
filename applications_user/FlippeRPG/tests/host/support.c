// Symbols the game logic expects from the rest of the app / the Flipper.
#include <stdint.h>

uint32_t fake_now = 0; // seconds; set by each test

void echo_event(int state) { (void)state; }
