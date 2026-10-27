#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Longest wrapped row, in characters (including the terminator).
#define TEXT_WRAP_ROW_LEN 64

// Pixel width of a string. On the Flipper this wraps canvas_string_width().
typedef uint16_t (*TextWidthFn)(void* ctx, const char* text);

// Word-wraps one paragraph into rows no wider than max_width, appending to rows[first_row...].
// A single word wider than max_width gets a row to itself. Stops at max_rows.
// Returns the new row count.
size_t text_wrap(
    const char* text,
    uint16_t max_width,
    TextWidthFn width,
    void* ctx,
    char rows[][TEXT_WRAP_ROW_LEN],
    size_t first_row,
    size_t max_rows);
