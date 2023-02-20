use std::time::Duration;
use crate::state::{State, StateEvent};
use crate::gui::{Gui, GuiEvent};
// use crate::config::{Config, ConfigEvent};
use crate::feed::{ Feed, FeedEvent};
use crate::feed::FeedEvent::Fetch;
use crate::xfce::ffi::{
    XfcePanelPluginPointer,
    xfce_panel_plugin_save_location
};
use crate::xfce::plugin::XfcePanelPlugin;
use crate::ui::ticker;

pub enum AppEvent {
    Init,
    Ticker,
    StateEvent(StateEvent),
    GuiEvent(GuiEvent),
    /*
    ConfigEvent(ConfigEvent),
    */
    FeedEvent(FeedEvent),
}

pub struct App {
    pub tx: glib::Sender<AppEvent>,
    pub state: State,
    pub gui: Gui,
    pub counter: i32,
    pub stop: bool,
    /*
    pub config: Config,
    */
    pub feed: Feed
}

impl App {
    pub fn new (pointer: XfcePanelPluginPointer, tx: glib::Sender<AppEvent>) -> Self {
        let gui = Gui::new(pointer, tx.clone());
        let state = State::new();
        // let config = Config::new();
        let feed = Feed::fetch_feed(&state);
        ticker::Ticker::create_ticker_content(&gui.ticker.viewport, 500, &feed.items);
        return App {
            tx,
            state,
            gui,
            counter: 0,
            stop: false,
            feed,
            /*
            config,*/
        }
    }

    pub fn reducer(&mut self, event: AppEvent) {
        match event {
            AppEvent::StateEvent(_) => {
                State::reducer(self, event);
            }
            AppEvent::GuiEvent(_) => {
                Gui::reducer(self, event);
            }
            /*
            AppEvent::ConfigEvent(_) => {
                Config::reducer(self, event);
            }
            AppEvent::FeedEvent(_) => {
                Feed::reducer(self, event);
            }
             */
            AppEvent::Ticker => {
                if self.stop { return; }
                let tx = self.tx.clone();
                if self.counter == 0 {
                    let items = self.feed.items.clone();
                    tx.send(AppEvent::GuiEvent(GuiEvent::CreateTickerContent(items)));
                }
                self.counter += 1;
                // eprintln!("Tick ! {}", self.counter);
                tx.send(AppEvent::GuiEvent(GuiEvent::MoveTicker));
                glib::timeout_add_local_once(Duration::from_millis(66), move || {
                    tx.send(AppEvent::Ticker);
                });
            }
            AppEvent::FeedEvent(Fetch) => {
                let tx = self.tx.clone();
                let state = State::new();
                glib::timeout_add_local_once(Duration::from_secs(600), move || {
                    let f = Feed::fetch_feed(&state);
                    tx.send(AppEvent::FeedEvent(FeedEvent::Fetched(Some(f))));
                    tx.send(AppEvent::FeedEvent(Fetch));
                });

            }
            AppEvent::FeedEvent(FeedEvent::Fetched(result)) => {
                self.feed = result.unwrap();
            }
            AppEvent::Init => self.init(),
            _ => {}
        };
        // Handle when to update?
        Gui::update(self);
    }

    fn init(&mut self) {
        /*
         Config::init(self);
         */
        Gui::init(self);
        /*
        self.dispatch(AppEvent::FeedEvent(FeedEvent::Start));
         */
    }

    pub fn dispatch(&mut self, event: AppEvent) {
        if let Err(_error) = self.tx.send(event) {
            self.state.error = Some(crate::state::ErrorType::CouldNotDispatch);
        }
    }

    pub fn start (pointer: XfcePanelPluginPointer) {
        let (tx, rx) = glib::MainContext::channel(glib::PRIORITY_DEFAULT);
        let mut app = App::new(pointer, tx.clone());
        let plugin = XfcePanelPlugin::from(pointer);
        eprintln!("{}", plugin.save_location(true).unwrap());
        app.dispatch(AppEvent::Init);
        app.dispatch(AppEvent::Ticker);
        app.dispatch(AppEvent::FeedEvent(Fetch));
        rx.attach(None, move |event| {
            app.reducer(event);
            glib::Continue(true)
        });
    }

}
