#!/bin/bash
# Take an emulator screenshot into assets/<emulator>_<n>.png, numbered after the
# ones already there. Pass a number as the second argument to override it.
if [ -z "$1" ]; then
    echo "Usage: $0 <emulator_name> [screenshot_number]"
    exit 1
fi
last=$(ls -1 assets/"$1"_*.png 2>/dev/null | sed 's/.*_\([0-9]*\)\.png$/\1/' | sort -n | tail -1)
pebble screenshot "assets/$1_${2:-$((last + 1))}.png" --emulator "$1"
