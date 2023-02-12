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
#include "tickr_list.h"

/* (In tickr_list.h)
typedef struct FeedListNode {
	char			*url;
	char			*title;
	zboolean		selected;
	char			rank[FLIST_RANK_STR_LEN + 1];
	struct FeedListNode	*prev;
	struct FeedListNode	*next;
	char			sig[FLIST_SIG_LEN + 1];
} FList;
*/

FList *f_list_new(const char *url, const char *title, zboolean selected, const char *rank)
{
	FList	*node = malloc2(sizeof(FList));
	/*int	url_len, title_len;*/

	node->url = malloc2(FLIST_URL_MAXLEN + 1);
	str_n_cpy(node->url, url, FLIST_URL_MAXLEN);

	node->title = malloc2(FLIST_TITLE_MAXLEN + 1);
	str_n_cpy(node->title, title, FLIST_TITLE_MAXLEN);

	/*
	 * If we want to use the following code (which saves memory use),
	 * we must implement funcs that reallocate memory when modifying
	 * data and we must never modify it directly.
	 */
	/*url_len = MIN(strlen(url), FLIST_URL_MAXLEN);
	node->url = malloc2(url_len + 1);
	str_n_cpy(node->url, url, url_len);

	title_len = MIN(strlen(title), FLIST_TITLE_MAXLEN);
	node->title = malloc2(title_len + 1);
	str_n_cpy(node->title, title, title_len);*/

	node->selected = selected;
	/* FLIST_RANK_STR_LEN chars long string with only digits/spaces,
	 * right justified. Value = atoi(rank), start at 1, 0 is interpreted as unset (blank)
	 * which is interpreted as last value (follows "999" for 3 chars). */
	if (atoi(rank) > 0)
		snprintf(node->rank, FLIST_RANK_STR_LEN + 1, FLIST_RANK_FORMAT, atoi(rank));
	else
		str_n_cpy(node->rank, BLANK_STR_16, FLIST_RANK_STR_LEN);

	node->prev = NULL;
	node->next = NULL;

	str_n_cpy(node->sig, FLIST_SIG, FLIST_SIG_LEN);
	return node;
}

/*
 * Free allocated memory.
 */
void f_list_free(FList *node)
{
	CHECK_IS_FLIST(node, __func__)

	memset(node->sig, 0, FLIST_SIG_LEN);
	free2(node->url);
	free2(node->title);
	free2(node);
}

/*
 * Free allocated memory.
 */
void f_list_free_all(FList *some_node)
{
	FList *node;

	CHECK_IS_FLIST(some_node, __func__)

	for (node = f_list_first(some_node); IS_FLIST(node); node = node->next)
		f_list_free(node);
}

FList *f_list_first(FList *node)
{
	CHECK_IS_FLIST(node, __func__)

	while (IS_FLIST(node->prev))
		node = node->prev;
	return node;
}

FList *f_list_last(FList *node)
{
	CHECK_IS_FLIST(node, __func__)

	while (IS_FLIST(node->next))
		node = node->next;
	return node;
}

/*
 * Return NULL if no previous node.
 */
FList *f_list_prev(FList *node)
{
	CHECK_IS_FLIST(node, __func__)

	if (IS_FLIST(node->prev))
		return node->prev;
	else
		return NULL;
}

/*
 * Return NULL if no next node.
 */
FList *f_list_next(FList *node)
{
	CHECK_IS_FLIST(node, __func__)

	if (IS_FLIST(node->next))
		return node->next;
	else
		return NULL;
}

/*
 * Return NULL if out of range.
 */
FList *f_list_nth(FList *some_node, int n)
{
	FList	*node;
	int	count, i = 1;

	CHECK_IS_FLIST(some_node, __func__)

	node = f_list_first(some_node);
	count = f_list_count(node);
	if (n > count) {
		INFO_ERR("%s(): %d is out of range\n", __func__, n)
		return NULL;
	} else {
		while (IS_FLIST(node->next) && i++ < n)
			node = node->next;
		if (IS_FLIST(node))
			return node;
		else
			return NULL;
	}
}

/*
 * Return index if found (starting at 0) / -1 otherwise.
 */
int f_list_index(FList *some_node)
{
	FList	*node;
	int	count, node_index = 0;

	if (IS_FLIST(some_node)) {
		node = f_list_first(some_node);
		count = f_list_count(node);
		for (; IS_FLIST(node) && node_index < count + 1; node = node->next) {
			if (node == some_node)
				return node_index;
			node_index++;
		}
	}
	return -1;
}

