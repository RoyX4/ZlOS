/* os_linux.c - Linux implementation of the os.h layer, mirroring os_win.c. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <ctype.h>
#include "os.h"

char **os_dir(const char *path, int *out_count) {
    DIR *d = opendir(path);
    if (!d) { *out_count = 0; return NULL; }
    int cap = 16, n = 0;
    char **names = malloc(sizeof(char*) * cap);
    struct dirent *ent;
    while ((ent = readdir(d)) != NULL) {
        if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) continue;
        if (n >= cap) { cap *= 2; names = realloc(names, sizeof(char*) * cap); }
        names[n++] = strdup(ent->d_name);
    }
    closedir(d);
    *out_count = n;
    return names;
}

char **os_procs(int *out_count) {
    DIR *d = opendir("/proc");
    if (!d) { *out_count = 0; return NULL; }
    int cap = 16, n = 0;
    char **names = malloc(sizeof(char*) * cap);
    struct dirent *ent;
    while ((ent = readdir(d)) != NULL) {
        int is_pid = 1;
        for (char *p = ent->d_name; *p; p++) if (!isdigit((unsigned char)*p)) { is_pid = 0; break; }
        if (!is_pid) continue;
        char path[300];
        snprintf(path, sizeof(path), "/proc/%s/comm", ent->d_name);
        FILE *f = fopen(path, "r");
        if (!f) continue;
        char comm[256];
        if (fgets(comm, sizeof(comm), f)) {
            comm[strcspn(comm, "\n")] = 0;
            if (n >= cap) { cap *= 2; names = realloc(names, sizeof(char*) * cap); }
            names[n++] = strdup(comm);
        }
        fclose(f);
    }
    closedir(d);
    *out_count = n;
    return names;
}
