# Product Manager — memory

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

## Understanding of the product

- [Client context](client_context.md) — Kyle/HoloSim, C++ CLI Florida Lotto statistics tool, cheap budget, no user docs needed.

## Client / stakeholder context

- See [client_context.md](client_context.md).

## Open questions log

- [Scope decisions & open questions](scope_decisions.md) — 5 open questions sent to client on issue #1 (2026-09-05): game scope, data source, historical rule changes, output size, target OS.

## Decisions made

- 2026-09-05: MVP = Florida Lotto only, single composite algorithm (client's described approach, not comparative research), backtest is a first-class feature. See [scope_decisions.md](scope_decisions.md).
