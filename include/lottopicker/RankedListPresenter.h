#pragma once

#include <ostream>
#include <vector>

#include "lottopicker/RankingEngine.h" // RankedCombo

namespace lottopicker {

// OUT-400: renders CORE-203/DATA-OUT-300's ranked list as a
// human-readable console table -- one header row followed by exactly
// `ranked.size()` data rows, in the same order the list is given
// (rank order is RankingEngine::rank's own contract; this class does
// not re-sort), per docs/SDD.md's "Console output" format ("a header
// row and clearly delimited ... columns"). Pure presentation: takes an
// already-computed std::vector<RankedCombo> and any std::ostream, with
// no dependency on ModelArtifact/RankingEngine's computation itself --
// trivially unit-testable against an std::ostringstream fixture
// without running a real ranking pass (matches TP-OUT-400's own
// wording, "given a DATA-OUT-300 fixture").
class RankedListPresenter {
public:
    // Writes one header row ("Rank | Combination | Score") followed by
    // one data row per entry of `ranked`, each column padded to the
    // widest value in that column (header included) and separated by
    // " | " -- unambiguous column delimiting regardless of how many
    // digits a rank or score happens to have (chosen over fixed-width
    // padding to an assumed max width, which docs/SDD.md also allows,
    // since scores have no fixed a priori magnitude). `combo` numbers
    // are printed space-separated in the entry's own (ascending)
    // order; `score` is printed with kScorePrecision fixed decimal
    // digits -- enough to distinguish adjacent ranks on a human-facing
    // table without full round-trip precision (unlike DATA-OUT-301's
    // persisted model, which needs bit-for-bit round-trip and uses far
    // more digits).
    static void print(std::ostream &out, const std::vector<RankedCombo> &ranked);
};

} // namespace lottopicker
