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

#define NO_RESOURCE_STR		"No resource"
#define WELCOME_STR		"Welcome to " APP_NAME " v" APP_V_NUM " !    "\
				"To open the main menu, right-click here ..."

/*
 * 'Private' struct's used to make an open stream 'readable' and 'renderable'
 * in both directions (L to R and R to L).
 */
typedef struct {
	char		*str;
	zboolean	is_valid;			/* Means string is readable (filled) and free-able */
	int		width;				/* Used to compute links' offsets */
} RenderString;

typedef struct {
	RenderString	render_str[N_RENDER_STR_MAX];	/* Array of strings from stream */
	int		n;				/* Number of strings (<= N_RENDER_STR_MAX) */
	int		i;				/* Current string index, -1 means empty array or error */
	int		links_extra_offset;		/* For the current surface */
} RenderStringArray;

/* 'Private' static array */
static RenderStringArray	s_a0, *s_a = &s_a0;

/* Prototypes for static funcs */
static cairo_surface_t	*render_string_and_layout_to_surface(const char *, PangoLayout *, const Params *, int *);
static zboolean		fill_in_string_array_from_stream(FILE *, FeedLinkAndOffset *, PangoLayout *, const Params *, int *);
static int		parse_utf8_str_until_layout_width(char *, PangoLayout *, int);
static char		*get_current_string_in_array(int *);
static char		*fill_in_offset_array_update_big_string(char *, FeedLinkAndOffset *, PangoLayout *);
static char		*get_big_string_from_stream(FILE *, PangoLayout *, const Params *, int *);
static char		*no_resource_str(PangoLayout *);
/*static char		*welcome_str(PangoLayout *);*/
static char		*create_empty_line_from_layout(PangoLayout *);
static char		*create_tab_line();
static int		get_layout_width(PangoLayout *, const char *);
static int		get_layout_height(PangoLayout *, const char *);

/* Init on first call, do nothing on next calls. */
void init_render_string_array()
{
	int		i;
	static int	j = -1;

	if (j == -1) {
		for (i = 0; i < N_RENDER_STR_MAX; i++) {
			s_a->render_str[i].is_valid = FALSE;
			s_a->render_str[i].width = 0;
		}
		s_a->n = 0;
		s_a->i = -1;
		s_a->links_extra_offset = 0;
		j++;
	}
}

void free_all_render_strings_in_array()
{
	int i;

	/* See: init_render_string_array() */
	for (i = 0; i < N_RENDER_STR_MAX; i++) {
		if (s_a->render_str[i].is_valid) {
			l_str_free(s_a->render_str[i].str);
			s_a->render_str[i].is_valid = FALSE;
		}
		s_a->render_str[i].width = 0;
	}
	/* Also reset all int vars */
	s_a->n = 0;
	s_a->i = -1;
	s_a->links_extra_offset = 0;
}

/* For the current surface */
int get_links_extra_offset()
{
	return s_a->links_extra_offset;
}

/*
 * Render a stream (opened text file) into images (cairo surfaces) of single
 * lines of text. Any surface width must be <= (XPIXMAP_MAXWIDTH = 32 K - 1)
 * because, for some reason, X pixmap width and height are signed 16-bit
 * integers (not sure if this relates to X or Pixman, but one can see this
 * very limit set in cairo-image-surface.c in cairo sources.) Which is a
 * PITA because implementation would be so much more staighforward otherwise.
 *
 * So one stream must be splitted into an array of strings on first call
 * and each string is rendered on each call, from first to last if scrolling
 * dir is R to L, or from last to first if scrolling dir is L to R. When
 * all strings have been rendered, get_ticker_env()->feed_fully_rendered
 * is set to TRUE. This flag can also be used to 'reset' this function
 * (as well as get_ticker_env()->reload_rq).
 *
 * Return newly created surface, or NULL if error.
 * If returned surface is NULL, then error_code is set accordingly and should
 * be checked to know which error occurred.
 * (Could be: OK, RENDER_NO_RESOURCE, RENDER_CAIRO_IMAGE_SURFACE_TOO_WIDE,
 * RENDER_CREATE_CAIRO_IMAGE_SURFACE_ERROR, RENDER_PANGO_LAYOUT_WIDTH_OVERFLOW,
 * RENDER_FILL_IN_STR_ARRAY_ERROR, RENDER_PROCESS_STR_ARRAY_ERROR,
 * READ_FROM_STREAM_ERROR.)
 *
 * Also set get_resource()->link_and_offset.offset_in_surface values.
 */
