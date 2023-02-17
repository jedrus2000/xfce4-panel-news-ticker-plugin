use std::time::Duration;
use glib_sys::gboolean;
use crate::state::{State, StateEvent};
use crate::gui::{Gui, GuiEvent};
// use crate::config::{Config, ConfigEvent};
// use crate::feed::{Feed, FeedEvent};
use crate::xfce::ffi::XfcePanelPluginPointer;
use crate::xfce::ffi::xfce_panel_plugin_save_location;
use crate::xfce::plugin::XfcePanelPlugin;

pub enum AppEvent {
    Init,
    Ticker,
    StateEvent(StateEvent),
    GuiEvent(GuiEvent),
    /*
    ConfigEvent(ConfigEvent),
    FeedEvent(FeedEvent),
    */
}

pub struct App {
    pub tx: glib::Sender<AppEvent>,
    pub state: State,
    pub gui: Gui,
    pub counter: i32,
    /*
    pub config: Config,
    pub feed: Feed
    */
}

impl App {
    pub fn new (pointer: XfcePanelPluginPointer, tx: glib::Sender<AppEvent>) -> Self {
        let gui = Gui::new(pointer, tx.clone());
        let state = State::new();
        // let config = Config::new();
        // let feed = Feed::new();
        let mut counter = 0;
        return App {
            tx,
            state,
            gui,
            counter,
            /*
            config,
            feed*/
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
                let tx = self.tx.clone();
                if self.counter == 0 {
                    tx.send(AppEvent::GuiEvent(GuiEvent::CreateTickerContent));
                }
                self.counter += 1;
                // eprintln!("Tick ! {}", self.counter);
                tx.send(AppEvent::GuiEvent(GuiEvent::MoveTicker));
                glib::timeout_add_local_once(Duration::from_millis(66), move || {
                    tx.send(AppEvent::Ticker);
                });
            }
            AppEvent::Init => self.init()
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
        rx.attach(None, move |event| {
            app.reducer(event);
            glib::Continue(true)
        });
    }

}
