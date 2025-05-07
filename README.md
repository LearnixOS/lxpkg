# LXPKG - Independent Source-Based Linux Package Manager

## ⚠️ Alpha Stage Warning
LXPKG is currently in alpha. Expect bugs, incomplete features, and occasional breakage. If you're comfortable testing early-stage software and reporting issues, your help would be appreciated!

## Overview
LXPKG is a lightweight, source-based package manager designed for Linux users who prefer compiling software from source. Inspired by KISS Linux but designed to work across multiple distributions, LXPKG provides a simple way to manage software builds while maintaining control over your system.

## Features
- **Minimal Dependencies**: Written in POSIX shell (`/bin/sh`) with no external language requirements
- **Source-Based**: Downloads and compiles packages from source
- **Flexible**: Works across multiple Linux distributions
- **Transparent**: Easy to understand and modify since it's just shell scripts
- **Lightweight**: No daemons or background processes

## Supported Distributions
- **Primary Development Platform**: Linux Mint
- **Known to Work On**: Ubuntu, Debian, Arch Linux, Fedora
- **Should Work On**: Most distributions with standard GNU toolchains

## Installation

### Quick Start
```bash
# Clone the repository
git clone https://github.com/learnixOS/repo.git /usr/src/lxpkg/repo
```

# Install the main script
sudo cp /usr/src/lxpkg/repo/lxpkg /usr/bin/lxpkg
sudo chmod +x /usr/bin/lxpkg

# Verify installation
lxpkg v

Dependencies

LXPKG requires basic build tools. Install these first:

Debian/Ubuntu/Mint:
```bash
sudo apt update
sudo apt install gcc make pkg-config aria2 tar gzip xz-utils zstd b3sum ncurses-dev git
```

Arch Linux:
```bash
sudo pacman -S gcc make pkgconf aria2 tar gzip xz zstd blake3 ncurses git
```

Fedora:
```bash
sudo dnf install gcc make pkg-config aria2 tar gzip xz zstd b3sum ncurses-devel git
```

Usage
Basic Commands
```bash
# Search for packages
lxpkg s <query>

# Install a package
lxpkg i <package>

# Remove a package
lxpkg r <package>

# List installed packages
lxpkg l

# Update repositories
lxpkg u

# Upgrade installed packages
lxpkg U
```

Package Creation Example (htop)

Here's how to create a package definition for htop:

    Create package directory:

```bash
mkdir -p /usr/src/lxpkg/repo/extra/htop
cd /usr/src/lxpkg/repo/extra/htop
```

    Create package files:

```bash

# sources file
echo "https://github.com/htop-dev/htop/releases/download/3.3.0/htop-3.3.0.tar.xz" > sources
```
```bash
# version file
echo "3.3.0 1" > version
```
```bash
# build script
cat > build << 'EOF'
#!/bin/sh
set -e
dest="$1"
./configure --prefix=/usr --enable-unicode --enable-cgroup
make
make install DESTDIR="$dest"
```

EOF
chmod +x build

```bash
# dependencies
echo "ncurses" > depends
```
```bash
# checksums (after downloading)
wget https://github.com/htop-dev/htop/releases/download/3.3.0/htop-3.3.0.tar.xz
sha256sum htop-3.3.0.tar.xz | awk '{print $1}' > checksums
rm htop-3.3.0.tar.xz
```
    Install the package:

```bash

lxpkg i htop
```

Technical Details
Package Structure

Each package lives in /usr/src/lxpkg/repo/<category>/<package>/ with these files:

    sources: Download URL(s)

    version: Package version and release

    build: Build script (optional)

    depends: Runtime dependencies

    checksums: Source file verification hashes

Database Structure

    Installed packages are tracked in /var/db/lxpkg/installed/

    Each installed package has:

        manifest: List of installed files

        version: Installed version and release

Contributing

LXPKG is a community project. We welcome contributions in these areas:

    Bug fixes (especially edge cases in shell script handling)

    Additional build system support

    Improved dependency resolution

    Documentation improvements

    Testing on different distributions

To contribute:

    Fork the repository

    Create a feature branch

    Submit a pull request

Known Issues

    Limited dependency resolution (basic ldd-based checking only)

    No package signing support yet

    Some edge cases with filenames containing special characters
