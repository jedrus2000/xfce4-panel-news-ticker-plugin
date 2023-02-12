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

int esc_key_pressed(GtkWidget *dialog2, GdkEventKey *event_key)
{
	if (event_key->keyval == GDK_Escape) {
		gtk_dialog_response(GTK_DIALOG(dialog2), GTK_RESPONSE_CANCEL_CLOSE);
		return TRUE;
	} else
		return FALSE;
}

void force_quit_dialog(GtkWidget *dialog2)
{
	gtk_dialog_response(GTK_DIALOG(dialog2), GTK_RESPONSE_CANCEL_CLOSE);
}

/*
 * 'Open Text File' dialog win
 */
void open_txt_file(Resource *resrc)
{
	TickerEnv	*env = get_ticker_env();
	GtkWidget	*dialog;
	char		resrc_id_bak[FILE_NAME_MAXLEN + 1];
	char		*file_name2;
	int		error_code;

	gtk_window_set_keep_above(GTK_WINDOW(env->win), FALSE);

	dialog = gtk_file_chooser_dialog_new(
			"Text File Picker", GTK_WINDOW(env->win),
			GTK_FILE_CHOOSER_ACTION_OPEN,
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL_CLOSE,
			GTK_STOCK_OK, GTK_RESPONSE_OK,
			NULL);

	set_tickr_icon_to_dialog(GTK_WINDOW(dialog));
	gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);

	g_signal_connect(G_OBJECT(dialog), "key-press-event", G_CALLBACK(esc_key_pressed), NULL);
	g_signal_connect(G_OBJECT(dialog), "delete_event", G_CALLBACK(force_quit_dialog), NULL);

	gtk_widget_show_all(dialog);
	/*
	 * Backup last valid opened resource (if any)
	 */
	str_n_cpy(resrc_id_bak, resrc->id, FILE_NAME_MAXLEN);

	while (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
		file_name2 = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		str_n_cpy(resrc->id, file_name2, FILE_NAME_MAXLEN);
		g_free(file_name2);

		get_ticker_env()->selection_mode = SINGLE;
		if ((error_code = load_resource_from_selection(resrc, NULL)) == OK) {
			env->reload_rq = TRUE;
			break;
		} else {
			warning(BLOCK, "%s(): %s", __func__, global_error_str(error_code));
			str_n_cpy(resrc->id, resrc_id_bak, FILE_NAME_MAXLEN);
			load_resource_from_selection(resrc, NULL);
		}
	}
	gtk_widget_destroy(dialog);
	check_main_win_always_on_top();
}

