#!/bin/sh

set -xe

CFLAGS="-Wall -Wextra -ggdb -pedantic -Iraylib/include"
LDFLAGS="-lraylib -lGL -lm -lpthread -ldl"

SOURCE_FILES="src/main.c"
OUTPUT_EXECUTABLE="main"

cc $CFLAGS -o $OUTPUT_EXECUTABLE $SOURCE_FILES $LDFLAGS

clean() {
    echo "Cleaning..."
    rm -f "$OUTPUT_EXECUTABLE"
}

