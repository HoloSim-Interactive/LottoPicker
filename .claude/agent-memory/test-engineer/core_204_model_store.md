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

**Trunk regression re-check** (same issue #16, after CI/CD merge
86ca3ed / v1.0.145): re-ran the exact 4-run CLI recipe above directly
against trunk's build — same rebuilt/reused/rebuilt/reused sequence,
`source_hash=` changed only on the appended-row runs. 64/64 ctest
(59 + CORE-202's 5). Confirms the pattern from issue #15's CORE-202
regression pass: a trunk-merge regression request routes back to
`agent:systems-engineer` / `status:ready-for-rtvm-update`, not to
CI/CD directly — Systems Engineer records the regression pass and
re-releases to CI/CD from there.
