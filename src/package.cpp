#include "package.hpp"
#include "util.hpp"
#include "config.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <cstdio>
#include <dirent.h>
#include <algorithm>
#include <string>

// ANSI color codes
#define RESET   "\033[0m"
#define BOLD    "\033[1m"
#define ITALIC  "\033[3m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define CYAN    "\033[36m"

// Fallback command database
const std::map<std::string, std::vector<std::string>> Package::fallbackCommands = {
    {"make", {"./configure --prefix=/usr/local", "make"}},
    {"cmake", {"cmake -DCMAKE_INSTALL_PREFIX=/usr/local .", "make"}},
    {"meson", {"meson setup builddir --prefix=/usr/local", "ninja -C builddir"}},
    {"ninja", {"ninja -C ."}},
    {"configure", {"./configure --prefix=/usr/local", "make"}},
    {"python", {"python3 setup.py build", "python3 setup.py install --prefix=/usr/local"}}
};

Package::Package(const std::string& name, const std::string& repoPath, const std::vector<std::string>& repoSubdirs)
    : name(name),
      version("unknown"),
      sourceUrl(""),
      buildSystem(""),
      buildDir("/tmp/lxpkg-build/" + name),
      packagePath("") {
    // Find package directory in repo subdirectories
    for (const auto& subdir : repoSubdirs) {
        std::string path = repoPath + "/" + subdir + "/" + name;
        if (util::dirExists(path)) {
            packagePath = path;
            break;
        }
    }
    if (packagePath.empty()) {
        std::cerr << RED << "Error: Package " << BOLD << name << RESET << RED << " not found in repository" << RESET << std::endl;
        packagePath = repoPath + "/extra/" + name; // Default to extra for error reporting
    }
    readSourcesFile(repoPath, repoSubdirs);
    readBuildFile(repoPath, repoSubdirs);
    readVersionFile(repoPath, repoSubdirs);
    readDependsFile(repoPath, repoSubdirs);
}

bool Package::readSourcesFile(const std::string& repoPath, const std::vector<std::string>& repoSubdirs) {
    std::ifstream inputFile(packagePath + "/sources");
    if (!inputFile.is_open()) {
        std::cerr << RED << "Error: Cannot open sources file for " << BOLD << name << RESET << std::endl;
        return false;
    }
    std::getline(inputFile, sourceUrl);
    inputFile.close();
    return true;
}

bool Package::readVersionFile(const std::string& repoPath, const std::vector<std::string>& repoSubdirs) {
    std::ifstream inputFile(packagePath + "/version");
    if (!inputFile.is_open()) {
        std::cout << YELLOW << "Warning: No version file for " << BOLD << name << RESET << YELLOW << ", using 'unknown'" << RESET << std::endl;
        version = "unknown";
        return true;
    }
    std::getline(inputFile, version);
    // Trim whitespace and extra tokens
    version.erase(std::remove_if(version.begin(), version.end(), isspace), version.end());
    size_t spacePos = version.find(' ');
    if (spacePos != std::string::npos) {
        version = version.substr(0, spacePos);
    }
    inputFile.close();
    return true;
}

bool Package::readBuildFile(const std::string& repoPath, const std::vector<std::string>& repoSubdirs) {
    std::ifstream inputFile(packagePath + "/build");
    if (!inputFile.is_open()) {
        std::cerr << RED << "Error: Cannot open build file for " << BOLD << name << RESET << std::endl;
        return false;
    }
    std::string line, fullCommand;
    bool continuation = false;
    while (std::getline(inputFile, line)) {
        // Remove trailing whitespace
        line.erase(line.find_last_not_of(" \t") + 1);
        if (line.empty()) continue;
        if (line.find("#!") == 0 || line[0] == '#') continue;
        // Handle line continuations
        if (line.back() == '\\') {
            line.pop_back();
            fullCommand += line + " ";
            continuation = true;
        } else {
            fullCommand += line;
            if (continuation) {
                continuation = false;
            } else {
                fullCommand += "\n";
            }
        }
        if (!continuation && !fullCommand.empty()) {
            if (fullCommand.find("BUILD_SYSTEM=") == 0) {
                buildSystem = fullCommand.substr(12);
            } else {
                buildCommands.push_back(fullCommand);
            }
            fullCommand.clear();
        }
    }
    // Handle last command if it ends with a backslash
    if (!fullCommand.empty()) {
        buildCommands.push_back(fullCommand);
    }
    inputFile.close();
    // If no build system specified, check for script-like behavior
    if (buildSystem.empty()) {
        std::string extractedFile = buildDir + "/" + name;
        if (util::fileExists(extractedFile)) {
            buildSystem = "script";
            if (buildCommands.empty()) {
                buildCommands.push_back("install -Dm755 " + name + " /usr/bin/" + name);
            }
        }
    }
    return true;
}

