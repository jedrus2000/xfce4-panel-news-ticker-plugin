
#ifndef __PLUGIN_H__
#define __PLUGIN_H__

void constructor(XfcePanelPlugin *plugin) {
    rust_constructor(plugin);
}

#endif
