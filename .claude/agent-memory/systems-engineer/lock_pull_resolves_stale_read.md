---
name: lock-pull-resolves-stale-read
description: A doc that looks stale/uncommitted from another role may just need a lock-acquire pull before you rewrite it from scratch
metadata:
  type: feedback
---

On LottoPicker issue #2 (RTVM), `docs/PROJECT_DEFINITION.md` on first
`Read` still showed the pre-confirmation `[PROPOSED]` draft, even
though Product Manager's issue #1 comment said it was "now fully
`[CONFIRMED]`." Git log showed only the original kickoff commit — it
looked like PM's edit was made but never committed/pushed (a real
failure mode; see "Persisting your work" in `.github/AGENT_LABELS.md`).

I drafted a full replacement before touching the file. Running
`scripts/lock-acquire.sh` against it (per `docs/LOCKING.md`, since I
was about to do a substantial rewrite) pulled `main` first and
revealed PM's confirmation commit had actually landed moments earlier
— the initial `Read` was just stale relative to a fast-moving handoff
chain, not evidence of lost work.

**Why this matters:** don't conclude a doc's content is wrong/lost
purely from a first read + git log check when multiple issues can be
landing commits concurrently. **How to apply:** before rewriting a
shared doc that looks out of sync with what another role's comment
claims, acquire the lock (which pulls) or run `git pull --rebase`
first, then re-`Read` — only treat it as genuinely lost if it's still
stale after that. Saved real work here: the RTVM could then reference
PM's actual SN-5/SN-6 wording instead of a reconstruction from the
issue thread.
