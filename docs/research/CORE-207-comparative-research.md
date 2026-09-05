# CORE-207 — Comparative Research Summary

Verifies: **CORE-207** (Inspection, TP-CORE-207). Feeds the composite
ranking formula (CORE-202) and the pool-size normalization method
(CORE-206), both finalized in `docs/SDD.md` §Architecture → Algorithm
Design.

Conducted 2026-09-05 by Systems Engineer, via live web/API lookups
(Crossref, Wikipedia, GitHub code search) during the SDD issue, per
the client's explicit ask (SN-5) that this be real research, not a
restatement of the client's own Kickoff Runbook description.

## Sources consulted

### 1. Suetens, Galbo-Jørgensen & Tyran (2016), *"Predicting Lotto
Numbers: A Natural Experiment on the Gambler's Fallacy and the
Hot-Hand Fallacy,"* Journal of the European Economic Association
14(3), 584–607. (Confirmed via Crossref DOI lookup — a real,
peer-reviewed publication.)

**What it studies:** Danish lottery bettors' number-choice behavior
after a number is or isn't drawn. Because prize money is split among
everyone who matches, and the researchers can observe how many people
bet on each number, they can measure whether bettors chase numbers
that were just drawn ("hot hand") or avoid them ("gambler's fallacy")
— and show both effects are real, in bettors.

**Applicability:** Does **not** apply to the drawing mechanism itself
— it is a study of human perception of pattern in a mechanically
random process, not evidence that the draw process carries memory.
It applies to this project as a caution: the client's SN-1 premise
("sequential-repeat probability... declines the more recently a
number was drawn") is *structurally* the same hypothesis this paper's
subjects act on and the paper is agnostic on whether it's true of the
draw mechanism — it only shows humans believe it. This project must
not take SN-1 as self-evidently true; it must be validated against
history via the backtest engine (CORE-205), which is exactly what
Source 2 below models methodologically.

### 2. `IoakeimKyrgiafinis/LotteryDataAnalysis` (GitHub, open-source
repo). *"Statistical analysis of 27 years of Greek Joker lottery
draws (1997–2024), testing whether the game operates as a random
process."*

**What it does:** 13 independent statistical tests (χ² goodness-of-fit
on number frequency, χ² on pairwise co-occurrence across all 990
number pairs, Ljung-Box serial-autocorrelation test, rolling-window χ²
for temporal drift, etc.) against a structurally similar 6-number
lottery. Result: no statistically significant deviation from IID
uniform randomness survives correction for multiple comparisons.

**Applicability:** Directly applies, and is the single most actionable
source. It supplies the exact statistical toolkit for treating
"does this model beat chance" as a falsifiable question rather than an
assumption — precisely the yardstick the Mission Statement commits to
("measurable skew... using historical regression testing... rather
than anecdote"). Recommendation: CORE-205's backtest report should
compare observed containment counts against the **chance-expected**
containment (computed from the hypergeometric distribution, Source 3),
not just report raw counts — an honest result may well be "consistent
with chance," and the tool should be capable of saying so rather than
presenting any nonzero containment as a win.

### 3. Wikipedia, *"Lottery mathematics."* Combinatorial/hypergeometric
framework for probability of a k-of-n match in an n-choose-6 draw.

**Applicability:** Directly applies to three separate items: (a)
CORE-205/DATA-OUT-302's partial-match percentile needs a chance
baseline to be meaningful (Source 2's recommendation); (b) CORE-202's
composite formula needs to weight an observed group's occurrence count
against that group's size-dependent base rate, not raw count alone,
since larger groups are combinatorially rarer by chance regardless of
any real skew; (c) CORE-206's pool-size normalization needs exactly
this machinery to compare an "observed count" from a 6/49-era draw
against a "chance-expected count" for a 6/49 pool, on the same
probability scale as a 6/53-era observation — see
`docs/SDD.md`'s CORE-206 method for the full derivation.

### 4. `ilyas7010/lottery-ml-analyzer` (GitHub, open-source repo).
Weighted-ranking model over `historical_frequency` and
`recent_frequency`.

**Applicability:** Applies as a *baseline-familiarity* check, not a
statistical validation — it is functionally the same frequency +
recency-weight approach as the client's Kickoff Runbook description,
built independently by an unrelated hobbyist, which corroborates that
this is a common, easily-implemented starting hypothesis. Its own
README carries the same disclaimer this project's Mission Statement
does ("not a guaranteed prediction tool"). It does **not** apply as
methodological validation — it has no backtest, no comparison against
a chance baseline, and does not address multi-number co-occurrence
groups or pool-size changes at all. Recommendation: it's reasonable
evidence the *shape* of CORE-200's per-number decay score is a
sensible starting point; it is not evidence that shape has predictive
value, which only this project's own CORE-205 backtest can establish.

### 5. Wikipedia, *"Lottery wheeling."* Published combinatorial
strategy for selecting number groups to guarantee minimum-match
coverage across multiple tickets.

**Applicability:** Applies conceptually to CORE-201/CORE-202's
justification for scoring 2–6 number *groups*, not just individual
numbers — wheeling systems are a real, decades-old published technique
built entirely around the idea that group structure (not just
per-number frequency) determines partial-match outcomes, which is
exactly what SN-2's 3/4/5/6 partial-match framing cares about. It does
not itself provide a scoring function (wheeling optimizes ticket
*coverage*, not recurrence-likelihood *ranking*) — CORE-202's composite
formula borrows the "groups matter" framing, not a wheeling algorithm
directly.

## Recommendation (feeds CORE-202 and CORE-206)

1. **The composite ranking metric (CORE-202) is a documented, tunable,
   linear combination** of a per-number decay score and per-group
   (2–6) co-occurrence scores — not a more elaborate model. Two
   independent reasons: (a) Source 4 shows this is the standard,
   well-understood baseline for this exact problem shape; (b) with
   roughly 2,000–4,000 historical Florida Lotto draws total against a
   22.9M-combination space, there is not enough data to fit anything
   more expressive than low-order frequency statistics without
   overfitting noise — a genuinely more complex model would be fitting
   Source 1's "hot hand" illusion, not signal.
2. **Every raw historical count — per-number and per-group — is
   expressed as observed-vs-chance-expected, not a raw tally**, using
   the hypergeometric baseline from Source 3. This is what makes group
   scores of different sizes comparable (a 6-number group repeating
   twice is far more surprising than a pair repeating twice) and is
   the same normalization CORE-206 needs to compare across pool-size
   eras. See `docs/SDD.md` for the exact formula.
3. **CORE-205's backtest report states the chance-expected containment
   alongside the observed containment** (Source 2's methodology), so
   the client gets an honest "beats chance by X" or "statistically
   indistinguishable from chance" result rather than an implied win
   from any nonzero match count.
4. Default composite weights (per-number vs. each group size) are a
   documented starting point in `docs/SDD.md`, explicitly expected to
   be tuned using CORE-205's own backtest as the empirical validation
   loop, not treated as fixed constants decided here.
