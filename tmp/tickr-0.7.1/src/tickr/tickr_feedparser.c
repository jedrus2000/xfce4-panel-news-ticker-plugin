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

/*
 * Here, 'RSS' is sometimes used as a synonym of 'feed' and sometimes
 * used by opposition to 'Atom' (this can be confusing).
 */

static int	depth;
static xmlNode	*item_or_entry_element;
static int	n;
static int	counter;

/*
 * In the algorithm, we now use strings instead of streams, so that we can
 * use l_str_insert_at_b() in case of reverse scrolling.  These strings,
 * as well as other variables, are set global in this src file because
 * of recursivity.
 */
static char	*xml_str, *xml_str_extra;

/*
 * Look for URL and, if valid, parse it then dump result into
 * <user_home_dir>/TICKR_DIR_NAME/XML_DUMP and -/XML_DUMP_EXTRA.
 * If URL isn't valid, only set error code and return.
 */
int get_feed(Resource *resrc, const Params *prm)
{
	char	feed_title[FEED_TITLE_MAXLEN + 1];
	char	feed_link[FILE_NAME_MAXLEN + 1];
	char	feed_ttl[32];
	char	file_name[FILE_NAME_MAXLEN + 1];
	char	url[FILE_NAME_MAXLEN + 1];
	int	suspend_rq_bak, error_code, i;
	char	*visual_str;

	suspend_rq_bak = get_ticker_env()->suspend_rq;
	get_ticker_env()->suspend_rq = TRUE;

	resrc->rss_ttl = prm->rss_refresh;

	/* 'xml dump' stuff, will open stream later. */
	str_n_cpy(resrc->xml_dump, get_datafile_full_name_from_name(XML_DUMP), FILE_NAME_MAXLEN);
	if (resrc->fp != NULL)
		fclose(resrc->fp);

	/* 'xml dump extra' stuff, will open stream later. */
	str_n_cpy(resrc->xml_dump_extra, get_datafile_full_name_from_name(XML_DUMP_EXTRA), FILE_NAME_MAXLEN);
	if (resrc->fp_extra != NULL)
		fclose(resrc->fp_extra);

	str_n_cpy(file_name, get_datafile_full_name_from_name(RESOURCE_DUMP), FILE_NAME_MAXLEN);

	/* We replace resrc->id with modified URL, file_name = downloaded resource. */
	if ((error_code = fetch_resource(resrc->id, (const char *)file_name, url)) == OK) {
		VERBOSE_INFO_OUT("Resource fetched: %s\n", (const char *)resrc->id);
	} else if (error_code == FEED_FORMAT_ERROR || error_code == RESOURCE_ENCODING_ERROR) {
		warning(M_S_MOD, "%s: %s", global_error_str(error_code), resrc->id);
		return error_code;
	} else if (error_code == CONNECT_TOO_MANY_ERRORS) {
		return error_code;
	} else {
		warning(M_S_MOD, "Can't fetch resource: %s", resrc->id);
		return error_code;
	}

	/* We use file_name instead of resrc->id. */
	if ((i = get_feed_info((int *)&resrc->format, file_name, feed_title, feed_link, feed_ttl)) != OK) {
		if (i == FEED_UNPARSABLE || i == FEED_EMPTY)
			warning(M_S_MOD, "%s: %s", global_error_str(i), resrc->id);
		else if (i == RSS_FORMAT_UNDETERMINED)
			warning(M_S_MOD, "Undetermined feed format: %s", resrc->id);
		else
			warning(M_S_MOD, "get_feed_info() error %d: %s: %s", i, global_error_str(i), resrc->id);
		return i;
	} else if (resrc->format == RSS_FORMAT_UNDETERMINED) {
		warning(M_S_MOD, "Undetermined feed format: %s", resrc->id);
		return RSS_FORMAT_UNDETERMINED;
	}

	/* We now use global strings instead of streams. */
	xml_str = l_str_new(NULL);
	xml_str_extra = l_str_new(NULL);

	str_n_cpy(resrc->feed_title, feed_title, FEED_TITLE_MAXLEN);
	if (prm->feed_title == 'y') {
		/* We remove any LINK_TAG_CHAR from str because it will be used in "link tag". */
		remove_char_from_str((char *)feed_title, LINK_TAG_CHAR);
		if (STANDARD_SCROLLING) {
			xml_str = l_str_cat(xml_str, (const char *)feed_title);
			xml_str = l_str_cat(xml_str, prm->feed_title_delimiter);
		} else {
			visual_str = log2vis_utf8((const char *)feed_title);
			xml_str = l_str_insert_at_b(xml_str, (const char *)visual_str);
			xml_str = l_str_insert_at_b(xml_str, prm->feed_title_delimiter);
			l_str_free(visual_str);
		}
	}
	if (feed_ttl[0] != '\0')
		resrc->rss_ttl = atoi(feed_ttl);

	/* Link and offset stuff reset */
	for (i = 0; i < NFEEDLINKANDOFFSETMAX; i++) {
		resrc->link_and_offset[i].offset_in_surface = 0;
		(resrc->link_and_offset[i].url)[0] = '\0';
	}

	/* We use file_name instead of resrc->id. */
	if ((error_code = parse_xml_file(resrc->format, file_name,
			resrc->link_and_offset, prm)) == FEED_NO_ITEM_OR_ENTRY_ELEMENT)
		warning(M_S_MOD, "No 'item' or 'entry' element found in: %s", resrc->id);
	if (error_code != OK) {
		l_str_free(xml_str);
		l_str_free(xml_str_extra);
		return error_code;
	}

	resrc->fp = open_new_datafile_with_name(XML_DUMP, "wb");
	fprintf(resrc->fp, "%s", xml_str);
	l_str_free(xml_str);
	/* We close then reopen stream in read-only mode. */
	fclose(resrc->fp);
	resrc->fp = open_new_datafile_with_name(XML_DUMP, "rb");

	resrc->fp_extra = open_new_datafile_with_name(XML_DUMP_EXTRA, "wb");
	fprintf(resrc->fp_extra, "%s", xml_str_extra);
	l_str_free(xml_str_extra);
	/* We close then reopen stream in read-only mode. */
	fclose(resrc->fp_extra);
	resrc->fp_extra = open_new_datafile_with_name(XML_DUMP_EXTRA, "rb");

	get_ticker_env()->suspend_rq = suspend_rq_bak;
	return OK;
}

