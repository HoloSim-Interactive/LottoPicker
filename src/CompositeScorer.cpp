#include "lottopicker/CompositeScorer.h"

#include <cstddef>

namespace lottopicker {

namespace {

// Shared implementation for both CompositeScorer::score() overloads:
// enumerates every subset of `combo`'s six numbers via a
// kNumbersPerDraw-bit mask (same technique as
// CooccurrenceScorer::score), applying `normDecayOf`/`normCoocOf` to
// build the composite sum. Templated on the two lookup callables rather
// than a shared abstract interface so this stays a plain inlined loop
// with no virtual-dispatch overhead -- CORE-203 calls this once per
// candidate combination, ~22.9M times per ranking run.
template <typename NormDecayFn, typename NormCoocFn>
double computeComposite(NormDecayFn &&normDecayOf, NormCoocFn &&normCoocOf,
                        const std::array<int, kNumbersPerDraw> &combo,
                        const CompositeWeights &weights) {
    double total = 0.0;

    for (const int number : combo) {
        total += weights.numberWeight * normDecayOf(number);
    }

    // kMaxGroupSize == kNumbersPerDraw, so the mask == all-ones case
    // (the full combo itself) is the only size-6 subset, matching the
    // formula's g=6 term.
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
        // index order, so `group` is already sorted -- matches both
        // lookup contracts' sorted-group requirement.

        total += weights.groupWeights[static_cast<std::size_t>(size - kMinGroupSize)] *
                 normCoocOf(group);
    }

    return total;
}

} // namespace

double CompositeScorer::score(const PoolSizeNormalizer::Result &normalized,
                              const std::array<int, kNumbersPerDraw> &combo,
                              const CompositeWeights &weights) {
    return computeComposite(
        [&normalized](int number) { return normalized.normDecay(number); },
        [&normalized](const GroupKey &group) { return normalized.normCooc(group); }, combo,
        weights);
}

double CompositeScorer::score(const ModelArtifact &model,
                              const std::array<int, kNumbersPerDraw> &combo,
                              const CompositeWeights &weights) {
    return computeComposite(
        [&model](int number) {
            const auto it = model.perNumber.find(number);
            return it == model.perNumber.end() ? 0.0 : it->second;
        },
        [&model](const GroupKey &group) {
            const auto size = static_cast<int>(group.size());
            const auto sizeIt = model.groupScores.find(size);
            if (sizeIt != model.groupScores.end()) {
                const auto groupIt = sizeIt->second.find(group);
                if (groupIt != sizeIt->second.end()) {
                    return groupIt->second;
                }
            }
            // Never historically observed: still contributes its
            // chance-expected-only baseline, not 0.0 -- see
            // ModelArtifact::baselineCooc's comment.
            return 0.0 - model.baselineCooc[static_cast<std::size_t>(size - kMinGroupSize)];
        },
        combo, weights);
}

} // namespace lottopicker
