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

#define BLANK_STR_2_SP	"  "
#define BLANK_STR_8_SP	"        "

/* These variables and widgets are all set as static global for convenience */
static TickerEnv	*env;
static char		current_url[FILE_NAME_MAXLEN + 1];

static GtkWidget	*dialog, *sc_win, *entry_homefeed, *table;
static GtkWidget	*cancel_but, *reset_but, *apply_but, *ok_but;
static GtkWidget	*fullsettings_but, *connsettings_but;
static GtkWidget	*top_but, *bottom_but, *fullwidth_but, *curfeed_but;
static GtkWidget	*spinbut_delay, *spinbut_shiftsize;
static GtkWidget	*spinbut_winx, *spinbut_winy, *spinbut_winw, *spinbut_winh;
static GtkWidget	*checkbut_disablescreenlimits;
static GtkWidget	*checkbut_setgradientbg, *checkbut_setclockgradientbg;
static GtkWidget	*checkbut_windec, *checkbut_iconintaskbar, *checkbut_winsticky;
static GtkWidget	*spinbut_shadowoffsetx, *spinbut_shadowoffsety;
static GtkWidget	*spinbut_shadowfx, *spinbut_rssrefresh;
static GtkWidget	*spinbut_wintransparency, *spinbut_nitemsperfeed;
static GtkWidget	*checkbut_shadow, *checkbut_alwaysontop, *checkbut_overrideredirect;
static GtkWidget	*checkbut_windec, *checkbut_spchars, *checkbut_revsc;
static GtkWidget	*checkbut_feedtitle, *checkbut_itemtitle, *checkbut_itemdes;
static GtkWidget	*checkbut_striptags, *checkbut_uppercase;
static GtkWidget	*checkbut_clocksec, *checkbut_clock12h, *checkbut_clockdate;
static GtkWidget	*checkbut_clockaltdateform;
static GtkWidget	*checkbut_nopopups, *checkbut_mouseover, *checkbut_noleftclick;
static GtkWidget	*checkbut_sfpickercloseswhenpointerleaves, *checkbut_feedordering;
static GtkWidget	*font_but, *clock_font_but;
static GtkWidget	*fg_color_but, /* *highlight_fg_color_but,*/ *bg_color_but, *bg_color2_but;
static GtkWidget	*clock_fg_color_but, *clock_bg_color_but, *clock_bg_color2_but;
/*static GtkWidget	*rbut_box_mi, *radio_but_mi_1, *radio_but_mi_2, *radio_but_mi_3;*/
static GtkWidget	*rbut_box_clock, *radio_but_clock_1, *radio_but_clock_2, *radio_but_clock_3;
static GtkWidget	*rbut_box_mw, *radio_but_mw_1, *radio_but_mw_2, *radio_but_mw_3;
static GtkWidget	*entry_linedel, *entry_newpagech, *entry_tabch;
static GtkWidget	*entry_feedtitledel, *entry_itemtitledel, *entry_itemdesdel;
static GtkWidget	*entry_openlinkcmd, *entry_openlinkargs;
static GtkObject	*adj_delay, *adj_shiftsize, *adj_winx, *adj_winy;
static GtkObject	*adj_winw, *adj_winh, *adj_shadowoffsetx, *adj_shadowfx;
static GtkObject	*adj_shadowoffsety, *adj_rssrefresh;
static GtkObject	*adj_wintransparency, *adj_nitemsperfeed;

/* Need this prototype */
static void set_ui_changes_to_params(Params *, zboolean);

static int update_win_x_y_w_adj(GtkWidget *widget) {
	zboolean disable_screen_limits;

	widget = widget;
	if ((zboolean)gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkbut_disablescreenlimits)))
		disable_screen_limits = TRUE;
	else
		disable_screen_limits = FALSE;
	gtk_adjustment_set_upper(GTK_ADJUSTMENT(adj_winx), disable_screen_limits ? WIN_MAX_X : env->screen_w);
	gtk_adjustment_set_upper(GTK_ADJUSTMENT(adj_winy), disable_screen_limits ? WIN_MAX_Y : env->screen_h);
	gtk_adjustment_set_upper(GTK_ADJUSTMENT(adj_winw), disable_screen_limits ? WIN_MAX_W : env->screen_w);
	gtk_adjustment_changed(GTK_ADJUSTMENT(adj_winx));
	gtk_adjustment_changed(GTK_ADJUSTMENT(adj_winy));
	gtk_adjustment_changed(GTK_ADJUSTMENT(adj_winw));
	gtk_spin_button_update(GTK_SPIN_BUTTON(spinbut_winx));
	gtk_spin_button_update(GTK_SPIN_BUTTON(spinbut_winy));
	gtk_spin_button_update(GTK_SPIN_BUTTON(spinbut_winw));
	return TRUE;
}

static int move_to_top(GtkWidget *widget)
{
	widget = widget;
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(spinbut_winy), 0);
	return TRUE;
}

static int move_to_bottom(GtkWidget *widget)
{
	char	font_name_size[FONT_MAXLEN + 1];
	char	font_name[FONT_NAME_MAXLEN + 1], font_size[FONT_SIZE_MAXLEN + 1];
	int	requested_font_size, requested_h, y_bottom;
	/* We backup this param first because... */
	char	disable_popups_bak = get_params()->disable_popups;

	/* ...we want this window to always popup */
	get_params()->disable_popups = 'n';

	widget = widget;
	/* We need to know requested ticker height */
	requested_h = gtk_spin_button_get_value(GTK_SPIN_BUTTON(spinbut_winh));
	if (requested_h > 0 && requested_h < DRWA_HEIGHT_MIN)
		requested_h = DRWA_HEIGHT_MIN;
	else if (requested_h == 0) {
		/* Compute ticker height from requested font size */
		str_n_cpy(font_name_size, (char *)gtk_font_button_get_font_name(GTK_FONT_BUTTON(font_but)), FONT_MAXLEN);
		split_font(font_name_size, font_name, font_size);
		/* In all cases, font size can't be > FONT_MAXSIZE */
		requested_font_size =  MIN(atoi(font_size), FONT_MAXSIZE);
		snprintf(font_size, FONT_SIZE_MAXLEN + 1, "%3d", requested_font_size);
		compact_font(font_name_size, font_name, font_size);
		requested_h = get_layout_height_from_font_name_size(font_name_size);
	}

	y_bottom = env->screen_h - requested_h;
#ifndef G_OS_WIN32
	/* How to get taskbar height on Linux ? */
	warning(BLOCK, "Taskbar height: Will use %d pixels arbitrary value - "
		"Please adjust as necessary afterwards", ARBITRARY_TASKBAR_HEIGHT);
	y_bottom -= ARBITRARY_TASKBAR_HEIGHT;
#else
	if (get_win32_taskbar_height() != -1)
		y_bottom -= get_win32_taskbar_height();
	else {
		warning(BLOCK, "Couldn't compute Taskbar height: Will use %d pixels arbitrary value - "
			"Please adjust as necessary afterwards", ARBITRARY_TASKBAR_HEIGHT);
		y_bottom -= ARBITRARY_TASKBAR_HEIGHT;
	}
#endif
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(spinbut_winy), y_bottom);

	get_params()->disable_popups = disable_popups_bak;
	return TRUE;
}

static int set_full_width(GtkWidget *widget)
{
	widget = widget;
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(spinbut_winw), env->screen_w);
	/* Will also push ticker in left corner */
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(spinbut_winx), 0);
	return TRUE;
}

static int get_current_url(GtkWidget *widget)
{
	widget = widget;
	gtk_entry_set_text(GTK_ENTRY(entry_homefeed), current_url);
	return TRUE;
}

GtkLabel *bg_color2_label;

static int check_set_gradient_widgets(GtkWidget *widget)
{
	widget = widget;
	if ((zboolean)gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkbut_setgradientbg))) {
		gtk_widget_set_sensitive(GTK_WIDGET(bg_color2_label), TRUE);
		gtk_widget_set_sensitive(GTK_WIDGET(bg_color2_but), TRUE);
	} else {
		gtk_widget_set_sensitive(GTK_WIDGET(bg_color2_label), FALSE);
		gtk_widget_set_sensitive(GTK_WIDGET(bg_color2_but), FALSE);
	}
	return TRUE;
}

GtkLabel *newpagech_label, *tabch_label;

static int check_set_sp_chars_widgets(GtkWidget *widget)
{
	widget = widget;
	if ((zboolean)gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkbut_spchars))) {
		gtk_widget_set_sensitive(GTK_WIDGET(newpagech_label), TRUE);
		gtk_widget_set_sensitive(GTK_WIDGET(entry_newpagech), TRUE);
		gtk_widget_set_sensitive(GTK_WIDGET(tabch_label), TRUE);
		gtk_widget_set_sensitive(GTK_WIDGET(entry_tabch), TRUE);
	} else {
		gtk_widget_set_sensitive(GTK_WIDGET(newpagech_label), FALSE);
		gtk_widget_set_sensitive(GTK_WIDGET(entry_newpagech), FALSE);
		gtk_widget_set_sensitive(GTK_WIDGET(tabch_label), FALSE);
		gtk_widget_set_sensitive(GTK_WIDGET(entry_tabch), FALSE);
	}
	return TRUE;
}

GtkLabel *clock_sec_label, *clock_12h_label, *clock_date_label, *clock_altdateform_label, *clock_font_label,
	*clock_fg_color_label, *clock_bg_color_label, *setclockgradientbg_label, *clock_bg_color2_label;

