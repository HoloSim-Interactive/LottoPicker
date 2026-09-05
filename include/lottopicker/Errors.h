#pragma once

#include <stdexcept>
#include <string>

namespace lottopicker {

// Base class for every user-facing input/validation error (bad CLI args,
// bad config, malformed CSV row, ...). docs/SDD.md's Coding Standards ->
// Error handling: these are caught once at main()'s boundary and converted
// to the documented stderr-message + non-zero-exit-code contract (UI-001,
// UI-002, DATA-IN-100). Internal algorithmic code must not use exceptions
// for control flow -- only LottoPickerError (or a subclass) should ever
// propagate out of a public API up to main().
class LottoPickerError : public std::runtime_error {
public:
    explicit LottoPickerError(const std::string &message) : std::runtime_error(message) {}
};

// The command line itself is malformed (wrong argument count, unrecognized
// flag) -- as opposed to a syntactically-valid argument that fails
// validation (see CliConfigNotFoundError). Resolves UI-001's "missing
// ... argument" case.
class CliUsageError : public LottoPickerError {
public:
    explicit CliUsageError(const std::string &message) : LottoPickerError(message) {}
};

// A syntactically-valid config-path argument does not resolve to an
// existing, readable regular file. Resolves UI-001's "non-existent config
// path" case.
class CliConfigNotFoundError : public LottoPickerError {
public:
    explicit CliConfigNotFoundError(const std::string &message) : LottoPickerError(message) {}
};

} // namespace lottopicker
