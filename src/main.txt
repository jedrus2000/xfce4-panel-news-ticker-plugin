use rss::Channel;

fn main () {
    let content = reqwest::blocking::get("https://rss.app/feeds/nCUE2hocDXI0wsUt.xml");

    let channel = match content {
        Err(err) => {
            Err(("ErrorType::UrlRequestError(err)"))
        }
        Ok(content) => {
            rss::Channel::read_from(std::io::BufReader::new(content))
                .map_err(|err| "Cos")
        }
    };
    let rss = channel.unwrap();
    for item in rss.items {
        println!("{:?}", item.link.unwrap());
    }

}