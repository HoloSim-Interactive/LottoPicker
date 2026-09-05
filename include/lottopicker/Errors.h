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

// The config file named by a syntactically-valid, existing path can't
// actually be opened/read (e.g. a permissions/race condition after
// CliArgs::parse's existence check). Distinct from ConfigValidationError,
// which is about the *contents* of a file that opened fine.
class ConfigFormatError : public LottoPickerError {
public:
    explicit ConfigFormatError(const std::string &message) : LottoPickerError(message) {}
};

// A config file opened and read fine, but its contents fail UI-002's
// validation: a required key (`data_file`, `top_n`) is missing, or
// `top_n` is not a positive integer. `message` names the offending key
// per docs/RTVM.md's TP-UI-002.
class ConfigValidationError : public LottoPickerError {
public:
    explicit ConfigValidationError(const std::string &message) : LottoPickerError(message) {}
};

// CORE-204/DATA-OUT-301: `data_file` can't be opened/read while
// computing the source-data hash (ModelStore::computeSourceHash), or a
// model artifact can't be written to disk (ModelSerializer::write).
// Distinct from CsvFormatError, which is specifically CsvIngestor's
// whole-file structural CSV checks -- this can fail even before
// ingestion runs (hashing happens first) or entirely outside it
// (serialization I/O). A malformed/corrupt *existing* artifact is
// deliberately NOT surfaced via this type at the ModelStore level --
// ModelSerializer::tryRead() swallows that internally and ModelStore
// treats it the same as "no model yet" (rebuild), since CORE-204 can
// always regenerate the artifact from the source CSV.
class ModelStoreError : public LottoPickerError {
public:
    explicit ModelStoreError(const std::string &message) : LottoPickerError(message) {}
};

// CORE-205: the requested `--backtest` (UI-003) sample date can't
// actually be backtested against the supplied history -- either no
// historical draw is dated exactly on it (nothing to hold out and
// compare the ranked list against), or no draw is dated strictly
// before it (no training data to build a truncated model from).
// Distinct from CliUsageError, which only validates the `--backtest`
// flag's date-list *syntax* (UI-003) -- this is a semantic problem
// with how the date relates to the actual history, only knowable once
// BacktestEngine::run looks at real history.
class BacktestError : public LottoPickerError {
public:
    explicit BacktestError(const std::string &message) : LottoPickerError(message) {}
};

} // namespace lottopicker