void show_resource_info(Resource *resrc)
{
	GtkWidget	*dialog, *table, *label[7], *close_but;
	char		tmp1[256], *tmp2, *tmp3;
	int		i, j;

	if (resrc->type != RESRC_URL && resrc->type != RESRC_FILE) {
		info_win("Resource Properties", "\nNo information available\n", INFO_ERROR, FALSE);
		return;
	}

	gtk_window_set_keep_above(GTK_WINDOW(get_ticker_env()->win), FALSE);

	dialog = gtk_dialog_new_with_buttons(
			"Resource Properties", GTK_WINDOW(get_ticker_env()->win),
			GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
			NULL);

	close_but = gtk_dialog_add_button(GTK_DIALOG(dialog), GTK_STOCK_CLOSE, GTK_RESPONSE_CANCEL_CLOSE);
	close_but = close_but;	/* To get rid of compiler warning */

	set_tickr_icon_to_dialog(GTK_WINDOW(dialog));
	gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);
	gtk_window_set_resizable(GTK_WINDOW(dialog), FALSE);

	g_signal_connect(G_OBJECT(dialog), "key-press-event", G_CALLBACK(esc_key_pressed), NULL);
	g_signal_connect(G_OBJECT(dialog), "delete_event", G_CALLBACK(force_quit_dialog), NULL);

	table = gtk_table_new(3, 3, FALSE);
	gtk_table_set_row_spacings(GTK_TABLE(table), 5);
	gtk_container_set_border_width(GTK_CONTAINER(table), 15);
	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), table);

	label[0] = gtk_label_new("        ");
	if (resrc->type == RESRC_URL) {
		label[1] = gtk_label_new("Resource type:");
		snprintf(tmp1, 256, "%s Feed", resrc->format == RSS_2_0 ? "RSS 2.0" : \
			(resrc->format == RSS_ATOM ? "Atom" : "RSS 1.0"));
		label[2] = gtk_label_new(tmp1);

		label[3] = gtk_label_new("Feed title:");
		label[4] = gtk_label_new(resrc->feed_title);

		label[5] = gtk_label_new("Feed URL:");
		tmp2 = l_str_new("<a href=\"");
		tmp3 = malloc2(sizeof(char) * FILE_NAME_MAXLEN * 2);
		/* Need to replace all "&" with "&amp;" to use markup. */
		for (i = 0, j = 0; resrc->id[i] != '\0' && j < (FILE_NAME_MAXLEN * 2) - 4; i++, j++)
			if (resrc->id[i] != '&')
				tmp3[j] = resrc->id[i];
			else {
				str_n_cpy(tmp3 + j, "&amp;", 5);
				j += 4;
			}
		tmp3[j] = '\0';
		tmp2 = l_str_cat(tmp2, tmp3);
		tmp2 = l_str_cat(tmp2, "\">");
		tmp2 = l_str_cat(tmp2, tmp3);
		tmp2 = l_str_cat(tmp2, "</a>");
		label[6] = gtk_label_new(tmp2);
		l_str_free(tmp2);
		free2(tmp3);
		gtk_label_set_use_markup(GTK_LABEL(label[6]), TRUE);

		for (i = 0; i < 7; i++)
			gtk_misc_set_alignment(GTK_MISC(label[i]), 0, 0.5);

		VERBOSE_INFO_OUT("Resource properties:\n"
			"  Resource type: %s\n  Feed title: %s\n  Feed URL: %s\n",
			tmp1, resrc->feed_title, resrc->id)
	} else if (resrc->type == RESRC_FILE) {
		label[1] = gtk_label_new("Resource type:  ");
		label[2] = gtk_label_new("File");

		label[3] = gtk_label_new("File name:  ");
		label[4] = gtk_label_new(resrc->id);
		gtk_label_set_selectable(GTK_LABEL(label[4]), TRUE);

		for (i = 0; i < 5; i++)
			gtk_misc_set_alignment(GTK_MISC(label[i]), 0, 0.5);

		VERBOSE_INFO_OUT("Resource Properties:\n"
			"  Resource type: File\n  File name: %s\n", resrc->id)
	}

	gtk_table_attach_defaults(GTK_TABLE(table), label[0], 1, 2, 0, 1);

	gtk_table_attach_defaults(GTK_TABLE(table), label[1], 0, 1, 0, 1);
	gtk_table_attach_defaults(GTK_TABLE(table), label[2], 2, 3, 0, 1);

	gtk_table_attach_defaults(GTK_TABLE(table), label[3], 0, 1, 1, 2);
	gtk_table_attach_defaults(GTK_TABLE(table), label[4], 2, 3, 1, 2);

	if (resrc->type == RESRC_URL) {
		gtk_table_attach_defaults(GTK_TABLE(table), label[5], 0, 1, 2, 3);
		gtk_table_attach_defaults(GTK_TABLE(table), label[6], 2, 3, 2, 3);
	}

	gtk_widget_show_all(dialog);
	while (gtk_dialog_run(GTK_DIALOG(dialog)) != GTK_RESPONSE_CANCEL_CLOSE);

	gtk_widget_destroy(dialog);
	check_main_win_always_on_top();
}

static void get_layout_dims(GtkWidget *view, int *layout_width, int *layout_height, char *str)
{
	PangoLayout *p_layout;

	p_layout = pango_layout_new(gtk_widget_get_pango_context(view));
	pango_layout_set_text(p_layout, str, -1);
	pango_layout_get_pixel_size(p_layout, layout_width, layout_height);
	if (p_layout != NULL)
		g_object_unref(p_layout);
}

#ifndef G_OS_WIN32
#  define HLPWIN_FONT		"DejaVu Sans Mono 8" /* 8 9 10 OK */
#else
#  define HLPWIN_FONT		"Courier New 8"
#endif
#define HLPWIN_FG_COLOR		"#000000ff"	/* Black */
#define HLPWIN_BG_COLOR		"#ffffffff"	/* White */

