#!/bin/sh
main=newstickerplugin

gcc -Wall -shared -o lib${main}.so -fPIC src/plugin.c\
    -Isrc -L. $PWD/target/debug/librs${main}.so\
    `pkg-config --cflags --libs libxfce4panel-2.0`\
