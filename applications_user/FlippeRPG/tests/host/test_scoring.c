// Host-side tests for the game rules: what counts, what it earns, how bands open.
// Run with tests/host/run.sh. A fake clock (fake_now) stands in for the Flipper's RTC.
#include "codex/codex.h"
#include "shrine/shrine.h"
#include "signal/signal_engine.h"
#include <stdio.h>
#include <string.h>

extern uint32_t fake_now;

static int failures = 0;

#define CHECK(cond, msg)                                  \
    do {                                                  \
        if(cond) {                                        \
            printf("ok    %s\n", msg);                    \
        } else {                                          \
            printf("FAIL  %s  (%s:%d)\n", msg, __FILE__, __LINE__); \
            failures++;                                   \
        }                                                 \
    } while(0)

#define DAY(n) ((uint32_t)(n) * SECONDS_PER_DAY)

static Codex c;

static void new_player(uint32_t day) {
    fake_now = DAY(day) + 3600; // 01:00 on that day
    init_codex(&c, "Test");
}

static ScanResult scan(SignalType type, const char* source) {
    switch(type) {
    case SIGNAL_IR:        return on_ir_scan(&c, source);
    case SIGNAL_SUBGHZ:    return on_subghz_scan(&c, source);
    case SIGNAL_NFC:       return on_nfc_scan(&c, source);
    case SIGNAL_RFID:      return on_rfid_scan(&c, source);
    case SIGNAL_BLUETOOTH: return on_bt_scan(&c, source);
    default: {
        ScanResult none = {false, 0};
        return none;
    }
    }
}

// n distinct sources of one type
static void scan_n(SignalType type, int n, const char* prefix) {
    for(int i = 0; i < n; i++) {
        char source[32];
        snprintf(source, sizeof(source), "%s%d", prefix, i);
        scan(type, source);
    }
}

// ---------------------------------------------------------------------------

static void test_gain_is_per_type_per_day(void) {
    printf("-- gain: +3, +1, +0 per type per day\n");
    new_player(100);
    ScanResult a = scan(SIGNAL_IR, "NEC:1");
    ScanResult b = scan(SIGNAL_IR, "NEC:2");
    ScanResult d = scan(SIGNAL_IR, "NEC:3");
    CHECK(a.counted && a.gain == 3, "first counted scan earns +3");
    CHECK(b.counted && b.gain == 1, "second distinct source earns +1");
    CHECK(d.counted && d.gain == 0, "third distinct source counts but earns +0");

    ScanResult nfc = scan(SIGNAL_NFC, "NFC:1");
    CHECK(nfc.gain == 3, "another type has its own daily gain");

    fake_now = DAY(101) + 3600;
    ScanResult next = scan(SIGNAL_IR, "NEC:4");
    CHECK(next.counted && next.gain == 3, "gain resets at midnight");
}

static void test_a_source_counts_once_a_day(void) {
    printf("-- sources count once per day\n");
    new_player(100);
    ScanResult first = scan(SIGNAL_NFC, "NFC:AAAA");
    ScanResult again = scan(SIGNAL_NFC, "NFC:AAAA");
    CHECK(first.counted, "first sighting counts");
    CHECK(!again.counted && again.gain == 0, "same source again the same day does not count");
    CHECK(c.band_scans[SIGNAL_NFC] == 1, "a repeat does not advance the band");
    CHECK(c.signal_score == 3, "a repeat earns nothing");

    fake_now = DAY(101) + 3600;
    CHECK(scan(SIGNAL_NFC, "NFC:AAAA").counted, "the same source counts again tomorrow");
}

static void test_band_completes_and_stops_advancing(void) {
    printf("-- band completion\n");
    new_player(100);
    scan_n(SIGNAL_IR, 4, "S");
    CHECK(c.band_scans[SIGNAL_IR] == 4, "4 counted scans so far");
    scan(SIGNAL_IR, "S4");
    CHECK(c.band_scans[SIGNAL_IR] == SCANS_PER_BAND, "5th counted scan completes the band");
    CHECK(c.band_complete_day[0] == 100, "the completion day is recorded");
    scan(SIGNAL_IR, "S5");
    CHECK(c.band_scans[SIGNAL_IR] == SCANS_PER_BAND, "scans past 5 do not advance the band");
}

