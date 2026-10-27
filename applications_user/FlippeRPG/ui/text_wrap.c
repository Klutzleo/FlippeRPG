#include "text_wrap.h"
#include <stdio.h>
#include <string.h>

size_t text_wrap(
    const char* text,
    uint16_t max_width,
    TextWidthFn width,
    void* ctx,
    char rows[][TEXT_WRAP_ROW_LEN],
    size_t first_row,
    size_t max_rows) {
    size_t n = first_row;
    char line[TEXT_WRAP_ROW_LEN] = {0};
    size_t len = 0;

    const char* p = text;
    while(*p && n < max_rows) {
        while(*p == ' ') p++;
        if(!*p) break;
        const char* word = p;
        while(*p && *p != ' ') p++;
        size_t word_len = (size_t)(p - word);
        if(word_len >= TEXT_WRAP_ROW_LEN) word_len = TEXT_WRAP_ROW_LEN - 1;

        // Would the word still fit on the current row?
        char trial[TEXT_WRAP_ROW_LEN];
        int needed = snprintf(trial, sizeof(trial), "%s%s%.*s", line, len ? " " : "", (int)word_len, word);
        bool fits = needed < (int)sizeof(trial) && (len == 0 || width(ctx, trial) <= max_width);

        if(fits) {
            memcpy(line, trial, (size_t)needed + 1);
            len = (size_t)needed;
        } else {
            snprintf(rows[n++], TEXT_WRAP_ROW_LEN, "%s", line);
            snprintf(line, sizeof(line), "%.*s", (int)word_len, word);
            len = strlen(line);
        }
    }

    if(len > 0 && n < max_rows) snprintf(rows[n++], TEXT_WRAP_ROW_LEN, "%s", line);
    return n;
}
