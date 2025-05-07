# LXPKG - Independent Source-Based Linux Package Manager

## ⚠️ Alpha Stage Warning

LXPKG is in alpha. Expect bugs, incomplete features, and occasional breakage. If you're comfortable testing early-stage software and reporting issues, your help is appreciated!

## Overview

LXPKG is a lightweight, source-based package manager for Linux users who prefer compiling software from source. Inspired by KISS Linux, it’s designed to work across multiple distributions, providing a simple and transparent way to manage software builds.

## Features

* **Minimal Dependencies**: Written in POSIX shell (/bin/sh) with no external language requirements.
* **Source-Based**: Downloads and compiles packages from source.
* **Flexible**: Works across multiple Linux distributions.
* **Transparent**: Easy to understand and modify as it's just shell scripts.
* **Lightweight**: No daemons or background processes.

## Supported Distributions

* **Primary Development Platform**: Linux Mint.
* **Tested On**: Ubuntu, Debian, Arch Linux, Fedora.
* **Expected Compatibility**: Most distributions with standard GNU toolchains.

## Installation

### Quick Start

```bash
# Clone the repository
git clone https://github.com/learnixOS/repo.git /usr/src/lxpkg/repo

# Install the main script
sudo cp /usr/src/lxpkg/repo/lxpkg /usr/bin/lxpkg
sudo chmod +x /usr/bin/lxpkg

# Verify installation
lxpkg v
```

### Dependencies

Ensure basic build tools are installed:

#### Debian/Ubuntu/Mint:

```bash
sudo apt update
sudo apt install gcc make pkg-config aria2 tar gzip xz-utils zstd b3sum ncurses-dev git
```

#### Arch Linux:

```bash
sudo pacman -S gcc make pkgconf aria2 tar gzip xz zstd blake3 ncurses git
```

#### Fedora:

```bash
sudo dnf install gcc make pkg-config aria2 tar gzip xz zstd b3sum ncurses-devel git
```

## Usage

### Basic Commands

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

### Package Creation Example: htop

#### Create the package directory:

```bash
mkdir -p /usr/src/lxpkg/repo/extra/htop
cd /usr/src/lxpkg/repo/extra/htop
```

#### Define package files:

##### Sources:

```bash
echo "https://github.com/htop-dev/htop/releases/download/3.3.0/htop-3.3.0.tar.xz" > sources
```

##### Version:

```bash
echo "3.3.0 1" > version
```

##### Build Script:

```bash
cat > build << 'EOF'
#!/bin/sh
set -e
dest="$1"
./configure --prefix=/usr --enable-unicode --enable-cgroup
make
make install DESTDIR="$dest"
EOF
chmod +x build
```

##### Dependencies:

```bash
echo "ncurses" > depends
```

##### Checksums:

```bash
wget https://github.com/htop-dev/htop/releases/download/3.3.0/htop-3.3.0.tar.xz
sha256sum htop-3.3.0.tar.xz | awk '{print $1}' > checksums
rm htop-3.3.0.tar.xz
```

## Technical Details

### Package Structure

Each package resides in `/usr/src/lxpkg/repo/<category>/<package>/` and includes:

* **sources**: Download URL(s).
* **version**: Package version and release.
* **build**: Build script (optional).
* **depends**: Runtime dependencies.
* **checksums**: Source file verification hashes.

### Database Structure

Installed packages are tracked in `/var/db/lxpkg/installed/`, each containing:

* **manifest**: List of installed files.
* **version**: Installed version and release.

## Contributing

LXPKG is a community project. Contributions are welcome in the following areas:

* Bug fixes (especially shell script edge cases).
* Support for additional build systems.
* Enhanced dependency resolution.
* Documentation improvements.
* Testing across distributions.

### Contribution Process

1. Fork the repository.
2. Create a feature branch.
3. Submit a pull request.

## Known Issues

* Limited dependency resolution (basic ldd-based checking only).
* No package signing support yet.
* Issues with filenames containing special characters.