cairo_surface_t *render_stream_to_surface(FILE *fp, FeedLinkAndOffset *link_and_offset, const Params *prm, int *error_code)
{
	cairo_surface_t		*c_surf = NULL;
	char			*cur_str, *str;
	PangoContext		*context;
	PangoLayout		*p_layout;
	PangoFontDescription	*f_des;
	/*zboolean		welcome = FALSE;*/

	if (get_ticker_env()->reload_rq)
		get_ticker_env()->feed_fully_rendered = TRUE;
	*error_code = OK;
	/*
	 * Create layout
	 */
	context = gtk_widget_get_pango_context(get_ticker_env()->win);
	p_layout = pango_layout_new(context);
	pango_layout_set_attributes(p_layout, NULL);
	pango_layout_set_single_paragraph_mode(p_layout, TRUE);
	f_des = pango_font_description_from_string((const char *)prm->font_name_size);
	pango_layout_set_font_description(p_layout, f_des);
	pango_font_description_free(f_des);
	/*
	 * Get one string (from text string array from stream)
	 */
	init_render_string_array();
	if (fp != NULL) {
		if (get_ticker_env()->feed_fully_rendered) {
			free_all_render_strings_in_array();
			if (!fill_in_string_array_from_stream(fp, link_and_offset, p_layout, prm, error_code)) {
				warning(M_S_MOD, "%s(): fill_in_string_array_from_stream(): %s", __func__,
					global_error_str(*error_code));
				/* We consider we don't have any valid resource */
				*error_code = RENDER_NO_RESOURCE;
			}
		} else
			*error_code = OK;

		if (*error_code == OK) {
			cur_str = get_current_string_in_array(error_code);
			if (cur_str != NULL)
				str = l_str_new(cur_str);
			else {
				warning(M_S_MOD, "%s(): get_current_string_in_array(): %s",
					__func__, global_error_str(*error_code));
				/* Again, we consider we don't have any valid resource */
				*error_code = RENDER_NO_RESOURCE;
			}
		}
	}

	if (fp == NULL || *error_code != OK) {
		get_resource()->id[0] = '\0';
		free_all_render_strings_in_array();
		/*if (welcome)
			str = welcome_str(p_layout);
		else*/
			str = no_resource_str(p_layout);
		s_a->render_str[0].str = str;
		s_a->n = 1;
		s_a->i = 0;
		if (M_S_MOD)
			get_ticker_env()->feed_fully_rendered = TRUE;
	}

	c_surf = render_string_and_layout_to_surface(str, p_layout, prm, error_code);
	/*
	 * If out of limits, set flag so that, in multiple selection mode,
	 * to load next feed on next call.
	 */
	if (STANDARD_SCROLLING) {
		if (s_a->i > s_a->n - 1)
			get_ticker_env()->feed_fully_rendered = TRUE;
	} else {
		if (s_a->i < 0)
			get_ticker_env()->feed_fully_rendered = TRUE;
	}

	l_str_free(str);
	if (p_layout != NULL)
		g_object_unref(p_layout);
	return c_surf;
}

/*
 * Create a cairo surface (image) of a string and layout.
 * 'Tail surface' is used in sequential calls to this function, ie the
 * 2nd call will get the tail part of the first call surface, which will
 * be the beginning part of the new surface.
 *
 * By setting get_ticker_env()->feed_fully_rendered to TRUE, the function
 * is kind of reset, and 'tail surface' is destroyed
 *
 * Return surface or NULL if error (and set error_code).
 */
