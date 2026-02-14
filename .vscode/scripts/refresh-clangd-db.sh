#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
cd "$repo_root"

# Use a repo-local PlatformIO core by default. This avoids permission and
# lockfile issues after switching between Linux/Windows setups.
if [[ -z "${PLATFORMIO_CORE_DIR:-}" ]]; then
  export PLATFORMIO_CORE_DIR="$repo_root/.pio-core"
fi
mkdir -p "$PLATFORMIO_CORE_DIR"

require_cmd() {
  if ! command -v "$1" >/dev/null 2>&1; then
    echo "Missing required command: $1" >&2
    exit 1
  fi
}

require_cmd pio
require_cmd jq
require_cmd grep

build_compiledb() {
  local env="$1"
  local target_dir="$2"
  pio run -t compiledb -e "$env"
  mkdir -p "$target_dir"
  cp compile_commands.json "$target_dir/compile_commands.json"
}

capture_test_compile_commands() {
  local log_file
  log_file="$(mktemp)"
  pio test -e native_csc -vvv --without-testing >"$log_file" 2>&1

  local csc_cmd test_cmd
  csc_cmd="$(grep -m1 -E '^g\+\+ .*src/core/steering/csc/csc\.cpp$' "$log_file" || true)"
  test_cmd="$(grep -m1 -E '^g\+\+ .*test/test_csc/test_csc\.cpp$' "$log_file" || true)"

  if [[ -z "$csc_cmd" || -z "$test_cmd" ]]; then
    local base_cmd
    base_cmd="$(jq -r '.[] | select(.file == "src/core/steering/csc/csc.cpp") | .command' .pio/build/native_csc/compile_commands.json)"
    if [[ -z "$base_cmd" || "$base_cmd" == "null" ]]; then
      echo "Could not capture or synthesize native_csc test compile commands." >&2
      cat "$log_file" >&2
      rm -f "$log_file"
      exit 1
    fi

    # Fallback when PlatformIO suppresses verbose compile lines in test mode.
    local base_no_src
    base_no_src="${base_cmd% src/core/steering/csc/csc.cpp}"
    csc_cmd="$base_no_src -DPIO_UNIT_TESTING -DUNIT_TEST -DUNITY_INCLUDE_CONFIG_H -I.pio/libdeps/native_csc/Unity/src -I.pio/build/native_csc/unity_config -Itest/test_csc -Itest src/core/steering/csc/csc.cpp"
    test_cmd="${base_no_src/ -o .pio\/build\/native_csc\/src\/core\/steering\/csc\/csc.o / -o .pio\/build\/native_csc\/test\/test_csc\/test_csc.o }"
    test_cmd="$test_cmd -DPIO_UNIT_TESTING -DUNIT_TEST -DUNITY_INCLUDE_CONFIG_H -I.pio/libdeps/native_csc/Unity/src -I.pio/build/native_csc/unity_config -Itest/test_csc -Itest test/test_csc/test_csc.cpp"
  fi

  rm -f "$log_file"
  printf '%s\n%s\n' "$csc_cmd" "$test_cmd"
}

inject_native_csc_test_entries() {
  local db_path=".pio/build/native_csc/compile_commands.json"
  local csc_cmd="$1"
  local test_cmd="$2"

  jq --arg csc "$csc_cmd" --arg test "$test_cmd" '
    . as $db
    | [ .[]
        | select(
            .file != "src/core/steering/csc.cpp"
            and .file != "test/test_csc/test_csc.cpp"
            and .file != "test\\test_csc\\test_csc.cpp"
          )
      ]
    + [
        {
          command: $csc,
          directory: "'"$repo_root"'",
          file: "src/core/steering/csc/csc.cpp",
          output: ".pio/build/native_csc/src/core/steering/csc/csc.o"
        },
        {
          command: $test,
          directory: "'"$repo_root"'",
          file: "test/test_csc/test_csc.cpp",
          output: ".pio/build/native_csc/test/test_csc/test_csc.o"
        }
      ]
  ' "$db_path" > "${db_path}.tmp"

  mv "${db_path}.tmp" "$db_path"
}

build_compiledb "genericSTM32F411RE" ".pio/build/genericSTM32F411RE"
build_compiledb "native_csc" ".pio/build/native_csc"

mapfile -t cmds < <(capture_test_compile_commands)
inject_native_csc_test_entries "${cmds[0]}" "${cmds[1]}"

# Keep root compile_commands.json focused on firmware env.
pio run -t compiledb -e genericSTM32F411RE
