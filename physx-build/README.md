# physx-build: Dockerised build of the "physx" Godot fork

Builds this fork (`4.7.2.physx.mono`) for **Linux and Windows** inside a Docker container running in **WSL2**. Output: editor, `template_debug` and `template_release` for each platform, all with C# (`module_mono_enabled=yes`), plus NuGet packages, `extension_api.json` and a `manifest.txt`.

The toolchain is the official one, unmodified: like the official builds, each platform is built in its own container derived from the official build containers ([godotengine/build-containers](https://github.com/godotengine/build-containers)) — `godot-physx-linux` (official `godot-linux`: Fedora 44, buildroot GCC SDK targeting glibc 2.34 so binaries also run on Ubuntu 22.04 WSL, .NET SDK) and `godot-physx-windows` (official `godot-windows`: Fedora MinGW, GCC, posix threads, no MSVC path). The derived images are configuration-only layers (build user, caches). `build.sh` builds the pinned base images automatically on the first run and runs the Linux pass first (it generates the mono glue the Windows pass compiles in).

Everything lives in `godot-physx/`. The build compiles **whatever is checked out** in the repo.

| File | Purpose |
| --- | --- |
| `Dockerfile` | Configuration layer (build user, caches) for the official `godot-linux` / `godot-windows` build containers |
| `flags.env` | Shared scons flags and profiles |
| `build.sh` | Entry point, run **inside WSL**; builds the images and orchestrates the passes |
| `container-build-linux.sh` | Linux pass: editor, mono glue, Linux templates, assemblies, API dump (runs in `godot-physx-linux`) |
| `container-build-windows.sh` | Windows pass: editor, templates, assemblies (runs in `godot-physx-windows`, after the Linux pass) |
| `setup-wsl.sh` | One-time Docker Engine setup in WSL |
| `install-windows.ps1` | Windows-side install: pulls the Windows subset from WSL, installs templates, nupkgs and env vars |

## 1. Requirements

- Windows 11 with WSL2. 16 GB RAM or more recommended (see [Memory](#memory-and-timings)).
- About 70 GB free disk (official base images ~6 GB, derived images, caches, build trees, outputs).

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

On the first run, `build.sh` also builds the official base images (`godot-fedora`, `godot-linux`, `godot-windows`) from the pinned [build-containers](https://github.com/godotengine/build-containers) commit, then the two thin derived images on top. This needs network access and a few minutes plus ~6 GB of disk; the checkout is cached in `~/.cache/godot-build-containers`. Override with the `BASE_TAG`, `BUILD_CONTAINERS_REF` and `BUILD_CONTAINERS_DIR` environment variables.

```bash
./physx-build/build.sh                       # dev profile, everything
./physx-build/build.sh --profile production  # production=yes lto=full
./physx-build/build.sh --only windows-editor,windows-template-release
./physx-build/build.sh --clean               # remove previous binaries and glue first
```

`--only` targets: `linux-editor`, `linux-template-debug`, `linux-template-release`, `windows-editor`, `windows-template-debug`, `windows-template-release`. Separate targets with commas (spaces are ignored). A partial build is merged into the existing output. Every run, partial or not, builds the Linux editor and mono glue and rebuilds the C# assemblies and nupkgs, so the output never mixes assemblies from different commits. `JOBS=<n>` limits parallel compiles. Do not run two builds at once: they share the source tree, `bin/` and the output folders.

Output goes to `~/godot-physx-out` in WSL (override with `GODOT_PHYSX_HOME`, a WSL path), published by rename so a failed build does not touch a previous good output. Nothing is copied to Windows by `build.sh`; `install-windows.ps1` (next section) pulls the Windows subset from WSL.

### Memory and timings

Record real numbers from your first runs here.

| Run | Time | Peak RAM |
| --- | --- | --- |
| Cold cache, dev | _to be measured_ | _to be measured_ |
| Warm cache, dev | _to be measured_ | _to be measured_ |
| Cold cache, production (LTO) | _to be measured_ | _to be measured_ |

If the build is killed (exit 137 / "Killed"), it ran out of memory: lower `JOBS`, raise `memory`/`swap` in `.wslconfig`, or use the dev profile.

## 7. Install on Windows

In Windows PowerShell, from the repo (or wherever you have the script):

```powershell
pwsh .\physx-build\install-windows.ps1 -SetEnv
```

This pulls the Windows subset (`windows\`, `nupkgs\`, `manifest.txt`) from the WSL output (`\\wsl.localhost\...`) and copies it to `%LOCALAPPDATA%\godot-physx`. It then copies the Windows templates to `%APPDATA%\Godot\export_templates\4.7.2.physx.mono\` (`windows_debug_x86_64.exe`, `windows_release_x86_64.exe`) and sets user-level `GODOT_PHYSICS_EXTENDED` (Windows editor exe) and `GODOT_PHYSICS_EXTENDED_NUPKGS` (nupkgs folder, used by `GodotPhysicsExtendedPackages` in `Game/TandemRideshare/nuget.config`). Without `-SetEnv` it only prints them. Restart terminals and IDEs afterwards.

The WSL output path is auto-detected by asking WSL (honors `GODOT_PHYSX_HOME` there, defaults to `~/godot-physx-out`). If you use several distros or a non-default location, pass `-Distro <name>` or `-Source <\\wsl.localhost\...>`.

## 8. Verify

```powershell
& $env:GODOT_PHYSICS_EXTENDED --version
```

Expected: `4.7.2.physx.mono...`. In WSL (the folder is `~/godot-physx-out/linux`):

```bash
./godot.linuxbsd.editor.x86_64.mono --version
```

### Outputs

```text
~/godot-physx-out/ (WSL)          # everything build.sh produces
  manifest.txt                 sha, dirty flag, profile, flags, tool versions, date
  windows\  godot.windows.{editor,template_debug,template_release}.x86_64.mono[.console].exe
            GodotSharp\  extension_api.json  manifest.txt
  linux\    godot.linuxbsd.{editor,template_debug,template_release}.x86_64.mono
            GodotSharp\  extension_api.json  manifest.txt
  nupkgs\   Godot.NET.Sdk, GodotSharp, ... *.nupkg

%LOCALAPPDATA%\godot-physx\ (Windows)  # Windows subset, copied by install-windows.ps1
  manifest.txt
  windows\  (same layout as above)
  nupkgs\   (same layout as above)
```

`--dump-extension-api` runs in a scratch folder, so the committed `extension_api.json` is untouched.

## Troubleshooting

| Symptom | Fix |
| --- | --- |
| `Cannot talk to the Docker daemon` | `sudo systemctl start docker`; make sure you are in the `docker` group and restarted WSL. |
| `refuse ... /mnt/c` | Move the clone into the WSL home (step 4). |
| Build killed / exit 137 | Out of memory. See [Memory](#memory-and-timings). |
| MinGW errors about threads or `std::mutex` | Fedora MinGW defaults to posix threads: `docker run --rm godot-physx-windows x86_64-w64-mingw32-g++ -v` should say `Thread model: posix`. |
| Base image build fails or is stale | First run builds `godot-fedora`/`godot-linux` from the pinned build-containers commit (see `build.sh`). To move to a newer toolchain, bump `BUILD_CONTAINERS_REF` (and `BASE_TAG`), then `docker rmi godot-linux:$BASE_TAG` and rerun. |
| `build_assemblies.py` fails (.NET) | The base image ships the same .NET SDK the official builds use. If your fork needs a different version, add an install step to `Dockerfile`. |
| D3D12 dependency download fails | Re-run; the script `misc/scripts/install_d3d12_sdk_windows.py` downloads into `bin/build_deps`. Delete that folder to force a refetch. |
| Files in the repo owned by root | The images were built with a different UID; `docker rmi godot-physx-linux godot-physx-windows` and rebuild (`build.sh` passes your UID). |
| Want a pristine rebuild | `./physx-build/build.sh --clean`, or `docker volume rm godot-physx-scons-cache` to drop the cache. |

## Optional

- Exclude the WSL VHDX (`%LOCALAPPDATA%\Packages\<distro>\LocalState\ext4.vhdx`) from Windows Defender real-time scanning for faster builds.
- The toolchain is pinned by the `BUILD_CONTAINERS_REF` commit and the `BASE_TAG` image tag in `build.sh`; bump both together when you want a newer toolchain.
