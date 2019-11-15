#!/usr/bin/env bash
set -eu

cd standalone
cp ../ClangFCO.cpp .
mkdir clang-fco-build
cd clang-fco-build
cmake -G "Ninja" ..
ninja
