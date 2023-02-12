/*
 *	TICKR - GTK-based Feed Reader - Copyright (C) Emmanuel Thomas-Maurin 2009-2021
 *	<manutm007@gmail.com>
 *
 * 	This program is free software: you can redistribute it and/or modify
 * 	it under the terms of the GNU General Public License as published by
 * 	the Free Software Foundation, either version 3 of the License, or
 * 	(at your option) any later version.
 *
 * 	This program is distributed in the hope that it will be useful,
 * 	but WITHOUT ANY WARRANTY; without even the implied warranty of
 * 	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * 	GNU General Public License for more details.
 *
 * 	You should have received a copy of the GNU General Public License
 * 	along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "tickr.h"

/*
 * Static global structs and variables for this module, with related setter/getter
 * functions for all other modules.
 */
static TickerEnv	*env = NULL;
static Resource		*resrc = NULL;
static FList		*feed_list = NULL, *feed_selection = NULL;
static Params		*prm = NULL;
static int		instance_id = 0;
static zboolean		no_ui = FALSE;
static int		speed_up_flag = FALSE;
static int		slow_down_flag = FALSE;

static GtkWidget	*main_hbox, *vbox_ticker, *vbox_clock, *popup_menu;
static GtkAccelGroup	*popup_menu_accel_group;
static int		popup_menu_accel_group_attached;

TickerEnv *get_ticker_env()
{
	return env;
}

Resource *get_resource()
{
	return resrc;
}

FList *get_feed_list()
{
	return feed_list;
}

void set_feed_list(FList *list)
{
	feed_list = list;
}

FList *get_feed_selection()
{
	return feed_selection;
}

void set_feed_selection(FList *list)
{
	feed_selection = list;
}

Params *get_params()
{
	return prm;
}

int get_instance_id()
{
	return instance_id;
}

/* Prototypes for static funcs */
static void	modify_params0();
static void	connection_settings0();
static int	shift2left_callback();
static void	check_time_load_resource_from_selection(check_time_mode);

#define START_PAUSE_TICKER_WHILE_OPENING\
	int suspend_rq_bak = env->suspend_rq;\
	env->suspend_rq = TRUE;

#define END_PAUSE_TICKER_WHILE_OPENING\
	env->suspend_rq = suspend_rq_bak;

/* Sometimes, we need to temporarily re-enable popups, just to get error messages. */
#define START_PAUSE_TICKER_ENABLE_POPUPS_WHILE_OPENING\
	int	suspend_rq_bak;\
	char	disable_popups_bak;\
	suspend_rq_bak = env->suspend_rq;\
	env->suspend_rq = TRUE;\
	disable_popups_bak = get_params()->disable_popups;\
	get_params()->disable_popups = 'n';

#define END_PAUSE_TICKER_ENABLE_POPUPS_WHILE_OPENING\
	get_params()->disable_popups = disable_popups_bak;\
	env->suspend_rq = suspend_rq_bak;

/*
 * funct_name0(***NO args***) is only used to call funct_name(***args***) from popup menus.
 */
static void manage_list_and_selection0()
{
	START_PAUSE_TICKER_ENABLE_POPUPS_WHILE_OPENING
	if (!no_ui)
		manage_list_and_selection(resrc);
	END_PAUSE_TICKER_ENABLE_POPUPS_WHILE_OPENING
}

static void open_txt_file0()
{
	START_PAUSE_TICKER_ENABLE_POPUPS_WHILE_OPENING
	if (!no_ui)
		open_txt_file(resrc);
	END_PAUSE_TICKER_ENABLE_POPUPS_WHILE_OPENING
}

static void import_opml_file0()
{
	START_PAUSE_TICKER_ENABLE_POPUPS_WHILE_OPENING
	if (!no_ui)
		if (import_opml_file() == OK)
			manage_list_and_selection(resrc);
	END_PAUSE_TICKER_ENABLE_POPUPS_WHILE_OPENING
}

static void export_opml_file0()
{
	START_PAUSE_TICKER_ENABLE_POPUPS_WHILE_OPENING
	if (!no_ui)
		export_opml_file();
	END_PAUSE_TICKER_ENABLE_POPUPS_WHILE_OPENING
}

static void show_resource_info0()
{
	START_PAUSE_TICKER_ENABLE_POPUPS_WHILE_OPENING
	show_resource_info(get_resource());
	END_PAUSE_TICKER_ENABLE_POPUPS_WHILE_OPENING
}

static void easily_modify_params0()
{
	next_action i;

	START_PAUSE_TICKER_WHILE_OPENING
	if (!no_ui) {
		if ((i = easily_modify_params(prm)) == DO_NEXT_OPEN_FULL_SETTINGS) {
			END_PAUSE_TICKER_WHILE_OPENING
			modify_params0();
		} else if (i == DO_NEXT_OPEN_CONN_SETTINGS) {
			END_PAUSE_TICKER_WHILE_OPENING
			connection_settings0();
		}
	}
	END_PAUSE_TICKER_WHILE_OPENING
}

static void modify_params0()
{
	next_action i;

	START_PAUSE_TICKER_WHILE_OPENING
	if (!no_ui) {
		if ((i = modify_params(prm)) == DO_NEXT_OPEN_CONN_SETTINGS) {
			END_PAUSE_TICKER_WHILE_OPENING
			connection_settings0();
		}
	}
	END_PAUSE_TICKER_WHILE_OPENING
}

static void connection_settings0()
{
	START_PAUSE_TICKER_WHILE_OPENING
	if (!no_ui)
		if (connection_settings(AUTH_PAGE) == GTK_RESPONSE_OK)
			current_feed();
	END_PAUSE_TICKER_WHILE_OPENING
}

static void import_params0()
{
	START_PAUSE_TICKER_WHILE_OPENING
	if (!no_ui)
		import_params();
	END_PAUSE_TICKER_WHILE_OPENING
}

static void export_params0()
{
	START_PAUSE_TICKER_WHILE_OPENING
	if (!no_ui)
		export_params();
	END_PAUSE_TICKER_WHILE_OPENING
}

static void pause_on_mouseover_enabled_warning_once()
{
	static int first_run = -1;

	if (prm->pause_on_mouseover == 'y') {
		if (first_run == -1)
			warning(BLOCK,
				"Setting 'Pause ticker on mouse-over' is enabled. When so, playing/pausing\n"
				"is always controlled by mouse pointer motions as well.");
		first_run++;
		first_run &= 1;
	}
}

static void ticker_play()
{
	env->suspend_rq = FALSE;
	pause_on_mouseover_enabled_warning_once();
}

static void ticker_pause()
{
	env->suspend_rq = TRUE;
	pause_on_mouseover_enabled_warning_once();
}

static void ticker_reload()
{
	current_feed();
	env->reload_rq = TRUE;
	env->suspend_rq = FALSE;
}

static void first_feed0()
{
	if (env->selection_mode == MULTIPLE)
		first_feed();
	else
		info_win_no_block("Single selection mode", INFO_WIN_WAIT_TIMEOUT);
}

static void previous_feed0()
{
	if (env->selection_mode == MULTIPLE)
		previous_feed();
	else
		info_win_no_block("Single selection mode", INFO_WIN_WAIT_TIMEOUT);
}

static void next_feed0()
{
	if (env->selection_mode == MULTIPLE)
		next_feed();
	else
		info_win_no_block("Single selection mode", INFO_WIN_WAIT_TIMEOUT);
}

static void last_feed0()
{
	if (env->selection_mode == MULTIPLE)
		last_feed();
	else
		info_win_no_block("Single selection mode", INFO_WIN_WAIT_TIMEOUT);
}

void toggle_speed_up_flag()
{
	speed_up_flag = TRUE;
}

void toggle_slow_down_flag()
{
	slow_down_flag = TRUE;
}

static void help_win0()
{
	START_PAUSE_TICKER_WHILE_OPENING
	help_win();
	END_PAUSE_TICKER_WHILE_OPENING
}

static void online_help0()
{
	START_PAUSE_TICKER_ENABLE_POPUPS_WHILE_OPENING
	online_help();
	END_PAUSE_TICKER_ENABLE_POPUPS_WHILE_OPENING
}

static void check_for_updates0()
{
	START_PAUSE_TICKER_ENABLE_POPUPS_WHILE_OPENING
	check_for_updates();
	END_PAUSE_TICKER_ENABLE_POPUPS_WHILE_OPENING
}

static void about_win0()
{
	START_PAUSE_TICKER_WHILE_OPENING
	about_win();
	END_PAUSE_TICKER_WHILE_OPENING
}

