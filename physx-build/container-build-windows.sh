#!/usr/bin/env bash
# Windows pass. Runs INSIDE the godot-physx-windows container, AFTER the Linux pass
# (container-build-linux.sh), which generated the mono glue and the Linux editor this
# pass builds on (both live in the shared /src tree). Called by build.sh; do not run directly.
#
# Env: PROFILE, ONLY (comma list or empty), JOBS.
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

# build.sh routes and validates targets, but keep this standalone-safe.
for t in ${ONLY//,/ }; do
  [[ " $ALL_TARGETS " == *" $t "* ]] || { echo "Unknown target: $t (valid: $ALL_TARGETS)" >&2; exit 2; }
done

step() { printf '\n==== %s ====\n' "$*"; }

scons_build() { # target
  # shellcheck disable=SC2086
  scons -j"$JOBS" $COMMON_FLAGS $WINDOWS_FLAGS $PROFILE_FLAGS target="$1"
}

mkdir -p "$OUT/windows" "$OUT/nupkgs" "$SCONS_CACHE" "$NUGET_PACKAGES"

# D3D12 dependencies (Mesa NIR, WinPixEventRuntime, Agility SDK). Cross-compiling puts them in bin/build_deps.
if want windows-editor || want windows-template-debug || want windows-template-release; then
  if [[ ! -d bin/build_deps/mesa-x86_64-gcc || ! -d bin/build_deps/agility_sdk || ! -d bin/build_deps/pix ]]; then
    step "D3D12 dependencies"
    python3 misc/scripts/install_d3d12_sdk_windows.py
  fi
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

if want windows-editor; then
  step "Windows editor (MinGW)"; scons_build editor
  collect "$OUT/windows" 'godot.windows.editor.x86_64.mono*.exe'
fi
if want windows-template-debug; then
  step "Windows template_debug (MinGW)"; scons_build template_debug
  collect "$OUT/windows" 'godot.windows.template_debug.x86_64.mono*.exe'
fi
if want windows-template-release; then
  step "Windows template_release (MinGW)"; scons_build template_release
  collect "$OUT/windows" 'godot.windows.template_release.x86_64.mono*.exe'
fi

# C# assemblies for Windows, rebuilt on every run like in the Linux pass. They only
# need the glue and GodotSharp API produced by the Linux pass, not the Windows editor
# built above.
step "Build assemblies (windows)"
python3 modules/mono/build_scripts/build_assemblies.py \
  --godot-output-dir=/src/bin --godot-platform=windows --push-nupkgs-local "$OUT/nupkgs"
rm -rf "$OUT/windows/GodotSharp"; cp -a bin/GodotSharp "$OUT/windows/GodotSharp"

step "Done (windows pass)"
