#!/usr/bin/env bash
# Entry point. Run from INSIDE WSL, from a checkout on the WSL ext4 filesystem.
#
# Usage: ./physx-build/build.sh [--profile dev|production] [--only <targets>] [--clean] [--allow-slow-fs]
#
# Targets for --only (comma list): linux-editor, linux-template-debug, linux-template-release,
#   windows-editor, windows-template-debug, windows-template-release
#
# Each platform is built in its own container, derived from the official Godot build
# containers (https://github.com/godotengine/build-containers):
#   godot-physx-linux   <- official godot-linux   (buildroot GCC SDK, SCons, .NET SDK)
#   godot-physx-windows <- official godot-windows (MinGW GCC, SCons, .NET SDK)
# The Linux pass (container-build-linux.sh) always runs first: it builds the Linux
# editor and generates the mono glue the Windows pass compiles in. The Windows pass
# (container-build-windows.sh) always runs too: it builds the requested Windows targets
# and refreshes the Windows C# assemblies, so the output never mixes commits.
#
# Output: ${GODOT_PHYSX_HOME:-~/godot-physx-out} (WSL ext4 path). The Windows side is
# handled by physx-build/install-windows.ps1, which pulls the Windows subset from WSL.
set -euo pipefail

PROFILE=dev
ONLY=""
CLEAN=0
ALLOW_SLOW_FS=0
IMAGE_LINUX=godot-physx-linux
IMAGE_WINDOWS=godot-physx-windows

# Official Godot build containers (https://github.com/godotengine/build-containers).
# The base images are built from this pinned commit on first run; override
# with BASE_TAG / BUILD_CONTAINERS_REF / BUILD_CONTAINERS_DIR.
BASE_TAG="${BASE_TAG:-4.7-f44}"
BUILD_CONTAINERS_REF="${BUILD_CONTAINERS_REF:-9d1304ed8a3f6a82d63a8392da0d6cc9e1b24dd2}"
BUILD_CONTAINERS_DIR="${BUILD_CONTAINERS_DIR:-$HOME/.cache/godot-build-containers}"

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
# Accept "a, b" as well as "a,b": the container scripts match targets against ",$ONLY,".
ONLY="${ONLY//[[:space:]]/}"

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

# Output home: a WSL (ext4) path. install-windows.ps1 pulls from here.
OUT_DIR="${GODOT_PHYSX_HOME:-$HOME/godot-physx-out}"

# 2. Images and volumes.
# Build the official base images on first use. The derived images are configuration-only
# layers (build user, caches) on top of the official toolchain images.
ensure_base_images() {
  if docker image inspect "godot-linux:${BASE_TAG}" >/dev/null 2>&1 && \
     docker image inspect "godot-windows:${BASE_TAG}" >/dev/null 2>&1; then return 0; fi
  echo "==> Official base images not found; building from godotengine/build-containers"
  echo "    pinned commit $BUILD_CONTAINERS_REF (checkout cached in $BUILD_CONTAINERS_DIR)"
  command -v git >/dev/null || { echo "git not found; needed to clone build-containers" >&2; exit 1; }
  if [[ ! -d "$BUILD_CONTAINERS_DIR/.git" ]]; then
    git clone --quiet https://github.com/godotengine/build-containers.git "$BUILD_CONTAINERS_DIR"
  fi
  git -C "$BUILD_CONTAINERS_DIR" fetch --quiet origin main
  git -C "$BUILD_CONTAINERS_DIR" checkout --quiet --force "$BUILD_CONTAINERS_REF"
  docker build -t "godot-fedora:${BASE_TAG}" -f "$BUILD_CONTAINERS_DIR/Dockerfile.base" "$BUILD_CONTAINERS_DIR"
  docker build --build-arg "img_version=${BASE_TAG}" -t "godot-linux:${BASE_TAG}" \
    -f "$BUILD_CONTAINERS_DIR/Dockerfile.linux" "$BUILD_CONTAINERS_DIR"
  docker build --build-arg "img_version=${BASE_TAG}" -t "godot-windows:${BASE_TAG}" \
    -f "$BUILD_CONTAINERS_DIR/Dockerfile.windows" "$BUILD_CONTAINERS_DIR"
}
ensure_base_images

