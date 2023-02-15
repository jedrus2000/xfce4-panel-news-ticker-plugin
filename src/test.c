public partial class MainWindow : Gtk.Window
{
    // viewport which we use to contain the label and handle the scrolling of the marquee.
    private Viewport vpMarquee = new Viewport();

    // marquee text should have some padding before and after the text, to avoid rendering errors, and
    // to allow the text to "flow in" when it is reset after reaching the end.
    private Label lblMarquee = new Label("                                                                                                 [lblMarquee] Lorem ipsum dolor sit amet, consectetur adipiscing elit. Donec metus quam, ullamcorper eu suscipit quis, rutrum sit amet massa.                                                                                                 ");//" Nunc dapibus accumsan metus, commodo placerat urna blandit non. Nullam turpis justo, dictum quis dignissim non, vehicula vel lorem. Fusce congue purus odio, lobortis pulvinar neque. Integer dui odio, venenatis sed tincidunt in, fringilla id urna. Maecenas faucibus massa eu orci tincidunt ac vulputate nibh aliquam. Aliquam ullamcorper erat nunc. Vestibulum hendrerit adipiscing neque quis interdum.");

    public MainWindow () : base(Gtk.WindowType.Toplevel)
    {
        // normally you'd be using Stetic, so you need to call it's build-constructor.
        Build ();        // add the control to the window.  in this example, i'm adding it to a VBox control as the last element.
        vpMarquee.BorderWidth = 0;
        vpMarquee.Add (lblMarquee);
        this.vbox1.PackEnd (vpMarquee, false, false, 0);

        // update marquee every 33ms (~30fps)
        GLib.Timeout.Add (33, new GLib.TimeoutHandler (MarqueeUpdate));      // add other code here for other things...
    }
    bool MarqueeUpdate () {
        Application.Invoke (delegate {
            // calculate the total amount of space the marquee requires.  this only works because I've manually set the size of the window,
            // otherwise you need another way to get the width of the window instead of this.
            double n = vpMarquee.Hadjustment.Upper - this.WidthRequest;
            if (n < 1)
                // safety incase we don't have an actual width calculated width
                n = 1;

            //Console.WriteLine("{0} > {1} ?", vpMarquee.Hadjustment.Value, n);
            if (vpMarquee.Hadjustment.Value >= n) {
                // we've reached the end.  reset the marquee to the 0-position.
                vpMarquee.Hadjustment.Value = 0;
            } else {
                // scroll the marquee to the left by 3px
                vpMarquee.Hadjustment.Value += 3;
            }

            // tell gtk# we want it to take into account the updated value.
            vpMarquee.Hadjustment.ChangeValue ();
            vpMarquee.Hadjustment.Change ();

            // redraw the control and it's children (the label).
            vpMarquee.ShowAll ();
        });

        // tell Timeout we want to be called again.
        return true;
    }    // ... add more stuff here for other functionality.
}