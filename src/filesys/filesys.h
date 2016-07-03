#ifndef INCLUDE_FILESYS_H
#define INCLUDE_FILESYS_H

// filesys.h
// Functions for interacting with directories and files.

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

#include "datatypes/string.h"

/*************
 * Constants *
 *************/

// The directory separator for the current OS.
extern string const * const FS_DIRSEP;

/*************
 * Functions *
 *************/

// Returns a new string that joins the given directory and child strings using
// the operating system's directory separator.
string * fs_dirchild(string const * const dir, string const * const child);

// Reads a file and returns a malloc'd char*.
// TODO: Use a string as the filename!
char * load_file(char const * const filename, size_t *size);

// Creates a directory if none exists.
void fs_ensure_dir(string const * const filestring, mode_t mode);


// Walks a directory tree recursively, filtering directories entered and files
// processed using the given filtering functions (each passed the corresponding
// extra argument).
void walk_dir_tree(
  char const * const root,
  int (*filter_links)(char const * const, struct stat const* const, void*),
  void *fl_arg,
  int (*filter_dirs)(char const * const, struct stat const * const, void*),
  void *fd_arg,
  int (*filter_files)(char const * const, struct stat const * const, void*),
  void *ff_arg,
  void (*op)(char const * const, struct stat const * const, void*),
  void *op_arg
);

// Some basic functions to use as arguments for walk_dir_tree:
int fs_walk_filter_skip_all(
  char const * const filename,
  struct stat const * const st,
  void *arg
);
int fs_walk_filter_handle_all(
  char const * const filename,
  struct stat const * const st,
  void *arg
);
int fs_walk_filter_ignore_hidden(
  char const * const filename,
  struct stat const * const st,
  void *arg
);
int fs_walk_filter_by_extension(
  char const * const filename,
  struct stat const * const st,
  void *v_ext
);

#endif // INCLUDE_FILESYS_H
