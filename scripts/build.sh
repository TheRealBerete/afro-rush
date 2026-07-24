#!/bin/bash
set -e

export VITASDK=/usr/local/vitasdk
export PATH="$VITASDK/bin:$PATH"

PROJECT_DIR=/mnt/d/Bérété/projets/psvita

cd "$PROJECT_DIR"

cmake -Bbuild -S. -DCMAKE_TOOLCHAIN_FILE="$VITASDK/share/vita.toolchain.cmake" -DCMAKE_BUILD_TYPE=Release
cmake --build build
