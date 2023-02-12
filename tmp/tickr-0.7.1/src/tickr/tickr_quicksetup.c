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

/* These variables and widgets are all set as static global for convenience. */
static TickerEnv	*env;
static Params		*prm, *prm_bak;
static GtkWidget	*qsetup;
static GtkWidget	*top_but, *bottom_but, *spinbut_winy, *checkbut_alwaysontop;
/*static GtkWidget	*checkbut_feedtitle, *checkbut_itemtitle, *checkbut_itemdes;*/
static GtkWidget	*entry_openlinkcmd/*, *checkbut_nopopups*/;
static	GtkObject*	adj_winy;

static void qsetup_prepare(GtkWidget *widget, GtkWidget *page, gpointer data)
{
	widget = widget;
	data = data;
	gtk_assistant_set_page_complete(GTK_ASSISTANT(qsetup), page, TRUE);
}

static void qsetup_cancel(GtkWidget *widget, gpointer data)
{
	widget = widget;
	data = data;
	memcpy((void *)prm, (const void *)prm_bak, sizeof(Params));
	gtk_main_quit();
}

static int qsetup_del_event()
{
	if (question_win("Cancel quick setup ?", -1) == YES) {
		memcpy((void *)prm, (const void *)prm_bak, sizeof(Params));
		gtk_main_quit();
		return FALSE;
	} else
		return TRUE;
}

static void keep_setting(GtkWidget *widget, gpointer data)
{
	widget = widget;
	switch((int)data) {
	case 1:
		/* Window y */
		prm->win_y = (int)gtk_spin_button_get_value(GTK_SPIN_BUTTON(spinbut_winy));
		break;
	case 2:
		/* Window always-on-top */
		if ((zboolean)gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkbut_alwaysontop)))
			prm->always_on_top = 'y';
		else
			prm->always_on_top = 'n';
		break;
	/*case 3:
		// Feed title
		if ((zboolean)gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkbut_feedtitle)))
			prm->feed_title = 'y';
		else
			prm->feed_title = 'n';
		break;
	case 4:
		// Item title
		if ((zboolean)gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkbut_itemtitle)))
			prm->item_title = 'y';
		else
			prm->item_title = 'n';
		break;
	case 5:
		// Item decription
		if ((zboolean)gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkbut_itemdes)))
			prm->item_description = 'y';
		else
			prm->item_description = 'n';
		break;*/
	case 3/*6*/:
		/* Open link cmd */
		str_n_cpy(prm->open_link_cmd,
			(char *)gtk_entry_get_text(GTK_ENTRY(entry_openlinkcmd)), FILE_NAME_MAXLEN);
		break;
	/*case 7:
		// Disable popups
		if ((zboolean)gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkbut_nopopups)))
			prm->disable_popups = 'y';
		else
			prm->disable_popups = 'n';
		break;*/
	default:
		break;
	}
}

static void qsetup_apply(GtkWidget *widget, gpointer data)
{
	widget = widget;
	data = data;
	if (gtk_assistant_get_current_page(GTK_ASSISTANT(qsetup)) == 4/*8*/) {
		save_to_config_file(prm);
		gtk_main_quit();
	}
}

static int move_to_top(GtkWidget *widget)
{
	widget = widget;
	get_params()->win_y = 0;
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
	requested_h = get_params()->win_h;
	if (requested_h > 0 && requested_h < DRWA_HEIGHT_MIN)
		requested_h = DRWA_HEIGHT_MIN;
	else if (requested_h == 0) {
		/* Compute ticker height from requested font size */
		str_n_cpy(font_name_size, get_params()->font_name_size, FONT_MAXLEN);
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
	get_params()->win_y = y_bottom;
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(spinbut_winy), y_bottom);
	get_params()->disable_popups = disable_popups_bak;
	return TRUE;
}

/*
 * TODO: which other ones here ????
 * Reverse scrolling / Read n items max per feed / Mouse wheel scrolling behaviour
 */