bool Package::readDependsFile(const std::string& repoPath, const std::vector<std::string>& repoSubdirs) {
    std::ifstream inputFile(packagePath + "/depends");
    if (!inputFile.is_open()) return true; // No depends file, assume no dependencies
    std::string line;
    while (std::getline(inputFile, line)) {
        if (!line.empty()) {
            std::stringstream ss(line);
            std::string depName, depVersion;
            ss >> depName >> depVersion;
            if (depVersion.empty()) {
                depVersion = "";
            }
            dependencies.emplace_back(depName, depVersion);
        }
    }
    inputFile.close();
    return true;
}

std::string Package::captureCommandOutput(const std::string& cmd) const {
    std::string result;
    std::string tempFilePath = buildDir + "/error.log";
    std::string fullCmd = cmd + " 2>" + tempFilePath;
    system(fullCmd.c_str());
    std::ifstream inputFile(tempFilePath);
    if (inputFile.is_open()) {
        std::stringstream buffer;
        buffer << inputFile.rdbuf();
        result = buffer.str();
        inputFile.close();
    }
    unlink(tempFilePath.c_str());
    return result;
}

// Helper function to find the extracted source directory
std::string Package::findSourceDir() const {
    DIR* dir = opendir(buildDir.c_str());
    if (!dir) {
        std::cerr << RED << "Error: Cannot open build directory " << buildDir << RESET << std::endl;
        return "";
    }
    struct dirent* entry;
    std::string sourceDir;
    while ((entry = readdir(dir))) {
        if (entry->d_type == DT_DIR && std::string(entry->d_name) != "." && std::string(entry->d_name) != "..") {
            // Assume the first subdirectory is the source directory
            sourceDir = buildDir + "/" + entry->d_name;
            break;
        }
    }
    closedir(dir);
    return sourceDir;
}

bool Package::fetchSource() const {
    std::cout << "\n" << CYAN << "==> Fetching source for " << BOLD << name << "-" << version << RESET << std::endl;
    if (sourceUrl.empty()) {
        std::cerr << RED << "Error: No source URL for " << BOLD << name << RESET << std::endl;
        return false;
    }

    // Create build directory first
    if (mkdir(buildDir.c_str(), 0755) != 0 && errno != EEXIST) {
        std::cerr << RED << "Error: Failed to create build directory " << BOLD << buildDir << RESET << std::endl;
        return false;
    }

    std::string cmd = "wget -O source.tar " + sourceUrl;
    std::cout << CYAN << "  -> Running: " << ITALIC << cmd << "\n" << RESET;
    if (system(cmd.c_str()) != 0) {
        std::cerr << RED << "Failed to fetch source for " << BOLD << name << RESET << std::endl;
        return false;
    }

    std::cout << "\n" << CYAN << "==> Extracting source for " << BOLD << name << "-" << version << RESET << std::endl;
    // Detect tarball format
    std::string extractCmd;
    if (sourceUrl.rfind(".tar.gz") != std::string::npos) {
        extractCmd = "tar -xzf source.tar -C " + buildDir;
    } else if (sourceUrl.rfind(".tar.xz") != std::string::npos) {
        extractCmd = "tar -xJf source.tar -C " + buildDir;
    } else if (sourceUrl.rfind(".tar.bz2") != std::string::npos) {
        extractCmd = "tar -xjf source.tar -C " + buildDir;
    } else {
        std::cerr << RED << "Error: Unsupported tarball format for " << BOLD << name << RESET << std::endl;
        return false;
    }
    std::cout << CYAN << "  -> Running: " << ITALIC << extractCmd << RESET << std::endl;
    if (system(extractCmd.c_str()) != 0) {
        std::cerr << RED << "Failed to extract source for " << BOLD << name << RESET << std::endl;
        return false;
    }
    unlink("source.tar");
    return true;
}