/* Find default browser if none is set. */
static int easily_link_with_browser2()
{
	char		browser_cmd[512], tmp[1024];
	int		error_code = NO_BROWSER_SET_ERROR;

#ifndef G_OS_WIN32
	str_n_cpy(browser_cmd, OPENLINKCMD, 511);
#else
	char		*browser_cmd_p;
	unsigned int	i, count, start = 0, end = 0;

	if ((browser_cmd_p = (char *)get_default_browser_from_win32registry()) != NULL) {
		/* Find 1st string in browser_cmd */
		str_n_cpy(tmp, browser_cmd_p, 511);
		for (i = 0, count = 0; i < strlen(tmp); i++) {
			if (tmp[i] == '"') {
				count++;
				if (count == 1)
					start = i + 1;
				else if (count == 2)
					end = i - 1;
			}
		}
		str_n_cpy(browser_cmd, tmp + start, MIN((end - start + 1), 511));
	} else {
		info_win("", "\nCan't find Browser shell command."
			"You will have to set this parameter manually in the Full Settings window.\n",
			INFO_ERROR, FALSE);
		return error_code;
	}
#endif
	snprintf(tmp, 1024,
		"\nThis is the Shell command that opens your default Browser:\n\n%s\n\n"
		"Do you want to use it ?\n", browser_cmd);
	if (question_win(tmp, YES) == YES) {
		str_n_cpy(prm->open_link_cmd, browser_cmd, FILE_NAME_MAXLEN);
		str_n_cpy(prm->open_link_args, "", FILE_NAME_MAXLEN);
		save_to_config_file(prm);
		error_code = OK;
	} else
		info_win("", "\nCancelled.\n"
			"You may set this parameter manually in the Full Settings window.\n",
			INFO, FALSE);
	return error_code;
}

int easily_link_with_browser()
{
	int error_code;

	gtk_window_set_keep_above(GTK_WINDOW(env->win), FALSE);
	error_code = easily_link_with_browser2();
	check_main_win_always_on_top();
	return error_code;
}

/*
 * If link found, copy it into env->active_link and return rank (starting at 1),
 * otherwise, return -1 (with env->active_link = "").
 */
static int get_visible_link_and_rank(int position_x_in_drwa)
{
	int		location_on_surface, n_links, i;
	zboolean	link_found = FALSE;

	env->active_link[0] = '\0';
	if (position_x_in_drwa > -1) {
		for (n_links = NFEEDLINKANDOFFSETMAX - 1; n_links >= 0; n_links--)
			if (resrc->link_and_offset[n_links].offset_in_surface > 0)
				break;
		if (STANDARD_SCROLLING) {
			location_on_surface = (env->shift_counter * prm->shift_size) + position_x_in_drwa +
				get_links_extra_offset();
			for (i = 0; i <= n_links; i++) {	/* First link rank = 1 */
				if (resrc->link_and_offset[i].offset_in_surface > location_on_surface) {
					link_found = TRUE;
					break;
				}
			}
		} else {
			location_on_surface = env->surf_width - env->drwa_width -
				(env->shift_counter * prm->shift_size) + position_x_in_drwa +
				get_links_extra_offset();
			for (i = n_links; i >= 0; i--) {
				if (resrc->link_and_offset[i].offset_in_surface < location_on_surface &&
						resrc->link_and_offset[i - 1].offset_in_surface > location_on_surface) {
					link_found = TRUE;
					break;
				}
			}
		}
		if (link_found && i > 0 && i <= n_links) {
			str_n_cpy(env->active_link, resrc->link_and_offset[i].url, FILE_NAME_MAXLEN);
			DEBUG_INFO("Link found (rank = %d)\n", i);
			/*show_str_beginning_and_end(env->active_link);*/
			return i;
		}
		DEBUG_INFO("No link found\n");
	}
	return -1;
}

static void open_link()
{
	char	tmp1[FILE_NAME_MAXLEN + 1], tmp2[FILE_NAME_MAXLEN + 1];
	char	*argv[32];	/* Up to 32 - 3 (prog name, URL, NULL) args */
	GPid	pid;
	GError	*error = NULL;
	int	i, j;

	if (prm->open_link_cmd[0] == '\0') {
		easily_link_with_browser();
		if (prm->open_link_cmd[0] == '\0') {
			warning(BLOCK, "Can't launch Browser: no command is defined.\n",
				"Please set the 'Open in Browser' option in the Full Settings window.");
			return;
		}
	}

	if (env->active_link[0] == '\0') {
		warning(BLOCK, "No link found");
		return;
	}

	INFO_OUT("Spawning: %s %s %s\n", prm->open_link_cmd, prm->open_link_args, env->active_link)
	str_n_cpy(tmp1, prm->open_link_cmd, FILE_NAME_MAXLEN);
	str_n_cpy(tmp2, prm->open_link_args, FILE_NAME_MAXLEN);
	argv[0] = tmp1;
	for (i = 0, j = 1; tmp2[i] != '\0' && j < 32 + 1 - 3; j++) {
		argv[j] = &tmp2[i];
		while (tmp2[++i] != ' ')
			if (tmp2[i] == '\0')
				break;
		tmp2[i] = '\0';
		while (tmp2[++i] == ' ');
	}
	argv[j] = env->active_link;
	argv[j + 1] = NULL;
	if (!g_spawn_async(NULL, argv, NULL, G_SPAWN_SEARCH_PATH, NULL, NULL, &pid, &error)) {
		warning(BLOCK, "%s: Can't create process %s - %s", APP_NAME, argv[0], error->message);
#ifndef G_OS_WIN32
		info_win("", "Please check the 'Open in Browser' option in the Full Settings window.",
			INFO_WARNING, FALSE);
#else
		if (easily_link_with_browser2() == OK)
			open_link();
#endif
	} else
		g_spawn_close_pid(pid);
}

static void open_link0()
{
	get_visible_link_and_rank(env->drwa_width / 2);
	open_link();
}

/*static void mark_item()
{
}

static void mark_item0()
{
	mark_item();
}*/

/*
 * Continuously get mouse x position inside drawing area.
 * Do nothing if outside, so must be used only by mouse-over-drawing-area /
 * click-on-drawing-area funcs.
 */
static int track_mouse_position_x(GtkWidget *widget, GdkEventMotion *event)
{
	widget = widget;
	if (event->type == GDK_MOTION_NOTIFY)	/* Means mouse has moved inside widget  */
		env->mouse_x_in_drwa = (int)(event->x);
	return FALSE;
}

static int left_click_on_drawing_area(GtkWidget *widget, GdkEventButton *event_but)
{
	widget = widget;
	if (prm->disable_leftclick != 'y' && event_but->type == GDK_BUTTON_PRESS &&\
			event_but->button == 1) {	/* 1 = mouse left button */
		if (event_but->state & GDK_CONTROL_MASK) {
			/* Do nothing so far */
			return FALSE;
		} else if (event_but->state & GDK_MOD1_MASK) {	/* = alt */
			/* Do nothing so far */
			return FALSE;
		} else {
			get_visible_link_and_rank(env->mouse_x_in_drwa);
			open_link();
			return TRUE;
		}
	} else
		return FALSE;
}

static int right_click_on_drawing_area(GtkWidget *widget, GdkEventButton *event_but)
{
	widget = widget;
	if (event_but->type == GDK_BUTTON_PRESS && event_but->button == 3) {	/* 3 = mouse right button */
		if (event_but->state & GDK_CONTROL_MASK) {
			quick_feed_picker();
			return TRUE;
		} else if (event_but->state & GDK_MOD1_MASK) {	/* = alt */
			/* Do nothing so far */
			return FALSE;
		} else {
			/* Popup main menu */
			gtk_menu_popup(GTK_MENU(popup_menu), NULL, NULL, NULL, NULL,
				event_but->button, event_but->time);
			return TRUE;
		}
	} else
		return FALSE;
}

static const char *get_win_title_text()
{
	static char	tmp1[2][256 + 64], tmp2[2][64];
	static int	i = -1;

	/* To allow 2 simultaneous calls */
	i++;
	i &= 1;

	if (resrc->type == RESRC_URL)
		str_n_cpy(tmp1[i], resrc->feed_title, 256);
	else if (resrc->type == RESRC_FILE)
		str_n_cpy(tmp1[i], resrc->id, 256);
	else
		str_n_cpy(tmp1[i], "No resource", 256);
	snprintf(tmp2[i], 64, "  |  %s-%s", APP_NAME, APP_V_NUM);
	return (const char *)str_n_cat(tmp1[i], tmp2[i], 63);
}

