#ifdef UNIT_TEST
#include "../codex/codex.h"
#include "../signal/signal_engine.h"
#include <stdio.h>
#include <string.h>

// ---------- Minimal assertion ----------

static int test_failures = 0;

#define ASSERT_EQ(label, got, expected)                                          \
    do {                                                                         \
        if((got) != (expected)) {                                                 \
            printf("FAIL  %s: expected %d, got %d\n", (label), (expected), (got)); \
            test_failures++;                                                      \
        } else {                                                                  \
            printf("PASS  %s\n", (label));                                        \
        }                                                                         \
    } while(0)

// ---------- Helpers ----------

// Backdate every entry in signal_history by offset_ms milliseconds.
// Used to simulate "yesterday" without a real clock.
static void backdate_all(Codex* codex, uint32_t offset_ms) {
    for(int i = 0; i < MAX_SIGNALS; i++) {
        if(codex->signal_history[i].hash[0] == '\0') continue;
        codex->signal_history[i].timestamp =
            (time_t)((uint32_t)codex->signal_history[i].timestamp - offset_ms);
    }
}

// ---------- Test 1: 3 / 1 / 0 gain sequence ----------
// Same signal type, three scans in a row within the same day.
// Expected gains: first=3, second=1, third=0.
static void test_gain_sequence(void) {
    Codex c;
    init_codex(&c, "TestPlayer");

    int g0 = scan_band(&c, SIGNAL_RF);
    int g1 = scan_band(&c, SIGNAL_RF);
    int g2 = scan_band(&c, SIGNAL_RF);

    ASSERT_EQ("3/1/0 first  scan gain", g0, 3);
    ASSERT_EQ("3/1/0 second scan gain", g1, 1);
    ASSERT_EQ("3/1/0 third  scan gain", g2, 0);
}

// ---------- Test 2: daily reset ----------
// After three same-type scans, backdate them by 25 hours.
// calculate_signal_gain should return 3 again — new day, clean slate.
static void test_daily_reset(void) {
    Codex c;
    init_codex(&c, "TestPlayer");

    // Exhaust today's gain for SIGNAL_IR
    scan_band(&c, SIGNAL_IR);
    scan_band(&c, SIGNAL_IR);
    scan_band(&c, SIGNAL_IR);

    // Confirm we're at 0
    int before = calculate_signal_gain(&c, SIGNAL_IR);
    ASSERT_EQ("daily reset: gate hits 0 before backdate", before, 0);

    // Backdate all entries by 25 hours (in ms)
    uint32_t ms_25h = 25u * 60u * 60u * 1000u;
    backdate_all(&c, ms_25h);

    // Now the day has rolled — should be eligible for +3 again
    int after = calculate_signal_gain(&c, SIGNAL_IR);
    ASSERT_EQ("daily reset: gain returns to 3 after 25h", after, 3);
}

// ---------- Test 3: type independence ----------
// Scanning SIGNAL_RF three times should not affect SIGNAL_NFC's counter.
static void test_type_independence(void) {
    Codex c;
    init_codex(&c, "TestPlayer");

    // Exhaust RF
    scan_band(&c, SIGNAL_RF);
    scan_band(&c, SIGNAL_RF);
    scan_band(&c, SIGNAL_RF);

    // NFC should still be fresh
    int nfc_gain = calculate_signal_gain(&c, SIGNAL_NFC);
    ASSERT_EQ("type independence: NFC unaffected by RF scans", nfc_gain, 3);
}

// ---------- Test 4: on_nfc_scan routes through progression ----------
// on_nfc_scan with a unique tag_id should log a SIGNAL_NFC entry.
// Call it three times with the same tag and verify gain drops 3 → 1 → 0.
static void test_on_nfc_scan_gain(void) {
    Codex c;
    init_codex(&c, "TestPlayer");

    // Capture gain before each scan by peeking at calculate_signal_gain
    int g0 = calculate_signal_gain(&c, SIGNAL_NFC);
    on_nfc_scan(&c, "04:AB:CD:EF");
    int g1 = calculate_signal_gain(&c, SIGNAL_NFC);
    on_nfc_scan(&c, "04:AB:CD:EF");
    int g2 = calculate_signal_gain(&c, SIGNAL_NFC);
    on_nfc_scan(&c, "04:AB:CD:EF");
    int g3 = calculate_signal_gain(&c, SIGNAL_NFC);

    ASSERT_EQ("nfc_scan gain pre-1st scan", g0, 3);
    ASSERT_EQ("nfc_scan gain pre-2nd scan", g1, 1);
    ASSERT_EQ("nfc_scan gain pre-3rd scan", g2, 0);
    ASSERT_EQ("nfc_scan gain  post-3rd scan", g3, 0);
}

// ---------- Entry point ----------

int main(void) {
    printf("=== signal_engine gain gate tests ===\n");

    test_gain_sequence();
    test_daily_reset();
    test_type_independence();
    test_on_nfc_scan_gain();

    printf("=====================================\n");
    if(test_failures == 0) {
        printf("ALL TESTS PASSED\n");
    } else {
        printf("%d TEST(S) FAILED\n", test_failures);
    }
    return test_failures;
}

#endif // UNIT_TEST
