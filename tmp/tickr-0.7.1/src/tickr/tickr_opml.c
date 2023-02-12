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

#define OPML_FEED_RANK	APP_CMD "FeedRank"

char	*opml_str_global;
char	tmp_str[TMPSTR_SIZE + 1];

/* Prototypes for static functions */
static char	*pick_opml_file();
static int	parse_opml_file(const char *);
static void	get_opml_selected_element(xmlNode *, const char *, const char *);
static char	*create_opml_str_from_feed_list_node(FList *);
static int	save_str_as_opml_file(const char *);

/*
 * Importing
 */
static int *get_feed_counter()
{
	static int feed_counter;

	return &feed_counter;
}

static void zero_feed_counter()
{
	*get_feed_counter() = 0;
}

static void inc_feed_counter()
{
	*get_feed_counter() += 1;
}

int import_opml_file()
{
	char	*opml_file_name, *url_list_file_name;
	FList	*node;
	FILE	*fp;
	int	status =  OPML_ERROR;

	if ((opml_file_name = pick_opml_file())[0] != '\0') {
		url_list_file_name = l_str_new(get_datafile_full_name_from_name(URL_LIST_FILE));
		if ((fp = g_fopen(url_list_file_name, "rb")) != NULL) {
			fclose(fp);
			if (question_win("Imported URLs will be merged with currently saved ones. "
					"Continue ?", -1) == NO) {
				l_str_free(url_list_file_name);
				return status;
			}
		} else if ((fp = g_fopen(url_list_file_name, "wb")) != NULL)
			fclose(fp);
		else {
			warning(BLOCK, "Can't create URL list '%s': %s", url_list_file_name, strerror(errno));
			return status;
		}
		win_with_progress_bar(WIN_WITH_PROGRESS_BAR_OPEN,
			"    Importing (and updating) data from OPML file, please wait ...    ");
		VERBOSE_INFO_OUT(
			"Importing (and updating) data from OPML file (%s), please wait ...\n", opml_file_name);
		zero_feed_counter();
		get_ticker_env()->selection_mode = MULTIPLE;
		opml_str_global = l_str_new(NULL);
		if (parse_opml_file(opml_file_name) == OK)
			if ((fp = g_fopen(url_list_file_name, "ab")) != NULL) {
				fprintf(fp, "\n%s\n", opml_str_global);
				fclose(fp);
				if (f_list_load_from_file(&node, NULL) == OK) {
					node = f_list_sort(node);
					if (f_list_save_to_file(node, NULL) == OK) {
						if (IS_FLIST(get_feed_list()))
							f_list_free_all(get_feed_list());
						set_feed_list(node);
						status = OK;
					}
				}
			}
		l_str_free(url_list_file_name);
		l_str_free(opml_str_global);
		win_with_progress_bar(WIN_WITH_PROGRESS_BAR_CLOSE, NULL);
		if (status == OK) {
			snprintf(tmp_str, TMPSTR_SIZE + 1, "\nFeed list (%d URLs) has been imported\n",
				*get_feed_counter());
			info_win("", tmp_str, INFO, FALSE);
		}
	}
	return status;
}

static char *pick_opml_file()
{
	GtkWidget	*dialog;
	GtkFileFilter	*filter;
	char		*file_name;
	static char	file_name2[FILE_NAME_MAXLEN + 1];

	gtk_window_set_keep_above(GTK_WINDOW(get_ticker_env()->win), FALSE);

	dialog = gtk_file_chooser_dialog_new("Import Feed List (OPML)", GTK_WINDOW(get_ticker_env()->win),
			GTK_FILE_CHOOSER_ACTION_OPEN,
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL_CLOSE,
			GTK_STOCK_OK, GTK_RESPONSE_OK,
			NULL);

	set_tickr_icon_to_dialog(GTK_WINDOW(dialog));
	gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);

	g_signal_connect(G_OBJECT(dialog), "key-press-event", G_CALLBACK(esc_key_pressed), NULL);
	g_signal_connect(G_OBJECT(dialog), "delete_event", G_CALLBACK(force_quit_dialog), NULL);

	gtk_widget_show_all(dialog);

	filter = gtk_file_filter_new();
	gtk_file_filter_add_pattern(filter, "*.opml");
	gtk_file_filter_add_pattern(filter, "*.xml");
	gtk_file_chooser_set_filter(GTK_FILE_CHOOSER(dialog), filter);

	file_name2[0] = '\0';
	if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
		if ((file_name = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog))) != NULL) {
			str_n_cpy(file_name2, file_name, FILE_NAME_MAXLEN);
			g_free(file_name);
		}
	}
	gtk_widget_destroy(dialog);
	check_main_win_always_on_top();
	return file_name2;
}

