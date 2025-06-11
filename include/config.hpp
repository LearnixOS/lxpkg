#ifndef LXPKG_CONFIG_HPP
#define LXPKG_CONFIG_HPP

#include <string>
#include <vector>

class Config {
public:
    Config();
    std::string getRepoPath() const { return repoPath; }
    std::string getDbPath() const { return dbPath; }
    std::string getBuildDir() const { return buildDir; }
    std::vector<std::string> getRepoSubdirs() const { return repoSubdirs; }
private:
    std::string repoPath;  // /var/db/lxpkg/repo
    std::string dbPath;    // /var/db/lxpkg/installed
    std::string buildDir;  // /tmp/lxpkg-build
    std::vector<std::string> repoSubdirs; // core,extra,wayland
};

#endif
