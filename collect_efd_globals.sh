#!/bin/sh
SRC_DIR=$1
DYN_DIR=$2
EFD_GLOBAL="^.*EFD_GL(\([^, ]*\),\s*\([^) ]*\)\s*).*$"
#EFD_GLOBAL="^.*EFD_GL(\([in]\),\s*\([^)]*\)).*$"

# Normal auto-globals:
echo -e \
'// auto-generated program to print EFD globals gleaned from source\n'\
'#include <stdio.h>\n'\
  > $DYN_DIR/print_efd_globals.c
grep -rl EFD_GL\( $SRC_DIR \
  | grep "^.*\.h$" \
  | sed -e "s/^src\//#include \"/" \
  | sed -e "s/$/\"/" \
  >> $DYN_DIR/print_efd_globals.c
echo -e \
'\nint main(int argc, char**argv) {\n'\
'  printf("##G auto_globals\\n");'\
  >> $DYN_DIR/print_efd_globals.c
grep -rh EFD_GL\( $SRC_DIR \
  | grep -v "#define EFD_GL(" \
  | sed "s/$EFD_GLOBAL/\1:\2/" \
  | awk -F: ' \
$1 == "i" { print "  printf(\"[[i "$2" %d]]\\n\", "$2");" } \
$1 == "n" { print "  printf(\"[[n "$2" %f]]\\n\", "$2");" } \
$1 == "s" { print "  printf(\"[[n "$2" %s]]\\n\", "$2");" } \
' \
  >> $DYN_DIR/print_efd_globals.c
echo -e \
'  printf("##");\n'\
'  return 0;\n'\
'}' \
  >> $DYN_DIR/print_efd_globals.c

#  | awk -F: '$1 == "n" { print $1 $2 }' \
