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
- [Direct-trunk commit recurred on issue #7](direct_trunk_commit_recurred_issue7.md) — recurred again on issue #12 (4th time); keep flagging each occurrence even though harmless so far.
- [Post-merge SHA recording direct-to-trunk is OK](post_merge_sha_recording_direct_to_trunk_ok.md) — 3rd direct-trunk edit on issue #8 was SE correctly recording an *already-reported* merge SHA, not a repeat violation; only flag if the SHA predates CI/CD's report or the edit does more than that.
- [Merge commit message inconsistency](merge_commit_message_inconsistency.md) — most trunk merges (incl. mine, issue #13/e020769) skip the required Summary/Source/Testing body; write it into the merge `-m` up front, since a bare title can't be fixed after push without a forbidden force-push.
