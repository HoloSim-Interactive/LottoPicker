---
name: concurrent-trunk-merge-race
description: When multiple issues' CI/CD runs merge to main around the same time, expect repeated push rejections and real conflicts in src/CMakeLists.txt and memory files — reset to origin/main and redo the merge each time, don't force-push.
metadata:
  type: project
---

On issue #6 (UI-001) merging to `main` on 2026-09-05, `origin/main`
advanced 4 separate times between my first `git merge --no-ff` and a
successful `git push` — issue #7's own CI/CD run was merging
DATA-IN-100 concurrently, plus several Systems Engineer/Test Engineer
memory and RTVM commits landed in between. Every `git push origin
main` got `[rejected] (fetch first)`.

**Why it matters:** LottoPicker has multiple issue branches with
independent agent pipelines that can all reach "ready to merge" close
together. This isn't a one-off — expect it whenever several feature
issues are in flight at once.

**How to apply:**
1. On push rejection, `git fetch origin main` and check `git log
   --oneline origin/main` for what landed. If it's just memory/RTVM
   doc commits (no source changes), `git reset --hard origin/main`
   and redo the exact same `git merge --no-ff issue-N` — conflicts
   will very likely repeat identically (same memory-file entries),
   so the resolution is fast the second/third time.
2. If another issue's real feature merge landed
   (e.g. issue-7's DATA-IN-100), expect a genuine conflict in
   `src/CMakeLists.txt`'s source-file list for `lottopicker_lib` —
   both branches add their own `.cpp` to the same `add_library(...)`
   call. Resolution is just: keep both filenames, combine the comment
   above the list. This is a mechanical, safe conflict — no need to
   read either issue's actual code changes to resolve it correctly.
3. Never force-push over a rejection — always reset to
   `origin/main` and redo the merge/build/test cycle. Full
   rebuild+retest after every redo caught nothing new here, but is
   the only way to be sure a conflict resolution didn't break the
   build.
4. Memory-file conflicts (this project's own `.claude/agent-memory/**`)
   between two issues' additions to the same file/section are always
   resolved by keeping both entries, never picking one — see
   [[shallow_clone_merge_base]] for the general "don't guess, verify"
   posture this shares.
