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

#define URL_ENTRY_LENGTH	100
#define RANK_ENTRY_LENGTH	FLIST_RANK_STR_LEN

enum {COLUMN_INT, COLUMN_BOOLEAN_CHECKED, COLUMN_STRING_RANK, COLUMN_STRING_TITLE, COLUMN_STRING_URL, N_COLUMNS};

static GtkWidget	*dialog, *url_entry, *rank_entry;
static FList		*flist;
static int		f_index;	/* Starting at 0 (row starts at 1) */
static char		home_feed[FILE_NAME_MAXLEN + 1];

/* Tree selection callback - update f_index and copy selected URL to url_entry (override) */
static int tree_selection_changed(GtkTreeSelection *selection)
{
	GtkTreeModel	*tree_model = NULL;
	GtkTreeIter	iter;
	char		*url_str, *rank_str;

	if (gtk_tree_selection_get_selected(selection, &tree_model, &iter)) {
		gtk_tree_model_get(tree_model, &iter,
			COLUMN_INT, &f_index,
			COLUMN_STRING_URL, &url_str,
			COLUMN_STRING_RANK, &rank_str,
			-1);
		gtk_entry_set_text(GTK_ENTRY(url_entry), url_str);
		if (get_params()->enable_feed_ordering == 'y')
			gtk_entry_set_text(GTK_ENTRY(rank_entry), itoa2(atoi(rank_str)));
		if (IS_FLIST(flist)) {
			if ((f_index = f_list_search(flist, url_str)) > -1)
				flist = f_list_nth(flist, f_index + 1);
		}
		g_free(url_str);
		g_free(rank_str);
	}
	return TRUE;
}

/* Catch double-click on tree view */
static int double_click_on_tree_view(GtkTreeView *tree_view, GtkTreePath *tree_path)
{
	GtkTreeModel	*tree_model = NULL;
	GtkTreeIter	iter;
	char		*url_str, *rank_str;

	tree_model = gtk_tree_view_get_model(tree_view);
	if (gtk_tree_model_get_iter(tree_model, &iter, tree_path)) {
		gtk_tree_model_get(tree_model, &iter,
			COLUMN_INT, &f_index,
			COLUMN_STRING_URL, &url_str,
			COLUMN_STRING_RANK, &rank_str,
			-1);
		gtk_entry_set_text(GTK_ENTRY(url_entry), url_str);
		if (get_params()->enable_feed_ordering == 'y')
			gtk_entry_set_text(GTK_ENTRY(rank_entry), rank_str);
		if (IS_FLIST(flist)) {
			if ((f_index = f_list_search(flist, url_str)) > -1)
				flist = f_list_nth(flist, f_index + 1);
		}
		g_free(url_str);
		g_free(rank_str);
	}
	gtk_dialog_response(GTK_DIALOG(dialog), GTK_RESPONSE_SINGLE);
	return TRUE;
}

static int enter_key_pressed_in_entry(GtkWidget *widget)
{
	widget = widget;
	gtk_dialog_response(GTK_DIALOG(dialog), GTK_RESPONSE_ADD_UPD);
	return TRUE;
}

static int clear_entry(GtkWidget *widget)
{
	widget = widget;
	gtk_entry_set_text(GTK_ENTRY(url_entry), "");
	if (get_params()->enable_feed_ordering == 'y')
		gtk_entry_set_text(GTK_ENTRY(rank_entry), "");
	gtk_widget_grab_focus(GTK_WIDGET(url_entry));
	return TRUE;
}

static int checkbox_toggled(GtkCellRendererToggle *renderer, char *path_str, gpointer tree_view)
{
	GtkTreeModel	*tree_model;
	GtkTreeIter	iter;
	FList		*flist2;
	gboolean	checked;

	renderer = renderer;
	tree_model = gtk_tree_view_get_model(GTK_TREE_VIEW(tree_view));
	if (gtk_tree_model_get_iter_from_string(tree_model, &iter, path_str)) {
		gtk_tree_model_get(tree_model, &iter,
			COLUMN_INT, &f_index,
			COLUMN_BOOLEAN_CHECKED, &checked,
			-1);
		checked = !checked;
		gtk_list_store_set(GTK_LIST_STORE(tree_model), &iter,
			COLUMN_BOOLEAN_CHECKED, checked,
			-1);
		if (IS_FLIST(flist)) {
			flist2 = f_list_nth(flist, f_index + 1);
			if (IS_FLIST(flist2))
				flist2->selected = checked;
		}
	}
	return TRUE;
}