static int check_set_clock_widgets(GtkWidget *widget)
{
	widget = widget;
	if (!(zboolean)gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_but_clock_3))) {	/* ! (clock = none) */
		gtk_widget_set_sensitive(GTK_WIDGET(clock_sec_label), TRUE);
		gtk_widget_set_sensitive(GTK_WIDGET(checkbut_clocksec), TRUE);
		gtk_widget_set_sensitive(GTK_WIDGET(clock_12h_label), TRUE);
		gtk_widget_set_sensitive(GTK_WIDGET(checkbut_clock12h), TRUE);
		gtk_widget_set_sensitive(GTK_WIDGET(clock_date_label), TRUE);
		gtk_widget_set_sensitive(GTK_WIDGET(checkbut_clockdate), TRUE);
		if ((zboolean)gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkbut_clockdate))) {
			gtk_widget_set_sensitive(GTK_WIDGET(clock_altdateform_label), TRUE);
			gtk_widget_set_sensitive(GTK_WIDGET(checkbut_clockaltdateform), TRUE);
		} else {
			gtk_widget_set_sensitive(GTK_WIDGET(clock_altdateform_label), FALSE);
			gtk_widget_set_sensitive(GTK_WIDGET(checkbut_clockaltdateform), FALSE);
		}
		gtk_widget_set_sensitive(GTK_WIDGET(clock_font_label), TRUE);
		gtk_widget_set_sensitive(GTK_WIDGET(clock_font_but), TRUE);
		gtk_widget_set_sensitive(GTK_WIDGET(clock_fg_color_label), TRUE);
		gtk_widget_set_sensitive(GTK_WIDGET(clock_fg_color_but), TRUE);
		gtk_widget_set_sensitive(GTK_WIDGET(clock_bg_color_label), TRUE);
		gtk_widget_set_sensitive(GTK_WIDGET(clock_bg_color_but), TRUE);
		gtk_widget_set_sensitive(GTK_WIDGET(setclockgradientbg_label), TRUE);
		gtk_widget_set_sensitive(GTK_WIDGET(checkbut_setclockgradientbg), TRUE);
		gtk_widget_set_sensitive(GTK_WIDGET(clock_bg_color2_label), TRUE);
		gtk_widget_set_sensitive(GTK_WIDGET(clock_bg_color2_but), TRUE);
	} else {
		gtk_widget_set_sensitive(GTK_WIDGET(clock_sec_label), FALSE);
		gtk_widget_set_sensitive(GTK_WIDGET(checkbut_clocksec), FALSE);
		gtk_widget_set_sensitive(GTK_WIDGET(clock_12h_label), FALSE);
		gtk_widget_set_sensitive(GTK_WIDGET(checkbut_clock12h), FALSE);
		gtk_widget_set_sensitive(GTK_WIDGET(clock_date_label), FALSE);
		gtk_widget_set_sensitive(GTK_WIDGET(checkbut_clockdate), FALSE);
		gtk_widget_set_sensitive(GTK_WIDGET(clock_altdateform_label), FALSE);
		gtk_widget_set_sensitive(GTK_WIDGET(checkbut_clockaltdateform), FALSE);
		gtk_widget_set_sensitive(GTK_WIDGET(clock_font_label), FALSE);
		gtk_widget_set_sensitive(GTK_WIDGET(clock_font_but), FALSE);
		gtk_widget_set_sensitive(GTK_WIDGET(clock_fg_color_label), FALSE);
		gtk_widget_set_sensitive(GTK_WIDGET(clock_fg_color_but), FALSE);
		gtk_widget_set_sensitive(GTK_WIDGET(clock_bg_color_label), FALSE);
		gtk_widget_set_sensitive(GTK_WIDGET(clock_bg_color_but), FALSE);
		gtk_widget_set_sensitive(GTK_WIDGET(setclockgradientbg_label), FALSE);
		gtk_widget_set_sensitive(GTK_WIDGET(checkbut_setclockgradientbg), FALSE);
		gtk_widget_set_sensitive(GTK_WIDGET(clock_bg_color2_label), FALSE);
		gtk_widget_set_sensitive(GTK_WIDGET(clock_bg_color2_but), FALSE);
	}
	return TRUE;
}

/* For alignment of GTK widgets */
#define LEFT		0
#define RIGHT		1
#define EXPAND		2

static GtkLabel *table_attach_styled_double_cell(GtkWidget *table2, char *text1, char *text2, char *tooltip,
	GtkWidget *w1, GtkWidget *w2, GtkWidget *w3, int column1, int column2, int row, int alignment1, int alignment2)
{
	GtkWidget	*hbox, *label;
	char		*label_text, *tooltip_text;

	/* Label in column 1 */
	label_text = l_str_new(text1);
	if (text2 != NULL) {
		label_text = l_str_cat(label_text, " <small>(");
		label_text = l_str_cat(label_text, text2);
		label_text = l_str_cat(label_text, ")</small>:" BLANK_STR_2_SP);
	} else
		label_text = l_str_cat(label_text, ":" BLANK_STR_2_SP);
	label = gtk_label_new(label_text);
	gtk_label_set_use_markup(GTK_LABEL(label), TRUE);
	gtk_misc_set_alignment(GTK_MISC(label), alignment1, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(table2), label, column1, column1 + 1, row, row + 1);
	l_str_free(label_text);

	/* Widgets in column 2 */
	hbox = gtk_hbox_new(FALSE, 0);
	if (alignment2 == RIGHT)
		gtk_box_pack_start(GTK_BOX(hbox), gtk_label_new(NULL), TRUE, TRUE, 0);
	if (w1 != NULL)
		gtk_box_pack_start(GTK_BOX(hbox), w1, alignment2 == EXPAND ? TRUE : FALSE,
			alignment2 == EXPAND ? TRUE : FALSE, 0);
	if (w2 != NULL)
		gtk_box_pack_start(GTK_BOX(hbox), w2, alignment2 == EXPAND ? TRUE : FALSE,
			alignment2 == EXPAND ? TRUE : FALSE, 0);
	if (w3 != NULL)
		gtk_box_pack_start(GTK_BOX(hbox), w3, alignment2 == EXPAND ? TRUE : FALSE,
			alignment2 == EXPAND ? TRUE : FALSE, 0);
	if (alignment2 == LEFT)
		gtk_box_pack_start(GTK_BOX(hbox), gtk_label_new(NULL), TRUE, TRUE, 0);
	gtk_table_attach_defaults(GTK_TABLE(table2), hbox, column2, column2 + 1, row, row + 1);

	/* Tooltip */
	tooltip_text = l_str_new(tooltip);
	gtk_widget_set_tooltip_text(label, tooltip_text);
	gtk_widget_set_tooltip_text(hbox, tooltip_text);
	l_str_free(tooltip_text);
	return GTK_LABEL(label);
}

/*
 * CHECK THIS:
 * - If font size is changed, tickr height is set to 0
 * - If tickr height is changed, font size is adjusted
 * -> ????
 * if (gtk_spin_button_get_value(GTK_SPIN_BUTTON(spinbut_winh)) > 0)
 */

/* Window layout (kind of) */
#ifdef COLUMN_1_ALIGN
#  undef COLUMN_1_ALIGN
#endif
#ifdef COLUMN_2_ALIGN
#  undef COLUMN_2_ALIGN
#endif
#ifdef ROW_SPACING
#  undef ROW_SPACING
#endif
#define COLUMN_1_ALIGN		RIGHT
#define COLUMN_2_ALIGN		LEFT
#define ROW_SPACING		5

/*
 * Open a dialog to edit a few useful params. (This is actually a stripped
 * down, slightly modified version of modify_params()).
 * Return next action requested (if any).
 */
next_action easily_modify_params(Params *prm)
{
	int		response;
	zboolean	changes_have_been_made;
	int		do_next = DO_NEXT_NOTHING;
	Params		*prm_bak;
	int		i;

	env = get_ticker_env();
	str_n_cpy(current_url, get_resource()->id, FILE_NAME_MAXLEN);

	gtk_window_set_keep_above(GTK_WINDOW(env->win), FALSE);

	dialog = gtk_dialog_new_with_buttons(
			"Preferences", GTK_WINDOW(env->win),
		 	GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
		 	NULL);

	set_tickr_icon_to_dialog(GTK_WINDOW(dialog));
	gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);
	gtk_window_set_resizable(GTK_WINDOW(dialog), FALSE);

	g_signal_connect(G_OBJECT(dialog), "key-press-event", G_CALLBACK(esc_key_pressed), NULL);
	g_signal_connect(G_OBJECT(dialog), "delete_event", G_CALLBACK(force_quit_dialog), NULL);

	fullsettings_but = gtk_dialog_add_button(GTK_DIALOG(dialog), "Full\nSettings", GTK_RESPONSE_FULL_SETTINGS);
	connsettings_but = gtk_dialog_add_button(GTK_DIALOG(dialog), "Connection\nSettings", GTK_RESPONSE_CONN_SETTINGS);
	cancel_but = gtk_dialog_add_button(GTK_DIALOG(dialog), GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL_CLOSE);
	reset_but = gtk_dialog_add_button(GTK_DIALOG(dialog), "Reset", GTK_RESPONSE_RESET);
	/* We only have 'OK', which apply ***and*** save settings. */
	ok_but = gtk_dialog_add_button(GTK_DIALOG(dialog), GTK_STOCK_OK, GTK_RESPONSE_OK);
	apply_but = gtk_dialog_add_button(GTK_DIALOG(dialog), GTK_STOCK_APPLY, GTK_RESPONSE_APPLY);

	gtk_widget_set_tooltip_text(fullsettings_but, "Full settings minus connection settings");
	gtk_widget_set_tooltip_text(ok_but, "Apply, save and quit");
	gtk_widget_set_tooltip_text(apply_but, "Don't save and don't quit");

	table = gtk_table_new(12, 3, FALSE);
	gtk_table_set_row_spacings(GTK_TABLE(table), ROW_SPACING);
	gtk_table_set_col_spacings(GTK_TABLE(table), 0);
	gtk_container_set_border_width(GTK_CONTAINER(table), 15);
	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), table);
	gtk_table_attach_defaults(GTK_TABLE(table), gtk_label_new(BLANK_STR_8_SP), 1, 2, 0, 1);

	i = 0;