static cairo_surface_t *render_string_and_layout_to_surface(const char *str2, PangoLayout *p_layout, const Params *prm, int *error_code)
{
	int			layout_width, layout_height, shift_x;
	char			*str, *empty_line;
	cairo_surface_t		*c_surf = NULL;
	static cairo_surface_t	*c_tail_surf = NULL;
	cairo_t			*c_context;
	cairo_t			*c_tail_context;
	cairo_pattern_t		*c_pattern;
	cairo_status_t		c_status;
	float			shadow_k;
	zboolean		errors = FALSE;
#ifdef G_OS_WIN32
	WCHAR			u202d[6];
#endif

	if (REVERSE_SCROLLING) {
#ifndef G_OS_WIN32
		str = l_str_new("\u202d");	/* Unicode LTR override - NEED THIS TO GET THINGS RIGHT (so to speak) */
#else
		g_unichar_to_utf8(L'\u202d', (char *)u202d);
		str = l_str_new((char *)u202d);
#endif
		str = l_str_cat(str, str2);
	} else
		str = l_str_new(str2);
	/*
	 * 'Tail' surface stuff
	 */
	if (get_ticker_env()->feed_fully_rendered) {
		if (c_tail_surf != NULL)
			cairo_surface_destroy(c_tail_surf);
		c_tail_surf = NULL;
		/* Reset 'global' variable get_ticker_env()->feed_fully_rendered */
		get_ticker_env()->feed_fully_rendered = FALSE;
	}
	if (c_tail_surf == NULL) {
		empty_line = create_empty_line_from_layout(p_layout);
		if (STANDARD_SCROLLING)
			str = l_str_insert_at_b(str, empty_line);
		else
			str = l_str_cat(str, empty_line);
		l_str_free(empty_line);
		shift_x = 0;
	} else
		shift_x = get_ticker_env()->drwa_width;
	/*
	 * Fill layout
	 */
	pango_layout_set_text(p_layout, str, -1);
	pango_layout_context_changed(p_layout);
	pango_layout_get_pixel_size(p_layout, &layout_width, &layout_height);
	if (layout_width + shift_x > XPIXMAP_MAXWIDTH) {
		*error_code = RENDER_CAIRO_IMAGE_SURFACE_TOO_WIDE;
		l_str_free(str);
		return NULL;
	}
	/*
	 * Create cairo image surface onto which layout will be rendered
	 */
	c_surf = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
		layout_width + shift_x, layout_height);
	if ((c_status = cairo_surface_status(c_surf)) != CAIRO_STATUS_SUCCESS) {
		INFO_ERR("%s(): cairo_image_surface_create(): %s\n", __func__,
			cairo_status_to_string(c_status))
		*error_code = RENDER_CREATE_CAIRO_IMAGE_SURFACE_ERROR;
		cairo_surface_destroy(c_surf);
		if (c_tail_surf != NULL)
			cairo_surface_destroy(c_tail_surf);
		l_str_free(str);
		return NULL;
	}
	c_context = cairo_create(c_surf);
	/*
	 * Copy c_tail_surf at beginning of c_surf
	 *
	 * === If reverse sc, replace 'tail' with 'head' and beginning part with ending part ===
	 */
	if (shift_x > 0) {
		if (STANDARD_SCROLLING) {
			/*
			 * cairo_set_source_(): dest_x - src_x, dest_y - src_y
			 */
			cairo_set_source_surface(c_context, c_tail_surf, 0, 0);
			/*
			 * cairo_rectangle(): dest_x, dest_y, src_w, src_h
			 */
			cairo_rectangle(c_context, 0, 0, shift_x, layout_height);
		} else {
			cairo_set_source_surface(c_context, c_tail_surf, layout_width, 0);
			cairo_rectangle(c_context, layout_width, 0, shift_x, layout_height);
		}
		cairo_set_operator(c_context, CAIRO_OPERATOR_SOURCE);
		cairo_fill(c_context);
		cairo_surface_destroy(c_tail_surf);
	}
	/*
	 * Render layout
	 */
	/* Draw background */
	if (get_params()->set_gradient_bg == 'y') {
		c_pattern = cairo_pattern_create_linear(0.0, 0.0, 0.0, (float)layout_height);
		if ((c_status = cairo_pattern_status(c_pattern)) == CAIRO_STATUS_SUCCESS) {
			if (cairo_pattern_get_type(c_pattern) == CAIRO_PATTERN_TYPE_LINEAR) {
				cairo_pattern_add_color_stop_rgba(c_pattern, 0.0,
					(float)prm->bg_color.red / G_MAXUINT16,
					(float)prm->bg_color.green / G_MAXUINT16,
					(float)prm->bg_color.blue / G_MAXUINT16,
					(float)prm->bg_color_alpha / G_MAXUINT16);
				cairo_pattern_add_color_stop_rgba(c_pattern, 1.0,
					(float)prm->bg_color2.red / (G_MAXUINT16),
					(float)prm->bg_color2.green / (G_MAXUINT16),
					(float)prm->bg_color2.blue / (G_MAXUINT16),
					(float)prm->bg_color_alpha / G_MAXUINT16);
				cairo_set_source(c_context, c_pattern);
				if (STANDARD_SCROLLING)
					cairo_rectangle(c_context, shift_x, 0, layout_width, layout_height);
				else
					cairo_rectangle(c_context, 0, 0, layout_width, layout_height);
				cairo_set_operator(c_context, CAIRO_OPERATOR_SOURCE);
				cairo_fill(c_context);
			} else {
				INFO_ERR("%s(): Cairo pattern type != linear (gradient)\n", __func__)
				errors = TRUE;
			}
		} else {
			INFO_ERR("%s(): cairo_pattern_create_linear(): %s\n", __func__,
				cairo_status_to_string(c_status))
			errors = TRUE;
		}
		cairo_pattern_destroy(c_pattern);
	} else {
		cairo_set_source_rgba(c_context,
			(float)prm->bg_color.red / G_MAXUINT16,
			(float)prm->bg_color.green / G_MAXUINT16,
			(float)prm->bg_color.blue / G_MAXUINT16,
			(float)prm->bg_color_alpha / G_MAXUINT16);
		if (STANDARD_SCROLLING)
			cairo_rectangle(c_context, shift_x, 0, layout_width, layout_height);
		else
			cairo_rectangle(c_context, 0, 0, layout_width, layout_height);
		cairo_set_operator(c_context, CAIRO_OPERATOR_SOURCE);
		cairo_fill(c_context);
	}
	if (errors) {
		*error_code = RENDER_CREATE_CAIRO_IMAGE_SURFACE_ERROR;
		cairo_surface_destroy(c_surf);
		cairo_destroy(c_context);
		l_str_free(str);
		return NULL;
	}
	/* Draw foreground */
	if (prm->shadow == 'y') {
		/* Draw shadow */
		if (prm->shadow_fx < 0)
			shadow_k = 1.0;
		else if (prm->shadow_fx > 10)
			shadow_k = 0.0;
		else
			shadow_k = 1.0 - (float)prm->shadow_fx / 10.0;
		if (get_params()->set_gradient_bg == 'y') {
			c_pattern = cairo_pattern_create_linear(0.0, 0.0, 0.0, (float)layout_height);
			if ((c_status = cairo_pattern_status(c_pattern)) == CAIRO_STATUS_SUCCESS) {
				if (cairo_pattern_get_type(c_pattern) == CAIRO_PATTERN_TYPE_LINEAR) {
					cairo_pattern_add_color_stop_rgba(c_pattern, 0.0,
						(float)prm->bg_color.red * shadow_k / G_MAXUINT16,
						(float)prm->bg_color.green * shadow_k / G_MAXUINT16,
						(float)prm->bg_color.blue * shadow_k / G_MAXUINT16,
						(float)prm->bg_color_alpha / G_MAXUINT16);
					cairo_pattern_add_color_stop_rgba(c_pattern, 1.0,
						(float)prm->bg_color2.red * shadow_k / (G_MAXUINT16),
						(float)prm->bg_color2.green * shadow_k / (G_MAXUINT16),
						(float)prm->bg_color2.blue * shadow_k / (G_MAXUINT16),
						(float)prm->bg_color_alpha / G_MAXUINT16);
					cairo_set_source(c_context, c_pattern);
					pango_cairo_update_layout(c_context, p_layout);
					if (STANDARD_SCROLLING)
						cairo_rectangle(c_context, shift_x + prm->shadow_offset_x, prm->shadow_offset_y,
							layout_width, layout_height);
					else
						cairo_rectangle(c_context, prm->shadow_offset_x, prm->shadow_offset_y,
							layout_width, layout_height);
					cairo_set_operator(c_context, CAIRO_OPERATOR_OVER);
					pango_cairo_show_layout(c_context, p_layout);
				} else {
					INFO_ERR("%s(): Cairo pattern type != linear (gradient)\n", __func__)
					errors = TRUE;
				}
			} else {
				INFO_ERR("%s(): cairo_pattern_create_linear(): %s\n", __func__,
					cairo_status_to_string(c_status))
				errors = TRUE;
			}
			cairo_pattern_destroy(c_pattern);
		} else {
			cairo_set_source_rgba(c_context,
				(float)prm->bg_color.red * shadow_k / G_MAXUINT16,
				(float)prm->bg_color.green * shadow_k / G_MAXUINT16,
				(float)prm->bg_color.blue * shadow_k / G_MAXUINT16,
				(float)prm->bg_color_alpha / G_MAXUINT16);
			pango_cairo_update_layout(c_context, p_layout);
			if (STANDARD_SCROLLING)
				cairo_rectangle(c_context, shift_x + prm->shadow_offset_x, prm->shadow_offset_y,
					layout_width, layout_height);
			else
				cairo_rectangle(c_context, prm->shadow_offset_x, prm->shadow_offset_y,
					layout_width, layout_height);
			cairo_set_operator(c_context, CAIRO_OPERATOR_OVER);
			pango_cairo_show_layout(c_context, p_layout);
		}
	}
	if (errors) {
		*error_code = RENDER_CREATE_CAIRO_IMAGE_SURFACE_ERROR;
		cairo_surface_destroy(c_surf);
		cairo_destroy(c_context);
		l_str_free(str);
		return NULL;
	}
	cairo_set_source_rgba(c_context,
		(float)prm->fg_color.red / G_MAXUINT16,
		(float)prm->fg_color.green / G_MAXUINT16,
		(float)prm->fg_color.blue / G_MAXUINT16,
		(float)prm->fg_color_alpha / G_MAXUINT16);
	pango_cairo_update_layout(c_context, p_layout);
	if (STANDARD_SCROLLING)
		cairo_rectangle(c_context, shift_x, 0, layout_width, layout_height);
	else
		cairo_rectangle(c_context, 0, 0, layout_width, layout_height);
	cairo_set_operator(c_context, CAIRO_OPERATOR_OVER);
	pango_cairo_show_layout(c_context, p_layout);
	/*
	 * Create 'tail' surface
	 */
	c_tail_surf = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
		get_ticker_env()->drwa_width, layout_height);
	c_tail_context = cairo_create(c_tail_surf);
	if (STANDARD_SCROLLING) {
		cairo_set_source_surface(c_tail_context, c_surf, - layout_width - shift_x + get_ticker_env()->drwa_width, 0);
		cairo_rectangle(c_tail_context, 0, 0, get_ticker_env()->drwa_width, layout_height);
	} else {
		cairo_set_source_surface(c_tail_context, c_surf, 0, 0);
		cairo_rectangle(c_tail_context, 0, 0, get_ticker_env()->drwa_width, layout_height);
	}
	cairo_set_operator(c_tail_context, CAIRO_OPERATOR_SOURCE);
	cairo_fill(c_tail_context);
	cairo_destroy(c_tail_context);
	if ((c_status = cairo_surface_status(c_tail_surf)) != CAIRO_STATUS_SUCCESS) {
		INFO_ERR("%s(): cairo_image_surface_create(): %s\n", __func__,
			cairo_status_to_string(c_status))
		*error_code = RENDER_CREATE_CAIRO_IMAGE_SURFACE_ERROR;
		cairo_surface_destroy(c_surf);
		cairo_surface_destroy(c_tail_surf);
		l_str_free(str);
		return NULL;
	}
	/*
	 * Drawing done - return surface
	 */
	*error_code = OK;	/* Needed ? Can't hurt anyways */
	cairo_destroy(c_context);
	l_str_free(str);
	return c_surf;
}

