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
#include "tickr_html_entities.h"

/*
 * Build selection f_list from full f_list.
 *
 * Always use get/set_feed_list() and get/set_feed_selection() to access
 * FList *feed_list and FList *feed_selection defined in tickr_main.c.
 */
int build_feed_selection_from_feed_list()
{
	FList *new_selection = NULL, *node = get_feed_list();

	if (IS_FLIST(node)) {
		for (node = f_list_first(node); IS_FLIST(node); node = node->next)
			if (node->selected)
				new_selection = f_list_add_at_end(new_selection,\
					node->url, node->title, TRUE, node->rank);
		if (IS_FLIST(new_selection)) {
			new_selection = f_list_first(new_selection);
			set_feed_selection(new_selection);
			return OK;
		} else {
			set_feed_selection(NULL);
			return SELECTION_EMPTY;
		}
	} else {
		set_feed_selection(NULL);
		return SELECTION_ERROR;
	}
}

/*
 * feed_index_in_selection starts at 0.
 * These funcs do nothing in single selection mode.
 */
void current_feed()
{
	FList	*selection = get_feed_selection();
	int	f_index;

	if (M_S_MOD) {
		if ((f_index = f_list_index(selection)) == 0)
			set_feed_selection(f_list_last(selection));
		else if (f_index > 0)
			set_feed_selection(f_list_nth(selection, f_index));	/* (rank = f_index + 1) - 1 */
		get_ticker_env()->reload_rq = TRUE;
	}
}

void first_feed()
{
	if (M_S_MOD) {
		set_feed_selection(f_list_first(get_feed_selection()));
		get_ticker_env()->reload_rq = TRUE;
	}
}

void last_feed()
{
	if (M_S_MOD) {
		set_feed_selection(f_list_last(get_feed_selection()));
		get_ticker_env()->reload_rq = TRUE;
	}
}

zboolean previous_feed()
{
	FList	*selection = get_feed_selection();
	int	f_index;

	if (M_S_MOD) {
		if ((f_index = f_list_index(selection)) > 1 || f_index == 0) {
			if (f_index > 1)
				set_feed_selection(f_list_nth(selection, f_index - 1));
			else if (f_index == 0)
				set_feed_selection(f_list_nth(selection, f_list_count(selection) - 1));
			get_ticker_env()->reload_rq = TRUE;
			return TRUE;
		} else if (f_index == 1)
			info_win_no_block("This is already the first feed", INFO_WIN_WAIT_TIMEOUT);
	}
	return FALSE;
}

zboolean next_feed()
{
	FList	*selection = get_feed_selection();
	int	f_index;

	if (M_S_MOD) {
		if ((f_index = f_list_index(selection)) > 0) {
			get_ticker_env()->reload_rq = TRUE;
			return TRUE;
		} else
			info_win_no_block("This is already the last feed", INFO_WIN_WAIT_TIMEOUT);
	}
	return FALSE;
}

/*
 * - Single selection mode: (re)load resrc->id = rss feed if valid or text file if it exists.
 * - Multiple selection mode: load sequentially all selected feeds in feed list.
 *
 * In case of URL:
 * load_resource_from_selection() -> get_feed() -> fetch_resource() -> parse_xml_file() -> format_resource()
 *
 * - file:///... is considered an URL and will be 'xml-parsed'.
 * - /path/file_name will be processed as a non-xml text file.
 *
 * Return OK or error code.
 */
