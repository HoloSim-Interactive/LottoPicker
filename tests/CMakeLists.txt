# One test binary for the whole suite, registered with ctest via
# catch_discover_tests (docs/SDD.md "Testing framework"). Add new test
# .cpp files here as each RTVM-tracked feature lands its own tests
# alongside its source file.
add_executable(lottopicker_tests
    SmokeTest.cpp
    CsvIngestorTest.cpp
)
target_link_libraries(lottopicker_tests PRIVATE lottopicker_lib Catch2::Catch2WithMain)

include(Catch)
catch_discover_tests(lottopicker_tests)
