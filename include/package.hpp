#ifndef LXPKG_PACKAGE_HPP
#define LXPKG_PACKAGE_HPP

#include <string>
#include <vector>
#include <set>
#include <map>

class Package {
public:
    Package(const std::string& name, const std::string& repoPath, const std::vector<std::string>& repoSubdirs);
    bool fetchSource() const;
    bool build() const;
    bool install() const;
    bool remove() const;
    std::string getName() const { return name; }
    std::string getVersion() const { return version; }
    std::vector<std::pair<std::string, std::string>> getDependencies() const { return dependencies; }
    bool resolveDependencies(const std::string& dbPath, std::set<std::string>& installed, std::vector<std::pair<std::string, std::string>>& toInstall) const;
    static bool confirmInstallation(const std::vector<std::pair<std::string, std::string>>& toInstall);
    static void searchPackages(const std::string& repoPath, const std::vector<std::string>& repoSubdirs, const std::string& searchName = "");

private:
    std::string name;
    std::string version;
    std::string sourceUrl;
    std::string buildSystem;
    std::string buildDir;
    std::string packagePath; // Path to package directory
    std::vector<std::string> buildCommands;
    std::vector<std::pair<std::string, std::string>> dependencies; // name, version
    
    std::string findSourceDir() const;


    bool readBuildFile(const std::string& repoPath, const std::vector<std::string>& repoSubdirs);
    bool readSourcesFile(const std::string& repoPath, const std::vector<std::string>& repoSubdirs);
    bool readVersionFile(const std::string& repoPath, const std::vector<std::string>& repoSubdirs);
    bool readDependsFile(const std::string& repoPath, const std::vector<std::string>& repoSubdirs);
    bool tryFallbackBuild() const;
    std::string captureCommandOutput(const std::string& cmd) const;

    static const std::map<std::string, std::vector<std::string>> fallbackCommands;
};

#endif
