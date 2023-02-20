use gtk::prelude::*;

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
        let mut viewport = gtk::Viewport::builder()
            .border_width(0)
            .width_request(width)
            .build();
        container.add(&viewport);
        event_box.add(&container);
        event_box.show_all();
        Ticker {
            event_box,
            container,
            viewport
        }
    }

    pub fn create_ticker_content(viewport : &gtk::Viewport, width: i32, items: &Vec<rss::Item>) {
        for child in viewport.children() {
            unsafe {
                child.destroy();
            }
        }
        let my_box = gtk::Box::new(gtk::Orientation::Horizontal, 1);
        let start_box = gtk::Box::new(gtk::Orientation::Horizontal, 1);
        start_box.set_size_request(width, -1);
        my_box.pack_start(&start_box, true, true, 1);
        // text = GLib.markup_escape_text('<span foreground="blue" style="italic">Test</span>')
        for item in items {
            let title = item.title.clone().unwrap();
            eprintln!("{:?}", title);
            let my_label = gtk::Label::new(Some(title.as_str()));
            /*
            my_label.set_events(Gdk.EventMask.ENTER_NOTIFY_MASK)
            my_label.connect("enter-notify-event", self.on_label_enter)
            my_label.connect("leave-notify-event", self.on_label_leave)
            */
            my_box.pack_start(&my_label, true, true, 1);
        };
        let end_box = gtk::Box::new(gtk::Orientation::Horizontal, 1);
        end_box.set_size_request(width, -1);
        // end_box.show();
        my_box.pack_end(&end_box, true, true, 1);
        viewport.add(&my_box);
        viewport.show_all();
    }

    pub fn as_widget (&self) -> &gtk::EventBox {
        &self.event_box
    }

}
