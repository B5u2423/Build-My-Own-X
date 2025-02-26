#!/usr/bin/env bash

CFLAGS="-Wall -Werror -pedantic -std=c99 -g"
OUTPUT_BIN="mysqlite"

gcc $CFLAGS -o $OUTPUT_BIN main.c