void quick_setup(Params *prm0)
{
#define N_PAGES_MAX	16
	GtkWidget	*page[N_PAGES_MAX], *label[N_PAGES_MAX], *hbox[N_PAGES_MAX];
	GtkWidget	*vbox[N_PAGES_MAX];
	int		i;

	env = get_ticker_env();
	prm = prm0;
	prm_bak = malloc2(sizeof(Params));
	memcpy((void *)prm_bak, (const void *)prm, sizeof(Params));

	qsetup = gtk_assistant_new();
	gtk_window_set_title(GTK_WINDOW(qsetup), "Quick Setup");
	set_tickr_icon_to_dialog(GTK_WINDOW(qsetup));
	gtk_window_set_position(GTK_WINDOW(qsetup), GTK_WIN_POS_CENTER);
	gtk_window_set_resizable(GTK_WINDOW(qsetup), FALSE);
	gtk_widget_set_size_request(qsetup, 550,  -1);

	g_signal_connect(G_OBJECT(qsetup), "delete-event", G_CALLBACK(qsetup_del_event), NULL);

	i = 0;
	/* Intro page */
	hbox[i] = gtk_hbox_new(FALSE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(hbox[i]), 15);

	label[i] = gtk_label_new("No configuration file was found.\n"
		"Would you like to setup a few parameters first ?");
	gtk_label_set_use_markup(GTK_LABEL(label[i]), TRUE);
	gtk_misc_set_alignment(GTK_MISC(label[i]), 0, 0.5);
	gtk_box_pack_start(GTK_BOX(hbox[i]), label[i], FALSE, FALSE, 0);

	page[i] = hbox[i];
	gtk_assistant_append_page(GTK_ASSISTANT(qsetup), page[i]);
	gtk_assistant_set_page_type(GTK_ASSISTANT(qsetup), page[i], GTK_ASSISTANT_PAGE_INTRO);
	gtk_assistant_set_page_title(GTK_ASSISTANT(qsetup), page[i], APP_NAME " first run ?");
	i++;

	/* win_y */
	hbox[i] = gtk_hbox_new(FALSE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(hbox[i]), 15);

	label[i] = gtk_label_new("Vertical Location <small>(Pixels)</small>:");
	gtk_label_set_use_markup(GTK_LABEL(label[i]), TRUE);
	gtk_misc_set_alignment(GTK_MISC(label[i]), 0, 0.5);
	gtk_box_pack_start(GTK_BOX(hbox[i]), label[i], FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox[i]), gtk_label_new(""), TRUE, FALSE, 0);

	top_but = gtk_button_new_with_label("Top");
	gtk_box_pack_start(GTK_BOX(hbox[i]), top_but, FALSE, FALSE, 0);
	bottom_but = gtk_button_new_with_label("Bottom");
	gtk_box_pack_start(GTK_BOX(hbox[i]), bottom_but, FALSE, FALSE, 0);
	adj_winy = gtk_adjustment_new(prm->win_y, 0, env->screen_h, 1, 5, 0);
	spinbut_winy = gtk_spin_button_new(GTK_ADJUSTMENT(adj_winy), 0.0, 0);
	gtk_box_pack_start(GTK_BOX(hbox[i]), spinbut_winy, FALSE, FALSE, 0);

	vbox[i] = gtk_vbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox[i]), gtk_label_new(""), FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox[i]), hbox[i], TRUE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox[i]), gtk_label_new(""), FALSE, FALSE, 0);

	page[i] = vbox[i];
	gtk_assistant_append_page(GTK_ASSISTANT(qsetup), page[i]);
	gtk_assistant_set_page_type(GTK_ASSISTANT(qsetup), page[i], GTK_ASSISTANT_PAGE_CONTENT);
	gtk_assistant_set_page_title(GTK_ASSISTANT(qsetup), page[i], APP_NAME " location");
	i++;

	/* Window always-on-top */
	hbox[i] = gtk_hbox_new(FALSE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(hbox[i]), 15);

	label[i] = gtk_label_new(APP_NAME " always above other windows:");
	gtk_label_set_use_markup(GTK_LABEL(label[i]), TRUE);
	gtk_misc_set_alignment(GTK_MISC(label[i]), 0, 0.5);
	gtk_box_pack_start(GTK_BOX(hbox[i]), label[i], FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox[i]), gtk_label_new(""), TRUE, FALSE, 0);

	checkbut_alwaysontop = gtk_check_button_new();
	if (prm->always_on_top == 'y')
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbut_alwaysontop), TRUE);
	else if (prm->always_on_top == 'n')
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbut_alwaysontop), FALSE);
	gtk_box_pack_start(GTK_BOX(hbox[i]), checkbut_alwaysontop, FALSE, FALSE, 0);

	vbox[i] = gtk_vbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox[i]), gtk_label_new(""), FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox[i]), hbox[i], TRUE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox[i]), gtk_label_new(""), FALSE, FALSE, 0);

	page[i] = vbox[i];
	gtk_assistant_append_page(GTK_ASSISTANT(qsetup), page[i]);
	gtk_assistant_set_page_type(GTK_ASSISTANT(qsetup), page[i], GTK_ASSISTANT_PAGE_CONTENT);
	gtk_assistant_set_page_title(GTK_ASSISTANT(qsetup), page[i], APP_NAME " always-on-top");
	i++;

