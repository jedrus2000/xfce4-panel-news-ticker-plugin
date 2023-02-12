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

void import_params()
{
	GtkWidget	*dialog;
	FILE		*fp1, *fp2;
	char		*file_name;
	char		tmp[TMPSTR_SIZE + 1];
	int		c;

	gtk_window_set_keep_above(GTK_WINDOW(get_ticker_env()->win), FALSE);

	dialog = gtk_file_chooser_dialog_new("Import Settings", GTK_WINDOW(get_ticker_env()->win),
			GTK_FILE_CHOOSER_ACTION_OPEN,
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL_CLOSE,
			GTK_STOCK_OK, GTK_RESPONSE_OK,
			NULL);

	set_tickr_icon_to_dialog(GTK_WINDOW(dialog));
	gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);

	g_signal_connect(G_OBJECT(dialog), "key-press-event", G_CALLBACK(esc_key_pressed), NULL);
	g_signal_connect(G_OBJECT(dialog), "delete_event", G_CALLBACK(force_quit_dialog), NULL);

	gtk_widget_show_all(dialog);

	if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
		if ((file_name = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog))) != NULL) {
			if ((fp1 = g_fopen(file_name, "rb")) != NULL) {
				snprintf(tmp, TMPSTR_SIZE + 1,
					"Settings are about to be imported from file: %s. "
					"Continue ?", file_name);
				if (question_win(tmp, -1) == YES) {
					if ((fp2 = g_fopen(get_datafile_full_name_from_name(CONFIG_FILE),
						 	"wb")) != NULL) {
						while ((c = fgetc(fp1)) != (int)EOF)
							fputc((int)c, fp2);
						fclose(fp2);
						get_config_file_options(get_params());
						current_feed();
						get_ticker_env()->reload_rq = TRUE;
						/* ????
						modify_params2();*/
					} else
						warning(BLOCK, "Can't save Settings to file '%s': %s",
							file_name, strerror(errno));
				}
				fclose(fp1);
			} else
				warning(BLOCK, "Can't open '%s': %s", file_name, strerror(errno));
			g_free(file_name);
		}
	}
	gtk_widget_destroy(dialog);
	check_main_win_always_on_top();
}

void export_params()
{
	GtkWidget	*dialog;
	FILE		*fp1, *fp2;
	char		*file_name = NULL;
	int		error_code = !OK;
	char		tmp[TMPSTR_SIZE + 1];
	int		c;

	if ((fp1 = g_fopen(get_datafile_full_name_from_name(CONFIG_FILE), "rb")) == NULL) {
		save_to_config_file(get_params());
		if ((fp1 = g_fopen(get_datafile_full_name_from_name(CONFIG_FILE), "rb")) == NULL) {
			warning(BLOCK, "Can't open file '%s': %s",
				get_datafile_full_name_from_name(CONFIG_FILE), strerror(errno));
			return;
		}
	}

	gtk_window_set_keep_above(GTK_WINDOW(get_ticker_env()->win), FALSE);

	dialog = gtk_file_chooser_dialog_new("Export Settings", GTK_WINDOW(get_ticker_env()->win),
			GTK_FILE_CHOOSER_ACTION_SAVE,
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL_CLOSE,
			GTK_STOCK_OK, GTK_RESPONSE_OK,
			NULL);

	set_tickr_icon_to_dialog(GTK_WINDOW(dialog));
	gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);

	g_signal_connect(G_OBJECT(dialog), "key-press-event", G_CALLBACK(esc_key_pressed), NULL);
	g_signal_connect(G_OBJECT(dialog), "delete_event", G_CALLBACK(force_quit_dialog), NULL);

	gtk_widget_show_all(dialog);

	/*gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), "~");*/
	gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), CONFIG_FILE);
	gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog), TRUE);

	if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
		if ((file_name = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog))) != NULL) {
			if ((fp2 = g_fopen(file_name, "wb")) != NULL) {
				while ((c = fgetc(fp1)) != (int)EOF)
					fputc((int)c, fp2);
				fclose(fp2);
				error_code = OK;
			} else
				warning(BLOCK, "Can't save Settings to file '%s': %s",
					file_name, strerror(errno));
		}
	}
	fclose(fp1);
	gtk_widget_destroy(dialog);
	if (error_code == OK) {
		snprintf(tmp, TMPSTR_SIZE + 1,
			"\nSettings have been exported to file: %s\n", file_name);
		info_win("", tmp, INFO, FALSE);
	}
	if (file_name != NULL)
		g_free(file_name);
	check_main_win_always_on_top();
}

