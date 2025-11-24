#pragma once
#include <stdbool.h>

// Protocol enum (expand later if needed)
typedef enum {
    SubGhzProtocolCustom,
    // add more protocols here
} SubGhzProtocol;

// Transmission config
typedef struct {
    unsigned int frequency;   // Hz
    SubGhzProtocol protocol;  // protocol type
    const void* data;         // pointer to raw data
    unsigned int data_size;   // size of data
} SubGhzTxConfig;

// Transmission start function
bool subghz_tx_start(const SubGhzTxConfig* config);