void help_win()
{
	GtkWidget		*dialog, *sc_win, *view;
	GtkTextBuffer		*buf;
	PangoFontDescription	*font_des;
	GdkColor		fg_color, bg_color;
	guint16			fgc_alpha, bgc_alpha;
	char			*txt;
	int			width, height;
	int			i;

	gtk_window_set_keep_above(GTK_WINDOW(get_ticker_env()->win), FALSE);

	dialog = gtk_dialog_new_with_buttons(
			"Quick Help", GTK_WINDOW(get_ticker_env()->win),
			GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_STOCK_CLOSE, GTK_RESPONSE_CANCEL_CLOSE,
			NULL);

	set_tickr_icon_to_dialog(GTK_WINDOW(dialog));
	gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);

	g_signal_connect(G_OBJECT(dialog), "key-press-event", G_CALLBACK(esc_key_pressed), NULL);
	g_signal_connect(G_OBJECT(dialog), "delete_event", G_CALLBACK(force_quit_dialog), NULL);

	sc_win = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sc_win), GTK_POLICY_AUTOMATIC,
		GTK_POLICY_AUTOMATIC);
	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), sc_win);
	gtk_container_set_border_width(GTK_CONTAINER(sc_win), 5);

	txt = l_str_new(NULL);
	buf = gtk_text_buffer_new(NULL);

	view = gtk_text_view_new_with_buffer(buf);
	gtk_text_view_set_editable(GTK_TEXT_VIEW(view), FALSE);
	gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(view), FALSE);
	font_des = pango_font_description_from_string(HLPWIN_FONT);
	gtk_widget_modify_font(view, font_des);

	get_gdk_color_from_hexstr(HLPWIN_FG_COLOR, &fg_color, &fgc_alpha);
	get_gdk_color_from_hexstr(HLPWIN_BG_COLOR, &bg_color, &bgc_alpha);
	/*
	 * Not using gtk_widget_modify_fg/bg() but gtk_widget_modify_text/base()
	 * because it's specifically for editable text.
	 */
	if (FALSE) {	/* One possible extra setting = 'same colors as ticker's' = 'yes' */
		gtk_widget_modify_text(view, GTK_STATE_NORMAL, &(get_params()->fg_color));
		gtk_widget_modify_base(view, GTK_STATE_NORMAL, &(get_params()->bg_color));
	} else if (TRUE) { /* Widget-level-defined */
		gtk_widget_modify_text(view, GTK_STATE_NORMAL, &fg_color);
		gtk_widget_modify_base(view, GTK_STATE_NORMAL, &bg_color);
	} else if (FALSE) { /* Same as label inside window */
		gtk_widget_modify_text(view, GTK_STATE_NORMAL,
			(const GdkColor *)&(gtk_widget_get_style(get_ticker_env()->win)->fg[GTK_STATE_NORMAL]));
		gtk_widget_modify_base(view, GTK_STATE_NORMAL,
			(const GdkColor *)&(gtk_widget_get_style(get_ticker_env()->win)->bg[GTK_STATE_NORMAL]));
	}

	gtk_text_view_set_left_margin(GTK_TEXT_VIEW(view), 15);
	gtk_text_view_set_right_margin(GTK_TEXT_VIEW(view), 15);
	gtk_container_add(GTK_CONTAINER(sc_win), view);

	txt = l_str_cat(txt, "\n");
	for (i = 0; get_help_str0()[i] != NULL; i++)
		txt = l_str_cat(txt, get_help_str0()[i]);
	for (i = 0; get_help_str1()[i] != NULL; i++)
		txt = l_str_cat(txt, get_help_str1()[i]);

	gtk_text_buffer_set_text(buf, txt, -1);
	get_layout_dims(view, &width, &height, txt);
	l_str_free(txt);
	if (width < 400)
		width = 400;
	else if (width > 800)
		width = 800;
	if (height > 450)
		height = 450;
	gtk_widget_set_size_request(sc_win, width + 30 + 15 + 15, height);

	gtk_widget_show_all(dialog);
	gtk_dialog_run(GTK_DIALOG(dialog));

	gtk_widget_destroy(dialog);
	check_main_win_always_on_top();
}

static int license_win()
{
	GtkWidget	*dialog, *label;
	char		*txt;
	int		i;

	gtk_window_set_keep_above(GTK_WINDOW(get_ticker_env()->win), FALSE);

	dialog = gtk_dialog_new_with_buttons(
			"License", GTK_WINDOW(get_ticker_env()->win),
			GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_STOCK_CLOSE, GTK_RESPONSE_CANCEL_CLOSE,
			NULL);

	set_tickr_icon_to_dialog(GTK_WINDOW(dialog));
	gtk_container_set_border_width(GTK_CONTAINER(dialog), 15);
	gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_NONE);
	gtk_window_set_resizable(GTK_WINDOW(dialog), FALSE);

	g_signal_connect(G_OBJECT(dialog), "key-press-event", G_CALLBACK(esc_key_pressed), NULL);
	g_signal_connect(G_OBJECT(dialog), "delete_event", G_CALLBACK(force_quit_dialog), NULL);

	txt = l_str_new(NULL);
