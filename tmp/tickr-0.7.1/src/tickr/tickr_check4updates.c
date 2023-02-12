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

static void new_version_available_win(char *version_num)
{
	GtkWidget	*dialog, *table, *label[2], *close_but;
	char		str1[256], str2[256];

	gtk_window_set_keep_above(GTK_WINDOW(get_ticker_env()->win), FALSE);

	dialog = gtk_dialog_new_with_buttons(
			"New upstream version found", GTK_WINDOW(get_ticker_env()->win),
			GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
			NULL);

	close_but = gtk_dialog_add_button(GTK_DIALOG(dialog), GTK_STOCK_CLOSE, GTK_RESPONSE_CANCEL_CLOSE);
	close_but = close_but;

	set_tickr_icon_to_dialog(GTK_WINDOW(dialog));
	gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);
	gtk_window_set_resizable(GTK_WINDOW(dialog), FALSE);

	g_signal_connect(G_OBJECT(dialog), "key-press-event", G_CALLBACK(esc_key_pressed), NULL);
	g_signal_connect(G_OBJECT(dialog), "delete_event", G_CALLBACK(force_quit_dialog), NULL);

	table = gtk_table_new(2, 1, FALSE);
	gtk_table_set_row_spacings(GTK_TABLE(table), 5);
	gtk_container_set_border_width(GTK_CONTAINER(table), 15);
	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), table);

	snprintf(str1, 256, "%s version <b>%s</b> is available for download from:", APP_NAME,
		version_num);
	snprintf(str2, 256, "<a href=\"%s\">%s</a>", DOWNLOAD_URL, DOWNLOAD_URL);
	label[0] = gtk_label_new(str1);
	gtk_label_set_use_markup(GTK_LABEL(label[0]), TRUE);
	gtk_misc_set_alignment(GTK_MISC(label[0]), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(table), label[0], 0, 1, 0, 1);
	label[1] = gtk_label_new(str2);
	gtk_label_set_use_markup(GTK_LABEL(label[1]), TRUE);
	gtk_misc_set_alignment(GTK_MISC(label[1]), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(table), label[1], 0, 1, 1, 2);

	gtk_widget_show_all(dialog);
	while (gtk_dialog_run(GTK_DIALOG(dialog)) != GTK_RESPONSE_CANCEL_CLOSE);

	gtk_widget_destroy(dialog);
	check_main_win_always_on_top();
}

void check_for_updates()
{
	sockt	sock;
	char	*response, *new_url;
	int	recv_status;
	char	last_stable_version[32], installed_version[32], *p;
	int	id_str_len, resp_len;
	int	info = OK + 1;

	INFO_OUT("Checking for updates:\n")

	if (connect_with_url(&sock, CHECK4UPDATES_URL) == OK) {
		if (get_http_response(sock, "GET", "", CHECK4UPDATES_URL, &new_url, &response, &recv_status) == OK) {
			id_str_len = strlen(CHECK4UPDATES_ID_STR);
			resp_len = strlen(response);
			p = response;
			last_stable_version[0] = '\0';

			while (p < response + resp_len - id_str_len) {
				if (strncmp(p, CHECK4UPDATES_ID_STR, id_str_len) == 0) {
					str_n_cpy(last_stable_version, p + id_str_len, 31);
					remove_trailing_whitespaces_from_str(last_stable_version);
					break;
				} else
					p++;
			}

			if (last_stable_version[0] != '\0') {
				p = installed_version;
				str_n_cpy(p, APP_V_NUM, 31);

				while (*p != '\0') {
					p++;
					if (*p == '~') {
						*p =  '\0';
						p--;
						*p -= 1;
						break;
					}
				}

				if (strcmp(last_stable_version, installed_version) > 0) {
					new_version_available_win(last_stable_version);
					INFO_OUT("%s version %s is available for download from: <%s>\n",
						APP_NAME, last_stable_version, DOWNLOAD_URL)
				} else {
					info_win("Check for Updates",
						"\n            " APP_NAME " is up to date  :)            \n",
						INFO, FALSE);
					INFO_OUT(APP_NAME " is up to date  :)\n")
				}
				info = OK;
			}
			free2(response);
		}
		CLOSE_SOCK(sock);
	}
	if (info != OK)
		warning(BLOCK, "Couldn't retrieve requested information from website");
}
