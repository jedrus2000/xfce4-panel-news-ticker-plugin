## XFCE4-Panel News Ticker Plugin

Just begin journey with ~~Rust~~,Python, Gtk, XFCE4 plugins...

### For Python 3.10
If different - edit file: `src/plugin.c` and change loaded lib.

### Install GTK Python stubs
```
pip install pygobject-stubs --config-settings=config=Gtk3,Gdk3,Soup2
```

### Using `Meson` to build
```
 meson compile -C builddir && meson install -C builddir && xfce4-panel -r
```

### Inspirations, docs, some code re-used from : 
- [Xfce4-panel RSS Plugin](https://github.com/siverv/rss-plugin)
- [TICKR - Feed Reader](https://open-tickr.net/index.php)
- https://wiki.xfce.org/dev/howto/panel_plugins
- https://gitlab.xfce.org/itsManjeet/sample-python-plugin

