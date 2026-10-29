#!/usr/bin/env bash
# Linux pass. Runs INSIDE the godot-physx-linux container. Called by build.sh; do not run directly.
#
# Always builds the Linux editor: it generates the mono glue that every other target
# (including the Windows pass, container-build-windows.sh) compiles in. Then builds
# the C# assemblies, the requested Linux targets, and the extension API dump when an
# editor is part of the build.
#
# Env: PROFILE, ONLY (comma list or empty), CLEAN (0/1), SRC_SHA, SRC_DIRTY, JOBS.
set -euo pipefail

cd /src
# shellcheck source=flags.env
source physx-build/flags.env

# Linux builds must use the buildroot SDK compiler (glibc 2.34 target, same as the
# official Godot Linux binaries), not the container's host compiler. The image puts
# it on PATH; this guard catches a stale or hand-built image.
if [[ -z "${GODOT_SDK_LINUX_X86_64:-}" || "$(command -v gcc)" != "$GODOT_SDK_LINUX_X86_64"/* ]]; then
  echo "ERROR: gcc is not the buildroot SDK compiler (GODOT_SDK_LINUX_X86_64=${GODOT_SDK_LINUX_X86_64:-unset})." >&2
  echo "Rebuild the image with ./physx-build/build.sh (BASE_IMAGE must be godot-linux:*)." >&2
  exit 1
fi

PROFILE="${PROFILE:-dev}"
ONLY="${ONLY:-}"
JOBS="${JOBS:-$(nproc)}"
OUT=/out

case "$PROFILE" in
  dev) PROFILE_FLAGS="$PROFILE_DEV_FLAGS" ;;
  production) PROFILE_FLAGS="$PROFILE_PRODUCTION_FLAGS" ;;
  *) echo "Unknown profile: $PROFILE" >&2; exit 2 ;;
esac

ALL_TARGETS="linux-editor linux-template-debug linux-template-release windows-editor windows-template-debug windows-template-release"

want() {
  [[ -z "$ONLY" ]] && return 0
  [[ ",$ONLY," == *",$1,"* ]]
}

for t in ${ONLY//,/ }; do
  [[ " $ALL_TARGETS " == *" $t "* ]] || { echo "Unknown target: $t (valid: $ALL_TARGETS)" >&2; exit 2; }
done

step() { printf '\n==== %s ====\n' "$*"; }

scons_build() { # target
  # shellcheck disable=SC2086
  scons -j"$JOBS" $COMMON_FLAGS $LINUX_FLAGS $PROFILE_FLAGS target="$1"
}

mkdir -p "$OUT/linux" "$OUT/windows" "$OUT/nupkgs" "$SCONS_CACHE" "$NUGET_PACKAGES"

if [[ "${CLEAN:-0}" == "1" ]]; then
  step "Cleaning"
  rm -rf bin/godot.* modules/mono/glue/GodotSharp/GodotSharp/Generated \
         modules/mono/glue/GodotSharp/GodotSharpEditor/Generated \
         modules/mono/glue/mono_glue.gen.cpp
fi

# The C# assemblies and nupkgs are rebuilt on every run, so a partial build never ships
# templates next to assemblies from an older commit. The Windows pass builds the Windows
# assemblies itself, after this pass. The API dump is only needed with an editor.
NEED_API_DUMP=0
if want linux-editor || want windows-editor; then NEED_API_DUMP=1; fi

LINUX_EDITOR=bin/godot.linuxbsd.editor.x86_64.mono

step "Linux editor"
scons_build editor

step "Generate mono glue"
"$LINUX_EDITOR" --headless --generate-mono-glue modules/mono/glue

step "Build assemblies (linuxbsd)"
# GodotTools.csproj references GodotSharp.dll at <src>/bin/GodotSharp/Api/Debug, so build
# into /src/bin and copy the result out afterwards.
rm -rf "$OUT/nupkgs"; mkdir -p "$OUT/nupkgs"
python3 modules/mono/build_scripts/build_assemblies.py \
  --godot-output-dir=/src/bin --godot-platform=linuxbsd --push-nupkgs-local "$OUT/nupkgs"
rm -rf "$OUT/linux/GodotSharp"; cp -a bin/GodotSharp "$OUT/linux/GodotSharp"

# Collect a built binary (and its .console variant) into an output dir.
collect() { # dest-dir pattern
  local dest="$1" pattern="$2" f found=0
  for f in bin/$pattern; do
    [[ -e "$f" ]] || continue
    cp -f "$f" "$dest/"; found=1
  done
  [[ $found == 1 ]] || { echo "Expected output bin/$pattern not found" >&2; exit 1; }
}

if want linux-editor; then collect "$OUT/linux" 'godot.linuxbsd.editor.x86_64.mono'; fi
if want linux-template-debug; then
  step "Linux template_debug"; scons_build template_debug
  collect "$OUT/linux" 'godot.linuxbsd.template_debug.x86_64.mono'
fi
if want linux-template-release; then
  step "Linux template_release"; scons_build template_release
  collect "$OUT/linux" 'godot.linuxbsd.template_release.x86_64.mono'
fi

if [[ $NEED_API_DUMP == 1 ]]; then
  step "Dump extension API (scratch dir)"
  scratch="$(mktemp -d)"
  ( cd "$scratch" && "/src/$LINUX_EDITOR" --headless --dump-extension-api )
  cp -f "$scratch/extension_api.json" "$OUT/windows/extension_api.json"
  cp -f "$scratch/extension_api.json" "$OUT/linux/extension_api.json"
  rm -rf "$scratch"
fi

# The mingw= and date= lines are appended by build.sh after the Windows pass: the
# MinGW compiler only exists in the Windows image.
step "Manifest"
version_py() { python3 -c "import version; print(version.$1)"; }
{
  echo "version=$(version_py major).$(version_py minor).$(version_py patch)"
  echo "status=$(version_py status)"
  echo "name=$(version_py name)"
  echo "sha=${SRC_SHA:-unknown}"
  echo "dirty=${SRC_DIRTY:-unknown}"
  echo "profile=$PROFILE"
  echo "targets=${ONLY:-all}"
  echo "common_flags=$COMMON_FLAGS"
  echo "linux_flags=$LINUX_FLAGS"
  echo "windows_flags=$WINDOWS_FLAGS"
  echo "profile_flags=$PROFILE_FLAGS"
  echo "scons=$(scons --version | grep -m1 -o 'SCons: v[0-9.]*\|SCons by .*' | head -1)"
  echo "gcc=$(gcc -dumpfullversion)"
  echo "dotnet=$(dotnet --version)"
} > "$OUT/manifest.txt"
cp -f "$OUT/manifest.txt" "$OUT/windows/manifest.txt"
cp -f "$OUT/manifest.txt" "$OUT/linux/manifest.txt"

step "Done (linux pass)"
