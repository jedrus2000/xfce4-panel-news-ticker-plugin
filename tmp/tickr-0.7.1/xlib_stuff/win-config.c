/*
 *	(Re)Configure xwin
 *	Compile with: gcc -o win-config win-config.c -lX11
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
		/* TODO: Use XConfigureWindow() / XReconfigureWMWindow() */

		screen_num = DefaultScreen(displ);
		displ_w = DisplayWidth(displ, screen_num);
		displ_h = DisplayHeight(displ, screen_num);

		XSync(displ, False);

		XCloseDisplay(displ);
		return 0;
	} else {
		fprintf(stderr, "%s: Can't connect to X server '%s'\n",
			argv[0], displ_name);
		return 1;
	}
}
