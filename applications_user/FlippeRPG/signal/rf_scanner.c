#include "rf_scanner.h"
#include <lib/subghz/devices/devices.h>
#include <lib/subghz/devices/cc1101_int/cc1101_int_interconnect.h>
#include <lib/subghz/devices/preset.h>
#include <furi.h>
#include <stdio.h>
#include <string.h>

// 433.92 MHz — most common ISM band (garage doors, weather stations, key fobs)
#define RF_SCAN_FREQUENCY 433920000
// 15 samples × 100ms = 1.5 second listen window
#define RF_SCAN_SAMPLES 15

static volatile bool rf_active = false;
static ViewDispatcher* rf_vd   = NULL;
static char rf_last_hash[32]   = {0};
static FuriThread* rf_thread   = NULL;

// Runs in its own FuriThread — scans CC1101 for RF activity via RSSI.
// Hash is ephemeral (RSSI + tick fragment): RF signals have no UID by design.
static int32_t rf_scan_thread(void* context) {
    (void)context;

    subghz_devices_init();
    const SubGhzDevice* device = subghz_devices_get_by_name(SUBGHZ_DEVICE_CC1101_INT_NAME);

    float max_rssi = -130.0f;

    if(device && subghz_devices_begin(device)) {
        subghz_devices_load_preset(device, FuriHalSubGhzPresetOok650Async, NULL);
        subghz_devices_set_frequency(device, RF_SCAN_FREQUENCY);
        subghz_devices_set_rx(device);

        for(int i = 0; i < RF_SCAN_SAMPLES && rf_active; i++) {
            furi_delay_ms(100);
            float rssi = subghz_devices_get_rssi(device);
            if(rssi > max_rssi) max_rssi = rssi;
        }

        subghz_devices_idle(device);
        subghz_devices_end(device);
    }

    subghz_devices_deinit();

    if(rf_active && rf_vd) {
        uint16_t rssi_part = (uint16_t)((max_rssi + 130.0f) * 100.0f);
        uint16_t tick_part = (uint16_t)(furi_get_tick() & 0xFFFF);
        snprintf(rf_last_hash, sizeof(rf_last_hash), "RF:%04X%04X", rssi_part, tick_part);
        view_dispatcher_send_custom_event(rf_vd, RF_SCAN_DONE_EVENT);
    }

    return 0;
}

void rf_scanner_start(ViewDispatcher* dispatcher) {
    if(rf_active) return;

    rf_vd           = dispatcher;
    rf_last_hash[0] = '\0';
    rf_active       = true;

    rf_thread = furi_thread_alloc_ex("RFScanThread", 2048, rf_scan_thread, NULL);
    furi_thread_start(rf_thread);
}

void rf_scanner_stop(void) {
    if(!rf_active) return;
    rf_active = false; // Thread sees this on next 100ms wake and exits early

    if(rf_thread) {
        furi_thread_join(rf_thread); // Blocks ≤100ms for thread to exit
        furi_thread_free(rf_thread);
        rf_thread = NULL;
    }
    rf_vd = NULL;
}

bool rf_scanner_is_active(void) {
    return rf_active;
}

void rf_scanner_get_hash(char* out_hash, size_t size) {
    strncpy(out_hash, rf_last_hash, size);
    if(size > 0) out_hash[size - 1] = '\0';
}