/*
 * Read and split stream into an array of lines so that, after rendering of any line,
 * we always get a cairo image surface whose width is <= XPIXMAP_MAXWIDTH.
 * Stream (*not* checked) is expected to be not NULL.
 *
 * s_a is a 'private' (static) RenderStringArray.
 */
static zboolean fill_in_string_array_from_stream(FILE *fp, FeedLinkAndOffset *link_and_offset,
	PangoLayout *p_layout, const Params *prm, int *error_code)
{
	char	*big_str, *sub_str, c;
	int	max_width, i, j;

	/* First we get the 'big' string. */
	big_str = get_big_string_from_stream(fp, p_layout, prm, error_code);
	if (big_str == NULL)
		return FALSE;
	else if (get_resource()->type == RESRC_FILE && strlen(big_str) > 50 * 1024) {
		/* FIXME: *MUST* BE OPTIMIZED (A LOT) */
		if (question_win("In this version, processing 'big' text files (size > 50 KiB) takes ages.\n"
			"(Time seems to increase exponentially with file size and this needs to be fixed.)\n"
			"Continue anyways ?", NO) == NO) {
			*error_code = RENDER_NO_RESOURCE;
			l_str_free(big_str);
			big_str = NULL;
			return FALSE;
		}
	}
	if (get_layout_width(p_layout, big_str) == 0) {
		*error_code = RENDER_PANGO_LAYOUT_WIDTH_OVERFLOW;
		l_str_free(big_str);
		big_str = NULL;
		return FALSE;
	}

	/*
	 * Compute offsets for 'big' string.
	 * get_resource()->link_and_offset->url values are already set.
	 */
	big_str = fill_in_offset_array_update_big_string(big_str, link_and_offset, p_layout);

	/* Then we fill in array with sub-strings. */
	max_width = XPIXMAP_MAXWIDTH - (2 * get_ticker_env()->drwa_width) - prm->shadow_offset_x;
	/*
	 * For debugging purposes, we can set max_width to a smaller value so that we get multiple sub-strings (RenderString's)
	 * ie setting max_width to 2000 or 3000;
	 */
	sub_str = big_str;

	for (i = 0; i < N_RENDER_STR_MAX; i++) {
#ifndef G_OS_WIN32
		if (get_resource()->type == RESRC_FILE)
			VERBOSE_INFO_OUT("\r%c Processing, please wait ... ", spinning_shape())
#endif
		s_a->render_str[i].str = l_str_new(NULL);
		s_a->render_str[i].is_valid = TRUE;
		s_a->render_str[i].width = 0;

		j = parse_utf8_str_until_layout_width(sub_str, p_layout, max_width);
		if (j == -2) {
			/* -2 means error */
			*error_code = RENDER_FILL_IN_STR_ARRAY_ERROR;
			warning(M_S_MOD, "%s(): %s", __func__, global_error_str(*error_code));
			break;
		} else if (j > 0) {
			/* = sub-string length in bytes (vs UTF-8 chars) */
			c = *(sub_str + j);
			*(sub_str + j) = '\0';
			if (g_utf8_validate(sub_str, -1, NULL)) {
				s_a->render_str[i].str = l_str_cat(s_a->render_str[i].str, sub_str);
				s_a->render_str[i].width = get_layout_width(p_layout, s_a->render_str[i].str);
			} else
				*error_code = RENDER_FILL_IN_STR_ARRAY_ERROR;
			*(sub_str + j) = c;
			sub_str += j;
			if (*error_code == RENDER_FILL_IN_STR_ARRAY_ERROR)
				break;
		} else {
			/* 0 or -1 means string width (in pixels) is already <= max_width */
			if (g_utf8_validate(sub_str, -1, NULL)) {
				s_a->render_str[i].str = l_str_cat(s_a->render_str[i].str, sub_str);
				s_a->render_str[i].width = get_layout_width(p_layout, s_a->render_str[i].str);
			} else
				*error_code = RENDER_FILL_IN_STR_ARRAY_ERROR;
			break;
		}
	}
#ifndef G_OS_WIN32
	if (get_resource()->type == RESRC_FILE)
		VERBOSE_INFO_OUT("Done\n")
#endif
	l_str_free(big_str);

	if (i >= N_RENDER_STR_MAX) {	/* Just warn - OK ? */
		warning(BLOCK, "%s(): Need more than %d strings to render stream.\n"
			"Compile option N_RENDER_STR_MAX should be adjust accordingly.\n",
			__func__, N_RENDER_STR_MAX);
		i = N_RENDER_STR_MAX - 1;
	}

	s_a->n = i + 1;
	if (STANDARD_SCROLLING)
		s_a->i = 0;
	else
		s_a->i = s_a->n - 1;

	if (*error_code == OK)
		return TRUE;
	else
		return FALSE;
}

