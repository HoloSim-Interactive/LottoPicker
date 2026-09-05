#include <catch2/catch_test_macros.hpp>

#include "lottopicker/Version.h"

// Proves the CMake + Catch2 + ctest harness works end-to-end (Generate
// Code Base). Real feature tests land alongside each RTVM-tracked
// source file, not here.
TEST_CASE("Build harness is wired end-to-end", "[smoke]") {
    REQUIRE(lottopicker::kVersion != nullptr);
    REQUIRE(1 + 1 == 2);
}
