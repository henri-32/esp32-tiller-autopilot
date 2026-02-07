#!/usr/bin/env bash
set -euo pipefail

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_TARGET="$PROJECT_ROOT/compile_commands.json"
SKETCH_TARGET="$PROJECT_ROOT/autopilot_maincode/compile_commands.json"

# mögliche Cache-Pfade
CANDIDATES=(
  "$HOME/.cache/arduino/sketches"
  "$HOME/.arduino15"
  "$HOME/snap/arduino-cli/current/.cache/arduino/sketches"
  "$HOME/snap/arduino-cli/62/.cache/arduino/sketches"
)

ARDUINO_CACHE=""

for dir in "${CANDIDATES[@]}"; do
  if [[ -d "$dir" ]]; then
    ARDUINO_CACHE="$dir"
    break
  fi
done

if [[ -z "$ARDUINO_CACHE" ]]; then
  echo "Arduino cache nicht gefunden."
  echo "Gesucht in:"
  printf '  %s\n' "${CANDIDATES[@]}"
  exit 1
fi

LATEST_BUILD_DIR=$(
  find "$ARDUINO_CACHE" -maxdepth 1 -type d \
    -printf '%T@ %p\n' \
  | sort -nr \
  | awk 'NR==1 {print $2}'
)

SRC="$LATEST_BUILD_DIR/compile_commands.json"

if [[ ! -f "$SRC" ]]; then
  echo "compile_commands.json nicht gefunden."
  echo "Bitte zuerst: arduino-cli compile"
  exit 1
fi

cp "$SRC" "$ROOT_TARGET"
cp "$SRC" "$SKETCH_TARGET"
echo "compile_commands.json aktualisiert:"
echo "  Quelle: $SRC"
echo "  Ziel 1: $ROOT_TARGET"
echo "  Ziel 2: $SKETCH_TARGET"
