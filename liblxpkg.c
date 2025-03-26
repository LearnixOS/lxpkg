#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <archive.h>
#include <archive_entry.h>

#define MAX_PATH 4096

// Extract a tarball (supports .tar.gz, .tar.xz, .tar.bz2)
int extract_tarball(const char *tarball_path, const char *dest_dir) {
    struct archive *a;
    struct archive_entry *entry;
    int flags = ARCHIVE_EXTRACT_TIME | ARCHIVE_EXTRACT_PERM | ARCHIVE_EXTRACT_ACL | ARCHIVE_EXTRACT_FFLAGS;
    int r;

    a = archive_read_new();
    archive_read_support_filter_all(a);
    archive_read_support_format_all(a);  // Fixed typo here

    r = archive_read_open_filename(a, tarball_path, 10240);
    if (r != ARCHIVE_OK) {
        fprintf(stderr, "Failed to open archive: %s\n", archive_error_string(a));
        archive_read_free(a);
        return -1;
    }

    // Create destination directory if it doesn’t exist
    mkdir(dest_dir, 0755);

    // Extract all entries
    while (archive_read_next_header(a, &entry) == ARCHIVE_OK) {
        r = archive_read_extract(a, entry, flags);
        if (r != ARCHIVE_OK) {
            fprintf(stderr, "Extraction failed: %s\n", archive_error_string(a));
            archive_read_free(a);
            return -1;
        }
    }

    archive_read_close(a);
    archive_read_free(a);
    return 0;
}

// Install files from source to destination
int install_files(const char *src_dir, const char *install_dir, const char *files[], int num_files) {
    char src_path[MAX_PATH];
    char dest_path[MAX_PATH];
    for (int i = 0; i < num_files; i++) {
        snprintf(src_path, MAX_PATH, "%s/%s", src_dir, files[i]);
        snprintf(dest_path, MAX_PATH, "%s/%s", install_dir, files[i]);

        // Ensure destination directory exists
        char *dir = strdup(dest_path);
        char *slash = strrchr(dir, '/');
        if (slash) {
            *slash = '\0';
            mkdir(dir, 0755); // Simplified, recursive mkdir would be better
        }
        free(dir);

        // Copy file
        FILE *src = fopen(src_path, "rb");
        if (!src) {
            fprintf(stderr, "Failed to open source file %s: %s\n", src_path, strerror(errno));
            return -1;
        }
        FILE *dest = fopen(dest_path, "wb");
        if (!dest) {
            fclose(src);
            fprintf(stderr, "Failed to open destination file %s: %s\n", dest_path, strerror(errno));
            return -1;
        }

        char buffer[4096];
        size_t bytes;
        while ((bytes = fread(buffer, 1, sizeof(buffer), src)) > 0) {
            fwrite(buffer, 1, bytes, dest);
        }
        fclose(src);
        fclose(dest);
    }
    return 0;
}

// Check if a package is installed on the system
int check_system_install(const char *package_name) {
    char path[MAX_PATH];
    const char *prefixes[] = {"/usr/bin/", "/usr/lib/", "/opt/"};
    for (int i = 0; i < 3; i++) {
        snprintf(path, MAX_PATH, "%s%s", prefixes[i], package_name);
        if (access(path, F_OK) == 0) {
            return 1; // Found
        }
    }
    return 0; // Not found
}