/*
 * Must be UTF-8 encoded.
 */
int parse_xml_file(int format, const char* file_name, FeedLinkAndOffset *link_and_offset,
	const Params *prm)
{
	xmlDoc	*doc;
	xmlNode	*root_element;

	VERBOSE_INFO_OUT("Parsing XML file ... ");

	if ((doc = xmlParseFile(file_name)) == NULL) {
		warning(M_S_MOD, "XML parser error: %s", xmlGetLastError()->message);
		return FEED_UNPARSABLE;
	}
	if ((root_element = xmlDocGetRootElement(doc)) == NULL) {
		xmlFreeDoc(doc);
		warning(M_S_MOD, "Empty XML document: '%s'", file_name);
		return FEED_EMPTY;
	}

	depth = 0;
	item_or_entry_element = NULL;
	n = 1;
	counter = 0;

	if (format == RSS_2_0)
		get_rss20_selected_elements1(root_element, doc);
	else if (format == RSS_1_0)
		get_rss10_selected_elements1(root_element, doc);
	else if (format == RSS_ATOM)
		get_atom_selected_elements1(root_element, doc);
	else {
		xmlFreeDoc(doc);
		return FEED_FORMAT_ERROR;
	}
	if (item_or_entry_element != NULL)
		get_feed_selected_elements2(format, item_or_entry_element, doc, link_and_offset, prm);
	else {
		xmlFreeDoc(doc);
		return FEED_NO_ITEM_OR_ENTRY_ELEMENT;
	}

	xmlFreeDoc(doc);
	VERBOSE_INFO_OUT("Done\n");
	return OK;
}

void get_rss20_selected_elements1(xmlNode *some_element, xmlDoc *doc)
{
	xmlNode	*cur_node;

	for (cur_node = some_element; cur_node != NULL; cur_node = cur_node->next) {
		if (item_or_entry_element != NULL)
			return;
		/* RSS 2.0: We don't want extra namespaces. */
		if (cur_node->ns != NULL)
			continue;
		if (xmlStrcmp(cur_node->name, (const xmlChar *)"rss") == 0 && depth == 0)
			depth = 1;
		else if (xmlStrcmp(cur_node->name, (const xmlChar *)"channel") == 0 && depth == 1)
			depth = 2;
		else if (xmlStrcmp(cur_node->name, (const xmlChar *)"item") == 0 && depth == 2)
			depth = 3;

		if (depth == 3)
			item_or_entry_element = cur_node;
		else
			get_rss20_selected_elements1(cur_node->children, doc);
	}
}

