# examples/

A small, self-contained, working example (`DELIV-900`) that the root
`README.md` walks through end to end:

- `sample_history.csv` — 15 synthetic draws, header
  `date,n1,n2,n3,n4,n5,n6`, all dated 2024 (current 6/53 era), per
  `docs/SDD.md`'s `DATA-IN-100` format. Illustrative only, not real
  Florida Lotto history (no real client-supplied CSV exists yet — see
  `docs/RTVM.md`'s `DATA-IN-101`/`DELIV-900` notes).
- `sample_config.cfg` — a matching `UI-002` config pointing
  `data_file` at `sample_history.csv` (resolved relative to this
  directory, not the caller's working directory) with `top_n=10`.

Run it from the repo root exactly as documented in the root
`README.md`'s "Launch command" section. A run against this data
produces a `sample_history.csv.model` file here (`DATA-OUT-301`,
`CORE-204`'s persisted model artifact) — that file is derived, regenerated
on demand, and intentionally `.gitignore`d (`*.model`), not committed.
