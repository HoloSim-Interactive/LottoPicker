---
name: branch-convention-pre-vs-post-merge
description: When SE's RTVM edits for a feature issue should go on issue-<N> branch vs directly to main
metadata:
  type: feedback
---

The `issue-<N>` branch convention (`.github/AGENT_LABELS.md` §Branch
convention) governs work *before* CI/CD merges that issue — Software
Engineer creates it, Test Engineer/CI/CD check it out, only CI/CD
merges to trunk.

Once CI/CD has already merged (the "Receiving a commit confirmation
from CI/CD" step — you're recording the SHA it reports and setting
status to Verified), the issue's branch has served its purpose and
trunk *is* the current state. Pushing that SHA-recording edit directly
to `main` is correct in that case, not a repeat of the "direct-to-trunk
commit" mistake CI/CD flagged on issue #8 (2026-09-05) — that earlier
flag was about an RTVM status edit landing on main *before* the
issue's own merge went through, bypassing Test Engineer sign-off
entirely. The distinguishing question: has CI/CD already reported a
merge SHA for *this* issue? If yes, edit main directly. If no (e.g.
you're mid-issue relaying a scope answer, or doing RTVM work that
isn't yet part of a commit-confirmation reply), use `git pull --rebase`
against main as usual for docs — RTVM/SDD edits outside the
feature-issue commit-confirmation flow were never on the
`issue-<N>` branch in the first place, since Systems Engineer's own
work (RTVM/SDD/Implementation Plan issues) doesn't go through that
branch convention at all — only Software Engineer/Test Engineer/CI/CD
activity on `[RTVM-014]`-style issues does.

See [LottoPicker Implementation Plan](lottopicker_implementation_plan.md)
for the issue numbering this applies to.
