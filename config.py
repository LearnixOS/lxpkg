import os

# Default directories
SOURCES_BASE_DIR = os.getenv('LXPKG_SOURCES_BASE_DIR', '/home/user/sources')
BUILD_DIR = os.getenv('LXPKG_BUILD_DIR', '/home/user/sources/build')
INSTALL_DIR = os.getenv('LXPKG_INSTALL_DIR', '/usr')

# Logging configuration
LOG_DIR = os.getenv('LXPKG_LOG_DIR', '/var/log/lxpkg')

