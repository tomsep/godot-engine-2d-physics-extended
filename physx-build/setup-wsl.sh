#!/usr/bin/env bash
# ==============================================================================
# WSL Setup Script
# ==============================================================================
# One-time setup for Windows Subsystem for Linux (WSL).
# Installs Docker Engine and configures systemd for automatic Docker daemon startup.
#
# Requirements:
# - WSL (Windows Subsystem for Linux)
# - Ubuntu or Debian distribution
# - Run as normal user (sudo is used for privileged operations)
#
# After running this script, you may need to:
# 1. Run 'wsl --shutdown' in Windows PowerShell
# 2. Reopen WSL
# 3. Verify with: docker run --rm hello-world
# ==============================================================================

set -euo pipefail

# ==============================================================================
# SAFETY CHECKS
# ==============================================================================

# Ensure script is not run as root (sudo is used where needed)
[[ $EUID -ne 0 ]] || {
    echo "ERROR: Run as your normal user, not root (sudo is used where needed)." >&2
    exit 1
}

# Verify this is actually running in WSL by checking /proc/version
# WSL reports itself as 'microsoft' in the version string
grep -qi microsoft /proc/version || {
    echo "ERROR: This does not look like WSL (Windows Subsystem for Linux)." >&2
    exit 1
}

# ==============================================================================
# DISTRIBUTION COMPATIBILITY CHECK
# ==============================================================================

# Load OS identification variables (ID, VERSION_CODENAME, etc.)
. /etc/os-release

# Only Ubuntu and Debian are supported for this setup
case "$ID" in
    ubuntu|debian)
        # Supported distribution
        ;;
    *)
        echo "ERROR: Unsupported distribution: $ID (use Ubuntu or Debian)." >&2
        exit 1
        ;;
esac

# ==============================================================================
# SYSTEMD CONFIGURATION
# ==============================================================================
# Docker requires systemd to start automatically in WSL.
# This section ensures systemd is enabled and running.

if [[ "$(ps -p 1 -o comm= 2>/dev/null)" != systemd ]]; then
    # Check if systemd is already enabled in wsl.conf
    if ! grep -qs '^systemd *= *true' /etc/wsl.conf; then
        echo "INFO: Enabling systemd in /etc/wsl.conf"
        # Append systemd configuration to wsl.conf
        printf '\n[boot]\nsystemd=true\n' | sudo tee -a /etc/wsl.conf >/dev/null
    fi

    echo "INFO: systemd is not currently running."
    echo "      Please run the following in Windows PowerShell:"
    echo "      > wsl --shutdown"
    echo "      Then reopen WSL and re-run this script."
    exit 0
fi

# ==============================================================================
# DOCKER ENGINE INSTALLATION
# ==============================================================================
# Install Docker Engine if not already present.
# Docker is required for containerized PhysX builds.

if ! command -v docker >/dev/null; then
    echo "INFO: Docker not found. Installing Docker Engine..."

    # Update package lists
    sudo apt-get update

    # Install prerequisites
    sudo apt-get install -y ca-certificates curl git

    # Create directory for Docker GPG key
    sudo install -m 0755 -d /etc/apt/keyrings

    # Download Docker's official GPG key
    sudo curl -fsSL "https://download.docker.com/linux/$ID/gpg" -o /etc/apt/keyrings/docker.asc
    sudo chmod a+r /etc/apt/keyrings/docker.asc

    # Add Docker's APT repository
    echo "deb [arch=$(dpkg --print-architecture) signed-by=/etc/apt/keyrings/docker.asc] https://download.docker.com/linux/$ID $VERSION_CODENAME stable" \
        | sudo tee /etc/apt/sources.list.d/docker.list >/dev/null

    # Update package lists again to include Docker packages
    sudo apt-get update

    # Install Docker Engine and related components
    sudo apt-get install -y \
        docker-ce \
        docker-ce-cli \
        containerd.io \
        docker-buildx-plugin \
        docker-compose-plugin

else
    echo "INFO: Docker already installed: $(docker --version)"
fi

# ==============================================================================
# DOCKER DAEMON CONFIGURATION
# ==============================================================================
# Enable Docker to start automatically and run it immediately

sudo systemctl enable --now docker

# ==============================================================================
# USER PERMISSIONS
# ==============================================================================
# Add the current user to the 'docker' group so Docker can be run without sudo.
# This requires a WSL restart to take effect.

if ! id -nG "$USER" | grep -qw docker; then
    echo "INFO: Adding $USER to the docker group..."
    sudo usermod -aG docker "$USER"
    echo "INFO: Added $USER to the docker group."
    echo "      Please run the following in Windows PowerShell:"
    echo "      > wsl --shutdown"
    echo "      Then reopen WSL to apply group changes."
fi