echo "==> Building images $IMAGE_LINUX and $IMAGE_WINDOWS"
docker build -t "$IMAGE_LINUX" \
  --build-arg BASE_IMAGE="godot-linux:${BASE_TAG}" \
  --build-arg USER_UID="$(id -u)" --build-arg USER_GID="$(id -g)" \
  "$SCRIPT_DIR"
docker build -t "$IMAGE_WINDOWS" \
  --build-arg BASE_IMAGE="godot-windows:${BASE_TAG}" \
  --build-arg USER_UID="$(id -u)" --build-arg USER_GID="$(id -g)" \
  "$SCRIPT_DIR"
docker volume create godot-physx-scons-cache >/dev/null
docker volume create godot-physx-nuget-cache >/dev/null
# Named volumes may be root-owned (created by an earlier run); make them writable by the build user.
docker run --rm --user 0 -v godot-physx-scons-cache:/cache/scons -v godot-physx-nuget-cache:/cache/nuget \
  "$IMAGE_LINUX" chown -R "$(id -u):$(id -g)" /cache/scons /cache/nuget

# Build into a sibling ".new" dir, publish by rename swap. For partial (--only) builds,
# seed it with the existing output so the result is merged.
NEW="$OUT_DIR.new"; OLD="$OUT_DIR.old"
rm -rf "$NEW" "$OLD"
mkdir -p "$NEW"
if [[ -n "$ONLY" && -d "$OUT_DIR" ]]; then
  echo "==> --only: seeding from existing output"
  cp -a "$OUT_DIR/." "$NEW/"
fi

# 3. Run the passes, Linux first (it generates the mono glue the Windows pass needs).
run_pass() { # image script
  local image="$1" script="$2"
  docker run --rm \
    -v "$REPO_ROOT":/src \
    -v godot-physx-scons-cache:/cache/scons \
    -v godot-physx-nuget-cache:/cache/nuget \
    -v "$NEW":/out \
    -e PROFILE="$PROFILE" -e ONLY="$ONLY" -e CLEAN="$CLEAN" \
    -e SRC_SHA="$SRC_SHA" -e SRC_DIRTY="$SRC_DIRTY" \
    -e JOBS="${JOBS:-$(nproc)}" \
    "$image" bash "physx-build/$script"
}

echo "==> Linux pass (profile=$PROFILE only=${ONLY:-all})"
run_pass "$IMAGE_LINUX" container-build-linux.sh

echo "==> Windows pass"
run_pass "$IMAGE_WINDOWS" container-build-windows.sh

# 4. Finish the manifest: the MinGW version lives in the Windows image, so the Linux
# pass cannot record it. Append it (and the build date) to all manifest copies.
MINGW="$(docker run --rm "$IMAGE_WINDOWS" x86_64-w64-mingw32-gcc -dumpfullversion 2>/dev/null || echo n/a)"
for f in "$NEW/manifest.txt" "$NEW/linux/manifest.txt" "$NEW/windows/manifest.txt"; do
  printf 'mingw=%s\ndate=%s\n' "$MINGW" "$(date -u +%Y-%m-%dT%H:%M:%SZ)" >> "$f"
done

# 5. Publish atomically: rename swap, so a failed build never touches the previous output.
echo "==> Publishing to $OUT_DIR"
if [[ -d "$OUT_DIR" ]]; then mv "$OUT_DIR" "$OLD"; fi
mv "$NEW" "$OUT_DIR"
rm -rf "$OLD"

echo "==> Done. Output: $OUT_DIR"
echo "    Next, on Windows:  pwsh physx-build\\install-windows.ps1 -SetEnv"
