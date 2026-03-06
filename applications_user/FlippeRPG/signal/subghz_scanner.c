#include "subghz_scanner.h"
#include <lib/subghz/environment.h>
#include <lib/subghz/receiver.h>
#include <lib/subghz/protocols/base.h>
#include <lib/subghz/subghz_protocol_registry.h>
#include <lib/subghz/subghz_worker.h>
#include <lib/subghz/devices/devices.h>
#include <lib/subghz/devices/cc1101_int/cc1101_int_interconnect.h>
#include <lib/subghz/devices/preset.h>
#include <furi.h>
#include <string.h>
#include <stdio.h>

// 433.92 MHz — most common ISM band for OOK remotes worldwide
#define SUBGHZ_SCAN_FREQUENCY  433920000
// 8-second window — matches IR scanner window
#define SUBGHZ_SCAN_TIMEOUT_MS 8000
#define SUBGHZ_POLL_INTERVAL_MS 50

static volatile bool sg_active    = false;
static volatile bool sg_msg_ready = false;
static ViewDispatcher* sg_vd      = NULL;
static char sg_last_hash[32]      = {0};
static bool sg_had_signal         = false;
static FuriThread* sg_thread      = NULL;

// ---- Receiver callbacks (run in SubGhzWorker thread context) ----

// Pair callback: worker feeds raw pulses here → receiver decodes protocol
static void sg_pair_callback(void* ctx, bool level, uint32_t duration) {
    SubGhzReceiver* receiver = ctx;
    subghz_receiver_decode(receiver, level, duration);
}

// RX callback: fires when a complete decodable protocol frame is found
static void sg_rx_callback(
    SubGhzReceiver* recv,
    SubGhzProtocolDecoderBase* decoder_base,
    void* ctx) {
    (void)recv;
    (void)ctx;

    if(sg_msg_ready) return; // Only capture the first frame

    const char* proto = decoder_base->protocol ? decoder_base->protocol->name : "SUB";
    uint8_t hash_byte = subghz_protocol_decoder_base_get_hash_data(decoder_base);

    snprintf(sg_last_hash, sizeof(sg_last_hash), "%.10s:%02X", proto, hash_byte);
    sg_msg_ready = true;

    if(sg_vd) {
        view_dispatcher_send_custom_event(sg_vd, SUBGHZ_SCAN_DONE_EVENT);
    }
}

// ---- Scan thread ----

static int32_t sg_scan_thread(void* context) {
    (void)context;

    // Environment and receiver
    SubGhzEnvironment* env = subghz_environment_alloc();
    subghz_environment_set_protocol_registry(env, &subghz_protocol_registry);

    SubGhzReceiver* receiver = subghz_receiver_alloc_init(env);
    subghz_receiver_set_filter(receiver, SubGhzProtocolFlag_Decodable);
    subghz_receiver_set_rx_callback(receiver, sg_rx_callback, NULL);

    // Worker: bridges CC1101 hardware pulses to the receiver
    SubGhzWorker* worker = subghz_worker_alloc();
    subghz_worker_set_pair_callback(worker, sg_pair_callback);
    subghz_worker_set_context(worker, receiver);

    // CC1101 device setup
    subghz_devices_init();
    const SubGhzDevice* device = subghz_devices_get_by_name(SUBGHZ_DEVICE_CC1101_INT_NAME);

    if(device && subghz_devices_begin(device)) {
        subghz_devices_reset(device);
        furi_delay_ms(50); // Let hardware settle after reset
        subghz_devices_load_preset(device, FuriHalSubGhzPresetOok650Async, NULL);
        subghz_devices_set_frequency(device, SUBGHZ_SCAN_FREQUENCY);

        subghz_worker_start(worker);
        subghz_devices_set_rx(device);
        subghz_devices_start_async_rx(device, subghz_worker_rx_callback, worker);

        // Wait for a signal or timeout
        uint32_t start = furi_get_tick();
        while(sg_active && !sg_msg_ready) {
            furi_delay_ms(SUBGHZ_POLL_INTERVAL_MS);
            if(furi_get_tick() - start > SUBGHZ_SCAN_TIMEOUT_MS) break;
        }

        // Tear down hardware before freeing software
        subghz_devices_stop_async_rx(device);
        subghz_worker_stop(worker);
        subghz_devices_idle(device);
        subghz_devices_end(device);
    }

    subghz_devices_deinit();
    subghz_worker_free(worker);
    subghz_receiver_free(receiver);
    subghz_environment_free(env);

    if(sg_active) {
        sg_had_signal = sg_msg_ready;
        if(!sg_msg_ready) {
            // Timeout with no signal — fire event so UI can clean up
            view_dispatcher_send_custom_event(sg_vd, SUBGHZ_SCAN_DONE_EVENT);
        }
        // If sg_msg_ready, the event was already sent from sg_rx_callback
    }

    return 0;
}

// ---- Public API ----

void subghz_scanner_start(ViewDispatcher* dispatcher) {
    if(sg_active) return;

    sg_vd          = dispatcher;
    sg_had_signal  = false;
    sg_msg_ready   = false;
    sg_last_hash[0] = '\0';
    sg_active      = true;

    sg_thread = furi_thread_alloc_ex("SGScanThread", 4096, sg_scan_thread, NULL);
    furi_thread_start(sg_thread);
}

void subghz_scanner_stop(void) {
    if(!sg_active) return;
    sg_active = false;

    if(sg_thread) {
        furi_thread_join(sg_thread);
        furi_thread_free(sg_thread);
        sg_thread = NULL;
    }
    sg_vd = NULL;
}

bool subghz_scanner_is_active(void) {
    return sg_active;
}

bool subghz_scanner_has_signal(void) {
    return sg_had_signal;
}

void subghz_scanner_get_hash(char* out_hash, size_t size) {
    strncpy(out_hash, sg_last_hash, size);
    if(size > 0) out_hash[size - 1] = '\0';
}