bool Package::tryFallbackBuild() const {
    std::cout << "\n" << CYAN << "==> Attempting fallback build for " << BOLD << name << "-" << version << RESET << std::endl;
    if (chdir(buildDir.c_str()) != 0) {
        std::cerr << RED << "Error: Failed to change to build directory " << buildDir << RESET << std::endl;
        return false;
    }
    // Handle script-based packages
    if (buildSystem == "script" || buildSystem.empty()) {
        std::string scriptPath = name;
        if (util::fileExists(scriptPath)) {
            std::string cmd = "install -Dm755 " + scriptPath + " /usr/bin/" + name;
            std::cout << CYAN << "  -> Trying script fallback: " << ITALIC << cmd << RESET << std::endl;
            std::string errorOutput = captureCommandOutput(cmd);
            if (system(cmd.c_str()) == 0) {
                std::cout << GREEN << "  -> Script fallback succeeded" << RESET << std::endl;
                return true;
            } else {
                std::cerr << RED << "Script fallback failed: " << errorOutput << RESET << std::endl;
                return false;
            }
        }
        std::cerr << RED << "Error: No script found for " << BOLD << name << RESET << std::endl;
        return false;
    }
    // Handle other build systems
    auto it = fallbackCommands.find(buildSystem);
    if (it == fallbackCommands.end()) {
        std::cerr << RED << "Error: No fallback commands for build system: " << buildSystem << RESET << std::endl;
        return false;
    }
    // Find source directory
    std::string sourceDir = findSourceDir();
    if (sourceDir.empty()) {
        std::cerr << RED << "Error: No source directory found in " << buildDir << RESET << std::endl;
        return false;
    }
    if (chdir(sourceDir.c_str()) != 0) {
        std::cerr << RED << "Error: Failed to change to source directory " << sourceDir << RESET << std::endl;
        return false;
    }
    for (const auto& cmd : it->second) {
        std::cout << CYAN << "  -> Trying fallback: " << ITALIC << cmd << RESET << std::endl;
        std::string errorOutput = captureCommandOutput(cmd);
        if (system(cmd.c_str()) == 0) {
            std::cout << GREEN << "  -> Fallback succeeded" << RESET << std::endl;
            return true;
        } else {
            std::cout << YELLOW << "  -> Fallback failed: " << errorOutput << RESET << std::endl;
        }
    }
    std::cerr << RED << "Error: All fallback attempts failed for " << BOLD << name << RESET << std::endl;
    return false;
}

bool Package::build() const {
    std::cout << "\n" << CYAN << "==> Building " << BOLD << name << "-" << version << RESET << std::endl;
    if (chdir(buildDir.c_str()) != 0) {
        std::cerr << RED << "Error: Failed to change to build directory " << buildDir << RESET << std::endl;
        return false;
    }
    if (buildSystem == "script") {
        // Skip build step for script-based packages
        std::cout << CYAN << "  -> No compilation needed for script-based package" << RESET << std::endl;
        return true;
    }
    // Find source directory
    std::string sourceDir = findSourceDir();
    if (sourceDir.empty()) {
        std::cerr << RED << "Error: No source directory found in " << buildDir << RESET << std::endl;
        return false;
    }
    // Create temporary build script
    std::string scriptPath = buildDir + "/build.sh";
    std::ofstream scriptFile(scriptPath);
    if (!scriptFile.is_open()) {
        std::cerr << RED << "Error: Cannot create build script at " << scriptPath << RESET << std::endl;
        return false;
    }
    scriptFile << "#!/bin/sh\n";
    scriptFile << "set -e\n";
    scriptFile << "cd \"" << sourceDir << "\" || exit 1\n";
    // Write all build commands, preserving their original format
    for (const auto& cmd : buildCommands) {
        scriptFile << cmd << "\n";
    }
    scriptFile.close();
    // Make script executable
    if (chmod(scriptPath.c_str(), 0755) != 0) {
        std::cerr << RED << "Error: Failed to make build script executable" << RESET << std::endl;
        return false;
    }
    // Execute script with DESTDIR as argument
    std::string destDir = buildDir + "/install";
    std::string fullCmd = scriptPath + " \"" + destDir + "\"";
    std::cout << CYAN << "  -> Running build script: " << ITALIC << fullCmd << RESET << std::endl;
    std::string errorOutput = captureCommandOutput(fullCmd);
    if (system(fullCmd.c_str()) != 0) {
        std::cerr << RED << "Build failed: " << errorOutput << RESET << std::endl;
        unlink(scriptPath.c_str());
        return tryFallbackBuild();
    }
    unlink(scriptPath.c_str());
    return true;
}