#ifdef COLUMN_1
#  undef COLUMN_1
#endif
#ifdef COLUMN_2
#  undef COLUMN_2
#endif
#define COLUMN_1	0
#define COLUMN_2	2
	/*
	 * Delay
	 */
	adj_delay = gtk_adjustment_new(prm->delay, 1, 50, 1, 5, 0);
	spinbut_delay = gtk_spin_button_new(GTK_ADJUSTMENT(adj_delay), 0.0, 0);
	table_attach_styled_double_cell(table, "Delay", "Milliseconds",
		APP_NAME " speed = K1 / scrolling delay\n- Decrease delay to speed up scrolling\n"
		"- Increase delay to slow down scrolling",
		spinbut_delay, NULL, NULL, COLUMN_1, COLUMN_2, i, COLUMN_1_ALIGN, COLUMN_2_ALIGN);
	i++;
	/*
	 * Font
	 */
	font_but = gtk_font_button_new_with_font(prm->font_name_size);
	table_attach_styled_double_cell(table, "Font", NULL,
		"WARNING: Font size will be overridden by " APP_NAME " height, if height > 0",
		font_but, NULL, NULL, COLUMN_1, COLUMN_2, i, COLUMN_1_ALIGN, COLUMN_2_ALIGN);
	i++;
	/*
	 * fg color
	 */
	fg_color_but = gtk_color_button_new_with_color(&prm->fg_color);
	gtk_color_button_set_use_alpha(GTK_COLOR_BUTTON(fg_color_but), TRUE);
	gtk_color_button_set_alpha(GTK_COLOR_BUTTON(fg_color_but), prm->fg_color_alpha);
	table_attach_styled_double_cell(table, "Foreground Color", NULL, NULL,
		fg_color_but, NULL, NULL, COLUMN_1, COLUMN_2, i, COLUMN_1_ALIGN, COLUMN_2_ALIGN);
	i++;
	/*
	 * bg color
	 */
	bg_color_but = gtk_color_button_new_with_color(&prm->bg_color);
	gtk_color_button_set_use_alpha(GTK_COLOR_BUTTON(bg_color_but), TRUE);
	gtk_color_button_set_alpha(GTK_COLOR_BUTTON(bg_color_but), prm->bg_color_alpha);
	table_attach_styled_double_cell(table, "Background Color", NULL, NULL,
		bg_color_but, NULL, NULL, COLUMN_1, COLUMN_2, i, COLUMN_1_ALIGN, COLUMN_2_ALIGN);
	i++;
	/*
	 * Set gradient bg
	 */
	checkbut_setgradientbg = gtk_check_button_new();
	if (prm->set_gradient_bg == 'y')
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbut_setgradientbg), TRUE);
	else if (prm->set_gradient_bg == 'n')
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbut_setgradientbg), FALSE);
	table_attach_styled_double_cell(table, "Use a gradient background", NULL, NULL,
		checkbut_setgradientbg, NULL, NULL, COLUMN_1, COLUMN_2, i, COLUMN_1_ALIGN, COLUMN_2_ALIGN);
	i++;
	/*
	 * bg color2
	 */
	bg_color2_but = gtk_color_button_new_with_color(&prm->bg_color2);
	gtk_color_button_set_use_alpha(GTK_COLOR_BUTTON(bg_color2_but), TRUE);
	gtk_color_button_set_alpha(GTK_COLOR_BUTTON(bg_color2_but), prm->bg_color2_alpha);
	bg_color2_label = table_attach_styled_double_cell(table, "Background Color2", NULL, NULL,
		bg_color2_but, NULL, NULL, COLUMN_1, COLUMN_2, i, COLUMN_1_ALIGN, COLUMN_2_ALIGN);
	i++;
	/*
	 * win_y
	 */
	adj_winy = gtk_adjustment_new(prm->win_y, 0,
		(prm->disable_screen_limits == 'y' ? WIN_MAX_Y : env->screen_h), 1, 5, 0);
	spinbut_winy = gtk_spin_button_new(GTK_ADJUSTMENT(adj_winy), 0.0, 0);
	top_but = gtk_button_new_with_label("Top");
	bottom_but = gtk_button_new_with_label("Bottom");
	table_attach_styled_double_cell(table, "Vertical Location", "Pixels", NULL,
		top_but, bottom_but, spinbut_winy, COLUMN_1, COLUMN_2, i, COLUMN_1_ALIGN, COLUMN_2_ALIGN);
	i++;
	/*
	 * win_h
	 */
	adj_winh = gtk_adjustment_new(prm->win_h, 0, env->screen_h, 1, 5, 0);
	spinbut_winh = gtk_spin_button_new(GTK_ADJUSTMENT(adj_winh), 0.0, 0);
	table_attach_styled_double_cell(table, APP_NAME " Height", "Pixels",
		"WARNING: If > 0, will override font size",
		spinbut_winh, NULL, NULL, COLUMN_1, COLUMN_2, i, COLUMN_1_ALIGN, COLUMN_2_ALIGN);
	i++;
	/*
	 * Window always-on-top
	 */
	checkbut_alwaysontop = gtk_check_button_new();
	if (prm->always_on_top == 'y')
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbut_alwaysontop), TRUE);
	else if (prm->always_on_top == 'n')
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbut_alwaysontop), FALSE);
	table_attach_styled_double_cell(table, APP_NAME " always above other windows", NULL, NULL,
		checkbut_alwaysontop, NULL, NULL, COLUMN_1, COLUMN_2, i, COLUMN_1_ALIGN, COLUMN_2_ALIGN);
	i++;
	/*
	 * Reverse scrolling
	 */
	checkbut_revsc = gtk_check_button_new();
	if (prm->reverse_sc == 'y')
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbut_revsc), TRUE);
	else if (prm->reverse_sc == 'n')
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbut_revsc), FALSE);

	table_attach_styled_double_cell(table, "Reverse scrolling", NULL,
		"For languages written/read from R to L",
		checkbut_revsc, NULL, NULL, COLUMN_1, COLUMN_2, i, COLUMN_1_ALIGN, COLUMN_2_ALIGN);
	i++;
	/*
	 * Read n items max per feed
	 */
	adj_nitemsperfeed = gtk_adjustment_new(prm->n_items_per_feed, 0, 500, 1, 5, 0);
	spinbut_nitemsperfeed = gtk_spin_button_new(GTK_ADJUSTMENT(adj_nitemsperfeed), 0.0, 0);
	table_attach_styled_double_cell(table, "Read N items max per feed", NULL, "0 = no limit",
		spinbut_nitemsperfeed, NULL, NULL, COLUMN_1, COLUMN_2, i, COLUMN_1_ALIGN, COLUMN_2_ALIGN);
	i++;
	/*
	 * Mouse wheel scrolling behaviour
	 */
	radio_but_mw_1 = gtk_radio_button_new_with_label(NULL, "Speed");
	radio_but_mw_2 = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio_but_mw_1), "Feed");
	radio_but_mw_3 = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio_but_mw_1), "None");
	rbut_box_mw = gtk_hbox_new(TRUE, 2);
	gtk_box_pack_start(GTK_BOX(rbut_box_mw), radio_but_mw_1, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(rbut_box_mw), radio_but_mw_2, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(rbut_box_mw), radio_but_mw_3, TRUE, TRUE, 0);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_but_mw_1), TRUE);
	if (prm->mouse_wheel_action == 's')
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_but_mw_1), TRUE);
	else if (prm->mouse_wheel_action == 'f')
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_but_mw_2), TRUE);
	else
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_but_mw_3), TRUE);
	table_attach_styled_double_cell(table, "Mouse Wheel acts on", NULL,
		"Use <ctrl> Mouse Wheel for alternative action",
		rbut_box_mw, NULL, NULL, COLUMN_1, COLUMN_2, i, COLUMN_1_ALIGN, COLUMN_2_ALIGN);

	/* Connect buttons / widgets to callbacks */
	g_signal_connect(G_OBJECT(checkbut_setgradientbg), "clicked", G_CALLBACK(check_set_gradient_widgets), NULL);
	g_signal_connect(G_OBJECT(top_but), "clicked", G_CALLBACK(move_to_top), NULL);
	g_signal_connect(G_OBJECT(bottom_but), "clicked", G_CALLBACK(move_to_bottom), NULL);

	/* This must be run once at least. */
	check_set_gradient_widgets(NULL);

	/* This will let us know if changes have been made. */
	prm_bak = malloc2(sizeof(Params));
	memcpy((void *)prm_bak, (const void *)prm, sizeof(Params));
	changes_have_been_made = FALSE;

	gtk_widget_grab_focus(cancel_but);
	gtk_widget_show_all(dialog);
	gtk_window_set_focus(GTK_WINDOW(dialog), NULL);

	while (1) {
		response = gtk_dialog_run(GTK_DIALOG(dialog));
		env->suspend_rq = TRUE;
		if (response == GTK_RESPONSE_RESET) {
			if (question_win("Reset *all* settings to default values ?\n"
					"(Your current settings will be lost)", NO) == YES) {
				set_default_options(prm);
				save_to_config_file(prm);
				current_feed();
				env->compute_rq = TRUE;
				/*do_next = DO_NEXT_REOPEN;	// Not used anymore */
				break;
			} else
				continue;
		} else if (response == GTK_RESPONSE_FULL_SETTINGS || response == GTK_RESPONSE_APPLY ||
				response == GTK_RESPONSE_OK) {
			set_ui_changes_to_params(prm, FALSE);
		}
		if (response == GTK_RESPONSE_FULL_SETTINGS) {
			do_next = DO_NEXT_OPEN_FULL_SETTINGS;
			break;
		} else if (response == GTK_RESPONSE_CONN_SETTINGS) {
			if (question_win("Save changes you (eventually) made before leaving ?", -1) == YES)
				set_ui_changes_to_params(prm, FALSE);
			do_next = DO_NEXT_OPEN_CONN_SETTINGS;
			break;
		} else if (response == GTK_RESPONSE_APPLY) {	/* Force apply */
			/*
			 * We want a new pixmap so that changes will be effective... now!
			 *
			 * Some setting changes (like 'read n items per feed') need the stream
			 * to be reloaded
			 */
			current_feed();
			changes_have_been_made = TRUE;
			env->suspend_rq = FALSE;
		} else if (response == GTK_RESPONSE_OK) {
			if (prm->item_title == 'n' && prm->item_description == 'n') {
				warning(BLOCK, "%s\n%s", "You can't uncheck both 'Item title' and 'Item description'",
					"(because no much useful information would be displayed)");
				continue;
			}
			/* Apply and save when necessary */
			if (memcmp((const void *)prm, (const void *)prm_bak, sizeof(Params)) != 0 || \
					changes_have_been_made) {
				/*
				 * Some setting changes (like 'read n items per feed') need the stream
				 * to be reloaded
				 */
				current_feed();
				save_to_config_file(prm);
			}
			break;
		} else {
			/* Restore and apply when necessary */
			if (memcmp((const void *)prm, (const void *)prm_bak, sizeof(Params)) != 0 || \
					changes_have_been_made) {
				memcpy((void *)prm, (const void *)prm_bak, sizeof(Params));
				/*
				 * Some setting changes (like 'read n items per feed') need the stream
				 * to be reloaded
				 */
				current_feed();
			}
			break;
		}
	}
	free2(prm_bak);
	gtk_widget_destroy(dialog);
	check_main_win_always_on_top();
	env->suspend_rq = FALSE;
	return do_next;
}

/* Window layout (kind of) */
#ifdef COLUMN_1_ALIGN
#  undef COLUMN_1_ALIGN
#endif
#ifdef COLUMN_2_ALIGN
#  undef COLUMN_2_ALIGN
#endif
#ifdef ROW_SPACING
#  undef ROW_SPACING
#endif
#define COLUMN_1_ALIGN		LEFT
#define COLUMN_2_ALIGN		EXPAND/*RIGHT*/
#define ROW_SPACING		5

/*
 * Open a dialog to edit all params.
 * Return next action requested (if any).
 */
