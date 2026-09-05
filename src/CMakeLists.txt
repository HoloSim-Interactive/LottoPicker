# lottopicker_lib holds all domain/application logic (CSV ingestion, era
# tagging, scoring, ranking, backtesting, model persistence — see
# docs/SDD.md's pipeline). It stays separate from the `lottopicker`
# executable so tests/ can link the logic directly without going through
# main()/argv.
#
# INTERFACE for now: no feature source files exist yet (this is the
# Generate Code Base scaffold, RTVM-tracked feature issues add the first
# ones). When the first .cpp lands, change this to:
#   add_library(lottopicker_lib STATIC <sources...>)
#   target_include_directories(lottopicker_lib PUBLIC ${CMAKE_SOURCE_DIR}/include)
#   target_compile_features(lottopicker_lib PUBLIC cxx_std_17)
add_library(lottopicker_lib INTERFACE)
target_include_directories(lottopicker_lib INTERFACE "${CMAKE_SOURCE_DIR}/include")
target_compile_features(lottopicker_lib INTERFACE cxx_std_17)

add_executable(lottopicker main.cpp)
target_link_libraries(lottopicker PRIVATE lottopicker_lib)