static void test_one_band_opens_per_day(void) {
    printf("-- one band opens per day\n");
    new_player(100);
    CHECK(c.bands_unlocked == 1, "starts with one band");
    scan_n(SIGNAL_IR, 5, "S");
    check_band_gate(&c);
    CHECK(c.bands_unlocked == 1, "the next band stays hidden the day the first completes");
    fake_now = DAY(101) + 3600;
    codex_refresh_bands(&c);
    CHECK(c.bands_unlocked == 2, "it opens the next day");

    // A save from before completion days were tracked must not hold anyone back
    c.band_complete_day[0] = 0;
    fake_now = DAY(101) + 7200;
    codex_refresh_bands(&c);
    CHECK(c.bands_unlocked == 2, "an unknown completion day does not block the next band");
}

static void test_bluetooth_has_no_source_id(void) {
    printf("-- bluetooth: capped and spaced\n");
    new_player(100);
    ScanResult a = scan(SIGNAL_BLUETOOTH, "BT:A");
    ScanResult b = scan(SIGNAL_BLUETOOTH, "BT:B");
    CHECK(a.counted && a.gain == 3, "first bluetooth scan counts");
    CHECK(!b.counted, "a second scan seconds later does not count");
    fake_now += SOURCELESS_MIN_GAP_SECONDS;
    CHECK(scan(SIGNAL_BLUETOOTH, "BT:C").counted, "a scan after the minimum gap counts");

    for(int i = 0; i < 10; i++) {
        fake_now += SOURCELESS_MIN_GAP_SECONDS;
        char s[16];
        snprintf(s, sizeof(s), "BT:%d", i);
        scan(SIGNAL_BLUETOOTH, s);
    }
    CHECK(c.band_scans[SIGNAL_BLUETOOTH] == SCANS_PER_BAND, "bluetooth fills the band");
    new_player(100);
    int counted = 0;
    for(int i = 0; i < 12; i++) {
        fake_now += SOURCELESS_MIN_GAP_SECONDS;
        char s[16];
        snprintf(s, sizeof(s), "BT:%d", i);
        if(scan(SIGNAL_BLUETOOTH, s).counted) counted++;
    }
    CHECK(counted == SCANS_PER_BAND, "no more than 5 bluetooth scans count in a day");
}

static void test_fast_path_reaches_present_at_zero_day(void) {
    printf("-- the fastest path to Zero Day\n");
    new_player(200);
    bool zero_day = false;
    for(int band = 0; band < NUM_BANDS; band++) {
        codex_refresh_bands(&c);
        CHECK(c.bands_unlocked == band + 1, "the band is open on its day");
        for(int i = 0; i < SCANS_PER_BAND; i++) {
            char source[32];
            snprintf(source, sizeof(source), "B%d_%d", band, i);
            if(band_order[band] == SIGNAL_BLUETOOTH) fake_now += SOURCELESS_MIN_GAP_SECONDS;
            if(scan(band_order[band], source).counted) apply_signal_gain(&c, TRANSMISSION_ACK_BONUS);
        }
        apply_signal_gain(&c, BAND_COMPLETE_BONUS);
        zero_day = check_band_gate(&c);
        fake_now += SECONDS_PER_DAY;
    }
    CHECK(zero_day && c.zero_day_confirmed && c.substrate_unlocked, "Zero Day fires after the 4th band");
    apply_signal_gain(&c, ZERO_DAY_ARRIVAL_BONUS);
    CHECK(c.signal_score == 351, "the fastest path totals exactly 351");
    CHECK(c.signal_strength_level == SIGNAL_STRENGTH_PRESENT, "and arrives at PRESENT");
    CHECK(c.zero_day_date != 0, "the Zero Day date is stamped");
    CHECK(!check_band_gate(&c), "Zero Day fires only once");
}