#ifndef G_OS_WIN32
	txt = l_str_cat(txt, "<small>");
#endif
	for (i = 0; get_license_str1()[i] != NULL; i++)
		txt = l_str_cat(txt, get_license_str1()[i]);
	txt = l_str_cat(txt, "<a href=\"http://www.gnu.org/licenses/\">");
	txt = l_str_cat(txt, get_license_str2());
	txt = l_str_cat(txt, "</a>");
#ifndef G_OS_WIN32
	txt = l_str_cat(txt, "</small>");
#endif
	txt = l_str_cat(txt, "\n");

	label = gtk_label_new(txt);
	gtk_label_set_use_markup(GTK_LABEL(label), TRUE);
	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), label);
	l_str_free(txt);

	gtk_widget_show_all(dialog);
	gtk_dialog_run(GTK_DIALOG(dialog));

	gtk_widget_destroy(dialog);
	check_main_win_always_on_top();
	return TRUE;
}

static int go_to_paypal0()
{
	go_to_paypal();
	return TRUE;
}

void about_win()
{
	GtkWidget	*dialog, *table, *image = NULL, *label[5];
	GtkWidget	*license_but, *go_to_paypal_but, *close_but;

	gtk_window_set_keep_above(GTK_WINDOW(get_ticker_env()->win), FALSE);

	dialog = gtk_dialog_new_with_buttons(
			"About " APP_NAME, GTK_WINDOW(get_ticker_env()->win),
			GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
			NULL);

	license_but = gtk_dialog_add_button(GTK_DIALOG(dialog), "License", GTK_RESPONSE_NONE);
	go_to_paypal_but = gtk_dialog_add_button(GTK_DIALOG(dialog), "Donate" , GTK_RESPONSE_NONE);
	close_but = gtk_dialog_add_button(GTK_DIALOG(dialog), GTK_STOCK_CLOSE, GTK_RESPONSE_CANCEL_CLOSE);
	close_but = close_but;	/* to Get rid of one stupid compiler warning */

	set_tickr_icon_to_dialog(GTK_WINDOW(dialog));
	gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);
	gtk_window_set_resizable(GTK_WINDOW(dialog), FALSE);

	g_signal_connect(G_OBJECT(license_but), "clicked", G_CALLBACK(license_win), NULL);
	g_signal_connect(G_OBJECT(go_to_paypal_but), "clicked", G_CALLBACK(go_to_paypal0), NULL);
	g_signal_connect(G_OBJECT(dialog), "key-press-event", G_CALLBACK(esc_key_pressed), NULL);
	g_signal_connect(G_OBJECT(dialog), "delete_event", G_CALLBACK(force_quit_dialog), NULL);

#ifndef ABOUT_WIN_QUOTE
	table = gtk_table_new(5, 1, FALSE);
#else
	table = gtk_table_new(6, 1, FALSE);
#endif
	gtk_table_set_row_spacings(GTK_TABLE(table), 5);
	gtk_container_set_border_width(GTK_CONTAINER(table), 15);
	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), table);

	image = gtk_image_new_from_file(get_imagefile_full_name_from_name(TICKR_LOGO));
	gtk_table_attach_defaults(GTK_TABLE(table), image, 0, 1, 0, 1);

	label[0] = gtk_label_new("\n<b>" APP_NAME " version " APP_V_NUM " - Feed Reader</b>");
	gtk_label_set_use_markup(GTK_LABEL(label[0]), TRUE);
	gtk_table_attach_defaults(GTK_TABLE(table), label[0], 0, 1, 1, 2);
	label[1] = gtk_label_new("<small>" COPYRIGHT_STR "</small>");
	gtk_label_set_use_markup(GTK_LABEL(label[1]), TRUE);
	gtk_table_attach_defaults(GTK_TABLE(table), label[1], 0, 1, 2, 3);
	label[2] = gtk_label_new("<a href=\"mailto:" SUPPORT_EMAIL_ADDR "\">" SUPPORT_EMAIL_ADDR "</a>");
	gtk_label_set_use_markup(GTK_LABEL(label[2]), TRUE);
	gtk_table_attach_defaults(GTK_TABLE(table), label[2], 0, 1, 3, 4);
	label[3] = gtk_label_new("<a href=\"" WEBSITE_URL "\">" WEBSITE_URL "</a>");
	gtk_label_set_use_markup(GTK_LABEL(label[3]), TRUE);
	gtk_table_attach_defaults(GTK_TABLE(table), label[3], 0, 1, 4, 5);
