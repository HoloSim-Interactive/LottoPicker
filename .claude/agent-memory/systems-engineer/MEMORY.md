# Systems Engineer — memory

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

## RTVM conventions

- [LottoPicker RTVM ID scheme](rtvm_id_scheme_lottopicker.md) — category prefixes/ranges and the 19 items populated 2026-09-05; console-only MVP output is deliberate, not an oversight.

## Cross-product interface standards

## Requirements patterns and traps

- [Lock-acquire pull resolves a stale-looking doc](lock_pull_resolves_stale_read.md) — before rewriting a shared doc that looks out of sync with another role's claim, pull (lock-acquire does this) and re-read before assuming the edit was lost.

## Documentation index