/*
 * Get length (in bytes) of a sub-string (of an UTF-8 string) whose, if set inside
 * a layout, will have a width (in pixels) near but <= max_width.
 *
 * Returned length is not the max length possible, but differs only from max length
 * by a small amount of bytes (which is OK for what this function is used for, and
 * this also makes the function a little bit faster.)
 *
 * Return sub-string length in bytes (vs UTF-8 chars), -1 if string width (in pixels)
 * is already <= max_width, -2 if error.
 */
static int parse_utf8_str_until_layout_width(char *str, PangoLayout *p_layout, int max_width)
{
	char		*p, c;
	int		str_width = get_layout_width(p_layout, str);	/* In pixels */
	size_t		str_len = strlen(str);				/* In bytes */
	long		offset, delta;					/* In UTF-8 chars, ie 1 to 4 bytes */
	zboolean	is_beyond;

	if (str_width > max_width) {
		offset = g_utf8_strlen(str, -1);
		delta = offset / 2;
		p = str;
		is_beyond = TRUE;
		while (1) {
			offset += is_beyond ? - delta: delta;
			delta /= 2;
			if (delta == 0)
				delta = 1;
			p = g_utf8_offset_to_pointer(str, offset);
			if (p == NULL || p < str || p > str + str_len) {
				VERBOSE_INFO_ERR("%s(): g_utf8_offset_to_pointer() returned invalid pointer\n",
					__func__)
				return -2;
			}
			c = *p;
			*p = '\0';
			str_width = get_layout_width(p_layout, str);
			*p = c;
			if (str_width == -1) {
				VERBOSE_INFO_ERR("%s(): get_layout_width() error\n", __func__)
				return -2;
			} else if (str_width == max_width)
				break;
			else if (str_width > max_width)
				is_beyond = TRUE;
			else {
				if (max_width - str_width < 50)
					break;
				is_beyond = FALSE;
			}
		}
		if (p != NULL)
			return (int)(p - str);
		else
			return -1;
	} else if (str_width == -1) {	/* Error computing width */
		VERBOSE_INFO_ERR("%s(): get_layout_width() error\n", __func__)
		return -2;
	} else				/* Nothing to do */
		return -1;
}

