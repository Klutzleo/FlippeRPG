#include "ir_scanner.h"
#include <lib/infrared/encoder_decoder/infrared.h>
#include <furi_hal_infrared.h>
#include <furi.h>
#include <string.h>
#include <stdio.h>

// 8-second listen window — gives time for AC remotes and slow presses
#define IR_SCAN_TIMEOUT_MS  8000
// Poll interval while waiting for a signal
#define IR_POLL_INTERVAL_MS 50
// HAL silence timeout: 150ms gap = end of IR burst
#define IR_HAL_TIMEOUT_US   150000
// Minimum raw pulses to accept as a valid unknown-protocol signal
#define IR_RAW_MIN_PULSES   20

static volatile bool ir_active        = false;
static volatile bool ir_msg_ready     = false;
static volatile bool ir_raw_signal    = false; // Set when raw pulses captured but no known protocol
static volatile uint32_t ir_raw_count = 0;
static uint32_t ir_raw_checksum       = 0;
static InfraredDecoderHandler* ir_dec = NULL;
static InfraredMessage ir_result      = {0};
static ViewDispatcher* ir_vd          = NULL;
static char ir_last_hash[32]          = {0};
static bool ir_had_signal             = false;
static FuriThread* ir_thread          = NULL;

// ---- ISR callbacks (run in interrupt context — no RTOS calls) ----

static void ir_capture_isr(void* ctx, bool level, uint32_t duration) {
    (void)ctx;
    if(!ir_active || !ir_dec) return;

    // Try to decode a known protocol first
    const InfraredMessage* msg = infrared_decode(ir_dec, level, duration);
    if(msg && !msg->repeat && !ir_msg_ready) {
        ir_result    = *msg;
        ir_msg_ready = true;
        return;
    }

    // Accumulate raw pulses as fallback for unknown protocols (e.g. AC remotes)
    if(!ir_msg_ready && !ir_raw_signal) {
        ir_raw_count++;
        ir_raw_checksum ^= (uint32_t)((duration >> 4) & 0xFFFF);
        if(ir_raw_count >= IR_RAW_MIN_PULSES) {
            ir_raw_signal = true;
        }
    }
}

static void ir_timeout_isr(void* ctx) {
    (void)ctx;
    if(!ir_active || !ir_dec || ir_msg_ready) return;

    // Some protocols (e.g. SIRC) only finalise after silence — check here
    const InfraredMessage* msg = infrared_check_decoder_ready(ir_dec);
    if(msg && !msg->repeat) {
        ir_result    = *msg;
        ir_msg_ready = true;
        return;
    }

    // Accept raw signal if we accumulated enough pulses
    if(ir_raw_signal) {
        ir_msg_ready = true;
    }
}

// ---- Scan thread ----

static int32_t ir_scan_thread(void* context) {
    (void)context;

    ir_dec       = infrared_alloc_decoder();
    ir_msg_ready = false;

    furi_hal_infrared_async_rx_set_capture_isr_callback(ir_capture_isr, NULL);
    furi_hal_infrared_async_rx_set_timeout_isr_callback(ir_timeout_isr, NULL);
    furi_hal_infrared_async_rx_set_timeout(IR_HAL_TIMEOUT_US);
    furi_hal_infrared_async_rx_start();

    uint32_t start = furi_get_tick();
    while(ir_active && !ir_msg_ready) {
        furi_delay_ms(IR_POLL_INTERVAL_MS);
        if(furi_get_tick() - start > IR_SCAN_TIMEOUT_MS) break;
    }

    // Stop hardware before touching decoder
    furi_hal_infrared_async_rx_stop();
    furi_hal_infrared_async_rx_set_capture_isr_callback(NULL, NULL);
    furi_hal_infrared_async_rx_set_timeout_isr_callback(NULL, NULL);

    infrared_free_decoder(ir_dec);
    ir_dec = NULL;

    if(ir_active) {
        if(ir_msg_ready) {
            if(!ir_raw_signal) {
                // Known protocol — deterministic hash from protocol name, address, command
                const char* proto = infrared_get_protocol_name(ir_result.protocol);
                snprintf(
                    ir_last_hash,
                    sizeof(ir_last_hash),
                    "%.6s:%04lX:%04lX",
                    proto ? proto : "IR",
                    (unsigned long)ir_result.address,
                    (unsigned long)ir_result.command);
            } else {
                // Unknown protocol — hash from pulse count + timing checksum
                snprintf(
                    ir_last_hash,
                    sizeof(ir_last_hash),
                    "RAW:%04lX:%04lX",
                    (unsigned long)(ir_raw_count & 0xFFFF),
                    (unsigned long)(ir_raw_checksum & 0xFFFF));
            }
            ir_had_signal = true;
        } else {
            ir_had_signal = false;
        }
        view_dispatcher_send_custom_event(ir_vd, IR_SCAN_DONE_EVENT);
    }

    return 0;
}

// ---- Public API ----

void ir_scanner_start(ViewDispatcher* dispatcher) {
    if(ir_active) return;

    ir_vd           = dispatcher;
    ir_had_signal   = false;
    ir_raw_signal   = false;
    ir_raw_count    = 0;
    ir_raw_checksum = 0;
    ir_last_hash[0] = '\0';
    ir_active       = true;

    ir_thread = furi_thread_alloc_ex("IRScanThread", 1024, ir_scan_thread, NULL);
    furi_thread_start(ir_thread);
}

void ir_scanner_stop(void) {
    if(!ir_active) return;
    ir_active = false;

    if(ir_thread) {
        furi_thread_join(ir_thread);
        furi_thread_free(ir_thread);
        ir_thread = NULL;
    }
    ir_vd = NULL;
}

bool ir_scanner_is_active(void) {
    return ir_active;
}

bool ir_scanner_has_signal(void) {
    return ir_had_signal;
}

void ir_scanner_get_hash(char* out_hash, size_t size) {
    strncpy(out_hash, ir_last_hash, size);
    if(size > 0) out_hash[size - 1] = '\0';
}
