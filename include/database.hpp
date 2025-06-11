#ifndef LXPKG_DATABASE_HPP
#define LXPKG_DATABASE_HPP

#include <string>
#include <vector>

class Database {
public:
    Database(const std::string& dbPath);
    bool addPackage(const std::string& name, const std::string& version);
    bool removePackage(const std::string& name);
    std::vector<std::pair<std::string, std::string>> listPackages() const;
    bool isInstalled(const std::string& name, const std::string& version) const;

private:
    std::string dbPath;
};

#endif
