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

#define CLOCK_DELIMITER_STR	" | "			/* TODO: Should be a user-defined setting */
#ifndef G_OS_WIN32
#  define NO_PADDING		"-"			/* Don't pad a numeric result string. The %- flag is a Glibc extension,
							 * not ISO C, so expect a compile-time warning. */
#else
#  define NO_PADDING		"#"			/* (win32) Remove leading zeros (if any) */
#endif
#define DATE_STR		"%a %b %" NO_PADDING "d  "	/* Mon Jan 01 */
#define DATE_STR_2		"%a %" NO_PADDING "d %b  "	/* Mon 01 Jan */

/*
 *	R -> " | 00:00:00 AM"
 *	L -> "00:00:00 AM | "
 */

void display_time(const Params *prm)
{
	TickerEnv		*env = get_ticker_env();
	PangoLayout		*p_layout;
	PangoFontDescription	*f_des;
	int			layout_width, layout_height;
	cairo_surface_t		*surf_clock;
	cairo_t			*cr;
	cairo_pattern_t		*cr_p;
	float			shadow_k;
	time_t			time2;
	char			*format;
	char			*date_str;
	char			time_str[64];
	char			tmp[64];
	int			height_diff;

	if (env->suspend_rq || (prm->clock != 'l' && prm->clock != 'r'))
		return;
	else if ((p_layout = pango_layout_new(gtk_widget_get_pango_context(env->win))) == NULL) {
		INFO_ERR("%s(): Can't create pango layout\n", __func__)
		return;
	}
	pango_layout_set_attributes(p_layout, NULL);
	pango_layout_set_single_paragraph_mode(p_layout, TRUE);
	f_des = pango_font_description_from_string((const char *)prm->clock_font_name_size);
	pango_layout_set_font_description(p_layout, f_des);
	pango_font_description_free(f_des);

	if (prm->clock_date == 'y') {
		if (prm->clock_sec == 'y') {
			if (prm->clock_12h == 'y') {
				if (prm->clock_alt_date_form != 'y')
					format = DATE_STR "%" NO_PADDING "I:%M:%S %p";
				else
					format = DATE_STR_2 "%" NO_PADDING "I:%M:%S %p";
			} else {
				if (prm->clock_alt_date_form != 'y')
					format = DATE_STR "%" NO_PADDING "H:%M:%S";
				else
					format = DATE_STR_2 "%" NO_PADDING "H:%M:%S";
			}
		} else {
			if (prm->clock_12h == 'y') {
				if (prm->clock_alt_date_form != 'y')
					format = DATE_STR "%" NO_PADDING "I:%M %p";
				else
					format = DATE_STR_2 "%" NO_PADDING "I:%M %p";
			} else {
				if (prm->clock_alt_date_form != 'y')
					format = DATE_STR "%" NO_PADDING "H:%M";
				else
					format = DATE_STR_2 "%" NO_PADDING "H:%M";
			}
		}
	} else {
		if (prm->clock_sec == 'y') {
			if (prm->clock_12h == 'y')
				format = "%" NO_PADDING "I:%M:%S %p";
			else
				format = "%" NO_PADDING "H:%M:%S";
		} else {
			if (prm->clock_12h == 'y')
				format = "%" NO_PADDING "I:%M %p";
			else
				format = "%" NO_PADDING "H:%M";
		}
	}

	time2 = time(NULL);
	strftime(tmp, 64, format, localtime(&time2));

	if (prm->clock == 'l')
		snprintf(time_str, 64, "%s%s", tmp, CLOCK_DELIMITER_STR);
	else if (prm->clock == 'r')
		snprintf(time_str, 64, "%s%s", CLOCK_DELIMITER_STR, tmp);
	pango_layout_set_text(p_layout, time_str, -1);
	pango_layout_context_changed(p_layout);
	pango_layout_get_pixel_size(p_layout, &layout_width, &layout_height);

	env->drwa_clock_width = get_clock_width(prm);		/* width_diff */
	/*
	 * For some fonts like 'impact bold' which give strange things
	 * width_diff = drwa_clock_width - layout_width.
	 */
	gtk_widget_set_size_request(env->drwa_clock, env->drwa_clock_width, env->drwa_height);
	height_diff = (env->drwa_height - layout_height) / 2;
	/*
	 * Create cairo image surface onto which layout will be rendered
	 */
	surf_clock = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
		env->drwa_clock_width, env->drwa_height);
	cr = cairo_create(surf_clock);
	/*
	 * Render layout
	 */
	/* Draw background */
	if (get_params()->set_clock_gradient_bg == 'y') {
		cr_p = cairo_pattern_create_linear(0.0, 0.0, 0.0, (float)env->drwa_height);
		if (cairo_pattern_status(cr_p) == CAIRO_STATUS_SUCCESS) {
			if (cairo_pattern_get_type(cr_p) == CAIRO_PATTERN_TYPE_LINEAR) {
				cairo_pattern_add_color_stop_rgba(cr_p, 0.0,
					(float)prm->clock_bg_color.red / G_MAXUINT16,
					(float)prm->clock_bg_color.green / G_MAXUINT16,
					(float)prm->clock_bg_color.blue / G_MAXUINT16,
					(float)prm->clock_bg_color_alpha / G_MAXUINT16);
				cairo_pattern_add_color_stop_rgba(cr_p, 1.0,
					(float)prm->clock_bg_color2.red / (G_MAXUINT16),
					(float)prm->clock_bg_color2.green / (G_MAXUINT16),
					(float)prm->clock_bg_color2.blue / (G_MAXUINT16),
					(float)prm->clock_bg_color_alpha / G_MAXUINT16);
				cairo_set_source(cr, cr_p);
				cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
				cairo_paint(cr);
			} else
				INFO_ERR("%s(): cairo pattern type != linear (gradient)\n",
					__func__)
		} else
			INFO_ERR("%s(): cairo_pattern_create_linear() error\n",
					__func__)
		cairo_pattern_destroy(cr_p);
	} else {
		cairo_set_source_rgba(cr,
			(float)prm->clock_bg_color.red / G_MAXUINT16,
			(float)prm->clock_bg_color.green / G_MAXUINT16,
			(float)prm->clock_bg_color.blue / G_MAXUINT16,
			(float)prm->clock_bg_color_alpha / G_MAXUINT16);
		cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
		cairo_paint(cr);
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
			cr_p = cairo_pattern_create_linear(0.0, 0.0, 0.0, (float)env->drwa_height);
			if (cairo_pattern_status(cr_p) == CAIRO_STATUS_SUCCESS) {
				if (cairo_pattern_get_type(cr_p) == CAIRO_PATTERN_TYPE_LINEAR) {
					cairo_pattern_add_color_stop_rgba(cr_p, 0.0,
						(float)prm->clock_bg_color.red * shadow_k / G_MAXUINT16,
						(float)prm->clock_bg_color.green * shadow_k / G_MAXUINT16,
						(float)prm->clock_bg_color.blue * shadow_k / G_MAXUINT16,
						(float)prm->clock_bg_color_alpha / G_MAXUINT16);
					cairo_pattern_add_color_stop_rgba(cr_p, 1.0,
						(float)prm->clock_bg_color2.red * shadow_k / (G_MAXUINT16),
						(float)prm->clock_bg_color2.green * shadow_k / (G_MAXUINT16),
						(float)prm->clock_bg_color2.blue * shadow_k / (G_MAXUINT16),
						(float)prm->clock_bg_color_alpha / G_MAXUINT16);
					cairo_set_source(cr, cr_p);
					pango_cairo_update_layout(cr, p_layout);
					cairo_move_to(cr, prm->shadow_offset_x, prm->shadow_offset_y);
					cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
					pango_cairo_show_layout(cr, p_layout);
				} else
					INFO_ERR("%s(): cairo pattern type != linear (gradient)\n",
						__func__)
			} else
				INFO_ERR("%s(): cairo_pattern_create_linear() error\n",
					__func__)
			cairo_pattern_destroy(cr_p);
		} else {
			cairo_set_source_rgba(cr,
				(float)prm->clock_bg_color.red * shadow_k / G_MAXUINT16,
				(float)prm->clock_bg_color.green * shadow_k / G_MAXUINT16,
				(float)prm->clock_bg_color.blue * shadow_k / G_MAXUINT16,
				(float)prm->clock_bg_color_alpha / G_MAXUINT16);
			pango_cairo_update_layout(cr, p_layout);
			cairo_move_to(cr, prm->shadow_offset_x, prm->shadow_offset_y);
			cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
			pango_cairo_show_layout(cr, p_layout);
		}
	}
	/* Draw text */
	cairo_set_source_rgba(cr,
		(float)prm->clock_fg_color.red / G_MAXUINT16,
		(float)prm->clock_fg_color.green / G_MAXUINT16,
		(float)prm->clock_fg_color.blue / G_MAXUINT16,
		(float)prm->clock_fg_color_alpha / G_MAXUINT16);
	pango_cairo_update_layout(cr, p_layout);
	cairo_move_to(cr, 0, 0);
	cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
	pango_cairo_show_layout(cr, p_layout);
	/* Drawing done */
	if (p_layout != NULL)
		g_object_unref(p_layout);
	cairo_destroy(cr);
	/*
	 * Draw onto clock area
	 * (We now use cairo instead of deprecated gdk_draw_sth.
	 * Should we use gdk_window_begin/end_paint_rect() stuff here too ?
	 * Not really as this is only called every 500 ms.)
	 */
	cr = gdk_cairo_create(GDK_DRAWABLE((env->drwa_clock)->window));
	cairo_set_source_surface(cr, surf_clock, 0, height_diff);
	cairo_rectangle(cr, 0, 0, env->drwa_clock_width, env->drwa_height);
	cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
	cairo_fill(cr);
	cairo_destroy(cr);
	cairo_surface_destroy(surf_clock);
}

