#!/usr/bin/env python

import gi
import requests

gi.require_version('Gtk','3.0')
from gi.repository import Gtk, GLib, Pango

PLUGIN_NAME        = 'Sample-Python-Plugin'
PLUGIN_VERSION     = '0.1.0'
PLUGIN_DESCRIPTION = 'Sample Plugin for Xfce4 Panel in python'
PLUGIN_AUTHOR      = 'Manjeet Singh <itsmanjeet1998@gmail.com>'
PLUGIN_ICON        = 'sample-plugin'


class PanelPlugin(Gtk.ScrolledWindow):
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
        self.my_window_width = 100
        #self.my_window = Gtk.Window(attached_to=self,
        #                            default_width=500, is_maximized=True, resizable=False,
        #                            destroy_with_parent=True, accept_focus=False, focus_visible=False, has_resize_grip=False,
        #                            hide_titlebar_when_maximized=True,)
        #self.my_box = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL, spacing=0)
        #self.add(self.my_box)
        self.set_size_request(200, -1)
        self.set_overlay_scrolling(True)
        self.set_policy(Gtk.PolicyType.NEVER,
                        Gtk.PolicyType.AUTOMATIC)
        self.my_layout: Gtk.Layout = Gtk.Layout()
        self.my_layout.set_size(1500, 20)
        self.my_layout.set_vexpand(True)
        self.my_layout.set_hexpand(True)
        self.add(self.my_layout)

        #self.my_viewport.set_border_width(0)
        #self.my_viewport.set_size_request(100, 20)
        #self.my_viewport.set_hexpand(False)
        #self.pack_start(self.my_viewport, False, False, 0)
        self.my_label = Gtk.Label("                                                                                                 [lblMarquee] Lorem ipsum dolor sit amet, consectetur adipiscing elit. Donec metus quam, ullamcorper eu suscipit quis, rutrum sit amet massa.                                                                                                 ")
        #self.my_label.set_width_chars(1000)
        #self.my_label.set_max_width_chars(1000)
        #self.my_label.set_hexpand(True)
        #self.my_label.set_ellipsize(Pango.EllipsizeMode.NONE)
        #self.my_box = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL, spacing=0)
        #self.my_box.pack_end(self.my_label, False, False, 0)
        self.my_layout.put(self.my_label, 0, 0)
        #hadjustment = self.my_layout.get_hadjustment()
        #self.set_hadjustment(hadjustment)
        #vadjustment =  self.my_layout.get_vadjustment()
        #self.set_vadjustment(vadjustment)
        #self.my_test_label = Gtk.Label("Test label")
        #self.my_viewport.add(self.my_label)
        #self.Box = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL, spacing=1)
        #
        # self.my_box.pack_end(self.my_label, False, False, 0)
        self.counter = 0
        self.countdown_event = GLib.timeout_add(33, self.my_scroll_callback)
        #while True:
        #    self.ScrolledWindow.do_scroll_child(Gtk.ScrollType.STEP_LEFT, True)
        #    while Gtk.events_pending():
        #        Gtk.main_iteration()

    def my_scroll_callback(self):
        #self.counter += 3
        #self.my_label.set_text(f"Counter: {self.counter}")
        h_adj: Gtk.Adjustment = self.my_layout.get_hadjustment()
        self.freeze_notify()
        n = h_adj.get_upper() # - self.my_window_width
        if n < 1:
            # safety incase we don't have an actual width calculated width
            n = 1

        if self.counter >= n:
            # we've reached the end.  reset the marquee to the 0-position.
            self.counter = 0
        else:
            # scroll the marquee to the left by 3px
            self.counter += 3

        self.my_label.set_text(f"{self.counter=} {h_adj.get_lower()=} {h_adj.get_upper()=} {h_adj.get_page_size()=}")

        # redraw the control and it's children (the label).
        # h_adj.configure(value=h_adj_value, lower=0, upper=h_adj.get_upper(), step_increment=3, page_increment=3, page_size=3)
        h_adj.set_value(self.counter) # h_adj_value)
        self.thaw_notify()
        # self.my_viewport.show_all()
        return True


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
        self.set_orientation(Gtk.Orientation(orientation))

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