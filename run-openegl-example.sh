#!/bin/bash

. conf

#WAYLAND_DISPLAY="wayland-0"
#WAYLAND_DISPLAY="weston0"

if [ "$1" == "-g" ]; then
	WAYLAND_DEBUG=1 gdb build/bin/opengles
else
	build/bin/opengles
fi

