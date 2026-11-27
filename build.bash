#!/usr/bin/env bash
set -euo pipefail
cd -- "$(dirname -- "$0")"

output_dir=${OUTPUT_DIR:-.}
mkdir -p -- "$output_dir"

build_native() {
    "${CXX:-c++}" -std=c++17 -O0 -pthread tool.cpp -o "$output_dir/tool"
    test "$("$output_dir/tool" echo test)" = test
}

build_browser() {
    : "${WASI_SDK_PATH:?Set WASI_SDK_PATH to the installed WASI SDK directory}"
    "$WASI_SDK_PATH/bin/clang++" --target=wasm32-unknown-unknown -std=c++17 -Oz \
        -nostdlib -fno-exceptions -fno-rtti -DTOOL_BROWSER_WASM \
        -Wl,--no-entry \
        -Wl,--export=tool_route_abi -Wl,--export=tool_route_state -Wl,--export=tool_route_step \
        -Wl,--export=tool_javascript_size -Wl,--export=tool_javascript_byte \
        tool.cpp -o "$output_dir/tool-route.wasm"
}

case "${1:-all}" in
    native) build_native ;;
    browser) build_browser ;;
    all) build_native && build_browser ;;
    verify) build_native && build_browser && node tests/echo.mjs "$output_dir" ;;
    *) printf 'Usage: %s [all|native|browser|verify]\n' "$0" >&2; exit 2 ;;
esac