char *get_feed_extra_info(int rank)
{
	FILE		*fp;
	char		*str1, tmp[4];
	size_t		str1_size = FGETS_STR_MAXLEN;
	static char	*str2 = NULL;
	int		tag_found = FALSE;

	if (str2 != NULL)
		l_str_free(str2);
	str2 = l_str_new("");
	if (rank == -1)
		return str2;
	if ((fp = g_fopen(resrc->xml_dump_extra, "rb")) != NULL) {
		str1 = malloc2(str1_size * sizeof(char));
#ifndef G_OS_WIN32
		while (getline(&str1, &str1_size, fp) != -1) {
#else
		while (fgets(str1, str1_size, fp) != NULL) {
#endif
			if (str1[0] == ITEM_TITLE_TAG_CHAR || str1[0] == ITEM_DES_TAG_CHAR) {
				if (tag_found)
					break;
				if (rank == atoi(str_n_cpy(tmp, str1 + 1, 3))) {
					tag_found = TRUE;
					str2 = l_str_cat(str2, str1 + 4);
				}
			} else if (tag_found)
				str2 = l_str_cat(str2, str1);
		}
		free2(str1);
		fclose(fp);
	}
	remove_trailing_whitespaces_from_str(str2);
	return str2;
}

static const char *get_ticker_tooltip_text(int rank)
{
	static char *tooltip_text = NULL;

	if (tooltip_text != NULL) {
		l_str_free(tooltip_text);
		tooltip_text = NULL;
	}
	if ((prm->item_title == 'y' && prm->item_description == 'n') ||
			(prm->item_title == 'n' && prm->item_description == 'y')) {
		tooltip_text = l_str_new(get_feed_extra_info(rank));
		if (tooltip_text[0] != '\0')
			tooltip_text = l_str_cat(tooltip_text, "\n\n");
		tooltip_text = l_str_cat(tooltip_text, get_win_title_text());
		return tooltip_text;
	} else
		return get_win_title_text();
}

/* Pause ticker when mouse pointer is over (opened) popup menu. */
static int mouse_over_opened_popup_menu(GtkWidget *widget, GdkEvent *event)
{
	widget = widget;
	if (prm->pause_on_mouseover == 'y') {
		if (event->type == GDK_EXPOSE)
			env->suspend_rq = TRUE;
		else if (event->type == GDK_LEAVE_NOTIFY)
			env->suspend_rq = FALSE;
	}
	return FALSE;
}

/* Pause ticker on mouseover. */
static int mouse_over_drawing_area(GtkWidget *widget, GdkEvent *event)
{
	int rank;

	widget = widget;
	if (event->type == GDK_ENTER_NOTIFY || event->type == GDK_LEAVE_NOTIFY) {
		if (prm->pause_on_mouseover == 'y') {
			if (event->type == GDK_ENTER_NOTIFY)
				env->suspend_rq = TRUE;
			else
				env->suspend_rq = FALSE;
		}
		if (event->type == GDK_ENTER_NOTIFY) {
			while (gdk_events_pending())	/* So that "motion-notify-event" is processed now. */
				gtk_main_iteration();
			rank = get_visible_link_and_rank(env->mouse_x_in_drwa);
			gtk_widget_set_tooltip_text(widget, get_ticker_tooltip_text(rank));
			gtk_tooltip_trigger_tooltip_query(gdk_display_get_default());
			return TRUE;
		}
	}
	return FALSE;
}

static int mouse_wheel_action_on_drawing_area(GtkWidget *widget, GdkEvent *event)
{
	char mouse_wheel_action;

	widget = widget;
	if (event->type == GDK_SCROLL) {
		mouse_wheel_action = prm->mouse_wheel_action;
		if (event->scroll.state & GDK_CONTROL_MASK) {
			/* If <ctrl> pressed, invert mouse_wheel_action */
			if (mouse_wheel_action == 's')
				mouse_wheel_action = 'f';
			else if (mouse_wheel_action == 'f')
				mouse_wheel_action = 's';
		}
		if (mouse_wheel_action == 's') {
			if (event->scroll.direction == GDK_SCROLL_UP)
				toggle_speed_up_flag();
			else if (event->scroll.direction == GDK_SCROLL_DOWN)
				toggle_slow_down_flag();
			env->suspend_rq = FALSE;
			return TRUE;
		} else if (mouse_wheel_action == 'f') {
			if (env->selection_mode == MULTIPLE) {
				/* Seems buggy
				if (event->scroll.direction == GDK_SCROLL_UP)
					next_feed0();
				else if (event->scroll.direction == GDK_SCROLL_DOWN)
					previous_feed0();*/
				quick_feed_picker();
				env->suspend_rq = FALSE;
			} else
				warning(BLOCK, "Single selection mode\n",
					"(You have set 'Mouse Wheel acts on: Feed')");
			return TRUE;
		}
	}
	return FALSE;
}

/*
 * Popup menu stuff
 */
static GtkItemFactoryEntry popup_menu_item[] = {
	{"/_File",				NULL, NULL, 0, "<Branch>", NULL},

	{"/File/Feed Organizer (_RSS|Atom)",	"<control>R", manage_list_and_selection0, 0, "<StockItem>",
						(gconstpointer)RSS_ICON},

	{"/File/Open _Text File",		"<control>T", open_txt_file0, 0, "<StockItem>",
						(gconstpointer)GTK_STOCK_OPEN},

	{"/File/sep",				NULL, NULL, 0, "<Separator>", NULL},

	{"/File/_Import Feed List (OPML)",	"<control>I", import_opml_file0, 0, NULL, NULL},

	{"/File/_Export Feed List (OPML)",	"<control>E", export_opml_file0, 0, NULL, NULL},

	{"/File/sep",				NULL, NULL, 0, "<Separator>", NULL},

	{"/File/Resource _Properties",		"<control>P", show_resource_info0, 0, "<StockItem>",
						(gconstpointer)GTK_STOCK_PROPERTIES},

	{"/File/sep",				NULL, NULL, 0, "<Separator>", NULL},

	{"/File/_Quit",				"<control>Q", gtk_main_quit, 0, "<StockItem>",
						(gconstpointer)GTK_STOCK_QUIT},

	{"/_Edit",				NULL, NULL, 0, "<Branch>", NULL},

	{"/Edit/Preference_s",			"<control>S", easily_modify_params0, 0, "<StockItem>",
						(gconstpointer)GTK_STOCK_PREFERENCES},

	{"/Edit/Full Settings",			"<control>", modify_params0, 0, "<StockItem>",
						(gconstpointer)GTK_STOCK_PREFERENCES},

	{"/Edit/Connection Settings",		"<control>", connection_settings0, 0, "<StockItem>",
						(gconstpointer)GTK_STOCK_CONNECT},

	{"/Edit/sep",				NULL, NULL, 0, "<Separator>", NULL},

	{"/Edit/Import Settings",		"<control>", import_params0, 0, NULL, NULL},

	{"/Edit/Export Settings",		"<control>", export_params0, 0, NULL, NULL},

	{"/_Control",				NULL, NULL, 0, "<Branch>", NULL},

	{"/Control/Open Link in _Browser",	"<control>B", open_link0, 0, "<StockItem>",
						(gconstpointer)GTK_STOCK_JUMP_TO/*GTK_STOCK_REDO*/},

	/*{"/Control/_Mark Item",			"<control>M", mark_item0, 0, "<StockItem>",
						(gconstpointer)GTK_STOCK_OK},*/

	{"/Control/sep",			NULL, NULL, 0, "<Separator>", NULL},

	{"/Control/Play",			"<control>J", ticker_play, 0, "<StockItem>",
						(gconstpointer)GTK_STOCK_MEDIA_PLAY},

	{"/Control/Pause",			"<control>K", ticker_pause, 0, "<StockItem>",
						(gconstpointer)GTK_STOCK_MEDIA_PAUSE},

	{"/Control/Reload",			"<control>L", ticker_reload, 0, "<StockItem>",
						(gconstpointer)GTK_STOCK_REFRESH},

	{"/Control/sep",			NULL, NULL, 0, "<Separator>", NULL},

	{"/Control/First Feed",			"<control>", first_feed0, 0, "<StockItem>",
						(gconstpointer)GTK_STOCK_GOTO_FIRST},

	{"/Control/Previous Feed",		"<control>", previous_feed0, 0,  "<StockItem>",
						(gconstpointer)GTK_STOCK_GO_BACK},

	{"/Control/Next Feed",			"<control>", next_feed0, 0, "<StockItem>",
						(gconstpointer)GTK_STOCK_GO_FORWARD},

	{"/Control/Last Feed",			"<control>", last_feed0, 0, "<StockItem>",
						(gconstpointer)GTK_STOCK_GOTO_LAST},

	{"/Control/sep",			NULL, NULL, 0, "<Separator>", NULL},

	{"/Control/Speed Up",			"<control>U", toggle_speed_up_flag, 0, "<StockItem>",
						(gconstpointer)GTK_STOCK_GO_UP},

	{"/Control/Speed Down",			"<control>D", toggle_slow_down_flag, 0,  "<StockItem>",
						(gconstpointer)GTK_STOCK_GO_DOWN},

	{"/_Help",				NULL, NULL, 0, "<Branch>", NULL},

	{"/Help/Quick Help",			"F1", help_win0, 0, "<StockItem>",
						(gconstpointer)GTK_STOCK_HELP},

	{"/Help/Online Help",			"<control>H", online_help0, 0, NULL, NULL},

	{"/Help/Check for Updates",		"<control>", check_for_updates0, 0, NULL, NULL},

	{"/Help/sep",				NULL, NULL, 0, "<Separator>", NULL},

	{"/Help/_About",			"<control>A", about_win0, 0, "<StockItem>",
						(gconstpointer)GTK_STOCK_ABOUT},
};

static int n_popup_menu_items = sizeof(popup_menu_item) / sizeof(popup_menu_item[0]);

static GtkWidget *get_popup_menu_menu(GtkWidget *win)
{
	GtkItemFactory *item_factory;

	win = win;
	popup_menu_accel_group = gtk_accel_group_new();
	item_factory = gtk_item_factory_new(GTK_TYPE_MENU, "<main>", popup_menu_accel_group);
	gtk_item_factory_create_items(item_factory, n_popup_menu_items, popup_menu_item, NULL);
	return gtk_item_factory_get_widget(item_factory, "<main>");
}

void check_main_win_always_on_top()
{
	zboolean z = prm->always_on_top == 'y' ? TRUE : FALSE;

	gtk_window_set_keep_above(GTK_WINDOW(env->win), z);
	if (GDK_IS_WINDOW(env->win->window))
		/* Does this make a difference, at all ? */
		gdk_window_set_keep_above(GDK_WINDOW(env->win->window), z);
}

static int update_win_dims_and_loc()
{
	/* Dimensions */
	/* Smallest width set to 1 as 0 will result in some weird on screen square artifact */
	gtk_widget_set_size_request(env->drw_a, MAX(env->drwa_width, 1), env->drwa_height);
	gtk_widget_set_size_request(env->drwa_clock, MAX(env->drwa_clock_width, 1), env->drwa_height);

	gtk_widget_set_size_request(env->win, 1, 1);
	gtk_window_resize(GTK_WINDOW(env->win), env->drwa_width + env->drwa_clock_width, env->drwa_height);

	/* Disabled to fix the flickering-every-500-ms issue on Linux Mint 18 Cinnamon
	gtk_window_unmaximize(GTK_WINDOW(env->win));*/

	/*
	 * === NEW & experimental - Set override_redirect flag, ie bypass window manager ===
	 * Where should this go ?
	 */
	if (GDK_IS_WINDOW(env->win->window))
		gdk_window_set_override_redirect(GDK_WINDOW(env->win->window),
			prm->override_redirect == 'y' ? TRUE : FALSE);
		/*gdk_window_set_type_hint(GDK_WINDOW(env->win->window),
			prm->override_redirect == 'y' ?
			GDK_WINDOW_TYPE_HINT_DOCK : GDK_WINDOW_TYPE_HINT_NORMAL);*/

	/* Location */
	gtk_window_move(GTK_WINDOW(env->win), prm->win_x, prm->win_y);
	return TRUE;
}

/* Check if win needs to be re-computed because of changed params. */
zboolean win_params_have_been_changed(Params *new_prm, int n_required_runs)
{
	static Params	prm_bak0;
	Params		*prm_bak = &prm_bak0;
	static int	initial_runs = 0;
	zboolean	changes = FALSE;

	if (initial_runs < n_required_runs) {
		/* 'Fake' params changes are required at program startup */
		memcpy((void *)prm_bak, (const void *)new_prm, sizeof(Params));
		initial_runs++;
		return TRUE;
	} else if (strcmp(new_prm->font_name_size, prm_bak->font_name_size) != 0)
		changes = TRUE;
	else if (new_prm->win_x != prm_bak->win_x)
		changes = TRUE;
	else if (new_prm->win_y != prm_bak->win_y)
		changes = TRUE;
	else if (new_prm->win_w != prm_bak->win_w)
		changes = TRUE;
	else if (new_prm->win_h != prm_bak->win_h)
		changes = TRUE;
	else if (new_prm->windec != prm_bak->windec)
		changes = TRUE;
	else if (new_prm->always_on_top != prm_bak->always_on_top)
		changes = TRUE;
	else if (new_prm->win_transparency != prm_bak->win_transparency)
		changes = TRUE;
	else if (new_prm->icon_in_taskbar != prm_bak->icon_in_taskbar)
		changes = TRUE;
	else if (new_prm->win_sticky != prm_bak->win_sticky)
		changes = TRUE;
	else if (new_prm->override_redirect != prm_bak->override_redirect)
		changes = TRUE;
	else if (new_prm->clock != prm_bak->clock)
		changes = TRUE;
	else if (new_prm->clock_sec != prm_bak->clock_sec)
		changes = TRUE;
	else if (new_prm->clock_12h != prm_bak->clock_12h)
		changes = TRUE;
	else if (new_prm->clock_date != prm_bak->clock_date)
		changes = TRUE;
	else if (strcmp(new_prm->clock_font_name_size, prm_bak->clock_font_name_size) != 0)
		changes = TRUE;
	/* Need to ckeck other params ? */
	if (changes == TRUE) {
		memcpy((void *)prm_bak, (const void *)new_prm, sizeof(Params));
		return TRUE;
	} else
		return FALSE;
}

/*
 * Create new or next cairo image surface and compute stuff to redraw window.
 */
static int compute_surface_and_win()
{
	char 		font_name[FONT_NAME_MAXLEN + 1], font_size[FONT_SIZE_MAXLEN + 1];
	char		c_font_name[FONT_NAME_MAXLEN + 1], c_font_size[FONT_SIZE_MAXLEN + 1];
	int		height1, height2;
	int		size, render_error_code, max_width, i;
	zboolean	new_win_params;

	/*
	 * The following code needs to be run:
	 * - twice at program startup (ie once after gtk_widget_show_all() has been called)
	 * - whenever some window params have been changed
	 * - not everytime a new feed is loaded
	 */
	new_win_params = win_params_have_been_changed(prm, 2);
	if (new_win_params) {
		/*
		 * Compute font size from requested height if > 0
		 */
		adjust_font_size_to_rq_height(prm->font_name_size, prm->win_h);
		/*
		 * Compute clock font size
		 * clock height = ticker height unless clock font size is set
		 * in which case clock height is always <= ticker height
		 */
		split_font(prm->clock_font_name_size, c_font_name, c_font_size);
		if (strcmp(font_name, c_font_name) == 0) {
			if (atoi(c_font_size) > atoi(font_size))
				str_n_cpy(c_font_size, font_size, FONT_SIZE_MAXLEN);
		} else {
			height1 = get_layout_height_from_font_name_size(prm->clock_font_name_size);
			height2 = get_layout_height_from_font_name_size(prm->font_name_size);
			if (height1 > height2) {
				size = get_font_size_from_layout_height(height2, c_font_name);
				snprintf(c_font_size, FONT_SIZE_MAXLEN + 1, "%3d", size);
			}
		}
		compact_font(prm->clock_font_name_size, c_font_name, c_font_size);
		/*
		 * Compute ticker width
		 */
		max_width = prm->disable_screen_limits == 'y' ? WIN_MAX_W : env->screen_w;
		if (prm->win_w >= DRWA_WIDTH_MIN && prm->win_w <= max_width)
			env->drwa_width = prm->win_w;
		else if (prm->win_w < DRWA_WIDTH_MIN)
			env->drwa_width = DRWA_WIDTH_MIN;
		else if (prm->win_w > max_width)
			env->drwa_width = max_width;
		env->drwa_clock_width = get_clock_width(prm);	/* = 0 if no clock, although actual widget width = 1.
								 * See quick hack below. */
		env->drwa_width -= env->drwa_clock_width;
	}
	/*
	 * Reset link_and_offset stuff
	 */
	if(resrc->id[0] == '\0' && resrc->fp != NULL) {
		fclose(resrc->fp);
		resrc->fp = NULL;
		for (i = 0; i < NFEEDLINKANDOFFSETMAX; i++) {
			resrc->link_and_offset[i].url[0] = '\0';
			resrc->link_and_offset[i].offset_in_surface = 0;
		}
		if(resrc->fp_extra != NULL) {
			fclose(resrc->fp_extra);
			resrc->fp_extra = NULL;
		}
	}
	/*
	 * Create cairo image surface of rendered text (one long single line)
	 */
	env->c_surf = render_stream_to_surface(resrc->fp, resrc->link_and_offset, prm, &render_error_code);
	if (env->c_surf != NULL) {
		env->surf_width = cairo_image_surface_get_width(env->c_surf);
		env->surf_height = cairo_image_surface_get_height(env->c_surf);
		env->drwa_height = (MIN(env->surf_height, env->screen_h));
	} else
		big_error(render_error_code, "render_stream_to_surface(): %s",
			global_error_str(render_error_code));
	/*
	 * Win stuff
	 */
	/* Title */
	gtk_window_set_title(GTK_WINDOW(env->win), get_win_title_text());
	/*
	 * The following code needs to be run:
	 * - twice at program startup (ie once after gtk_widget_show_all() has been called)
	 * - whenever some window params have been changed
	 * - not everytime a new feed is loaded
	 */
	if (new_win_params) {
		gtk_window_set_keep_above(GTK_WINDOW(env->win), FALSE);
		gtk_window_set_skip_taskbar_hint(GTK_WINDOW(env->win), FALSE);
		gtk_window_unstick(GTK_WINDOW(env->win));
		/* Transparency */
		gtk_window_set_opacity(GTK_WINDOW(env->win), prm->win_transparency);
		/* Decoration */
		if (prm->windec == 'y') {
			gtk_window_set_decorated(GTK_WINDOW(env->win), TRUE);
			if (!popup_menu_accel_group_attached) {
				gtk_window_add_accel_group(GTK_WINDOW(env->win), popup_menu_accel_group);
				popup_menu_accel_group_attached = TRUE;
			}
		} else if (prm->windec == 'n') {
			gtk_window_set_decorated(GTK_WINDOW(env->win), FALSE);
			if (popup_menu_accel_group_attached) {
				gtk_window_remove_accel_group(GTK_WINDOW(env->win), popup_menu_accel_group);
				popup_menu_accel_group_attached = FALSE;
			}
		}
		/* Clock */
		if (prm->clock == 'l')
			gtk_box_reorder_child(GTK_BOX(main_hbox), vbox_clock, 0);
		else if (prm->clock == 'r')
			gtk_box_reorder_child(GTK_BOX(main_hbox), vbox_clock, 1);
		else
			/*
			 * Quick hack to fix 'remaining 1 pixel wide line when enabling then disabling
			 * left clock' bug, because then vbox_clock/drwa_clock min size is still 1x1 (not 0x0)
			 * and is the first widget packed in main_hbox.
			 * So, when no clock is set, we make sure the 'empty clock widget' is always on
			 * the right side.
			 */
			 gtk_box_reorder_child(GTK_BOX(main_hbox), vbox_clock, 1);
		/* Move win */
		/*gtk_window_move(GTK_WINDOW(env->win), prm->win_x, prm->win_y);*/
		/* Icon in taskbar ? */
		gtk_window_set_skip_taskbar_hint(GTK_WINDOW(env->win),
			(prm->icon_in_taskbar == 'y') ? FALSE : TRUE);
		/* On all desktops ? */
		if (prm->win_sticky == 'y')
			gtk_window_stick(GTK_WINDOW(env->win));
		else if (prm->win_sticky == 'n')
			gtk_window_unstick(GTK_WINDOW(env->win));
		check_main_win_always_on_top();
		gtk_window_present(GTK_WINDOW(env->win));
		/*
		 * === NEW & experimental - Set override_redirect flag, ie bypass window manager ===
		 * Where should this go ?
		 */
		if (GDK_IS_WINDOW(env->win->window))
			gdk_window_set_override_redirect(GDK_WINDOW(env->win->window),
				prm->override_redirect == 'y' ? TRUE : FALSE);
			/*gdk_window_set_type_hint(GDK_WINDOW(env->win->window),
				prm->override_redirect == 'y' ?
				GDK_WINDOW_TYPE_HINT_DOCK : GDK_WINDOW_TYPE_HINT_NORMAL);*/
		/* Move win */
		gtk_window_move(GTK_WINDOW(env->win), prm->win_x, prm->win_y);
	}
	while (gtk_events_pending())
		gtk_main_iteration();
	return render_error_code;
}

/*
 * Timeout handler to render one part of cairo image surface onto drawing
 * area - image is shifted to left by <shift_size> pixels.
 *
 * Handler 'listens' to flags: suspend_rq, reload_rq, compute_rq and
 * feed_fully_rendered.
 *
 * Notes:
 * - reload_rq may be a misleading name as, in multiple selection mode,
 * we will load next stream, not reload the same one.
 *
 * - feed_fully_rendered is (or should be) only relevant in multiple selection
 * mode because there is no need to reload each time in single selection mode.
 *
 * - If reverse scrolling param set, actually shift to right.
 */
static int shift2left_callback()
{
	cairo_t		*cr;
	GdkRectangle	r;
	char		tmp[256];
	int		i;

	if (env->suspend_rq)
		return TRUE;	/* Does nothing - just return */
	else if ((env->shift_counter * prm->shift_size < env->surf_width - env->drwa_width) &&
			!env->reload_rq && !env->compute_rq) {
		env->suspend_rq = TRUE;
		/*
		 * Draw onto ticker area
		 * (We now use cairo instead of deprecated gdk_draw_ stuff.)
		 */
		/* Checking first can't hurt */
		if (env->c_surf == NULL)
			big_error(SHIFT2LEFT_NULL_CAIRO_IMAGE_SURFACE,
				"%s(): cairo image surface = NULL", __func__);
		/* Double buffering disabled for drawing area so we need this */
		r.x = 0;
		r.y = 0;
		r.width = env->drwa_width;
		r.height = env->drwa_height;
		gdk_window_begin_paint_rect(env->drw_a->window, &r);
		/* Cairo stuff */
		cr = gdk_cairo_create(GDK_DRAWABLE(env->drw_a->window));
		/* cairo_set_source_(): dest_x - src_x, dest_y - src_y */
		if (STANDARD_SCROLLING)
			cairo_set_source_surface(cr, env->c_surf, - (env->shift_counter++ * prm->shift_size), 0);
		else
			cairo_set_source_surface(cr, env->c_surf, - env->surf_width + env->drwa_width +
				(env->shift_counter++ * prm->shift_size), 0);
		/* cairo_rectangle(): dest_x, dest_y, src_w, src_h */
		cairo_rectangle(cr, 0, 0, env->drwa_width, env->drwa_height);
		cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
		cairo_fill(cr);
		cairo_destroy(cr);
		/* We're done */
		gdk_window_end_paint(env->drw_a->window);
		env->suspend_rq = FALSE;
		/*
		 * Speeding up / slowing down on the fly
		 */
		if (speed_up_flag == TRUE || slow_down_flag == TRUE) {
			if (speed_up_flag == TRUE) {
				speed_up_flag = FALSE;
				if (prm->delay > 1)
					prm->delay--;
				else
					return TRUE;
			} else if (slow_down_flag == TRUE) {
				slow_down_flag = FALSE;
				if (prm->delay < 50)
					prm->delay++;
				else
					return TRUE;
			}
			g_timeout_add_full(G_PRIORITY_DEFAULT, prm->delay, shift2left_callback, NULL, NULL);
			return FALSE;
		} else
			return TRUE;
	} else {
		/*
		 * When cairo_image surface scrolling completed or if request to
		 * reload or recompute everything, will create new surface or use next one.
		 */
		env->suspend_rq = TRUE;
		if (env->c_surf != NULL) {
			cairo_surface_destroy(env->c_surf);
			env->c_surf = NULL;
		}
		i = OK;
		if (env->feed_fully_rendered || env->reload_rq) {
			if (env->selection_mode == SINGLE) {
				if ((i = load_resource_from_selection(resrc, NULL)) != OK) {
					if (RESOURCE_ERROR_SSM_QUIT) {
						warning(BLOCK, "load_resource_from_selection() error: %s\n\n",
							"Will quit now (program was compiled with RESOURCE_ERROR_SSM_QUIT "
							"set to TRUE)", global_error_str(i));
						quit_with_cli_info(i);
					}
					/*
					 * Nothing more to do here as render_stream_to_surface() will handle
					 * rescr->fp = NULL.
					 */
				}
			} else while ((i = load_resource_from_selection(resrc, feed_selection)) != OK) {
				if (env->selection_mode == MULTIPLE && i == CONNECT_TOO_MANY_ERRORS) {
					if (get_params()->disable_popups == 'n') {
						snprintf(tmp, 256,
							"\nFailed to connect %d times in a row.\n\n"
							"Please check your internet connection and/or "
							"your connection settings.\n\n"
							"Switch to single selection mode ?\n",
							CONNECT_FAIL_MAX);
						if (question_win(tmp, -1) == YES) {
							env->selection_mode = SINGLE;
							break;
						}
					} else {
						fprintf(STD_ERR,
							"Failed to connect %d times in a row\n"
							"Please check your internet connection and/or "
							"your connection settings\n",
							CONNECT_FAIL_MAX);
						if (gtk_events_pending()) {
							if (gtk_main_iteration_do(FALSE))
								break;
						} else
#ifndef G_OS_WIN32
							sleep(CONNECT_FAIL_TIMEOUT);
#else
							Sleep(CONNECT_FAIL_TIMEOUT * 1000);
#endif
					}
				}
			}
			check_time_load_resource_from_selection(TIME_RESET);
		}
		compute_surface_and_win();
		env->shift_counter = prm->shift_size;	/* To avoid displaying twice the same thing */
		g_timeout_add_full(G_PRIORITY_DEFAULT, prm->delay, shift2left_callback, NULL, NULL);
		env->reload_rq = FALSE;
		env->compute_rq = FALSE;
		env->suspend_rq = FALSE;
		return FALSE;
	}
}

static int display_time0()
{
	display_time(prm);
	return TRUE;
}

static int get_win_position()	/* Only if win is 'draggable' */
{
	if (prm->windec == 'y')
		gtk_window_get_position(GTK_WINDOW(env->win), &prm->win_x, &prm->win_y);
	return TRUE;
}

/*
 * Called every second. Reload stuff every <resrc->rss_ttl * 60> seconds
 * but if prm->rss_refresh is set to 0.
 */
static void check_time_load_resource_from_selection(check_time_mode mode)
{
	static unsigned long elapsed_time_in_sec;

	if (mode == TIME_RESET) {
		elapsed_time_in_sec = 0;
		if (resrc->type == RESRC_FILE)
			resrc->rss_ttl = prm->rss_refresh;
	} else if (mode == TIME_CHECK) {
		if (prm->rss_refresh == 0) {
			elapsed_time_in_sec = 0;
			return;
		} else if ((++elapsed_time_in_sec) / 60 >= (unsigned long)resrc->rss_ttl) {
			elapsed_time_in_sec = 0;
			env->reload_rq = TRUE;
		}
	}
}

static int check_time_load_resource_from_selection0()
{
	check_time_load_resource_from_selection(TIME_CHECK);
	return TRUE;
}

static void print_help()
{
	int i;

	for (i = 0; get_help_str1()[i] != NULL; i++)
		fprintf(STD_OUT, "%s", get_help_str1()[i]);
	fprintf(STD_OUT, "\n");
}

static void print_version()
{
	fprintf(STD_OUT, APP_NAME " version " APP_V_NUM "\n");
}

static void print_license()
{
	int i;

	for (i = 0; get_license_str1()[i] != NULL; i++)
		fprintf(STD_OUT, "%s", get_license_str1()[i]);
	fprintf(STD_OUT, "%s\n", get_license_str2());
}

/*
 * Show up if no config file found
 */
static void welcome_at_first_run()
{
	if (!g_file_test(get_datafile_full_name_from_name(CONFIG_FILE), G_FILE_TEST_EXISTS) && !no_ui) {
		/* Useless and boring so disabled -
		quick_setup(prm);*/
		info_win("Welcome !",
			"  === Welcome to " APP_NAME " v" APP_V_NUM " ! ===\n\n"
			"- To open the main menu, right-click inside " APP_NAME " area.\n\n"
			"- You can import feed subscriptions with 'File > Import Feed List (OPML)',\n"
			"  for instance your Google Reader subscriptions.\n\n"
			"- To add a new feed, open 'File > Feed Organizer (RSS/Atom)', then look\n"
			"  for 'New Feed -> Enter URL' at the bottom of the window, click 'Clear'\n"
			"  and type or paste the feed URL.\n\n"
			"- To open a link in your browser, left-click on text.\n\n"
			"- Basically, use 'File > Feed Organizer (RSS|Atom)' to manage your feed\n"
			"  list, select feeds, subscribe to new ones, and 'Edit > Preferences' to\n"
			"  tweak " APP_NAME " appearance and behaviour.",
			INFO, FALSE);
		save_to_config_file(prm);	/* Save config to file so that this win will not appear again */
	}
}

/*
 * Recompute pixmap (from strings array, from opened stream), in order to
 * update all display params as soon as scrolling has started.
 */
static void update_pixmap_from_opened_stream()
{
	zboolean suspend_rq_bak = env->suspend_rq;

	env->suspend_rq = TRUE;

	env->feed_fully_rendered = TRUE;
	compute_surface_and_win();
	env->shift_counter = 0;

	env->suspend_rq = suspend_rq_bak;
}

static int do_at_startup()
{
	if (env->shift_counter > 4) {	/* Hmmm */
		env->suspend_rq = TRUE;
		update_pixmap_from_opened_stream();
		env->shift_counter = 0;
		env->suspend_rq = FALSE;
		return FALSE;
	} else
		return TRUE;
}

#ifdef G_OS_WIN32
/* This is meant to be used with the new GTK/GLib-win32 runtime. */
static int init_win32_mmtimer()
{
	TIMECAPS	tc;
	unsigned	int highest_res;

	timeGetDevCaps(&tc, sizeof(TIMECAPS));
	highest_res = tc.wPeriodMin;
	if (timeBeginPeriod(highest_res) == TIMERR_NOERROR)
		return 0;
	else {
		INFO_ERR("init_win32_mmtimer() error")
		return -1;
	}
}

/* Swap win32 log files every hour to prevent generating huge ones. */
static int swap_win32_logfiles()
{
	static unsigned long	elapsed_time_in_sec = 0;
	static int		counter = 0;
	int			suspend_rq_bak;
	time_t			time2 = time(NULL);

	if ((++elapsed_time_in_sec) >= (unsigned long)3600) {
		elapsed_time_in_sec = 0;
		counter++;
		counter &= 1;
		suspend_rq_bak = env->suspend_rq;
		env->suspend_rq = TRUE;
		if (STD_OUT != NULL) {
			if (fclose(STD_OUT) != 0)
				big_error(WIN32V_ERROR, "Can't close STD_OUT: %s", strerror(errno));
			STD_OUT = NULL;
		}
		if (STD_ERR != NULL) {
			if (fclose(STD_ERR) != 0)
				big_error(WIN32V_ERROR, "Can't close STD_ERR: %s", strerror(errno));
			STD_ERR = NULL;
		}
		if (counter == 0) {
			STD_OUT = open_new_datafile_with_name(STDOUT_FILENAME1, "wb");
			STD_ERR = open_new_datafile_with_name(STDERR_FILENAME1, "wb");
		} else {
			STD_OUT = open_new_datafile_with_name(STDOUT_FILENAME2, "wb");
			STD_ERR = open_new_datafile_with_name(STDERR_FILENAME2, "wb");
		}
		fprintf(STD_OUT, "%s", ctime(&time2));
		fprintf(STD_ERR, "%s", ctime(&time2));
		env->suspend_rq = suspend_rq_bak;
	}
	return TRUE;
}
#endif

/* TODO: is this still needed ? */
/*
 * Question at program start-up about new feed list format conversion:
 * if version >= 0.6.2 and feed list exists and feed list backup doesn't
 * exist, create backup and convert to new format.
 */
#define URL_LIST_BAK_FILE	URL_LIST_FILE "-bak"

static void test_convert_local_feed_list()
{
	FILE	*fp1, *fp2;
	char	*str;
	size_t	str_size = FGETS_STR_MAXLEN;

	if (strncmp(APP_V_NUM, "0.6.2", 5) >= 0)
		if (g_file_test(get_datafile_full_name_from_name(URL_LIST_FILE), G_FILE_TEST_EXISTS) &&\
				!g_file_test(get_datafile_full_name_from_name(URL_LIST_BAK_FILE), G_FILE_TEST_EXISTS))
			/* Now done automatically */
			/*if (question_win(APP_NAME " version 0.6.2 or later uses a new feed list format internally.\n"
					"Do you wish your local feed list to be checked and eventually converted ?\n"
					"(Doing this once is recommended.)", YES) == YES)*/ {
				fp1 = open_new_datafile_with_name(URL_LIST_FILE, "rb");
				fp2 = open_new_datafile_with_name(URL_LIST_BAK_FILE, "wb");
				str = malloc2(str_size * sizeof(char));
#ifndef G_OS_WIN32
				while (getline(&str, &str_size, fp1) != -1)
#else
				while (fgets(str, str_size, fp1) != NULL)
#endif
					fprintf(fp2, "%s", str);
				fclose(fp1);
				fclose(fp2);
				fp1 = open_new_datafile_with_name(URL_LIST_FILE, "wb");
				fp2 = open_new_datafile_with_name(URL_LIST_BAK_FILE, "rb");
#ifndef G_OS_WIN32
				while (getline(&str, &str_size, fp2) != -1)
#else
				while (fgets(str, str_size, fp2) != NULL)
#endif
					if (strncmp(str + 1, "http", 4) == 0 || strncmp(str + 1, "file", 4) == 0)
						fprintf(fp1, "%c   %s", str[0], str + 1);
					else
						fprintf(fp1, "%s", str);
				free2(str);
				fclose(fp1);
				fclose(fp2);
			}
}

void free_all()
{
	if (get_ticker_env() != NULL) {
		if (GTK_IS_WIDGET(env->win))
			gtk_widget_destroy(env->win);
		if (GTK_IS_WIDGET(popup_menu))
			gtk_widget_destroy(popup_menu);
	}
#ifdef G_OS_WIN32
	libetm_cleanup_win32_sockets();
#endif
	xmlCleanupParser();
	gnutls_global_deinit();
	if (get_resource() != NULL) {
		if (get_resource()->fp != NULL)
			fclose(get_resource()->fp);
		if (get_resource()->fp_extra != NULL)
			fclose(get_resource()->fp_extra);
	}
	free_all_render_strings_in_array();
	if (IS_FLIST(get_feed_list()))
		f_list_free_all(get_feed_list());
	if (IS_FLIST(get_feed_selection()))
		f_list_free_all(get_feed_selection());
	if (get_ticker_env() != NULL)
		free2(get_ticker_env());
	if (get_resource() != NULL)
		free2(get_resource());
	if (get_params() != NULL)
		free2(get_params());
}

void quit_with_cli_info(tickr_error_code error_code)
{
	free_all();
	fprintf(STD_ERR, "Try '" APP_CMD " --help' for more info\n");
	exit(error_code);
}

#ifndef G_OS_WIN32
void segfault_sig_handler(int sig_num, siginfo_t *sig_info, void *context)
{
	sig_info = sig_info;
	context = context;
	if (sig_num == SIGSEGV)
		big_error(SEGFAULT, global_error_str(SEGFAULT));
}
#endif

int main(int argc, char *argv[])
{
	GdkPixbuf		*pixb = NULL;
	GtkIconFactory		*icon_factory = NULL;
	GError			*error = NULL;
#ifndef G_OS_WIN32
	GdkColormap		*colormap = NULL;
	struct sigaction	action;
#else
	time_t			time2;
#endif
	int			n_options, n_resources, i, j;

	LIBXML_TEST_VERSION
#ifndef LIBXML_TREE_ENABLED
#  error "Libxml2: tree support not compiled in"
#endif
#if GNUTLS_VERSION_NUMBER < REQUIRED_GNUTLS_V_NUM
#  error "GnuTLS version 3.1.9 is required to compile"
#endif
	gtk_init(&argc, &argv);

#ifndef G_OS_WIN32
	/*
	 * segfault handling
	 */
	memset(&action, 0, sizeof(action));
	action.sa_sigaction = &segfault_sig_handler;
	action.sa_flags = SA_SIGINFO;
	if (sigaction(SIGSEGV, &action, NULL) != 0)
		INFO_ERR("sigaction() error: %s\n", strerror(errno))
#endif
	/*
	 * Init env, rescr, prm
	 */
	env = malloc2(sizeof(TickerEnv));
	resrc = malloc2(sizeof(Resource));
	prm = malloc2(sizeof(Params));
	env->win = env->drw_a = env->drwa_clock = NULL;
	env->c_surf = NULL,
	env->screen = NULL;
	env->visual = NULL;
	env->screen_w = env->screen_h = env->depth = 0;
	env->drwa_width = env->drwa_clock_width = env->drwa_height = 0;
	env->surf_width = env->surf_height = 0;
	env->shift_counter = 0;
	env->active_link[0] = '\0';
	env->mouse_x_in_drwa = -1;
	env->suspend_rq = env->compute_rq = env->reload_rq = FALSE;
	env->feed_fully_rendered = TRUE;
	/*
	 * env->selection_mode: default = MULTIPLE, set to SINGLE internally if
	 * selection is unavalaible/empty or after picking a single resource.
	 */
	env->selection_mode = MULTIPLE;
	/*
	 * Top level window
	 */
	env->win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(env->win), APP_NAME "-" APP_V_NUM);
	gtk_container_set_border_width(GTK_CONTAINER(env->win), 0);
	g_signal_connect(G_OBJECT(env->win), "delete_event", G_CALLBACK(gtk_main_quit), NULL);
	g_signal_connect(G_OBJECT(env->win), "destroy", G_CALLBACK(gtk_widget_destroy), &(env->win));

#ifdef G_OS_WIN32
	if (get_progfiles_dir() == NULL)
		big_error(WIN32V_ERROR, "Can't find Program Files directory");
	else if (get_appdata_dir() == NULL)
		big_error(WIN32V_ERROR, "Can't find Application Data directory");
	else if (init_win32_mmtimer() != 0)
		if (question_win("Can't initialize win32 multimedia timer. "
				"Scrolling may be very slow. Continue ?", -1) == NO) {
			free_all();
			exit(WIN32V_ERROR);
		}
#endif
	/*
	 * Create dir in user home dir if it doesn't exist.
	 */
	g_mkdir(get_datadir_full_path(), S_IRWXU);
#ifdef G_OS_WIN32
	/*
	 * Create win32 logfiles.
	 */
	libetm_init_win32_stdout_stderr(
		open_new_datafile_with_name(STDOUT_FILENAME1, "wb"),
		open_new_datafile_with_name(STDERR_FILENAME1, "wb"));
	/* Now, STD_OUT/ERR are initialized. */
	time2 = time(NULL);
	fprintf(STD_OUT, "%s", ctime(&time2));
	fprintf(STD_ERR, "%s", ctime(&time2));

	libetm_init_win32_sockets(STD_OUT, STD_ERR);
#endif
	xmlInitParser();
	/*
	 * Test HTTPS support
	 */
	env->https_support = FALSE;
	if ((i = gnutls_global_init()) != GNUTLS_E_SUCCESS)
		INFO_ERR("GnuTLS error: %s\n", gnutls_strerror(i))
	else if (gnutls_check_version(REQUIRED_GNUTLS_V_NUM_STR) == NULL)
		INFO_OUT("Can't enable HTTPS support (GnuTLS required version is %s "
			"but installed version is %s)\n" , REQUIRED_GNUTLS_V_NUM_STR,
			gnutls_check_version(NULL))
	else
		env->https_support = TRUE;

	init_render_string_array();
	/*
	 * Show help, version, license, dump misc info, set instance id, or run with no-ui.
	 */
	if (argc >= 2 && (strcmp(argv[1], "-help") == 0 || strcmp(argv[1], "--help") == 0 ||
			strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "-?") == 0)) {
		print_help();
		free_all();
		return 0;
	} else if (argc >= 2 && (strcmp(argv[1], "-version") == 0 || strcmp(argv[1], "--version") == 0 ||
			strcmp(argv[1], "-v") == 0)) {
		print_version();
		free_all();
		return 0;
	} else if (argc >= 2 && (strcmp(argv[1], "-license") == 0 || strcmp(argv[1], "--license") == 0 ||
			strcmp(argv[1], "-l") == 0)) {
		print_license();
		free_all();
		return 0;
	} else if (argc >= 2 && (strcmp(argv[1], "-dumpfontlist") == 0 || strcmp(argv[1], "--dumpfontlist") == 0)) {
		dump_font_list();
		free_all();
		return 0;
	} else if (argc >= 2 && (strcmp(argv[1], "-dumpconfig") == 0 || strcmp(argv[1], "--dumpconfig") == 0)) {
		dump_config();
		free_all();
		return 0;
	} else if (argc >= 2 && (strcmp(argv[1], "-dumperrorcodes") == 0 || strcmp(argv[1], "--dumperrorcodes") == 0)) {
		dump_error_codes();
		free_all();
		return 0;
	}
	/* instance-id must be the 1st arg to be effective */
	if (argc >= 2 && strncmp(argv[1], "-instance-id=", strlen("-instance-id=")) == 0) {
		if ((instance_id = atoi(argv[1] + strlen("-instance-id="))) < 1 || instance_id > 99) {
			INFO_ERR("Instance ID invalid value: %d\n", instance_id)
			instance_id = 0;
		}
	}
	/* We want "--instance-id=" to be valid too */
	if (argc >= 2 && strncmp(argv[1], "--instance-id=", strlen("--instance-id=")) == 0) {
		if ((instance_id = atoi(argv[1] + strlen("--instance-id="))) < 1 || instance_id > 99) {
			INFO_ERR("Instance ID invalid value: %d\n", instance_id)
			instance_id = 0;
		}
	}
	/* no-ui must be the 1st or 2nd arg to be effective */
	if ((argc >= 2 && strncmp(argv[1], "-no-ui", strlen("-no-ui")) == 0) ||
			(argc >= 3 && strncmp(argv[2], "-no-ui", strlen("-no-ui")) == 0))
		no_ui = TRUE;
	/* We want "--no-ui" to be valid too */
	if ((argc >= 2 && strncmp(argv[1], "--no-ui", strlen("--no-ui")) == 0) ||
			(argc >= 3 && strncmp(argv[2], "--no-ui", strlen("--no-ui")) == 0))
		no_ui = TRUE;
	set_tickr_icon_to_dialog(GTK_WINDOW(env->win));
	/*
	 * Create rss icon
	 */
	pixb = gdk_pixbuf_new_from_file(get_imagefile_full_name_from_name(RSS_ICON), &error);
	icon_factory = gtk_icon_factory_new();
	gtk_icon_factory_add(icon_factory, RSS_ICON, gtk_icon_set_new_from_pixbuf(pixb));
	gtk_icon_factory_add_default(icon_factory);
	if (pixb != NULL)
		g_object_unref(pixb);
	/*
	 * Get some system graphical info
	 */
	env->screen = gtk_widget_get_screen(env->win);
