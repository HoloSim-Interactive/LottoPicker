#include "lottopicker/EraTable.h"

namespace lottopicker {

int poolSizeForDate(const std::string &date, const std::vector<EraBoundary> &eras) {
    // eras is small (a handful of documented rule changes across the
    // game's whole history) and ascending by effectiveFrom, so a linear
    // scan for the last boundary not after `date` is simple and fast
    // enough -- no need for binary search machinery over a table this
    // size.
    int poolSize = eras.front().poolSize;
    for (const EraBoundary &era : eras) {
        if (date >= era.effectiveFrom) {
            poolSize = era.poolSize;
        } else {
            break;
        }
    }
    return poolSize;
}

} // namespace lottopicker
