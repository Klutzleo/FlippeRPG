#include "../shrine/shrine_tags.h"
#include "../codex/codex.h"
#include "../tests/test_codex.c"

int main() {
    imprint_shrine_tag(SHRINE_FLAME_REACH, "TAG-FLAME");
    scan_shrine_tag(&test_codex, "TAG-FLAME");
    return 0;
}