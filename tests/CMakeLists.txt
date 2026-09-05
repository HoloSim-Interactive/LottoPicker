# One test binary for the whole suite, registered with ctest via
# catch_discover_tests (docs/SDD.md "Testing framework"). Add new test
# .cpp files here as each RTVM-tracked feature lands its own tests
# alongside its source file.
add_executable(lottopicker_tests
    SmokeTest.cpp
    CompositeScorerTest.cpp
    CooccurrenceScorerTest.cpp
    ConfigTest.cpp
    CsvIngestorTest.cpp
    DecayScorerTest.cpp
    EraTaggerTest.cpp
    ModelSerializerTest.cpp
    ModelStoreTest.cpp
    PoolSizeNormalizerTest.cpp
    RankingEngineTest.cpp
    Sha256Test.cpp
)
target_link_libraries(lottopicker_tests PRIVATE lottopicker_lib Catch2::Catch2WithMain)

include(Catch)
catch_discover_tests(lottopicker_tests)
