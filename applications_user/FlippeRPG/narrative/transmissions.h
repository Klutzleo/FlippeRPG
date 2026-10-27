#pragma once
#include <stdbool.h>
#include <stddef.h>
#include "../core/constants.h"

// Band transmissions, one set per Tier 1 band in band_order[] order (IR, Sub-GHz, NFC, Bluetooth).
// Each is a readout (the machine lines) followed by the Substrate's words. The data is generated
// from the lore documents — see transmissions.c.

#define TX_ARC_COUNT     5   // Ambient, Noticed, Responsive, Urgent, Threshold
#define TX_RESCAN_COUNT  3   // Friendly, Encouraging, Pushing
#define TX_AMBIENT_COUNT 10
#define TX_TEXT_LINES    3

typedef struct {
    const char* readout[3];         // "> IR SIGNAL: ambient" ...
    const char* text[TX_TEXT_LINES]; // Substrate's words; NULL ends the list early
    bool        threshold;          // Arc's last transmission: closes with the tagline
} Transmission;

// Shown once each, in order, as a band's first five counted scans land.
extern const Transmission tx_arc[NUM_BANDS][TX_ARC_COUNT];
// Shown when a scan doesn't count. The tone grows more pointed the more you repeat in a day.
extern const Transmission tx_rescan[NUM_BANDS][TX_RESCAN_COUNT];
// Drawn at random once a band's arc is complete. They keep the world breathing.
extern const Transmission tx_ambient[NUM_BANDS][TX_AMBIENT_COUNT];
// The retired RF band's ambient entries, kept as the "Ancient Pull" easter egg.
extern const Transmission tx_ancient_pull[TX_AMBIENT_COUNT];
