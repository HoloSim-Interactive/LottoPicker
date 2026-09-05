# GitHub Actions self-hosted runner watchdog (created 2026-08-25).
#
# WHY: a listener can stay ALIVE while silently losing its connection to
# GitHub (observed 2026-08-24 17:55Z -- the process ran for 32 minutes,
# logged nothing, and GitHub reported the runner offline the whole time).
# The Startup-folder launcher only fires at logon, so it cannot recover a
# mid-session wedge. Process-existence checks cannot either, because the
# process IS running. The only reliable signal is what GitHub itself
# reports, so that is what this polls.
#
# Restarts only a runner GitHub reports as NOT online, and only after
# killing whatever listener still belongs to that install directory.

$ErrorActionPreference = 'SilentlyContinue'
# Re-pointed 2026-08-26: DungeonMaster -> SPA-CGAL. The runners themselves
# were re-registered to the new repo; a runner belongs to exactly one repo,
# so this must name the same one they are registered to or every poll
# reports nothing and the watchdog silently never restarts anything.
$repo = 'Holosim/SPA-CGAL'

# Test-rig janitor (added 2026-08-29): agents stage ~5.2 GB throwaway
# Unreal hosts under C:\temp and do not remove them; fifty of them filled
# the disk and killed three relay runs in job setup. Runs hourly from this
# loop rather than as its own scheduled task, so it lives and dies with
# the watchdog the client already trusts and needs no elevation.
$janitor      = 'C:\actions-runner\rig-janitor.ps1'
$janitorEvery = 20      # loops; the loop sleeps 180 s, so ~1 hour
$loopCount    = 0
$log  = "$env:TEMP\runner-watchdog.log"
# holosim-agents-02 re-registered to SPA-CGAL 2026-08-27 (client direction
# after upgrading to 20X usage): two agent hand-offs execute concurrently.
# To drop back to the single-runner budget lever, stop its listener and
# remove it from this map.
$map  = @{
  'holosim-agents-01' = 'C:\actions-runner'
  'holosim-agents-02' = 'C:\actions-runner-agents2'
  'holosim-ci-01'     = 'C:\actions-runner-ci'
}

while ($true) {
  try {
    $raw = & gh api "repos/$repo/actions/runners" 2>$null
    if ($raw) {
      $data = $raw | ConvertFrom-Json
      foreach ($r in $data.runners) {
        if ($r.status -ne 'online' -and $map.ContainsKey($r.name)) {
          $dir = $map[$r.name]
          Add-Content $log "$(Get-Date -Format o)  $($r.name) reported $($r.status) -- restarting from $dir"
          Get-CimInstance Win32_Process -Filter "Name='Runner.Listener.exe'" |
            Where-Object { $_.ExecutablePath -like "$dir\*" } |
            ForEach-Object { Stop-Process -Id $_.ProcessId -Force }
          Start-Sleep -Seconds 5
          Start-Process -FilePath "$dir\run.cmd" -WindowStyle Minimized
          Add-Content $log "$(Get-Date -Format o)  $($r.name) relaunched"
        }
      }
    }
    # No else: an unreachable API means the network is down, not that a
    # runner is broken. The listeners retry on their own in that case.
  } catch { }

  # Janitor: hourly, and never allowed to take the watchdog down with it -
  # a runner that has gone offline matters more than disk hygiene.
  $loopCount++
  if ($loopCount % $janitorEvery -eq 0) {
    try {
      if (Test-Path $janitor) { & $janitor }
    } catch {
      Add-Content $log "$(Get-Date -Format o)  janitor threw: $($_.Exception.Message)"
    }
  }

  Start-Sleep -Seconds 180
}
