#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include "hal_storage.h"

#include "logging.h"
#include "Arduino.h"

#define SDCARD_ROOT "./sdcard"

static char mapped_path[512];

static const char *map_path(const char *path) {
    if (path == NULL) return SDCARD_ROOT;
    if (path[0] == '/') {
        snprintf(mapped_path, sizeof(mapped_path), "%s%s", SDCARD_ROOT, path);
    } else {
        snprintf(mapped_path, sizeof(mapped_path), "%s/%s", SDCARD_ROOT, path);
    }
    return mapped_path;
}

bool hal_storage_init(void) {
    struct stat st;
    if (stat(SDCARD_ROOT, &st) == 0 && S_ISDIR(st.st_mode)) {
        LOG_INF("EMULATOR-STORAGE", "Storage initialized at %s", SDCARD_ROOT);
        return true;
    }
    LOG_ERR("EMULATOR-STORAGE", "SD card root %s not found!", SDCARD_ROOT);
    return false;
}

bool hal_storage_ready(void) { return true; }

bool hal_storage_exists(const char *path) {
    struct stat st;
    return stat(map_path(path), &st) == 0;
}

bool hal_storage_mkdir(const char *path) {
    // Basic recursive mkdir or just single level for now
    return mkdir(map_path(path), 0755) == 0 || errno == EEXIST;
}

bool hal_storage_remove(const char *path) {
    return remove(map_path(path)) == 0;
}

bool hal_storage_rename(const char *old_path, const char *new_path) {
    char old_mapped[512];
    strncpy(old_mapped, map_path(old_path), sizeof(old_mapped));
    return rename(old_mapped, map_path(new_path)) == 0;
}

void *hal_storage_open(const char *path, int mode) {
    const char *m = (mode == 0) ? "rb" : "wb";
    FILE *f = fopen(map_path(path), m);
    return (void *)f;
}

int hal_storage_file_read(void *handle, void *buf, size_t count) {
    if (!handle) return -1;
    size_t n = fread(buf, 1, count, (FILE *)handle);
    if (n == 0 && ferror((FILE *)handle)) return -1;
    return (int)n;
}

size_t hal_storage_file_write(void *handle, const void *buf, size_t count) {
    if (!handle) return 0;
    return fwrite(buf, 1, count, (FILE *)handle);
}

bool hal_storage_file_seek(void *handle, size_t pos) {
    if (!handle) return false;
    return fseek((FILE *)handle, pos, SEEK_SET) == 0;
}

size_t hal_storage_file_position(void *handle) {
    if (!handle) return 0;
    return ftell((FILE *)handle);
}

size_t hal_storage_file_size(void *handle) {
    if (!handle) return 0;
    FILE *f = (FILE *)handle;
    long current = ftell(f);
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, current, SEEK_SET);
    return (size_t)size;
}

int hal_storage_file_available(void *handle) {
    if (!handle) return 0;
    FILE *f = (FILE *)handle;
    long current = ftell(f);
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, current, SEEK_SET);
    return (int)(size - current);
}

void hal_storage_file_close(void *handle) {
    if (handle) fclose((FILE *)handle);
}

void *hal_storage_dir_open(const char *path) {
    DIR *d = opendir(map_path(path));
    return (void *)d;
}

bool hal_storage_dir_next(void *handle, char *name_buf, size_t buf_size, bool *is_dir) {
    if (!handle) return false;
    struct dirent *de = readdir((DIR *)handle);
    while (de != NULL && (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0)) {
        de = readdir((DIR *)handle);
    }
    if (!de) return false;

    strncpy(name_buf, de->d_name, buf_size - 1);
    name_buf[buf_size - 1] = '\0';
    
    if (de->d_type == DT_DIR) {
        *is_dir = true;
    } else if (de->d_type == DT_REG) {
        *is_dir = false;
    } else {
        // Fallback for filesystems that don't support d_type
        *is_dir = false; // simplify
    }
    return true;
}

void hal_storage_dir_close(void *handle) {
    if (handle) closedir((DIR *)handle);
}

bool hal_storage_reinit(void) {
    return hal_storage_init();
}
