#!/bin/bash

if [ "$1" == "-g" ]; then
	LD_LIBRARY_PATH+=`pwd`/build/lib WAYLAND_DEBUG=1 gdb build/bin/example
else
	LD_LIBRARY_PATH+=`pwd`/build/lib build/bin/example
fi