static void select_all(GtkTreeView *tree_view)
{
	GtkTreeModel	*tree_model;
	GtkTreeIter	iter;
	FList		*flist2;

	tree_model = gtk_tree_view_get_model(tree_view);
	if (gtk_tree_model_get_iter_first(tree_model, &iter)) {
		do {
			gtk_list_store_set(GTK_LIST_STORE(tree_model), &iter,
				COLUMN_BOOLEAN_CHECKED, TRUE,
				-1);
		} while (gtk_tree_model_iter_next(tree_model, &iter));
		if (IS_FLIST(flist)) {
			for (flist2 = f_list_first(flist); IS_FLIST(flist2); flist2 = flist2->next)
				flist2->selected = TRUE;
		}
	}
}

static void unselect_all(GtkTreeView *tree_view)
{
	GtkTreeModel	*tree_model;
	GtkTreeIter	iter;
	FList		*flist2;

	tree_model = gtk_tree_view_get_model(tree_view);
	if (gtk_tree_model_get_iter_first(tree_model, &iter)) {
		do {
			gtk_list_store_set(GTK_LIST_STORE(tree_model), &iter,
				COLUMN_BOOLEAN_CHECKED, FALSE,
				-1);
		} while (gtk_tree_model_iter_next(tree_model, &iter));
		if (IS_FLIST(flist)) {
			for (flist2 = f_list_first(flist); IS_FLIST(flist2); flist2 = flist2->next)
				flist2->selected = FALSE;
		}
	}
}

static void clear_ranking(GtkTreeView *tree_view)
{
	GtkTreeModel	*tree_model;
	GtkTreeIter	iter;
	FList		*flist2;
	char		rank[FLIST_RANK_STR_LEN + 1];

	tree_model = gtk_tree_view_get_model(tree_view);
	str_n_cpy(rank, BLANK_STR_16, FLIST_RANK_STR_LEN);
	/*snprintf(rank, FLIST_RANK_STR_LEN + 1, FLIST_RANK_FORMAT, 0);*/
	if (gtk_tree_model_get_iter_first(tree_model, &iter)) {
		do {
			gtk_list_store_set(GTK_LIST_STORE(tree_model), &iter,
				COLUMN_STRING_RANK, rank,
				-1);
		} while (gtk_tree_model_iter_next(tree_model, &iter));
		if (IS_FLIST(flist)) {
			for (flist2 = f_list_first(flist); IS_FLIST(flist2); flist2 = flist2->next)
				str_n_cpy(flist2->rank, rank, FLIST_RANK_STR_LEN);
		}
	}
}

/* Select and scroll to row */
void highlight_and_go_to_row(GtkTreeView *tree_view, GtkTreeSelection *selection, int row)
{
	GtkTreeModel	*tree_model;
	GtkTreePath	*tree_path;
	GtkTreeIter	iter;
	int		i = 0;

	if (row > 0) {
		tree_model = gtk_tree_view_get_model(tree_view);
		tree_path = gtk_tree_path_new_first();
		if (gtk_tree_model_get_iter_first(tree_model, &iter)) {
			while (gtk_tree_model_iter_next(tree_model, &iter) && i++ < row - 1)
				gtk_tree_path_next(tree_path);
			gtk_tree_selection_select_path(selection, tree_path);
			gtk_tree_view_set_cursor_on_cell(tree_view, tree_path, NULL, NULL, FALSE);
			gtk_tree_view_scroll_to_cell(tree_view, tree_path, NULL, FALSE, 0.5, 0);
		}
	}
}

/* Clear the list before filling it */
static void fill_list_store_from_flnode(GtkListStore *list_store)
{
	FList		*flist2;
	GtkTreeIter	iter;
	int		i = 0;

	gtk_list_store_clear(list_store);
	if (IS_FLIST(flist)) {
		for (flist2 = f_list_first(flist); IS_FLIST(flist2); flist2 = flist2->next) {
			gtk_list_store_append(list_store, &iter);
			gtk_list_store_set(list_store, &iter,
				COLUMN_INT, i,
				COLUMN_BOOLEAN_CHECKED, flist2->selected,
				COLUMN_STRING_RANK, flist2->rank,
				COLUMN_STRING_TITLE, flist2->title,
				COLUMN_STRING_URL, flist2->url,
				-1);
			i++;
		}
	}
}

