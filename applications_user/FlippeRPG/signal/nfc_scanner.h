#pragma once
#include <gui/view_dispatcher.h>
#include <stdbool.h>
#include <stddef.h>

// Posted via view_dispatcher_send_custom_event when the scan window closes.
// True result: nfc_scanner_has_signal() && nfc_scanner_get_hash().
// False result: timeout with no card in field.
#define NFC_SCAN_DONE_EVENT 104

void nfc_scanner_start(ViewDispatcher* dispatcher);
void nfc_scanner_stop(void);
bool nfc_scanner_is_active(void);
bool nfc_scanner_has_signal(void);
void nfc_scanner_get_hash(char* out_hash, size_t size);
