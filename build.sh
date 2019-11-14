#!/usr/bin/env bash
set -eu

git clone --depth 1 https://github.com/llvm/llvm-project.git
echo "add_subdirectory(clang-fco)" >> llvm-project/clang-tools-extra/CMakeLists.txt
mkdir llvm-project/clang-tools-extra/clang-fco
cp CMakeLists.txt ClangFCO.cpp llvm-project/clang-tools-extra/clang-fco
mkdir llvm-project-build
cd llvm-project-build
cmake -G "Ninja" -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_INSTALL_PREFIX=../llvm-project-root \
      -DLLVM_TARGETS_TO_BUILD=X86 -DLLVM_BUILD_LLVM_DYLIB=True \
      -DLLVM_ENABLE_PROJECTS="clang;clang-tools-extra" ../llvm-project/llvm
ninja