#ifdef ABOUT_WIN_QUOTE
	label[4] = gtk_label_new("<small><i>\n\"" ABOUT_WIN_QUOTE "\"</i></small>");
	gtk_label_set_use_markup(GTK_LABEL(label[4]), TRUE);
	gtk_table_attach_defaults(GTK_TABLE(table), label[4], 0, 1, 5, 6);
#endif

	gtk_widget_show_all(dialog);
	while (gtk_dialog_run(GTK_DIALOG(dialog)) != GTK_RESPONSE_CANCEL_CLOSE);

	gtk_widget_destroy(dialog);
	check_main_win_always_on_top();
}

/*
 * Insert newline(s) if one string (ie between 2 newlines) is too long
 * (ie > MAX_LINE_LEN). Returned string must be free2() afterwards.
 */
#define MAX_LINE_LEN	100
#define MAX_NEWLINES_N	20

static char *insert_newlines_if_too_long(const char *txt)
{
	char	*txt2;
	int	line_len, word_len;
	int	i, j, k;

	txt2 = malloc2(strlen(txt) + MAX_NEWLINES_N + 1);
	line_len = 0;
	word_len = 0;
	i = j = k = 0;

	while (*(txt + i) != '\0') {
		if (*(txt + i) == ' ')
			word_len = -1;
		if (*(txt + i) == '\n') {
			line_len = -1;
			word_len = -1;
		} else if (line_len > 0 && line_len % MAX_LINE_LEN == 0) {
			if (k++ < MAX_NEWLINES_N) {
				if (word_len < line_len) {
					i -= word_len;
					j -= word_len + 1;
				}
				*(txt2 + j++) = '\n';
				line_len = 0;
				word_len = 0;
			} else
				break;
		}
		*(txt2 + j++) = *(txt + i++);
		line_len++;
		word_len++;
	}
	*(txt2 + j) = '\0';

	return txt2;
}

/*
 * Info, warning or error. Format text in case of long strings.
 * ========================================================================
 * get_params()->disable_popups applies to info_win(), info_win_no_block(),
 * and warning().
 * warning() always send info to sdtout/stderr.
 * ========================================================================
 */