next_action modify_params(Params *prm)
{
	char		c[2]= "c";
	int		response;
	zboolean	changes_have_been_made;
	int		do_next = DO_NEXT_NOTHING;
	Params		*prm_bak;
	int		i;

	env = get_ticker_env();
	str_n_cpy(current_url, get_resource()->id, FILE_NAME_MAXLEN);

	gtk_window_set_keep_above(GTK_WINDOW(env->win), FALSE);

	dialog = gtk_dialog_new_with_buttons(
			"Full Settings", GTK_WINDOW(env->win),
		 	GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
		 	NULL);

	set_tickr_icon_to_dialog(GTK_WINDOW(dialog));
	gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);

	g_signal_connect(G_OBJECT(dialog), "key-press-event", G_CALLBACK(esc_key_pressed), NULL);
	g_signal_connect(G_OBJECT(dialog), "delete_event", G_CALLBACK(force_quit_dialog), NULL);

	sc_win = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sc_win), GTK_POLICY_AUTOMATIC,
		GTK_POLICY_AUTOMATIC);
	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), sc_win);

	connsettings_but = gtk_dialog_add_button(GTK_DIALOG(dialog), "Connection Settings", GTK_RESPONSE_CONN_SETTINGS);
	cancel_but = gtk_dialog_add_button(GTK_DIALOG(dialog), GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL_CLOSE);
	reset_but = gtk_dialog_add_button(GTK_DIALOG(dialog), "Reset", GTK_RESPONSE_RESET);
	/* We only have 'OK', which apply ***and*** save settings. */
	ok_but = gtk_dialog_add_button(GTK_DIALOG(dialog), GTK_STOCK_OK, GTK_RESPONSE_OK);
	apply_but = gtk_dialog_add_button(GTK_DIALOG(dialog), GTK_STOCK_APPLY, GTK_RESPONSE_APPLY);

	gtk_widget_set_tooltip_text(ok_but, "Apply, save and quit");
	gtk_widget_set_tooltip_text(apply_but, "Don't save and don't quit");

	table = gtk_table_new(17, 5, FALSE);
	gtk_table_set_row_spacings(GTK_TABLE(table), ROW_SPACING);
	gtk_table_set_col_spacings(GTK_TABLE(table), 0);
	gtk_container_set_border_width(GTK_CONTAINER(table), 15);
	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(sc_win), table);
	/* Whole window must be visible on netbooks as well */
	gtk_widget_set_size_request(sc_win, 1000, 450);

	i = 0;
#ifdef COLUMN_1
#  undef COLUMN_1
#endif
#ifdef COLUMN_2
#  undef COLUMN_2
#endif
#define COLUMN_1	0
#define COLUMN_2	1
	/*
	 * Delay
	 */
	adj_delay = gtk_adjustment_new(prm->delay, 1, 50, 1, 5, 0);
	spinbut_delay = gtk_spin_button_new(GTK_ADJUSTMENT(adj_delay), 0.0, 0);
	table_attach_styled_double_cell(table, "Delay", "Milliseconds",
		APP_NAME " speed = K1 / scrolling delay\n- Decrease delay to speed up scrolling\n"
		"- Increase delay to slow down scrolling",
		spinbut_delay, NULL, NULL, COLUMN_1, COLUMN_2, i, COLUMN_1_ALIGN, COLUMN_2_ALIGN);
	i++;
	/*
	 * Shift size
	 */
	adj_shiftsize = gtk_adjustment_new(prm->shift_size, 1, 200, 1, 5, 0);
	spinbut_shiftsize = gtk_spin_button_new(GTK_ADJUSTMENT(adj_shiftsize), 0.0, 0);
	table_attach_styled_double_cell(table, "Shift size", "Pixels",
		APP_NAME " speed = K2 x shift size\nYou can increase shift size and delay simultaneously "
		"to reduce CPU load although scrolling will get less smooth",
		spinbut_shiftsize, NULL, NULL, COLUMN_1, COLUMN_2, i, COLUMN_1_ALIGN, COLUMN_2_ALIGN);
	i++;
	/*
	 * Font
	 */
	font_but = gtk_font_button_new_with_font(prm->font_name_size);
	table_attach_styled_double_cell(table, "Font", "Size can't be > 200",		/* Check this is up to date */
		"WARNING: Font size will be overridden by " APP_NAME " height, if height > 0",
		font_but, NULL, NULL, COLUMN_1, COLUMN_2, i, COLUMN_1_ALIGN, COLUMN_2_ALIGN);
	i++;
	/*
	 * fg color
	 */
	fg_color_but = gtk_color_button_new_with_color(&prm->fg_color);
	gtk_color_button_set_use_alpha(GTK_COLOR_BUTTON(fg_color_but), TRUE);
	gtk_color_button_set_alpha(GTK_COLOR_BUTTON(fg_color_but), prm->fg_color_alpha);
	table_attach_styled_double_cell(table, "Foreground Color", NULL, NULL,
		fg_color_but, NULL, NULL, COLUMN_1, COLUMN_2, i, COLUMN_1_ALIGN, COLUMN_2_ALIGN);
	i++;
	/*
	 * Marked items fg color
	 */
	/*highlight_fg_color_but = gtk_color_button_new_with_color(&prm->highlight_fg_color);
	gtk_color_button_set_use_alpha(GTK_COLOR_BUTTON(highlight_fg_color_but), TRUE);
	gtk_color_button_set_alpha(GTK_COLOR_BUTTON(highlight_fg_color_but), prm->highlight_fg_color_alpha);
	table_attach_styled_double_cell(table, "Marked items foreground color", NULL, NULL,
		highlight_fg_color_but, NULL, NULL, COLUMN_1, COLUMN_2, i, COLUMN_1_ALIGN, COLUMN_2_ALIGN);
	i++;*/
	/*
	 * bg color
	 */
	bg_color_but = gtk_color_button_new_with_color(&prm->bg_color);
	gtk_color_button_set_use_alpha(GTK_COLOR_BUTTON(bg_color_but), TRUE);
	gtk_color_button_set_alpha(GTK_COLOR_BUTTON(bg_color_but), prm->bg_color_alpha);
	table_attach_styled_double_cell(table, "Background Color", NULL, NULL,
		bg_color_but, NULL, NULL, COLUMN_1, COLUMN_2, i, COLUMN_1_ALIGN, COLUMN_2_ALIGN);
	i++;
	/*
	 * Set gradient bg
	 */
	checkbut_setgradientbg = gtk_check_button_new();
	if (prm->set_gradient_bg == 'y')
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbut_setgradientbg), TRUE);
	else if (prm->set_gradient_bg == 'n')
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbut_setgradientbg), FALSE);
	table_attach_styled_double_cell(table, "Use a gradient background", NULL, NULL,
		checkbut_setgradientbg, NULL, NULL, COLUMN_1, COLUMN_2, i, COLUMN_1_ALIGN, COLUMN_2_ALIGN);
	i++;
	/*
	 * bg color2
	 */
	bg_color2_but = gtk_color_button_new_with_color(&prm->bg_color2);
	gtk_color_button_set_use_alpha(GTK_COLOR_BUTTON(bg_color2_but), TRUE);
	gtk_color_button_set_alpha(GTK_COLOR_BUTTON(bg_color2_but), prm->bg_color2_alpha);
	bg_color2_label = table_attach_styled_double_cell(table, "Background Color2", NULL, NULL,
		bg_color2_but, NULL, NULL, COLUMN_1, COLUMN_2, i, COLUMN_1_ALIGN, COLUMN_2_ALIGN);
	i++;
	/*
	 * Disable screen limits
	 */
	checkbut_disablescreenlimits = gtk_check_button_new();
	if (prm->disable_screen_limits == 'y')
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbut_disablescreenlimits), TRUE);
	else if (prm->disable_screen_limits == 'n')
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbut_disablescreenlimits), FALSE);
	table_attach_styled_double_cell(table, "Disable screen limits", NULL,
		"Apply to " APP_NAME " X, Y positions and width",
		checkbut_disablescreenlimits, NULL, NULL, COLUMN_1, COLUMN_2, i, COLUMN_1_ALIGN, COLUMN_2_ALIGN);
	i++;
	/*
	 * win_x
	 */
	adj_winx = gtk_adjustment_new(prm->win_x, 0,
		(prm->disable_screen_limits == 'y' ? WIN_MAX_X : env->screen_w - 20), 1, 5, 0);		/* Magic value '20' found here */
	spinbut_winx = gtk_spin_button_new(GTK_ADJUSTMENT(adj_winx), 0.0, 0);
	table_attach_styled_double_cell(table, "X position", "Pixels", NULL,
		spinbut_winx, NULL, NULL, COLUMN_1, COLUMN_2, i, COLUMN_1_ALIGN, COLUMN_2_ALIGN);
	i++;
	/*
	 * win_y
	 */
	adj_winy = gtk_adjustment_new(prm->win_y, 0,
		(prm->disable_screen_limits == 'y' ? WIN_MAX_Y : env->screen_h), 1, 5, 0);
	spinbut_winy = gtk_spin_button_new(GTK_ADJUSTMENT(adj_winy), 0.0, 0);
	top_but = gtk_button_new_with_label("Top");
	bottom_but = gtk_button_new_with_label("Bottom");
	table_attach_styled_double_cell(table, "Y position", "Pixels", NULL,
		top_but, bottom_but, spinbut_winy, COLUMN_1, COLUMN_2, i, COLUMN_1_ALIGN, COLUMN_2_ALIGN);
	i++;
	/*
	 * win_w
	 */
	adj_winw = gtk_adjustment_new(prm->win_w, DRWA_WIDTH_MIN,
		(prm->disable_screen_limits == 'y' ? WIN_MAX_W : env->screen_w), 1, 5, 0);
	spinbut_winw = gtk_spin_button_new(GTK_ADJUSTMENT(adj_winw), 0.0, 0);
	fullwidth_but = gtk_button_new_with_label("Full width");
	table_attach_styled_double_cell(table, "Width", "Pixels", NULL,
		fullwidth_but, spinbut_winw, NULL, COLUMN_1, COLUMN_2, i, COLUMN_1_ALIGN, COLUMN_2_ALIGN);
	i++;
	/*
	 * win_h
	 */
	adj_winh = gtk_adjustment_new(prm->win_h, 0, env->screen_h, 1, 5, 0);
	spinbut_winh = gtk_spin_button_new(GTK_ADJUSTMENT(adj_winh), 0.0, 0);
	table_attach_styled_double_cell(table, APP_NAME " Height", "Pixels",
		"WARNING: If > 0, will override font size",
		spinbut_winh, NULL, NULL, COLUMN_1, COLUMN_2, i, COLUMN_1_ALIGN, COLUMN_2_ALIGN);
	i++;
	/*
	 * windec
	 */
	checkbut_windec = gtk_check_button_new();
	if (prm->windec == 'y')
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbut_windec), TRUE);
	else if (prm->windec == 'n')
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbut_windec), FALSE);
	table_attach_styled_double_cell(table, "Window decoration", NULL, NULL,
		checkbut_windec, NULL, NULL, COLUMN_1, COLUMN_2, i, COLUMN_1_ALIGN, COLUMN_2_ALIGN);
	i++;
	/*
	 * Window always-on-top
	 */
	checkbut_alwaysontop = gtk_check_button_new();
	if (prm->always_on_top == 'y')
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbut_alwaysontop), TRUE);
	else if (prm->always_on_top == 'n')
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbut_alwaysontop), FALSE);
	table_attach_styled_double_cell(table, "Window Always-On-Top", NULL, NULL,
		checkbut_alwaysontop, NULL, NULL, COLUMN_1, COLUMN_2, i, COLUMN_1_ALIGN, COLUMN_2_ALIGN);
	i++;
	/*
	 * Window transparency
	 */
	/* transparency set from gui ranges from 1 (0.1) to 10 (1.0) / 0.0 (invisible!) is kind of "useless" */
	adj_wintransparency = gtk_adjustment_new(prm->win_transparency * 10, 1, 10, 1, 2, 0);
	spinbut_wintransparency = gtk_spin_button_new(GTK_ADJUSTMENT(adj_wintransparency), 0.0, 0);
	table_attach_styled_double_cell(table, "Window opacity", "x 10", NULL,
		spinbut_wintransparency, NULL, NULL, COLUMN_1, COLUMN_2, i, COLUMN_1_ALIGN, COLUMN_2_ALIGN);
	i++;
	/*
	 * Icon in taskbar
	 */
	checkbut_iconintaskbar = gtk_check_button_new();
	if (prm->icon_in_taskbar == 'y')
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbut_iconintaskbar), TRUE);
	else if (prm->icon_in_taskbar == 'n')
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbut_iconintaskbar), FALSE);
	table_attach_styled_double_cell(table, "Icon in Taskbar", NULL, NULL,
		checkbut_iconintaskbar, NULL, NULL, COLUMN_1, COLUMN_2, i, COLUMN_1_ALIGN, COLUMN_2_ALIGN);
	i++;
	/*
	 * Window sticky
	 */
	checkbut_winsticky = gtk_check_button_new();
	if (prm->win_sticky == 'y')
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbut_winsticky), TRUE);
	else if (prm->win_sticky == 'n')
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbut_winsticky), FALSE);
	table_attach_styled_double_cell(table, "Visible on all User Desktops", NULL, NULL,
		checkbut_winsticky, NULL, NULL, COLUMN_1, COLUMN_2, i, COLUMN_1_ALIGN, COLUMN_2_ALIGN);
	i++;
	/*
	 * Set window override_redirect flag
	 */
	checkbut_overrideredirect = gtk_check_button_new();
	if (prm->override_redirect == 'y')
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbut_overrideredirect), TRUE);
	else if (prm->override_redirect == 'n')
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbut_overrideredirect), FALSE);
	table_attach_styled_double_cell(table, "Set override_redirect flag", "Warning:\n***EXPERIMENTAL***",
		APP_NAME " will bypass window manager, which may lead to unexpected behaviour - "
		"You've been warned ;)",
		checkbut_overrideredirect, NULL, NULL, COLUMN_1, COLUMN_2, i, COLUMN_1_ALIGN, COLUMN_2_ALIGN);
	i++;
	/*
	 * Shadow
	 */
	checkbut_shadow = gtk_check_button_new();
	if (prm->shadow == 'y')
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbut_shadow), TRUE);
	else if (prm->shadow == 'n')
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbut_shadow), FALSE);
	table_attach_styled_double_cell(table, "Shadow", NULL, NULL,
		checkbut_shadow, NULL, NULL, COLUMN_1, COLUMN_2, i, COLUMN_1_ALIGN, COLUMN_2_ALIGN);
	i++;
	/*
	 * shadow_offset_x
	 */
	adj_shadowoffsetx = gtk_adjustment_new(prm->shadow_offset_x, -20, 20, 1, 5, 0);
	spinbut_shadowoffsetx = gtk_spin_button_new(GTK_ADJUSTMENT(adj_shadowoffsetx), 0.0, 0);
	table_attach_styled_double_cell(table, "Shadow x offset", "Pixels", NULL,
		spinbut_shadowoffsetx, NULL, NULL, COLUMN_1, COLUMN_2, i, COLUMN_1_ALIGN, COLUMN_2_ALIGN);
	i++;
	/*
	 * shadow_offset_y
	 */
	adj_shadowoffsety = gtk_adjustment_new(prm->shadow_offset_y, -20, 20, 1, 5, 0);
	spinbut_shadowoffsety = gtk_spin_button_new(GTK_ADJUSTMENT(adj_shadowoffsety), 0.0, 0);
	table_attach_styled_double_cell(table, "Shadow y offset", "Pixels", NULL,
		spinbut_shadowoffsety, NULL, NULL, COLUMN_1, COLUMN_2, i, COLUMN_1_ALIGN, COLUMN_2_ALIGN);
	i++;
	/*
	 * shadow_fx
	 */
	adj_shadowfx = gtk_adjustment_new(prm->shadow_fx, 0, 10, 1, 5, 0);
	spinbut_shadowfx = gtk_spin_button_new(GTK_ADJUSTMENT(adj_shadowfx), 0.0, 0);
	table_attach_styled_double_cell(table, "Shadow fx", "0 = none -> 10 = full", NULL,
		spinbut_shadowfx, NULL, NULL, COLUMN_1, COLUMN_2, i, COLUMN_1_ALIGN, COLUMN_2_ALIGN);
	i++;
	/*
	 * Line delimiter
	 */
	entry_linedel = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(entry_linedel), DELIMITER_MAXLEN);
	gtk_entry_set_text(GTK_ENTRY(entry_linedel), prm->line_delimiter);
	table_attach_styled_double_cell(table, "Line delimiter", NULL,
		"String that will replace every newline occurrence",
		entry_linedel, NULL, NULL, COLUMN_1, COLUMN_2, i, COLUMN_1_ALIGN, COLUMN_2_ALIGN);
	i++;
	/*
	 * Enable special chars
	 */
	checkbut_spchars = gtk_check_button_new();
	if (prm->special_chars == 'y')
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbut_spchars), TRUE);
	else if (prm->special_chars == 'n')
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbut_spchars), FALSE);
	table_attach_styled_double_cell(table, "Special characters enabled", NULL,
		"Apply only to text files",
		checkbut_spchars, NULL, NULL, COLUMN_1, COLUMN_2, i, COLUMN_1_ALIGN, COLUMN_2_ALIGN);
	i++;
	/*
	 * 'new page' special char
	 */
	entry_newpagech = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(entry_newpagech), 1);
	/* This does nothing - gtk_entry_set_width_chars(GTK_ENTRY(entry_newpagech), 1);*/
	c[0] = prm->new_page_char;
	gtk_entry_set_text(GTK_ENTRY(entry_newpagech), c);
	newpagech_label = table_attach_styled_double_cell(table, "'New page' special character", NULL, NULL,
		entry_newpagech, NULL, NULL, COLUMN_1, COLUMN_2, i, COLUMN_1_ALIGN, COLUMN_2_ALIGN);
	i++;
	/*
	 * 'tab' special char
	 */
	entry_tabch = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(entry_tabch), 1);
	/* This does nothing - gtk_entry_set_width_chars(GTK_ENTRY(entry_tabch), 1);*/
	c[0] = prm->tab_char;
	gtk_entry_set_text(GTK_ENTRY(entry_tabch), c);
	tabch_label = table_attach_styled_double_cell(table, "'Tab' (8 spaces) special character", NULL, NULL,
		entry_tabch, NULL, NULL, COLUMN_1, COLUMN_2, i, COLUMN_1_ALIGN, COLUMN_2_ALIGN);
	i++;
	/*
	 * Reload delay (rss refresh) - Now up to 1440 mn = 24 h
	 */
	adj_rssrefresh = gtk_adjustment_new(prm->rss_refresh, 0, 1440, 1, 5, 0);
	spinbut_rssrefresh = gtk_spin_button_new(GTK_ADJUSTMENT(adj_rssrefresh), 0.0, 0);
	table_attach_styled_double_cell(table, "Reload delay", "Minutes",
		"Delay before reloading resource (URL or text file.)\n"
		"0 = never force reload - Otherwise, apply only if no TTL inside "
		"feed or if resource is text file.\n"
		"(Actually, in multiple selections mode, all feeds are always reloaded "
		"sequentially, because there is no caching involved)",
		spinbut_rssrefresh, NULL, NULL, COLUMN_1, COLUMN_2, i, COLUMN_1_ALIGN, COLUMN_2_ALIGN);
	i++;
	/*
	 * Reverse scrolling
	 */
	checkbut_revsc = gtk_check_button_new();
	if (prm->reverse_sc == 'y')
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbut_revsc), TRUE);
	else if (prm->reverse_sc == 'n')
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbut_revsc), FALSE);
	table_attach_styled_double_cell(table, "Reverse scrolling", NULL,
		"For languages written/read from R to L",
		checkbut_revsc, NULL, NULL, COLUMN_1, COLUMN_2, i, COLUMN_1_ALIGN, COLUMN_2_ALIGN);
	i++;

