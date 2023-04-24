
use crate::app::{App, AppEvent};
use crate::state::{State, ErrorType, StateEvent};
use glib::SourceId;
use glib::translate::{FromGlib, IntoGlib};
use futures::future::join_all;
use tokio;
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
        // eprintln!("{:?}", content);
        let channel = match content {
            Err(err) => {
                Err("ErrorType::UrlRequestError(err)")
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
    pub fn fetch_feed(state: &State) -> Self {
        let inputs = vec!["https://rsshub.app/telegram/channel/KyivIndependent_official"];
        let mut results: Vec<Channel> =
            tokio::runtime::Runtime::new()
                .unwrap()
                .block_on(async {
                    let mut futures = vec![];
                    for input in inputs {
                        futures.push(Feed::get_rss_from_url(input));
                    }
                    join_all(futures).await
                });
        // eprintln!("{:?}", results);
        let mut feed = Feed::new(state);
        for channel in results {
            for item in channel.items {
                feed.items.push(item)
            }
        }
        feed
    }

}
