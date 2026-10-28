# Pinned toolchain image for building the "physx" Godot fork (Linux + Windows, mono).
#
# Build context is the physx-build/ folder. Nothing from the engine source is copied in;
# the source tree is bind-mounted at /src at run time.
#
# To pin the base image by digest, run `docker pull ubuntu:22.04`, then
# `docker inspect --format '{{index .RepoDigests 0}}' ubuntu:22.04`, and pass it with
# `--build-arg BASE_IMAGE=ubuntu@sha256:...` (or edit the default below).
ARG BASE_IMAGE=ubuntu:22.04
FROM ${BASE_IMAGE}

ARG DEBIAN_FRONTEND=noninteractive
ARG SCONS_VERSION=4.10.1
# Microsoft feed package version. Pin to an exact version (for example 8.0.404-1) once known
# good: `apt-cache madison dotnet-sdk-8.0` inside the image lists the options.
ARG DOTNET_SDK_PACKAGE=dotnet-sdk-8.0
ARG USER_UID=1000
ARG USER_GID=1000

RUN apt-get update && apt-get install -y --no-install-recommends \
        build-essential pkg-config git curl wget ca-certificates xz-utils unzip \
        python3 python3-pip ccache \
        mingw-w64 mingw-w64-tools \
        libx11-dev libxcursor-dev libxinerama-dev libxi-dev libxrandr-dev \
        libgl1-mesa-dev libglu1-mesa-dev libasound2-dev libpulse-dev libudev-dev \
        libwayland-dev libdecor-0-dev libdbus-1-dev libspeechd-dev libfontconfig-dev \
    && rm -rf /var/lib/apt/lists/*

# Windows cross-compile must use the posix thread model (std::thread, std::mutex).
RUN update-alternatives --set x86_64-w64-mingw32-gcc /usr/bin/x86_64-w64-mingw32-gcc-posix \
    && update-alternatives --set x86_64-w64-mingw32-g++ /usr/bin/x86_64-w64-mingw32-g++-posix

RUN pip3 install --no-cache-dir "scons==${SCONS_VERSION}"

# .NET SDK 8 from the Microsoft feed.
RUN curl -fsSL https://packages.microsoft.com/config/ubuntu/22.04/packages-microsoft-prod.deb -o /tmp/ms.deb \
    && dpkg -i /tmp/ms.deb && rm /tmp/ms.deb \
    && apt-get update && apt-get install -y --no-install-recommends ${DOTNET_SDK_PACKAGE} \
    && rm -rf /var/lib/apt/lists/*

ENV DOTNET_CLI_TELEMETRY_OPTOUT=1 \
    DOTNET_NOLOGO=1 \
    DOTNET_SKIP_FIRST_TIME_EXPERIENCE=1 \
    NUGET_PACKAGES=/cache/nuget \
    SCONS_CACHE=/cache/scons

# Non-root user matching the host UID/GID so files written to the bind mount are owned by you.
# Ubuntu 22.04 images may already have a user/group at these IDs, so remove clashes first.
RUN (getent passwd ${USER_UID} | cut -d: -f1 | xargs -r userdel -r || true) \
    && (getent group ${USER_GID} | cut -d: -f1 | xargs -r groupdel || true) \
    && groupadd -g ${USER_GID} builder \
    && useradd -m -u ${USER_UID} -g ${USER_GID} -s /bin/bash builder \
    && mkdir -p /cache/scons /cache/nuget /out && chown -R builder:builder /cache /out

USER builder
WORKDIR /src
