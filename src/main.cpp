#include "config.hpp"
#include "package.hpp"
#include "database.hpp"
#include <iostream>
#include <string>
#include <set>
#include <vector>

// ANSI color codes
#define RESET   "\033[0m"
#define BOLD    "\033[1m"
#define ITALIC  "\033[3m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define CYAN    "\033[36m"

void printUsage() {
    std::cout << CYAN << "Usage: " << BOLD << "lxpkg" << RESET << CYAN << " <command> [package]\n"
              << "Commands:\n"
              << "  " << ITALIC << "install" << RESET << CYAN << " <pkg>  Install a package\n"
              << "  " << ITALIC << "remove" << RESET << CYAN << " <pkg>   Remove a package\n"
              << "  " << ITALIC << "list" << RESET << CYAN << "           List installed packages\n"
              << "  " << ITALIC << "search" << RESET << CYAN << " [pkg]    Search available packages\n" << RESET;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printUsage();
        return 1;
    }

    Config config;
    Database db(config.getDbPath());

    std::string command = argv[1];

    if (command == "install" && argc == 3) {
        Package pkg(argv[2], config.getRepoPath(), config.getRepoSubdirs());
        std::set<std::string> installed;
        std::vector<std::pair<std::string, std::string>> toInstall;
        toInstall.emplace_back(pkg.getName(), pkg.getVersion());

        std::cout << "\n" << CYAN << "Resolving dependencies for " << BOLD << pkg.getName() << "-" << pkg.getVersion() << RESET << CYAN << " ...\n" << RESET;
        if (!pkg.resolveDependencies(config.getDbPath(), installed, toInstall)) {
            std::cerr << RED << "Error: Dependency resolution failed for " << BOLD << argv[2] << RESET << std::endl;
            return 1;
        }

        if (!Package::confirmInstallation(toInstall)) {
            std::cout << CYAN << "Installation aborted by user.\n" << RESET;
            return 0;
        }

        for (const auto& p : toInstall) {
            if (db.isInstalled(p.first, p.second)) {
                std::cout << CYAN << "Skipping " << BOLD << p.first << "-" << p.second << RESET << CYAN << " (already installed)\n" << RESET;
                continue;
            }
            Package installPkg(p.first, config.getRepoPath(), config.getRepoSubdirs());
            std::cout << "\n" << CYAN << "Installing " << BOLD << p.first << "-" << p.second << RESET << CYAN << " ...\n" << RESET;
            if (!installPkg.fetchSource()) {
                std::cerr << RED << "Failed to fetch source for " << BOLD << p.first << RESET << std::endl;
                return 1;
            }
            if (!installPkg.build()) {
                std::cerr << RED << "Failed to build " << BOLD << p.first << RESET << std::endl;
                return 1;
            }
            if (!installPkg.install()) {
                std::cerr << RED << "Failed to install " << BOLD << p.first << RESET << std::endl;
                return 1;
            }
            if (!db.addPackage(p.first, p.second)) {
                std::cerr << RED << "Failed to update database for " << BOLD << p.first << RESET << std::endl;
                return 1;
            }
            std::cout << GREEN << "Successfully installed " << BOLD << p.first << "-" << p.second << RESET << std::endl;
        }
    } else if (command == "remove" && argc == 3) {
        Package pkg(argv[2], config.getRepoPath(), config.getRepoSubdirs());
        std::cout << "\n" << CYAN << "Removing " << BOLD << pkg.getName() << "-" << pkg.getVersion() << RESET << CYAN << " ...\n" << RESET;
        if (!pkg.remove()) {
            std::cerr << RED << "Failed to remove " << BOLD << pkg.getName() << RESET << std::endl;
            return 1;
        }
        if (!db.removePackage(pkg.getName())) {
            std::cerr << RED << "Failed to update database for " << BOLD << pkg.getName() << RESET << std::endl;
            return 1;
        }
        std::cout << GREEN << "Successfully removed " << BOLD << pkg.getName() << RESET << std::endl;
    } else if (command == "list" && argc == 2) {
        std::cout << "\n" << CYAN << "Installed packages:\n" << RESET;
        auto packages = db.listPackages();
        if (packages.empty()) {
            std::cout << CYAN << "  (none)\n" << RESET;
        } else {
            for (const auto& pkg : packages) {
                std::cout << "  " << BOLD << pkg.first << "-" << pkg.second << RESET << "\n";
            }
        }
    } else if (command == "search" && (argc == 2 || argc == 3)) {
        std::string searchName = (argc == 3) ? argv[2] : "";
        Package::searchPackages(config.getRepoPath(), config.getRepoSubdirs(), searchName);
    } else {
        printUsage();
        return 1;
    }

    return 0;
}
