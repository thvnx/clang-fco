# Derived from:
#   https://github.com/include-what-you-use/include-what-you-use (393a5fa)

cmake_minimum_required(VERSION 3.4.3)

if (CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  message(STATUS "ClangFCO: out-of-tree configuration")
  set(ClangFCO_IN_TREE OFF)
else()
  message(STATUS "ClangFCO: in-tree configuration")
  set(ClangFCO_IN_TREE ON)
endif()

if (NOT ClangFCO_IN_TREE)
  cmake_policy(SET CMP0048 NEW)
  project(clang-fco)

  find_package(LLVM CONFIG REQUIRED)
  find_package(Clang CONFIG REQUIRED)

  list(APPEND CMAKE_MODULE_PATH ${LLVM_DIR})
  include(AddLLVM)
  include(HandleLLVMOptions)
endif()

message(STATUS "ClangFCO: configuring for LLVM ${LLVM_VERSION}...")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_definitions(${LLVM_DEFINITIONS})
include_directories(
  ${LLVM_INCLUDE_DIRS}
  ${CLANG_INCLUDE_DIRS}
  )

set(LLVM_LINK_COMPONENTS
  Support
  )

# in-tree uses add_clang_executable!?
add_llvm_executable(clang-fco
  ClangFCO.cpp
  )

# Synthesize a clang-resource-headers target for out-of-tree builds (in-tree
# already has it available by default)
if (NOT ClangFCO_IN_TREE)
  # Use only major.minor.patch for the resource directory structure; some
  # platforms include suffix in LLVM_VERSION.
  set(llvm_ver ${LLVM_VERSION_MAJOR}.${LLVM_VERSION_MINOR}.${LLVM_VERSION_PATCH})
  set(clang_headers_src ${CMAKE_PREFIX_PATH}/lib/clang/${llvm_ver}/include)
  set(clang_headers_dst ${CMAKE_BINARY_DIR}/lib/clang/${llvm_ver}/include)

  file(GLOB_RECURSE in_files RELATIVE ${clang_headers_src} ${clang_headers_src}/*)

  set(out_files)
  foreach (file ${in_files})
    set(src ${clang_headers_src}/${file})
    set(dst ${clang_headers_dst}/${file})

    add_custom_command(OUTPUT ${dst}
      DEPENDS ${src}
      COMMAND ${CMAKE_COMMAND} -E copy_if_different ${src} ${dst}
      COMMENT "Copying clang's ${file}...")
    list(APPEND out_files ${dst})
  endforeach()

  add_custom_target(clang-resource-headers ALL DEPENDS ${out_files})
endif()

# Add a dependency on clang-resource-headers to ensure the builtin headers are
# available when ClangFCO is executed from the build dir.
add_dependencies(clang-fco clang-resource-headers)

if (MINGW)
  # Work around 'too many sections' error with MINGW/GCC
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wa,-mbig-obj")
endif()

if (MSVC)
  # Disable warnings for ClangFCO, and disable exceptions in MSVC's STL.
  add_definitions(
    -wd4722 # Suppress ''destructor'' : destructor never returns, potential memory leak
    -D_HAS_EXCEPTIONS=0
    )

  # Enable bigobj support and sane C++ exception semantics.
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj /EHsc")
endif()

# If only clang-cpp is available, we take that.
if (TARGET clang-cpp AND NOT TARGET clangBasic)
  target_link_libraries(clang-fco PRIVATE clang-cpp)
else()
  target_link_libraries(clang-fco
    PRIVATE
    clangAST
    clangASTMatchers
    clangBasic
    clangFrontend
    clangTooling
    clangToolingRefactoring
    )
endif()

# Platform dependencies.
if (WIN32)
  target_link_libraries(clang-fco
    PRIVATE
    shlwapi  # For PathMatchSpecA
    )
endif()

# Pick up Git revision so we can report it in version information.
include(FindGit)
if (GIT_FOUND AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.git")
  execute_process(COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_VARIABLE ClangFCO_GIT_REV
    OUTPUT_STRIP_TRAILING_WHITESPACE)
else()
  message(STATUS "Warning: ClangFCO Git version info not found, DO NOT release "
                 "from this build tree!")
endif()
add_definitions(-DClangFCO_GIT_REV="${ClangFCO_GIT_REV}")

# Install programs
include(GNUInstallDirs)
install(TARGETS clang-fco RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
# install(PROGRAMS fix_includes.py iwyu_tool.py DESTINATION ${CMAKE_INSTALL_BINDIR})

# # Install mapping files
# file(GLOB MAPPING_FILES *.imp)
# install(FILES ${MAPPING_FILES} DESTINATION ${CMAKE_INSTALL_DATADIR}/clang-fco)

# # Install man page on Unix-like systems
# if (UNIX)
#   install(FILES clang-fco.1 DESTINATION ${CMAKE_INSTALL_MANDIR}/man1)
# endif()

find_package(PythonInterp)
if(PYTHONINTERP_FOUND)
  enable_testing()
  add_test(NAME cfco_tests
    COMMAND ${PYTHON_EXECUTABLE} run_iwyu_tests.py -- $<TARGET_FILE:clang-fco>
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
  add_test(NAME fix_includes_test
    COMMAND ${PYTHON_EXECUTABLE} fix_includes_test.py
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
  add_test(NAME cfco_tool_test
    COMMAND ${PYTHON_EXECUTABLE} iwyu_tool_test.py
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endif()
