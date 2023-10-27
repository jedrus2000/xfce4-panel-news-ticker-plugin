
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
                Err(format!("Failed to connect site. Error: {}", err))
            }
            Ok(content) => {
                let bytes = content.bytes().await.unwrap();
                let cursor = std::io::Cursor::new(bytes);
                rss::Channel::read_from(std::io::BufReader::new(cursor))
                    .map_err(|err| {
                        format!("Failed to read feed content for {}", url)
                    })
            }
        };
        match channel {
            Err(err) => {
                let mut ch = rss::Channel::default();
                let item = rss::Item {
                    title: Some(String::from(err)),
                    link: Some(String::from("")),
                    description: None,
                    author: None,
                    categories: vec![],
                    comments: None,
                    enclosure: None,
                    guid: None,
                    pub_date: None,
                    source: None,
                    content: None,
                    extensions: Default::default(),
                    itunes_ext: None,
                    dublin_core_ext: None,
                };
                ch.items.push(item);
                ch
            }
            Ok(ch) => {
                ch
            }
        }
    }
    pub fn fetch_feed(state: &State) -> Self {
        let inputs = vec!["https://www.ukrinform.net/rss/block-lastnews"]; // https://uaposition.com/feed/
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
