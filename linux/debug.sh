#!/bin/sh
. linux/config.env
gdb --args "${TARGET}" "test/fib.txt"