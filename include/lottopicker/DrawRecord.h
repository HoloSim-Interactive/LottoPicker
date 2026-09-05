#pragma once

#include <array>
#include <string>

namespace lottopicker {

// How many numbers make up one Florida Lotto draw. Fixed by the game
// itself (docs/SDD.md's CSV format: `date,n1,n2,n3,n4,n5,n6`), not a
// config value.
inline constexpr int kNumbersPerDraw = 6;

// One in-memory record per historical draw (DATA-IN-100): a date plus
// the six numbers drawn. `numbers` is always stored sorted ascending
// (docs/SDD.md's Interfaces & File Formats: "Ingestion sorts each row's
// six numbers ascending internally regardless of file order, so group
// keys (CORE-201) hash consistently") — CsvIngestor guarantees this
// invariant; nothing downstream needs to re-sort.
//
// `date` is kept as the raw `YYYY-MM-DD` string rather than a calendar
// type: no other stage needs calendar arithmetic on it (decay scoring
// operates on draw *index*, not calendar time, per CORE-200), so a
// dependency on <chrono>'s calendar types (or a hand-rolled date type)
// would be unused complexity. DATA-IN-101's era tagging compares this
// string lexicographically against era boundary dates, which works
// correctly for zero-padded ISO-8601 dates.
//
// `poolSize` is 0 until DATA-IN-101's EraTagger runs (CsvIngestor
// itself doesn't set it -- ingestion validates against the *widest*
// documented pool, see CsvIngestor.h, and stays decoupled from era
// logic). A value of 0 therefore means "not yet tagged", never a real
// pool size.
struct DrawRecord {
    std::string date;
    std::array<int, kNumbersPerDraw> numbers{};
    int poolSize = 0;

    bool operator==(const DrawRecord &other) const {
        return date == other.date && numbers == other.numbers && poolSize == other.poolSize;
    }
};

} // namespace lottopicker
