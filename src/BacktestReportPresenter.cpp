#include "lottopicker/BacktestReportPresenter.h"

#include <algorithm>
#include <cstddef>
#include <iomanip>
#include <sstream>
#include <string>

namespace lottopicker {

namespace {

// Decimal digits shown for the percentile figure -- a human-facing
// precision choice, same rationale as RankedListPresenter's
// kScorePrecision (unrelated to DATA-OUT-301's round-trip precision).
constexpr int kPercentilePrecision = 2;

// The literal text TP-OUT-401/docs/SDD.md require for an actual draw
// that did not appear anywhere in the topN ranked list -- never a
// numeric placeholder like -1 or 0.
constexpr const char *kNotFoundText = "not found in top-N";

std::string formatDraw(const BacktestResult &entry) {
    std::ostringstream draw;
    bool first = true;
    for (int number : entry.actualDraw) {
        if (!first) {
            draw << ' ';
        }
        draw << number;
        first = false;
    }
    return draw.str();
}

std::string formatRank(const BacktestResult &entry) {
    if (!entry.found) {
        return kNotFoundText;
    }
    std::ostringstream rank;
    rank << entry.rank << "/" << entry.topN << " (" << std::fixed
         << std::setprecision(kPercentilePrecision) << entry.percentile << " pct)";
    return rank.str();
}

std::string formatContainment(const BacktestResult &entry) {
    std::ostringstream containment;
    bool first = true;
    for (int count : entry.observedContainment) {
        if (!first) {
            containment << ' ';
        }
        containment << count;
        first = false;
    }
    return containment.str();
}

} // namespace

void BacktestReportPresenter::print(std::ostream &out, const std::vector<BacktestResult> &report) {
    std::vector<std::string> dateColumn{"Sample Date"};
    std::vector<std::string> drawColumn{"Actual Draw"};
    std::vector<std::string> rankColumn{"Rank"};
    std::vector<std::string> containmentColumn{"Containment (3/4/5/6 of 6)"};
    dateColumn.reserve(report.size() + 1);
    drawColumn.reserve(report.size() + 1);
    rankColumn.reserve(report.size() + 1);
    containmentColumn.reserve(report.size() + 1);

    for (const BacktestResult &entry : report) {
        dateColumn.push_back(entry.sampleDate);
        drawColumn.push_back(formatDraw(entry));
        rankColumn.push_back(formatRank(entry));
        containmentColumn.push_back(formatContainment(entry));
    }

    std::size_t dateWidth = 0;
    std::size_t drawWidth = 0;
    std::size_t rankWidth = 0;
    std::size_t containmentWidth = 0;
    for (const std::string &s : dateColumn) {
        dateWidth = std::max(dateWidth, s.size());
    }
    for (const std::string &s : drawColumn) {
        drawWidth = std::max(drawWidth, s.size());
    }
    for (const std::string &s : rankColumn) {
        rankWidth = std::max(rankWidth, s.size());
    }
    for (const std::string &s : containmentColumn) {
        containmentWidth = std::max(containmentWidth, s.size());
    }

    // Row 0 is the header; rows 1..report.size() are the data rows --
    // exactly report.size() + 1 lines written in total, matching
    // TP-OUT-401's "exactly one ... row per year" wording.
    for (std::size_t i = 0; i < dateColumn.size(); ++i) {
        out << std::left << std::setw(static_cast<int>(dateWidth)) << dateColumn[i] << " | "
            << std::left << std::setw(static_cast<int>(drawWidth)) << drawColumn[i] << " | "
            << std::left << std::setw(static_cast<int>(rankWidth)) << rankColumn[i] << " | "
            << std::left << std::setw(static_cast<int>(containmentWidth)) << containmentColumn[i]
            << "\n";
    }
}

} // namespace lottopicker
