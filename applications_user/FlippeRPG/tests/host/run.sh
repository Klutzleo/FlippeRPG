#!/usr/bin/env bash
# Builds and runs the host-side game-logic tests on this machine (no Flipper or fbt needed).
# Covers scoring, the daily band gate, stamina/absence, shrines and convergence.
set -euo pipefail

here="$(cd "$(dirname "$0")" && pwd)"
app="$here/../.."
out="$(mktemp -d)"
trap 'rm -rf "$out"' EXIT

cc -Wall -I "$here/stubs" -I "$app" \
    "$app/codex/codex.c" "$app/signal/signal_engine.c" "$app/shrine/shrine.c" \
    "$here/support.c" "$here/test_scoring.c" -o "$out/test_scoring"
"$out/test_scoring"
