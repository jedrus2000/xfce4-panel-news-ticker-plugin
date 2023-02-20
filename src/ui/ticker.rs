use gtk::prelude::*;
use webkit2gtk::{
    traits::{SettingsExt, WebContextExt, WebViewExt},
    WebContext, WebView,
};
use webkit2gtk::ffi::{webkit_settings_new, WebKitSettings, WebKitSettingsClass};
use crate::app;

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

    pub fn create_ticker_content(app: &app::App, items: &Vec<rss::Item>) {
        let viewport = &app.gui.ticker.viewport;
        let width = &app.config.width;

        for child in viewport.children() {
            unsafe {
                child.destroy();
            }
        }
        let context = WebContext::default().unwrap();

        let my_box = gtk::Box::new(gtk::Orientation::Horizontal, 1);
        let start_box = gtk::Box::new(gtk::Orientation::Horizontal, 1);
        start_box.set_size_request(*width, -1);
        my_box.pack_start(&start_box, true, true, 1);
        my_box.set_events(gdk::EventMask::ENTER_NOTIFY_MASK);
        // text = GLib.markup_escape_text('<span foreground="blue" style="italic">Test</span>')
        for item in items {
            let title = format!("<a href=\"{}\" title=\"\">{}</a>",
                item.link.clone().unwrap(), item.title.clone().unwrap() );
            eprintln!("{:?}", item);
            let my_label = gtk::Label::builder()
                .use_markup(true)
                .events(gdk::EventMask::ENTER_NOTIFY_MASK)
                .events(gdk::EventMask::LEAVE_NOTIFY_MASK)
                // .tooltip_text("test")
                .build();
            my_label.set_markup(title.as_str());
            my_label.set_has_tooltip(true);

            let text = item.description.clone().unwrap().to_owned();
            my_label.connect_query_tooltip(move |label, x, y, keyboard_mode, tooltip| {
                // let window = gtk::Window::new(gtk::WindowType::Popup);
                let webview =
                    WebView::new();
                let settings = WebViewExt::settings(&webview).unwrap();
                settings.set_default_font_size(10);
                webview.load_html(text.as_str(), None);
                // window.add(&webview);
                // window.move_(x, y);
                webview.set_size_request(500, 200);
                // window.show_all();
                webview.show_all();
                tooltip.set_custom(Some(&webview));
                // tooltip.set_custom(Some(&window));
                // eprintln!("Setting tooltip !");
                true
            });
            my_label.connect_leave_notify_event(move |label, _event| {
                println!("Leave notify event received for label {:?}", label);
                gtk::Inhibit(false)
            });
            my_label.connect_enter_notify_event(move |label, _event| {
                println!("Enter notify event received for label {:?}", label);
                gtk::Inhibit(false)
            });
            // my_label.set_label(item.title.clone().unwrap().as_str());
            // my_label.set_tooltip_markup(Some(item.description.clone().unwrap().as_str()));
            /*
            my_label.set_events(Gdk.EventMask.ENTER_NOTIFY_MASK)
            my_label.connect("enter-notify-event", self.on_label_enter)
            my_label.connect("leave-notify-event", self.on_label_leave)
            */
            my_box.pack_start(&my_label, true, true, 1);
        };
        let end_box = gtk::Box::new(gtk::Orientation::Horizontal, 1);
        end_box.set_size_request(*width, -1);
        // end_box.show();
        my_box.pack_end(&end_box, true, true, 1);
        viewport.add(&my_box);
        viewport.show_all();
    }

    pub fn as_widget (&self) -> &gtk::EventBox {
        &self.event_box
    }

}