/* TODO: Add option to import feed list without checking all URLs */
static int parse_opml_file(const char *file_name)
{
	xmlDoc		*doc = NULL;
	xmlNode		*cur_node = NULL, *cur_node_start = NULL;
	int		body_element = NO, outline_element = NO;

	VERBOSE_INFO_OUT("Parsing OPML file ...\n");
	if ((doc = xmlParseFile(file_name)) == NULL) {
		warning(BLOCK, "Can't parse XML file: %s", xmlGetLastError()->message);
		return XML_UNPARSABLE;
	} else if ((cur_node = xmlDocGetRootElement(doc)) == NULL) {
		warning(BLOCK, "Empty XML document: '%s'", file_name);
		xmlFreeDoc(doc);
		return XML_EMPTY;
	} else if (xmlStrcmp(cur_node->name, (const xmlChar *)"opml") != 0) {
		warning(BLOCK, "Not an OPML document: '%s'", file_name);
		xmlFreeDoc(doc);
		return OPML_ERROR;
	}
	cur_node_start = cur_node;
	for (cur_node = cur_node->children; cur_node != NULL; cur_node = cur_node->next) {
		if (xmlStrcmp(cur_node->name, (const xmlChar *)"body") == 0) {
			body_element = YES;
			break;
		}
	}
	if (body_element == YES) {
		for (cur_node = cur_node->children; cur_node != NULL; cur_node = cur_node->next) {
			if (xmlStrcmp(cur_node->name, (const xmlChar *)"outline") == 0) {
				outline_element = YES;
				break;
			}
		}
	}
	if (body_element == YES && outline_element == YES) {
		get_opml_selected_element(cur_node_start, "outline", "xmlUrl");
		VERBOSE_INFO_OUT("Done\n");
		xmlFreeDoc(doc);
		return OK;
	} else {
		warning(BLOCK, "Couldn't find all required OPML elements in: '%s'",
			file_name);
		xmlFreeDoc(doc);
		return OPML_ERROR;
	}
}

/*
 * Entry format in URL list file:
 *	['*' (selected) or '-' (unselected) + "000" (3 chars rank) + URL [+ '>' + title] + '\n']
 *
 * Entry max length = FILE_NAME_MAXLEN
 * See also:	(UN)SELECTED_URL_CHAR/STR and TITLE_TAG_CHAR/STR in tickr.h
 * 		FLIST_RANK_FORMAT and FLIST_RANK_STR_LEN in tickr_list.h
 */
static void get_opml_selected_element(xmlNode *some_node, const char *selected_element, const char *selected_attribute)
{
	xmlNode	*cur_node;
	xmlChar	*str1, *str2, *str3;
	char	feed_url[FILE_NAME_MAXLEN + 1];
	char	feed_title[FEED_TITLE_MAXLEN + 1];
	char	feed_rank[FLIST_RANK_STR_LEN + 1];
	char	*feed_str;

	for (cur_node = some_node; cur_node != NULL; cur_node = cur_node->next) {
		if (xmlStrcmp(cur_node->name, (const xmlChar *)selected_element) == 0) {
			/*
			 * Need to get attribute name and value,
			 * then get value for name = "xmlUrl",
			 * check if URL is reachable and valid,
			 * also eventually get value for (NOT STANDARD) name = OPML_FEED_RANK.
			 */
			if ((str1 = xmlGetProp(cur_node, (xmlChar *)selected_attribute)) != NULL) {
				if ((str2 = xmlGetProp(cur_node, (xmlChar *)"xmlUrl")) != NULL) {
					/* First, we add one UNSELECTED_URL_CHAR */
					feed_str = l_str_new(UNSELECTED_URL_STR);
					/* then FLIST_RANK_STR_LEN chars */
					if ((str3 = xmlGetProp(cur_node, (xmlChar *)OPML_FEED_RANK)) != NULL) {
						snprintf(feed_rank, FLIST_RANK_STR_LEN + 1, FLIST_RANK_FORMAT, atoi((char *)str3));
						xmlFree(str3);
					} else
						str_n_cpy(feed_rank, BLANK_STR_16, FLIST_RANK_STR_LEN);
					feed_str = l_str_cat(feed_str, feed_rank);
					str_n_cpy(feed_url, (const char *)str2, FILE_NAME_MAXLEN);
					/* We check the feed URL */
					if (check_and_update_feed_url(feed_url, feed_title) == OK) {
						/* We add feed URL */
						feed_str = l_str_cat(feed_str, (const char*)feed_url);
						if (feed_title[0] != '\0') {
							/* and feed title */
							feed_str = l_str_cat(feed_str, TITLE_TAG_STR);
							feed_str = l_str_cat(feed_str, feed_title);
						}
					} else {
						feed_str = l_str_cat(feed_str, (const char*)feed_url);
						feed_str = l_str_cat(feed_str, TITLE_TAG_STR);
						feed_str = l_str_cat(feed_str, "-");		/* Should find sth fancier */
						VERBOSE_INFO_OUT("URL is unreachable or invalid (or whatever)\n")
					}
					/* We're done for that */
					feed_str = l_str_cat(feed_str, "\n");
					opml_str_global = l_str_cat(opml_str_global, feed_str);
					inc_feed_counter();
					snprintf(tmp_str, TMPSTR_SIZE + 1, "%d feed%s imported so far, please wait ...",
						*get_feed_counter(), *get_feed_counter() <= 1 ? "" : "s");
					/*snprintf(tmp_str, TMPSTR_SIZE + 1, "%d feed%s imported so far, please wait ...\n%s",
						*get_feed_counter(), *get_feed_counter() <= 1 ? "" : "s", feed_url);*/
					win_with_progress_bar(WIN_WITH_PROGRESS_BAR_PULSE, tmp_str);
					l_str_free(feed_str);
					xmlFree(str2);
				}
				xmlFree(str1);
			}
		}
		get_opml_selected_element(cur_node->children, selected_element, selected_attribute);
	}
}

