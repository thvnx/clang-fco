set(LLVM_LINK_COMPONENTS
  Support
  )

add_clang_executable(clang-fco
  ClangFCO.cpp
  )

target_link_libraries(clang-fco
  PRIVATE
  clangAST
  clangASTMatchers
  clangBasic
  clangFrontend
  clangTooling
  clangToolingRefactoring
  )
