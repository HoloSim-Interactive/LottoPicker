# lottopicker_lib holds all domain/application logic (CSV ingestion, era
# tagging, scoring, ranking, backtesting, model persistence — see
# docs/SDD.md's pipeline). It stays separate from the `lottopicker`
# executable so tests/ can link the logic directly without going through
# main()/argv.
#
# STATIC since DATA-IN-100 (CsvIngestor) added the first feature source
# file. Add each subsequent RTVM feature's .cpp to the list below as it
# lands.
add_library(lottopicker_lib STATIC
    CsvIngestor.cpp
)
target_include_directories(lottopicker_lib PUBLIC "${CMAKE_SOURCE_DIR}/include")
target_compile_features(lottopicker_lib PUBLIC cxx_std_17)

add_executable(lottopicker main.cpp)
target_link_libraries(lottopicker PRIVATE lottopicker_lib)