/* Commenting out this block would change from 2 columns to 1 single column window */
	gtk_table_attach_defaults(GTK_TABLE(table),
		gtk_label_new(BLANK_STR_8_SP BLANK_STR_8_SP BLANK_STR_8_SP), 2, 3, 0, 1);
	i = 0;
#ifdef COLUMN_1
#  undef COLUMN_1
#endif
#ifdef COLUMN_2
#  undef COLUMN_2
#endif
#define COLUMN_1	3
#define COLUMN_2	4
/* (commenting out until here) */

	/*
	 * Feed title
	 */
	checkbut_feedtitle = gtk_check_button_new();
	if (prm->feed_title == 'y')
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbut_feedtitle), TRUE);
	else if (prm->feed_title == 'n')
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbut_feedtitle), FALSE);
	table_attach_styled_double_cell(table, "Show feed title", NULL, NULL,
		checkbut_feedtitle, NULL, NULL, COLUMN_1, COLUMN_2, i, COLUMN_1_ALIGN, COLUMN_2_ALIGN);
	i++;
	/*
	 * Feed title delimiter
	 */
	entry_feedtitledel = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(entry_feedtitledel), DELIMITER_MAXLEN);
	gtk_entry_set_text(GTK_ENTRY(entry_feedtitledel), prm->feed_title_delimiter);
	table_attach_styled_double_cell(table, "Feed title delimiter", NULL, NULL,
		entry_feedtitledel, NULL, NULL, COLUMN_1, COLUMN_2, i, COLUMN_1_ALIGN, COLUMN_2_ALIGN);
	i++;
	/*
	 * Item title
	 */
	checkbut_itemtitle = gtk_check_button_new();
	if (prm->item_title == 'y')
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbut_itemtitle), TRUE);
	else if (prm->item_title == 'n')
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbut_itemtitle), FALSE);
	table_attach_styled_double_cell(table, "Show item title", NULL, NULL,
		checkbut_itemtitle, NULL, NULL, COLUMN_1, COLUMN_2, i, COLUMN_1_ALIGN, COLUMN_2_ALIGN);
	i++;
	/*
	 * Item title delimiter
	 */
	entry_itemtitledel = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(entry_itemtitledel), DELIMITER_MAXLEN);
	gtk_entry_set_text(GTK_ENTRY(entry_itemtitledel), prm->item_title_delimiter);
	table_attach_styled_double_cell(table, "Item title delimiter", NULL, NULL,
		entry_itemtitledel, NULL, NULL, COLUMN_1, COLUMN_2, i, COLUMN_1_ALIGN, COLUMN_2_ALIGN);
	i++;
	/*
	 * Item description
	 */
	checkbut_itemdes = gtk_check_button_new();
	if (prm->item_description == 'y')
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbut_itemdes), TRUE);
	else if (prm->item_description == 'n')
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbut_itemdes), FALSE);
	table_attach_styled_double_cell(table, "Show item description", NULL, NULL,
		checkbut_itemdes, NULL, NULL, COLUMN_1, COLUMN_2, i, COLUMN_1_ALIGN, COLUMN_2_ALIGN);
	i++;
	/*
	 * Item description delimiter
	 */
	entry_itemdesdel = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(entry_itemdesdel), DELIMITER_MAXLEN);
	gtk_entry_set_text(GTK_ENTRY(entry_itemdesdel), prm->item_description_delimiter);
	table_attach_styled_double_cell(table, "Item description delimiter", NULL, NULL,
		entry_itemdesdel, NULL, NULL, COLUMN_1, COLUMN_2, i, COLUMN_1_ALIGN, COLUMN_2_ALIGN);
	i++;
	/*
	 * Read n items max per feed
	 */
	adj_nitemsperfeed = gtk_adjustment_new(prm->n_items_per_feed, 0, 500, 1, 5, 0);
	spinbut_nitemsperfeed = gtk_spin_button_new(GTK_ADJUSTMENT(adj_nitemsperfeed), 0.0, 0);
	table_attach_styled_double_cell(table, "Read N items max per feed", NULL, "0 = no limit",
		spinbut_nitemsperfeed, NULL, NULL, COLUMN_1, COLUMN_2, i, COLUMN_1_ALIGN, COLUMN_2_ALIGN);
	i++;
	/*
	 * Mark item action
	 */
	/*radio_but_mi_1 = gtk_radio_button_new_with_label(NULL, "Hide");
	radio_but_mi_2 = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio_but_mi_1), "Color");
	radio_but_mi_3 = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio_but_mi_1), "None");
	rbut_box_mi = gtk_hbox_new(TRUE, 2);
	gtk_box_pack_start(GTK_BOX(rbut_box_mi), radio_but_mi_1, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(rbut_box_mi), radio_but_mi_2, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(rbut_box_mi), radio_but_mi_3, TRUE, TRUE, 0);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_but_mi_1), TRUE);
	if (prm->mark_item_action == 'h')
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_but_mi_1), TRUE);
	else if (prm->mark_item_action == 'c')
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_but_mi_2), TRUE);
	else
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_but_mi_3), TRUE);
	table_attach_styled_double_cell(table, "Mark item action", NULL, NULL,
		rbut_box_mi, NULL, NULL, COLUMN_1, COLUMN_2, i, COLUMN_1_ALIGN, COLUMN_2_ALIGN);
	i++;*/
	/*
	 * Remove html tags
	 */
	checkbut_striptags = gtk_check_button_new();
	if (prm->strip_html_tags == 'y')
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbut_striptags), TRUE);
	else if (prm->strip_html_tags == 'n')
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbut_striptags), FALSE);
	table_attach_styled_double_cell(table, "Strip HTML tags", NULL, NULL,
		checkbut_striptags, NULL, NULL, COLUMN_1, COLUMN_2, i, COLUMN_1_ALIGN, COLUMN_2_ALIGN);
	i++;
	/*
	 * Upper case text
	 */
	checkbut_uppercase = gtk_check_button_new();
	if (prm->upper_case_text == 'y')
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbut_uppercase), TRUE);
	else if (prm->upper_case_text == 'n')
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbut_uppercase), FALSE);
	table_attach_styled_double_cell(table, "Set all text to upper case", NULL, NULL,
		checkbut_uppercase, NULL, NULL, COLUMN_1, COLUMN_2, i, COLUMN_1_ALIGN, COLUMN_2_ALIGN);
	i++;
	/*
	 * Homefeed / set current feed as homefeed
	 */
	curfeed_but = gtk_button_new_with_label("Current");
	entry_homefeed = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(entry_homefeed), FILE_NAME_MAXLEN);
	gtk_entry_set_text(GTK_ENTRY(entry_homefeed), prm->homefeed);
	table_attach_styled_double_cell(table, "Default feed", "'Homefeed'", NULL,
		curfeed_but, entry_homefeed, NULL, COLUMN_1, COLUMN_2, i, COLUMN_1_ALIGN, COLUMN_2_ALIGN);
	i++;
	/*
	 * "Open in Browser" Shell command
	 */
	entry_openlinkcmd = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(entry_openlinkcmd), FILE_NAME_MAXLEN);
	gtk_entry_set_text(GTK_ENTRY(entry_openlinkcmd), prm->open_link_cmd);
	table_attach_styled_double_cell(table, "'Open in Browser' Shell command", NULL,
		"Browser to open links with",
		entry_openlinkcmd, NULL, NULL, COLUMN_1, COLUMN_2, i, COLUMN_1_ALIGN, COLUMN_2_ALIGN);
	i++;
	/*
	 * "Open in Browser" args
	 */
	entry_openlinkargs = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(entry_openlinkargs), FILE_NAME_MAXLEN);
	gtk_entry_set_text(GTK_ENTRY(entry_openlinkargs), prm->open_link_args);
	table_attach_styled_double_cell(table, "Optional arguments", NULL,
		"'Open in Browser' optional arguments",
		entry_openlinkargs, NULL, NULL, COLUMN_1, COLUMN_2, i, COLUMN_1_ALIGN, COLUMN_2_ALIGN);
	i++;
	/*
	 * Clock
	 */
	radio_but_clock_1 = gtk_radio_button_new_with_label(NULL, "Left");
	radio_but_clock_2 = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio_but_clock_1), "Right");
	radio_but_clock_3 = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio_but_clock_1), "None");
	rbut_box_clock = gtk_hbox_new(TRUE, 2);
	gtk_box_pack_start(GTK_BOX(rbut_box_clock), radio_but_clock_1, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(rbut_box_clock), radio_but_clock_2, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(rbut_box_clock), radio_but_clock_3, TRUE, TRUE, 0);
	if (prm->clock == 'l')
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_but_clock_1), TRUE);
	else if (prm->clock == 'r')
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_but_clock_2), TRUE);
	else
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_but_clock_3), TRUE);
	table_attach_styled_double_cell(table, "Clock", NULL, NULL,
		rbut_box_clock, NULL, NULL, COLUMN_1, COLUMN_2, i, COLUMN_1_ALIGN, COLUMN_2_ALIGN);
	i++;
	/*
	 * Clock - show seconds
	 */
	checkbut_clocksec = gtk_check_button_new();
	if (prm->clock_sec == 'y')
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbut_clocksec), TRUE);
	else if (prm->clock_sec == 'n')
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbut_clocksec), FALSE);
	clock_sec_label = table_attach_styled_double_cell(table, "Show seconds", NULL, NULL,
		checkbut_clocksec, NULL, NULL, COLUMN_1, COLUMN_2, i, COLUMN_1_ALIGN, COLUMN_2_ALIGN);
	i++;
	 /*
	  * Clock - 12h time format
	  */
	checkbut_clock12h = gtk_check_button_new();
	if (prm->clock_12h == 'y')
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbut_clock12h), TRUE);
	else if (prm->clock_12h == 'n')
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbut_clock12h), FALSE);
	clock_12h_label = table_attach_styled_double_cell(table, "12h time format", NULL, NULL,
		checkbut_clock12h, NULL, NULL, COLUMN_1, COLUMN_2, i, COLUMN_1_ALIGN, COLUMN_2_ALIGN);
	i++;
	 /*
	  * Clock - show date
	  */
	checkbut_clockdate = gtk_check_button_new();
	if (prm->clock_date == 'y')
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbut_clockdate), TRUE);
	else if (prm->clock_date == 'n')
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbut_clockdate), FALSE);
	clock_date_label = table_attach_styled_double_cell(table, "Show date", NULL, NULL,
		checkbut_clockdate, NULL, NULL, COLUMN_1, COLUMN_2, i, COLUMN_1_ALIGN, COLUMN_2_ALIGN);
	i++;
	 /*
	  * Clock - use alt date format
	  */
	checkbut_clockaltdateform = gtk_check_button_new();
	if (prm->clock_alt_date_form == 'y')
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbut_clockaltdateform), TRUE);
	else if (prm->clock_alt_date_form == 'n')
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbut_clockaltdateform), FALSE);
	clock_altdateform_label = table_attach_styled_double_cell(table, "Alt format (Mon Jan 01 -> Mon 01 Jan)", NULL, NULL,
		checkbut_clockaltdateform, NULL, NULL, COLUMN_1, COLUMN_2, i, COLUMN_1_ALIGN, COLUMN_2_ALIGN);
	i++;
	/*
	 * Clock font
	 */
	clock_font_but = gtk_font_button_new_with_font(prm->clock_font_name_size);
	clock_font_label = table_attach_styled_double_cell(table, "Clock font", NULL,
		"Clock font size can't be > " APP_NAME " height",
		clock_font_but, NULL, NULL, COLUMN_1, COLUMN_2, i, COLUMN_1_ALIGN, COLUMN_2_ALIGN);
	i++;
	/*
	 * Clock fg color
	 */
	clock_fg_color_but = gtk_color_button_new_with_color(&prm->clock_fg_color);
	gtk_color_button_set_use_alpha(GTK_COLOR_BUTTON(clock_fg_color_but), TRUE);
	gtk_color_button_set_alpha(GTK_COLOR_BUTTON(clock_fg_color_but), prm->clock_fg_color_alpha);
	clock_fg_color_label = table_attach_styled_double_cell(table, "Clock foreground color", NULL, NULL,
		clock_fg_color_but, NULL, NULL, COLUMN_1, COLUMN_2, i, COLUMN_1_ALIGN, COLUMN_2_ALIGN);
	i++;
	/*
	 * Clock bg color
	 */
	clock_bg_color_but = gtk_color_button_new_with_color(&prm->clock_bg_color);
	gtk_color_button_set_use_alpha(GTK_COLOR_BUTTON(clock_bg_color_but), TRUE);
	gtk_color_button_set_alpha(GTK_COLOR_BUTTON(clock_bg_color_but), prm->clock_bg_color_alpha);
	clock_bg_color_label = table_attach_styled_double_cell(table, "Clock background color", NULL, NULL,
		clock_bg_color_but, NULL, NULL, COLUMN_1, COLUMN_2, i, COLUMN_1_ALIGN, COLUMN_2_ALIGN);
	i++;
	/*
	 * Set clock gradient bg
	 */
	checkbut_setclockgradientbg = gtk_check_button_new();
	if (prm->set_clock_gradient_bg == 'y')
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbut_setclockgradientbg), TRUE);
	else if (prm->set_clock_gradient_bg == 'n')
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbut_setclockgradientbg), FALSE);
	setclockgradientbg_label = table_attach_styled_double_cell(table, "Set clock gradient background", NULL, NULL,
		checkbut_setclockgradientbg, NULL, NULL, COLUMN_1, COLUMN_2, i, COLUMN_1_ALIGN, COLUMN_2_ALIGN);
	i++;
	/*
	 * Clock bg color2
	 */
	clock_bg_color2_but = gtk_color_button_new_with_color(&prm->clock_bg_color2);
	gtk_color_button_set_use_alpha(GTK_COLOR_BUTTON(clock_bg_color2_but), TRUE);
	gtk_color_button_set_alpha(GTK_COLOR_BUTTON(clock_bg_color2_but), prm->clock_bg_color2_alpha);
	clock_bg_color2_label = table_attach_styled_double_cell(table, "Clock background color2", NULL, NULL,
		clock_bg_color2_but, NULL, NULL, COLUMN_1, COLUMN_2, i, COLUMN_1_ALIGN, COLUMN_2_ALIGN);
	i++;
	/*
	 * Disable popups
	 */
	checkbut_nopopups = gtk_check_button_new();
	if (prm->disable_popups == 'y')
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbut_nopopups), TRUE);
	else if (prm->disable_popups == 'n')
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbut_nopopups), FALSE);
	table_attach_styled_double_cell(table, "Disable error/warning popups", NULL,
		"Prevent error/warning windows to popup",
		checkbut_nopopups, NULL, NULL, COLUMN_1, COLUMN_2, i, COLUMN_1_ALIGN, COLUMN_2_ALIGN);
	i++;
	/*
	 * Pause on mouse-over
	 */
	checkbut_mouseover = gtk_check_button_new();
	if (prm->pause_on_mouseover == 'y')
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbut_mouseover), TRUE);
	else if (prm->pause_on_mouseover == 'n')
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbut_mouseover), FALSE);
	table_attach_styled_double_cell(table, "Pause ticker on mouse-over", NULL, NULL,
		checkbut_mouseover, NULL, NULL, COLUMN_1, COLUMN_2, i, COLUMN_1_ALIGN, COLUMN_2_ALIGN);
	i++;
	/*
	 * Disable left-ckick
	 */
	checkbut_noleftclick = gtk_check_button_new();
	if (prm->disable_leftclick == 'y')
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbut_noleftclick), TRUE);
	else if (prm->disable_leftclick == 'n')
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbut_noleftclick), FALSE);
	table_attach_styled_double_cell(table, "Disable left-click", NULL, NULL,
		checkbut_noleftclick, NULL, NULL, COLUMN_1, COLUMN_2, i, COLUMN_1_ALIGN, COLUMN_2_ALIGN);
	i++;
	/*
	 * Mouse wheel scrolling behaviour
	 */
	radio_but_mw_1 = gtk_radio_button_new_with_label(NULL, "Speed");
	radio_but_mw_2 = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio_but_mw_1), "Feed");
	radio_but_mw_3 = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio_but_mw_1), "None");
	rbut_box_mw = gtk_hbox_new(TRUE, 2);
	gtk_box_pack_start(GTK_BOX(rbut_box_mw), radio_but_mw_1, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(rbut_box_mw), radio_but_mw_2, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(rbut_box_mw), radio_but_mw_3, TRUE, TRUE, 0);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_but_mw_1), TRUE);
	if (prm->mouse_wheel_action == 's')
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_but_mw_1), TRUE);
	else if (prm->mouse_wheel_action == 'f')
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_but_mw_2), TRUE);
	else
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_but_mw_3), TRUE);
	table_attach_styled_double_cell(table, "Mouse Wheel acts on", NULL,
		"Use <ctrl> Mouse Wheel for alternative action",
		rbut_box_mw, NULL, NULL, COLUMN_1, COLUMN_2, i, COLUMN_1_ALIGN, COLUMN_2_ALIGN);
	i++;
	/*
	 * Selected feed picker window closes when pointer leaves area
	 */
	checkbut_sfpickercloseswhenpointerleaves = gtk_check_button_new();
	if (prm->sfeedpicker_autoclose == 'y')
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbut_sfpickercloseswhenpointerleaves), TRUE);
	else if (prm->sfeedpicker_autoclose == 'n')
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbut_sfpickercloseswhenpointerleaves), FALSE);
	table_attach_styled_double_cell(table, "Selected Feed Picker auto-close", NULL,
		"Selected Feed Picker window closes when pointer leaves area",
		checkbut_sfpickercloseswhenpointerleaves, NULL, NULL, COLUMN_1, COLUMN_2, i, COLUMN_1_ALIGN, COLUMN_2_ALIGN);
	i++;
	/*
	 * Enable feed re-ordering (by user)
	 */
	checkbut_feedordering = gtk_check_button_new();
	if (prm->enable_feed_ordering == 'y')
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbut_feedordering), TRUE);
	else if (prm->enable_feed_ordering == 'n')
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbut_feedordering), FALSE);
	table_attach_styled_double_cell(table, "Enable feed re-ordering", NULL,
		"Allow user to re-order feeds by assigning ranks to them",
		checkbut_feedordering, NULL, NULL, COLUMN_1, COLUMN_2, i, COLUMN_1_ALIGN, COLUMN_2_ALIGN);

	/* Connect buttons / widgets to callbacks */
	g_signal_connect(G_OBJECT(checkbut_setgradientbg), "clicked", G_CALLBACK(check_set_gradient_widgets), NULL);
	g_signal_connect(G_OBJECT(checkbut_disablescreenlimits), "clicked", G_CALLBACK(update_win_x_y_w_adj), NULL);
	g_signal_connect(G_OBJECT(checkbut_spchars), "clicked", G_CALLBACK(check_set_sp_chars_widgets), NULL);
	g_signal_connect(G_OBJECT(radio_but_clock_1), "clicked", G_CALLBACK(check_set_clock_widgets), NULL);
	g_signal_connect(G_OBJECT(radio_but_clock_2), "clicked", G_CALLBACK(check_set_clock_widgets), NULL);
	g_signal_connect(G_OBJECT(radio_but_clock_3), "clicked", G_CALLBACK(check_set_clock_widgets), NULL);
	g_signal_connect(G_OBJECT(checkbut_clockdate), "clicked", G_CALLBACK(check_set_clock_widgets), NULL);
	g_signal_connect(G_OBJECT(top_but), "clicked", G_CALLBACK(move_to_top), NULL);
	g_signal_connect(G_OBJECT(bottom_but), "clicked", G_CALLBACK(move_to_bottom), NULL);
	g_signal_connect(G_OBJECT(fullwidth_but), "clicked", G_CALLBACK(set_full_width), NULL);
	g_signal_connect(G_OBJECT(curfeed_but), "clicked", G_CALLBACK(get_current_url), NULL);

	/* This must be run once at least */
	check_set_gradient_widgets(NULL);
	update_win_x_y_w_adj(NULL);
	check_set_sp_chars_widgets(NULL);
	check_set_clock_widgets(NULL);

	/* This will let us know if changes have been made */
	prm_bak = malloc2(sizeof(Params));
	memcpy((void *)prm_bak, (const void *)prm, sizeof(Params));
	changes_have_been_made = FALSE;

	gtk_widget_grab_focus(cancel_but);
	gtk_widget_show_all(dialog);
	gtk_window_set_focus(GTK_WINDOW(dialog), NULL);

	while (1) {
		response = gtk_dialog_run(GTK_DIALOG(dialog));
		env->suspend_rq = TRUE;
		if (response == GTK_RESPONSE_RESET) {
			if (question_win("Reset *all* settings to default values ?\n"
					"(Your current settings will be lost)", NO) == YES) {
				set_default_options(prm);
				save_to_config_file(prm);
				current_feed();
				env->compute_rq = TRUE;
				/*do_next = DO_NEXT_REOPEN;	// Not used anymore */
				break;
			} else
				continue;
		} else if (response == GTK_RESPONSE_APPLY || response == GTK_RESPONSE_OK) {
			set_ui_changes_to_params(prm, TRUE);
		}
		if (response == GTK_RESPONSE_CONN_SETTINGS) {
			if (question_win("Save changes you (eventually) made before leaving ?", -1) == YES)
				set_ui_changes_to_params(prm, TRUE);
			do_next = DO_NEXT_OPEN_CONN_SETTINGS;
			break;
		} else if (response == GTK_RESPONSE_APPLY) {	/* Force apply */
			/*
			 * We want a new pixmap so that changes will be effective... now!
			 *
			 * Some setting changes (like 'read n items per feed') need the stream
			 * to be reloaded
			 */
			current_feed();
			changes_have_been_made = TRUE;
			env->suspend_rq = FALSE;
		} else if (response == GTK_RESPONSE_OK) {
			if (prm->item_title == 'n' && prm->item_description == 'n') {
				warning(BLOCK, "%s\n%s", "You can't uncheck both 'Item title' and 'Item description'",
					"(because no much useful information would be displayed)");
				continue;
			}
			/* Apply and save when necessary */
			if (memcmp((const void *)prm, (const void *)prm_bak, sizeof(Params)) != 0 || \
					changes_have_been_made) {
				/*
				 * Some setting changes (like 'read n items per feed') need the stream
				 * to be reloaded
				 */
				current_feed();
				save_to_config_file(prm);
			}
			break;
		} else {
			/* Restore and apply when necessary */
			if (memcmp((const void *)prm, (const void *)prm_bak, sizeof(Params)) != 0 || \
					changes_have_been_made) {
				memcpy((void *)prm, (const void *)prm_bak, sizeof(Params));
				/*
				 * Some setting changes (like 'read n items per feed') need the stream
				 * to be reloaded
				 */
				current_feed();
			}
			break;
		}
	}
	free2(prm_bak);
	gtk_widget_destroy(dialog);
	check_main_win_always_on_top();
	env->suspend_rq = FALSE;
	return do_next;
}