/* Actually clock max width */
int get_clock_width(const Params *prm)
{
	static char		clock_font_name_size_bak[FONT_MAXLEN + 1] = "";
	static char		clock_sec_bak, clock_12h_bak, clock_date_bak;
	static int		date_width_bak = 0;
	int			date_width;
	static int		width = 0;
	PangoLayout		*p_layout;
	PangoFontDescription	*f_des;
	int			layout_width, layout_height, layout_max_width;
	char			*time_str;
	time_t			time2;
	struct tm		*local_time;
	zboolean		hour_is_double_digit;
	char			tmp[64];
	int			i;

	if (prm->clock == 'l' || prm->clock == 'r') {
		p_layout = pango_layout_new(gtk_widget_get_pango_context(get_ticker_env()->win));
		pango_layout_set_attributes(p_layout, NULL);
		pango_layout_set_single_paragraph_mode(p_layout, TRUE);
		f_des = pango_font_description_from_string((const char *)prm->clock_font_name_size);
		pango_layout_set_font_description(p_layout, f_des);
		pango_font_description_free(f_des);

		time2 = time(NULL);
		local_time = localtime(&time2);

		if (prm->clock_date == 'y') {
			strftime(tmp, 64, DATE_STR, local_time);	/* Will have same width using DATE_STR or DATE_STR_2 */
			pango_layout_set_text(p_layout, tmp, -1);
			pango_layout_context_changed(p_layout);
			pango_layout_get_pixel_size(p_layout, &layout_width, &layout_height);
			date_width = layout_width;
		} else
			date_width = 0;

		if (		prm->clock_sec != clock_sec_bak ||
				prm->clock_12h != clock_12h_bak ||
				prm->clock_date != clock_date_bak ||
				strcmp(prm->clock_font_name_size, clock_font_name_size_bak) != 0 ||
				date_width != date_width_bak ||
				width == 0) {
			clock_sec_bak = prm->clock_sec;
			clock_12h_bak = prm->clock_12h;
			clock_date_bak = prm->clock_date;
			str_n_cpy(clock_font_name_size_bak, prm->clock_font_name_size, FONT_MAXLEN);
			date_width_bak = date_width;

			if (prm->clock_sec == 'y') {
				if (prm->clock_12h == 'y')
					time_str = "%c%c:%c%c:%c%c AM" CLOCK_DELIMITER_STR;	/* Ordering doesn't change witdh */
				else
					time_str = "%c%c:%c%c:%c%c" CLOCK_DELIMITER_STR;
			} else {
				if (prm->clock_12h == 'y')
					time_str = "%c%c:%c%c AM" CLOCK_DELIMITER_STR;
				else
					time_str = "%c%c:%c%c" CLOCK_DELIMITER_STR;
			}

			/*
			 * Hack to adjust clock width because hours are not padded:
			 * we test current hour.
			 */
			strftime(tmp, 64, prm->clock_12h == 'y' ? "%I" : "%H", local_time);
			if (atoi(tmp) > 9)
				hour_is_double_digit = TRUE;
			else
				hour_is_double_digit = FALSE;

			layout_max_width = 0;
			for (i = '0'; i <= '9'; i++) {
				if (prm->clock_sec == 'y')
					snprintf(tmp, 64, time_str, i, i, i, i, i, i);
				else
					snprintf(tmp, 64, time_str, i, i, i, i);

				time_str = tmp;
				/* Hack to adjust width */
				if (!hour_is_double_digit)
					time_str++;

				pango_layout_set_text(p_layout, time_str, -1);
				pango_layout_context_changed(p_layout);
				pango_layout_get_pixel_size(p_layout, &layout_width, &layout_height);
				if (layout_width > layout_max_width)
					layout_max_width = layout_width;
			}
			width = date_width + layout_max_width;

			if (p_layout != NULL)
				g_object_unref(p_layout);
		}
	} else
		width = 0;	/* Return 0 although if clock = none, actual widget width = 1 */
	return width;
}
