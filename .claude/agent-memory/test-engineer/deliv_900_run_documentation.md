---
name: deliv-900-run-documentation
description: How TP-DELIV-900 (README run documentation) was verified — clean-clone build/run reproduction, and that clang-format doesn't apply to .cfg fixture files.
metadata:
  type: project
---

DELIV-900 (issue #25, 2026-09-05, passed): docs/fixtures-only change
(root `README.md`, `examples/README.md`, `examples/sample_config.cfg`,
`examples/sample_history.csv`, `.gitignore`) — no `.cpp`/`.h` diff, so
[[core_203_ranking_engine]]'s clang-format check doesn't apply here.

- Ran TP-DELIV-900 literally: clean `rm -rf build` + `cmake --preset
  default && cmake --build --preset default && ctest --preset default`
  (88/88), then both documented commands exactly as written in the
  README against the committed `examples/` fixtures.
- Both console captures matched the README's pasted "Sample output"
  byte-for-byte (rank table and `--backtest 2024-03-13` report,
  including the `not found in top-N` / `4 1 0 0` containment row).
- Reused the ~1min-per-run timing budget from
  [[core_203_ranking_engine]] via `run_in_background` + a poll loop —
  don't mistake either command for a hang.
- Gotcha: `clang-format --dry-run` on a `.cfg` file produces a wall of
  spurious violations — it's treating plain text as C++. Not a real
  gate; only check clang-format when `.cpp`/`.h` files actually
  changed (confirmed via `git show --stat` on the issue's commits).
- Cleaned up the `examples/sample_history.csv.model` artifact each run
  produces (correctly `.gitignore`d, confirmed `git status` clean
  after removal).