/*
 * Check if URL is valid  and reacheable, and update it if so
 * feed_url max len = FILE_NAME_MAXLEN
 * feed_title max len = FEED_TITLE_MAXLEN
 * feed_title can be NULL
 */
int check_and_update_feed_url(char *feed_url, char *feed_title)
{
	char	*unesc_str;
	int	unused;

	unesc_str = g_uri_unescape_string((const char *)feed_url, NULL);
	if (unesc_str != NULL) {
		str_n_cpy(feed_url, unesc_str, FILE_NAME_MAXLEN);
		g_free(unesc_str);
	} else {
		DEBUG_INFO("URL is not valid\n")
		return RESOURCE_INVALID;
	}
	/* URL updated for moved-permanently redirects */
	if (fetch_resource(feed_url, get_datafile_full_name_from_name(TMP_FILE), feed_url) == OK) {
		get_feed_info(&unused, get_datafile_full_name_from_name(TMP_FILE), feed_title, NULL, NULL);
		g_unlink(get_datafile_full_name_from_name(TMP_FILE));
		return OK;
	} else {
		DEBUG_INFO("URL is unreachable or invalid\n")
		return RESOURCE_NOT_FOUND;
	}
}

/*
 * Use url_entry and rank_entry (global vars in this src file)
 * Update resrc (eventually)
 */
static int add_feed_to_flnode_and_list_store(Resource *resrc, GtkListStore *list_store)
{
	char	feed_url[FILE_NAME_MAXLEN + 1];
	char	feed_title[FEED_TITLE_MAXLEN + 1];
	char	rank[FLIST_RANK_STR_LEN + 1];
	FList	*flist2;
	int	check_status;

	str_n_cpy(feed_url, (char *)gtk_entry_get_text(GTK_ENTRY(url_entry)), FILE_NAME_MAXLEN);

	if ((check_status = check_and_update_feed_url(feed_url, feed_title)) == OK) {
		str_n_cpy(resrc->id, feed_url, FILE_NAME_MAXLEN);
		if (get_params()->enable_feed_ordering != 'y') {
			if (IS_FLIST(flist) && (flist2 = f_list_nth(flist, f_list_search(flist, resrc->id) + 1)) != NULL)
				str_n_cpy(rank, flist2->rank, FLIST_RANK_STR_LEN);
			else
				str_n_cpy(rank, BLANK_STR_16, FLIST_RANK_STR_LEN);
		} else {
			str_n_cpy(rank, (char *)gtk_entry_get_text(GTK_ENTRY(rank_entry)), FLIST_RANK_STR_LEN);
		}
		/* Fix LP bug #1272129 */
		if (IS_FLIST(flist))
			flist = f_list_last(flist);
		/* Added URL is set selected */
		flist = f_list_add_at_end(flist, feed_url, feed_title, TRUE, rank);
		flist = f_list_sort(flist);
		fill_list_store_from_flnode(list_store);
	} else {
		warning(BLOCK, "URL is unreachable or invalid (or whatever)");
	}
	return check_status;
}

/*
 * Open a dialog with a list of URLs to choose from and an URL entry.
 *
 * Entry format in URL list file:
 *	['*' (selected) or '-' (unselected) + "000" (3 chars rank) + URL [+ '>' + title] + '\n']
 *
 * Entry max length = FILE_NAME_MAXLEN
 * See also:	(UN)SELECTED_URL_CHAR/STR and TITLE_TAG_CHAR/STR in tickr.h
 * 		FLIST_RANK_FORMAT and FLIST_RANK_STR_LEN in tickr_list.h
 */
