#pragma once
#include <gui/view_dispatcher.h>
#include <stdbool.h>
#include <stddef.h>

// Posted via view_dispatcher_send_custom_event when the scan window closes.
// True result: bt_scanner_has_signal() — RSSI above ambient noise floor was detected.
// False result: 3-second window elapsed with only background noise.
#define BT_SCAN_DONE_EVENT 105

void bt_scanner_start(ViewDispatcher* dispatcher);
void bt_scanner_stop(void);
bool bt_scanner_is_active(void);
bool bt_scanner_has_signal(void);
void bt_scanner_get_hash(char* out_hash, size_t size);
