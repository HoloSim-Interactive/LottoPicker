# Barebones UE 5.6 host project

Client-directed template resource (2026-08-27): a minimal, text-only
Unreal Engine 5.6 C++ project for hosting and building a plugin under
test, so no project ever spends agent budget regenerating this
boilerplate. Six source/config files, no binary assets, no Content/.

## Use

1. Copy this whole folder somewhere outside the repo (or into a
   scratch/staging area), e.g. `C:\staging\HostProject\`.
2. Copy or symlink the plugin under test into `Plugins/`, e.g.
   `HostProject\Plugins\SweptPathAnalysis\`.
3. Build the editor target:

   ```powershell
   & "C:\Program Files\Epic Games\UE_5.6\Engine\Build\BatchFiles\Build.bat" `
     HostProjectEditor Win64 Development -Project="C:\staging\HostProject\HostProject.uproject" -WaitMutex
   ```

4. Boot / plugin-mount sanity (exit 0 on a clean run):

   ```powershell
   & "C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" `
     "C:\staging\HostProject\HostProject.uproject" -ExecCmds="QUIT_EDITOR" `
     -nullrhi -unattended -nopause -nosplash
   ```

Read unattended-run evidence from `HostProject\Saved\Logs\HostProject.log`
(stdout is filtered without `-FullStdOutLogOutput`) — scoring rules in
the project's SDD § *Reading an unattended engine run*.

Renaming: replace "HostProject" consistently across the `.uproject`,
both `*.Target.cs` files (class names AND `ExtraModuleNames`), the
`Source/HostProject/` folder, `HostProject.Build.cs` class name, and
the `IMPLEMENT_PRIMARY_GAME_MODULE` line. Or don't rename — the name
is irrelevant to plugin testing.

EngineAssociation is pinned to `5.6` (the installed engine on the
pipeline host). Update it when the host engine moves.