#ifndef G_OS_WIN32
	if ((colormap = gdk_screen_get_rgba_colormap(env->screen)) != NULL) {
#ifdef VERBOSE_OUTPUT
		if (gdk_screen_is_composited(env->screen))
			INFO_OUT("Composited GDK screen\n")
#endif
	} else
		colormap = gdk_screen_get_rgb_colormap(env->screen);
	gtk_widget_set_colormap(env->win, colormap);
	env->visual = gdk_colormap_get_visual(colormap);
#else
	env->visual = gdk_visual_get_best();
#endif
	env->depth = env->visual->depth;
	env->screen_w = gdk_screen_get_width(gtk_window_get_screen(GTK_WINDOW(env->win)));
	env->screen_h = gdk_screen_get_height(gtk_window_get_screen(GTK_WINDOW(env->win)));
#ifdef VERBOSE_OUTPUT
	INFO_OUT("GDK visual: screen width = %d, screen height = %d, depth = %d\n",
		env->screen_w, env->screen_h, env->depth)
#ifndef G_OS_WIN32
	INFO_OUT("Window manager: %s\n", gdk_x11_screen_get_window_manager_name(env->screen))
#endif
#endif
	resrc->type = RESRC_UNSPECIFIED;
	resrc->format = RSS_FORMAT_UNDETERMINED;
	resrc->id[0] = '\0';
	resrc->orig_url[0] = '\0';
	resrc->xml_dump[0] = '\0';
	resrc->fp = NULL;
	resrc->fp_extra = NULL;
	for (i = 0; i < NFEEDLINKANDOFFSETMAX; i++) {
		resrc->link_and_offset[i].url[0] = '\0';
		resrc->link_and_offset[i].offset_in_surface = 0;
	}
	init_authentication();
	init_proxy();
	/*
	 * Options and resource stuff
	 */
	set_default_options(prm);	/* Init all params */
	if (g_file_test(get_datafile_full_name_from_name(CONFIG_FILE), G_FILE_TEST_EXISTS))
		get_config_file_options(prm);
	else
		welcome_at_first_run();
	n_options = 0;
	n_resources = 0;
	if (argc > 1) {
		/* Parse args: first -options, then resource - the rest is ignored. */
		for (i = 1; i < argc; i++)
			if (argv[i][0] == '-')
				n_options ++;
			else
				break;
		if (i < argc)
			n_resources++;
		if (n_resources == 0) {
			INFO_OUT("%s\n", global_error_str(NO_RESOURCE_SPECIFIED))
			if (RESOURCE_ERROR_SSM_QUIT) {
				INFO_ERR("Exiting (program was compiled with "
					"RESOURCE_ERROR_SSM_QUIT set to TRUE)\n")
				quit_with_cli_info(NO_RESOURCE_SPECIFIED);
			}
		} else {
			env->selection_mode = SINGLE;
			/* Will get rss feed or open text file */
			str_n_cpy(resrc->id, argv[n_options + 1], FILE_NAME_MAXLEN);
		}
		if (n_options > 0)
			if ((i = parse_options_array(prm, n_options, (const char **)(argv + 1)) != OK))
				if (OPTION_ERROR_QUIT) {
					INFO_ERR("Exiting (program was compiled with "
						"OPTION_ERROR_QUIT set to TRUE)\n")
					quit_with_cli_info(i);
				}
	} else {
		INFO_OUT("%s\n", global_error_str(NO_RESOURCE_SPECIFIED))
		if (RESOURCE_ERROR_SSM_QUIT) {
			INFO_ERR("Exiting (program was compiled with "
				"RESOURCE_ERROR_SSM_QUIT set to TRUE)\n")
			quit_with_cli_info(NO_RESOURCE_SPECIFIED);
		}
	}
	/* Apply connection settings */
	compute_auth_and_proxy_str();
	set_connect_timeout_sec(prm->connect_timeout);
	set_send_recv_timeout_sec(prm->send_recv_timeout);