int f_list_count(FList *some_node)
{
	FList	*node;
	int	counter = 0;

	if (IS_FLIST(some_node))
		for (node = f_list_first(some_node); IS_FLIST(node); node = node->next)
			counter++;
	/*else
		INFO_ERR("%s(): List is empty\n", __func__)*/
	return counter;
}

/*
 * Create and add node at start of list.
 * 'some_node' may be any node in the list or NULL -> when creating list.
 */
FList *f_list_add_at_start(FList *some_node, const char *url, const char *title, zboolean selected,
	const char *rank)
{
	FList *instance;

	instance = f_list_new(url, title, selected, rank);	/* ->prev = ->next = NULL */
	if (IS_FLIST(some_node)) {
		some_node = f_list_first(some_node);
		instance->next = some_node;
		some_node->prev = instance;
	}
	return instance;
}

/*
 * Create and add node at end of list.
 * 'some_node' may be any node in the list or NULL -> when creating list.
 */
FList *f_list_add_at_end(FList *some_node, const char *url, const char *title, zboolean selected,
	const char *rank)
{
	FList *instance;

	instance = f_list_new(url, title, selected, rank);	/* ->prev = ->next = NULL */
	if (IS_FLIST(some_node)) {
		some_node = f_list_last(some_node);
		instance->prev = some_node;
		some_node->next = instance;
	}
	return instance;
}

/*
 * Create and insert node before 'some_node'.
 */
FList *f_list_insert_before(FList *some_node, const char *url, const char *title, zboolean selected,
	const char *rank)
{
	FList *instance;

	CHECK_IS_FLIST(some_node, __func__)

	instance = f_list_new(url, title, selected, rank);	/* ->prev = ->next = NULL */
	if (IS_FLIST(some_node->prev)) {
		instance->prev = some_node->prev;
		some_node->prev->next = instance;
	}
	instance->next = some_node;
	some_node->prev = instance;
	return instance;
}

/*
 * Create and insert node after 'some_node'.
 */
FList *f_list_insert_after(FList *some_node, const char *url, const char *title, zboolean selected,
	const char *rank)
{
	FList *instance;

	CHECK_IS_FLIST(some_node, __func__)

	instance = f_list_new(url, title, selected, rank);	/* ->prev = ->next = NULL */
	instance->prev = some_node;
	if (IS_FLIST(some_node->next)) {
		instance->next = some_node->next;
		some_node->next->prev = instance;
	}
	instance->prev = some_node;
	some_node->next = instance;
	return instance;
}

/*
 * Remove then return first node or NULL if empty.
 */
FList *f_list_remove(FList *node)
{
	FList *prev = NULL, *next = NULL, *node2 = NULL;

	CHECK_IS_FLIST(node, __func__)

	if (IS_FLIST(node->prev)) {
		prev = node->prev;
		prev->next = node->next;
		node2 = prev;
	}
	if (IS_FLIST(node->next)) {
		next = node->next;
		next->prev = node->prev;
		node2 = next;
	}
	f_list_free(node);
	if (IS_FLIST(node2))
		return f_list_first(node2);
	else
		return NULL;
}

void f_list_swap(FList *node1, FList *node2)
{
	FList tmp;

	CHECK_IS_FLIST(node1, __func__);
	CHECK_IS_FLIST(node2, __func__);

	(&tmp)->prev = node1->prev;
	(&tmp)->next = node1->next;
	node1->prev = node2->prev;
	node1->next = node2->next;
	node2->prev = (&tmp)->prev;
	node2->next = (&tmp)->next;
}

/*
 * Return url from ":" + 1, or url if not found
 *
 * url string may start with a sequence of other chars before the scheme
 * -> "blabla1http://blablablabla2"
 */
static char *get_url_beyond_scheme(const char *url)
{
	unsigned int i;

	for (i = 0; i < strlen(url); i++) {
		if (url[i] == ':')
			return (char *)url + i + 1;
	}
	return (char *)url;
}

/*
 * Sort by node->url (ignoring scheme), then remove duplicated and empty
 * entries, then return first node.
 *
 * If SORT_FLIST_BY_RANK is TRUE, sort 2 times, 2nd time by node->rank.
 */
