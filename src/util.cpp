#include "util.hpp"
#include <sys/stat.h>

namespace util {
    bool dirExists(const std::string& path) {
        struct stat st;
        return stat(path.c_str(), &st) == 0 && S_ISDIR(st.st_mode);
    }

    bool fileExists(const std::string& path) {
        struct stat st;
        return stat(path.c_str(), &st) == 0 && S_ISREG(st.st_mode);
    }
}
