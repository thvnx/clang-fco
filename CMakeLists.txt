set(LLVM_LINK_COMPONENTS
  Support
  )

add_clang_executable(clang-fco
  ClangFCO.cpp
  )

target_link_libraries(clang-fco
  clangAST
  clangASTMatchers
  clangBasic
  clangFrontend
  clangTooling
  )

install(TARGETS clang-fco RUNTIME DESTINATION bin)