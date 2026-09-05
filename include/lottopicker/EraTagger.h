#pragma once

#include <vector>

#include "lottopicker/DrawRecord.h"
#include "lottopicker/EraTable.h"

namespace lottopicker {

// DATA-IN-101: tags each already-ingested draw record with the
// number-pool size in effect on its draw date, so downstream scoring
// (CORE-200/201/206) can normalize across eras instead of assuming a
// constant pool size across all history.
//
// Runs strictly after CsvIngestor (DATA-IN-100) as its own pipeline
// stage (docs/SDD.md's two-stage pipeline diagram: Ingest -> Era
// Tagger -> scorers) -- it does not re-parse or re-validate anything
// CsvIngestor already handled; it only looks up and sets `poolSize`
// from an era table (EraTable.h).
class EraTagger {
public:
    // Sets `record.poolSize` for every record in `records`, in place,
    // based on `record.date` and `eras` (defaults to the real
    // documented table, `eraTable()`). All other fields are left
    // untouched.
    //
    // `eras` is a parameter (not hardcoded) so TP-DATA-IN-101 part 1's
    // synthetic fixture can exercise the tagging logic against a
    // hand-picked boundary date without depending on real-world
    // Florida Lotto history -- the same reasoning as CsvIngestor's
    // parameterized `poolMin`/`poolMax`.
    static void tag(std::vector<DrawRecord> &records,
                    const std::vector<EraBoundary> &eras = eraTable());
};

} // namespace lottopicker
