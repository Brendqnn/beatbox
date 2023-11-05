#!/bin/sh

#set -xe

CFLAGS="-Wall -Wextra -Iraylib/include"
LDFLAGS="-lraylib -lGL -lm -lpthread -ldl"

SOURCE_FILES="src/main.c"
OUTPUT_EXECUTABLE="main"

cc $CFLAGS -o $OUTPUT_EXECUTABLE $SOURCE_FILES $LDFLAGS
