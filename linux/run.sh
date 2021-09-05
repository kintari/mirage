#!/bin/sh
SELF=`realpath $PWD/$0`
. `dirname $SELF`/config.env
ARGS=test/fib.txt
exec ${TARGET} "${ARGS}"