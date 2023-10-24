extern crate cc;

fn main() {
    cc::Build::new()
        .file("src/plugin.c")
        .include("/usr/include/xfce4/libxfce4panel-2.0")
        .include("/usr/include/gtk-3.0")
        .include("/usr/include/pango-1.0")
        .include("/usr/include/glib-2.0")
        .include("/usr/lib/glib-2.0/include")
        .include("/usr/include/sysprof-4")
        .include("/usr/include/harfbuzz")
        .include("/usr/include/freetype2")
        .include("/usr/include/libpng16")
        .include("/usr/include/libmount")
        .include("/usr/include/blkid")
        .include("/usr/include/fribidi")
        .include("/usr/include/cairo")
        .include("/usr/include/pixman-1")
        .include("/usr/include/gdk-pixbuf-2.0")
        .include("/usr/include/gio-unix-2.0")
        .include("/usr/include/cloudproviders")
        .include("/usr/include/atk-1.0")
        .include("/usr/include/at-spi2-atk/2.0")
        .include("/usr/include/at-spi-2.0")
        .include("/usr/include/dbus-1.0")
        .include("/usr/lib/dbus-1.0/include")
        .include("/usr/include/xfce4")
        .compile("plugin");
}