void get_rss10_selected_elements1(xmlNode *some_element, xmlDoc *doc)
{
	xmlNode	*cur_node;

	for (cur_node = some_element; cur_node != NULL; cur_node = cur_node->next) {
		if (item_or_entry_element != NULL)
			return;
		if (xmlStrcmp(cur_node->name, (const xmlChar *)"RDF") == 0 && depth == 0)
			depth = 1;
		else if (xmlStrcmp(cur_node->name, (const xmlChar *)"item") == 0 && depth == 1)
			depth = 2;

		if (depth == 2)
			item_or_entry_element = cur_node;
		else
			get_rss10_selected_elements1(cur_node->children, doc);
	}
}

void get_atom_selected_elements1(xmlNode *some_element, xmlDoc *doc)
{
	xmlNode	*cur_node;

	for (cur_node = some_element; cur_node != NULL; cur_node = cur_node->next) {
		if (item_or_entry_element != NULL)
			return;
		if (xmlStrcmp(cur_node->name, (const xmlChar *)"feed") == 0)
			depth = 1;
		else if (xmlStrcmp(cur_node->name, (const xmlChar *)"entry") == 0 && depth == 1)
			depth = 2;

		if (depth == 2)
			item_or_entry_element = cur_node;
		else
			get_atom_selected_elements1(cur_node->children, doc);
	}
}

/*
 * For every link found, we insert LINK_TAG_CHAR + "00n" inside text with
 * n = link rank and we fill link_and_offset->url with URL. This is used
 * later in render_stream_to_surface().
 *
 * With reverse scrolling, we need to use Unicode control characters for
 * bidi text so that tags and delimiters are correctly placed.
 */