int load_resource_from_selection(Resource *resrc, FList *feed_selection)
{
	int rss_status, error_code;

	if (resrc->fp != NULL) {
		fclose(resrc->fp);
		resrc->fp = NULL;
	}
	if (resrc->fp_extra != NULL) {
		fclose(resrc->fp_extra);
		resrc->fp_extra = NULL;
	}
	resrc->xml_dump[0] = '\0';
	resrc->type = RESRC_TYPE_UNDETERMINED;
	resrc->format = RSS_FORMAT_UNDETERMINED;

	if (M_S_MOD) {
		if (IS_FLIST(feed_selection)) {
			str_n_cpy(resrc->id, feed_selection->url, FILE_NAME_MAXLEN);
			if (IS_FLIST(feed_selection->next))
				feed_selection = feed_selection->next;
			else if (IS_FLIST(f_list_first(feed_selection)))
				feed_selection = f_list_first(feed_selection);
			set_feed_selection(feed_selection);
		} else {
			warning(M_S_MOD, "No feed selected or no feed selection available\n"
				"Switching to single selection mode");
			get_ticker_env()->selection_mode = SINGLE;
			resrc->id[0] = '\0';
		}
	}

	if (resrc->id[0] != '\0') {
		if (strcmp(get_scheme_from_url(resrc->id), "http") == 0 ||
				strcmp(get_scheme_from_url(resrc->id), "https") == 0 ||
				strcmp(get_scheme_from_url(resrc->id), "file") == 0) {
			resrc->type = RESRC_URL;
			if ((rss_status = get_feed(resrc, get_params())) != OK) {
				/* Don't display some error messages supposedly already shown */
				if (		rss_status != OPEN_FILE_ERROR &&
						rss_status != FEED_FORMAT_ERROR &&
						rss_status != FEED_UNPARSABLE &&
						rss_status != FEED_EMPTY &&
						rss_status != FEED_NO_ITEM_OR_ENTRY_ELEMENT &&
						rss_status != TCP_SOCK_CANT_CONNECT &&
						rss_status != CONNECT_TOO_MANY_ERRORS &&
						rss_status != RESOURCE_ENCODING_ERROR &&
						rss_status != HTTP_INVALID_PORT_NUM &&
						rss_status != HTTP_BAD_REQUEST &&
						rss_status != HTTP_FORBIDDEN &&
						rss_status != HTTP_NOT_FOUND &&
						rss_status != HTTP_GONE &&
						rss_status != HTTP_INT_SERVER_ERROR &&
						rss_status != HTTP_NO_STATUS_CODE) {
					if (rss_status <= 10000)
						warning(M_S_MOD, "get_feed(%s, ...): %s", resrc->id,
							global_error_str(rss_status));
				}
				if (rss_status == CONNECT_TOO_MANY_ERRORS)
					error_code = CONNECT_TOO_MANY_ERRORS;
				else {
					resrc->id[0] ='\0';
					error_code = RESOURCE_INVALID;
				}
			} else
				error_code = OK;
		} else {
			resrc->type = RESRC_FILE;
			if ((resrc->fp = g_fopen(resrc->id, "rb")) == NULL) {
				warning(M_S_MOD, "Can't open '%s': %s", resrc->id, strerror(errno));
				resrc->id[0] ='\0';
				error_code = RESOURCE_NOT_FOUND;
			} else
				error_code = OK;
		}
	} else {
		resrc->type = RESRC_UNSPECIFIED;
		error_code = NO_RESOURCE_SPECIFIED;
	}

	if (error_code == OK) {
		if ((error_code = format_resource(resrc, XML_DUMP)) == OK)
			if (resrc->type == RESRC_URL)
				error_code = format_resource(resrc, XML_DUMP_EXTRA);
		if (error_code != OK) {
			warning(M_S_MOD, "%s: %s", global_error_str(FEED_FORMAT_ERROR), resrc->id);
			resrc->id[0] ='\0';
		}
	}
	return error_code;
}

/*
 * Do:
 * - Check UTF-8 encoding
 * - Strip html tags
 * - NOT ANYMORE - 'Translate' html entities
 * Then resrc->fp refers to a "formatted" file.
 *
 * Return OK or error code (FEED_FORMAT_ERROR, CREATE_FILE_ERROR, NOT_UTF8_ENCODED,
 * READ_FROM_STREAM_ERROR).
 *
 */
