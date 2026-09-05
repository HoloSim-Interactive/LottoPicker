#include "lottopicker/CompositeScorer.h"

#include <cstddef>

namespace lottopicker {

double CompositeScorer::score(const PoolSizeNormalizer::Result &normalized,
                              const std::array<int, kNumbersPerDraw> &combo,
                              const CompositeWeights &weights) {
    double total = 0.0;

    for (const int number : combo) {
        total += weights.numberWeight * normalized.normDecay(number);
    }

    // Enumerate every subset of size 2..6 of `combo`'s six numbers via a
    // kNumbersPerDraw-bit mask, same technique as
    // CooccurrenceScorer::score. kMaxGroupSize == kNumbersPerDraw, so
    // the mask == all-ones case (the full combo itself) is the only
    // size-6 subset, matching the formula's g=6 term.
    const unsigned maskLimit = 1u << kNumbersPerDraw;
    for (unsigned mask = 1; mask < maskLimit; ++mask) {
        const int size = static_cast<int>(std::bitset<kNumbersPerDraw>(mask).count());
        if (size < kMinGroupSize || size > kMaxGroupSize) {
            continue;
        }

        GroupKey group;
        group.reserve(static_cast<std::size_t>(size));
        for (int bit = 0; bit < kNumbersPerDraw; ++bit) {
            if (mask & (1u << bit)) {
                group.push_back(combo[static_cast<std::size_t>(bit)]);
            }
        }
        // `combo` is sorted ascending and bits are visited in ascending
        // index order, so `group` is already sorted -- matches
        // PoolSizeNormalizer::Result::normCooc's sorted-group contract.

        total += weights.groupWeights[static_cast<std::size_t>(size - kMinGroupSize)] *
                 normalized.normCooc(group);
    }

    return total;
}

} // namespace lottopicker
