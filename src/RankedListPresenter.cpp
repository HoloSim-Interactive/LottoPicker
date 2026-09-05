#include "lottopicker/RankedListPresenter.h"

#include <algorithm>
#include <cstddef>
#include <iomanip>
#include <sstream>
#include <string>

namespace lottopicker {

namespace {

// Decimal digits shown for each score value -- see the header comment
// for why this differs from DATA-OUT-301's round-trip precision.
constexpr int kScorePrecision = 6;

std::string formatCombo(const RankedCombo &entry) {
    std::ostringstream combo;
    bool first = true;
    for (int number : entry.combo) {
        if (!first) {
            combo << ' ';
        }
        combo << number;
        first = false;
    }
    return combo.str();
}

std::string formatScore(double score) {
    std::ostringstream out;
    out << std::fixed << std::setprecision(kScorePrecision) << score;
    return out.str();
}

} // namespace

void RankedListPresenter::print(std::ostream &out, const std::vector<RankedCombo> &ranked) {
    std::vector<std::string> rankColumn{"Rank"};
    std::vector<std::string> comboColumn{"Combination"};
    std::vector<std::string> scoreColumn{"Score"};
    rankColumn.reserve(ranked.size() + 1);
    comboColumn.reserve(ranked.size() + 1);
    scoreColumn.reserve(ranked.size() + 1);

    for (const RankedCombo &entry : ranked) {
        rankColumn.push_back(std::to_string(entry.rank));
        comboColumn.push_back(formatCombo(entry));
        scoreColumn.push_back(formatScore(entry.score));
    }

    std::size_t rankWidth = 0;
    std::size_t comboWidth = 0;
    std::size_t scoreWidth = 0;
    for (const std::string &s : rankColumn) {
        rankWidth = std::max(rankWidth, s.size());
    }
    for (const std::string &s : comboColumn) {
        comboWidth = std::max(comboWidth, s.size());
    }
    for (const std::string &s : scoreColumn) {
        scoreWidth = std::max(scoreWidth, s.size());
    }

    // Row 0 is the header; rows 1..ranked.size() are the data rows --
    // exactly ranked.size() + 1 lines written in total, matching
    // TP-OUT-400's "header row" plus "exactly N rows" wording.
    for (std::size_t i = 0; i < rankColumn.size(); ++i) {
        out << std::left << std::setw(static_cast<int>(rankWidth)) << rankColumn[i] << " | "
            << std::left << std::setw(static_cast<int>(comboWidth)) << comboColumn[i] << " | "
            << std::right << std::setw(static_cast<int>(scoreWidth)) << scoreColumn[i] << "\n";
    }
}

} // namespace lottopicker