void get_feed_selected_elements2(int feed_format, xmlNode *some_element, xmlDoc *doc,
	FeedLinkAndOffset *link_and_offset, const Params *prm)
{
	xmlNode	*cur_node, *cur_node_bak;
	xmlChar	*str;
	xmlChar	item_or_entry[16];
	xmlChar description_or_summary[16];
	char	tmp_5[5];			/* 1 (ascii char) + 3 ("000" -> "999") + 1 (NULL terminator) */
	char	*visual_str;

	if (feed_format == RSS_2_0 || feed_format == RSS_1_0) {
		str_n_cpy((char *)item_or_entry, "item", 15);
		str_n_cpy((char *)description_or_summary, "description", 15);
	} else if (feed_format == RSS_ATOM) {
		str_n_cpy((char *)item_or_entry, "entry", 15);
		str_n_cpy((char *)description_or_summary, "summary", 15);
	} else {
		INFO_ERR("%s(): Undefined feed format\n", __func__)
		return;
	}
	for (cur_node = some_element; cur_node != NULL; cur_node = cur_node->next) {
		/* RSS 2.0: We don't want extra namespaces. */
		if (feed_format == RSS_2_0 && cur_node->ns != NULL)
			continue;
		if (xmlStrcmp(cur_node->name, (const xmlChar *)item_or_entry) == 0) {
			cur_node_bak = cur_node;
			cur_node = cur_node->children;
			for (; cur_node != NULL; cur_node = cur_node->next) {
				/* RSS 2.0: We don't want extra namespaces. */
				if (feed_format == RSS_2_0 && cur_node->ns != NULL)
					continue;
				if (xmlStrcmp(cur_node->name, (const xmlChar *)"title") == 0) {
					if ((str = xmlNodeListGetString(doc, cur_node->children, 1)) != NULL) {
						/* We remove any LINK_TAG_CHAR from str because it will be used in "link tag". */
						remove_char_from_str((char *)str, LINK_TAG_CHAR);
						remove_char_from_str((char *)str, ITEM_TITLE_TAG_CHAR);
						if (prm->item_title == 'y') {
							if (STANDARD_SCROLLING) {
								xml_str = l_str_cat(xml_str, (const char *)str);
								xml_str = l_str_cat(xml_str, prm->item_title_delimiter);
							} else {
								visual_str = log2vis_utf8((const char *)str);
								xml_str = l_str_insert_at_b(xml_str, (const char *)visual_str);
								xml_str = l_str_insert_at_b(xml_str, prm->item_title_delimiter);
								l_str_free(visual_str);

							}
						} else if (prm->item_description == 'y') {
							snprintf(tmp_5, 5, "%c%03d", ITEM_TITLE_TAG_CHAR, n);
							if (STANDARD_SCROLLING) {
								xml_str_extra = l_str_cat(xml_str_extra, tmp_5);
								xml_str_extra = l_str_cat(xml_str_extra, (const char *)str);
								xml_str_extra = l_str_cat(xml_str_extra, "\n");
							} else {
								xml_str_extra = l_str_insert_at_b(xml_str_extra, "\n");
								xml_str_extra = l_str_insert_at_b(xml_str_extra, (const char *)str);
								xml_str_extra = l_str_insert_at_b(xml_str_extra, tmp_5);
							}
						}
						xmlFree(str);
					}
				} else if (xmlStrcmp(cur_node->name, (const xmlChar *)description_or_summary) == 0) {
					if ((str = xmlNodeListGetString(doc, cur_node->children, 1)) != NULL) {
						/* We remove any LINK_TAG_CHAR from str because it will be used in "link tag". */
						remove_char_from_str((char *)str, LINK_TAG_CHAR);
						remove_char_from_str((char *)str, ITEM_DES_TAG_CHAR);
						if (prm->item_description == 'y') {
							if (STANDARD_SCROLLING) {
								xml_str = l_str_cat(xml_str, (const char *)str);
								xml_str = l_str_cat(xml_str, prm->item_description_delimiter);
							} else {
								visual_str = log2vis_utf8((const char *)str);
								xml_str = l_str_insert_at_b(xml_str, (const char *)visual_str);
								xml_str = l_str_insert_at_b(xml_str, prm->item_description_delimiter);
								l_str_free(visual_str);
							}
						} else if (prm->item_title == 'y') {
							snprintf(tmp_5, 5, "%c%03d", ITEM_DES_TAG_CHAR, n);
							if (STANDARD_SCROLLING) {
								xml_str_extra = l_str_cat(xml_str_extra, tmp_5);
								xml_str_extra = l_str_cat(xml_str_extra, (const char *)str);
								xml_str_extra = l_str_cat(xml_str_extra, "\n");
							} else {
								xml_str_extra = l_str_insert_at_b(xml_str_extra, "\n");
								xml_str_extra = l_str_insert_at_b(xml_str_extra, (const char *)str);
								xml_str_extra = l_str_insert_at_b(xml_str_extra, tmp_5);
							}
						}
						xmlFree(str);
					}
				}
			}
			cur_node = cur_node_bak;
			cur_node = cur_node->children;
			for (; cur_node != NULL; cur_node = cur_node->next) {
				/* RSS 2.0: We don't want extra namespaces. */
				if (feed_format == RSS_2_0 && cur_node->ns != NULL)
					continue;
				if (xmlStrcmp(cur_node->name, (const xmlChar *)"link") == 0) {
					/* Node content (RSS 2.0 / RSS 1.0) or node attribute (Atom) */
					if ((feed_format != RSS_ATOM &&	((str = xmlNodeListGetString(doc, cur_node->children, 1)) != NULL)) ||
							(str = xmlGetProp(cur_node, (const xmlChar *)"href")) != NULL) {
						if (n < NFEEDLINKANDOFFSETMAX + 1) {
							str_n_cpy((link_and_offset + n)->url, (const char *)str,
								FILE_NAME_MAXLEN);
							snprintf(tmp_5, 5, "%c%03d", LINK_TAG_CHAR, n++);
							if (STANDARD_SCROLLING)
								xml_str = l_str_cat(xml_str, tmp_5);
							else
								xml_str = l_str_insert_at_b(xml_str, tmp_5);
						}
						xmlFree(str);
					}
				}
			}
			cur_node = cur_node_bak;
			if (prm->n_items_per_feed != 0)
				if (++counter >= prm->n_items_per_feed)
					break;
		}
	}
}

/* Any useful ? Text is already supposed to be UTF-8 encoded. */
static const char *try_str_to_utf8(const char *str)
{
	static char	str2[1024];
	int		i;

	str_n_cpy(str2, str, 1023);
	for (i = strlen(str2); i > 0; i--) {
		str2[i - 1] = '\0';
		if (g_utf8_validate(str2, -1, NULL))
			break;
	}
	if (i == 0)
		str_n_cpy(str2, "(not UTF-8 encoded)", 1023);
	return (const char *)str2;
}

/*
 * Info is 1 int + 4 strings, feed_* can be NULL.
 */
