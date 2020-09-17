#!/bin/bash

SCRIPT_DIR=$(dirname $0)
GODOT_DIR=$SCRIPT_DIR/godot

SCONS="scons -C$GODOT_DIR -j$(nproc) custom_modules=../modules"

if [ ! -e $GODOT_DIR/modules/mono/glue/mono_glue.gen.cpp ]
then
	MONO_GLUE="mono_glue=no"

	$SCONS tools=yes module_mono_enabled=yes $MONO_GLUE

	$GODOT_DIR/bin/godot.x11.tools.64.mono --generate-mono-glue $GODOT_DIR/modules/mono/glue
fi

$SCONS tools=yes module_mono_enabled=yes