#pragma once

#include <array>
#include <string>
#include <vector>

#include "lottopicker/CompositeScorer.h" // CompositeWeights
#include "lottopicker/CsvIngestor.h"     // kPoolMin/kCurrentPoolMax
#include "lottopicker/DecayScorer.h"     // kDefaultHalfLifeDraws
#include "lottopicker/DrawRecord.h"      // DrawRecord, kNumbersPerDraw
#include "lottopicker/EraTable.h"        // EraBoundary, eraTable()

namespace lottopicker {

// CORE-205's partial-match containment is reported at levels 3 through
// 6 (docs/RTVM.md: "partial-match containment counts at the 3/4/5/6
// level") -- levels 0-2 are not reported (nearly every retained
// top-N combination shares at least a couple of numbers with any given
// actual draw by pure chance, so they carry little signal and the RTVM
// item doesn't ask for them). Named distinctly from
// CooccurrenceScorer's kMinGroupSize/kMaxGroupSize (2..6): those bound
// a *co-occurrence group size* fed into scoring; these bound a
// *containment level* -- how many of the actual draw's 6 numbers one
// already-ranked candidate combination happens to share -- a different
// concept that is only coincidentally similar in range.
inline constexpr int kMinContainmentLevel = 3;
inline constexpr int kMaxContainmentLevel = kNumbersPerDraw;
inline constexpr std::size_t kContainmentLevelCount =
    static_cast<std::size_t>(kMaxContainmentLevel - kMinContainmentLevel + 1);

// DATA-OUT-302: one backtest report row for a single sampled date --
// the actual draw's numbers, its rank/percentile (or "not found"), and
// partial-match containment counts at the 3/4/5/6 level. This struct,
// as produced by CORE-205's BacktestEngine::run below, *is* DATA-OUT-302's
// report-row structure -- the same relationship RankedCombo (CORE-203)
// already has to DATA-OUT-300 (see RankingEngine.h's header comment).
// The full report for a `--backtest` invocation (UI-003) with N sample
// dates is simply `std::vector<BacktestResult>` with one entry per
// requested date, populated by calling `run` once per date (this class
// deliberately has no "list of dates" concept of its own -- see `run`'s
// doc comment below) -- no separate collection type is needed, mirroring
// how DATA-OUT-300's "ordered records for the retained top-N" is just
// `std::vector<RankedCombo>`, not its own named container. This vector
// is what OUT-401's BacktestReportPresenter consumes.
struct BacktestResult {
    // The requested sample date, echoed back (YYYY-MM-DD).
    std::string sampleDate;

    // The actual historical draw's numbers on `sampleDate` -- the
    // held-out combination the truncated model's ranked list is
    // compared against. Sorted ascending, same invariant as
    // DrawRecord::numbers.
    std::array<int, kNumbersPerDraw> actualDraw{};

    // The era-appropriate pool upper bound (docs/SDD.md's EraTable/
    // DATA-IN-101) used to both build the truncated model and bound
    // RankingEngine's combination domain for this sample date --
    // ranking against the *current* pool size would generate
    // combinations the actual draw could never have been drawn from
    // under an earlier, smaller-pool era.
    int poolMax = 0;

    // Size of the ranked list actually produced for this run (normally
    // the requested topN; only smaller if the era-appropriate
    // combination space itself has fewer members, mirroring
    // RankingEngine::rank's own "fewer only if..." contract).
    int topN = 0;

    // True iff actualDraw's exact 6-number combination appears
    // somewhere in the truncated model's topN ranked list (i.e.
    // observedContainment[kMaxContainmentLevel level] == 1). When
    // false, `rank`/`percentile` are left at their default (0/0.0) and
    // the caller must render "not found in top-N" rather than a
    // fabricated rank (TP-CORE-205 part 2) -- `found` is the single
    // authoritative flag for that decision, not `rank == 0` (0 is also
    // BacktestResult's default, so testing it directly would be
    // ambiguous).
    bool found = false;

    // 1-based rank within the topN ranked list, valid only if `found`.
    int rank = 0;

    // Percentile within the retained topN envelope, valid only if
    // `found`: 100.0 * (topN - rank + 1) / topN, so rank 1 reads as the
    // 100th percentile and rank topN reads as roughly the
    // (100/topN)th. Chosen over a percentile against the full
    // ~22.9M-combination space because CORE-203 never materializes
    // that space (there is no true global rank to compute a
    // full-space percentile from for anything outside the retained
    // topN) -- this formula is a Software Engineer judgment call, not
    // fixed by docs/SDD.md or docs/RTVM.md, since neither documents a
    // percentile formula and TP-CORE-205 doesn't assert a specific
    // value; flagged to Systems Engineer in this issue's hand-off in
    // case a different definition is wanted.
    double percentile = 0.0;

