
use crate::app::{App, AppEvent};
use crate::state::{State, ErrorType, StateEvent};
use glib::SourceId;
use glib::translate::{FromGlib, IntoGlib};
use futures::future::join_all;
use rss::Channel;
/*
use crate::config::Config;
*/


pub enum ReasonForStopping {
    // CleanStop,
    // Restart,
    FaultyStop
}

pub enum FeedEvent {
    Fetch,
    Fetched(Option<(Feed)>),
}

pub struct Feed {
    pub items: Vec<rss::Item>
}

impl Feed {
    pub fn new (state: &State) -> Self {
        if let Some(ErrorType::InvalidFeedUrl) = state.error  {

        }
        return Feed {
            items: Vec::default()
        }
    }

    async fn get_rss_from_url(url: &str) -> rss::Channel {
        let content = reqwest::get(url).await;

        let channel = match content {
            Err(err) => {
                Err(("ErrorType::UrlRequestError(err)"))
            }
            Ok(content) => {
                let bytes = content.bytes().await.unwrap();
                let cursor = std::io::Cursor::new(bytes);
                rss::Channel::read_from(std::io::BufReader::new(cursor))
                    .map_err(|err| "Cos")
            }
        };
        channel.unwrap()
    }
    pub fn fetch_feed(app: &mut App) {
        let inputs = vec!["https://rss.app/feeds/nCUE2hocDXI0wsUt.xml"];
        let mut results: Vec<Channel> = vec![];
        async {
            let mut futures = vec![];
            for input in inputs {
                futures.push(Feed::get_rss_from_url(input));
            }
            results = join_all(futures).await;
        };
        println!("Results: {:?}", results);

        /*
        if !app.config.active {
            return;
        }
        // TODO: Make polling non-blocking.
        let channel = Feed::read_channel(&app.config);
        if let Ok(channel) = channel {
            let items = channel.items();
            let mut new_items: Vec<rss::Item> = items.iter().filter(|item| {
                !app.feed.all_ids.contains(item.guid().unwrap())
            }).map(|item| item.clone()).collect();

            if app.config.preserve_items {
                new_items.iter().for_each(|item| {
                    app.feed.all_ids.push(item.guid().unwrap().clone());
                });
            } else {
                app.feed.all_ids = items.iter().map(|item| item.guid().unwrap().clone()).collect();
            }
            app.feed.unseen_ids = app.feed.unseen_ids.iter().filter(|id| {
                app.feed.all_ids.contains(id)
            }).chain(new_items.iter().map(|item| item.guid().unwrap())).map(|id| id.clone()).collect();

            if new_items.len() > 0 {
                if app.config.preserve_items {
                    app.feed.items.append(&mut new_items);
                } else {
                    app.feed.items = Vec::from(items);
                }
                app.dispatch(AppEvent::FeedEvent(
                    FeedEvent::Polled(Some(()))
                ));
            } else {
                app.dispatch(AppEvent::FeedEvent(
                    FeedEvent::Polled(None)
                ));
            }
        } else if let Err(err) = channel {
            if app.feed.is_polling {
                Feed::stop_polling(app, ReasonForStopping::FaultyStop);
            }
            app.dispatch(AppEvent::StateEvent(StateEvent::Error(err)));
        }

         */
    }

}
