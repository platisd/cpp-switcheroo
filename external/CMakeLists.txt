### GoogleTest
# Adopted from https://github.com/google/googletest/blob/master/googletest/README.md
include(FetchContent)
FetchContent_Declare(
  googletest
  # Specify the commit you depend on and update it regularly.
  URL https://github.com/google/googletest/archive/refs/tags/v1.13.0.zip
)
FetchContent_MakeAvailable(googletest)

# Prevent overriding the parent project's compiler/linker
# settings on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Fix pthread related crash with GMock when running on Cygwin
# This should be included *before* adding the googletest directory
if ("${CMAKE_SYSTEM_NAME}" STREQUAL "CYGWIN")
    add_definitions(-DTEST_HAS_PTHREAD=0)
    set(gtest_disable_pthreads ON)
endif ()

# GTest and GMock do not play nicely with these flags, so disable them
target_compile_options(gtest PRIVATE -Wno-ctor-dtor-privacy -Wno-conversion)
target_compile_options(gmock PRIVATE -Wno-ctor-dtor-privacy)

function(configure_test testExecutable)
    # Link against gtest library
    target_link_libraries(${testExecutable} gtest gtest_main gmock_main)
    # Disable variadic macro warnings (can be a problem when compiling with Clang)
    target_compile_options(${testExecutable} PRIVATE -Wno-gnu-zero-variadic-macro-arguments)
    # Create test name as the capitalized form of the test executable
    string(TOUPPER ${testExecutable} testName)
    # Add executable to test suite
    add_test(${testName} ${testExecutable} ${GTEST_RUN_FLAGS})
endfunction(configure_test)
