#pragma once

#include <cstddef>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <vector>

#include "lottopicker/DrawRecord.h"

namespace lottopicker {

// Lower bound of the Florida Lotto number pool. Unchanged across every
// documented rule era (docs/SDD.md's Algorithm Design: the 1988 6/49
// launch and 1999 6/53 move only ever raised the upper bound).
inline constexpr int kPoolMin = 1;

// Upper bound of the *current* number pool (6/53), used by CsvIngestor
// to reject numbers that could never be valid under any documented
// era. This is deliberately the widest bound, not an era-specific one:
// DATA-IN-101 (not yet implemented) tags each ingested record with the
// pool size in effect on its own draw date for downstream *scoring*
// normalization; DATA-IN-100 only needs to catch numbers that are
// invalid full stop (e.g. 60, 0, -3), so it validates against the
// widest historical pool rather than duplicating era logic here.
inline constexpr int kCurrentPoolMax = 53;

// Thrown for whole-file structural problems that make row-by-row
// ingestion meaningless to attempt: the file can't be opened/read, or
// the header line doesn't match the documented
// `date,n1,n2,n3,n4,n5,n6` format. Distinct from a malformed *row*
// (DATA-IN-100's partial-file tolerance: a bad row is reported and
// skipped, not fatal) — caught once at main()'s boundary per
// docs/SDD.md's Coding Standards "Error handling" convention.
class CsvFormatError : public std::runtime_error {
public:
    explicit CsvFormatError(const std::string &message) : std::runtime_error(message) {}
};

// One malformed-row report: `row` is the 1-based physical line number
// within the CSV file (header counts as row 1, matching what a user
// sees opening the file in a text editor or spreadsheet), `message` is
// the specific reason, ready to be reported verbatim as
// `row <n>: <message>` per docs/SDD.md's documented format.
struct RowError {
    std::size_t row = 0;
    std::string message;
};

// Result of ingesting one CSV file: every well-formed row becomes a
// DrawRecord; every malformed row becomes a RowError instead. Both are
// populated together — a malformed row never prevents the remaining
// rows from ingesting (DATA-IN-100's partial-file tolerance).
struct IngestResult {
    std::vector<DrawRecord> records;
    std::vector<RowError> errors;
};

// Ingests the client-supplied historical draw CSV (DATA-IN-100) into
// one in-memory DrawRecord per well-formed row.
class CsvIngestor {
public:
    // Reads `csvPath` and parses every data row.
    //
    // Throws CsvFormatError if the file can't be opened/read, or if the
    // header row doesn't match `date,n1,n2,n3,n4,n5,n6` exactly.
    //
    // Never throws for a per-row problem (wrong count of numbers,
    // non-numeric field, out-of-pool-range number, duplicate number
    // within a row, malformed date) — each is instead appended to the
    // returned IngestResult::errors, and the row is excluded from
    // IngestResult::records. `poolMin`/`poolMax` default to the widest
    // documented pool (see kPoolMin/kCurrentPoolMax) and are exposed as
    // parameters so a future caller (e.g. a stricter single-era check)
    // can narrow them without changing this signature.
    static IngestResult ingest(const std::filesystem::path &csvPath, int poolMin = kPoolMin,
                               int poolMax = kCurrentPoolMax);
};

} // namespace lottopicker