bool Package::install() const {
    std::cout << "\n" << CYAN << "==> Installing " << BOLD << name << "-" << version << RESET << std::endl;
    if (chdir(buildDir.c_str()) != 0) {
        std::cerr << RED << "Error: Failed to change to build directory " << buildDir << RESET << std::endl;
        return false;
    }
    if (buildSystem == "script") {
        std::string cmd = buildCommands.empty() ? "install -Dm755 " + name + " /usr/bin/" + name : buildCommands[0];
        std::cout << CYAN << "  -> Running: " << ITALIC << cmd << RESET << std::endl;
        std::string errorOutput = captureCommandOutput(cmd);
        if (system(cmd.c_str()) != 0) {
            std::cerr << RED << "Installation failed for " << BOLD << name << RESET << ": " << errorOutput << std::endl;
            return tryFallbackBuild();
        }
    } else {
        // For non-script packages, installation is handled in the build script
        // Move files from DESTDIR to system
        std::string destDir = buildDir + "/install";
        if (!util::dirExists(destDir)) {
            std::cerr << RED << "Error: Installation directory " << destDir << " not found" << RESET << std::endl;
            return false;
        }
        std::string cmd = "cp -r " + destDir + "/* /";
        std::cout << CYAN << "  -> Running: " << ITALIC << cmd << RESET << std::endl;
        std::string errorOutput = captureCommandOutput(cmd);
        if (system(cmd.c_str()) != 0) {
            std::cerr << RED << "Installation failed for " << BOLD << name << RESET << ": " << errorOutput << RESET << std::endl;
            return false;
        }
    }
    // Update desktop database and icon cache
    std::cout << CYAN << "  -> Updating desktop database..." << RESET << std::endl;
    system("update-desktop-database /usr/share/applications 2>/dev/null || true");
    std::cout << CYAN << "  -> Updating icon cache..." << RESET << std::endl;
    system("gtk-update-icon-cache /usr/share/icons/hicolor 2>/dev/null || true");
    return true;
}

bool Package::remove() const {
    std::cout << "\n" << CYAN << "==> Removing " << BOLD << name << "-" << version << RESET << std::endl;
    // Common installation paths
    std::vector<std::string> paths = {
        "/usr/local/bin/" + name,
        "/usr/bin/" + name,
        "/opt/" + name,
        "/usr/local/sbin/" + name,
        "/usr/sbin/" + name,
        "/bin/" + name,
        "/sbin/" + name,
        "/usr/local/lib/" + name,
        "/usr/lib/" + name,
        "/usr/local/share/" + name,
        "/usr/share/" + name
    };
    bool success = true;
    for (const auto& path : paths) {
        std::string cmd = "rm -rf " + path;
        std::cout << CYAN << "  -> Running: " << ITALIC << cmd << RESET << std::endl;
        if (system(cmd.c_str()) != 0) {
            std::cerr << YELLOW << "Warning: Failed to remove " << path << RESET << std::endl;
            success = false;
        }
    }
    if (!success) {
        std::cerr << YELLOW << "Some paths could not be removed for " << BOLD << name << RESET << std::endl;
    }
    return success;
}

