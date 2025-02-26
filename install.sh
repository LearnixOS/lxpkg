#!/bin/bash

# Define required packages
REQUIRED_PACKAGES=(git python3-pip)
REPO_URL="https://github.com/user7210unix/lxpkg"
INSTALL_PATH="/usr/bin/lxpkg"

# Function to detect OS and install dependencies
detect_and_install() {
    if [ -f /etc/debian_version ]; then
        echo "Detected Debian/Ubuntu-based system."
        sudo apt update
        sudo apt install -y "${REQUIRED_PACKAGES[@]}"
    elif [ -f /etc/redhat-release ]; then
        if command -v dnf &>/dev/null; then
            echo "Detected RHEL-based system with DNF."
            sudo dnf install -y "${REQUIRED_PACKAGES[@]}"
        else
            echo "Detected RHEL-based system with YUM."
            sudo yum install -y "${REQUIRED_PACKAGES[@]}"
        fi
    elif [ -f /etc/arch-release ]; then
        echo "Detected Arch-based system."
        sudo pacman -Sy --noconfirm "${REQUIRED_PACKAGES[@]}"
    elif [ -f /etc/gentoo-release ]; then
        echo "Detected Gentoo system."
        sudo emerge "${REQUIRED_PACKAGES[@]}"
    elif [ -f /etc/alpine-release ]; then
        echo "Detected Alpine Linux system."
        sudo apk add "${REQUIRED_PACKAGES[@]}"
    elif [ -f /etc/lfs-release ]; then
        echo "Detected Linux From Scratch system."
        echo "Please ensure the following packages are installed manually: ${REQUIRED_PACKAGES[*]}"
        exit 0
    elif [ -f /etc/SuSE-release ] || [ -f /etc/os-release ] && grep -iq suse /etc/os-release; then
        echo "Detected SUSE-based system."
        sudo zypper install -y "${REQUIRED_PACKAGES[@]}"
    elif [ -f /etc/learnixos-release ]; then
        echo "Detected LearnixOS system."
        lxpkg install "${REQUIRED_PACKAGES[@]}"
    else
        echo "Unsupported OS. Please install ${REQUIRED_PACKAGES[*]} manually."
        exit 1
    fi
}

# Function to clone the repository and install lxpkg
install_lxpkg() {
    TEMP_DIR=$(mktemp -d)
    echo "Cloning the lxpkg repository..."
    git clone "$REPO_URL" "$TEMP_DIR"

    echo "Setting executable permissions for lxpkg..."
    chmod +x "$TEMP_DIR/lxpkg"

    echo "Copying lxpkg to /usr/bin/..."
    sudo cp -r "$TEMP_DIR/lxpkg" "$INSTALL_PATH"

    echo "Cleaning up temporary files..."
    rm -rf "$TEMP_DIR"

    echo -e "\nClean!\n✅ Successfully installed lxpkg! 😄"
}

# Main script
if [ "$EUID" -ne 0 ]; then
    echo "Please run this script as root or with sudo."
    exit 1
fi

detect_and_install
install_lxpkg

