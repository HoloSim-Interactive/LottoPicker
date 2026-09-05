#include "lottopicker/DecayScorer.h"

#include <algorithm>
#include <cmath>

namespace lottopicker {

DecayScorer::Result::Result(int poolMin, int poolMax)
    : m_poolMin(poolMin), m_poolMax(poolMax),
      m_scores(static_cast<std::size_t>(poolMax - poolMin + 1), 0.0) {}

double DecayScorer::Result::at(int number) const {
    return m_scores[static_cast<std::size_t>(number - m_poolMin)];
}

DecayScorer::Result DecayScorer::score(const std::vector<DrawRecord> &history, int poolMin,
                                       int poolMax, int halfLifeDraws) {
    Result result(poolMin, poolMax);

    if (history.empty()) {
        return result;
    }

    // Guard against a non-positive half-life (division by zero / NaN
    // weights) rather than trusting the caller — see header comment.
    const double effectiveHalfLife = halfLifeDraws > 0 ? static_cast<double>(halfLifeDraws) : 1.0;

    // Sort a local copy by date so recency is determined by the draw's
    // actual date, not by whatever order `history` arrived in (see
    // header comment: CsvIngestor preserves file order only).
    std::vector<const DrawRecord *> byDate;
    byDate.reserve(history.size());
    for (const DrawRecord &record : history) {
        byDate.push_back(&record);
    }
    std::stable_sort(byDate.begin(), byDate.end(),
                     [](const DrawRecord *a, const DrawRecord *b) { return a->date < b->date; });

    const std::size_t mostRecentIndex = byDate.size() - 1;
    static const double kLn2 = std::log(2.0);

    for (std::size_t i = 0; i < byDate.size(); ++i) {
        const double age = static_cast<double>(mostRecentIndex - i);
        const double weight = std::exp(-kLn2 * age / effectiveHalfLife);
        for (const int number : byDate[i]->numbers) {
            if (number < poolMin || number > poolMax) {
                // Out-of-pool numbers should never reach this stage
                // (CsvIngestor validates against kPoolMin/kCurrentPoolMax
                // already), but a stale/narrower poolMin/poolMax pairing
                // is guarded against defensively rather than indexing
                // out of bounds.
                continue;
            }
            result.m_scores[static_cast<std::size_t>(number - poolMin)] += weight;
        }
    }

    return result;
}

} // namespace lottopicker
