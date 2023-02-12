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

static Authentication	*auth;
static Proxy		*proxy;

static GtkWidget	*dialog, *entry_user, *entry_psw, *check_but1;
static GtkWidget	*entry_host, *entry_port, *entry_proxy_user, *entry_proxy_psw;
static GtkWidget	*spinbut_connect_timeout, *spinbut_send_recv_timeout;
static GtkObject	*adj_connect_timeout, *adj_send_recv_timeout;
static GtkWidget	*check_but2, *check_but3, *label[16];

void init_authentication()
{
	static Authentication auth0;

	auth = &auth0;
	auth->use_authentication = FALSE;
	auth->user[0] = '\0';
	auth->psw[0] = '\0';
	auth->auth_str[0] = '\0';
}

void init_proxy()
{
	static Proxy proxy0;

	proxy = &proxy0;
	proxy->use_proxy = FALSE;
	proxy->host[0] = '\0';
	proxy->port[0] = '\0';
	proxy->use_proxy_authentication = FALSE;
	proxy->user[0] = '\0';
	proxy->psw[0] = '\0';
	proxy->auth_str[0] = '\0';
}

static int enter_key_pressed_in_entry(GtkWidget *widget)
{
	widget = widget;
	gtk_dialog_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);
	return TRUE;
}

static void set_entries_sensitive1(gboolean state)
{
	gtk_widget_set_sensitive(label[1], state);
	gtk_widget_set_sensitive(entry_user, state);
	gtk_widget_set_sensitive(label[2], state);
	gtk_widget_set_sensitive(entry_psw, state);
}

static void set_entries_sensitive2(gboolean state)
{
	gtk_widget_set_sensitive(label[4], state);
	gtk_widget_set_sensitive(entry_host, state);
	gtk_widget_set_sensitive(label[5], state);
	gtk_widget_set_sensitive(entry_port, state);
	gtk_widget_set_sensitive(check_but3, state);
}

static void set_entries_sensitive3(gboolean state)
{
	gtk_widget_set_sensitive(label[6], state);
	gtk_widget_set_sensitive(entry_proxy_user, state);
	gtk_widget_set_sensitive(label[7], state);
	gtk_widget_set_sensitive(entry_proxy_psw, state);
}

static int check_but1_toggled(GtkWidget *check_button)
{
	auth->use_authentication = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button));
	set_entries_sensitive1(auth->use_authentication);
	return TRUE;
}

static int check_but2_toggled(GtkWidget *check_button)
{
	proxy->use_proxy = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button));
	set_entries_sensitive2(proxy->use_proxy);
	set_entries_sensitive3(proxy->use_proxy && proxy->use_proxy_authentication);
	return TRUE;
}

static int check_but3_toggled(GtkWidget *check_button)
{
	proxy->use_proxy_authentication = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button));
	set_entries_sensitive3(proxy->use_proxy && proxy->use_proxy_authentication);
	return TRUE;
}

void compute_auth_and_proxy_str()
{
	char *tmp1, *tmp2;

	if (auth->user[0] != '\0' && auth->psw[0] != '\0') {
		tmp1 = l_str_new(auth->user);
		tmp1 = l_str_cat(tmp1, ":");
		tmp1 = l_str_cat(tmp1, auth->psw);
		tmp2 = g_base64_encode((guchar *)tmp1, (gsize)strlen(tmp1));
		str_n_cpy(auth->auth_str, tmp2, AUTH_STR_MAXLEN);
		g_free(tmp2);
		l_str_free(tmp1);
	} else
		auth->auth_str[0] = '\0';
	if (proxy->host[0] != '\0') {
		tmp1 = l_str_new(proxy->host);
		if (proxy->port[0] == '\0')
			str_n_cpy(proxy->port, PROXYPORT, PROXY_PORT_MAXLEN);
		if (proxy->port[0] != '\0') {
			tmp1 = l_str_cat(tmp1, ":");
			tmp1 = l_str_cat(tmp1, proxy->port);
		}
		str_n_cpy(proxy->proxy_str, tmp1, PROXY_STR_MAXLEN);
		l_str_free(tmp1);
	} else
		proxy->proxy_str[0] = '\0';
	if (proxy->use_proxy_authentication && proxy->user[0] != '\0' && proxy->psw[0] != '\0') {
		tmp1 = l_str_new(proxy->user);
		tmp1 = l_str_cat(tmp1, ":");
		tmp1 = l_str_cat(tmp1, proxy->psw);
		tmp2 = g_base64_encode((guchar *)tmp1, (gsize)strlen(tmp1));
		str_n_cpy(proxy->auth_str, tmp2, PROXY_AUTH_STR_MAXLEN);
		g_free(tmp2);
		l_str_free(tmp1);
	} else
		proxy->auth_str[0] = '\0';
}