void info_win(const char *title, const char *txt, info_type info, zboolean selectable)
{
	GtkWidget	*dialog, *table, *image = NULL, *label[2];
	char		*txt2;

	if (get_params()->disable_popups == 'y')
		return;
	gtk_window_set_keep_above(GTK_WINDOW(get_ticker_env()->win), FALSE);

	dialog = gtk_dialog_new_with_buttons(
			/* FIXME: Use APP_NAME if title is empty ? */
			title[0] != '\0' ? title : APP_NAME, GTK_WINDOW(get_ticker_env()->win),
			GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_STOCK_OK, GTK_RESPONSE_OK,
			NULL);

	set_tickr_icon_to_dialog(GTK_WINDOW(dialog));
	gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);
	gtk_window_set_resizable(GTK_WINDOW(dialog), FALSE);
	gtk_window_set_keep_above(GTK_WINDOW(dialog), TRUE);

	g_signal_connect(G_OBJECT(dialog), "key-press-event", G_CALLBACK(esc_key_pressed), NULL);
	g_signal_connect(G_OBJECT(dialog), "delete_event", G_CALLBACK(force_quit_dialog), NULL);

	table = gtk_table_new(1, 3, FALSE);
	gtk_table_set_col_spacings(GTK_TABLE(table), 10);
	gtk_container_set_border_width(GTK_CONTAINER(table), 0);
	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), table);

	if (info == INFO) {
		/* Better without any image if info only so do nothing here -
		image = gtk_image_new_from_stock(GTK_STOCK_DIALOG_INFO, GTK_ICON_SIZE_DIALOG);*/
	} else if (info == INFO_WARNING) {
		image = gtk_image_new_from_stock(GTK_STOCK_DIALOG_WARNING, GTK_ICON_SIZE_DIALOG);
		gtk_table_attach_defaults(GTK_TABLE(table), image, 0, 1, 0, 1);
	} else if (info == INFO_ERROR) {
		image = gtk_image_new_from_stock(GTK_STOCK_DIALOG_ERROR, GTK_ICON_SIZE_DIALOG);
		gtk_table_attach_defaults(GTK_TABLE(table), image, 0, 1, 0, 1);
	}

	txt2 = insert_newlines_if_too_long(txt);
	label[0] = gtk_label_new(txt2);
	free2((char *)txt2);

	/* Better if disabled -
	gtk_label_set_use_markup(GTK_LABEL(label[0]), TRUE);*/
	pango_layout_set_single_paragraph_mode(gtk_label_get_layout(GTK_LABEL(label[0])), FALSE);
	/* Useless now - Hmmm, should be
	pango_layout_get_pixel_size(gtk_label_get_layout(GTK_LABEL(label[0])),
		&layout_width, &layout_height);
	if (info == INFO) {
		if (layout_width > get_ticker_env()->screen_w - 60)
			gtk_widget_set_size_request(label[0], get_ticker_env()->screen_w - 60, -1);
	} else {
		if (layout_width > get_ticker_env()->screen_w - 120)	// Extra 60 pixels for image width
			gtk_widget_set_size_request(label[0], get_ticker_env()->screen_w - 120, -1);
	}*/
	if (selectable)
		gtk_label_set_selectable(GTK_LABEL(label[0]), TRUE);
	gtk_table_attach_defaults(GTK_TABLE(table), label[0], 1, 2, 0, 1);
	label[1] = gtk_label_new("");
	gtk_table_attach_defaults(GTK_TABLE(table), label[1], 2, 3, 0, 1);

	gtk_widget_show_all(dialog);
	gtk_dialog_run(GTK_DIALOG(dialog));

	gtk_widget_destroy(dialog);
	while (gtk_events_pending())	/* So that win will close immediately */
		gtk_main_iteration();
	check_main_win_always_on_top();
}

/* Need only gtk_init() so called only from big_error() in tickr_error.c. */
void minimalistic_info_win(const char *title, const char *txt)
{
	GtkWidget	*dialog, *table, *image = NULL, *label[2];
	char		*txt2;

	dialog = gtk_dialog_new_with_buttons(
			title, NULL,
			GTK_DIALOG_MODAL,
			GTK_STOCK_OK, GTK_RESPONSE_OK,
			NULL);

	set_tickr_icon_to_dialog(GTK_WINDOW(dialog));
	gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);
	gtk_window_set_resizable(GTK_WINDOW(dialog), FALSE);
	gtk_window_set_keep_above(GTK_WINDOW(dialog), TRUE);

	table = gtk_table_new(1, 3, FALSE);
	gtk_table_set_col_spacings(GTK_TABLE(table), 10);
	gtk_container_set_border_width(GTK_CONTAINER(table), 0);
	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), table);

	image = gtk_image_new_from_stock(GTK_STOCK_DIALOG_ERROR, GTK_ICON_SIZE_DIALOG);
	gtk_table_attach_defaults(GTK_TABLE(table), image, 0, 1, 0, 1);

	txt2 = insert_newlines_if_too_long(txt);
	label[0] = gtk_label_new(txt2);
	free2((char *)txt2);
	pango_layout_set_single_paragraph_mode(gtk_label_get_layout(GTK_LABEL(label[0])), FALSE);

	gtk_table_attach_defaults(GTK_TABLE(table), label[0], 1, 2, 0, 1);
	label[1] = gtk_label_new("");
	gtk_table_attach_defaults(GTK_TABLE(table), label[1], 2, 3, 0, 1);

	gtk_widget_show_all(dialog);
	gtk_dialog_run(GTK_DIALOG(dialog));

	gtk_widget_destroy(dialog);
	while (gtk_events_pending())	/* So that win will close immediately */
		gtk_main_iteration();
}

/* Don't block but show up for <delay> ms then close.
 * TODO: write a more configurable func ? */