int format_resource(Resource *resrc, const char *pathname)
{
	FILE	*fp;
	char	*str;
	int	i;

	if (strcmp(pathname, XML_DUMP) == 0)
		fp = resrc->fp;
	else if (strcmp(pathname, XML_DUMP_EXTRA) == 0)
		fp = resrc->fp_extra;
	else {
		VERBOSE_INFO_ERR("%s(): %s\n", __func__, global_error_str(FEED_FORMAT_ERROR))	/* TODO: Which error here ? */
		return FEED_FORMAT_ERROR;
	}

	if ((i = get_stream_contents(fp, &str, TRUE)) == OK) {
		if (get_resource()->type == RESRC_URL)	/* ???? */
			str = format_resource_str(str);
		fclose(fp);
		if ((fp = open_new_datafile_with_name(pathname, "wb+")) != NULL) {
			fprintf(fp, "%s", str);
			fseek(fp, 0, SEEK_SET);
			i = OK;
		} else {
			i = CREATE_FILE_ERROR;
			VERBOSE_INFO_ERR("%s(): %s: '%s'\n", __func__, global_error_str(i), pathname)
		}
		l_str_free(str);
	} else {
		fclose(fp);
		VERBOSE_INFO_ERR("%s(): %s\n", __func__, global_error_str(i))
	}

	if (strcmp(pathname, XML_DUMP) == 0)
		resrc->fp = fp;
	else if (strcmp(pathname, XML_DUMP_EXTRA) == 0)
		resrc->fp_extra = fp;

	return i;
}

/*
 * Actual 'formatting' is done here.
 * str must have been created with l_str_new() (or malloc()).
 */
char *format_resource_str(char *str)
{
	char 		*str2, *s, *d;
	zboolean	is_inside, strip_tags;
	char		c;

	/*
	 * Strip html tags
	 */
	str2 = l_str_new(str);
	strip_tags = get_params()->strip_html_tags == 'y' ? TRUE : FALSE;
	is_inside = FALSE;

	for (s = str, d = str2; s < str + strlen(str); s++) {
		c = *s;
		if (strip_tags) {
			if (c == '<')
				is_inside = TRUE;
			else if (c == '>')
				is_inside = FALSE;
		} else
			is_inside = FALSE;
		if (!is_inside && !(strip_tags && c == '>'))
			*d++ = c;
	}

	*d = '\0';
	l_str_free(str);
	str = str2;
	remove_trailing_whitespaces_from_str(str);
	str = realloc2(str, strlen(str) + 1);

	return str;
}

/*
 * Convert from a specified encoding to UTF-8.
 * New str must be g_free'd after usage.
 */
char *convert_str_to_utf8(const char *str, const char *encoding)
{
	char	*str2;
	gsize	read, write;
	GError	*error = NULL;

	if ((str2 = g_convert(str, -1, "UTF-8", encoding, &read, &write, &error)) != NULL) {
		VERBOSE_INFO_ERR("%s(): Expected original encoding = '%s'\n", __func__, encoding)
		return str2;
	 } else {
		VERBOSE_INFO_ERR("%s(): %s\n", __func__, error->message)
		g_error_free(error);
		return NULL;
	}
}

/*
 * Read full contents of an opened stream and check UTF-8 encoding
 * (don't close stream afterward but rewind it).
 * str must be l_str_free'd after usage.
 * Return OK or NOT_UTF8_ENCODED or READ_FROM_STREAM_ERROR.
 */
int get_stream_contents(FILE *fp, char **str, zboolean check_utf8)
{
	size_t	str2_size = FGETS_STR_MAXLEN;
	char	*str2 = malloc2(str2_size * sizeof(char));
	int	i;

	fseek(fp, 0, SEEK_SET);
	*str = l_str_new(NULL);

#ifndef G_OS_WIN32
	while (getline(&str2, &str2_size, fp) != -1)
#else
	while (fgets(str2, str2_size, fp) != NULL)
#endif
		*str = l_str_cat(*str, str2);
	free2(str2);

	if (feof(fp) != 0) {
		if (check_utf8) {
			if (g_utf8_validate(*str, -1, NULL))
				i = OK;
			else /*{
				str2 = convert_str_to_utf8((const char*)*str, get_params()->alt_encoding);
				if (str2 != NULL) {
					l_str_free(*str);
					*str = l_str_new(str2);
					g_free(str2);
					i = OK;
				} else*/
					i = NOT_UTF8_ENCODED;
			/*}*/
		} else
			i = OK;
	} else
		i = READ_FROM_STREAM_ERROR;

	fseek(fp, 0, SEEK_SET);
	if (i != OK)
		VERBOSE_INFO_ERR("%s(): %s\n", __func__, global_error_str(i))
	return i;
}

/*
 * *** File names, paths and dirs stuff ***
 */
void set_tickr_icon_to_dialog(GtkWindow *dialog)
{
	gtk_window_set_icon_from_file(dialog, get_imagefile_full_name_from_name(TICKR_ICON), NULL);
}

