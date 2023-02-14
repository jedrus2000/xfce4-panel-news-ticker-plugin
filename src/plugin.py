#!/usr/bin/env python

import gi
import requests
import cairo

gi.require_version('Gtk','3.0')
from gi.repository import Gtk, GLib, Pango, PangoCairo

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
        self.set_size_request(200, -1)
        self.set_policy(Gtk.PolicyType.EXTERNAL,
                Gtk.PolicyType.AUTOMATIC)
        self.my_viewport = Gtk.Viewport()
        self.my_viewport.set_border_width(0)
        # self.my_viewport.set_size_request(1000, 20)
        self.my_label = Gtk.Label("                                                                                                 [lblMarquee] Lorem ipsum dolor sit amet, consectetur adipiscing elit. Donec metus quam, ullamcorper eu suscipit quis, rutrum sit amet massa.                                                                                                 end......")
        self.my_viewport.add(self.my_label)
        #self.my_viewport.set_hexpand(False)
        #hadjustment = self.my_layout.get_hadjustment()
        #self.set_hadjustment(hadjustment)
        #vadjustment =  self.my_layout.get_vadjustment()
        #self.set_vadjustment(vadjustment)
        #self.my_test_label = Gtk.Label("Test label")
        #self.my_viewport.add(self.my_label)

        #
        self.add(self.my_viewport)
        self.counter = 0
        self.countdown_event = GLib.timeout_add(133, self.my_scroll_callback)
        #while True:
        #    self.ScrolledWindow.do_scroll_child(Gtk.ScrollType.STEP_LEFT, True)
        #    while Gtk.events_pending():
        #        Gtk.main_iteration()

    def my_scroll_callback(self):
        #self.counter += 3
        #self.my_label.set_text(f"Counter: {self.counter}")

        h_adj: Gtk.Adjustment = self.my_viewport.get_hadjustment()
        h_adj.freeze_notify()
        n = h_adj.get_upper() - 200
        if n < 1:
            # safety incase we don't have an actual width calculated width
            n = 1

        if self.counter >= n:
            # we've reached the end.  reset the marquee to the 0-position.
            self.counter = 0
        else:
            # scroll the marquee to the left by 3px
            self.counter += 3

        k = int(1600 / points_to_pixels(self.my_label.get_style_context().get_font(Gtk.StateFlags.NORMAL))[1].height)
        s = k * " "
        # s = ' ' * int(400 / (self.my_label.get_style_context().get_font(Gtk.StateFlags.NORMAL).get_size_is_absolute() .get_size() / Pango.SCALE))

        #self.my_label.set_text(f"{s} {h_adj.get_lower()=} {h_adj.get_upper()=} {h_adj.get_page_size()=}")

        # redraw the control and it's children (the label).
        # h_adj.configure(value=h_adj_value, lower=0, upper=h_adj.get_upper(), step_increment=3, page_increment=3, page_size=3)
        h_adj.set_value(self.counter) # h_adj_value)
        h_adj.thaw_notify()
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

def points_to_pixels(font: Pango.FontDescription):
    surface = cairo.ImageSurface(cairo.FORMAT_ARGB32, 0, 0)
    ctx = cairo.Context(surface)
    layout = PangoCairo.create_layout(ctx)
    layout.set_font_description(font)
    extents = layout.get_pixel_extents()
    return extents