/* Must be initialized with init_authentication() and init_proxy() */
int connection_settings(connection_settings_page page)
{
	TickerEnv	*env = get_ticker_env();
	Params		*prm = get_params();
	GtkWidget	*notebook, *table1, *table2, *table3, *hbox;
	int		response = GTK_RESPONSE_CANCEL_CLOSE;

	gtk_window_set_keep_above(GTK_WINDOW(env->win), FALSE);

	dialog = gtk_dialog_new_with_buttons(
			"Connection Settings", GTK_WINDOW(env->win),
			GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL_CLOSE,
			GTK_STOCK_OK, GTK_RESPONSE_OK,
			NULL);

	set_tickr_icon_to_dialog(GTK_WINDOW(dialog));
	gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);
	gtk_window_set_resizable(GTK_WINDOW(dialog), FALSE);
	/*
	 * Fill structs from params
	 */
	auth->use_authentication = (prm->use_authentication == 'y') ? TRUE : FALSE;
	str_n_cpy(proxy->user, prm->user, USER_MAXLEN);
	proxy->use_proxy = (prm->use_proxy == 'y') ? TRUE : FALSE;
	str_n_cpy(proxy->host, prm->proxy_host, PROXY_HOST_MAXLEN);
	str_n_cpy(proxy->port, prm->proxy_port, PROXY_PORT_MAXLEN);
	proxy->use_proxy_authentication = (prm->use_proxy_authentication == 'y') ? TRUE : FALSE;
	str_n_cpy(proxy->user, prm->proxy_user, PROXY_USER_MAXLEN);

	notebook = gtk_notebook_new();
	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), notebook);

	label[0]= gtk_label_new("Configure HTTP Basic Authentication");
	table1 = gtk_table_new(4/*6*/, 2, TRUE);
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), table1, label[0]);

	gtk_table_set_row_spacings(GTK_TABLE(table1), 5);
	gtk_table_set_col_spacings(GTK_TABLE(table1), 5);
	gtk_container_set_border_width(GTK_CONTAINER(table1), 10);

	check_but1 = gtk_check_button_new_with_label(" Use Authentication");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_but1), auth->use_authentication);
	gtk_table_attach_defaults(GTK_TABLE(table1), check_but1, 0, 1, 0, 1);

	label[1] = gtk_label_new("Username:");
	gtk_label_set_use_markup(GTK_LABEL(label[1]), TRUE);
	gtk_misc_set_alignment(GTK_MISC(label[1]), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(table1), label[1], 0, 1, 1, 2);
	entry_user = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(entry_user), USER_MAXLEN);
	gtk_entry_set_text(GTK_ENTRY(entry_user), auth->user);
	gtk_table_attach_defaults(GTK_TABLE(table1), entry_user, 1, 2, 1, 2);

	label[2] = gtk_label_new("Password:");
	gtk_label_set_use_markup(GTK_LABEL(label[2]), TRUE);
	gtk_misc_set_alignment(GTK_MISC(label[2]), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(table1), label[2], 0, 1, 2, 3);
	entry_psw = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(entry_psw), PSW_MAXLEN);
	gtk_entry_set_visibility(GTK_ENTRY(entry_psw), FALSE);
	gtk_entry_set_text(GTK_ENTRY(entry_psw), auth->psw);
	gtk_table_attach_defaults(GTK_TABLE(table1), entry_psw, 1, 2, 2, 3);
	gtk_entry_set_visibility(GTK_ENTRY(entry_psw), FALSE);

	label[3] = gtk_label_new("Configure HTTP Proxy Server");
	table2 = gtk_table_new(6, 2, TRUE);
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), table2, label[3]);

	gtk_table_set_row_spacings(GTK_TABLE(table2), 5);
	gtk_table_set_col_spacings(GTK_TABLE(table2), 5);
	gtk_container_set_border_width(GTK_CONTAINER(table2), 10);

	check_but2 = gtk_check_button_new_with_label(" Connect via Proxy");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_but2), proxy->use_proxy);
	gtk_table_attach_defaults(GTK_TABLE(table2), check_but2, 0, 1, 0, 1);

	label[4] = gtk_label_new("Host:");
	gtk_label_set_use_markup(GTK_LABEL(label[4]), TRUE);
	gtk_misc_set_alignment(GTK_MISC(label[4]), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(table2), label[4], 0, 1, 1, 2);
	entry_host = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(entry_host), PROXY_HOST_MAXLEN);
	gtk_entry_set_text(GTK_ENTRY(entry_host), proxy->host);
	gtk_table_attach_defaults(GTK_TABLE(table2), entry_host, 1, 2, 1, 2);

	label[5] = gtk_label_new("Port:");
	gtk_label_set_use_markup(GTK_LABEL(label[5]), TRUE);
	gtk_misc_set_alignment(GTK_MISC(label[5]), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(table2), label[5], 0, 1, 2, 3);
	entry_port = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(entry_port), PROXY_PORT_MAXLEN);
	if (proxy->port[0] == '\0')
		str_n_cpy(proxy->port, PROXYPORT, PROXY_PORT_MAXLEN);
	gtk_entry_set_text(GTK_ENTRY(entry_port), proxy->port);
	gtk_table_attach_defaults(GTK_TABLE(table2), entry_port, 1, 2, 2, 3);

	check_but3 = gtk_check_button_new_with_label(" Proxy requires Authentication");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_but3), proxy->use_proxy_authentication);
	gtk_table_attach_defaults(GTK_TABLE(table2), check_but3, 0, 1, 3, 4);

	label[6] = gtk_label_new("    Username:");
	gtk_label_set_use_markup(GTK_LABEL(label[6]), TRUE);
	gtk_misc_set_alignment(GTK_MISC(label[6]), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(table2), label[6], 0, 1, 4, 5);
	entry_proxy_user = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(entry_proxy_user), PROXY_USER_MAXLEN);
	gtk_entry_set_text(GTK_ENTRY(entry_proxy_user), proxy->user);
	gtk_table_attach_defaults(GTK_TABLE(table2), entry_proxy_user, 1, 2, 4, 5);

	label[7] = gtk_label_new("    Password:");
	gtk_label_set_use_markup(GTK_LABEL(label[7]), TRUE);
	gtk_misc_set_alignment(GTK_MISC(label[7]), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(table2), label[7], 0, 1, 5, 6);
	entry_proxy_psw = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(entry_proxy_psw), PROXY_PSW_MAXLEN);
	gtk_entry_set_visibility(GTK_ENTRY(entry_proxy_psw), FALSE);
	gtk_entry_set_text(GTK_ENTRY(entry_proxy_psw), proxy->psw);
	gtk_table_attach_defaults(GTK_TABLE(table2), entry_proxy_psw, 1, 2, 5, 6);

	hbox = gtk_hbox_new(0, FALSE);
	label[8] = gtk_label_new("    ");
	gtk_box_pack_start(GTK_BOX(hbox), label[8], FALSE, FALSE, 0);

	label[9] = gtk_label_new("Override Default Timeouts");
	table3 = gtk_table_new(4/*6*/, 2, TRUE);
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), table3, label[9]);

	gtk_table_set_row_spacings(GTK_TABLE(table3), 5);
	gtk_table_set_col_spacings(GTK_TABLE(table3), 5);
	gtk_container_set_border_width(GTK_CONTAINER(table3), 10);

	label[10] = gtk_label_new("Connect Timeout (default = 5 s):");
	gtk_label_set_use_markup(GTK_LABEL(label[10]), TRUE);
	gtk_misc_set_alignment(GTK_MISC(label[10]), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(table3), label[10], 0, 1, 0, 1);
	adj_connect_timeout = gtk_adjustment_new(prm->connect_timeout, 1, 60, 1, 5, 0);
	spinbut_connect_timeout = gtk_spin_button_new(GTK_ADJUSTMENT(adj_connect_timeout), 0.0, 0);
	gtk_table_attach_defaults(GTK_TABLE(table3), spinbut_connect_timeout, 1, 2, 0, 1);

	label[11] = gtk_label_new("Send/Recv Timeout (default = 1 s):");
	gtk_label_set_use_markup(GTK_LABEL(label[11]), TRUE);
	gtk_misc_set_alignment(GTK_MISC(label[11]), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(table3), label[11], 0, 1, 1, 2);
	adj_send_recv_timeout = gtk_adjustment_new(prm->send_recv_timeout, 1, 60, 1, 5, 0);
	spinbut_send_recv_timeout = gtk_spin_button_new(GTK_ADJUSTMENT(adj_send_recv_timeout), 0.0, 0);
	gtk_table_attach_defaults(GTK_TABLE(table3), spinbut_send_recv_timeout, 1, 2, 1, 2);

	label[12] = gtk_label_new("<b>Warning</b>: "
	/*label[12] = gtk_label_new("<span color=\"red\"><b>Warning:</b></span> "*/
		"Change these settings *only* if you know what you're doing,\n"
		"as high values might make the application almost unresponsive.");
	gtk_label_set_use_markup(GTK_LABEL(label[12]), TRUE);
	gtk_misc_set_alignment(GTK_MISC(label[12]), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(table3), label[12], 0, 2, 2, 3);

	set_entries_sensitive1(auth->use_authentication);
	set_entries_sensitive2(proxy->use_proxy);
	set_entries_sensitive3(proxy->use_proxy && proxy->use_proxy_authentication);
	g_signal_connect(G_OBJECT(check_but1), "toggled", G_CALLBACK(check_but1_toggled), NULL);
	g_signal_connect(G_OBJECT(check_but2), "toggled", G_CALLBACK(check_but2_toggled), NULL);
	g_signal_connect(G_OBJECT(check_but3), "toggled", G_CALLBACK(check_but3_toggled), NULL);

	g_signal_connect(G_OBJECT(entry_user), "activate", G_CALLBACK(enter_key_pressed_in_entry), NULL);
	g_signal_connect(G_OBJECT(entry_psw), "activate", G_CALLBACK(enter_key_pressed_in_entry), NULL);
	g_signal_connect(G_OBJECT(entry_host), "activate", G_CALLBACK(enter_key_pressed_in_entry), NULL);
	g_signal_connect(G_OBJECT(entry_port), "activate", G_CALLBACK(enter_key_pressed_in_entry), NULL);
	g_signal_connect(G_OBJECT(entry_proxy_user), "activate", G_CALLBACK(enter_key_pressed_in_entry), NULL);
	g_signal_connect(G_OBJECT(entry_proxy_psw), "activate", G_CALLBACK(enter_key_pressed_in_entry), NULL);
	g_signal_connect(G_OBJECT(spinbut_connect_timeout), "activate", G_CALLBACK(enter_key_pressed_in_entry), NULL);
	g_signal_connect(G_OBJECT(spinbut_send_recv_timeout), "activate", G_CALLBACK(enter_key_pressed_in_entry), NULL);

	gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook), (int)page);

	gtk_widget_show_all(dialog);
	gtk_window_set_focus(GTK_WINDOW(dialog), NULL);

	env->suspend_rq = TRUE;

	if ((response = gtk_dialog_run(GTK_DIALOG(dialog))) == GTK_RESPONSE_OK) {
		/*
		 * Configure authentication
		 */
		auth->use_authentication = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_but1));
		str_n_cpy(auth->user, (char *)gtk_entry_get_text(GTK_ENTRY(entry_user)), USER_MAXLEN);
		/* TODO: How to allow username string to contain inside spaces ? */
		remove_surrounding_whitespaces_from_str(auth->user);
		str_n_cpy(auth->psw, (char *)gtk_entry_get_text(GTK_ENTRY(entry_psw)), PSW_MAXLEN);
		remove_char_from_str(auth->psw, ' ');
		/*
		 * Configure proxy
		 */
		proxy->use_proxy = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_but2));
		str_n_cpy(proxy->host, (char *)gtk_entry_get_text(GTK_ENTRY(entry_host)), PROXY_HOST_MAXLEN);
		remove_char_from_str(proxy->host, ' ');
		str_n_cpy(proxy->port, (char *)gtk_entry_get_text(GTK_ENTRY(entry_port)), PROXY_PORT_MAXLEN);
		remove_char_from_str(proxy->port, ' ');
		/*
		 * Configure proxy authentication
		 */
		proxy->use_proxy_authentication = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_but3));
		str_n_cpy(proxy->user, (char *)gtk_entry_get_text(GTK_ENTRY(entry_proxy_user)), PROXY_USER_MAXLEN);
		/* TODO: How to allow username string to contain inside spaces ? */
		remove_surrounding_whitespaces_from_str(proxy->user);
		str_n_cpy(proxy->psw, (char *)gtk_entry_get_text(GTK_ENTRY(entry_proxy_psw)), PROXY_PSW_MAXLEN);
		remove_char_from_str(proxy->psw, ' ');
		/*
		 * Compute auth and proxy stuff
		 */
		compute_auth_and_proxy_str();
		/*
		 * Save struct members as params
		 */
		prm->use_authentication = auth->use_authentication ? 'y' : 'n';
		str_n_cpy(prm->user, auth->user, USER_MAXLEN);
		prm->use_proxy = proxy->use_proxy ? 'y' : 'n';
		str_n_cpy(prm->proxy_host, proxy->host, PROXY_HOST_MAXLEN);
		str_n_cpy(prm->proxy_port, proxy->port, PROXY_PORT_MAXLEN);
		prm->use_proxy_authentication = proxy->use_proxy_authentication ? 'y' : 'n';
		str_n_cpy(prm->proxy_user, proxy->user, PROXY_USER_MAXLEN);
		/*
		 * Get socket timeouts
		 */
		prm->connect_timeout = (int)gtk_spin_button_get_value(GTK_SPIN_BUTTON(spinbut_connect_timeout));
		prm->send_recv_timeout = (int)gtk_spin_button_get_value(GTK_SPIN_BUTTON(spinbut_send_recv_timeout));
		/*
		 * Set timeouts
		 */
		set_connect_timeout_sec(prm->connect_timeout);
		set_send_recv_timeout_sec(prm->send_recv_timeout);
		/*
		 * Then save everything to file
		 */
		save_to_config_file(prm);
	}
	gtk_widget_destroy(dialog);
	check_main_win_always_on_top();
	env->suspend_rq = FALSE;
	return response;
}

void set_use_authentication(zboolean value)
{
	auth->use_authentication = value;
}

zboolean get_use_authentication()
{
	return auth->use_authentication;
}

char *get_http_auth_user()
{
	return (char *)auth->user;
}

char *get_http_auth_psw()
{
	return (char *)auth->psw;
}

char *get_http_auth_str()
{
	return (char *)auth->auth_str;
}

void set_use_proxy(zboolean value)
{
	libetm_socket_set_use_proxy(value);
	proxy->use_proxy = libetm_socket_get_use_proxy();
}

zboolean get_use_proxy()
{
	return proxy->use_proxy;
}

char *get_proxy_host()
{
	return (char *)proxy->host;
}

char *get_proxy_port()
{
	return (char *)proxy->port;
}

char *get_proxy_str()
{
	return (char *)proxy->proxy_str;
}

void set_use_proxy_auth(zboolean value)
{
	proxy->use_proxy_authentication = value;
}

zboolean get_use_proxy_auth()
{
	return proxy->use_proxy_authentication;
}

char *get_proxy_auth_user()
{
	return (char *)proxy->user;
}

char *get_proxy_auth_psw()
{
	return (char *)proxy->psw;
}

char *get_proxy_auth_str()
{
	return (char *)proxy->auth_str;
}
