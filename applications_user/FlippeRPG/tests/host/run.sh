#!/usr/bin/env bash
# Builds and runs the host-side game-logic tests on this machine (no Flipper or fbt needed).
#  - test_scoring: scoring, the daily band gate, stamina/absence, shrines, convergence
#  - test_transmissions: the transmission text and its word-wrap
set -euo pipefail

here="$(cd "$(dirname "$0")" && pwd)"
app="$here/../.."
out="$(mktemp -d)"
trap 'rm -rf "$out"' EXIT

cc -Wall -I "$here/stubs" -I "$app" \
    "$app/codex/codex.c" "$app/signal/signal_engine.c" "$app/shrine/shrine.c" \
    "$here/support.c" "$here/test_scoring.c" -o "$out/test_scoring"

cc -Wall -I "$here/stubs" -I "$app" \
    "$app/narrative/transmissions.c" "$app/ui/text_wrap.c" \
    "$here/test_transmissions.c" -o "$out/test_transmissions"

status=0
"$out/test_scoring" || status=1
"$out/test_transmissions" || status=1
exit $status
