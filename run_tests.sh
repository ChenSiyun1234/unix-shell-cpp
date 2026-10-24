#!/usr/bin/env bash
# Runs the OSTEP processes-shell tests against ./wish.
# Extra arguments go to the OSTEP tester, for example -c to keep going after a failure.
set -euo pipefail

OSTEP_COMMIT=76cff3f89f4bf337af6e02e53a831b7eeb1396df
here="$(cd "$(dirname "$0")" && pwd)"
ostep="$here/.ostep"

if [ ! -d "$ostep/processes-shell/tests" ]; then
    rm -rf "$ostep"
    mkdir -p "$ostep"
    curl -sSfL "https://codeload.github.com/remzi-arpacidusseau/ostep-projects/tar.gz/$OSTEP_COMMIT" |
        tar -xz -C "$ostep" --strip-components=1
fi

cp "$here/wish" "$ostep/processes-shell/wish"
cd "$ostep/processes-shell"
./test-wish.sh "$@"
