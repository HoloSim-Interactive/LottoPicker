---
name: test-engineer-format-check-scope-gap
description: Test Engineer's clang-format re-check after a fix-and-refail can miss other files the same commit touched, if the check only re-scopes to the files named in the original FAIL comment — always run clang-format over the branch's full changed-file diff before merging, not just what the last comment listed.
metadata:
  type: project
---

On issue #20 (CORE-205), the original commit (`d19b260`) modified
`src/ModelStore.cpp` (adding a `ModelStore::` qualifier at a call site
when `buildArtifact` was made `public`) in addition to the two files
Test Engineer's FAIL comment named (`src/BacktestEngine.cpp`,
`tests/BacktestEngineTest.cpp`). That qualifier pushed one line to 103
columns (limit 100), a genuine `clang-format-18 --dry-run -Werror`
violation — but Test Engineer's fix-confirmation re-check (and
Software Engineer's `-i` fix) both only touched the two originally-named
files plus the header, so it went uncaught through PASS, RTVM update,
and into CI/CD's hands.

**Why it matters:** once Test Engineer's FAIL identifies a specific
file list, everyone downstream (Software Engineer's fix, Test
Engineer's re-check) tends to re-scope to exactly that list rather
than the branch's full diff — a correct fix to the named files can
still leave the branch non-compliant elsewhere.

**How to apply:** before merging, run
`clang-format-18 --dry-run -Werror` over every `.cpp`/`.h` file the
branch's full `git diff main...issue-N --stat` touches, not just the
files named in the most recent Test Engineer comment. If it finds a
genuine violation (confirm real column-limit/style breakage, not the
`.clang-format`-discovery false-negative from copying a file outside
the repo tree — see [[shallow_clone_merge_base]] for the same
verify-before-trusting posture), a whitespace-only `-i` fix with a
full rebuild+retest to confirm no regression is within CI/CD's own
merge-preparation scope (same class of fix as the mechanical
`src/CMakeLists.txt` conflict resolution already expected here) —
document it plainly in the merge commit message rather than silently
folding it in.
