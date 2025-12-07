#ifdef UNIT_TEST
#include "../codex/codex.h"
#include "../signal/signal_engine.h"

int main(void) {
    Codex test_codex;
    codex_init(&test_codex); // or whatever init function you have
    enter_manual_signal(&test_codex, "MANUAL-1234");
    return 0;
}
#endif