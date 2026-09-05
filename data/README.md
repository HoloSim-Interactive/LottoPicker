# `data/` — Florida Lotto historical draw data

## Files

| File | Rows | Schema | Use |
| --- | --- | --- | --- |
| `Lotto-6.pdf` | — | — | Client-supplied source of record (Florida Lottery "Winning Numbers History", generated 05-SEP-2026, 40pp). |
| `florida_lotto_history.csv` | 3402 | `date,n1..n6` | **Canonical ingest file.** Main draws only, exactly the header `CsvIngestor` requires. |
| `florida_lotto_full.csv` | 4018 | `date,n1..n6,draw_type,pool_size,label_inferred` | Everything, including Double Play draws and era metadata. |

Both CSVs are chronological, oldest first. Extraction is reproducible:
`pdftotext -layout Lotto-6.pdf` then the parser described below.

## Draw-type codes in the PDF

Researched against the Florida Lottery's own material (see Sources):

| Code | Meaning | Affects the numbers? |
| --- | --- | --- |
| *(none)* | Main draw, pre-2009 (the PDF simply printed no label then). | — |
| `LOTTO` | Main draw. | — |
| `X2`–`X5` | **XTRA multiplier.** A value 2–5 drawn at random *before* each draw; multiplies non-jackpot prize amounts only. | **No.** It is a prize multiplier, not a drawn ball. Present 2009-10-14 … 2020-10-07. |
| `LOTTO DP` | **Double Play.** A *second, independent* drawing from the same 6/53 pool, held immediately after the main draw. | **Yes — it is a separate set of 6 drawn numbers.** Present 2020-10-10 … present. |

The X-codes carry no information about which numbers were drawn, so they
are dropped from the canonical CSV. Double Play draws *are* real draws and
are preserved in `florida_lotto_full.csv`.

## Rule eras — established from this data, not assumed

| Era | Dates | Pool | Schedule |
| --- | --- | --- | --- |
| Launch | 1988-05-07 … 1999-10-23 | **6/49** | Weekly, Saturday |
| Current | 1999-10-27 … 2026-09-02 | **6/53** | Twice weekly, Wed + Sat |

Evidence, all from the 4018 parsed records:

- 1988–1998 draws contain **every** number 1–49 and never a number above 49.
- From 1999-10-27 they contain **every** number 1–53.
- The earliest draw containing a number >49 is **1999-10-27**; the last draw
  before it is **1999-10-23**. So the boundary lies in `(10-23, 10-27]`, which
  agrees with the Florida Lottery's documented change date of **1999-10-24**.
- 1999-10-27 is also the first Wednesday draw: the matrix change and the move
  from weekly to twice-weekly happened together.

> **This corrects a flagged open item.** `include/lottopicker/EraTable.h`
> carries `{"1999-01-01", 53}` as an explicitly-labelled *working hypothesis*
> taken from a secondary blog, with a comment saying it must be confirmed
> against real data once a real CSV exists. It is wrong by ~10 months: every
> 1999 draw from Jan 1 to Oct 23 was 6/49, not 6/53. The correct boundary is
> `{"1999-10-24", 53}`.

## Parsing notes (why this was not a straight text dump)

The PDF's text layer does not read out in visual order. What the parser relies on:

1. **Two columns per page**, and the sequence runs *down the left column, then
   down the right column of the same page* — not down one long column across
   pages. Verified: with that ordering all 4018 records come out in strict
   reverse-chronological order with no breaks.
2. **Irregular spacing** around the separators (`18- 4- 17- 42- 14 - 43`), so
   numbers are matched with a tolerant `\s*-\s*` pattern rather than split.
3. **Records are segmented at date boundaries** within each line, so a
   left-column record cannot swallow the right-column record beside it.
4. **The last two records on each page carry no draw-type label at all** — a
   rendering artifact affecting exactly one same-date pair on each of the 12
   Double-Play-era pages (24 records). In all 604 pairs where both labels
   survived, the order is main draw first, Double Play second — 604/604, no
   exceptions — so that convention fills the gap. Those rows are marked
   `label_inferred=1` in `florida_lotto_full.csv`.
5. **Sorting must be stable.** Reversing the parsed list to get chronological
   order also flips the two records that share a date, which puts every Double
   Play draw *ahead of* the main draw it followed. A stable sort by date is
   required. (This was a real bug caught in validation, not a hypothetical.)

### Number order within a row

Early-era rows preserve **draw order** (`1988-05-07,30,44,17,49,42,15` — not
ascending); modern rows are printed ascending. Source order is preserved as-is.
Anything doing set-based analysis must sort per row rather than assume order.

## Validation

All checks pass on both files: 4018 records parsed with **0 unparsed lines**,
0 duplicate numbers within any draw, 0 numbers outside their era's pool,
strict date ordering, and 616 Double Play draws each paired to exactly one
main draw on the same date. Spot-checked against the raw PDF text.

## Sources

- [Florida Lottery — Draw Games FAQ](https://flalottery.com/enes/flalottery.com/lotto-faq)
- [Florida Lottery — FLORIDA LOTTO game page](https://floridalottery.com/games/draw-games/florida-lotto)
- [FLORIDA LOTTO with XTRA — FAQs (PDF)](https://files.floridalottery.com/exptkt/FLORIDALOTTOwithXTRAFAQs-Post-Launch.pdf)
- [FLORIDA LOTTO Fact Sheet (PDF)](https://files.floridalottery.com/exptkt/FLORIDALOTTOFactSheetUpdated.pdf)

Other Florida games publish the same PDF format if more data is ever wanted:
[Mega Millions](https://files.floridalottery.com/exptkt/mmil.pdf) ·
[Florida Lotto](https://files.floridalottery.com/exptkt/l6.pdf) ·
[Cash4Life](https://files.floridalottery.com/exptkt/c4l.pdf) ·
[Jackpot Triple Play](https://files.floridalottery.com/exptkt/jtp.pdf) ·
[Fantasy 5](https://files.floridalottery.com/exptkt/ff.pdf) ·
[Pick 5](https://files.floridalottery.com/exptkt/p5.pdf) ·
[Pick 4](https://files.floridalottery.com/exptkt/p4.pdf) ·
[Pick 3](https://files.floridalottery.com/exptkt/p3.pdf) ·
[Pick 2](https://files.floridalottery.com/exptkt/p2.pdf)