/*
 * Return current str (or NULL if error) and increment/decrement index for
 * next call. Also compute s_a->links_extra_offset for current surface.
 *
 * s_a is a 'private' (static) RenderStringArray.
 */
static char *get_current_string_in_array(int *error_code)
{
	int j;

	*error_code = RENDER_PROCESS_STR_ARRAY_ERROR;

	if (s_a->n < 1) {
		INFO_ERR("%s(): s_a->n (= %d) < 1\n", __func__, s_a->n)
		return NULL;
	} else if (s_a->i >= 0 && s_a->i < s_a->n) {
		if (s_a->render_str[s_a->i].str == NULL) {
			INFO_ERR("%s(): s_a->render_str[%d].str = NULL\n", __func__, s_a->i)
			return NULL;
		} else if (!s_a->render_str[s_a->i].is_valid) {
			INFO_ERR("%s(): s_a->render_str[%d].is_valid = FALSE\n", __func__, s_a->i)
			return NULL;
		} else {
			/* Compute links extra offset for current surface */
			s_a->links_extra_offset = 0;
			for (j = 0; j < s_a->i; j++)
				s_a->links_extra_offset += s_a->render_str[j].width;
			DEBUG_INFO("RenderStringArray: s_a->n = %d, s_a->i = %d\n", s_a->n, s_a->i)
			*error_code = OK;
			/* New index on next call */
			if (STANDARD_SCROLLING)
				return s_a->render_str[s_a->i++].str;
			else
				return s_a->render_str[s_a->i--].str;
		}
	} else {
		INFO_ERR("%s(): s_a->i (= %d) is out of bounds (0 - %d)\n", __func__, s_a->i, s_a->n - 1)
		return NULL;
	}
}

/*
 * Translate link ranks inside text into offsets in surface.
 *
 * Scan text for "<LINK_TAG_CHAR>00n" and get offset = layout_width(< text from start to tag >),
 * for every rss item found. Then remove tags and fill an array of FeedLinkAndOffset's with offsets
 * (ranks and URLs are already known/set).
 *
 * For reverse scrolling, work somehow differently
 *
 * Return new str.
 */
