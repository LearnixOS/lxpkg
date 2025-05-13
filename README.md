
<div align="center">


<img src="showcase/LXOS.png" align="center" alt=" Preview" width="250" style="display: block; margin: 32px auto; border: 2px solid #555; border-radius: 12px; box-shadow: 0 4px 10px rgba(0, 0, 0, 0.3);">

<div align="center">

## ⚠️ Alpha Stage Warning ⚠️

<div align="left">

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
sudo git clone https://github.com/learnixOS/repo.git /usr/src/lxpkg/repo

# Install the main script
sudo git clone https://github.com/LearnixOS/lxpkg.git /usr/bin/
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

### Commands

```bash
  a,alternatives       List or swap package alternatives
  b,build              Build packages from source
  c,checksum           Generate package checksums
  clean                Clean package cache
  d,depends            List package dependencies
  i,install            Install packages
  m,manifest           Show installed package files
  o,owns               Show which package owns a file
  O,orphans            List orphaned packages
  r,remove             Remove packages
  R,revdepends         List installed reverse dependencies
  rr,reporevdepends    List repository reverse dependencies
  s,search             Search for packages
  S,size               Show package size
  l,list               List installed packages
  u,update             Update repositories
  U,upgrade            Upgrade installed packages
  v,version            Show lxpkg version
  h,help               Show this help message
Environment Variables:
  BOOTSTRAP_LXPKG      Install to a custom root (e.g., /mnt)
  LXPKG_PATH           Repository paths (default: /usr/src/lxpkg/repo)
  LXPKG_COMPRESS       Compression type (gz, xz, zst; default: xz)
  LXPKG_SKIP_CHECKSUMS Skip checksum verification (set to 1)
  LXPKG_STRIP          Disable binary stripping (set to 0)
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
* **build**: Build script.
* **depends**: Dependencies.
* **checksums**: Source file verification hashes.
* **patches**: Patches.

### Database Structure

Installed packages are tracked in `/var/db/lxpkg/installed/`, each containing:

* **manifest**: List of installed files.
* **version**: Installed version and release.
* **depends**: List of needed dependencies for specific package.

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


## Changelog version 1.2.4

### Major Changes

- **Fixed POSIX Compatibility in `pkg_install`**
  - Replaced Bash-specific parameter expansion (`${var//pattern/replacement}`) with POSIX-compatible `sed` for cleaning `repo_ver` and `repo_rel` in the package database recording step.
  - Resolved the "Bad substitution" error when running under `sh` (e.g., `dash`), ensuring compatibility with POSIX shells.
  - Added validation for version and release values to prevent invalid database writes.

- **Enhanced Checksum Validation in `pkg_install`**
  - Added pre-build validation of the `checksums` file to check for existence and non-empty content before calling `pkg_build`.
  - Prevents downstream errors from malformed or missing `checksums` files.
  - Allows bypassing with `LXPKG_SKIP_CHECKSUMS=1` for flexibility during debugging.

- **Improved `pkg_verify` Robustness**
  - Refined checksum verification to handle malform==> lxpkg version 1.2.4ed `checksums` files more gracefully.
  - Added checks for empty files and invalid entries (missing hash or filename), with clearer error messages (e.g., `Malformed checksum entry #1 in ... (expected: <hash> <filename>, got: ...)`).
  - Introduced a `valid_checksums` counter to ensure at least one valid checksum is processed.
  - Retained `b3sum` for BLAKE3 checksum verification as requested.

- **Better Error Handling and Logging in `pkg_install`**
  - Improved logging throughout `pkg_install` to trace the installation process (e.g., cache checking, building, database recording).
  - Added error handling for file copying and database writes.
  - Logging warnings for non-critical failures (e.g., missing source files) while ensuring critical failures (e.g., manifest copying) terminate with clear errors.

- **Better Coloring and Searching**
  - Improved terminal output with enhanced coloring to distinguish warnings, errors, and informational messages.
  - Enhanced search functionality for packages, making it faster and more user-friendly.

### Known Issues

- **No Full Automatic Dependency Checker**
  - Currently lacks a fully automated dependency checker to skip already installed packages from other package managers.
