use gtk;
use gtk::prelude::*;

#[derive(Shrinkwrap)]
pub struct AboutDialog {
    dialog: gtk::AboutDialog
}

impl AboutDialog {
    pub fn new (parent: &gtk::Window) -> Self {
        let dialog = gtk::AboutDialog::builder()
            .title("About")
            .version(crate::res::VERSION)
            .program_name(crate::res::APP_NAME)
            .authors(crate::res::AUTHORS
                .to_string()
                .split(':')
                .collect::<Vec<&str>>()
            )
            .window_position(gtk::WindowPosition::Center)
            .transient_for(parent)
            .destroy_with_parent(true)
            .logo_icon_name("newstickerplugin")
            .icon_name("xfce4-about")
            .build();
        dialog.add_credit_section("Thanks to:",crate::res::THANKS_TO);
        unsafe {
            dialog.connect_response(|dialog, _| dialog.destroy());
        }
        dialog.show();
        AboutDialog {
            dialog
        }
    }
}
