#include "config.hpp"
#include <unistd.h>
#include <sys/stat.h>

Config::Config() {
    repoPath = "/var/db/lxpkg/repo";
    dbPath = "/var/db/lxpkg/installed";
    buildDir = "/tmp/lxpkg-build";
    repoSubdirs = {"core", "extra", "wayland"};

    mkdir(dbPath.c_str(), 0755);
}