static void set_ui_changes_to_params(Params *prm, zboolean full)
{
	/* Delay */
	prm->delay = (int)gtk_spin_button_get_value(GTK_SPIN_BUTTON(spinbut_delay));

	if (full) {
		/* Shift size */
		prm->shift_size = (int)gtk_spin_button_get_value(GTK_SPIN_BUTTON(spinbut_shiftsize));
	}

	/* Font */
	str_n_cpy(prm->font_name_size, (char *)gtk_font_button_get_font_name(
		GTK_FONT_BUTTON(font_but)), FONT_MAXLEN);

	/* Colors - fg and bg */
	gtk_color_button_get_color(GTK_COLOR_BUTTON(fg_color_but), &prm->fg_color);
	prm->fg_color_alpha = gtk_color_button_get_alpha(GTK_COLOR_BUTTON(fg_color_but));
	/*gtk_color_button_get_color(GTK_COLOR_BUTTON(highlight_fg_color_but), &prm->highlight_fg_color);
	prm->highlight_fg_color_alpha = gtk_color_button_get_alpha(GTK_COLOR_BUTTON(highlight_fg_color_but));*/
	gtk_color_button_get_color(GTK_COLOR_BUTTON(bg_color_but), &prm->bg_color);
	prm->bg_color_alpha = gtk_color_button_get_alpha(GTK_COLOR_BUTTON(bg_color_but));
	if ((zboolean)gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkbut_setgradientbg)))
		prm->set_gradient_bg = 'y';
	else
		prm->set_gradient_bg = 'n';
	gtk_color_button_get_color(GTK_COLOR_BUTTON(bg_color2_but), &prm->bg_color2);
	prm->bg_color2_alpha = gtk_color_button_get_alpha(GTK_COLOR_BUTTON(bg_color2_but));

	if (full) {
		/* Disable screen limits */
		if ((zboolean)gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkbut_disablescreenlimits)))
			prm->disable_screen_limits = 'y';
		else
			prm->disable_screen_limits = 'n';

		/* Window x, w */
		prm->win_x = (int)gtk_spin_button_get_value(GTK_SPIN_BUTTON(spinbut_winx));
		prm->win_w = (int)gtk_spin_button_get_value(GTK_SPIN_BUTTON(spinbut_winw));
	}

	/* Window y */
	prm->win_y = (int)gtk_spin_button_get_value(GTK_SPIN_BUTTON(spinbut_winy));

	/* If win_h is > 0, it will override requested font size with computed one. */
	prm->win_h = (int)gtk_spin_button_get_value(GTK_SPIN_BUTTON(spinbut_winh));
	if (prm->win_h > 0 && prm->win_h < DRWA_HEIGHT_MIN)
		prm->win_h = DRWA_HEIGHT_MIN;

	if (full) {
		/* Window decoration */
		if ((zboolean)gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkbut_windec)))
			prm->windec = 'y';
		else
			prm->windec = 'n';
	}

	/* Window always-on-top */
	if ((zboolean)gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkbut_alwaysontop)))
		prm->always_on_top = 'y';
	else
		prm->always_on_top = 'n';

	if (full) {
		/* Window transparency */
		prm->win_transparency = gtk_spin_button_get_value(GTK_SPIN_BUTTON(spinbut_wintransparency)) / 10;

		/* Icon in taskbar */
		if ((zboolean)gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkbut_iconintaskbar)))
			prm->icon_in_taskbar = 'y';
		else
			prm->icon_in_taskbar = 'n';

		/* Window sticky */
		if ((zboolean)gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkbut_winsticky)))
			prm->win_sticky = 'y';
		else
			prm->win_sticky = 'n';

		/* Set window override_redirect flag */
		if ((zboolean)gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkbut_overrideredirect)))
			prm->override_redirect = 'y';
		else
			prm->override_redirect = 'n';

		/* Shadow */
		if ((zboolean)gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkbut_shadow)))
			prm->shadow = 'y';
		else
			prm->shadow = 'n';
		prm->shadow_offset_x = (int)gtk_spin_button_get_value(GTK_SPIN_BUTTON(spinbut_shadowoffsetx));
		prm->shadow_offset_y = (int)gtk_spin_button_get_value(GTK_SPIN_BUTTON(spinbut_shadowoffsety));
		prm->shadow_fx = (int)gtk_spin_button_get_value(GTK_SPIN_BUTTON(spinbut_shadowfx));

		/* Line delimiter */
		str_n_cpy(prm->line_delimiter,
			(char *)gtk_entry_get_text(GTK_ENTRY(entry_linedel)), DELIMITER_MAXLEN);

		/* Enable special chars */
		if ((zboolean)gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkbut_spchars)))
			prm->special_chars = 'y';
		else
			prm->special_chars = 'n';
		prm->new_page_char = gtk_entry_get_text(GTK_ENTRY(entry_newpagech))[0];
		prm->tab_char = gtk_entry_get_text(GTK_ENTRY(entry_tabch))[0];

		/* Reload delay (rss refresh) */
		prm->rss_refresh = (int)gtk_spin_button_get_value(GTK_SPIN_BUTTON(spinbut_rssrefresh));
	}

	/* Reverse scrolling */
	if ((zboolean)gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkbut_revsc)))
		prm->reverse_sc = 'y';
	else
		prm->reverse_sc = 'n';

	if (full) {
		/* Feed title */
		if ((zboolean)gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkbut_feedtitle)))
			prm->feed_title = 'y';
		else
			prm->feed_title = 'n';
		str_n_cpy(prm->feed_title_delimiter,
			(char *)gtk_entry_get_text(GTK_ENTRY(entry_feedtitledel)), DELIMITER_MAXLEN);

		/* Item title */
		if ((zboolean)gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkbut_itemtitle)))
			prm->item_title = 'y';
		else
			prm->item_title = 'n';
		str_n_cpy(prm->item_title_delimiter,
			(char *)gtk_entry_get_text(GTK_ENTRY(entry_itemtitledel)), DELIMITER_MAXLEN);

		/* Item decription */
		if ((zboolean)gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkbut_itemdes)))
			prm->item_description = 'y';
		else
			prm->item_description = 'n';
		str_n_cpy(prm->item_description_delimiter,
			(char *)gtk_entry_get_text(GTK_ENTRY(entry_itemdesdel)), DELIMITER_MAXLEN);
	}

	/* Read n items max per feed */
	prm->n_items_per_feed = (int)gtk_spin_button_get_value(GTK_SPIN_BUTTON(spinbut_nitemsperfeed));

	if (full) {
		/* Mark item action */
		/*if ((zboolean)gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_but_mi_1)))
			prm->mark_item_action = 'h';
		else if ((zboolean)gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_but_mi_2)))
			prm->mark_item_action = 'c';
		else
			prm->mark_item_action = 'n';*/

		/* Strip html tags */
		if ((zboolean)gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkbut_striptags)))
			prm->strip_html_tags = 'y';
		else
			prm->strip_html_tags = 'n';

		/* Upper case text */
		if ((zboolean)gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkbut_uppercase)))
			prm->upper_case_text = 'y';
		else
			prm->upper_case_text = 'n';

		/* Homefeed */
		str_n_cpy(prm->homefeed,
			(char *)gtk_entry_get_text(GTK_ENTRY(entry_homefeed)), FILE_NAME_MAXLEN);

		/* Open link cmd */
		str_n_cpy(prm->open_link_cmd,
			(char *)gtk_entry_get_text(GTK_ENTRY(entry_openlinkcmd)), FILE_NAME_MAXLEN);

		/* Open link args */
		str_n_cpy(prm->open_link_args,
			(char *)gtk_entry_get_text(GTK_ENTRY(entry_openlinkargs)), FILE_NAME_MAXLEN);

		/* Clock */
		if ((zboolean)gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_but_clock_1)))
			prm->clock = 'l';
		else if ((zboolean)gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_but_clock_2)))
			prm->clock = 'r';
		else
			prm->clock = 'n';

		/* Clock - show seconds */
		if ((zboolean)gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkbut_clocksec)))
			prm->clock_sec = 'y';
		else
			prm->clock_sec = 'n';

		/* Clock - 12h time format */
		if ((zboolean)gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkbut_clock12h)))
			prm->clock_12h = 'y';
		else
			prm->clock_12h = 'n';

		/* Clock - show date */
		if ((zboolean)gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkbut_clockdate)))
			prm->clock_date = 'y';
		else
			prm->clock_date = 'n';

		/* Clock - use alt date format */
		if ((zboolean)gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkbut_clockaltdateform)))
			prm->clock_alt_date_form = 'y';
		else
			prm->clock_alt_date_form = 'n';

		str_n_cpy(prm->clock_font_name_size, (char *)gtk_font_button_get_font_name(GTK_FONT_BUTTON(clock_font_but)), FONT_MAXLEN);

		gtk_color_button_get_color(GTK_COLOR_BUTTON(clock_fg_color_but), &prm->clock_fg_color);
		prm->clock_fg_color_alpha = gtk_color_button_get_alpha( GTK_COLOR_BUTTON(clock_fg_color_but));
		gtk_color_button_get_color(GTK_COLOR_BUTTON(clock_bg_color_but), &prm->clock_bg_color);
		prm->clock_bg_color_alpha = gtk_color_button_get_alpha(GTK_COLOR_BUTTON(clock_bg_color_but));
		if ((zboolean)gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkbut_setclockgradientbg)))
			prm->set_clock_gradient_bg = 'y';
		else
			prm->set_clock_gradient_bg = 'n';
		gtk_color_button_get_color(GTK_COLOR_BUTTON(clock_bg_color2_but), &prm->clock_bg_color2);
		prm->clock_bg_color2_alpha = gtk_color_button_get_alpha(GTK_COLOR_BUTTON(clock_bg_color2_but));

		/* Disable popups */
		if ((zboolean)gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkbut_nopopups)))
			prm->disable_popups = 'y';
		else
			prm->disable_popups = 'n';

		/* Pause on mouse-over */
		if ((zboolean)gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkbut_mouseover)))
			prm->pause_on_mouseover = 'y';
		else
			prm->pause_on_mouseover = 'n';

		/* Disable left-click */
		if ((zboolean)gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkbut_noleftclick)))
			prm->disable_leftclick = 'y';
		else
			prm->disable_leftclick = 'n';
	}

	/* Mouse wheel scrolling behaviour */
	if ((zboolean)gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_but_mw_1)))
		prm->mouse_wheel_action = 's';
	else if ((zboolean)gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_but_mw_2)))
		prm->mouse_wheel_action = 'f';
	else
		prm->mouse_wheel_action = 'n';

	if (full) {
		/* Selected feed picker window closes when pointer leaves area */
		if ((zboolean)gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(
				checkbut_sfpickercloseswhenpointerleaves)))
			prm->sfeedpicker_autoclose = 'y';
		else
			prm->sfeedpicker_autoclose = 'n';

		/* Enable feed re-ordering (by user) */
		if ((zboolean)gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkbut_feedordering)))
			prm->enable_feed_ordering = 'y';
		else
			prm->enable_feed_ordering = 'n';
	}
}
