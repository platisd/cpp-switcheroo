# Dummy test
add_executable(switcheroo_test SwitcherooTest.cpp)
target_include_directories(switcheroo_test PRIVATE ${CMAKE_SOURCE_DIR})
configure_test(switcheroo_test)
