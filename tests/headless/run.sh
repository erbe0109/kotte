#!/bin/sh
set -eu
cd "$(dirname "$0")/../.."
mkdir -p build/headless
CXX="${CXX:-g++}"
FLAGS="-std=c++23 -Wall -Wextra -Wpedantic -Werror"
INC="-Ikotte/include -isystem vendor/raylib/include -isystem vendor/cpp_prngs/include"
$CXX $FLAGS -DKOTTE_GAME_TEST $INC \
    kotte/src/application.cpp kotte/src/camera.cpp \
    kotte/src/entity_store.cpp kotte/src/renderer.cpp \
    kotte/src/spatial_grid.cpp kotte/src/tile_map.cpp \
    kotte/src/window.cpp tests/headless/raylib_stubs.cpp \
    tests/headless/gameplay_tests.cpp -o build/headless/gameplay_tests
./build/headless/gameplay_tests
$CXX $FLAGS -Wno-missing-field-initializers $INC \
    kotte/src/entity_store.cpp kotte.tests/src/entity_store_tests.cpp \
    -o build/headless/entity_store_tests
./build/headless/entity_store_tests
