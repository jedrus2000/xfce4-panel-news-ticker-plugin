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

static GtkWidget	*dialog, *sc_win;
static FList		*flist;
static int		f_index;	/* Starting at 0 (row starts at 1) */
static char		url[FILE_NAME_MAXLEN + 1];

enum {COLUMN_INT, COLUMN_STRING_TITLE, COLUMN_STRING_URL, N_COLUMNS};

static int enter_key_pressed(GtkWidget *dialog2, GdkEventKey *event_key)
{
	if (event_key->keyval == GDK_Return) {
		gtk_dialog_response(GTK_DIALOG(dialog2), GTK_RESPONSE_OK);
		return TRUE;
	} else
		return FALSE;
}

static int mouse_over_area(GtkWidget *widget, GdkEvent *event)
{
	static int	i = 0;

	widget = widget;
	if (get_params()->sfeedpicker_autoclose == 'y' &&\
			event->type == GDK_LEAVE_NOTIFY && i == 0) {
		i = 1;
		return TRUE;
	} else if (get_params()->sfeedpicker_autoclose == 'y' &&\
			event->type == GDK_LEAVE_NOTIFY && i == 1) {
		i = 0;
		force_quit_dialog(dialog);
		return TRUE;
	} else
		return FALSE;
}

/* Tree selection callback - get URL */
static int tree_selection_changed(GtkTreeSelection *selection)
{
	GtkTreeModel	*tree_model;
	GtkTreeIter	iter;
	char		*str_url;

	if (gtk_tree_selection_get_selected(selection, &tree_model, &iter)) {
		gtk_tree_model_get(tree_model, &iter,
			COLUMN_INT, &f_index,
			COLUMN_STRING_URL, &str_url,
			-1);
		str_n_cpy(url, (const char *)str_url, FILE_NAME_MAXLEN);
		g_free(str_url);
	}
	gtk_dialog_response(GTK_DIALOG(dialog), GTK_RESPONSE_NONE);
	return TRUE;
}

/* Catch double-click on tree view */
static int double_click_on_tree_view(GtkTreeView *tree_view, GtkTreePath *tree_path)
{
	GtkTreeModel	*tree_model;
	GtkTreeIter	iter;
	char		*str_url;

	tree_model = gtk_tree_view_get_model(tree_view);
	if (gtk_tree_model_get_iter(tree_model, &iter, tree_path)) {
		gtk_tree_model_get(tree_model, &iter,
			COLUMN_INT, &f_index,
			COLUMN_STRING_URL, &str_url,
			-1);
		str_n_cpy(url, (const char *)str_url, FILE_NAME_MAXLEN);
		g_free(str_url);
	}
	gtk_dialog_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);
	return TRUE;
}

/* Clear the list before filling it */
static void fill_list_store_from_flnode(GtkListStore *list_store)
{
	FList		*flist2;
	GtkTreeIter	iter;
	int		i = 0;

	gtk_list_store_clear(list_store);
	if (IS_FLIST(flist))
		for (flist2 = f_list_first(flist); IS_FLIST(flist2); flist2 = flist2->next) {
			gtk_list_store_append(list_store, &iter);
			gtk_list_store_set(list_store, &iter,
				COLUMN_INT, i,
				COLUMN_STRING_TITLE, flist2->title,
				COLUMN_STRING_URL, flist2->url,
				-1);
			i++;
		}
}