int get_feed_info(int *format, const char *file_name, char *feed_title, char *feed_link, char *feed_ttl)
{
	xmlDoc	*doc;
	xmlNode	*root_element;

	*format = RSS_FORMAT_UNDETERMINED;
	if ((doc = xmlParseFile(file_name)) == NULL) {
		return FEED_UNPARSABLE;
	} else {
		if ((root_element = xmlDocGetRootElement(doc)) == NULL) {
			xmlFreeDoc(doc);
			return FEED_EMPTY;
		} else {
			if (xmlStrcmp(root_element->name, (const xmlChar *)"rss") == 0)
				*format = RSS_2_0;
			else if (xmlStrcmp(root_element->name, (const xmlChar *)"RDF") == 0)
				*format = RSS_1_0;
			else if (xmlStrcmp(root_element->name, (const xmlChar *)"feed") == 0)
				*format = RSS_ATOM;
			else {
				xmlFreeDoc(doc);
				return *format;
			}
			if (feed_title != NULL) {
				feed_title[0] = '\0';
				get_xml_first_element(root_element->children, doc,
					"title", feed_title, FEED_TITLE_MAXLEN);
			}
			if (feed_link != NULL) {
				feed_link[0] = '\0';
				get_xml_first_element(root_element->children, doc,
					"link", feed_link, FILE_NAME_MAXLEN);
			}
			if (feed_ttl != NULL) {
				feed_ttl[0] = '\0';
				get_xml_first_element(root_element->children, doc,
					"ttl", feed_ttl, 31);
			}
			xmlFreeDoc(doc);
 			if (!g_utf8_validate(feed_title, -1, NULL))
				str_n_cpy(feed_title, try_str_to_utf8(feed_title), 255);
			/* We remove any LINK_TAG_CHAR from str because it will be used in "link tag". */
			remove_char_from_str(feed_title, LINK_TAG_CHAR);
			return OK;
		}
	}
}

/*
 * string must be empty (string[0] = '\0') as function is recursive.
 */
void get_xml_first_element(xmlNode *some_element, xmlDoc *doc, char *name, char *string, int length)
{
	xmlNode	*cur_node;
	xmlChar	*str;

	if (string[0] != '\0')
		return;
	for (cur_node = some_element; cur_node != NULL; cur_node = cur_node->next) {
		if (xmlStrcmp(cur_node->name, (const xmlChar *)name) == 0) {
			if ((str = xmlNodeListGetString(doc, cur_node->children, 1)) != NULL) {
				str_n_cpy(string, (const char *)str, length);
				xmlFree(str);
			} else
				string[0] = '\0';
			break;
		}
		get_xml_first_element(cur_node->children, doc, name, string, length);
	}
}

/*
 * Reorder a UTF-8 string from logical to visual.
 * Original (logical) string is not modified.
 * Returned (visual) string must be freed (l_str_free) afterwards.
 */
char *log2vis_utf8(const char *str)
{
	char 		*utf8_str;
	char		*utf8_str_2;
	FriBidiChar	*unicode_str;		/* (uint32_t *) */
	FriBidiChar	*unicode_str_2;
	FriBidiParType	*pbase_dir;		/* (uint32_t *) */
	FriBidiLevel	*embedding_levels;	/* (signed char) */
	size_t		mem_size;
	int		utf8_str_len;
	char		*visual_str;

	utf8_str = l_str_new(str);
	utf8_str_2 = l_str_new(str);
	mem_size = (strlen(str) + 1) * 4;	/* Not sure what size should be */
	unicode_str = (FriBidiChar *)malloc2(mem_size);
	unicode_str_2 = (FriBidiChar *)malloc2(mem_size);
	pbase_dir =  (FriBidiChar *)malloc2(mem_size / 2 );		/* Same here */
	embedding_levels =  (signed char *)malloc2(mem_size / 2);	/* And here */

	utf8_str_len = fribidi_charset_to_unicode(FRIBIDI_CHAR_SET_UTF8, utf8_str, strlen(utf8_str), unicode_str);

	if (fribidi_log2vis(unicode_str, utf8_str_len, pbase_dir, unicode_str_2, NULL, NULL, embedding_levels) == 0) {
		INFO_ERR("%s(): fribidi_log2vis() returned 0\n", __func__)
		l_str_free(utf8_str_2);
		visual_str = l_str_new("Invalid_string");
	} else {
		fribidi_unicode_to_charset(FRIBIDI_CHAR_SET_UTF8, unicode_str_2, utf8_str_len, utf8_str_2);
		visual_str = utf8_str_2;
	}

	l_str_free(utf8_str);
	free2(unicode_str);
	free2(unicode_str_2);
	free2(pbase_dir);
	free2(embedding_levels);

	return visual_str;
}