static void test_stamina_and_absence(void) {
    printf("-- stamina and absence\n");
    new_player(100);
    codex_drain_stamina(&c, 20);
    CHECK(c.stamina == 80, "a drain lowers stamina");
    fake_now += 10 * STAMINA_RECOVERY_SECONDS_PER_POINT;
    codex_apply_time_passage(&c);
    CHECK(c.stamina == 90, "stamina recovers one point per interval");
    fake_now += 100u * STAMINA_RECOVERY_SECONDS_PER_POINT;
    codex_apply_time_passage(&c);
    CHECK(c.stamina == STAMINA_MAX, "stamina never exceeds the max");
    codex_drain_stamina(&c, 500);
    CHECK(c.stamina == 0, "stamina never goes below zero");

    new_player(100);
    apply_signal_gain(&c, 100);
    fake_now = DAY(101) + 3600; // the very next day: not absent
    codex_apply_time_passage(&c);
    CHECK(c.signal_score == 100, "coming back the next day costs nothing");
    fake_now = DAY(111) + 3600; // 9 whole days with no launch
    codex_apply_time_passage(&c);
    CHECK(c.signal_score == 91, "each whole inactive day costs 1 signal");
    fake_now = DAY(1000) + 3600;
    codex_apply_time_passage(&c);
    CHECK(c.signal_score == 1, "absence decay is capped");
    apply_signal_gain(&c, -50);
    CHECK(c.signal_score == 0 && c.signal_strength_level == SIGNAL_STRENGTH_STATIC, "signal never goes below zero");
}

static void test_shrines(void) {
    printf("-- shrines\n");
    new_player(100);
    CHECK(shrine_is_active(SHRINE_POCKET_CARRIED), "The Unread is active");
    CHECK(!shrine_is_active(SHRINE_ECHO_TOUCHED), "The First Frequency is retired");
    CHECK(!shrine_is_active(SHRINE_UNKNOWN), "the unknown sentinel is not a shrine");

    trigger_shrine(&c, SHRINE_ECHO_TOUCHED, SIGNAL_SUBGHZ);
    CHECK(!c.shrine_progress[SHRINE_ECHO_TOUCHED].completed, "a retired shrine cannot be triggered");

    complete_shrine(&c, SHRINE_POCKET_CARRIED);
    CHECK(strcmp(c.aura_trait, "Legible") == 0, "The Unread grants the Legible aura");
    complete_shrine(&c, SHRINE_FLAME_REACH);
    CHECK(strcmp(c.aura_trait, "Clearseeing") == 0, "The Fixed Gaze grants Clearseeing");
}

static void test_convergence_ignores_retired_shrine(void) {
    printf("-- convergence\n");
    new_player(100);
    apply_signal_gain(&c, 900); // DEEP
    CHECK(c.signal_strength_level >= SIGNAL_STRENGTH_DEEP, "player is DEEP");
    CHECK(!ready_for_convergence(&c), "not ready with no shrines done");

    for(int s = 0; s < SHRINE_UNKNOWN; s++) {
        if(shrine_is_active(s)) c.shrine_progress[s].completed = true;
    }
    CHECK(!c.shrine_progress[SHRINE_ECHO_TOUCHED].completed, "the retired shrine is still incomplete");
    CHECK(ready_for_convergence(&c), "all the real shrines done is enough");

    c.shrine_progress[SHRINE_BIND_WHISPER].completed = false;
    CHECK(!ready_for_convergence(&c), "a missing real shrine still blocks it");
}

int main(void) {
    printf("=== FlippeRPG host tests ===\n");
    test_gain_is_per_type_per_day();
    test_a_source_counts_once_a_day();
    test_band_completes_and_stops_advancing();
    test_one_band_opens_per_day();
    test_bluetooth_has_no_source_id();
    test_fast_path_reaches_present_at_zero_day();
    test_stamina_and_absence();
    test_shrines();
    test_convergence_ignores_retired_shrine();
    printf("%s (%d failure%s)\n", failures ? "FAILED" : "ALL PASSED", failures, failures == 1 ? "" : "s");
    return failures ? 1 : 0;
}