#ifdef VERBOSE_OUTPUT
	/* Here so to be able to check the disablepopups option */
	if (no_ui)
		warning(BLOCK, "%s will run with the 'no-ui' option.\n"
			"Opening of UI elements which can modify settings and/or \n"
			"URL list/selection is disabled.", APP_NAME);
#endif
	/*
	 * Feed list stuff
	 */
	test_convert_local_feed_list();		/* Still needed ? Probably not but just in case */
	if ((i = f_list_load_from_file(&feed_list, NULL)) == OK) {
		feed_list = f_list_sort(feed_list);
		feed_list = f_list_first(feed_list);
		f_list_save_to_file(feed_list, NULL);
	} else if (i == LOAD_URL_LIST_NO_LIST) {
		if (question_win("No URL list has been saved yet. Import one (OPML format required) ?", -1) == YES) {
			if ((i = import_opml_file()) == OK)
				manage_list_and_selection(resrc);
		} else if (question_win("No URL list has been saved yet. Use sample one ?", -1) == YES) {
			if ((i = f_list_load_from_file(&feed_list, get_sample_url_list_full_name())) == OK) {
				feed_list = f_list_sort(feed_list);
				feed_list = f_list_first(feed_list);
				f_list_save_to_file(feed_list, NULL);
				manage_list_and_selection(resrc);
			}
		}
	}
	j = build_feed_selection_from_feed_list();
	if (n_resources == 0 && env->selection_mode == MULTIPLE && i == OK && j == OK)
		INFO_OUT("Will use feed selection\n")
	else {
		if (env->selection_mode == MULTIPLE && (i != OK || j != OK)) {
			if (j == SELECTION_EMPTY)
				warning(NO_BLOCK, "No feed selected - "
					"Switching to single selection mode");
			else if (j == SELECTION_ERROR)
				warning(NO_BLOCK, "No feed selection available - "
					"Switching to single selection mode");
		}
		env->selection_mode = SINGLE;
		if (n_resources == 0) {
			if (prm->homefeed[0] != '\0') {
				str_n_cpy(resrc->id, prm->homefeed, FILE_NAME_MAXLEN);
				INFO_OUT("Will use default resource (Homefeed): %s\n", resrc->id)
			} else {
				INFO_ERR("No default resource available\n")
				if (RESOURCE_ERROR_SSM_QUIT) {
					INFO_ERR("Exiting (program was compiled with "
						"RESOURCE_ERROR_SSM_QUIT set to TRUE)\n")
					quit_with_cli_info(NO_RESOURCE_SPECIFIED);
				}
			}
		}
	}
	if ((i = load_resource_from_selection(resrc, feed_selection)) != OK)
		if (RESOURCE_ERROR_SSM_QUIT) {
			INFO_ERR("Exiting (program was compiled with "
				"RESOURCE_ERROR_SSM_QUIT set to TRUE)\n")
			quit_with_cli_info(i);
		}
	/* ... to here */
	resrc->rss_ttl = prm->rss_refresh;
	first_feed();
	/*
	 * Set window layout
	 */
	main_hbox = gtk_hbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(env->win), main_hbox);
	popup_menu = get_popup_menu_menu(NULL);
	popup_menu_accel_group_attached = FALSE;
	/*
	 * Create 2 drawing areas
	 */
	env->drw_a = gtk_drawing_area_new();
	/*
	 * We disable double buffering for drawing area and will use
	 * gdk_window_begin_paint_rect() and gdk_window_end_paint().
	 */
	gtk_widget_set_double_buffered(env->drw_a, FALSE);
	env->drwa_clock = gtk_drawing_area_new();
	gtk_widget_add_events(env->drw_a, GDK_ALL_EVENTS_MASK);	/* Change mask ? */

	g_signal_connect(G_OBJECT(env->drw_a), "button-press-event", G_CALLBACK(left_click_on_drawing_area), NULL);
	g_signal_connect(G_OBJECT(env->drw_a), "button-press-event", G_CALLBACK(right_click_on_drawing_area), NULL);
	g_signal_connect(G_OBJECT(env->drw_a), "enter-notify-event", G_CALLBACK(mouse_over_drawing_area), NULL);
	g_signal_connect(G_OBJECT(env->drw_a), "leave-notify-event", G_CALLBACK(mouse_over_drawing_area), NULL);
	g_signal_connect(G_OBJECT(popup_menu), "expose-event", G_CALLBACK(mouse_over_opened_popup_menu), NULL);
	g_signal_connect(G_OBJECT(popup_menu), "leave-notify-event", G_CALLBACK(mouse_over_opened_popup_menu), NULL);
	g_signal_connect(G_OBJECT(env->drw_a), "scroll-event", G_CALLBACK(mouse_wheel_action_on_drawing_area), NULL);
	g_signal_connect(G_OBJECT(env->drw_a), "motion-notify-event", G_CALLBACK(track_mouse_position_x), NULL);

	vbox_ticker = gtk_vbox_new(FALSE, 0);	/* vbox_ticker is only used to have a v-centered ticker */
	gtk_box_pack_start(GTK_BOX(vbox_ticker), GTK_WIDGET(env->drw_a), FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(main_hbox), vbox_ticker, FALSE, FALSE, 0);
	vbox_clock = gtk_vbox_new(FALSE, 0);	/* vbox_clock is only used to have a v-centered clock */
	gtk_box_pack_start(GTK_BOX(vbox_clock), GTK_WIDGET(env->drwa_clock), TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(main_hbox), vbox_clock, FALSE, FALSE, 0);
	/*
	 * Create cairo image surface from text file or rss feed and compute stuff to draw window.
	 */
	compute_surface_and_win();

	/* === Timeout callbacks === */
	/*
	 * Refresh the image every <delay> milliseconds.
	 */
	g_timeout_add_full(G_PRIORITY_DEFAULT, prm->delay, shift2left_callback, NULL, NULL);
	/*
	 * Display clock every 0.5 seconds.
	 */
	g_timeout_add_full(G_PRIORITY_DEFAULT, 500, display_time0, NULL, NULL);
	/*
	 * Get current win position (if 'dragged') every 0.5 seconds.
	 */
	g_timeout_add_full(G_PRIORITY_DEFAULT, 500, get_win_position, NULL, NULL);
	/*
	 * Check every sec until elapsed time in mn > <resrc->rss_ttl> mn then reload rss feed (or text file).
	 */
	check_time_load_resource_from_selection(TIME_RESET);
	g_timeout_add_full(G_PRIORITY_DEFAULT, 1000, check_time_load_resource_from_selection0, NULL, NULL);
	/*
	 * Regularly update Tickr window dims and location
	 */
	g_timeout_add_full(G_PRIORITY_DEFAULT, 500, update_win_dims_and_loc, NULL, NULL);

	/*
	 * Called at program startup only
	 */
	g_timeout_add_full(G_PRIORITY_DEFAULT, 100, do_at_startup, NULL, NULL);
	/*
	 * Check every sec until elapsed time >= 1 hour then swap win32 log files to prevent generating huge ones.
	 */
#ifdef G_OS_WIN32
	g_timeout_add_full(G_PRIORITY_DEFAULT, 1000, swap_win32_logfiles, NULL, NULL);
#endif

	env->suspend_rq = FALSE;
	env->compute_rq = FALSE;
	env->reload_rq = TRUE;

	gtk_widget_show_all(env->win);
	/* To get rid of starting 'ghost' square win - but is it effective ? */
	gtk_widget_set_size_request(env->win, 1, 1);
	gtk_window_resize(GTK_WINDOW(env->win), 1, 1);
	gtk_main();
	/*
	 * Cleanup stuff
	 */
	free_all();
	return 0;
}
