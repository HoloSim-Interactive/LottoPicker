#!/usr/bin/env bash
# Release a symbolic lock previously acquired with lock-acquire.sh.
#
# Usage: lock-release.sh <path> [holder-role]
#
# If <holder-role> is given, the release is refused unless that role
# actually holds the lock. Pass it -- releasing another role's lock
# lets two agents edit the same binary file at once, which is exactly
# what the lock exists to prevent, and git can't merge the result.

set -euo pipefail

PATH_TO_LOCK="$1"
HOLDER="${2:-}"
LOCK_FILE=".claude/locks/${PATH_TO_LOCK}.lock"

git config user.name  >/dev/null 2>&1 || git config user.name  "agent-relay-bot"
git config user.email >/dev/null 2>&1 || git config user.email "agent-relay-bot@users.noreply.github.com"

if [[ ! -f "$LOCK_FILE" ]]; then
  echo "No lock file at $LOCK_FILE -- nothing to release." >&2
  exit 0
fi

if [[ -n "$HOLDER" ]]; then
  CURRENT_HOLDER="$(jq -r '.holder // empty' "$LOCK_FILE" 2>/dev/null || true)"
  if [[ -n "$CURRENT_HOLDER" && "$CURRENT_HOLDER" != "$HOLDER" ]]; then
    echo "Refusing: lock on $PATH_TO_LOCK is held by $CURRENT_HOLDER, not $HOLDER." >&2
    echo "If it's genuinely abandoned, let it go stale (60m) instead of" >&2
    echo "force-releasing it -- someone may be mid-edit." >&2
    exit 1
  fi
fi

BRANCH="$(git rev-parse --abbrev-ref HEAD)"
if [[ "$BRANCH" == "HEAD" ]]; then
  echo "Detached HEAD -- can't determine a branch to push the unlock to." >&2
  exit 1
fi

git rm -q "$LOCK_FILE"
git commit -m "Unlock: $PATH_TO_LOCK" >/dev/null

for attempt in 1 2 3; do
  if git push origin "HEAD:${BRANCH}" >/dev/null 2>&1; then
    echo "Lock released: $PATH_TO_LOCK" >&2
    exit 0
  fi
  git fetch origin >/dev/null 2>&1
  # A rebase that hits a conflict leaves the repo mid-rebase, which breaks
  # every later git command in the run. Abort rather than leave that state.
  if ! git rebase "origin/${BRANCH}" >/dev/null 2>&1; then
    git rebase --abort >/dev/null 2>&1 || true
    echo "Rebase conflicted while releasing the lock -- aborted cleanly." >&2
  fi
  sleep $(( attempt * 2 ))
done

echo "Could not push the unlock for $PATH_TO_LOCK -- resolve manually." >&2
exit 1