FList *f_list_sort(FList *some_node)
{
	FList	*node1, *node2, *node_i[N_FLIST_MAX], *tmp;
	int	list_len, min, i, j;

	CHECK_IS_FLIST(some_node, __func__)

	node1 = f_list_first(some_node);
	for (i = 0; i < N_FLIST_MAX; i++) {
		if (IS_FLIST(node1)) {
			node_i[i] = node1;
			node1 = node1->next;
		} else
			break;
	}
	list_len = i;
	/* Selection sort */
	for (i = 0; i < list_len; i++) {
		min = i;
		for (j = i + 1; j < list_len; j++) {
			if (strcmp(get_url_beyond_scheme(node_i[min]->url),
					get_url_beyond_scheme(node_i[j]->url)) > 0)
				min = j;
		}
		tmp = node_i[i];
		node_i[i] = node_i[min];
		node_i[min] = tmp;
	}
	/* Remove duplicated entries */
	for (i = 0; i < list_len; i++) {
		for (j = i + 1; j < list_len; j++) {
			if (strcmp(node_i[i]->url, node_i[j]->url) == 0) {
				if (node_i[i]->selected || node_i[j]->selected) {
					node_i[i]->selected = TRUE;
					node_i[j]->selected = TRUE;
				}
				/* We want to use rank in the second entry (ie the last added one). */
				str_n_cpy(node_i[i]->rank, node_i[j]->rank, FLIST_RANK_STR_LEN);
				if (node_i[j]->title[0] == '\0')	/* If no title in the second entry, */
					node_i[j]->url[0] = '\0';	/* we keep the first one. */
				else
					node_i[i]->url[0] = '\0';
			}
		}
	}
	/* Remove empty entries */
	node2 = NULL;
	for (i = 0; i < list_len; i++)
		if (node_i[i]->url[0] != '\0')
			node2 = f_list_add_at_end(node2, node_i[i]->url, node_i[i]->title,
				node_i[i]->selected, node_i[i]->rank);
	node2 = f_list_first(node2);
	f_list_free_all(some_node);
	if (!SORT_FLIST_BY_RANK)
		return node2;
	else {
		/* If SORT_FLIST_BY_RANK is true, sort again, now by node->rank */
		for (i = 0; i < N_FLIST_MAX; i++) {
			if (IS_FLIST(node2)) {
				node_i[i] = node2;
				node2 = node2->next;
			} else
				break;
		}
		list_len = i;
		/* Selection sort */
		/* === rank starts at 1 / 0 = last rank === */
		for (i = 0; i < list_len; i++) {
			min = i;
			for (j = i + 1; j < list_len; j++)
				if (atoi((const char *)(node_i[min]->rank)) > atoi((const char *)(node_i[j]->rank)))
					min = j;
			tmp = node_i[i];
			node_i[i] = node_i[min];
			node_i[min] = tmp;
		}
		node1 = NULL;
		for (i = 0; i < list_len; i++)
			/* We want feeds sorted by rank > 0 first */
			if (atoi(node_i[i]->rank) > 0)
				node1 = f_list_add_at_end(node1, node_i[i]->url, node_i[i]->title,
					node_i[i]->selected, node_i[i]->rank);
		for (i = 0; i < list_len; i++)
			if (atoi(node_i[i]->rank) == 0)
				node1 = f_list_add_at_end(node1, node_i[i]->url, node_i[i]->title,
					node_i[i]->selected, node_i[i]->rank);
		node1 = f_list_first(node1);
		return node1;
	}
}

/*
 * Search by node->url and return index if found (starting at 0) / -1 otherwise.
 */
int f_list_search(FList *some_node, const char *str)
{
	FList	*node;
	int	i = 0;

	CHECK_IS_FLIST(some_node, __func__)

	for (node = f_list_first(some_node); IS_FLIST(node); node = node->next) {
		if (strcmp(str, node->url) == 0)
			return i;
		else
			i++;
	}
	return -1;
}

/*
 * Use provided file_name if not NULL / default file name (URL_LIST_FILE) otherwise.
 * Create new f_list.
 *
 * Entry format in URL list file:
 *	['*' (selected) or '-' (unselected) + "000" (3 chars rank) + URL [+ '>' + title] + '\n']
 *
 * Entry max length = FILE_NAME_MAXLEN
 * See also:	(UN)SELECTED_URL_CHAR/STR and TITLE_TAG_CHAR/STR in tickr.h
 * 		FLIST_RANK_FORMAT and FLIST_RANK_STR_LEN in tickr_list.h
 *
 * Return OK (and first node of list) or error code (LOAD_URL_LIST_NO_LIST, LOAD_URL_LIST_EMPTY_LIST,
 * LOAD_URL_LIST_ERROR.)
 */
