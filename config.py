import os

# Use ~/sources/ for universal user compatibility
SOURCES_BASE_DIR = os.getenv('LXPKG_SOURCES_BASE_DIR', os.path.expanduser('~/sources'))
BUILD_DIR = os.getenv('LXPKG_BUILD_DIR', os.path.expanduser('~/sources/build'))
INSTALL_DIR = os.getenv('LXPKG_INSTALL_DIR', os.path.expanduser('~/local'))  # Changed to user-writable default
LOG_DIR = os.getenv('LXPKG_LOG_DIR', os.path.expanduser('~/.config/lxpkg/logs'))