/* Doesn't use get_params()->open_link_args (which is done in open_link()) */
void open_url_in_browser(char *url)
{
	char	tmp1[FILE_NAME_MAXLEN + 1];
	char	*argv[3];
	GPid	pid;
	GError	*error = NULL;

	if (get_params()->open_link_cmd[0] == '\0') {
		easily_link_with_browser();
		if (get_params()->open_link_cmd[0] == '\0') {
			warning(BLOCK, "Can't launch Browser: no command is defined.\n",
				"Please set the 'Open in Browser' option in the Full Settings window.");
			return;
		}
	}
	/* Launch browser */
	INFO_OUT("Spawning: %s %s\n", get_params()->open_link_cmd, url)
	str_n_cpy(tmp1, get_params()->open_link_cmd, FILE_NAME_MAXLEN);
	argv[0] = tmp1;
	argv[1] = url;
	argv[2] = NULL;
	if (!g_spawn_async(NULL, argv, NULL, G_SPAWN_SEARCH_PATH, NULL, NULL, &pid, &error)) {
		warning(BLOCK, "%s: Can't create process %s - %s", APP_NAME, argv[0], error->message);
		info_win("", "Please check the 'Open in Browser' option in the Full Settings window.",
			INFO_WARNING, FALSE);
	} else
		g_spawn_close_pid(pid);
}

void online_help()
{
	open_url_in_browser(SUPPORT_URL);
}

void go_to_paypal()
{
	open_url_in_browser(DONATE_WITH_PAYPAL_URL);
}

#define NFAMMAX		(2 * 1024)
#define NFACEMAX	64

void dump_font_list()
{
	PangoFontFamily		**font_fam;
	PangoFontFamily		*font_fam2[NFAMMAX];
	PangoFontFace		**font_face;
	PangoFontFace		*font_face2[NFACEMAX];
	PangoFontDescription	*font_desc;
	char			*font[NFAMMAX * NFACEMAX + 1], *tmp;
	int			n_fam, n_face, i, j, k, overflow, min;

	fprintf(STD_OUT, "List of available fonts on this system:\n");
	font_fam = font_fam2;
	font_face = font_face2;
	overflow = FALSE;
	k = 0;
	pango_font_map_list_families(pango_cairo_font_map_get_default(), &font_fam, &n_fam);
	if (n_fam >= NFAMMAX)
		overflow = TRUE;
	for (i = 0; i < n_fam && i < NFAMMAX; i++) {
		pango_font_family_list_faces(font_fam[i], &font_face, &n_face);
		if (n_face >= NFACEMAX)
			overflow = TRUE;
		for (j = 0; j < n_face && j < NFACEMAX; j++) {
			font_desc = pango_font_face_describe(font_face[j]);
			font[k++] = pango_font_description_to_string(font_desc);
			pango_font_description_free(font_desc);
		}
	}
	font[k] = NULL;
	/* Not sure if we must g_free() sth ? */
	/* Sort array (selection sort) */
	for (i = 0; i < k; i++) {
		min = i;
		for (j = i  + 1; j < k; j++) {
			if (strcmp(font[min], font[j]) > 0)
				min = j;
		}
		tmp = font[i];
		font[i] = font[min];
		font[min] = tmp;
	}
	for (i = 0; i < k; i++) {
		if (font[i] != NULL)
			fprintf(STD_OUT, "%s\n", font[i]);
		else
			break;
	}
	if (overflow)
		fprintf(STD_ERR,
			"In function dump_font_list(): need to allocate more space for array\n"
			"Can't dump full list (more than %d available fonts)\n", k);
	else
		fprintf(STD_OUT, "(%d available fonts)\n", k);
}

void dump_config()
{
	char	*tmp;
	GError	*error = NULL;

	fprintf(STD_OUT, APP_NAME " config (from file):\n");
	if (g_file_get_contents(get_datafile_full_name_from_name(CONFIG_FILE), &tmp, NULL, &error)) {
		fprintf(STD_OUT, "%s", tmp);
		g_free(tmp);
	} else if (error != NULL) {
		fprintf(STD_ERR, "%s\n", error->message);
		g_error_free(error);
	} else
		fprintf(STD_ERR, "Can't open file '%s': %s\n",
			get_datafile_full_name_from_name(CONFIG_FILE), strerror(errno));
}