    // observedContainment[level - kMinContainmentLevel] = the number of
    // topN ranked combinations sharing exactly `level` numbers with
    // actualDraw (TP-CORE-205 part 1's "2 combinations in top-100
    // match exactly 4 of 6 numbers").
    std::array<int, kContainmentLevelCount> observedContainment{};

    // chanceExpectedContainment[level - kMinContainmentLevel] = the
    // number of topN entries CORE-206's own hypergeometric baseline
    // (PoolSizeNormalizer::hypergeometricProbability, reused verbatim,
    // not a new formula) would predict purely by chance, for
    // comparison against observedContainment above. This is the
    // probability a *specific* `level`-sized subset of actualDraw's
    // six numbers is fully contained in one random topMax-of-poolMax
    // combination, scaled by topN -- a documented approximation to
    // "chance-expected containment", reusing CORE-206's already-
    // Verified public formula rather than deriving a new exact-match
    // hypergeometric PMF; not exercised by TP-CORE-205's test
    // procedure (which only checks observed containment counts and the
    // not-found case), so this is a design choice with room to revisit
    // once DATA-OUT-302 fixes the report's exact presentation.
    std::array<double, kContainmentLevelCount> chanceExpectedContainment{};
};

// CORE-205: for one sample date, runs the model using only data
// available strictly before that date, then reports the actual draw's
// rank/percentile (or "not found in top-N") and partial-match
// containment counts at the 3/4/5/6 level -- docs/SDD.md's Top-level
// control flow diagram's "Truncate" + "Report" steps, run once per
// `--backtest` (UI-003) sample date by the caller (one BacktestEngine::run
// call per date; this class has no notion of "a list of dates" itself).
class BacktestEngine {
public:
    // `fullHistory` must already be ingested (DATA-IN-100) and era-
    // tagged (DATA-IN-101) -- the same precondition ModelStore::buildArtifact
    // and PoolSizeNormalizer::normalize already carry; this class adds
    // no new ingestion/tagging of its own.
    //
    // Held-out draw selection: the "actual draw" is the record in
    // `fullHistory` whose `date` equals `sampleDate` exactly (assumed
    // to name an actual historical draw date in the fixture/real
    // history, per docs/RTVM.md's TP-CORE-205 fixtures); the truncated
    // training set is every record dated *strictly before* `sampleDate`
    // (excluding that record itself, so the model is never trained on
    // the very answer it is being scored against) -- reads "data
    // available up to that date" (docs/RTVM.md's CORE-205 wording) as
    // excluding the sample date's own draw, the only reading under
    // which a backtest measures anything.
    //
    // Throws BacktestError if no record in `fullHistory` is dated
    // exactly `sampleDate` (nothing to hold out and compare against),
    // or if no record is dated strictly before it (no training data to
    // build a truncated model from) -- both are semantic problems with
    // how `sampleDate` relates to the actual supplied history, only
    // knowable once real history is available, unlike UI-003's
    // CliArgs::parse which only validates the flag's date-list syntax.
    //
    // `weights`/`poolMin`/`halfLifeDraws` are forwarded to the
    // truncated model's construction and ranking pass unchanged
    // (defaults mirror CompositeScorer/CsvIngestor/DecayScorer's own
    // documented defaults); `poolMax` is NOT a direct parameter here --
    // deliberately looked up per `sampleDate` via `eras`
    // (EraTable::poolSizeForDate) internally, since backtesting against
    // a pre-1999 sample date must rank within that era's smaller pool,
    // not the current 6/53 one. `eras` defaults to the real documented
    // `eraTable()` and is exposed as a parameter for the same reason
    // EraTagger::tag/poolSizeForDate already expose it: so a test
    // fixture can exercise this at a small, hand-tractable pool size
    // instead of the real (13.9M-/22.9M-combination) eras -- RankingEngine::rank's
    // full-space scan cost scales with the pool, and BacktestEngine::run
    // performs one such scan per call.
    static BacktestResult run(const std::vector<DrawRecord> &fullHistory,
                              const std::string &sampleDate, int topN,
                              const CompositeWeights &weights = CompositeWeights{},
                              int poolMin = kPoolMin, int halfLifeDraws = kDefaultHalfLifeDraws,
                              const std::vector<EraBoundary> &eras = eraTable());
};

} // namespace lottopicker
