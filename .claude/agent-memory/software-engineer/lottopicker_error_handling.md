---
name: lottopicker-error-handling
description: Typed-exception pattern established at UI-001 (issue #6) that later validation-error RTVM items (UI-002, DATA-IN-100, ...) should extend, not re-derive
metadata:
  type: project
---

`docs/SDD.md`'s Coding Standards ("Error handling") calls for a small
set of typed exceptions caught once at `main()`'s boundary. UI-001
(issue #6, branch `issue-6`) established the concrete shape in
`include/lottopicker/Errors.h`:

- `LottoPickerError : std::runtime_error` — base class; `main()` only
  ever catches this one type and prints `e.what()` to stderr +
  `EXIT_FAILURE`.
- `CliUsageError` / `CliConfigNotFoundError` — UI-001's two cases
  (malformed invocation vs. a syntactically-valid path that doesn't
  exist).

**How to apply:** later validation-error RTVM items (UI-002 config-key
validation, DATA-IN-100 malformed CSV rows) should add their own
`LottoPickerError` subclass in `Errors.h` rather than inventing a
parallel mechanism — `main()`'s single catch block already handles any
of them uniformly. Don't move error formatting into the throwing code;
the thrown message *is* the stderr text verbatim (see `CliArgs.cpp`).

Also: `CliArgs::parse` deliberately only handles the required
positional `<config-path>` for now. The optional `--backtest <dates>`
flag is UI-003's scope (see `docs/IMPLEMENTATION_PLAN.md`) — extend
this same parser when that issue lands rather than redesigning it.

**Clang-format gotcha:** the checked-in `.clang-format` (LLVM base)
right-aligns pointers/references (`char *const`, `const T &x`), not
the left-aligned style (`char* const`, `const T& x`) that reads more
naturally when typing. Always run `clang-format -i` on new/changed
files before committing — hand-typed code fails `--dry-run -Werror`
on pointer/reference spacing every time. Note: the Generate Code
Base-era `include/lottopicker/Version.h` itself is not clang-format
clean (left-aligned `const char* kVersion`) — pre-existing, left alone
as out-of-scope for a feature issue; worth a dedicated cleanup issue
if it ever matters.
