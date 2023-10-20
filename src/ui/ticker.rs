use gdk::EventMask;
use gdk::keys::constants::f;
use glib::translate::{from_glib, ToGlibPtr};
use gtk::prelude::*;
use gtk::ResizeMode;
use libc::c_double;
use webkit2gtk::{JavascriptResult, LoadEvent, traits::{SettingsExt, WebContextExt, WebViewExt}, WebContext, WebView, WindowPropertiesExt};
use webkit2gtk::ffi::{webkit_settings_new, WebKitSettings, WebKitSettingsClass};
use crate::app;
use crate::app::AppEvent;

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
        let my_box = gtk::Box::new(gtk::Orientation::Horizontal, 1);
        let start_box = gtk::Box::new(gtk::Orientation::Horizontal, 1);
        start_box.set_size_request(*width, -1);
        my_box.pack_start(&start_box, true, true, 1);

        // text = GLib.markup_escape_text('<span foreground="blue" style="italic">Test</span>')
        for item in items {
            let title = format!("<span>     </span><a href=\"{}\" title=\"\">{}</a>",
                item.link.clone().unwrap(), item.title.clone().unwrap() );
            eprintln!("{:?}", item);
            let my_label = gtk::Label::builder()
                .use_markup(true)
                .single_line_mode(true)
                .build();
            my_label.set_markup(title.as_str());
            my_label.set_track_visited_links(true);
            my_label.set_events(EventMask::ENTER_NOTIFY_MASK | EventMask::LEAVE_NOTIFY_MASK);

            let text = format!("<!DOCTYPE html><html><body>{}</body></html>", item.description.clone().unwrap());
            // let tx_tooltip = app.tx.clone();
            my_label.connect_query_tooltip(move |label, x, y, keyboard_mode, tooltip| {
                // let tx_inner = tx_tooltip.clone();
                let webview =
                    WebView::new();
                let settings = WebViewExt::settings(&webview).unwrap();
                settings.set_default_font_size(10);
                settings.set_enable_accelerated_2d_canvas(true);
                tooltip.set_custom(Some(&webview));
                webview.connect_load_changed(move |webview, load_event| {
                    if load_event == LoadEvent::Finished {
                        // eprintln!("Stop becouse of tooltip");
                        // tx_inner.send(AppEvent::StopMoving);
                        let mut width : i32 = 500;
                        let mut height : i32 = 200;

                        let js = "document.body.scrollHeight;";
                        webview.run_javascript(js, gio::Cancellable::NONE, move |result| {
                            if let Ok(js_value) = result {
                                height = js_value.js_value().unwrap().to_string().parse::<i32>().unwrap();
                                // eprintln!("New Height: {:?}", height);
                            }
                        });
                        let js = "document.body.scrollWidth;";
                        webview.run_javascript(js, gio::Cancellable::NONE, move |result| {
                            if let Ok(js_value) = result {
                                width = js_value.js_value().unwrap().to_string().parse::<i32>().unwrap();
                                // eprintln!("New Width: {:?}", width);
                            }
                        });

                        webview.set_size_request(width, height);
                        // eprintln!("width = {}, height = {}", width, height);
                    }
                });
                webview.load_html(text.as_str(), None);
                webview.set_margin(0);
                true
            });

            let tx_leave_notify = app.tx.clone();
            my_label.connect_leave_notify_event(move |label, _event| {
                // eprintln!("leave");
                tx_leave_notify.send(AppEvent::StartMoving);
                gtk::Inhibit(true)
            });
            let tx_enter_notify = app.tx.clone();
            my_label.connect_enter_notify_event(move | label, _ | {
                // eprintln!("enter");
                tx_enter_notify.send(AppEvent::StopMoving);
                gtk::Inhibit(true)
            });

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
