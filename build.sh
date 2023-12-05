#!/bin/sh

#set -xe

CFLAGS="-Wall -Wextra -Ilib/raylib-5.0_linux_amd64/include"
LDFLAGS="-lraylib -lGL -lm -lpthread -ldl"

SOURCE_FILES="src/main.c"

OUTPUT_EXECUTABLE="main"

cc $CFLAGS -o $OUTPUT_EXECUTABLE $SOURCE_FILES $LDFLAGS


