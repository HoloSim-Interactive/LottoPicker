#pragma once

#include <vector>

#include "lottopicker/CsvIngestor.h" // kPoolMin/kCurrentPoolMax
#include "lottopicker/DrawRecord.h"

namespace lottopicker {

// Half-life, in draws (not calendar time), for CORE-200's exponential
// recency decay. Defaults to 104 (~1 year at Florida Lotto's
// twice-weekly cadence) per docs/SDD.md's Algorithm Design. A compiled
// default, not a required config key (UI-002 only fixes
// `data_file`/`top_n` as the minimum) — may be exposed as an optional
// config key later without an RTVM change.
inline constexpr int kDefaultHalfLifeDraws = 104;

// Per-number frequency-decay score (CORE-200): a recency-weighted
// count of historical occurrences per pool number, so more recent
// draws weigh more heavily than older ones. This is the *raw* score —
// CORE-206's chance-expected normalization (observed-minus-expected
// across pool-size eras) is layered on top of this in a later,
// separate stage; this class only computes the documented formula
// below.
//
// docs/SDD.md's Algorithm Design:
//   decay_score(k) = Sum_d  w(age(d)) * [k in draw(d)]
//   w(age)         = exp(-ln(2) * age / HALF_LIFE_DRAWS)
// where age(d) is the number of draws between draw `d` and the most
// recent draw in the supplied history (draw-index based, not calendar
// time, so behavior is deterministic regardless of gaps in the draw
// schedule).
class DecayScorer {
public:
    // decay_score(k) for every pool number k in [poolMin, poolMax],
    // computed over `history`.
    //
    // `history` need not already be ordered chronologically: CsvIngestor
    // preserves file order rather than guaranteeing it's date-sorted, so
    // recency here is determined by sorting a local copy by `DrawRecord::date`
    // (lexicographically, which is correct for zero-padded ISO-8601 dates —
    // same convention DATA-IN-101 relies on) rather than by trusting
    // caller/vector order. The most recent draw by date has age 0.
    //
    // A pool number with zero occurrences in `history` gets the
    // documented floor score of 0.0 (TP-CORE-200 case 2), not an error
    // and not an absent entry.
    class Result {
    public:
        Result(int poolMin, int poolMax);

        // decay_score(number). `number` must be within [poolMin, poolMax]
        // this Result was computed for.
        double at(int number) const;

        int poolMin() const { return m_poolMin; }
        int poolMax() const { return m_poolMax; }

    private:
        friend class DecayScorer;

        int m_poolMin;
        int m_poolMax;
        // Indexed by (number - m_poolMin); one entry per pool number.
        std::vector<double> m_scores;
    };

    // Computes decay_score(k) for k in [poolMin, poolMax] (defaults to
    // the widest documented pool, see kPoolMin/kCurrentPoolMax) over
    // `history`, using half-life `halfLifeDraws` (defaults to
    // kDefaultHalfLifeDraws). `halfLifeDraws` must be positive; a
    // non-positive value is clamped to 1 to avoid division-by-zero
    // rather than throwing (this is a compiled/config default, not
    // user input requiring the validation-error contract).
    static Result score(const std::vector<DrawRecord> &history, int poolMin = kPoolMin,
                        int poolMax = kCurrentPoolMax, int halfLifeDraws = kDefaultHalfLifeDraws);
};

} // namespace lottopicker