int f_list_load_from_file(FList **new_node, const char *file_name)
{
	char		*list_file_name, default_list_file_name[FILE_NAME_MAXLEN + 1];
	FILE		*fp;
	char		*tmp, tmp2[FLIST_RANK_STR_LEN + 1];
	FList		*node;
	size_t		tmp_size = FILE_NAME_MAXLEN + 1;
	zboolean	selected, title_found;
	int		title_shift, i;

	if (file_name == NULL) {
		str_n_cpy(default_list_file_name, get_datafile_full_name_from_name(URL_LIST_FILE), FILE_NAME_MAXLEN);
		list_file_name = default_list_file_name;
		if ((fp = g_fopen(list_file_name, "rb")) == NULL)
				return LOAD_URL_LIST_NO_LIST;
	} else
		list_file_name = (char *)file_name;
	node = NULL;
	if ((fp = g_fopen(list_file_name, "rb")) != NULL) {
		tmp = malloc2(tmp_size * sizeof(char));
#ifndef G_OS_WIN32
		while (getline(&tmp, &tmp_size, fp) != EOF) {
#else
		while (fgets(tmp, tmp_size, fp) != NULL) {
#endif
			if (tmp[0] == SELECTED_URL_CHAR || tmp[0] == UNSELECTED_URL_CHAR) {
				selected = (tmp[0] == SELECTED_URL_CHAR) ? TRUE : FALSE;
				for (i = 0; i < FLIST_RANK_STR_LEN; i++)
					tmp2[i] = tmp[i + 1];
				tmp2[i] = '\0';

				title_shift = -1;
				title_found = FALSE;
				for (i = 0; i < FILE_NAME_MAXLEN; i++) {
					if (!title_found && tmp[i] == TITLE_TAG_CHAR) {
						title_shift = i + 1;
						tmp[i] = '\0';
						title_found = TRUE;
					} else if (tmp[i] == '\n') {
						tmp[i] = '\0';
						break;
					}
				}
				node = f_list_add_at_end(node, tmp + 1 + FLIST_RANK_STR_LEN,
					(title_shift != -1) ? tmp + title_shift : "", selected, tmp2);
			}
		}
		free2(tmp);
		fclose(fp);
		if (node != NULL) {
			*new_node = f_list_first(node);
			return OK;
		} else {
			warning(BLOCK, "URL list '%s' is empty", list_file_name);
			return LOAD_URL_LIST_EMPTY_LIST;
		}
	} else {
		warning(BLOCK, "Can't load URL list '%s': %s", list_file_name, strerror(errno));
		return LOAD_URL_LIST_ERROR;
	}
}

/*
 * Use provided file_name if not NULL / default file name (URL_LIST_FILE) otherwise.
 *
 * Entry format in URL list file:
 *	['*' (selected) or '-' (unselected) + "000" (3 chars rank) + URL [+ '>' + title] + '\n']
 *
 * Entry max length = FILE_NAME_MAXLEN
 * See also:	(UN)SELECTED_URL_CHAR/STR and TITLE_TAG_CHAR/STR in tickr.h
 * 		FLIST_RANK_FORMAT and FLIST_RANK_STR_LEN in tickr_list.h
 *
 * Return OK (and first node of list) or SAVE_URL_LIST_ERROR.
 */
int f_list_save_to_file(FList *node, const char *file_name)
{
	char	*list_file_name;
	FILE	*fp;

	if (file_name == NULL)
		list_file_name = l_str_new(get_datafile_full_name_from_name(URL_LIST_FILE));
	else
		list_file_name = l_str_new(file_name);
	if ((fp = g_fopen(list_file_name, "wb")) != NULL) {
		if (IS_FLIST(node)) {
			for(node = f_list_first(node); IS_FLIST(node); node = node->next) {
				/* TODO: We should use: "%c" + FLIST_RANK_FORMAT + "%s%s%s\n" */
				fprintf(fp, "%c%3d%s%s%s\n",
					node->selected ? SELECTED_URL_CHAR : UNSELECTED_URL_CHAR,
					atoi(node->rank),
					node->url,
					(node->title[0] != '\0') ? TITLE_TAG_STR : "",
					(node->title[0] != '\0') ? node->title : "");
			}
		}
		fclose(fp);
		l_str_free(list_file_name);
		return OK;
	} else {
		warning(BLOCK, "Can't save URL list '%s': %s", list_file_name, strerror(errno));
		l_str_free(list_file_name);
		return SAVE_URL_LIST_ERROR;
	}
}

FList *f_list_clone(FList *some_node)
{
	FList *new = NULL, *node;

	CHECK_IS_FLIST(some_node, __func__)

	for (node = f_list_first(some_node); IS_FLIST(node); node = node->next)
		new = f_list_add_at_end(new, node->url, node->title, node->selected, node->rank);
	new = f_list_first(new);
	return new;
}