void info_win_no_block(const char *txt, int delay)
{
	GtkWidget	*win;
	GtkWidget	*table, *image, *label[2];
	int		layout_width, layout_height;

	if (get_params()->disable_popups == 'y')
		return;
	gtk_window_set_keep_above(GTK_WINDOW(get_ticker_env()->win), FALSE);

	win = gtk_window_new(GTK_WINDOW_POPUP);

	set_tickr_icon_to_dialog(GTK_WINDOW(win));
	gtk_window_set_position(GTK_WINDOW(win), INFO_WIN_WAIT_POS);
	gtk_window_set_keep_above(GTK_WINDOW(win), TRUE);

	table = gtk_table_new(1, 3, FALSE);
	gtk_table_set_col_spacings(GTK_TABLE(table), 10);
	gtk_container_set_border_width(GTK_CONTAINER(table), 0);
	gtk_container_add(GTK_CONTAINER(GTK_WINDOW(win)), table);

	image = gtk_image_new_from_stock(GTK_STOCK_DIALOG_WARNING, GTK_ICON_SIZE_DIALOG);
	gtk_table_attach_defaults(GTK_TABLE(table), image, 0, 1, 0, 1);

	label[0] = gtk_label_new(txt);
	/* Better if disabled -
	gtk_label_set_use_markup(GTK_LABEL(label[0]), TRUE);*/
	pango_layout_get_pixel_size(gtk_label_get_layout(GTK_LABEL(label[0])),
		&layout_width, &layout_height);
	if (layout_width > get_ticker_env()->screen_w - 120)
		gtk_widget_set_size_request(label[0], get_ticker_env()->screen_w - 120, -1);
	gtk_table_attach_defaults(GTK_TABLE(table), label[0], 1, 2, 0, 1);
	label[1] = gtk_label_new("");
	gtk_table_attach_defaults(GTK_TABLE(table), label[1], 2, 3, 0, 1);

	gtk_widget_show_all(win);
	while (gtk_events_pending())	/* So that win will popup immediately */
		gtk_main_iteration();

#ifndef G_OS_WIN32
	nanosleep2(delay * 1000000);
#else
	Sleep(delay);
#endif
	gtk_widget_destroy(win);
	while (gtk_events_pending())	/* So that win will close immediately */
		gtk_main_iteration();
	check_main_win_always_on_top();
}

/* Win centered.
 * default_response = YES / NO / -1 (or anything != YES and != NO) for nothing selected. */
int question_win(const char *txt, int default_response)
{
	return question_win_at(txt, default_response, GTK_WIN_POS_CENTER);
}

/* default_response = YES / NO / -1 (or anything != YES and != NO) for nothing selected. */
int question_win_at(const char *txt, int default_response, int win_pos)
{
	GtkWidget	*dialog, *table, *image, *label[2];
	int		response;

	gtk_window_set_keep_above(GTK_WINDOW(get_ticker_env()->win), FALSE);

	dialog = gtk_dialog_new_with_buttons(
			APP_NAME, GTK_WINDOW(get_ticker_env()->win),
			GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_STOCK_NO, GTK_RESPONSE_NO,
			GTK_STOCK_YES, GTK_RESPONSE_YES,
			NULL);

	set_tickr_icon_to_dialog(GTK_WINDOW(dialog));
	gtk_window_set_position(GTK_WINDOW(dialog), win_pos);
	gtk_window_set_resizable(GTK_WINDOW(dialog), FALSE);
	gtk_window_set_keep_above(GTK_WINDOW(dialog), TRUE);
	if (default_response == YES)
		gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_YES);
	else if (default_response == NO)
		gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_NO);
	else
		default_response = -1;

	table = gtk_table_new(1, 3, FALSE);
	gtk_table_set_col_spacings(GTK_TABLE(table), 10);
	gtk_container_set_border_width(GTK_CONTAINER(table), 0);
	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), table);

	image = gtk_image_new_from_stock(GTK_STOCK_DIALOG_QUESTION,
							GTK_ICON_SIZE_DIALOG);
	gtk_table_attach_defaults(GTK_TABLE(table), image, 0, 1, 0, 1);
	label[0] = gtk_label_new(txt);
	gtk_table_attach_defaults(GTK_TABLE(table), label[0], 1, 2, 0, 1);
	label[1] = gtk_label_new("");
	gtk_table_attach_defaults(GTK_TABLE(table), label[1], 2, 3, 0, 1);

	gtk_widget_show_all(dialog);
	if (default_response == -1)
		gtk_window_set_focus(GTK_WINDOW(dialog), NULL);
	response = gtk_dialog_run(GTK_DIALOG(dialog));

	gtk_widget_destroy(dialog);
	while (gtk_events_pending())	/* So that win will close immediately */
		gtk_main_iteration();
	check_main_win_always_on_top();
	return ((response == GTK_RESPONSE_YES) ? YES : NO);
}

