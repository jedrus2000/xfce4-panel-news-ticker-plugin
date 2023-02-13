#!/usr/bin/env python

import gi
import requests

gi.require_version('Gtk','3.0')
from gi.repository import Gtk

PLUGIN_NAME        = 'Sample-Python-Plugin'
PLUGIN_VERSION     = '0.1.0'
PLUGIN_DESCRIPTION = 'Sample Plugin for Xfce4 Panel in python'
PLUGIN_AUTHOR      = 'Manjeet Singh <itsmanjeet1998@gmail.com>'
PLUGIN_ICON        = 'sample-plugin'

class PanelPlugin(Gtk.Box):
    """
    Xfce4 PanelPlugin,
    This class got called from the C interface of Plugin to embedd the
    resulting python object as Gtk Widget in Xfce4 panel.
    
    Gtk.Box is taken only as example, 
    Any gobject widget can be used as parent class
    """
    def __init__(self) -> None:
        """
        This method is called by sample_py_new() method
        """
        super().__init__()
        #self.my_window = Gtk.Window(attached_to=self,
        #                            default_width=500, is_maximized=True, resizable=False,
        #                            destroy_with_parent=True, accept_focus=False, focus_visible=False, has_resize_grip=False,
        #                            hide_titlebar_when_maximized=True,)
        self.set_size_request(500, -1)
        self.hello_lbl = Gtk.Label(requests.get("https://httpbin.org/uuid").json()["uuid"])
        self.world_lbl = Gtk.Label("World")
        #self.Box = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL, spacing=1)
        self.set_orientation(orientation=Gtk.Orientation.HORIZONTAL)
        self.set_spacing(spacing=1)
        self.pack_start(self.hello_lbl, True, True, 1)
        self.pack_end(self.world_lbl, True, True, 1)

        #while True:
        #    self.ScrolledWindow.do_scroll_child(Gtk.ScrollType.STEP_LEFT, True)
        #    while Gtk.events_pending():
        #        Gtk.main_iteration()

    def free(self):
        """
        Free method called by sample_py_free() when panel sends the "free"
        signal to plugin to clean up the allocations or post tasks
        like saving the configurations etc.
        """
        print("cleaning plugin from python side")


    def orientation_changed(self, orientation: int):
        """
        When the panel orientation changes then it emits a signal of
        orientation changed to all child plugins with current orientation

        Parameters:
            orientation (int): current orientation of plugin
                               0 = Gtk.Orientation.HORIZONTAL
                               1 = Gtk.Orientation.VERTICAL
        """
        self.Box.set_orientation(orientation)

    def about(self):
        """
        Xfce4 panel emit "about" signal whenever user request the information
        about the plugin from right-click-menu
        """
        dialog = Gtk.AboutDialog()
        dialog.set_title("About Dialog")
        dialog.set_program_name(PLUGIN_NAME)
        dialog.set_version(PLUGIN_VERSION)
        dialog.set_comments(PLUGIN_DESCRIPTION)
        dialog.set_website("https://your.plugin/website")
        dialog.set_authors([PLUGIN_AUTHOR])
        dialog.set_logo_icon_name(PLUGIN_ICON)

        dialog.connect('response', lambda dialog, data: dialog.destroy())
        dialog.show_all()