/*
 * Open/create file in data dir from name.
 * Data dir = config files, not images files.
 */
FILE *open_new_datafile_with_name(const char *name, const char *mode_str)
{
	char	file_name[FILE_NAME_MAXLEN + 1];
	FILE	*fp;

	str_n_cpy(file_name, get_datafile_full_name_from_name(name), FILE_NAME_MAXLEN);
	if ((fp = g_fopen(file_name, mode_str)) == NULL) {
		if (mode_str[0] == 'w')
			big_error(CREATE_FILE_ERROR, "Can't create file '%s': %s", file_name, strerror(errno));
		else if (mode_str[0] == 'r')
			big_error(OPEN_FILE_ERROR, "Can't open file '%s': %s", file_name, strerror(errno));
	}
	return fp;
}

/*
 * Get full path and name for file in data dir from name.
 * Data dir = config files, not images files.
 */
char *get_datafile_full_name_from_name(const char *name)
{
	static char file_name[FILE_NAME_MAXLEN + 1];

	snprintf(file_name, FILE_NAME_MAXLEN + 1 - 2, "%s%c%s", get_datadir_full_path(), SEPARATOR_CHAR, name);
	if (get_instance_id() != 0)
		str_n_cat(file_name, itoa2(get_instance_id()), 2);
	return file_name;
}

/* Now, image files */
char *get_imagefile_full_name_from_name(const char *name)
{
	static char file_name[FILE_NAME_MAXLEN + 1];

#ifndef G_OS_WIN32
	snprintf(file_name, FILE_NAME_MAXLEN + 1, "%s%c%s", IMAGES_PATH, SEPARATOR_CHAR, name);
#else
	snprintf(file_name, FILE_NAME_MAXLEN + 1, "%s%c%s%c%s",
		get_progfiles_dir(), SEPARATOR_CHAR, IMAGES_PATH, SEPARATOR_CHAR, name);
#endif
	return file_name;
}

/* Data dir = config files, not images files */
char *get_datadir_full_path()
{
	static char full_path[TMPSTR_SIZE + 1];

#ifndef G_OS_WIN32
	snprintf(full_path, TMPSTR_SIZE + 1, "%s%c%s", usr_home_dir(), SEPARATOR_CHAR, TICKR_DIR_NAME);
#else
	snprintf(full_path, TMPSTR_SIZE + 1, "%s%c%s", get_appdata_dir_utf8(), SEPARATOR_CHAR, TICKR_DIR_NAME);
#endif
	return full_path;
}

char *usr_home_dir()
{
	static char str[TMPSTR_SIZE+ 1] = "";

#ifndef G_OS_WIN32
	return str_n_cpy(str, getpwuid(getuid())->pw_dir, TMPSTR_SIZE);
#endif
	/* For win32 version, no home dir - See win32 specific funcs in libetm. */
	return str;
}

/* Fix non-ascii (for instance cyrillic) user name in app data dir issue on win32. */
#ifdef G_OS_WIN32
const char *get_appdata_dir_utf8()
{
	static char	file_name[FILE_NAME_MAXLEN + 1];
	char		*file_name_utf8;

	if ((file_name_utf8 = g_utf16_to_utf8((const gunichar2 *)get_appdata_dir_w(), FILE_NAME_MAXLEN,
			NULL, NULL, NULL)) != NULL) {
		str_n_cpy(file_name, file_name_utf8, FILE_NAME_MAXLEN);
		g_free(file_name_utf8);
		return (const char *)file_name;
	} else
		return NULL;
}
#endif

const char *get_sample_url_list_full_name()
{
	static char file_name[FILE_NAME_MAXLEN + 1];

#ifndef G_OS_WIN32
	snprintf(file_name, FILE_NAME_MAXLEN + 1, "%s%c%s", INSTALL_PATH, SEPARATOR_CHAR, URL_LIST_FILE);
#else
	snprintf(file_name, FILE_NAME_MAXLEN + 1, "%s%c%s%c%s",
		get_progfiles_dir(), SEPARATOR_CHAR, TICKR_DIR_NAME, SEPARATOR_CHAR, URL_LIST_FILE);
#endif
	return (const char *)file_name;
}
