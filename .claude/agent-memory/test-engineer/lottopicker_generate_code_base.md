---
name: lottopicker-generate-code-base
description: How to verify LottoPicker's CMake/Catch2 scaffold and how the Generate Code Base issue (#5) hands off despite having no RTVM item.
metadata:
  type: project
---

LottoPicker (HoloSim-Interactive/LottoPicker) is a C++17 CLI, built with
CMake + Ninja on Linux via presets, Catch2 v3 via FetchContent.

**Verification command** (issue #5, 2026-09-05, passed):
```
cmake --preset default && cmake --build --preset default && ctest --preset default
./build/default/src/lottopicker   # prints "lottopicker 0.1.0", exit 0
```
Runner already has cmake 3.31, ninja, gcc/g++ preinstalled — no setup needed.
Full clean build (incl. fetching/building Catch2) took well under the
default 2-minute Bash timeout.

**No-RTVM-item issues still route through the normal pass path.** Generate
Code Base (and the other "first five" doc-producing issue types) don't
trace to an `docs/RTVM.md` row, but Test Engineer's standard hand-off
(`status:ready-for-rtvm-update` → `agent:systems-engineer`) still applies —
Systems Engineer is the one who decides what "no RTVM update needed"
means for that issue type (e.g. closing it directly to release the
already-created, `status:on-hold` downstream RTVM issues via
`dependency-check.yml`), not Test Engineer's call to skip.

**Regression pass on trunk (2026-09-05):** After CI/CD's fast-forward
merge to `main` (no separate merge commit — main just advanced to the
feature commits), same verification command passed cleanly from a
clean `main` checkout: 113/113 targets, 1/1 smoke test, `lottopicker`
runs. Worth checking `git log --oneline -1` on `main` matches the
commits cited in the merge comment before assuming a merge commit
exists to diff against.

**Known deferred item, not a test gate:** `docs/ci/windows-verification.yml`
exists but `.github/workflows/windows-verification.yml` does not yet — a
human needs to copy it once (GitHub App can't grant itself `workflows`
permission). Deferred to DELIV-901 per Software Engineer's comment on #5;
don't treat its absence as a failure on feature issues before then.

**Post-merge regression fast path (issue #8, 2026-09-05):** when CI/CD or
Systems Engineer hands back `status:ready-for-test` purely for
"regression testing needed" after a trunk merge (no new code, RTVM
already updated with the commit SHA recorded), just pull `main`, confirm
the RTVM row and SHA match what was reported, and rerun the standard
build/test command on trunk. On pass, hand off the normal way
(`status:ready-for-rtvm-update` → `agent:systems-engineer`) even though
RTVM is already current — Systems Engineer's fast path (per
AGENT_LABELS.md) is to no-op through to CI/CD when nothing needs
changing, not for Test Engineer to skip straight to CI/CD.

**Inspection-type RTVM items (e.g. CORE-207, issue #8, 2026-09-05) need no
build/runtime verification at all** — the TP-CORE-xxx test procedure is a
checklist against the delivered doc's own content (source count, per-source
applicability statements, named recommendation), not a command to run.
Read the doc, check each numbered criterion in `docs/RTVM.md`'s TP-CORE-xxx
literally, and hand off the same way (`status:ready-for-rtvm-update` →
`agent:systems-engineer`) as any code pass.

**UI-001 pass (issue #6, 2026-09-05):** TP-UI-001's step 3 says "proceeds
to ranking without error" but ranking (UI-002/CORE) isn't built yet — this
is expected per the Implementation Plan's ordering, not a defect. Judge
step 3 as satisfied if the tool exits 0 with no error on a well-formed
config path, even though it doesn't yet actually rank anything. Re-check
this note once UI-002/CORE land — at that point "proceeds to ranking"
should mean real ranking output, and the looser bar no longer applies.

**CORE-201 pass (issue #12, 2026-09-05):** first CORE-2xx algorithm item to
land (CooccurrenceScorer). Standard build/ctest command sufficed — no new
harness needed. `tests/CooccurrenceScorerTest.cpp` maps 1:1 onto
TP-CORE-201's two parts (pair-frequency ordering, extended to 3-/4-number
groups); worth reading the test file directly against the TP text rather
than trusting the pass count alone, since Catch2 tags (`[CORE-201]`) don't
by themselves prove fixture values match the RTVM's literal example.

**UI-002 pass (issue #9, 2026-09-05):** confirms the earlier UI-001 note —
"data_file matches the fixture values exactly" in TP-UI-002 part 3 means the
*resolved* absolute path (config's own directory + the fixture's relative
value), not a literal string match against what's written in the config
file. `ConfigTest.cpp` already asserts this (`config.dataFile ==
path.parent_path() / "fixture_5draws.csv"`), and I re-confirmed it at the
CLI level by invoking the binary from a cwd different from the config's
directory — the printed `data_file` came back as the correct absolute path,
not the literal relative string. If a future TP says a field must "match
the fixture value exactly" for a path-typed key, check whether resolution
is part of that field's documented contract (SDD "Interfaces & File
Formats") before treating a resolved-vs-literal mismatch as a defect.

**UI-001 regression pass, same issue #6, second hand-off (2026-09-05):**
after CI/CD merged issue-6's 85b764f into `main` as 8fd22b2 (real merge
commit this time, not fast-forward — trunk had moved from issue-7's
DATA-IN-100 merge in between) and Systems Engineer recorded the SHA +
Verified status in RTVM, CI/CD routed back to Test Engineer for
regression. Confirms the fast-path noted above generalizes: pull `main`,
confirm RTVM SHA present, rerun full build/ctest (117/117, 10/10 —
includes issue-7's CsvIngestor suite), spot-recheck the specific TP steps
directly against the trunk binary. Same hand-off shape even on a second
pass through the same issue number.

**CORE-201 regression pass on trunk, second hand-off (issue #12, 2026-09-05):**
after CI/CD merged `issue-12`'s ed954a6 into `main`, SE recorded the SHA and
CI/CD routed back for regression testing per the fast path above. Pulled
`main` (b249751, fast-forwarded from ed954a6 through CORE-200's merge/RTVM
commits too), reran the standard build/ctest command: 23/23 pass (up from
17 — CORE-200's DecayScorer suite had landed on trunk in between). `git
status` clean. Confirms the fast path generalizes even when unrelated work
(CORE-200) merged to trunk between the original pass and the regression
hand-off — just verify the full current suite, not only the item under
regression.

**CORE-200 DecayScorer pass (issue #11, 2026-09-05):** clean `issue-11`
checkout, same build command, 16/16 ctest pass (10 pre-existing +
6 new `DecayScorerTest.cpp`). TP-CORE-200's two fixture cases map to
named tests directly: "favors a number seen only in the most recent
draw" (`score(7) > score(12)`) and "floors an unseen number's score at
zero" (`score(40) == 0.0`) — when a test file names its cases after the
TP language like this, citing the test name in the pass comment is
enough; no need to hand-verify the arithmetic by hand unless the SDD
formula itself is in question. Also checked `clang-format --dry-run
-Werror` on the three new/changed files as SWE claimed — clean.

**CORE-206 PoolSizeNormalizer pass (issue #14, 2026-09-05):** first CORE-2xx
item that reuses two prior scorers' raw scores algebraically rather than
recomputing (baseline-subtraction shortcut documented in the header) —
worth reading the header's derivation, not just running tests, since a
subtly wrong algebraic shortcut could still pass tests built against the
same (wrong) reasoning. Standard build/ctest sufficed: 42/42 (34 prior + 8
new `PoolSizeNormalizerTest.cpp`), TP-CORE-206's 3 parts map to named
tests directly. Also confirms the `kDefaultHalfLifeDraws` dedup (moved
solely into `DecayScorer.h`, `CooccurrenceScorer.h` now includes it) —
a housekeeping fix bundled into a feature commit, no RTVM impact, worth
spot-checking for behavior change but not flagging as scope creep.

**CORE-200 regression pass on trunk, same issue #11, second hand-off
(2026-09-05):** after CI/CD merged `issue-11` into `main` at 5e7f0fc
(three reset-and-redo cycles against concurrent #10/#12 activity,
resolved mechanically) and Systems Engineer recorded the SHA + Verified
status in RTVM, routed back for regression. Fast-path confirmed again:
`git checkout main && git pull`, RTVM row's SHA matched, clean rebuild
(`rm -rf build`), 23/23 ctest pass (16 prior + CORE-201's additions all
present together). No new harness needed.

**UI-002 regression pass on trunk, same issue #9, second hand-off
(2026-09-05):** after CI/CD's 7-reset-cycle merge of `issue-9` into
`main` at 302c87a and Systems Engineer recorded the SHA + Verified
status in RTVM, routed back for regression. `main` @ c44ed31, clean
rebuild, 35/35 ctest pass (up from 19 — EraTagger/EraTable and other
concurrent-issue suites now on trunk too). Re-verified TP-UI-002's
three cases directly against the trunk binary again. Fast path holds
even after a merge with many reset-and-redo cycles; nothing
issue-9-specific broke.
