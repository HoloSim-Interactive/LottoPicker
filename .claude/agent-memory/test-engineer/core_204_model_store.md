---
name: core-204-model-store
description: How TP-CORE-204 (model persistence/reuse) was verified end-to-end at the CLI level, and what the persisted .model file looks like.
metadata:
  type: project
---

CORE-204 (issue #16, 2026-09-05, passed): `ModelStore::loadOrBuild` wired
into `main.cpp` for the first time — this is the first issue where the CLI
actually does something beyond parsing config. Standard build/ctest
(59/59) wasn't enough on its own since TP-CORE-204 requires an actual
"reused vs. rebuilt" behavior across two real runs, not just unit tests
of the hash function.

**Manual verification recipe** (no shipped fixture files exist in the
repo — build one ad hoc):
```
mkdir /tmp/x && cd /tmp/x
# fixture_5draws.csv: header + 5 well-formed draw rows
# fixture.cfg: data_file=fixture_5draws.csv / top_n=10
./lottopicker fixture.cfg   # -> "model: rebuilt (5 draw(s)) -> ...model"
./lottopicker fixture.cfg   # -> "model: reused (source unchanged) <- ...model"
echo "2020-01-20,2,4,6,8,10,12" >> fixture_5draws.csv
./lottopicker fixture.cfg   # -> "model: rebuilt (6 draw(s)) -> ...model", source_hash line changed
./lottopicker fixture.cfg   # -> reused again, confirms rebuild path re-settles
```
All 4 runs behaved as expected; `.model` file's `source_hash=` line
(grep it directly) changed exactly when the CSV changed, confirming the
hash check isn't a no-op. The persisted artifact's on-disk path is
`<data_file>.model` (SWE flagged this convention as unspecified by SDD/
RTVM — not a defect, just worth knowing if a future issue changes it).

Also re-confirmed `clang-format --dry-run -Werror` clean on all new/
changed files (ModelStore, Sha256, ModelArtifact, ModelSerializer,
main.cpp) — same check pattern as [[lottopicker_generate_code_base]]'s
CORE-200 note.
