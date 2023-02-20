#!/bin/sh
main=newstickerplugin

gcc -Wall -shared -o lib${main}.so -fPIC src/plugin.c\
    -Isrc -L. $PWD/target/debug/librs${main}.so\
    `pkg-config --cflags --libs libxfce4panel-2.0`\

#gcc -Wall -shared -o lib${main}.so\
#     -Wl,--whole-archive $PWD/target/debug/librs${main}.a\
#     -Wl,--no-whole-archive -fPIC src/plugin.c -Isrc\
#    `pkg-config --cflags --libs libxfce4panel-2.0`\
