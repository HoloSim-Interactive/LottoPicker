#pragma once

#include <filesystem>
#include <optional>

#include "lottopicker/ModelArtifact.h"

namespace lottopicker {

// DATA-OUT-301: reads/writes a ModelArtifact using the documented
// plain-text format (docs/SDD.md's Interfaces & File Formats -> "Model
// artifact"):
//
//   LOTTOPICKER_MODEL v1
//   source_hash=<sha256 of data_file's bytes>
//   date_range=<earliest_date>,<latest_date>
//   draw_count=<n>
//   [per_number]
//   <number>,<decay_score>
//   ...
//   [group_scores:2]
//   <n1>,<n2>,<score>
//   ...
//   [group_scores:3..6] (same shape, more numbers per line)
//
// CORE-204's ModelStore uses this to persist a freshly built model and
// to load a previously persisted one back in.
class ModelSerializer {
public:
    // Writes `artifact` to `path`, overwriting any existing contents.
    // Scores are written via std::to_chars in shortest-round-trip mode
    // so TP-DATA-OUT-301's bit-for-bit round-trip clause holds.
    //
    // Throws ModelStoreError if `path` can't be opened for writing.
    static void write(const ModelArtifact &artifact, const std::filesystem::path &path);

    // Reads `path` back into a ModelArtifact.
    //
    // Throws ModelStoreError if `path` can't be opened/read, or its
    // contents don't match the documented format (missing/mismatched
    // header, unparseable field, out-of-order or missing section).
    static ModelArtifact read(const std::filesystem::path &path);

    // Resilient variant of read(): std::nullopt instead of throwing for
    // any failure (missing file, corrupt/foreign contents). ModelStore
    // uses this for its hash-check-driven load decision -- a
    // missing/corrupt artifact at the model path isn't a fatal, user-
    // facing error (per docs/SDD.md's Coding Standards, "no exceptions
    // for control flow" in internal algorithmic code): CORE-204 always
    // has a fallback, rebuilding from the source CSV. This is
    // implemented as a single try/catch around read() -- the one
    // sanctioned boundary for that, mirroring main()'s single
    // LottoPickerError catch, rather than scattering error-handling
    // throughout parsing.
    static std::optional<ModelArtifact> tryRead(const std::filesystem::path &path);
};

} // namespace lottopicker
