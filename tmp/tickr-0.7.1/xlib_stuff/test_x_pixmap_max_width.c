/*
 *	Trying to determine max width of a X pixmap. If signed 16-bit int,
 *	should be 32767.
 *
 *	Copyright (C) Emmanuel Thomas-Maurin 2016-2021  <manutm007@gmail.com>
 *
 *	Compile with: gcc -o test_x_pixmap_max_width test_x_pixmap_max_width.c -lX11
 */

#include <X11/Xlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#define	FPRINTF(...) \
	{fprintf(stdout, __VA_ARGS__); fflush(stdout);}

int main(int argc, char *argv[])
{
	Display			*displ;
	int			screen_num;
	Window			win;
	GC			gc;
	Pixmap			pixmap;
	unsigned int		width;
	unsigned int		height;
	unsigned int		depth;
	char			*displ_name = getenv("DISPLAY");
	struct timespec		tm_req, tm_rem;
	int			i, j;

	if ((displ = XOpenDisplay(displ_name)) != NULL) {
		screen_num = DefaultScreen(displ);

		win = XCreateSimpleWindow(
			displ, RootWindow(displ, screen_num),
			0, 0, 400, 200, 2,
			BlackPixel(displ, screen_num),
			WhitePixel(displ, screen_num));

		FPRINTF("=== Trying to determine max width of a X pixmap ===\n\n"
			"XCreatePixmap() is supposed to use unsigned int for width and height but it\n"
			"actually uses signed 16-bit int, and this can be verified by creating pixmaps\n"
			"of variable width in loop and see what happens.\n\n"
			"-> If signed 16-bit int, last valid width should be 32767, and any value above\n"
			"that should trigger an error - So let's try\n\n");

		height = 50;
		depth = DefaultDepth(displ, screen_num);

		FPRINTF("Will create pixmap: variable width, height = %d, depth = %d\n"
			"Press <enter> to proceed\n",
			height, depth);
		fgetc(stdin);

		XMapWindow(displ, win);
		XSync(displ, False);

		width = 0;
		j = 0;
		tm_req.tv_sec = 0;
		tm_req.tv_nsec = 10000000;

		for (i = 1; i < 2000; i++) {
			if (width < 32750 || width > 32780)
				width = i * 50;
			else {
				width++;
				i = (32780 / 50) + 1;
			}

			FPRINTF("\rwidth = %d --- ", width)

			pixmap = XCreatePixmap(displ, RootWindow(displ, screen_num), width, height, depth);
			gc = XCreateGC(displ, pixmap, 0, NULL);
			XSetForeground(displ, gc, BlackPixel(displ, screen_num));
			XFillRectangle(displ, pixmap, gc, 0, 0, width, height);
			XClearArea(displ, win, 0, 0, 400, 200, False);
			XCopyArea(displ, pixmap, win, gc, 0, 0, 200, 50, 25 * j / 4, 25 * j / 4);
			if (++j > 32)
				j = 0;
			XMapWindow(displ, win);
			XFlush(displ);
			XMoveWindow(displ, win, 1000, 0);
			nanosleep(&tm_req, &tm_rem);

			XFreePixmap(displ, pixmap);
			XFreeGC(displ, gc);
			XSync(displ, False);
		}
		FPRINTF("\n")
		return 0;
	} else {
		fprintf(stderr, "%s: Can't connect to X server '%s'\n",
			argv[0], displ_name);
		return 1;
	}
}
