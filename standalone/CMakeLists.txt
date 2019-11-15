cmake_minimum_required(VERSION 2.8.8)

project(clang-fco)
option(BUILD_CLANG_FCO "Build clang-fco tool" ON)
mark_as_advanced(BUILD_CLANG_FCO)

if(BUILD_CLANG_FCO)
  # Find llvm-config
  find_program(LLVM_CONFIG_PATH "llvm-config")
  if(NOT LLVM_CONFIG_PATH)
    message(FATAL_ERROR "llvm-config not found")
  endif()
  message(STATUS "Found LLVM_CONFIG as ${LLVM_CONFIG_PATH}")

  # This function saves the output of the llvm-config command with the given
  # flag to the variable named VARNAME, example: llvm_config(LLVM_CXXFLAGS "--cxxflags")
  function(llvm_config VARNAME flag)
    set(CONFIG_COMMAND "${LLVM_CONFIG_PATH}" "${flag}")
    execute_process(
      COMMAND ${CONFIG_COMMAND}
      RESULT_VARIABLE HAD_ERROR
      OUTPUT_VARIABLE CONFIG_OUTPUT
    )

    if(HAD_ERROR)
      string(REPLACE ";" " " CONFIG_COMMAND_STR "${CONFIG_COMMAND}")
      message(STATUS "${CONFIG_COMMAND_STR}")
      message(FATAL_ERROR "llvm-config failed with status ${HAD_ERROR}")
    endif()

    # replace linebreaks with semicolon
    string(REGEX REPLACE "[ \t]*[\r\n]+[ \t]*" ";" CONFIG_OUTPUT ${CONFIG_OUTPUT})

    # make result available outside
    set(${VARNAME} ${CONFIG_OUTPUT} PARENT_SCOPE)

    message(STATUS "llvm_config(${VARNAME})=>${CONFIG_OUTPUT}")
    unset(CONFIG_COMMAND)
  endfunction(llvm_config)

  llvm_config(LLVM_CXXFLAGS "--cxxflags")
  llvm_config(LLVM_LDFLAGS "--ldflags")
  llvm_config(LLVM_LIBS "--libs")
  llvm_config(LLVM_LIBDIR "--libdir")
  llvm_config(LLVM_INCLUDE_DIR "--includedir")
  llvm_config(LLVM_SYSTEM_LIBS "--system-libs")

  add_definitions(${LLVM_CXXFLAGS})
  add_definitions(-fno-rtti)
  include_directories(${LLVM_INCLUDE_DIR})
  link_directories(${LLVM_LIBDIR})

  add_executable(clang-cfo
    ClangFCO.cpp
    )

  target_link_libraries(clang-cfo
    ${LLVM_LDFLAGS}
    -lLLVM
    -lclangTooling
    -lclangFrontendTool
    -lclangFrontend
    -lclangDriver
    -lclangSerialization
    -lclangParse
    -lclangSema
    -lclangAnalysis
    -lclangEdit
    -lclangAST
    -lclangLex
    -lclangBasic
    -lclangASTMatchers
    -lclangRewriteFrontend
    -lclangStaticAnalyzerFrontend
    ${LLVM_LIBS}
  )
endif()
