#pragma once

#include <string>

namespace lottopicker {

// Minimal, self-contained SHA-256 (FIPS 180-4) implementation -- no
// external crypto dependency, per docs/SDD.md's Dependency policy
// ("standard library plus Catch2 (test-only) only for MVP"). Used
// solely to compute CORE-204's `source_hash`: a stable, content-based
// fingerprint of the historical draw CSV's bytes so a rebuild-vs-reuse
// decision can be made without re-parsing the whole file. Not intended
// for anything security-sensitive.
class Sha256 {
public:
    // Lowercase hex digest (64 characters) of `data`'s bytes.
    static std::string hexDigest(const std::string &data);
};

} // namespace lottopicker
