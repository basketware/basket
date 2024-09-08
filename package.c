#define BASKET_INTERNAL
#include "basket.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <stdint.h>
#include <fcntl.h>

#ifdef _WIN32
    #include <windows.h>
    #include <libloaderapi.h>
    #include <direct.h>
    #define PATH_MAX _MAX_PATH
    #define realpath(N,R) _fullpath((R),(N),PATH_MAX)

#else
    #include <sys/mman.h>
    #include <sys/stat.h>
    #include <unistd.h>
    #include <limits.h>
    #define __USE_XOPEN_EXTENDED

    #if __APPLE__
        #include <mach-o/dyld.h>
    #endif

#endif

#define MINIZ_NO_DEFLATE_APIS
#define MINIZ_NO_ARCHIVE_WRITING_APIS
#include "lib/miniz.h"

typedef struct {
    void* data;
    size_t size;
    #ifdef _WIN32
        HANDLE file_handle;
        HANDLE mapping_handle;
    #else
        int fd;
    #endif
} mmap_file;

static mmap_file* mmap_file_open(const char* filename) {
    mmap_file* mf = (mmap_file*)malloc(sizeof(mmap_file));
    if (!mf) return NULL;

#ifdef _WIN32
    mf->file_handle = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (mf->file_handle == INVALID_HANDLE_VALUE) {
        free(mf);
        return NULL;
    }

    mf->size = GetFileSize(mf->file_handle, NULL);
    mf->mapping_handle = CreateFileMapping(mf->file_handle, NULL, PAGE_READONLY, 0, 0, NULL);
    if (!mf->mapping_handle) {
        CloseHandle(mf->file_handle);
        free(mf);
        return NULL;
    }

    mf->data = MapViewOfFile(mf->mapping_handle, FILE_MAP_READ, 0, 0, 0);
    if (!mf->data) {
        CloseHandle(mf->mapping_handle);
        CloseHandle(mf->file_handle);
        free(mf);
        return NULL;
    }
#else
    mf->fd = open(filename, O_RDONLY);
    if (mf->fd == -1) {
        free(mf);
        return NULL;
    }

    struct stat sb;
    if (fstat(mf->fd, &sb) == -1) {
        close(mf->fd);
        free(mf);
        return NULL;
    }
    mf->size = sb.st_size;

#ifdef __APPLE__
    mf->data = mmap(NULL, mf->size, PROT_READ, MAP_SHARED | MAP_NOCACHE, mf->fd, 0);
#else
    mf->data = mmap(NULL, mf->size, PROT_READ, MAP_SHARED, mf->fd, 0);
#endif
    if (mf->data == MAP_FAILED) {
        close(mf->fd);
        free(mf);
        return NULL;
    }
#endif

    return mf;
}

static void mmap_file_close(mmap_file* mf) {
    if (!mf) return;

#ifdef _WIN32
    UnmapViewOfFile(mf->data);
    CloseHandle(mf->mapping_handle);
    CloseHandle(mf->file_handle);
#else
    munmap(mf->data, mf->size);
    close(mf->fd);
#endif

    free(mf);
}

static long find_zip_header(mmap_file* mf) {
    if (!mf || !mf->data || mf->size < 30) return -1;
    uint8_t* data = (uint8_t*)mf->data;
    long file_size = mf->size;

    if (memcmp(data, "\x50\x4b\x03\x04", 4) == 0)
        return 0;

    long earliest_valid_header = -1;

    for (long i = 0; i < file_size - 30; i++) {
        if (memcmp(data + i, "\x50\x4b\x03\x04", 4) == 0) {
            // Check if this could be the global header
            uint16_t version_needed = *(uint16_t*)(data + i + 4);
            uint16_t general_flag = *(uint16_t*)(data + i + 6);
            uint16_t compression_method = *(uint16_t*)(data + i + 8);
            uint32_t compressed_size = *(uint32_t*)(data + i + 18);
            uint16_t filename_length = *(uint16_t*)(data + i + 26);
            uint16_t extra_field_length = *(uint16_t*)(data + i + 28);

            // Check for characteristics of a global header
            if (version_needed <= 20 && // Lower versions are more likely for the first entry
                general_flag == 0 &&    // No encryption or special flags
                compressed_size == 0 && // Often 0 for directory entries
                i + 30 + filename_length + extra_field_length <= file_size) {

                // POTENTIAL GLOBAL HEADER!
                if (earliest_valid_header == -1 || i < earliest_valid_header) {
                    earliest_valid_header = i;
                }
            }
        }
    }

    return earliest_valid_header;
}

static int is_directory(const char *path) {
    struct stat path_stat;
    if (stat(path, &path_stat) != 0)
        return -1;
    return S_ISDIR(path_stat.st_mode);
}

static char mounted_dir[PATH_MAX] = {0};

static mmap_file *zip_file = NULL;
static mz_zip_archive zip_archive;

static void pak_unmount() {
    mounted_dir[0] = 0;

    if (zip_archive.m_archive_size) {
        mz_zip_reader_end(&zip_archive);
        memset(&zip_archive, 0, sizeof(zip_archive));
    }

    if (zip_file != NULL) {
        mmap_file_close(zip_file);
        zip_file = NULL;
    }
}

int pak_mount(const char *name) {
    int is_dir = is_directory(name);

    if (is_dir == -1) return 1;

    if (is_dir) {
        char real_path[PATH_MAX];

        // Resolve the real path
        if (realpath(name, real_path) == NULL)
            return 1;

        pak_unmount();

        strncpy(mounted_dir, real_path, PATH_MAX - 1);
        mounted_dir[PATH_MAX - 1] = '\0';

        return 0;
    }

    mmap_file *tmp_zip_file = mmap_file_open(name);
    if (tmp_zip_file == NULL)
        return 1;

    long start = find_zip_header(tmp_zip_file);
    if (start == -1)
        return 1;

    char *data = (char*)(tmp_zip_file->data + start);
    size_t size = tmp_zip_file->size - start;

    static mz_zip_archive tmp_zip_archive;
    memset(&tmp_zip_archive, 0, sizeof(tmp_zip_archive));

    // Initialize the zip archive from memory
    if (!mz_zip_reader_init_mem(&tmp_zip_archive, data, size, 0)) {
        mmap_file_close(tmp_zip_file);
        return 1;
    }

    zip_archive = tmp_zip_archive;
    zip_file = tmp_zip_file;

    return 1;
}

char *pak_read(const char* name, size_t *size) {
    static size_t _t;

    if (size == NULL)
        size = &_t;

    *size = 0;

    if (zip_archive.m_archive_size) {
        int idx = mz_zip_reader_locate_file(&zip_archive, name, NULL, 0);
        if (idx == -1)
            return NULL;

        return mz_zip_reader_extract_to_heap(&zip_archive, idx, size, 0);
    }

    char full_path[PATH_MAX];

    // Construct the full path
    snprintf(full_path, PATH_MAX, "%s/%s", mounted_dir, name);

    mmap_file *file = mmap_file_open(full_path);
    if (file == NULL)
        return NULL;

    // Allocate buffer
    char *buffer = (char*)malloc(file->size);
    memcpy(buffer, file->data, file->size);
    *size = file->size;

    mmap_file_close(file);

    return buffer;
}
