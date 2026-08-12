/* os.h - the operating-system layer.
 *
 * This is where "control your PC" physically happens. The language
 * (interp.c) calls these plain functions; the messy platform code
 * (windows.h, and later Linux/your-own-OS) lives behind them in
 * os_win.c. Keeping it separate means <windows.h> never collides
 * with the language's own type names.
 *
 * Later, real kill/start/poke/screen-drawing go here too - one file
 * to swap when the language targets a different OS.
 */
#ifndef OS_H
#define OS_H

/* List the files in a folder.
 * Returns a malloc'd array of malloc'd strings (caller frees each
 * name and the array). *out_count gets how many. Empty on bad path. */
char **os_dir(const char *path, int *out_count);

/* List the names of every running process (read-only - safe).
 * Same ownership contract as os_dir. */
char **os_procs(int *out_count);

#endif /* OS_H */
