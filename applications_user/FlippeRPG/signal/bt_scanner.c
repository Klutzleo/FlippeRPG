#include "bt_scanner.h"
#include <furi_hal_bt.h>
#include <furi.h>
#include <stdio.h>
#include <string.h>

// 3-second listen window across all three BLE advertising channels
#define BT_SCAN_TIMEOUT_MS   3000
#define BT_POLL_INTERVAL_MS  50

// Rotate channels every 300ms → ~10 samples per channel per rotation
#define BT_CHANNEL_DWELL_MS  300

// Any RSSI above this floor means something is actively broadcasting nearby.
// Background noise / no signal reads as -127.0. BLE devices typically read -40 to -90.
#define BT_RSSI_THRESHOLD_DBM  (-90.0f)

// BLE advertising channels by ACI RF channel number:
//   37 = 2402 MHz → ACI channel 0
//   38 = 2426 MHz → ACI channel 12
//   39 = 2480 MHz → ACI channel 39
static const uint8_t BT_ADV_CHANNELS[] = {0, 12, 39};
#define BT_ADV_CHANNEL_COUNT 3

static volatile bool bt_active    = false;
static volatile bool bt_had_signal = false;
static ViewDispatcher* bt_vd      = NULL;
static char bt_last_hash[32]      = {0};
static FuriThread* bt_thread      = NULL;

// ---- Scan thread ----

static int32_t bt_scan_thread(void* context) {
    (void)context;

    // Bail if BT radio stack isn't running — can't scan
    if(!furi_hal_bt_is_alive()) {
        if(bt_active && bt_vd) {
            view_dispatcher_send_custom_event(bt_vd, BT_SCAN_DONE_EVENT);
        }
        return 0;
    }

    float max_rssi = -127.0f;
    uint32_t start = furi_get_tick();
    uint32_t channel_start = start;
    int ch_index = 0;

    furi_hal_bt_start_rx(BT_ADV_CHANNELS[ch_index]);

    while(bt_active && (furi_get_tick() - start) < BT_SCAN_TIMEOUT_MS) {
        furi_delay_ms(BT_POLL_INTERVAL_MS);

        float rssi = furi_hal_bt_get_rssi();
        if(rssi > max_rssi) max_rssi = rssi;

        // Rotate to next BLE advertising channel
        if((furi_get_tick() - channel_start) >= BT_CHANNEL_DWELL_MS) {
            furi_hal_bt_stop_rx();
            ch_index = (ch_index + 1) % BT_ADV_CHANNEL_COUNT;
            furi_hal_bt_start_rx(BT_ADV_CHANNELS[ch_index]);
            channel_start = furi_get_tick();
        }
    }

    furi_hal_bt_stop_rx();

    if(bt_active && bt_vd) {
        if(max_rssi > BT_RSSI_THRESHOLD_DBM) {
            // Something was broadcasting — encode RSSI + tick as ephemeral hash
            uint16_t rssi_part = (uint16_t)((max_rssi + 127.0f) * 100.0f);
            uint16_t tick_part = (uint16_t)(furi_get_tick() & 0xFFFF);
            snprintf(bt_last_hash, sizeof(bt_last_hash), "BT:%04X%04X", rssi_part, tick_part);
            bt_had_signal = true;
        }
        // bt_had_signal stays false if only background noise was detected
        view_dispatcher_send_custom_event(bt_vd, BT_SCAN_DONE_EVENT);
    }

    return 0;
}

// ---- Public API ----

void bt_scanner_start(ViewDispatcher* dispatcher) {
    if(bt_active) return;

    bt_vd          = dispatcher;
    bt_had_signal  = false;
    bt_last_hash[0] = '\0';
    bt_active      = true;

    bt_thread = furi_thread_alloc_ex("BTScanThread", 2048, bt_scan_thread, NULL);
    furi_thread_start(bt_thread);
}

void bt_scanner_stop(void) {
    if(!bt_active) return;
    bt_active = false;

    if(bt_thread) {
        furi_thread_join(bt_thread);
        furi_thread_free(bt_thread);
        bt_thread = NULL;
    }
    bt_vd = NULL;
}

bool bt_scanner_is_active(void) {
    return bt_active;
}

bool bt_scanner_has_signal(void) {
    return bt_had_signal;
}

void bt_scanner_get_hash(char* out_hash, size_t size) {
    strncpy(out_hash, bt_last_hash, size);
    if(size > 0) out_hash[size - 1] = '\0';
}