static char *fill_in_offset_array_update_big_string(char *str, FeedLinkAndOffset *link_and_offset, PangoLayout *p_layout)
{
	char	tmp[4];
	int	i, j;

	if (STANDARD_SCROLLING) {
		link_and_offset->offset_in_surface = get_ticker_env()->drwa_width;	/* First offset (000) = get_ticker_env()->drwa_width */
		for (i = 1; i < NFEEDLINKANDOFFSETMAX; i++)				/* We reset all other offsets to 0 */
			(link_and_offset + i)->offset_in_surface = 0;
	} else {
		for (i = 0; i < NFEEDLINKANDOFFSETMAX; i++)				/* We reset all offsets to 0 */
			(link_and_offset + i)->offset_in_surface = 0;
	}

	for (i = 0; str[i] != '\0'; i++) {
		if (str[i] == LINK_TAG_CHAR) {
			str_n_cpy(tmp, str + i + 1, 3);
			j = atoi(tmp);
			str[i] = '\0';
			if (j > 0 && j < NFEEDLINKANDOFFSETMAX) {
				if (STANDARD_SCROLLING)
					(link_and_offset + j)->offset_in_surface =
						get_ticker_env()->drwa_width + get_layout_width(p_layout, str);
				else
					(link_and_offset + j)->offset_in_surface = get_layout_width(p_layout, str);
			}
			str = l_str_cat(str, str + i + 4);
		}
	}

	if (REVERSE_SCROLLING)
		(link_and_offset)->offset_in_surface = get_layout_width(p_layout, str);		/* First offset = layout width of full string */

	return str;
}

/*
 * Create a single long string from stream and do some processing (line delimiters,
 * special chars, upper case text.) Created string must be l_str_free'd when done.
 * Stream is expected to be not NULL, but *not* checked.
 * If error, return NULL and set error_code.
 */
