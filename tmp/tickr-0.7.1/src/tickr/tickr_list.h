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

#ifndef INC_TICKR_LIST_H
#define INC_TICKR_LIST_H

#define FLIST_URL_MAXLEN	FILE_NAME_MAXLEN
#define FLIST_TITLE_MAXLEN	FEED_TITLE_MAXLEN
#define N_FLIST_MAX		(32 * 1024)		/* To prevent infinite loops - adjust as necessary */
#define FLIST_SIG		"fl_node"		/* Make sure FLIST_SIG_LEN = strlen(FLIST_SIG) */
#define FLIST_SIG_LEN		7
#define FLIST_RANK_FORMAT	"%3d"			/* Make sure format and length match */
#define FLIST_RANK_STR_LEN	3
#define BLANK_STR_16		"                "	/* Make sure FLIST_RANK_STR_LEN < (BLANK_STR_)16 */

#define SORT_FLIST_BY_RANK\
	(get_params()->enable_feed_ordering == 'y')

#define	IS_FLIST(node)\
	(node != NULL && strcmp(node->sig, FLIST_SIG) == 0)

#define CHECK_IS_FLIST(node, func)\
	if (!IS_FLIST(node))\
		big_error(FLIST_ERROR, "%s(): Invalid node in list", func);

typedef struct FeedListNode {
	char			*url;
	char			*title;
	zboolean		selected;
	char			rank[FLIST_RANK_STR_LEN + 1];
	struct FeedListNode	*prev;
	struct FeedListNode	*next;
	char			sig[FLIST_SIG_LEN + 1];
} FList;

FList		*f_list_new(const char *, const char *, zboolean, const char *);
void		f_list_free(FList *);
void		f_list_free_all(FList *);
FList		*f_list_first(FList *);
FList		*f_list_last(FList *);
FList		*f_list_prev(FList *);
FList		*f_list_next(FList *);
FList		*f_list_nth(FList *, int);
int		f_list_index(FList *);
int		f_list_count(FList *);
FList		*f_list_add_at_start(FList *, const char *, const char *, zboolean, const char *);
FList		*f_list_add_at_end(FList *, const char *, const char *, zboolean, const char *);
FList		*f_list_insert_before(FList *, const char *, const char *, zboolean, const char *);
FList		*f_list_insert_after(FList *, const char *, const char *, zboolean, const char *);
FList		*f_list_remove(FList *);
void		f_list_swap(FList *, FList *);
FList		*f_list_sort(FList *);
int		f_list_search(FList *, const char *);
int		f_list_load_from_file(FList **, const char *);
int		f_list_save_to_file(FList *, const char *);
FList		*f_list_clone(FList *);
#endif /* INC_TICKR_LIST_H */