void manage_list_and_selection(Resource *resrc)
{
	TickerEnv		*env;
	GtkWidget		*sc_win, *hbox;
	GtkWidget		*cancel_but, *selectall_but, *unselectall_but, *clear_ranking_but;
	GtkWidget		*top_but, *current_but, *home_but, *remove_but, *add_but;
	GtkWidget		*single_but, *selection_but, *clear_but;
	GtkWidget		*tmp_label;
	GtkTreeView		*tree_view = NULL;
	GtkListStore		*list_store = NULL;
	GtkTreeModel		*tree_model = NULL;
	GtkTreeIter		iter;
	GtkCellRenderer		*renderer1 = NULL, *renderer2 = NULL, *renderer3 = NULL, *renderer4 = NULL;
	GtkTreeViewColumn	*column1 = NULL, *column2 = NULL, *column3 = NULL, *column4 = NULL;
	GtkTreeSelection	*selection = NULL;
	FList			*flist_bak = NULL, *flist2 = NULL;
	char			resrc_id_bak[FILE_NAME_MAXLEN + 1];
	int			response;
	int			cursor_position;
	char			*tooltip_text;

	env = get_ticker_env();
	env->suspend_rq = TRUE;
	gtk_window_set_keep_above(GTK_WINDOW(env->win), FALSE);

	dialog = gtk_dialog_new_with_buttons(
			"Feed Organizer (RSS/Atom)", GTK_WINDOW(env->win),
			GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
			NULL);

	cancel_but = gtk_dialog_add_button(GTK_DIALOG(dialog), GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL_CLOSE);
	selectall_but = gtk_dialog_add_button(GTK_DIALOG(dialog), "Select All", GTK_RESPONSE_SELECT_ALL);
	unselectall_but = gtk_dialog_add_button(GTK_DIALOG(dialog), "Unsel All", GTK_RESPONSE_UNSELECT_ALL);
	if (get_params()->enable_feed_ordering == 'y')
		clear_ranking_but = gtk_dialog_add_button(GTK_DIALOG(dialog), "Clear Rank", GTK_RESPONSE_CLEAR_RANKING);
	top_but = gtk_dialog_add_button(GTK_DIALOG(dialog), "Top", GTK_RESPONSE_TOP);
	current_but = gtk_dialog_add_button(GTK_DIALOG(dialog), "Current", GTK_RESPONSE_CURRENT);
	home_but = gtk_dialog_add_button(GTK_DIALOG(dialog), GTK_STOCK_HOME, GTK_RESPONSE_HOME);
	remove_but = gtk_dialog_add_button(GTK_DIALOG(dialog), GTK_STOCK_REMOVE, GTK_RESPONSE_REMOVE);
	add_but = gtk_dialog_add_button(GTK_DIALOG(dialog), "Add/Upd", GTK_RESPONSE_ADD_UPD);
	single_but = gtk_dialog_add_button(GTK_DIALOG(dialog), "OK (Single)", GTK_RESPONSE_SINGLE);
	selection_but = gtk_dialog_add_button(GTK_DIALOG(dialog), "OK (Selec)", GTK_RESPONSE_SELECTION);
	clear_but = gtk_button_new_with_label("Clear");

	/* Tooltips */
	tooltip_text = l_str_new("Will try to connect and add or update URL and title");
	gtk_widget_set_tooltip_text(add_but, tooltip_text);
	l_str_free(tooltip_text);
	tooltip_text = l_str_new("'Single selection' mode - Will read only the highlighted URL");
	gtk_widget_set_tooltip_text(single_but, tooltip_text);
	l_str_free(tooltip_text);
	tooltip_text = l_str_new("'Multiple selection' mode - Will read sequentially all selected URLs, "
		"starting from highlighted one - more exactly URL in entry (if any) / first one otherwise");
	gtk_widget_set_tooltip_text(selection_but, tooltip_text);
	l_str_free(tooltip_text);

	/* To get rid of these boring compiler warnings */
	cancel_but = cancel_but;
	selectall_but = selectall_but;
	unselectall_but = unselectall_but;
	clear_ranking_but = clear_ranking_but;
	top_but = top_but;
	current_but = current_but;
	home_but = home_but;
	remove_but = remove_but;
	add_but = add_but;
	single_but = single_but;
	selection_but = selection_but;
	clear_but = clear_but;
	/* (Until here) */

	set_tickr_icon_to_dialog(GTK_WINDOW(dialog));
	gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER_ALWAYS);

	sc_win = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sc_win),
		GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_container_set_border_width(GTK_CONTAINER(sc_win), 5);
	/* Whole window must be visible on netbooks as well */
	gtk_widget_set_size_request(sc_win, 1000, 450);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), sc_win, TRUE, TRUE, 0);

	str_n_cpy(home_feed, get_params()->homefeed, FILE_NAME_MAXLEN);
	list_store = gtk_list_store_new(N_COLUMNS, G_TYPE_INT, G_TYPE_BOOLEAN, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);

	if (IS_FLIST(get_feed_list()))
		f_list_free_all(get_feed_list());
	if (f_list_load_from_file(&flist, NULL) == OK) {
		flist = f_list_sort(flist);
		fill_list_store_from_flnode(list_store);
		flist_bak = f_list_clone(flist);
	} else {
		flist = NULL;
	}
	set_feed_list(flist);

	tree_view = GTK_TREE_VIEW(gtk_tree_view_new_with_model(GTK_TREE_MODEL(list_store)));

	renderer1 = gtk_cell_renderer_toggle_new();
	gtk_cell_renderer_toggle_set_radio(GTK_CELL_RENDERER_TOGGLE(renderer1), FALSE);
	column1 = gtk_tree_view_column_new_with_attributes(NULL, renderer1,
		"active", COLUMN_BOOLEAN_CHECKED, NULL);
	gtk_tree_view_append_column(tree_view, column1);

	if (get_params()->enable_feed_ordering == 'y') {
		renderer2 = gtk_cell_renderer_text_new();
#ifndef G_OS_WIN32
		/* Doesn't compile on win32 because of older GTK version */
		gtk_cell_renderer_set_alignment(renderer2, 1.0, 0.5);
#endif
		column2 = gtk_tree_view_column_new_with_attributes(NULL/*"Rank"*/, renderer2,
			"text", COLUMN_STRING_RANK, NULL);
		gtk_tree_view_append_column(tree_view, column2);
	}

	renderer3 = gtk_cell_renderer_text_new();
	column3 = gtk_tree_view_column_new_with_attributes("Feed Title", renderer3,
		"text", COLUMN_STRING_TITLE, NULL);
	gtk_tree_view_append_column(tree_view, column3);

	renderer4 = gtk_cell_renderer_text_new();
	column4 = gtk_tree_view_column_new_with_attributes("Feed URL", renderer4,
		"text", COLUMN_STRING_URL, NULL);
	gtk_tree_view_append_column(tree_view, column4);

	gtk_container_add(GTK_CONTAINER(sc_win), GTK_WIDGET(tree_view));

	selection = gtk_tree_view_get_selection(tree_view);
	gtk_tree_selection_set_mode(selection, GTK_SELECTION_SINGLE);
	g_signal_connect(G_OBJECT(selection), "changed", G_CALLBACK(tree_selection_changed), selection);
	g_signal_connect(G_OBJECT(tree_view), "row_activated", G_CALLBACK(double_click_on_tree_view), NULL);
	g_signal_connect(G_OBJECT(renderer1), "toggled", G_CALLBACK(checkbox_toggled), tree_view);

	hbox = gtk_hbox_new(FALSE, 0);
	if (get_params()->enable_feed_ordering != 'y') {
		tmp_label = gtk_label_new("<b>New Feed</b>  ->  Enter URL:");	/* Or <span foreground=\"red\"></span>, <big></big>, ... */
	} else {
		tmp_label = gtk_label_new("<b>New Feed</b>  ->  Enter rank and URL:");
	}
	gtk_label_set_use_markup(GTK_LABEL(tmp_label), TRUE);
	gtk_box_pack_start(GTK_BOX(hbox), tmp_label, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), gtk_label_new("  "), TRUE, FALSE, 0);
	if (get_params()->enable_feed_ordering == 'y') {
		rank_entry = gtk_entry_new();
		gtk_entry_set_max_length(GTK_ENTRY(rank_entry), FLIST_RANK_STR_LEN);
		gtk_entry_set_width_chars(GTK_ENTRY(rank_entry), FLIST_RANK_STR_LEN);
		gtk_box_pack_start(GTK_BOX(hbox), rank_entry, FALSE, FALSE, 0);
	}
	url_entry = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(url_entry), FILE_NAME_MAXLEN);
	if (get_params()->enable_feed_ordering != 'y') {
		gtk_entry_set_width_chars(GTK_ENTRY(url_entry), URL_ENTRY_LENGTH);
	} else {
		gtk_entry_set_width_chars(GTK_ENTRY(url_entry), URL_ENTRY_LENGTH - 10);		/* About ... */
	}
	gtk_box_pack_start(GTK_BOX(hbox), url_entry, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), clear_but, FALSE, FALSE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(hbox), 5);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), hbox, FALSE, FALSE, 0);

	g_signal_connect(G_OBJECT(url_entry), "activate", G_CALLBACK(enter_key_pressed_in_entry), NULL);
	if (get_params()->enable_feed_ordering == 'y')
		g_signal_connect(G_OBJECT(rank_entry), "activate", G_CALLBACK(enter_key_pressed_in_entry), NULL);
	g_signal_connect(G_OBJECT(clear_but), "clicked", G_CALLBACK(clear_entry), NULL);
	g_signal_connect(G_OBJECT(dialog), "key-press-event", G_CALLBACK(esc_key_pressed), NULL);
	g_signal_connect(G_OBJECT(dialog), "delete_event", G_CALLBACK(force_quit_dialog), NULL);

	gtk_widget_show_all(dialog);

	/* Backup last valid opened resource (if any) */
	str_n_cpy(resrc_id_bak, resrc->id, FILE_NAME_MAXLEN);
	if (IS_FLIST(flist) && resrc->id[0] != '\0') {
		if ((f_index = f_list_search(flist, resrc->id)) > -1) {
			highlight_and_go_to_row(tree_view, selection, f_index + 1);
		}else if ((f_index = f_list_search(flist, resrc->orig_url)) > -1) {	/* Try original URL in case of HTTP redirects */
			highlight_and_go_to_row(tree_view, selection, f_index + 1);
		}
	}
	gtk_widget_grab_focus(GTK_WIDGET(tree_view));

	while ((response = gtk_dialog_run(GTK_DIALOG(dialog))) != GTK_RESPONSE_CANCEL_CLOSE) {
		if (response == GTK_RESPONSE_SELECT_ALL) {
			if (question_win("Select all URLs ?", NO) == YES)
				select_all(tree_view);
		} else if (response == GTK_RESPONSE_UNSELECT_ALL) {
			if (question_win("Unselect all URLs ?", NO) == YES)
				unselect_all(tree_view);
		} else if (response == GTK_RESPONSE_CLEAR_RANKING) {
			if (question_win("Clear all URLs ranking ?", NO) == YES)
				clear_ranking(tree_view);
		} else if (response == GTK_RESPONSE_TOP) {
			highlight_and_go_to_row(tree_view, selection, 1);
		} else if (response == GTK_RESPONSE_CURRENT) {
			gtk_entry_set_text(GTK_ENTRY(url_entry), (const char *)resrc->id);
			if (IS_FLIST(flist)) {
				if ((f_index = f_list_search(flist, gtk_entry_get_text(GTK_ENTRY(url_entry)))) > -1)
					highlight_and_go_to_row(tree_view, selection, f_index + 1);
			}
		} else if (response == GTK_RESPONSE_HOME) {
			gtk_entry_set_text(GTK_ENTRY(url_entry), (const char *)home_feed);
			if (IS_FLIST(flist)) {
				if ((f_index = f_list_search(flist, gtk_entry_get_text(GTK_ENTRY(url_entry)))) > -1)
					highlight_and_go_to_row(tree_view, selection, f_index + 1);
			}
		} else if (response == GTK_RESPONSE_REMOVE) {
			if (IS_FLIST(flist) && f_index > -1) {
				if (gtk_tree_selection_get_selected(selection, &tree_model, &iter) &&
						question_win("Remove highlighted URL from list ?", NO) == YES) {
					gtk_list_store_remove(list_store, &iter);
					flist = f_list_remove(flist);
					if (f_list_count(flist) > 0) {
						set_feed_list(f_list_first(flist));
					} else {
						set_feed_list(NULL);
					}
					f_index = -1;
					gtk_entry_set_text(GTK_ENTRY(url_entry), "");
				}
			} else if (! IS_FLIST(flist)) {
				set_feed_list(flist_bak);
				fill_list_store_from_flnode(list_store);
				continue;
			} else if (f_index < 0) {
				warning(BLOCK, "You must select an URL first");
			}
		} else if (response == GTK_RESPONSE_ADD_UPD) {
			if (get_params()->enable_feed_ordering == 'y') {
				if (!(str_is_num(gtk_entry_get_text(GTK_ENTRY(rank_entry))) &&
						atoi(gtk_entry_get_text(GTK_ENTRY(rank_entry))) >= 0) &&
						!str_is_blank(gtk_entry_get_text(GTK_ENTRY(rank_entry)))) {
					warning(BLOCK, "You must enter a numerical (>= 0 integer) value");
					gtk_widget_grab_focus(GTK_WIDGET(rank_entry));
					continue;
				}
			}
			if (gtk_entry_get_text(GTK_ENTRY(url_entry))[0] != '\0') {
				cursor_position = gtk_editable_get_position(GTK_EDITABLE(url_entry));
				str_n_cpy(resrc->id, gtk_entry_get_text(GTK_ENTRY(url_entry)), FILE_NAME_MAXLEN);
				if (add_feed_to_flnode_and_list_store(resrc, list_store) == OK) {
					if ((f_index = f_list_search(flist, gtk_entry_get_text(GTK_ENTRY(url_entry)))) > -1)
						highlight_and_go_to_row(tree_view, selection, f_index + 1);
					/* Backup last valid opened resource (if any) */
					str_n_cpy(resrc_id_bak, resrc->id, FILE_NAME_MAXLEN);
				} else {
					continue;
				}
			} else {
				warning(BLOCK, "You must enter an URL first");
				gtk_widget_grab_focus(GTK_WIDGET(url_entry));
				continue;
			}
		} else if (response == GTK_RESPONSE_SINGLE) {
			if (f_index > -1 && gtk_entry_get_text(GTK_ENTRY(url_entry))[0] == '\0')
				tree_selection_changed(selection);
			if (gtk_entry_get_text(GTK_ENTRY(url_entry))[0] != '\0') {
				if (add_feed_to_flnode_and_list_store(resrc, list_store) == OK) {
					env->selection_mode = SINGLE;
					env->reload_rq = TRUE;
					break;
				} else {
					break;
				}
			} else {
				warning(BLOCK, "You must enter or select an URL first");
			}
		} else if (response == GTK_RESPONSE_SELECTION) {
			if (IS_FLIST(flist)) {
				for (flist2 = f_list_first(flist); IS_FLIST(flist2); flist2 = flist2->next) {
					if (flist2->selected)
						break;
				}
				if (IS_FLIST(flist2)) {
					if (flist2->selected) {
						env->selection_mode = MULTIPLE;
						env->reload_rq = TRUE;
						break;
					}
				}
				warning(BLOCK, "Selection is empty");
			} else {
				warning(BLOCK, "URL list is empty");
			}
		}
		gtk_widget_grab_focus(GTK_WIDGET(tree_view));
	}
	if (list_store != NULL)
		g_object_unref(list_store);
	if (response != GTK_RESPONSE_CANCEL_CLOSE) {
		if (IS_FLIST(flist)) {
			flist = f_list_first(flist);
			f_list_save_to_file(flist, NULL);
			set_feed_list(flist);
			build_feed_selection_from_feed_list();
			if (IS_FLIST(get_feed_selection()) && response == GTK_RESPONSE_SELECTION) {
				/*
				 * Multiple selection mode: start reading selection with highlighted feed
				 * - more exactly URL in entry (if any) / first one otherwise
				 */
				if ((f_index = f_list_search(get_feed_selection(), gtk_entry_get_text(GTK_ENTRY(url_entry)))) > -1) {
					set_feed_selection(f_list_nth(get_feed_selection(), f_index + 1));
				} else {
					first_feed();
				}
			}
			if (IS_FLIST(flist_bak))
				f_list_free_all(flist_bak);
		} else {
			/*info_win("", "Invalid new feed list", INFO_ERROR, TRUE);
			fprintf(STD_ERR, "Invalid new feed list\n");*/
			warning(BLOCK, "Invalid new feed list"),
			flist = flist_bak;
		}
	} else {
		/* If cancelled, we only restore feed list */
		if (IS_FLIST(flist))
			f_list_free_all(flist);
		flist = flist_bak;
		set_feed_list(flist);
	}
	gtk_widget_destroy(dialog);
	check_main_win_always_on_top();
	env->suspend_rq = FALSE;
}
