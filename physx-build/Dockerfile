# Configuration layer on top of an official Godot build container:
#   https://github.com/godotengine/build-containers
# build.sh builds this file twice, once per platform, from a pinned commit of the
# build-containers repo:
#   --build-arg BASE_IMAGE=godot-linux:<tag>   -> godot-physx-linux   (buildroot GCC SDK, SCons, .NET SDK)
#   --build-arg BASE_IMAGE=godot-windows:<tag> -> godot-physx-windows (MinGW GCC, SCons, .NET SDK)
# All toolchains live in the official base images; this layer only adds a non-root
# build user and the cache/output layout. Keep it package-free.
#
# Build context is the physx-build/ folder. Nothing from the engine source is copied in;
# the source tree is bind-mounted at /src at run time.
ARG BASE_IMAGE=godot-linux:4.7-f44
FROM ${BASE_IMAGE}

ARG USER_UID=1000
ARG USER_GID=1000

# The official images run as root and keep toolchains under /root (buildroot SDKs,
# llvm-mingw); the non-root build user below needs to be able to read them.
RUN chmod 755 /root

# Linux builds pick up the buildroot SDK compiler from PATH (this mirrors how the
# official godot-build-scripts build the Linux binaries). Harmless in the Windows
# image, where the directory does not exist.
ENV DOTNET_SKIP_FIRST_TIME_EXPERIENCE=1 \
    NUGET_PACKAGES=/cache/nuget \
    SCONS_CACHE=/cache/scons \
    PATH="/root/x86_64-godot-linux-gnu_sdk-buildroot/bin:${PATH}"

# Non-root user matching the host UID/GID so files written to the bind mount are owned by you.
RUN (getent passwd ${USER_UID} | cut -d: -f1 | xargs -r userdel -r || true) \
    && (getent group ${USER_GID} | cut -d: -f1 | xargs -r groupdel || true) \
    && groupadd -g ${USER_GID} builder \
    && useradd -m -u ${USER_UID} -g ${USER_GID} -s /bin/bash builder \
    && mkdir -p /cache/scons /cache/nuget /out && chown -R builder:builder /cache /out

USER builder
WORKDIR /src