/*
 * Exporting
 */
void export_opml_file()
{
	FList	*node;
	char	*str;

	if (f_list_load_from_file(&node, NULL) == OK) {
		str = create_opml_str_from_feed_list_node(node);
		save_str_as_opml_file(str);
		l_str_free(str);
		f_list_free_all(node);
	}
}

/* Must l_str_free() opml_str afterwards */
static char *create_opml_str_from_feed_list_node(FList *node)
{
#define OPML_TITLE	APP_NAME " Feed List"
#define STR_SIZE	(4 * 1024)

	char 	*opml_header =
		"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
		"<opml version=\"2.0\">\n"
		"  <head>\n"
		"    <title>%s</title>\n"
		"  </head>\n"
		"  <body>\n";
	char 	*opml_item =
		"    <outline text=\"%s\" xmlUrl=\"%s\" " OPML_FEED_RANK "=\"%s\"/>\n";
	char 	*opml_footer =
		"  </body>\n"
		"</opml>\n";
	char	*opml_str, tmp[STR_SIZE];
	char	*esc_str1, *esc_str2;
	time_t	time2;
	char	time_str[64];

	snprintf(tmp, STR_SIZE, opml_header, OPML_TITLE);
	opml_str = l_str_new(tmp);
	for (; node != NULL; node = node->next) {
		if (!g_utf8_validate(node->url, -1, NULL)) {
			warning(BLOCK, "%s: '%s' - Skipped", global_error_str(NOT_UTF8_ENCODED), node->url);
			continue;
		}
		esc_str1 = g_uri_escape_string(node->title, " AÄääàçéèêÖöôÜüùà'()[]-_|{}/.,;:?!%€£$*+=", TRUE);
		esc_str2 = g_uri_escape_string(node->url, G_URI_RESERVED_CHARS_GENERIC_DELIMITERS, TRUE);
		snprintf(tmp, STR_SIZE, opml_item, esc_str1, esc_str2, node->rank);
		g_free(esc_str1);
		g_free(esc_str2);
		opml_str = l_str_cat(opml_str, tmp);
	}
	l_str_cat(opml_str, opml_footer);
	/* Add time of export (as a comment) */
	time2 = time(NULL);
	l_str_cat(opml_str, "<!-- Exported on ");
	strftime(time_str, 64, "%c", localtime(&time2));
	l_str_cat(opml_str, time_str);
	l_str_cat(opml_str, " -->\n");
	return opml_str;
}

static int save_str_as_opml_file(const char *str)
{
	GtkWidget	*dialog;
	char		*file_name = NULL;
	FILE		*fp;
	int		error_code = CREATE_FILE_ERROR;
	char		tmp[TMPSTR_SIZE + 1];

	gtk_window_set_keep_above(GTK_WINDOW(get_ticker_env()->win), FALSE);

	dialog = gtk_file_chooser_dialog_new("Export Feed List (OPML)", GTK_WINDOW(get_ticker_env()->win),
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
	gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), APP_CMD "-feed-list.opml");
	gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog), TRUE);

	if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
		if ((file_name = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog))) != NULL) {
			if ((fp = g_fopen(file_name, "wb")) != NULL) {
				fprintf(fp, "%s",  str);
				fclose(fp);
				error_code = OK;
			} else
				warning(BLOCK, "Can't save OPML file '%s': %s", file_name,
					strerror(errno));
		}
	}
	gtk_widget_destroy(dialog);
	if (error_code == OK) {
		snprintf(tmp, TMPSTR_SIZE + 1,
			"\nFeed list has been exported to OPML file: '%s'\n", file_name);
		info_win("", tmp, INFO, FALSE);
	}
	if (file_name != NULL)
			g_free(file_name);
	check_main_win_always_on_top();
	return error_code;
}