bool Package::resolveDependencies(const std::string& dbPath, std::set<std::string>& installed, std::vector<std::pair<std::string, std::string>>& toInstall) const {
    for (const auto& dep : dependencies) {
        std::string depName = dep.first;
        std::string depVersion = dep.second;
        std::string depKey = depName + "-" + (depVersion.empty() ? "unknown" : depVersion);
        if (installed.find(depKey) != installed.end()) {
            std::cout << CYAN << "  -> Dependency " << BOLD << depName << "-" << (depVersion.empty() ? "unknown" : depVersion) << RESET << CYAN << " already installed" << RESET << std::endl;
            continue;
        }
        if (util::fileExists(dbPath + "/" + depName)) {
            std::ifstream inputFile(dbPath + "/" + depName);
            std::string installedVersion;
            std::getline(inputFile, installedVersion);
            if (depVersion.empty() || installedVersion == depVersion) {
                std::cout << CYAN << "  -> Dependency " << BOLD << depName << "-" << installedVersion << RESET << CYAN << " already installed" << RESET << std::endl;
                installed.insert(depKey);
                continue;
            }
        }
        Package depPkg(depName, buildDir.substr(0, buildDir.find_last_of('/')), Config().getRepoSubdirs());
        toInstall.emplace_back(depName, depVersion.empty() ? depPkg.getVersion() : depVersion);
        if (!depPkg.resolveDependencies(dbPath, installed, toInstall)) {
            std::cerr << RED << "Failed to resolve dependencies for " << BOLD << depName << RESET << std::endl;
            return false;
        }
    }
    return true;
}

bool Package::confirmInstallation(const std::vector<std::pair<std::string, std::string>>& toInstall) {
    if (toInstall.empty()) return true;
    std::cout << "\n" << CYAN << "The following packages will be installed:" << RESET << "\n";
    for (const auto& pkg : toInstall) {
        std::cout << "  " << BOLD << pkg.first << "-" << pkg.second << RESET << "\n";
    }
    std::cout << "\n" << CYAN << "Proceed with installation? [Y/n] " << RESET;
    std::string input;
    std::getline(std::cin, input);
    return input.empty() || input[0] == 'Y' || input[0] == 'y';
}

void Package::searchPackages(const std::string& repoPath, const std::vector<std::string>& repoSubdirs, const std::string& searchName) {
    std::cout << "\n" << CYAN << "Available packages:" << RESET << "\n";
    bool found = false;
    for (const auto& subdir : repoSubdirs) {
        std::string dirPath = repoPath + "/" + subdir;
        DIR* dir = opendir(dirPath.c_str());
        if (!dir) {
            std::cerr << YELLOW << "Warning: Cannot open directory " << dirPath << RESET << std::endl;
            continue;
        }
        struct dirent* entry;
        while ((entry = readdir(dir))) {
            if (entry->d_type == DT_DIR && std::string(entry->d_name) != "." && std::string(entry->d_name) != "..") {
                if (!searchName.empty() && std::string(entry->d_name) != searchName) continue;
                Package pkg(entry->d_name, repoPath, repoSubdirs);
                std::cout << "  " << BOLD << "Name: " << RESET << pkg.getName() << "\n"
                          << "  " << BOLD << "Version: " << RESET << pkg.getVersion() << "\n"
                          << "  " << BOLD << "Build System: " << RESET << (pkg.buildSystem.empty() ? "unknown" : pkg.buildSystem) << "\n"
                          << "  " << BOLD << "Dependencies: " << RESET;
                auto deps = pkg.getDependencies();
                if (deps.empty()) {
                    std::cout << "(none)";
                } else {
                    for (size_t i = 0; i < deps.size(); ++i) {
                        std::cout << deps[i].first;
                        if (!deps[i].second.empty()) std::cout << "-" << deps[i].second;
                        if (i < deps.size() - 1) std::cout << ", ";
                    }
                }
                std::cout << "\n  " << BOLD << "Source: " << RESET << (pkg.sourceUrl.empty() ? "unknown" : pkg.sourceUrl)
                          << "\n  " << BOLD << "Repository: " << RESET << subdir << "\n\n";
                found = true;
            }
        }
        closedir(dir);
    }
    if (!found && !searchName.empty()) {
        std::cerr << RED << "Error: No package named " << BOLD << searchName << RESET << RED << " found in repository" << RESET << std::endl;
    }
}
