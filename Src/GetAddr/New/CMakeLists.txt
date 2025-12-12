cmake_minimum_required(VERSION 3.28)
project(test)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(DwarfCpp)
file(GLOB LIB_SOURCE
        ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
)
target_sources(DwarfCpp PRIVATE
        ${LIB_SOURCE}
)
target_include_directories(DwarfCpp PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
find_package(libdwarf CONFIG REQUIRED)
target_link_libraries(DwarfCpp PUBLIC libdwarf::dwarf)



include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(
        DwarfCppTest
        Gtest.cpp
)
target_link_libraries(
        DwarfCppTest
        DwarfCpp
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(DwarfCppTest)

