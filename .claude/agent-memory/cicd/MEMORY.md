# CI/CD — memory

**This file is an index, not a store.** It is loaded on every run you
ever do, so anything verbose here is re-read on every future hand-off
for the rest of the project. Keep each entry to one line: a link and a
one-sentence summary. Put the actual detail in its own file in this
folder.

    - [Short title](descriptive_slug.md) — one sentence on what it is.

A genuinely one-line fact can stay a plain line with no file of its
own. Split a lesson out when it needs a reproduction, a command
sequence, or real reasoning to be useful later. See "Memory structure"
in `.github/AGENT_LABELS.md`.

## Branching conventions

## Build & toolchain notes

- [Shallow-clone merge-base gotcha](shallow_clone_merge_base.md) — `git merge-base`/`rev-list` on `origin/main` lie if the checkout is shallow; `git fetch --unshallow` before trusting them.

## Release & versioning

- LottoPicker's `VERSION` file didn't exist before issue #5's merge (Generate Code Base, the project's first trunk merge) — created as `1.0` per the standing rule, noted in that commit.
- LottoPicker: no RTVM item reaches Verified until individual `[RTVM-xxx]` feature issues land — Generate Code Base merge (v1.0.28) only tagged, no release cut.

## Known issues

- [Concurrent trunk-merge race](concurrent_trunk_merge_race.md) — multiple issues merging to `main` around the same time causes repeated push rejections; reset-and-redo, expect a mechanical `src/CMakeLists.txt` source-list conflict. Confirmed a 3rd time on issue #11.
- [Next line must match labels applied](next_line_must_match_labels_applied.md) — after a trunk merge, hand back to SE with `status:ready-for-rtvm-update`, not the `status:ready-for-commit` I inherited.

- [Inspection-item direct-trunk commit](inspection_item_direct_trunk_commit.md) — on CORE-207/issue #8, Systems Engineer committed the RTVM status change straight to main instead of via CI/CD; check both directions of the diff before assuming the branch has the work.
- [Direct-trunk commit recurred on issue #7](direct_trunk_commit_recurred_issue7.md) — recurred on issue #12 (4th) and issue #19 (5th); always auto-resolves clean if branch doesn't itself touch RTVM.md.
- [Post-merge SHA recording direct-to-trunk is OK](post_merge_sha_recording_direct_to_trunk_ok.md) — SE recording an already-known SHA (or a no-branch-at-all Inspection item, e.g. issue #24) direct-to-trunk isn't a violation; verify nothing's pending, hand off with no status label, `Next: agent:systems-engineer`.
- [Merge commit message inconsistency](merge_commit_message_inconsistency.md) — most trunk merges (incl. mine, issue #13/e020769) skip the required Summary/Source/Testing body; write it into the merge `-m` up front, since a bare title can't be fixed after push without a forbidden force-push.
- [Post-regression no-new-commit](post_regression_no_new_commit.md) — TE's regression pass can land back on `agent:cicd`/`status:ready-for-commit` with nothing new to merge (issue #13); confirm no diff vs main, then hand back with no new status label.
- [Test Engineer format-check scope gap](test_engineer_format_check_scope_gap.md) — a fix-and-recheck cycle scoped to Test Engineer's named files can miss a real clang-format violation elsewhere in the same commit's diff (issue #20); always check the branch's full diff before merging.
