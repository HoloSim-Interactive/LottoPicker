#!/usr/bin/env python3
"""
Parse the Florida Lottery 'FLORIDA LOTTO' winning-numbers-history PDF
(pdftotext -layout output) into chronological CSV.

Layout facts this relies on (verified against Lotto-6.pdf, 40pp, 2026-09-05):
  * Two columns per page. Reading order is LEFT column top-to-bottom,
    THEN RIGHT column top-to-bottom -- the sequence continues between
    columns on the same page, not across pages.
  * Records are reverse-chronological overall.
  * Left-column records start at character offset 0; right-column
    records start at offset 47/50/57 (varies because -layout pads
    around the left record's trailing code, whose width differs).
  * Six numbers, dash-separated, with irregular spacing around the
    dashes ("18- 4- 17- 42- 14 - 43").
  * Trailing draw-type code is one of: absent, X2..X5, LOTTO, LOTTO DP.
"""
import re, sys, csv, collections

DATE = r'(\d{2})/(\d{2})/(\d{2})'
NUMS = r'\s*'.join([r'(\d{1,2})'] + [r'-\s*(\d{1,2})'] * 5)
CODE = r'\s*(LOTTO\s+DP|LOTTO|X[2-5])?'
REC = re.compile(DATE + r'\s+' + NUMS + CODE)
DATE_ANCHOR = re.compile(r'\d{2}/\d{2}/\d{2}')


def parse_year(yy):
    """Florida Lotto began 1988-04-30; no draws before that. Pivot at 30."""
    y = int(yy)
    return 1900 + y if y >= 30 else 2000 + y


def parse(path):
    txt = open(path, encoding='utf-8', errors='replace').read()
    pages = txt.split('\f')
    records = []
    problems = []
    for pno, page in enumerate(pages, 1):
        left, right = [], []
        for line in page.split('\n'):
            starts = [m.start() for m in DATE_ANCHOR.finditer(line)]
            if not starts:
                continue
            # Segment the line at date boundaries so a left-column record
            # cannot swallow the right-column record that follows it.
            bounds = starts + [len(line)]
            for i, s in enumerate(starts):
                seg = line[s:bounds[i + 1]]
                m = REC.match(seg)
                if not m:
                    problems.append((pno, s, seg.strip()))
                    continue
                g = m.groups()
                mm, dd, yy = g[0], g[1], g[2]
                nums = [int(x) for x in g[3:9]]
                code = (g[9] or '').strip()
                rec = {
                    'date': f'{parse_year(yy)}-{mm}-{dd}',
                    'nums': nums,
                    'code': re.sub(r'\s+', ' ', code),
                    'page': pno,
                }
                (left if s == 0 else right).append(rec)
        records.extend(left + right)   # column order within the page
    return records, problems


DP_START = '2020-10-10'   # first Double Play draw present in this PDF
MATRIX_CHANGE = '1999-10-27'  # first draw on the 6/53 matrix (6/49 before)


def infer_missing_labels(records):
    """The last two records of a page's right column render with no
    trailing draw-type label (a pdftotext layout artifact -- verified on
    page 1 of Lotto-6.pdf, where 03/11/26's pair carries no label at all).
    This affects exactly one same-date pair per page across the 12
    Double-Play-era pages.

    In all 604 pairs where BOTH labels survived, the source order is
    main draw first, Double Play second -- 604/604, no exceptions. That
    convention is what fills the gap here.  Records are in the PDF's own
    reverse-chronological order at this point.
    """
    idx = collections.defaultdict(list)
    for i, r in enumerate(records):
        idx[r['date']].append(i)
    inferred = 0
    for dte, positions in idx.items():
        if dte < DP_START or len(positions) != 2:
            continue
        a, b = positions
        if records[a]['code'] == '' and records[b]['code'] == '':
            records[a]['code'], records[a]['inferred'] = 'LOTTO', True
            records[b]['code'], records[b]['inferred'] = 'LOTTO DP', True
            inferred += 2
    return inferred


def main():
    src, out = sys.argv[1], sys.argv[2]
    records, problems = parse(src)
    for r in records:
        r.setdefault('inferred', False)

    print(f'parsed records : {len(records)}')
    print(f'unparsed lines : {len(problems)}')
    for p in problems[:10]:
        print(f'   page {p[0]} col {p[1]}: {p[2]!r}')

    n_inf = infer_missing_labels(records)
    print(f'labels inferred: {n_inf} (page-bottom pairs with no label in the PDF)')

    # --- integrity checks -------------------------------------------------
    dates = [r['date'] for r in records]
    print(f'date range     : {dates[-1]} .. {dates[0]}  (as parsed, newest first)')
    desc = all(dates[i] >= dates[i + 1] for i in range(len(dates) - 1))
    print(f'monotonic desc : {desc}')
    if not desc:
        bad = [(i, dates[i], dates[i + 1]) for i in range(len(dates) - 1)
               if dates[i] < dates[i + 1]]
        print(f'  {len(bad)} order breaks, first 5: {bad[:5]}')

    for r in records:
        if len(set(r['nums'])) != 6:
            print(f"  DUPLICATE NUMBERS {r['date']} {r['nums']}")

    codes = collections.Counter(r['code'] or '(none)' for r in records)
    print(f'draw-type codes: {dict(codes)}')

    # Pool size by era, inferred from the observed maximum.
    by_year = collections.defaultdict(list)
    for r in records:
        by_year[r['date'][:4]].extend(r['nums'])
    print('max number drawn, by year:')
    ys = sorted(by_year)
    line = []
    for y in ys:
        line.append(f'{y}:{max(by_year[y])}')
    for i in range(0, len(line), 10):
        print('   ' + '  '.join(line[i:i + 10]))

    # --- write chronological CSV -----------------------------------------
    # STABLE sort ascending, NOT reverse(). A blind reverse() also flips
    # the two records that share a date, which put every Double Play draw
    # AHEAD of the main draw it follows -- wrong for any sequential model.
    # A stable sort preserves the PDF's within-date order (main, then DP).
    records.sort(key=lambda r: r['date'])

    pair_order = collections.Counter()
    seen = collections.defaultdict(list)
    for r in records:
        seen[r['date']].append(r['code'])
    for dte, codes in seen.items():
        if dte >= DP_START and len(codes) == 2:
            pair_order[tuple(codes)] += 1
    print(f'within-date pair order after sort: {dict(pair_order)}')

    with open(out, 'w', newline='', encoding='utf-8') as fh:
        w = csv.writer(fh)
        w.writerow(['draw_date', 'n1', 'n2', 'n3', 'n4', 'n5', 'n6',
                    'draw_type', 'pool_size', 'label_inferred', 'source_page'])
        for r in records:
            pool = 49 if r['date'] < MATRIX_CHANGE else 53
            w.writerow([r['date']] + r['nums'] +
                       [r['code'] or 'LOTTO', pool,
                        int(r['inferred']), r['page']])
    print(f'\nwrote {out} ({len(records)} rows, chronological oldest-first)')


if __name__ == '__main__':
    main()
