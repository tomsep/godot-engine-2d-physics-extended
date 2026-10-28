#!/usr/bin/env bash
# Entry point. Run from INSIDE WSL, from a checkout on the WSL ext4 filesystem.
#
# Usage: ./physx-build/build.sh [--profile dev|production] [--only <targets>] [--clean] [--allow-slow-fs]
#
# Targets for --only (comma list): linux-editor, linux-template-debug, linux-template-release,
#   windows-editor, windows-template-debug, windows-template-release
#
# Output: ${GODOT_PHYSX_HOME:-%LOCALAPPDATA%\godot-physx} (as seen from WSL)
set -euo pipefail

PROFILE=dev
ONLY=""
CLEAN=0
ALLOW_SLOW_FS=0
IMAGE=godot-physx-build

while [[ $# -gt 0 ]]; do
  case "$1" in
    --profile) PROFILE="${2:?--profile needs a value}"; shift 2 ;;
    --only) ONLY="${2:?--only needs a value}"; shift 2 ;;
    --clean) CLEAN=1; shift ;;
    --allow-slow-fs) ALLOW_SLOW_FS=1; shift ;;
    -h|--help) sed -n '2,10p' "$0"; exit 0 ;;
    *) echo "Unknown argument: $1" >&2; exit 2 ;;
  esac
done

[[ "$PROFILE" == dev || "$PROFILE" == production ]] || { echo "--profile must be dev or production" >&2; exit 2; }

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(git -C "$SCRIPT_DIR" rev-parse --show-toplevel)"

# 1. Refuse slow filesystems.
if [[ "$REPO_ROOT" == /mnt/* && $ALLOW_SLOW_FS == 0 ]]; then
  echo "ERROR: $REPO_ROOT is on a Windows drive (/mnt/...). Builds there are extremely slow." >&2
  echo "Clone the repo into your WSL home (for example ~/godot-physx), or pass --allow-slow-fs." >&2
  exit 1
fi

command -v docker >/dev/null || { echo "docker not found. Run physx-build/setup-wsl.sh first." >&2; exit 1; }
docker info >/dev/null 2>&1 || { echo "Cannot talk to the Docker daemon. Is it running, and are you in the docker group? (see README troubleshooting)" >&2; exit 1; }

SRC_SHA="$(git -C "$REPO_ROOT" rev-parse HEAD)"
if [[ -n "$(git -C "$REPO_ROOT" status --porcelain)" ]]; then SRC_DIRTY=yes; else SRC_DIRTY=no; fi
[[ $SRC_DIRTY == yes ]] && echo "Note: working tree is dirty; the manifest will record dirty=yes."

# Resolve the output home as a WSL path.
if [[ -n "${GODOT_PHYSX_HOME:-}" ]]; then
  HOME_DIR="$GODOT_PHYSX_HOME"
else
  WIN_LOCALAPPDATA="$(cmd.exe /c 'echo %LOCALAPPDATA%' 2>/dev/null | tr -d '\r' || true)"
  [[ -n "$WIN_LOCALAPPDATA" ]] || { echo "Could not read %LOCALAPPDATA%. Set GODOT_PHYSX_HOME (WSL path)." >&2; exit 1; }
  HOME_DIR="$(wslpath "$WIN_LOCALAPPDATA")/godot-physx"
fi

# 2. Image and volumes.
echo "==> Building image $IMAGE"
docker build -t "$IMAGE" \
  --build-arg USER_UID="$(id -u)" --build-arg USER_GID="$(id -g)" \
  "$SCRIPT_DIR"
docker volume create godot-physx-scons-cache >/dev/null
docker volume create godot-physx-nuget-cache >/dev/null
# Named volumes may be root-owned (created by an earlier run); make them writable by the build user.
docker run --rm --user 0 -v godot-physx-scons-cache:/cache/scons -v godot-physx-nuget-cache:/cache/nuget \
  "$IMAGE" chown -R "$(id -u):$(id -g)" /cache/scons /cache/nuget

# Staging dir on ext4. For partial (--only) builds, seed it with the existing output so it is merged.
STAGE="${GODOT_PHYSX_STAGE:-$HOME/.cache/godot-physx-stage}"
rm -rf "$STAGE"; mkdir -p "$STAGE"
if [[ -n "$ONLY" && -d "$HOME_DIR" ]]; then
  echo "==> --only: seeding staging from existing output"
  cp -a "$HOME_DIR/." "$STAGE/"
fi

# 3. Run.
echo "==> Running build (profile=$PROFILE only=${ONLY:-all})"
docker run --rm \
  -v "$REPO_ROOT":/src \
  -v godot-physx-scons-cache:/cache/scons \
  -v godot-physx-nuget-cache:/cache/nuget \
  -v "$STAGE":/out \
  -e PROFILE="$PROFILE" -e ONLY="$ONLY" -e CLEAN="$CLEAN" \
  -e SRC_SHA="$SRC_SHA" -e SRC_DIRTY="$SRC_DIRTY" \
  -e JOBS="${JOBS:-$(nproc)}" \
  "$IMAGE" bash physx-build/container-build.sh

# 5. Publish atomically: copy to a sibling temp dir, then swap by rename.
echo "==> Publishing to $HOME_DIR"
mkdir -p "$(dirname "$HOME_DIR")"
NEW="$HOME_DIR.new"; OLD="$HOME_DIR.old"
rm -rf "$NEW" "$OLD"
cp -a "$STAGE" "$NEW"
[[ -d "$HOME_DIR" ]] && mv "$HOME_DIR" "$OLD"
mv "$NEW" "$HOME_DIR"
rm -rf "$OLD" "$STAGE"

echo "==> Done. Output: $HOME_DIR"
echo "    Next, on Windows:  pwsh physx-build\\install-windows.ps1 -SetEnv"
