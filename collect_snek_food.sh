#!/bin/sh
SRC_DIR=$1
DYN_DIR=$2
SNEK_EAT="^.*FEED_SNEK(\([^, ]*\),\s*\([^) ]*\)\s*).*$"

# Normal auto-globals:
echo -e \
'// auto-generated program to print Elf Forest globals gleaned from source\n'\
'#include <stdio.h>\n'\
  > $DYN_DIR/offer_snek_food.c
grep -rl FEED_SNEK\( $SRC_DIR \
  | grep "^.*\.h$" \
  | sed -e "s/^src\//#include \"/" \
  | sed -e "s/$/\"/" \
  >> $DYN_DIR/offer_snek_food.c
echo -e \
'\nint main(int argc, char**argv) {\n'\
'  printf("# auto-generated globals\\n");'\
  >> $DYN_DIR/offer_snek_food.c
grep -rh EFD_GL\( $SRC_DIR \
  | grep -v "#define FEED_SNEK(" \
  | sed "s/$SNEK_EAT/\1:\2/" \
  | awk -F: ' \
$1 == "i" { print "  printf(\""$2" = %d\\n\", "$2");" } \
$1 == "n" { print "  printf(\""$2" = %f\\n\", "$2");" } \
$1 == "s" { print "  printf(\""$2" = %s\\n\", "$2");" } \
' \
  >> $DYN_DIR/offer_snek_food.c
echo -e \
'  printf("# end auto-generated globals");\n'\
'  return 0;\n'\
'}' \
  >> $DYN_DIR/offer_snek_food.c
