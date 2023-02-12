/*
 *	Very basic xlib prog
 *	Compile with: gcc -o xlib-prog xlib-prog.c -lX11
 */

#include <X11/Xlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
	Display		*displ;
	int		screen_num;
	Window		win;
	unsigned int	displ_w;
	unsigned int	displ_h;
	unsigned int	win_x;
	unsigned int	win_y;
	unsigned int	win_w;
	unsigned int	win_h;
	unsigned int	win_border_w;
	char		*displ_name = getenv("DISPLAY");

	if ((displ = XOpenDisplay(displ_name)) != NULL) {
		fprintf(stdout, "This window will stay for 5 sec\n");

		screen_num = DefaultScreen(displ);
		displ_w = DisplayWidth(displ, screen_num);
		displ_h = DisplayHeight(displ, screen_num);

		win_x = displ_w / 4;
		win_y = displ_h / 4;
		win_w = displ_w / 2;
		win_h = displ_h / 2;
		win_border_w = 2;

		fprintf(stdout, "win_x = %d, win_y = %d, win_w = %d, win_h = %d\n",
			win_x, win_y, win_w, win_h);

		win = XCreateSimpleWindow(
			displ, RootWindow(displ, screen_num),
			win_x, win_y, win_w, win_h, win_border_w,
			BlackPixel(displ, screen_num),
			WhitePixel(displ, screen_num));

		XMapWindow(displ, win);
		XSync(displ, False);

		/* Need this to get the correct win position */
		XMoveWindow(displ, win, win_x, win_y);
		XSync(displ, False);

		sleep(5);
		XCloseDisplay(displ);
		return 0;
	} else {
		fprintf(stderr, "%s: Can't connect to X server '%s'\n",
			argv[0], displ_name);
		return 1;
	}
}