/* Following 3 pages now disabled:
	// Feed title
	hbox[i] = gtk_hbox_new(FALSE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(hbox[i]), 15);

	label[i] = gtk_label_new("Feed title:");
	gtk_label_set_use_markup(GTK_LABEL(label[i]), TRUE);
	gtk_misc_set_alignment(GTK_MISC(label[i]), 0, 0.5);
	gtk_box_pack_start(GTK_BOX(hbox[i]), label[i], FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox[i]), gtk_label_new(""), TRUE, FALSE, 0);

	checkbut_feedtitle = gtk_check_button_new();
	if (prm->feed_title == 'y')
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbut_feedtitle), TRUE);
	else if (prm->feed_title == 'n')
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbut_feedtitle), FALSE);
	gtk_box_pack_start(GTK_BOX(hbox[i]), checkbut_feedtitle, FALSE, FALSE, 0);

	vbox[i] = gtk_vbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox[i]), gtk_label_new(""), FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox[i]), hbox[i], TRUE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox[i]), gtk_label_new(""), FALSE, FALSE, 0);

	page[i] = vbox[i];
	gtk_assistant_append_page(GTK_ASSISTANT(qsetup), page[i]);
	gtk_assistant_set_page_type(GTK_ASSISTANT(qsetup), page[i], GTK_ASSISTANT_PAGE_CONTENT);
	gtk_assistant_set_page_title(GTK_ASSISTANT(qsetup), page[i], "Display feed title");
	i++;

	// Item title
	hbox[i] = gtk_hbox_new(FALSE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(hbox[i]), 15);

	label[i] = gtk_label_new("Item title:");
	gtk_label_set_use_markup(GTK_LABEL(label[i]), TRUE);
	gtk_misc_set_alignment(GTK_MISC(label[i]), 0, 0.5);
	gtk_box_pack_start(GTK_BOX(hbox[i]), label[i], FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox[i]), gtk_label_new(""), TRUE, FALSE, 0);

	checkbut_itemtitle = gtk_check_button_new();
	if (prm->item_title == 'y')
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbut_itemtitle), TRUE);
	else if (prm->item_title == 'n')
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbut_itemtitle), FALSE);
	gtk_box_pack_start(GTK_BOX(hbox[i]), checkbut_itemtitle, FALSE, FALSE, 0);

	vbox[i] = gtk_vbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox[i]), gtk_label_new(""), FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox[i]), hbox[i], TRUE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox[i]), gtk_label_new(""), FALSE, FALSE, 0);

	page[i] = vbox[i];
	gtk_assistant_append_page(GTK_ASSISTANT(qsetup), page[i]);
	gtk_assistant_set_page_type(GTK_ASSISTANT(qsetup), page[i], GTK_ASSISTANT_PAGE_CONTENT);
	gtk_assistant_set_page_title(GTK_ASSISTANT(qsetup), page[i], "Display item title");
	i++;

	// Item description
	hbox[i] = gtk_hbox_new(FALSE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(hbox[i]), 15);

	label[i] = gtk_label_new("Item description:");
	gtk_label_set_use_markup(GTK_LABEL(label[i]), TRUE);
	gtk_misc_set_alignment(GTK_MISC(label[i]), 0, 0.5);
	gtk_box_pack_start(GTK_BOX(hbox[i]), label[i], FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox[i]), gtk_label_new(""), TRUE, FALSE, 0);

	checkbut_itemdes = gtk_check_button_new();
	if (prm->item_description == 'y')
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbut_itemdes), TRUE);
	else if (prm->item_description == 'n')
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbut_itemdes), FALSE);
	gtk_box_pack_start(GTK_BOX(hbox[i]), checkbut_itemdes, FALSE, FALSE, 0);

	vbox[i] = gtk_vbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox[i]), gtk_label_new(""), FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox[i]), hbox[i], TRUE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox[i]), gtk_label_new(""), FALSE, FALSE, 0);

	page[i] = vbox[i];
	gtk_assistant_append_page(GTK_ASSISTANT(qsetup), page[i]);
	gtk_assistant_set_page_type(GTK_ASSISTANT(qsetup), page[i], GTK_ASSISTANT_PAGE_CONTENT);
	gtk_assistant_set_page_title(GTK_ASSISTANT(qsetup), page[i], "Display item description");
	i++;
*/

	/* "Open in browser" command line */
	hbox[i] = gtk_hbox_new(FALSE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(hbox[i]), 15);

	label[i] = gtk_label_new("Browser to open links with:\n"
		"<small>(Shell command - leave blank/unchanged if unsure)</small>");
	gtk_label_set_use_markup(GTK_LABEL(label[i]), TRUE);
	gtk_misc_set_alignment(GTK_MISC(label[i]), 0, 0.5);
	gtk_box_pack_start(GTK_BOX(hbox[i]), label[i], FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox[i]), gtk_label_new(""), TRUE, FALSE, 0);

	entry_openlinkcmd = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(entry_openlinkcmd), FILE_NAME_MAXLEN);
	gtk_entry_set_text(GTK_ENTRY(entry_openlinkcmd), prm->open_link_cmd);
	gtk_box_pack_start(GTK_BOX(hbox[i]), entry_openlinkcmd, FALSE, FALSE, 0);

	vbox[i] = gtk_vbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox[i]), gtk_label_new(""), FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox[i]), hbox[i], TRUE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox[i]), gtk_label_new(""), FALSE, FALSE, 0);

	page[i] = vbox[i];
	gtk_assistant_append_page(GTK_ASSISTANT(qsetup), page[i]);
	gtk_assistant_set_page_type(GTK_ASSISTANT(qsetup), page[i], GTK_ASSISTANT_PAGE_CONTENT);
	gtk_assistant_set_page_title(GTK_ASSISTANT(qsetup), page[i], "Pick a Browser");
	i++;

