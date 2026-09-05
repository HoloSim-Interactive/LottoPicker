#include "lottopicker/PoolSizeNormalizer.h"

#include <algorithm>
#include <array>
#include <cmath>

namespace lottopicker {

namespace {

// C(n, k), computed via the standard incremental-product formula
// (never materializing large factorials). Every intermediate `result`
// is mathematically an integer by Pascal's-rule construction, and n/k
// stay small here (n <= a documented pool size, k <= kNumbersPerDraw),
// so double precision holds it exactly. Returns 0.0 for an
// out-of-domain (k < 0 or k > n) input rather than a negative/NaN
// result.
double nCr(int n, int k) {
    if (k < 0 || k > n) {
        return 0.0;
    }
    k = std::min(k, n - k); // C(n,k) == C(n,n-k); fewer multiplications.
    double result = 1.0;
    for (int i = 0; i < k; ++i) {
        result = result * static_cast<double>(n - i) / static_cast<double>(i + 1);
    }
    return result;
}

} // namespace

double PoolSizeNormalizer::hypergeometricProbability(int groupSize, int poolSize) {
    if (groupSize < kMinGroupSize || groupSize > kMaxGroupSize || poolSize < kNumbersPerDraw) {
        return 0.0;
    }
    const double denominator = nCr(poolSize, kNumbersPerDraw);
    if (denominator <= 0.0) {
        return 0.0;
    }
    return nCr(poolSize - groupSize, kNumbersPerDraw - groupSize) / denominator;
}

double PoolSizeNormalizer::expectedCount(int groupSize, int poolSize, int drawsInEra) {
    return hypergeometricProbability(groupSize, poolSize) * static_cast<double>(drawsInEra);
}

PoolSizeNormalizer::Result::Result(int poolMin, int poolMax)
    : m_poolMin(poolMin), m_poolMax(poolMax),
      m_normDecay(static_cast<std::size_t>(poolMax - poolMin + 1), 0.0) {}

double PoolSizeNormalizer::Result::normDecay(int number) const {
    return m_normDecay[static_cast<std::size_t>(number - m_poolMin)];
}

double PoolSizeNormalizer::Result::normCooc(const GroupKey &sortedGroup) const {
    const auto size = static_cast<int>(sortedGroup.size());
    if (size < kMinGroupSize || size > kMaxGroupSize) {
        return 0.0;
    }
    const double raw = CooccurrenceScorer::scoreFor(m_rawCooc, sortedGroup);
    return raw - m_baselineCooc[static_cast<std::size_t>(size - kMinGroupSize)];
}

PoolSizeNormalizer::Result PoolSizeNormalizer::normalize(const std::vector<DrawRecord> &history,
                                                         int poolMin, int poolMax,
                                                         int halfLifeDraws) {
    Result result(poolMin, poolMax);

    // Raw scores (CORE-200/CORE-201) already implement recency
    // weighting and chronological-order-independence; reuse them
    // directly rather than re-deriving the decay sums here (see
    // header comment's algebraic simplification).
    const DecayScorer::Result rawDecay =
        DecayScorer::score(history, poolMin, poolMax, halfLifeDraws);
    result.m_rawCooc = CooccurrenceScorer::score(history, halfLifeDraws);

    if (history.empty()) {
        for (int number = poolMin; number <= poolMax; ++number) {
            result.m_normDecay[static_cast<std::size_t>(number - poolMin)] = rawDecay.at(number);
        }
        return result;
    }

    // Same sort-by-date convention as DecayScorer/CooccurrenceScorer:
    // `history` need not arrive chronologically ordered.
    std::vector<const DrawRecord *> byDate;
    byDate.reserve(history.size());
    for (const DrawRecord &record : history) {
        byDate.push_back(&record);
    }
    std::stable_sort(byDate.begin(), byDate.end(),
                     [](const DrawRecord *a, const DrawRecord *b) { return a->date < b->date; });

    const int mostRecentIndex = static_cast<int>(byDate.size()) - 1;

    // baselineDecay = Sum_d w(age(d)) * 6/n_era(d)
    // baselineCooc[g] = Sum_d w(age(d)) * p(g, n_era(d)), g in [2,6]
    // Both are scalars per group size, not per number/group -- see
    // header comment.
    double baselineDecay = 0.0;
    std::array<double, kGroupSizeCount> baselineCooc{};

    for (int index = 0; index <= mostRecentIndex; ++index) {
        const DrawRecord &draw = *byDate[static_cast<std::size_t>(index)];
        const int age = mostRecentIndex - index;
        const double weight = CooccurrenceScorer::decayWeight(age, halfLifeDraws);

        // A record must be era-tagged (DATA-IN-101) before reaching
        // this stage; poolSize == 0 is EraTagger's "untagged" sentinel
        // (DrawRecord.h) and a negative value is likewise invalid --
        // both fall back defensively to the widest, most-current
        // documented pool rather than dividing by zero (should not
        // occur on a properly-tagged pipeline run).
        const int nEra = draw.poolSize > 0 ? draw.poolSize : kCurrentPoolMax;

        baselineDecay +=
            weight * (static_cast<double>(kNumbersPerDraw) / static_cast<double>(nEra));
        for (int groupSize = kMinGroupSize; groupSize <= kMaxGroupSize; ++groupSize) {
            baselineCooc[static_cast<std::size_t>(groupSize - kMinGroupSize)] +=
                weight * hypergeometricProbability(groupSize, nEra);
        }
    }

    for (int number = poolMin; number <= poolMax; ++number) {
        result.m_normDecay[static_cast<std::size_t>(number - poolMin)] =
            rawDecay.at(number) - baselineDecay;
    }
    result.m_baselineCooc = baselineCooc;

    return result;
}

} // namespace lottopicker
