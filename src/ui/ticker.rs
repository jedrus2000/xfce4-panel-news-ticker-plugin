use std::rc::Rc;
use std::cell::RefCell;
use std::sync::Mutex;
use std::boxed::Box;
use std::time::Duration;
use glib;
use gtk::prelude::*;
use gtk::Adjustment;
use gtk::Container;
use gtk::Widget;
use gdk::EventMask;

#[derive(Shrinkwrap)]
pub struct Ticker {
    #[shrinkwrap(main_field)]
    event_box: gtk::EventBox,
    pub container: gtk::ScrolledWindow,
    pub viewport: gtk::Viewport
}

impl Ticker {
    pub fn new (width: i32) -> Self {
        let event_box = gtk::EventBox::new();
        let container = gtk::ScrolledWindow::builder()
            .width_request(width)
            .events(gdk::EventMask::BUTTON_PRESS_MASK)
            .hscrollbar_policy(gtk::PolicyType::External)
            .vscrollbar_policy(gtk::PolicyType::Automatic)
            .build();
        let viewport = gtk::Viewport::builder()
            .border_width(0)
            .width_request(width)
            .build();
        Ticker::create_ticker_content(&viewport, width);
        container.add(&viewport);
        event_box.add(&container);
        event_box.show_all();
        Ticker {
            event_box,
            container,
            viewport
        }
    }

    pub fn create_ticker_content(viewport : &gtk::Viewport, width: i32) {
        let my_box = gtk::Box::new(gtk::Orientation::Horizontal, 1);
        let start_box = gtk::Box::new(gtk::Orientation::Horizontal, 1);
        start_box.set_size_request(width, -1);
        my_box.pack_start(&start_box, true, true, 1);
        // text = GLib.markup_escape_text('<span foreground="blue" style="italic">Test</span>')
        let labels = ["jeden1111111111111", "dwa", "trzy3333333333333_E", "cztery4444444444444_E", "piec55555555555555_E"];
        labels.map(|s| {
            let my_label = gtk::Label::new(Some(s));
            /*
            my_label.set_events(Gdk.EventMask.ENTER_NOTIFY_MASK)
            my_label.connect("enter-notify-event", self.on_label_enter)
            my_label.connect("leave-notify-event", self.on_label_leave)
            */
            my_box.pack_start(&my_label, true, true, 1);
        });
        let end_box = gtk::Box::new(gtk::Orientation::Horizontal, 1);
        end_box.set_size_request(width, -1);
        // end_box.show();
        my_box.pack_end(&end_box, true, true, 1);
        viewport.add(&my_box);
    }

    pub fn as_widget (&self) -> &gtk::EventBox {
        &self.event_box
    }

}
