// filesys.c
// Functions for interacting with directories and files.

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

#include "datatypes/string.h"

#include "filesys.h"

/*************
 * Constants *
 *************/

// TODO: Better OS branching!
#ifdef __linux__
CSTR(FS_DIRSEP, "/", 1);
#elif _WIN32
CSTR(FS_DIRSEP, "\\", 1);
#else
#error "OS not supported!"
#endif

/*************
 * Functions *
 *************/

string * fs_dirchild(string const * const dir, string const * const child) {
  return s_join(FS_DIRSEP, dir, child, NULL);
}

// Reads a file and returns a malloc'd char *:
char * load_file(char const * const filename, size_t *size) {
  char * buffer = NULL;
  FILE * f = fopen(filename, "rb");

  if (f == NULL) {
    fprintf(
      stderr,
      "Error: unable to open file '%s'.\n",
      filename
    );
    exit(EXIT_FAILURE);
  }
  fseek (f, 0, SEEK_END);
  *size = ftell(f);
  fseek (f, 0, SEEK_SET);
  buffer = malloc(*size);
  if (buffer == NULL) {
    fprintf(
      stderr,
      "Error: unable to allocate memory to read file '%s'.\n",
      filename
    );
    fclose(f);
    exit(EXIT_FAILURE);
  }
  fread(buffer, 1, *size, f);
  fclose(f);
  return buffer;
}

void fs_ensure_dir(string const * const filestring, mode_t mode) {
  char *filename
  struct stat st;

  filename = s_encode_nt(filestring);

  if (stat(filename, &st) != 0) {
#ifdef DEBUG
    // TODO: Careful around that user-supplied string!
    printf("Creating directory '%s'...\n", filename);
#endif
    if (mkdir(filename, mode) != 0) {
      perror("Failed to create directory.");
      free(filename);
      exit(errno);
    }
  }

  free(filename);
}


void walk_dir_tree(
  string const * const root,
  int (*filter_links)(string const * const, struct stat const* const, void*),
  void *fl_arg,
  int (*filter_dirs)(string const * const, struct stat const * const, void*),
  void *fd_arg,
  int (*filter_files)(string const * const, struct stat const * const, void*),
  void *ff_arg,
  void (*op)(string const * const, struct stat const * const, void*),
  void *op_arg
) {
  // Modeled on http://rosettacode.org/wiki/Walk_a_directory/Recursively#C
  struct dirent *entry;
  DIR *directory;
  struct stat st;
  char *tmp;
  string *current_filename;
  string *entry_filename;
  int len = s_count_bytes(root);

  if (len >= FILENAME_MAX - 1) {
    fprintf(
      stderr,
      "ERROR: walk_dir_tree given filename that's too long (%d chars).\n",
      len
    );
    exit(EXIT_FAILURE);
  }

  tmp = s_encode_nt(root);
  directory = opendir(tmp);
  free(tmp);

  if (directory == NULL) {
    fprintf(
      stderr,
      "ERROR: walk_dir_tree given invalid root directory '%.*s'.\n",
      s_count_bytes(root),
      s_raw(root)
    );
    exit(EXIT_FAILURE);
  }

  while ( (entry = readdir(dir)) ) { // assignment + check
    if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")) {
      continue;
    }

    entry_filename = create_string_from_ntchars(entry->d_name);
    current_filename = fs_dirchild(root, entry_filename);
    tmp = s_encode_nt(current_filename);

    if (lstat(tmp, &st) == -1) {
      fprintf(
        stderr,
        "ERROR: walk_dir_tree failed to stat entry '%.*s'.\n",
        s_count_bytes(current_filename),
        s_raw(current_filename)
      );
      perror("Bad entry or filename length exceeded.\n");
      exit(EXIT_FAILURE);
    }

    if (S_ISLNK(st.st_mode)) {
      if (!filter_links(entry_filename, &st, fl_arg)) {
        cleanup_string(entry_filename);
        cleanup_string(current_filename);
        free(tmp);
        continue;
      }
      // switch our stat info to the target of the symlink:
      if (stat(tmp, &st) == -1) {
        fprintf(
          stderr,
          "ERROR: walk_dir_tree failed to stat target of link '%.*s'.\n",
          s_count_bytes(current_filename),
          s_raw(current_filename)
        );
        perror("Broken link.\n");
        exit(EXIT_FAILURE);
      }
    }

    // we can free our temporary buffer now:
    free(tmp);

    if (S_ISDIR(st.st_mode)) {
      if (!filter_dirs(entry_filename, &st, fd_arg)) {
        cleanup_string(entry_filename);
        cleanup_string(current_filename);
        continue;
      }
      // recurse on this directory:
      walk_dir_tree(
        current_filename,
        filter_links, fl_arg,
        filter_dirs, fd_arg,
        filter_files, ff_arg,
        op, op_arg
      );
    } else {
      if (!filter_files(entry_filename, &st, ff_arg)) {
        cleanup_string(entry_filename);
        cleanup_string(current_filename);
        continue;
      }
      // process this file:
      op(current_filename, &st, op_arg);

      // cleanup allocated strings:
      cleanup_string(entry_filename);
      cleanup_string(current_filename);
    }
  }
}

int fs_walk_filter_skip_all(
  string const * const filename,
  struct stat const * const st,
  void *arg
) {
  return 0;
}

int fs_walk_filter_handle_all(
  string const * const filename,
  struct stat const * const st,
  void *arg
) {
  return 1;
}

int fs_walk_filter_ignore_hidden(
  string const * const filename,
  struct stat const * const st,
  void *arg
) {
  return s_get_char(filename, 0) != s_as_codepoint('.');
}

int fs_walk_filter_by_extension(
  string const * const filename,
  struct stat const * const st,
  void *v_ext
) {
  char *ext = (char*) v_ext;
  int el = strlen(ext);
  int fl = s_count_bytes(filename);
  char *fbytes = s_raw(filename);
  size_t i, j;
  if (el >= fl || fbytes[fl - el - 1] != '.') { return 0; }
  j = 0;
  for (i = fl - el; i < fl; ++i) {
    if (fbytes[i] != ext[j]) { return 0; }
    j += 1;
  }
  return 1;
}

