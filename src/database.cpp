#include "database.hpp"
#include "util.hpp"
#include <fstream>
#include <dirent.h>
#include <iostream> 
#include <unistd.h> 

Database::Database(const std::string& dbPath) : dbPath(dbPath) {}

bool Database::addPackage(const std::string& name, const std::string& version) {
    std::ofstream file(dbPath + "/" + name);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open database file for " << name << std::endl;
        return false;
    }
    file << version << "\n";
    file.close();
    return true;
}

bool Database::removePackage(const std::string& name) {
    return unlink((dbPath + "/" + name).c_str()) == 0;
}

std::vector<std::pair<std::string, std::string>> Database::listPackages() const {
    std::vector<std::pair<std::string, std::string>> packages;
    DIR* dir = opendir(dbPath.c_str());
    if (!dir) return packages;

    struct dirent* entry;
    while ((entry = readdir(dir))) {
        if (entry->d_type == DT_REG) {
            std::ifstream file(dbPath + "/" + entry->d_name);
            std::string version;
            std::getline(file, version);
            packages.emplace_back(entry->d_name, version);
        }
    }
    closedir(dir);
    return packages;
}

bool Database::isInstalled(const std::string& name, const std::string& version) const {
    std::ifstream file(dbPath + "/" + name);
    if (!file.is_open()) return false;
    std::string installedVersion;
    std::getline(file, installedVersion);
    return installedVersion == version;
}