static char *get_big_string_from_stream(FILE *fp, PangoLayout *p_layout, const Params *prm, int *error_code)
{
	char	*str, *empty_line, *tab_line;
	char	*tmp, *p, *p2, c;
	size_t	tmp_size = FGETS_STR_MAXLEN;

	fseek(fp, 0, SEEK_SET);
	empty_line = create_empty_line_from_layout(p_layout);
	tab_line = create_tab_line();

	str = l_str_new(NULL);
	/*
	 * We read stream (text file) one line at a time, to build str
	 */
	tmp = malloc2((tmp_size + 1) * sizeof(char));

#ifndef G_OS_WIN32
	while (getline(&tmp, &tmp_size, fp) != -1) {
#else
	while (fgets(tmp, tmp_size, fp) != NULL) {
#endif
		for (p = tmp; p != NULL && *p != '\0'; p = g_utf8_find_next_char(p, NULL)) {
			if (*p == '\n')
				str = l_str_cat(str, prm->line_delimiter);
			else if (*p == prm->new_page_char && prm->special_chars == 'y')
				str = l_str_cat(str, empty_line);
			else if (*p == prm->tab_char && prm->special_chars == 'y')
				str = l_str_cat(str, tab_line);
			else {
				p2 = g_utf8_find_next_char(p, NULL);
				if (p2 != NULL) {
					c = *p2;
					*p2 = '\0';
					str = l_str_cat(str, p);
					*p2 = c;
				} else
					break;
			}
		}
	}
	free(tmp);

	/* EOF or error ? */
	if (feof(fp) != 0) {
		if (prm->upper_case_text == 'y') {
			p = g_utf8_strup(str, -1);
			l_str_free(str);
			str = l_str_new(p);
			g_free(p);
		}
		/*
		 * str ends with an "empty line"
		 */
		if (STANDARD_SCROLLING)
			str = l_str_cat(str, empty_line);
		else
			str = l_str_insert_at_b(str, empty_line);
		*error_code = OK;
	} else {
		l_str_free(str);
		str = NULL;
		*error_code = READ_FROM_STREAM_ERROR;
		INFO_ERR("%s(): %s\n", __func__, global_error_str(*error_code))
	}

	l_str_free(empty_line);
	l_str_free(tab_line);

	return str;
}

/* Returned string must be l_str_free'd when done. */
static char *no_resource_str(PangoLayout *p_layout)
{
	char *str, *empty_line = create_empty_line_from_layout(p_layout);

	str = l_str_new(empty_line);
	str = l_str_cat(str, NO_RESOURCE_STR);
	str = l_str_cat(str, empty_line);
	l_str_free(empty_line);

	return str;
}

/* Returned string must be l_str_free'd when done. */
/*static char *welcome_str(PangoLayout *p_layout)
{
	char *str, *empty_line = create_empty_line_from_layout(p_layout);

	str = l_str_new(empty_line);
	str = l_str_cat(str, WELCOME_STR);
	str = l_str_cat(str, empty_line);
	l_str_free(empty_line);

	return str;
}*/

/* Created string must be l_str_free'd when done. */
static char *create_empty_line_from_layout(PangoLayout *p_layout)
{
	char *empty_line = l_str_new(NULL);

	do
		empty_line = l_str_cat(empty_line, " ");
	while (get_layout_width(p_layout, empty_line) < get_ticker_env()->drwa_width);

	return empty_line;
}

/* Created string must be l_str_free'd when done. */
static char *create_tab_line()
{
	char	*tab_line = l_str_new(NULL);
	int	i;

	for (i = 0; i < TAB_SIZE; i++)
		tab_line = l_str_cat(tab_line, " ");

	return tab_line;
}

int get_font_size_from_layout_height(int height, const char *font_name2)
{
	PangoLayout		*p_layout;
	PangoFontDescription	*f_des;
	char			font_name_size[FONT_MAXLEN + 1];
	char			font_name[FONT_NAME_MAXLEN + 1], font_size[FONT_SIZE_MAXLEN + 1];
	char			check_str[] = "gjpqyÀÄÅÇÉÖÜ€£$!&({[?|";	/* Should be enough - Is it ? */
	int			height2;
	int			i = 1;

	p_layout = pango_layout_new(gtk_widget_get_pango_context(get_ticker_env()->win));
	pango_layout_set_attributes(p_layout, NULL);
	pango_layout_set_single_paragraph_mode(p_layout, TRUE);

	str_n_cpy(font_name, font_name2, FONT_NAME_MAXLEN);

	do {
		snprintf(font_size, FONT_SIZE_MAXLEN + 1, "%3d", i++);
		compact_font(font_name_size, font_name, font_size);
		f_des = pango_font_description_from_string((const char *)font_name_size);
		pango_layout_set_font_description(p_layout, f_des);
		pango_font_description_free(f_des);
		height2 = get_layout_height(p_layout, check_str);
	} while (height2 < height && i < 1000);

	if (p_layout != NULL)
		g_object_unref(p_layout);

	return (i - 1);
}

/* In *pixels*, return -1 if error. */
int get_layout_height_from_font_name_size(const char *font_name_size)
{
	PangoLayout		*p_layout;
	PangoFontDescription	*f_des;
	char			check_str[] = "gjpqyÀÄÅÇÉÖÜ€£$!&({[?|";
	int			height;

	p_layout = pango_layout_new(gtk_widget_get_pango_context(get_ticker_env()->win));
	pango_layout_set_attributes(p_layout, NULL);
	pango_layout_set_single_paragraph_mode(p_layout, TRUE);

	f_des = pango_font_description_from_string(font_name_size);
	pango_layout_set_font_description(p_layout, f_des);
	pango_font_description_free(f_des);

	height = get_layout_height(p_layout, check_str);
	if (p_layout != NULL)
		g_object_unref(p_layout);

	return height;
}

/* In *pixels*, return 0 if empty str, -1 if error. */
static int get_layout_width(PangoLayout *p_layout, const char *str)
{
	int layout_width;

	if (str != NULL) {
		if (str[0] != '\0') {
			pango_layout_set_text(p_layout, str, -1);
			pango_layout_context_changed(p_layout);
			pango_layout_get_pixel_size(p_layout, &layout_width, NULL);
		} else {
			DEBUG_INFO("%s(pango_layout, str): str length = 0 - Troubles ahead ?\n", __func__)
			return 0;
		}
	} else {
		VERBOSE_INFO_ERR("%s(pango_layout, str): str is NULL\n", __func__)
		layout_width = -1;
	}
	return layout_width;
}

/* In *pixels*, return 0 if empty str, -1 if error. */
static int get_layout_height(PangoLayout *p_layout, const char *str)
{
	int layout_height;

	if (str != NULL) {
		if (str[0] != '\0') {
			pango_layout_set_text(p_layout, str, -1);
			pango_layout_context_changed(p_layout);
			pango_layout_get_pixel_size(p_layout, NULL, &layout_height);
		} else {
			DEBUG_INFO("%s(pango_layout, str): str length = 0 - Troubles ahead ?\n", __func__)
			return 0;
		}
	} else {
		VERBOSE_INFO_ERR("%s(pango_layout, str): str is NULL\n", __func__)
		layout_height = -1;
	}
	return layout_height;
}

/* Return '-', '\', '|', '/', sequentially */
char spinning_shape()
{
	static int	counter = 0;
	char		str[4] = "-\\|/";

	counter = counter & 3;
	return str[counter++];
}

/*
 * Show begining and end of a 'long' UTF-8 string, mainly for debugging purposes
 * -> Fist 30 chars + " ... " + last 30 chars
 */
void show_str_beginning_and_end(char *str)
{
	char	*p, c;
	char	*s1 = NULL, *s2 = NULL;
	int	i;

	for (p = str, i = 0; p != NULL && *p != '\0'; p = g_utf8_find_next_char(p, NULL), i++) {
		if (i == 30) {
			c = *p;
			*p = 0;
			s1 = l_str_new(str);
			*p = c;
		} else if (i == (int)strlen(str) - 30) {
			s2 = l_str_new(p);
			break;
		}
	}
	INFO_OUT("%s ... %s\n", s1, s2)
	if (s1 != NULL)
		l_str_free(s1);
	if (s2 != NULL)
		l_str_free(s2);
}
