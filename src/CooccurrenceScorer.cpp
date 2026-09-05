#include "lottopicker/CooccurrenceScorer.h"

#include <algorithm>
#include <bitset>
#include <cmath>
#include <cstddef>

namespace lottopicker {

double CooccurrenceScorer::decayWeight(int age, int halfLifeDraws) {
    return std::exp(-std::log(2.0) * static_cast<double>(age) / static_cast<double>(halfLifeDraws));
}

CooccurrenceScores CooccurrenceScorer::score(const std::vector<DrawRecord> &history,
                                             int halfLifeDraws) {
    CooccurrenceScores result{};
    if (history.empty()) {
        return result;
    }

    // Order draws chronologically (oldest first) regardless of the
    // order `history` was supplied in, so "age" reflects draw-index
    // distance from the most recent draw, not vector position.
    std::vector<const DrawRecord *> byDate;
    byDate.reserve(history.size());
    for (const auto &record : history) {
        byDate.push_back(&record);
    }
    std::sort(byDate.begin(), byDate.end(),
              [](const DrawRecord *a, const DrawRecord *b) { return a->date < b->date; });

    const int mostRecentIndex = static_cast<int>(byDate.size()) - 1;

    for (int index = 0; index <= mostRecentIndex; ++index) {
        const DrawRecord &draw = *byDate[static_cast<std::size_t>(index)];
        const int age = mostRecentIndex - index;
        const double weight = decayWeight(age, halfLifeDraws);

        // Enumerate every subset of size 2..6 of this draw's six
        // (already sorted ascending, per DrawRecord's invariant)
        // numbers via a kNumbersPerDraw-bit mask. kMaxGroupSize ==
        // kNumbersPerDraw, so no subset larger than the draw itself
        // is possible.
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
                    group.push_back(draw.numbers[static_cast<std::size_t>(bit)]);
                }
            }
            // draw.numbers is sorted ascending and bits are visited in
            // ascending index order, so `group` is already sorted.

            result[static_cast<std::size_t>(size - kMinGroupSize)][group] += weight;
        }
    }

    return result;
}

double CooccurrenceScorer::scoreFor(const CooccurrenceScores &scores, const GroupKey &sortedGroup) {
    const auto size = static_cast<int>(sortedGroup.size());
    if (size < kMinGroupSize || size > kMaxGroupSize) {
        return 0.0;
    }
    const auto &map = scores[static_cast<std::size_t>(size - kMinGroupSize)];
    const auto it = map.find(sortedGroup);
    return it == map.end() ? 0.0 : it->second;
}

} // namespace lottopicker
