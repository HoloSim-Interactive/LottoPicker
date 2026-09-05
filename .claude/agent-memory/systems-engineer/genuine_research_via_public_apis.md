---
name: genuine-research-via-public-apis
description: How to actually do "genuine comparative research" RTVM/SDD tasks (e.g. CORE-207-style) with curl when network access is available — real sources, not invented ones.
metadata:
  type: feedback
---

When a stakeholder need explicitly demands real comparative/literature
research (not a restatement of the client's own description dressed up
as research), check network access first (`curl -m5 https://...`) —
it was available in this sandbox and using it produced a materially
better, verifiable research artifact than reasoning from training data
alone would have.

**Why:** the client on LottoPicker (SN-5) called real research "the
whole point of the exercise" and explicitly rejected a PM proposal to
skip it. Citing sources that can't be checked (or worse, that don't
exist) would fail that need silently — nobody catches a fabricated
citation in an Inspection-verified deliverable until the client does.

**How to apply — concrete, reusable calls:**
- `api.crossref.org/works?query=...` — confirms a real peer-reviewed
  paper exists and gets its exact title/journal/year without needing
  a paywalled database. Use this before citing anything as "a
  published paper."
- `api.github.com/search/repositories?q=...&sort=stars` — finds real
  open-source projects attempting the same problem; fetch their raw
  README (`raw.githubusercontent.com/<owner>/<repo>/main/README.md`,
  fall back to `/master/`) to read their actual approach rather than
  guessing from the description.
- `en.wikipedia.org/api/rest_v1/page/summary/<Title>` — quick, reliable
  summaries for background/formalism (e.g. combinatorics, established
  techniques) without scraping full HTML.
- `html.duckduckgo.com/html/?q=...` works sometimes for general web
  search but is unreliable (empty results half the time in this
  environment) — treat it as a bonus, not a dependency; official sites
  built as JS SPAs (e.g. floridalottery.com) return no scrapable
  content via curl at all — don't spend more than one attempt on those,
  document the gap honestly instead (see [[lottopicker_sdd_decisions]]
  for how that was handled) rather than presenting an unconfirmed guess
  as fact.
- For every source cited in the deliverable, state explicitly whether/
  why it does or doesn't apply — a source that only justifies a
  tangential point (e.g. a paper about human behavior, not the actual
  random process) is still worth citing, but say so plainly rather than
  implying stronger support than it gives.
