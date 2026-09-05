#pragma once

#include <array>
#include <vector>

#include "lottopicker/CooccurrenceScorer.h" // kMinGroupSize/kMaxGroupSize, GroupKey, CooccurrenceScores
#include "lottopicker/CsvIngestor.h"        // kPoolMin/kCurrentPoolMax
#include "lottopicker/DecayScorer.h"        // kDefaultHalfLifeDraws
#include "lottopicker/DrawRecord.h"

namespace lottopicker {

// CORE-206: normalizes CORE-200/CORE-201's raw, per-era-inconsistent
// counts into observed-minus-chance-expected values ("surprise", not
// raw occurrence count) so a draw recorded under a smaller historical
// pool size is combined with current-era draws on the same probability
// scale, rather than discarded. Method resolved in docs/SDD.md's
// Algorithm design section, informed by CORE-207's research (the
// hypergeometric framework, source 3):
//
//   p(g, n) = C(n-g, 6-g) / C(n, 6)
//   norm_decay(k)  = Sum_d  w(age(d)) * ( [k in draw(d)]     - 6/n_era(d) )
//   norm_cooc(grp) = Sum_d  w(age(d)) * ( [grp subset draw(d)] - p(|grp|, n_era(d)) )
//
// `n_era(d)` is the pool size in effect for draw `d`, per DATA-IN-101's
// EraTagger (`DrawRecord::poolSize`). Requires records to already be
// tagged before normalizing.
//
// Key algebraic simplification this implementation relies on: since
// `6/n_era(d)` in norm_decay and `p(|grp|, n_era(d))` in norm_cooc do
// not depend on `k`/`grp` themselves (only on group size and the
// draw's era), each raw score's *baseline* (the chance-expected part)
// is a single scalar per group size, shared across every number/group
// of that size:
//
//   norm_decay(k)   = decay_score(k) - baselineDecay
//   norm_cooc(grp)  = cooc_score(grp) - baselineCooc[|grp|]
//   baselineDecay    = Sum_d w(age(d)) * 6/n_era(d)
//   baselineCooc[g]  = Sum_d w(age(d)) * p(g, n_era(d))
//
// This lets normalization reuse CORE-200/CORE-201's already-computed
// raw scores directly (one extra pass over `history` to accumulate the
// baselines) rather than recomputing a per-number/per-group sum from
// scratch, and preserves CORE-201's sparse-storage contract: an
// unobserved group's norm_cooc is still just `0.0 - baselineCooc[g]`,
// no extra storage needed for groups never seen.
class PoolSizeNormalizer {
public:
    // p(g, n): probability a specific group of `groupSize` numbers is
    // fully contained in one random 6-of-`poolSize` draw. Returns 0.0
    // for an invalid combination (`groupSize` outside
    // [kMinGroupSize, kMaxGroupSize], or `poolSize < kNumbersPerDraw`)
    // rather than dividing by zero or indexing invalid combinatorics.
    static double hypergeometricProbability(int groupSize, int poolSize);

    // expected_count(group, era) = p(groupSize, poolSize) * drawsInEra,
    // per docs/SDD.md's Algorithm design and TP-CORE-206 part 3.
    // Exposed standalone (not just folded into the baseline sums
    // above) so it can be hand-verified against a fixture directly.
    static double expectedCount(int groupSize, int poolSize, int drawsInEra);

    // Normalized results for every pool number in [poolMin, poolMax]
    // and every group actually observed in `history`.
    class Result {
    public:
        Result(int poolMin, int poolMax);

        // norm_decay(number). `number` must be within [poolMin, poolMax]
        // this Result was computed for.
        double normDecay(int number) const;

        // norm_cooc(sortedGroup). 0.0 - baselineCooc[size] for a group
        // never historically observed (consistent with CORE-201's
        // sparse-map contract: absence of a raw occurrence is a valid,
        // typically-negative, chance-expected "surprise", not an
        // error).
        double normCooc(const GroupKey &sortedGroup) const;

        int poolMin() const { return m_poolMin; }
        int poolMax() const { return m_poolMax; }

    private:
        friend class PoolSizeNormalizer;

        int m_poolMin;
        int m_poolMax;
        // Indexed by (number - m_poolMin); decay_score(k) - baselineDecay.
        std::vector<double> m_normDecay;
        // Raw co-occurrence scores (CORE-201), kept as-is (sparse) --
        // the baseline is subtracted lazily in normCooc() rather than
        // materializing a normalized copy of every observed group.
        CooccurrenceScores m_rawCooc;
        // Indexed by (groupSize - kMinGroupSize); baselineCooc[g].
        std::array<double, kGroupSizeCount> m_baselineCooc{};
    };

    // Computes norm_decay/norm_cooc over `history` (defaults mirror
    // CORE-200/CORE-201: widest documented pool, kDefaultHalfLifeDraws).
    // `history` must already be era-tagged (DATA-IN-101's EraTagger) --
    // a record with `poolSize == 0` (EraTagger's untagged sentinel, see
    // DrawRecord.h) or a negative value defensively falls back to
    // `kCurrentPoolMax` for that single draw's baseline contribution
    // (the widest, most-current documented pool) rather than dividing
    // by zero; this should not happen on a properly-tagged pipeline
    // run.
    static Result normalize(const std::vector<DrawRecord> &history, int poolMin = kPoolMin,
                            int poolMax = kCurrentPoolMax,
                            int halfLifeDraws = kDefaultHalfLifeDraws);
};

} // namespace lottopicker
