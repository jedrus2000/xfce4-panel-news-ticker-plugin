
use gtk::{Widget};
use gtk::prelude::WidgetExtManual;

use crate::app::{App, AppEvent};
use crate::state::{StateEvent};
use crate::feed::{FeedEvent};
use crate::ui::config_dialog::ConfigDialog;

use crate::xfce::rc::*;

pub enum ConfigEvent {
    DialogResponse(gtk::ResponseType),
    Loaded(bool),
    // Save,
    Saved(bool)
}

#[derive(Debug, Clone)]
pub struct FeedUrl {
    pub url: String,
    pub polling_interval: u32, // minutes
}

#[derive(Debug, Clone)]
pub struct Config {
    pub feeds_urls: Vec<FeedUrl>,
    pub ticker_speed: u32, // pixels
    pub width: i32,
    pub save_location: Option<String>,
}

impl Default for Config {
    fn default () -> Self {
        Config {
            feeds_urls: Vec::new(),
            ticker_speed: 1, // pixels
            save_location: None,
            width: 1920
        }
    }
}

impl Config {
    pub fn new() -> Self {
        Config::default()
    }

    pub fn reducer(app: &mut App, event: AppEvent) {
        if let AppEvent::ConfigEvent(event) = event {
            match event {
                ConfigEvent::DialogResponse(gtk::ResponseType::Accept) => {
                    Config::save_and_close_config_dialog(app);
                }
                _ => {}
            }
        }
    }
    
    pub fn init(app: &mut App) {
        app.config.save_location = app.gui.plugin.save_location(true);
        Config::load(app);
    }

    pub fn load(app: &mut App) {
        if let Some(rc_file) = &app.config.save_location {
            let (feeds_urls, ticker_speed) = rc_simple(rc_file, |rc| {
                (
                    read_list_entry(rc, "feed_urls", ";\n"),
                    std::convert::TryInto::try_into(read_int_entry(rc, "ticker_speed", 1)).unwrap(),
                )
            });
            app.config.ticker_speed = ticker_speed;
            app.config.feeds_urls = feeds_urls.iter().map(|s| {
                let offset = s.find(',').unwrap_or(s.len());
                let (url, polling_interval) = s.split_at(offset);
                FeedUrl {url: String::from(url), polling_interval: polling_interval.parse().unwrap_or(30)}
            }).collect();
            app.dispatch(AppEvent::ConfigEvent(
                ConfigEvent::Loaded(true)
            ));
        } else {
            app.dispatch(AppEvent::ConfigEvent(
                ConfigEvent::Loaded(false)
            ));
        }
    }

    pub fn save(app: &mut App) -> bool {
        if let Some(rc_file) = &app.config.save_location {
            rc_simple_mut(rc_file, |rc| {
                write_int_entry(rc, "ticker_speed", std::convert::TryInto::try_into(app.config.ticker_speed).unwrap());
                let urls: Vec<String> = app.config.feeds_urls.iter().map(|feed| format!("{},{}",feed.url, feed.polling_interval)).collect();
                write_list_entry(rc, "feed_headers", urls,";\n");
            });
            app.dispatch(AppEvent::ConfigEvent(ConfigEvent::Saved(true)));
            return true;
        } else {
            app.dispatch(AppEvent::ConfigEvent(ConfigEvent::Saved(false)));
            return false;
        }
    }

    pub fn set_from_dialog(&mut self, dialog: &ConfigDialog) -> Result<(),crate::state::ErrorType> {
        self.active = dialog.get_active();
        self.preserve_items = dialog.get_preserve_items();
        let feed = dialog.get_feed();
        // TODO: If new feed, dispatch FeedEvent::Clear
        if is_feed_valid(&feed) {
            self.feed = feed;
        } else {
            self.active = false;
            return Err(crate::state::ErrorType::InvalidFeedUrl);
        }
        self.polling_interval = dialog.get_polling_interval().unwrap_or(self.polling_interval);
        self.feed_headers = dialog.get_headers();
        self.feed_request_method = dialog.get_request_method();
        self.feed_request_body = dialog.get_request_body(&self.feed_request_method);
        return Ok(());
    }

    fn save_and_close_config_dialog(app: &mut App){
        /*
        if let Some(dialog) = &app.gui.config_dialog {
            if let Err(error) = app.config.set_from_dialog(&dialog) {
                app.dispatch(AppEvent::StateEvent(
                    StateEvent::Error(error)
                ));
                return;
            }
            app.dispatch(AppEvent::StateEvent(StateEvent::ClearError));
            let ok = Config::save(app);
            if !ok {
                // TODO: Handle
            }
            Config::close_config_dialog(app);
            if app.config.active {
                app.dispatch(AppEvent::FeedEvent(FeedEvent::Start));
                app.dispatch(AppEvent::FeedEvent(FeedEvent::Poll));
            }
        } else {
            // TODO: Error
            return;
        }
         */
    }

    fn close_config_dialog(app: &mut App) {
        /*
        if let Some(dialog) = &app.gui.config_dialog {
            unsafe { dialog.destroy(); }
        }
         */
    }
}

fn is_feed_valid(feed: &str) -> bool {
    return feed.len() >= 5;
}
