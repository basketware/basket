#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
#include <shlobj.h>
#else
#include <fcntl.h>
#include <unistd.h>
#endif

#define TEMP_LOG_FILE "napkin_log.txt"

#ifdef _WIN32
static int in_terminal() {
    return GetConsoleWindow() != NULL;
}
#else
static int in_terminal() {
    return isatty(fileno(stdout));
}
#endif

void err_init() {
    FILE *log_file;

    if (in_terminal()) return;

#ifdef _WIN32
    char temp_path[MAX_PATH];
    if (!GetTempPathA(MAX_PATH, temp_path)) return;

    snprintf(
        temp_path + strlen(temp_path),
        sizeof(temp_path) - strlen(temp_path),
        TEMP_LOG_FILE
    );
#else
    char temp_path[] = "/tmp/" TEMP_LOG_FILE;
#endif

    log_file = fopen(temp_path, "w");

    if (!log_file) return;

    freopen(temp_path, "w", stdout);
    freopen(temp_path, "w", stderr);
    fclose(log_file);
}

void err_fatal(const char *title, const char *format, ...) {
    char message[512];

    va_list args;
    va_start(args, format);
    vsnprintf(message, sizeof(message), format, args);
    va_end(args);

    fprintf(stderr, "%s\n", message);

    if (in_terminal()) exit(1);

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL_Init Error: %s\n", SDL_GetError());
        exit(1);
    }

    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, title, message, NULL);

    SDL_Quit();

    exit(1);
}