/* Actually ***selected feed*** picker. */
void quick_feed_picker()
{
	GtkTreeView		*tree_view;
	GtkListStore		*list_store;
	GtkCellRenderer		*renderer1, *renderer2;
	GtkTreeViewColumn	*column1, *column2;
	GtkTreeSelection	*selection;
	int			response;

	dialog = gtk_dialog_new_with_buttons(
			"Selected Feed Picker", GTK_WINDOW(get_ticker_env()->win),
			GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
			NULL);

	set_tickr_icon_to_dialog(GTK_WINDOW(dialog));
	gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_MOUSE/*GTK_WIN_POS_CENTER*/);
	gtk_window_set_resizable(GTK_WINDOW(dialog), FALSE);
	gtk_container_set_border_width(GTK_CONTAINER(dialog), 0);
	gtk_widget_set_size_request(dialog, 600, 150);
	/* Not sure about that:
	if (get_params()->sfeedpicker_autoclose == 'y')
		gtk_window_set_decorated(GTK_WINDOW(dialog), FALSE);*/

	sc_win = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sc_win), GTK_POLICY_AUTOMATIC,
		GTK_POLICY_AUTOMATIC);
	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), sc_win);

	list_store = gtk_list_store_new(N_COLUMNS, G_TYPE_INT, G_TYPE_STRING, G_TYPE_STRING);
	if (IS_FLIST(get_feed_selection())) {
		flist = f_list_first(get_feed_selection());
		fill_list_store_from_flnode(list_store);
	} else {
		warning(BLOCK, "No feed selection available\n",
			"(You have set 'Mouse Wheel acts on: Feed' and\n"
			"either there is no feed list or no feed has been selected)");
		gtk_widget_destroy(dialog);
		return;
	}

	tree_view = GTK_TREE_VIEW(gtk_tree_view_new_with_model(GTK_TREE_MODEL(list_store)));
	renderer1 = gtk_cell_renderer_text_new();
	column1 = gtk_tree_view_column_new_with_attributes("Selected Feed Title", renderer1,
		"text", COLUMN_STRING_TITLE, NULL);
	gtk_tree_view_append_column(tree_view, column1);
	renderer2 = gtk_cell_renderer_text_new();
	column2 = gtk_tree_view_column_new_with_attributes("Selected Feed URL", renderer2,
		"text", COLUMN_STRING_URL, NULL);
	gtk_tree_view_append_column(tree_view, column2);
	gtk_container_add(GTK_CONTAINER(sc_win), GTK_WIDGET(tree_view));

	selection = gtk_tree_view_get_selection(tree_view);
	gtk_tree_selection_set_mode(selection, GTK_SELECTION_SINGLE);

	g_signal_connect(G_OBJECT(dialog), "key-press-event", G_CALLBACK(esc_key_pressed), NULL);
	g_signal_connect(G_OBJECT(dialog), "key-press-event", G_CALLBACK(enter_key_pressed), NULL);
	g_signal_connect(G_OBJECT(dialog), "delete_event", G_CALLBACK(force_quit_dialog), NULL);
	g_signal_connect(G_OBJECT(selection), "changed", G_CALLBACK(tree_selection_changed), NULL);
	g_signal_connect(G_OBJECT(tree_view), "row_activated", G_CALLBACK(double_click_on_tree_view), NULL);
	g_signal_connect(G_OBJECT(tree_view), "leave-notify-event", G_CALLBACK(mouse_over_area), NULL);

	gtk_widget_show_all(dialog);
	if (IS_FLIST(flist) && (get_resource()->id)[0] != '\0') {
		if ((f_index = f_list_search(flist, get_resource()->id)) > -1)
			highlight_and_go_to_row(tree_view, selection, f_index + 1);
		/*
		 * Try original URL in case of HTTP redirects.
		 * TODO: Find best practice - Should new URL replace original URL ?
		 */
		else if ((f_index = f_list_search(flist, get_resource()->orig_url)) > -1)
			highlight_and_go_to_row(tree_view, selection, f_index + 1);
	}
	while ((response = gtk_dialog_run(GTK_DIALOG(dialog))) != GTK_RESPONSE_CANCEL_CLOSE) {
		if (response == GTK_RESPONSE_OK) {
			if ((f_index = f_list_search(flist, url)) > -1) {
				set_feed_selection(f_list_nth(flist, f_index + 1));
				get_ticker_env()->reload_rq = TRUE;
			}
			break;
		}
	}
	gtk_widget_destroy(dialog);
}
