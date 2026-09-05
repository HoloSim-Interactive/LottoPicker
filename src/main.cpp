#include <cstdlib>
#include <iostream>

#include "lottopicker/Version.h"

// Host/entry point only — no domain logic here (see lottopicker_lib in
// src/CMakeLists.txt). CLI argument handling (config path, --backtest)
// is implemented incrementally by RTVM UI-001/UI-002/UI-003; this stub
// only proves the executable configures, builds, links, and runs
// end-to-end for the Generate Code Base scaffold.
int main(int argc, char** argv) {
    (void)argc;
    (void)argv;
    std::cout << "lottopicker " << lottopicker::kVersion << "\n";
    return EXIT_SUCCESS;
}
