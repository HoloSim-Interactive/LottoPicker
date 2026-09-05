# lottopicker_lib holds all domain/application logic (CLI arg parsing, CSV
# ingestion, era tagging, scoring, ranking, backtesting, model persistence
# — see docs/SDD.md's pipeline). It stays separate from the `lottopicker`
# executable so tests/ can link the logic directly without going through
# main()/argv.
#
# STATIC now that feature source files have landed (CliArgs.cpp, RTVM
# UI-001; CsvIngestor.cpp, RTVM DATA-IN-100; DecayScorer.cpp, RTVM
# CORE-200; CooccurrenceScorer.cpp, RTVM CORE-201; Config.cpp, RTVM
# UI-002). Add each new feature's .cpp here as it lands.
add_library(lottopicker_lib STATIC
    CliArgs.cpp
    CooccurrenceScorer.cpp
    Config.cpp
    CsvIngestor.cpp
    DecayScorer.cpp
)
target_include_directories(lottopicker_lib PUBLIC "${CMAKE_SOURCE_DIR}/include")
target_compile_features(lottopicker_lib PUBLIC cxx_std_17)

add_executable(lottopicker main.cpp)
target_link_libraries(lottopicker PRIVATE lottopicker_lib)
