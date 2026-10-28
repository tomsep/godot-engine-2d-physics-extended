# physx-build: Dockerised build of the "physx" Godot fork

Builds this fork (`4.7.2.physx.mono`) for **Linux and Windows** inside a Docker container running in **WSL2**. Output: editor, `template_debug` and `template_release` for each platform, all with C# (`module_mono_enabled=yes`), plus NuGet packages, `extension_api.json` and a `manifest.txt`.

Windows binaries are cross-compiled with MinGW-w64 (GCC, posix threads). There is no MSVC path.

Everything lives in `godot-physx/`. The build compiles **whatever is checked out** in the repo.

| File | Purpose |
| --- | --- |
| `Dockerfile` | Pinned toolchain image `godot-physx-build` |
| `flags.env` | Shared scons flags and profiles |
| `build.sh` | Entry point, run **inside WSL** |
| `container-build.sh` | The steps that run inside the container (called by `build.sh`) |
| `setup-wsl.sh` | One-time Docker Engine setup in WSL |
| `install-windows.ps1` | Windows-side install of templates, nupkgs and env vars |

## 1. Requirements

- Windows 11 with WSL2. 16 GB RAM or more recommended (see [Memory](#memory-and-timings)).
- About 60 GB free disk (image, caches, build trees, outputs).

## 2. Install WSL and a distro

In Windows PowerShell:

```powershell
wsl --install -d Ubuntu-22.04
```

An existing Ubuntu or Debian distro also works; the toolchain is in the container. Create your Linux user when prompted.

## 3. Configure WSL resources

Create `%USERPROFILE%\.wslconfig`:

```ini
[wsl2]
memory=12GB
swap=16GB
processors=10
```

Adjust to your machine (leave a few GB for Windows). Then apply:

```powershell
wsl --shutdown
```

## 4. Clone the engine repo into the WSL home

Do **not** use `/mnt/c`. File access there is extremely slow and `build.sh` refuses it.

```bash
cd ~
git clone https://github.com/tomsep/godot-engine-2d-physics-extended.git godot-physx
cd godot-physx
git checkout 4.7.2-develop
```

## 5. Install Docker Engine in WSL

Run inside WSL (in the cloned repo):

```bash
./physx-build/setup-wsl.sh
```

It enables systemd (needed so Docker starts automatically), installs Docker Engine and adds you to the `docker` group. If it asks you to, run `wsl --shutdown` from Windows and reopen WSL, then re-run it until it prints `OK`. Check:

```bash
docker run --rm hello-world
```

## 6. Build

```bash
./physx-build/build.sh                       # dev profile, everything
./physx-build/build.sh --profile production  # production=yes lto=thin
./physx-build/build.sh --only windows-editor,windows-template-release
./physx-build/build.sh --clean               # remove previous binaries and glue first
```

`--only` targets: `linux-editor`, `linux-template-debug`, `linux-template-release`, `windows-editor`, `windows-template-debug`, `windows-template-release`. A partial build is merged into the existing output. The Linux editor and mono glue are built automatically when needed. `JOBS=<n>` limits parallel compiles.

Output goes to `%LOCALAPPDATA%\godot-physx` (override with `GODOT_PHYSX_HOME`, a WSL path). A failed build does not touch a previous good output.

### Memory and timings

Record real numbers from your first runs here.

| Run | Time | Peak RAM |
| --- | --- | --- |
| Cold cache, dev | _to be measured_ | _to be measured_ |
| Warm cache, dev | _to be measured_ | _to be measured_ |
| Cold cache, production (LTO) | _to be measured_ | _to be measured_ |

If the build is killed (exit 137 / "Killed"), it ran out of memory: lower `JOBS`, raise `memory`/`swap` in `.wslconfig`, or use the dev profile.

## 7. Install on Windows

In Windows PowerShell, from the repo (or wherever you have the script; WSL files are at `\\wsl$\<distro>\home\<you>\godot-physx`):

```powershell
pwsh .\physx-build\install-windows.ps1 -SetEnv
```

This copies the Windows templates to `%APPDATA%\Godot\export_templates\4.7.2.physx.mono\` (`windows_debug_x86_64.exe`, `windows_release_x86_64.exe`) and sets user-level `GODOT_PHYSICS_EXTENDED` (Windows editor exe) and `GODOT_PHYSICS_EXTENDED_NUPKGS` (nupkgs folder, used by `GodotPhysicsExtendedPackages` in `Game/TandemRideshare/nuget.config`). Without `-SetEnv` it only prints them. Restart terminals and IDEs afterwards.

## 8. Verify

```powershell
& $env:GODOT_PHYSICS_EXTENDED --version
```

Expected: `4.7.2.physx.mono...`. In WSL, the Linux binaries (the folder is `/mnt/c/Users/<you>/AppData/Local/godot-physx/linux`):

```bash
./godot.linuxbsd.editor.x86_64.mono --version
```

If that fails with "Permission denied", NTFS lacks exec bits: `cp -r` the `linux` folder into your WSL home and `chmod +x` the binaries.

### Outputs

```text
%LOCALAPPDATA%\godot-physx\
  manifest.txt                 sha, dirty flag, profile, flags, tool versions, date
  windows\  godot.windows.{editor,template_debug,template_release}.x86_64.mono[.console].exe
            GodotSharp\  extension_api.json  manifest.txt
  linux\    godot.linuxbsd.{editor,template_debug,template_release}.x86_64.mono
            GodotSharp\  extension_api.json  manifest.txt
  nupkgs\   Godot.NET.Sdk, GodotSharp, ... *.nupkg
```

`--dump-extension-api` runs in a scratch folder, so the committed `extension_api.json` is untouched.

## Troubleshooting

| Symptom | Fix |
| --- | --- |
| `Cannot talk to the Docker daemon` | `sudo systemctl start docker`; make sure you are in the `docker` group and restarted WSL. |
| `refuse ... /mnt/c` | Move the clone into the WSL home (step 5). |
| Build killed / exit 137 | Out of memory. See [Memory](#memory-and-timings). |
| MinGW errors about threads or `std::mutex` | The image must select the posix variant: `docker run --rm godot-physx-build x86_64-w64-mingw32-g++ -v` should say `Thread model: posix`. |
| D3D12 dependency download fails | Re-run; the script `misc/scripts/install_d3d12_sdk_windows.py` downloads into `bin/build_deps`. Delete that folder to force a refetch. |
| Files in the repo owned by root | The image was built with a different UID; `docker rmi godot-physx-build` and rebuild (`build.sh` passes your UID). |
| Want a pristine rebuild | `./physx-build/build.sh --clean`, or `docker volume rm godot-physx-scons-cache` to drop the cache. |

## Optional

- Exclude the WSL VHDX (`%LOCALAPPDATA%\Packages\<distro>\LocalState\ext4.vhdx`) from Windows Defender real-time scanning for faster builds.
- Pin the base image digest and .NET SDK package version in the `Dockerfile` for reproducible images (see comments there).