/* This page also disabled:
	// Disable popups
	hbox[i] = gtk_hbox_new(FALSE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(hbox[i]), 15);

	label[i] = gtk_label_new("Disable error/warning popup windows:");
	gtk_label_set_use_markup(GTK_LABEL(label[i]), TRUE);
	gtk_misc_set_alignment(GTK_MISC(label[i]), 0, 0.5);
	gtk_box_pack_start(GTK_BOX(hbox[i]), label[i], FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox[i]), gtk_label_new(""), TRUE, FALSE, 0);

	checkbut_nopopups = gtk_check_button_new();
	if (prm->disable_popups == 'y')
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbut_nopopups), TRUE);
	else if (prm->disable_popups == 'n')
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbut_nopopups), FALSE);
	gtk_box_pack_start(GTK_BOX(hbox[i]), checkbut_nopopups, FALSE, FALSE, 0);

	vbox[i] = gtk_vbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox[i]), gtk_label_new(""), FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox[i]), hbox[i], TRUE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox[i]), gtk_label_new(""), FALSE, FALSE, 0);

	page[i] = vbox[i];
	gtk_assistant_append_page(GTK_ASSISTANT(qsetup), page[i]);
	gtk_assistant_set_page_type(GTK_ASSISTANT(qsetup), page[i], GTK_ASSISTANT_PAGE_CONTENT);
	gtk_assistant_set_page_title(GTK_ASSISTANT(qsetup), page[i], "Disable popups");
	i++;
*/

	/* Confirm page */
	hbox[i] = gtk_hbox_new(FALSE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(hbox[i]), 15);

	label[i] = gtk_label_new("Quick setup complete !\n"
		"To open the preferences or full settings window, right-click on " APP_NAME " then\n"
		" 'Edit > Preferences' or 'Edit > Full Settings'.");
	gtk_label_set_use_markup(GTK_LABEL(label[i]), TRUE);
	gtk_misc_set_alignment(GTK_MISC(label[i]), 0, 0.5);
	gtk_box_pack_start(GTK_BOX(hbox[i]), label[i], FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox[i]), gtk_label_new(""), TRUE, FALSE, 0);

	page[i] = hbox[i];
	gtk_container_set_border_width(GTK_CONTAINER(hbox[i]), 15);

	gtk_assistant_append_page(GTK_ASSISTANT(qsetup), page[i]);
	gtk_assistant_set_page_type(GTK_ASSISTANT(qsetup), page[i], GTK_ASSISTANT_PAGE_CONFIRM);
	gtk_assistant_set_page_title(GTK_ASSISTANT(qsetup), page[i], "Done");

	g_signal_connect(G_OBJECT(top_but), "clicked", G_CALLBACK(move_to_top), NULL);
	g_signal_connect(G_OBJECT(bottom_but), "clicked", G_CALLBACK(move_to_bottom), NULL);

	g_signal_connect(G_OBJECT(spinbut_winy), "changed", G_CALLBACK(keep_setting), (gpointer)1);
	g_signal_connect(G_OBJECT(checkbut_alwaysontop), "toggled", G_CALLBACK(keep_setting), (gpointer)2);
	/*g_signal_connect(G_OBJECT(checkbut_feedtitle), "toggled", G_CALLBACK(keep_setting), (gpointer)3);
	g_signal_connect(G_OBJECT(checkbut_itemtitle), "toggled", G_CALLBACK(keep_setting), (gpointer)4);
	g_signal_connect(G_OBJECT(checkbut_itemdes), "toggled", G_CALLBACK(keep_setting), (gpointer)5);*/
	g_signal_connect(G_OBJECT(entry_openlinkcmd), "changed", G_CALLBACK(keep_setting), (gpointer)3/*6*/);
	/*g_signal_connect(G_OBJECT(checkbut_nopopups), "toggled", G_CALLBACK(keep_setting), (gpointer)7);*/

	g_signal_connect(G_OBJECT(qsetup), "cancel", G_CALLBACK(qsetup_cancel), NULL);
	g_signal_connect(G_OBJECT(qsetup), "prepare", G_CALLBACK(qsetup_prepare), NULL);
	g_signal_connect(G_OBJECT(qsetup), "apply", G_CALLBACK(qsetup_apply), NULL);

	gtk_widget_show_all(qsetup);
	gtk_main();
	while (gtk_events_pending())
		gtk_main_iteration();
	free2(prm_bak);
	gtk_widget_destroy(qsetup);
}
