
use gtk::prelude::*;
use gtk::Container;
use gtk::Widget;
use gdk::EventMask;

#[derive(Shrinkwrap)]
pub struct Ticker {
    #[shrinkwrap(main_field)]
    event_box: gtk::EventBox,
    pub container: gtk::Box
}

pub const WIDTH: i32 = 500;

impl Ticker {
    pub fn new () -> Self {
        let event_box = gtk::EventBox::new();
        let hadj = gtk::Adjustment::new(0 as f64, 0 as f64, 1000 as f64, 1 as f64, 20 as f64, 20 as f64);
        let vadj = gtk::Adjustment::new(0 as f64, 0 as f64, 0 as f64, 0 as f64, 0 as f64, 0 as f64);
        /*
        let container = gtk::ScrolledWindow::builder()
            .hadjustment(&hadj)
            .vadjustment(&vadj)
            .build();
        container.set_size_request(WIDTH, -1);
        container.set_policy(gtk::PolicyType::External,
                             gtk::PolicyType::Automatic);
        container.set_events(gdk::EventMask::BUTTON_PRESS_MASK);
         */
        let container = gtk::Box::new(gtk::Orientation::Horizontal, 1);
        // container.show();
        let my_viewport = gtk::Viewport::builder()
            .hadjustment(&hadj)
            .vadjustment(&vadj)
            .build();
        my_viewport.set_border_width(0);
        // my_viewport.show();
        let my_box = gtk::Box::new(gtk::Orientation::Horizontal, 1);
        // my_box.show();
        let start_box = gtk::Box::new(gtk::Orientation::Horizontal, 1);
        // start_box.show();
        start_box.set_size_request(WIDTH, -1);
        my_box.pack_start(&start_box, true, true, 1);
        // text = GLib.markup_escape_text('<span foreground="blue" style="italic">Test</span>')
        let labels = ["jeden1111111111111", "dwa", "trzy3333333333333_E", "cztery4444444444444_E", "piec55555555555555_E"];
        labels.map(|s| {
            let my_label = gtk::Label::new(Some("aaaaaa")); // Some(s));
            // my_label.show();
            /*
            my_label.set_events(Gdk.EventMask.ENTER_NOTIFY_MASK)
            my_label.connect("enter-notify-event", self.on_label_enter)
            my_label.connect("leave-notify-event", self.on_label_leave)
            */
            my_box.pack_start(&my_label, true, true, 1);
        });
        let end_box = gtk::Box::new(gtk::Orientation::Horizontal, 1);
        end_box.set_size_request(WIDTH, -1);
        // end_box.show();
        my_box.pack_end(&end_box, true, true, 1);
        /*
        my_viewport.add(&my_box);
        container.add(&my_viewport);
         */
        container.pack_start(&my_box, true, true, 1);
        event_box.add(&container);
        event_box.show_all();

        Ticker {
            event_box,
            container
        }
    }

    pub fn as_widget (&self) -> &gtk::EventBox {
        &self.event_box
    }
}
