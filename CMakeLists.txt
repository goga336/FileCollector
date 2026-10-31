cmake_minimum_required(VERSION 3.16)
project(file_collector CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)


enable_testing()

find_package(Threads REQUIRED)

add_executable(
  file_collector_test
  tests/test_base.cpp
  tests/test_errors.cpp
  tests/test_chunks.cpp
  tests/test_read.cpp
  tests/test_threads.cpp
  src/file_collector.cpp
)

target_include_directories(
  file_collector_test
  PRIVATE inc
)

target_link_libraries(
  file_collector_test
  GTest::gtest_main
  Threads::Threads 
)

include(GoogleTest)
gtest_discover_tests(file_collector_test)

find_package(Doxygen)

if(Doxygen_FOUND)
    set(DOXYGEN_GENERATE_HTML          YES)
    set(DOXYGEN_GENERATE_LATEX         NO)
    set(DOXYGEN_EXTRACT_ALL            YES)
    set(DOXYGEN_EXTRACT_PRIVATE        YES)
    set(DOXYGEN_SOURCE_BROWSER         YES)
    set(DOXYGEN_QUIET                  YES)
    set(DOXYGEN_WARN_IF_UNDOCUMENTED   NO)
    set(DOXYGEN_USE_MDFILE_AS_MAINPAGE "${CMAKE_CURRENT_SOURCE_DIR}/README.md")

    doxygen_add_docs(
        doc
        ${CMAKE_CURRENT_SOURCE_DIR}/inc/file_collector.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/file_collector.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/README.md
        ALL
        USE_STAMP_FILE
        COMMENT "Generating API documentation with Doxygen"
    )
endif()