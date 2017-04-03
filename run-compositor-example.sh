#!/bin/bash

. conf

OPTS=--display=wayland-0

if [ "$1" == "-g" ]; then
	WAYLAND_DEBUG=client gdb --args build/bin/compositor ${OPTS}
else
	build/bin/compositor ${OPTS}
fi

