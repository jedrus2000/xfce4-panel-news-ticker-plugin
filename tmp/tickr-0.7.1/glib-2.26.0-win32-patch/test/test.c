#include <stdio.h>
#include <time.h>
#include <glib.h>
#include <windows.h>

GMainLoop	*main_loop;
int		delay = 50, counter = 0;
time_t		time0 = 0, time1 = 0;

static gint callback()
{
	if (counter == 0)
		time0 = time(NULL);
	else if (counter == 1000 - 1)
		time1 = time(NULL);

	if (counter++ < 1000 - 1)
		return TRUE;
	else {
		printf("%4d ms\n", (int)difftime(time1, time0));
		if (delay > 30)
			delay -= 10;
		else {
			if (delay > 15)
				delay -= 5;
			else
				delay--;
		}

		if (delay > 0) {
			counter = 0;
			printf("%4d ms - ", delay);
			g_timeout_add_full(G_PRIORITY_HIGH, delay, callback, NULL, NULL);
		} else
			g_main_loop_quit(main_loop);

		return FALSE;
	}
}

int main()
{
	printf(	"=== Testing glib with 1000 calls to callback function ===\n"
		"Requested g_timeout interval - real interval\n");

	printf("%4d ms - ", delay);
	g_timeout_add_full(G_PRIORITY_HIGH, delay, callback, NULL, NULL);

	main_loop = g_main_loop_new(NULL, TRUE);
	g_main_loop_run(main_loop);

	return 0;
}