/* The spinner doesn't always spin as expected (hmm...) so now commented out */
/*void win_with_spinner(win_with_spinner_mode mode, const char *txt)
{
	static GtkWidget	*win;
	GtkWidget		*hbox, *label;
#ifndef G_OS_WIN32
	static GtkWidget	*spinner;
	GtkWidget		*space_label;
#endif
	if (mode == WIN_WITH_SPINNER_OPEN) {
		gtk_window_set_keep_above(GTK_WINDOW(get_ticker_env()->win), FALSE);

		win = gtk_window_new(GTK_WINDOW_POPUP);
		set_tickr_icon_to_dialog(GTK_WINDOW(win));
		gtk_window_set_position(GTK_WINDOW(win), GTK_WIN_POS_CENTER);
		gtk_window_set_keep_above(GTK_WINDOW(win), TRUE);

		hbox = gtk_hbox_new(FALSE, 0);
		gtk_container_add(GTK_CONTAINER(GTK_WINDOW(win)), hbox);
#ifndef G_OS_WIN32
		spinner = gtk_spinner_new();
		space_label = gtk_label_new("    ");
#endif
		label = gtk_label_new(txt);
#ifndef G_OS_WIN32
		gtk_container_add(GTK_CONTAINER(GTK_BOX(hbox)), spinner);
		gtk_container_add(GTK_CONTAINER(GTK_BOX(hbox)), space_label);
#endif
		gtk_container_add(GTK_CONTAINER(GTK_BOX(hbox)), label);
		gtk_container_set_border_width(GTK_CONTAINER(GTK_BOX(hbox)), 15);

		gtk_widget_show_all(win);
		while (gtk_events_pending())	// So that win will popup immediately
			gtk_main_iteration();
#ifndef G_OS_WIN32
		gtk_spinner_start(GTK_SPINNER(spinner));
#endif
	} else if (mode == WIN_WITH_SPINNER_CLOSE) {
#ifndef G_OS_WIN32
		gtk_spinner_stop(GTK_SPINNER(spinner));
#endif
		gtk_widget_destroy(win);
		check_main_win_always_on_top();
	} else
		warning(BLOCK, "win_with_spinner(): Invalid mode");
}*/

void win_with_progress_bar(win_with_progress_bar_mode mode, const char *txt)
{
	static GtkWidget *win, *hbox, *progress_bar;

	if (mode == WIN_WITH_PROGRESS_BAR_OPEN) {
		gtk_window_set_keep_above(GTK_WINDOW(get_ticker_env()->win), FALSE);

		win = gtk_window_new(GTK_WINDOW_POPUP);
		set_tickr_icon_to_dialog(GTK_WINDOW(win));
		gtk_window_set_position(GTK_WINDOW(win), GTK_WIN_POS_CENTER);
		gtk_window_set_keep_above(GTK_WINDOW(win), TRUE);

		hbox = gtk_hbox_new(FALSE, 0);
		gtk_container_add(GTK_CONTAINER(GTK_WINDOW(win)), hbox);
		progress_bar = gtk_progress_bar_new();
		gtk_container_add(GTK_CONTAINER(GTK_BOX(hbox)), progress_bar);
		gtk_container_set_border_width(GTK_CONTAINER(GTK_BOX(hbox)), 15);

		gtk_progress_bar_set_text(GTK_PROGRESS_BAR(progress_bar), txt);
		gtk_progress_bar_set_pulse_step(GTK_PROGRESS_BAR(progress_bar), 0.2);

		gtk_widget_show_all(win);

		while (gtk_events_pending())	/* So that win will popup immediately */
			gtk_main_iteration();
	} else if (mode == WIN_WITH_PROGRESS_BAR_PULSE) {
		if (txt != NULL)
			gtk_progress_bar_set_text(GTK_PROGRESS_BAR(progress_bar), txt);
		gtk_progress_bar_pulse(GTK_PROGRESS_BAR(progress_bar));

		while (gtk_events_pending())
			gtk_main_iteration();
	} else if (mode == WIN_WITH_PROGRESS_BAR_CLOSE) {
		gtk_widget_destroy(win);
		check_main_win_always_on_top();
	} else
		warning(BLOCK, "win_with_progress_bar(): Invalid mode");
}

#ifndef G_OS_WIN32
void nanosleep2(long nano_sec)
{
	struct timespec ts;

	ts.tv_sec = 0;
	ts.tv_nsec = nano_sec;
	nanosleep(&ts, NULL);
}
#endif
