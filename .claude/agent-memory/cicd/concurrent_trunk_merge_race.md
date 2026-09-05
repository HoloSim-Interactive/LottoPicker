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

**Confirmed again on issue #11 (CORE-200) merging 2026-09-05**, three
resets in a row before the push succeeded: first two rejections were
doc-only commits from issue #10 (reset-and-redo, identical RTVM
conflict both times — same two adjacent rows, e.g. CORE-200/CORE-201,
each branch's own row stale relative to the other's already-merged
update), the third was issue #12's real CORE-201 merge landing
concurrently, which added the expected `src/CMakeLists.txt` conflict
(comment text only — the actual `add_library(...)` file list
auto-merged cleanly) *plus* a memory-file (`software-engineer/MEMORY.md`)
conflict, same keep-both resolution. Rebuilt full suite after every
redo; only the final one mattered (23/23, combining both features'
tests) since nothing was pushed until then.

**Worst case so far: issue #9 (UI-002) merging 2026-09-05 took 7
reset-and-redo cycles** against #10/#11/#12 all landing doc/memory
commits concurrently (never a new source merge after the first
CORE-200 one, but the doc/memory churn alone kept re-triggering
rejections). The conflicting hunks were byte-identical every single
cycle (same `src/CMakeLists.txt`/`tests/CMakeLists.txt` source-list
conflict, same two memory-file conflicts) once the source-side base
had stabilized — after the 2nd redo it was faster to keep a known-good
resolution (a heredoc `cat >` rewrite of the two CMakeLists files, a
small Python regex substitution for the two memory files) ready to
reapply than to re-derive it from the diff each time. Don't mistake a
long streak of doc-only rejections for something wrong with the merge
itself — keep resetting and redoing exactly the same way; it terminates
once the doc/memory-commit burst quiets down.

**Confirmed a 5th time on issue #10 (DATA-IN-101) merging 2026-09-05**:
first merge attempt (against a stale local `main` one commit behind
origin) hit the usual `src/CMakeLists.txt`/`tests/CMakeLists.txt`/
`software-engineer/MEMORY.md` conflicts; aborted and fast-forwarded
local `main` first this time rather than resolving against a base I
already knew was behind. Redid the merge — same three conflicts,
resolved keep-both/combine-list as usual, committed, but the push was
*still* rejected because issue #9 (UI-002, a real feature merge) had
landed in the gap between fetch and push. Reset to the new
`origin/main` and redid the merge a third time — same conflict set
plus UI-002's own `Config.cpp`/`ConfigTest.cpp` entries, resolved the
same mechanical way. Lesson: fast-forwarding local `main` before the
first merge attempt doesn't avoid the race, since the window that
matters is between merge-commit and push, not before it — just accept
that a redo-after-push-rejection is normal and don't try to pre-empt
it.

**Issue #16 (CORE-204) merging 2026-09-05: push succeeded on the first
try, no race** — worth noting as the counterexample: the race isn't
guaranteed, it just needs to be expected. Conflict set was the usual
`src/CMakeLists.txt` comment-text-only conflict (file list auto-merged)
plus, for the first time, *both* `software-engineer/MEMORY.md` and
`test-engineer/MEMORY.md` conflicting in the same merge (previously
only software-engineer's had come up alongside CMakeLists) — same
keep-both resolution, no new pattern needed.
