// Host-side tests for the transmission text: the data is complete and well-formed, the word-wrap
// behaves, and every transmission fits the screen once wrapped.
#include "narrative/transmissions.h"
#include "ui/text_wrap.h"
#include <stdio.h>
#include <string.h>

static int failures = 0;

#define CHECK(cond, msg)                                           \
    do {                                                           \
        if(cond) {                                                 \
            printf("ok    %s\n", msg);                             \
        } else {                                                   \
            printf("FAIL  %s  (%s:%d)\n", msg, __FILE__, __LINE__); \
            failures++;                                            \
        }                                                          \
    } while(0)

// FontSecondary is proportional; 6 px per character is deliberately pessimistic (real text
// averages a little under 5.5), so a pass here leaves room to spare on the device.
static uint16_t fake_width(void* ctx, const char* text) {
    (void)ctx;
    return (uint16_t)(strlen(text) * 6);
}

#define SCREEN_WIDTH     124
#define ROWS_PER_PAGE    5
#define MAX_ROWS         16
#define MAX_TEXT_PAGES   4

static size_t rows_for(const Transmission* tx, char rows[][TEXT_WRAP_ROW_LEN]) {
    size_t n = 0;
    for(int i = 0; i < TX_TEXT_LINES && tx->text[i] && n < MAX_ROWS; i++) {
        n = text_wrap(tx->text[i], SCREEN_WIDTH, fake_width, NULL, rows, n, MAX_ROWS);
    }
    return n;
}

static void test_wrap(void) {
    printf("-- word wrap\n");
    char rows[8][TEXT_WRAP_ROW_LEN];

    size_t n = text_wrap("Nothing here needs wrapping", 400, fake_width, NULL, rows, 0, 8);
    CHECK(n == 1 && strcmp(rows[0], "Nothing here needs wrapping") == 0, "a short line stays one row");

    n = text_wrap("It has been waiting for you specifically.", 120, fake_width, NULL, rows, 0, 8);
    CHECK(n == 3, "a long line wraps on word boundaries");
    bool fits = true;
    for(size_t i = 0; i < n; i++) fits = fits && fake_width(NULL, rows[i]) <= 120;
    CHECK(fits, "every wrapped row fits the width");
    char joined[TEXT_WRAP_ROW_LEN * 8] = {0};
    for(size_t i = 0; i < n; i++) {
        if(i) strcat(joined, " ");
        strcat(joined, rows[i]);
    }
    CHECK(strcmp(joined, "It has been waiting for you specifically.") == 0, "words are never split, lost or reordered");

    n = text_wrap("Supercalifragilisticexpialidocious", 60, fake_width, NULL, rows, 0, 8);
    CHECK(n == 1, "a word wider than the screen still gets its own row");

    n = text_wrap("One", 100, fake_width, NULL, rows, 0, 8);
    n = text_wrap("Two", 100, fake_width, NULL, rows, n, 8);
    CHECK(n == 2 && strcmp(rows[1], "Two") == 0, "paragraphs append after earlier rows");

    n = text_wrap("a b c d e f g h", 6, fake_width, NULL, rows, 0, 3);
    CHECK(n == 3, "wrapping stops at the row limit");
    n = text_wrap("   spaced   out   ", 400, fake_width, NULL, rows, 0, 8);
    CHECK(n == 1 && strcmp(rows[0], "spaced out") == 0, "extra spaces collapse");
    n = text_wrap("", 100, fake_width, NULL, rows, 0, 8);
    CHECK(n == 0, "empty text makes no rows");
}

static void check_transmission(const Transmission* tx, const char* what, int* bad_shape, int* too_long) {
    for(int i = 0; i < 3; i++) {
        if(!tx->readout[i] || tx->readout[i][0] != '>') (*bad_shape)++;
    }
    if(!tx->text[0]) (*bad_shape)++;
    for(int i = 1; i < TX_TEXT_LINES; i++) {
        if(tx->text[i] && !tx->text[i - 1]) (*bad_shape)++; // no gaps in the list
    }
    char rows[MAX_ROWS][TEXT_WRAP_ROW_LEN];
    size_t n = rows_for(tx, rows);
    size_t pages = (n + ROWS_PER_PAGE - 1) / ROWS_PER_PAGE;
    if(n >= MAX_ROWS || pages > MAX_TEXT_PAGES) {
        printf("      too long: %s (%zu rows)\n", what, n);
        (*too_long)++;
    }
}

static void test_data(void) {
    printf("-- transmission data\n");
    int bad_shape = 0, too_long = 0, count = 0;
    for(int b = 0; b < NUM_BANDS; b++) {
        for(int i = 0; i < TX_ARC_COUNT; i++)     { check_transmission(&tx_arc[b][i], "arc", &bad_shape, &too_long); count++; }
        for(int i = 0; i < TX_RESCAN_COUNT; i++)  { check_transmission(&tx_rescan[b][i], "rescan", &bad_shape, &too_long); count++; }
        for(int i = 0; i < TX_AMBIENT_COUNT; i++) { check_transmission(&tx_ambient[b][i], "ambient", &bad_shape, &too_long); count++; }
    }
    for(int i = 0; i < TX_AMBIENT_COUNT; i++) { check_transmission(&tx_ancient_pull[i], "ancient pull", &bad_shape, &too_long); count++; }

    CHECK(count == 4 * (5 + 3 + 10) + 10, "all 82 transmissions are present");
    CHECK(bad_shape == 0, "every transmission has a 3-line readout and at least one line of text");
    CHECK(too_long == 0, "every transmission fits within a few screens, even at a pessimistic font width");

    bool thresholds_ok = true;
    for(int b = 0; b < NUM_BANDS; b++) {
        for(int i = 0; i < TX_ARC_COUNT; i++) {
            if(tx_arc[b][i].threshold != (i == TX_ARC_COUNT - 1)) thresholds_ok = false;
        }
        for(int i = 0; i < TX_RESCAN_COUNT; i++) if(tx_rescan[b][i].threshold) thresholds_ok = false;
        for(int i = 0; i < TX_AMBIENT_COUNT; i++) if(tx_ambient[b][i].threshold) thresholds_ok = false;
    }
    CHECK(thresholds_ok, "only each arc's fifth transmission is a Threshold");

    bool bands_ok = true;
    const char* names[NUM_BANDS] = {"> IR ", "> SUB-GHz ", "> NFC ", "> BLUETOOTH"};
    for(int b = 0; b < NUM_BANDS; b++) {
        for(int i = 0; i < TX_ARC_COUNT; i++) {
            if(strncmp(tx_arc[b][i].readout[0], names[b], strlen(names[b])) != 0) bands_ok = false;
        }
    }
    CHECK(bands_ok, "each band's arc is filed under the right band");
    CHECK(strncmp(tx_ancient_pull[0].readout[0], "> RF ", 5) == 0, "the easter egg entries are the RF ones");
    CHECK(strcmp(tx_arc[0][0].text[0], "Something in this room has eyes.") == 0, "wording matches the lore (IR, Ambient)");
}

int main(void) {
    printf("=== transmission text tests ===\n");
    test_wrap();
    test_data();
    printf("%s (%d failure%s)\n", failures ? "FAILED" : "ALL PASSED", failures, failures == 1 ? "" : "s");
    return failures ? 1 : 0;
}
