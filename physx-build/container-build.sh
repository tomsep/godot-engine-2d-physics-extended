#!/usr/bin/env bash
# Runs INSIDE the godot-physx-build container. Called by build.sh; do not run directly.
# Env: PROFILE, ONLY (comma list or empty), CLEAN (0/1), SRC_SHA, SRC_DIRTY, JOBS.
set -euo pipefail

cd /src
# shellcheck source=flags.env
source physx-build/flags.env

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

scons_build() { # platform-flags target
  local pf="$1" target="$2"
  # shellcheck disable=SC2086
  scons -j"$JOBS" $COMMON_FLAGS $pf $PROFILE_FLAGS target="$target"
}

mkdir -p "$OUT/linux" "$OUT/windows" "$OUT/nupkgs" "$SCONS_CACHE"
mkdir -p "$NUGET_PACKAGES"

if [[ "${CLEAN:-0}" == "1" ]]; then
  step "Cleaning"
  rm -rf bin/godot.* modules/mono/glue/GodotSharp/GodotSharp/Generated \
         modules/mono/glue/GodotSharp/GodotSharpEditor/Generated \
         modules/mono/glue/mono_glue.gen.cpp
fi

# What do we need?
NEED_LINUX_EDITOR=0
NEED_ASSEMBLIES=0
NEED_API_DUMP=0
if want linux-editor || want windows-editor; then NEED_LINUX_EDITOR=1; NEED_ASSEMBLIES=1; NEED_API_DUMP=1; fi
# Windows templates and editor read generated glue (mono_glue.gen.cpp), so they need glue too.
if want windows-template-debug || want windows-template-release || want linux-template-debug || want linux-template-release; then
  NEED_LINUX_EDITOR=1
fi

LINUX_EDITOR=bin/godot.linuxbsd.editor.x86_64.mono

if [[ $NEED_LINUX_EDITOR == 1 ]]; then
  step "Linux editor"
  scons_build "$LINUX_FLAGS" editor

  step "Generate mono glue"
  "$LINUX_EDITOR" --headless --generate-mono-glue modules/mono/glue
fi

# D3D12 dependencies (Mesa NIR, WinPixEventRuntime, Agility SDK). Cross-compiling puts them in bin/build_deps.
if want windows-editor || want windows-template-debug || want windows-template-release; then
  if [[ ! -d bin/build_deps/mesa-x86_64-gcc || ! -d bin/build_deps/agility_sdk || ! -d bin/build_deps/pix ]]; then
    step "D3D12 dependencies"
    python3 misc/scripts/install_d3d12_sdk_windows.py
  fi
fi

if [[ $NEED_ASSEMBLIES == 1 ]]; then
  step "Build assemblies (linuxbsd)"
  python3 modules/mono/build_scripts/build_assemblies.py \
    --godot-output-dir="$OUT/linux" --godot-platform=linuxbsd --push-nupkgs-local "$OUT/nupkgs"
  step "Build assemblies (windows)"
  python3 modules/mono/build_scripts/build_assemblies.py \
    --godot-output-dir="$OUT/windows" --godot-platform=windows --push-nupkgs-local "$OUT/nupkgs"
fi

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
  step "Linux template_debug"; scons_build "$LINUX_FLAGS" template_debug
  collect "$OUT/linux" 'godot.linuxbsd.template_debug.x86_64.mono'
fi
if want linux-template-release; then
  step "Linux template_release"; scons_build "$LINUX_FLAGS" template_release
  collect "$OUT/linux" 'godot.linuxbsd.template_release.x86_64.mono'
fi
if want windows-editor; then
  step "Windows editor (MinGW)"; scons_build "$WINDOWS_FLAGS" editor
  collect "$OUT/windows" 'godot.windows.editor.x86_64.mono*.exe'
fi
if want windows-template-debug; then
  step "Windows template_debug (MinGW)"; scons_build "$WINDOWS_FLAGS" template_debug
  collect "$OUT/windows" 'godot.windows.template_debug.x86_64.mono*.exe'
fi
if want windows-template-release; then
  step "Windows template_release (MinGW)"; scons_build "$WINDOWS_FLAGS" template_release
  collect "$OUT/windows" 'godot.windows.template_release.x86_64.mono*.exe'
fi

if [[ $NEED_API_DUMP == 1 ]]; then
  step "Dump extension API (scratch dir)"
  scratch="$(mktemp -d)"
  ( cd "$scratch" && "/src/$LINUX_EDITOR" --headless --dump-extension-api )
  cp -f "$scratch/extension_api.json" "$OUT/windows/extension_api.json"
  cp -f "$scratch/extension_api.json" "$OUT/linux/extension_api.json"
  rm -rf "$scratch"
fi

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
  echo "mingw=$(x86_64-w64-mingw32-gcc-posix -dumpfullversion 2>/dev/null || echo n/a)"
  echo "dotnet=$(dotnet --version)"
  echo "date=$(date -u +%Y-%m-%dT%H:%M:%SZ)"
} > "$OUT/manifest.txt"
cp -f "$OUT/manifest.txt" "$OUT/windows/manifest.txt"
cp -f "$OUT/manifest.txt" "$OUT/linux/manifest.txt"

step "Done"
cat "$OUT/manifest.txt"
