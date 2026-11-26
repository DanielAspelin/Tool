#!/usr/bin/env bash
set -euo pipefail
cd -- "$(dirname -- "$0")"
: "${WASI_SDK_PATH:?Set WASI_SDK_PATH to the installed WASI SDK directory}"
"${CXX:-c++}" -std=c++17 -O2 -pthread tool.cpp -o tool
"$WASI_SDK_PATH/bin/clang++" -std=c++17 -O2 tool.cpp -o tool.wasm
printf 'help\necho test\nexit\n' | ./tool
