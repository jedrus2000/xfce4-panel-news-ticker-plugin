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

void set_default_options(Params *prm)
{
	char tmp_font[FONT_MAXLEN + 1];

	prm->delay = DELAY;
	prm->shift_size = SHIFTSIZE;

	compact_font(tmp_font, FONTNAME, FONTSIZE);
	str_n_cpy(prm->font_name_size, tmp_font, FONT_MAXLEN);
	get_gdk_color_from_hexstr(FGCOLOR, &prm->fg_color, &prm->fg_color_alpha);
	/*get_gdk_color_from_hexstr(HIGHLIGHTFGCOLOR, &prm->highlight_fg_color, &prm->highlight_fg_color_alpha);*/
	get_gdk_color_from_hexstr(BGCOLOR, &prm->bg_color, &prm->bg_color_alpha);
	prm->set_gradient_bg = SETGRADIENTBG;
	get_gdk_color_from_hexstr(BGCOLOR2, &prm->bg_color2, &prm->bg_color2_alpha);

	prm->disable_screen_limits = DISABLESCREENLIMITS;
	prm->win_x = WIN_X;
	prm->win_y = WIN_Y;
	prm->win_w = get_ticker_env()->screen_w;	/* Instead of WIN_W; */
	prm->win_h = WIN_H;

	prm->windec = WINDEC;
	prm->always_on_top = ALWAYSONTOP;
	prm->win_transparency = WINTRANSPARENCY;
	prm->icon_in_taskbar = ICONINTASKBAR;
	prm->win_sticky = WINSTICKY;
	prm->override_redirect = OVERRIDE_REDIRECT;

	prm->shadow = SHADOW;
	prm->shadow_offset_x = SHADOWOFFSET_X;
	prm->shadow_offset_y = SHADOWOFFSET_Y;
	prm->shadow_fx = SHADOWFX;

	str_n_cpy(prm->line_delimiter, LINEDELIMITER, DELIMITER_MAXLEN);

	prm->special_chars = SPECIALCHARS;
	prm->new_page_char = NEWPG;
	prm->tab_char = TABCH;

	prm->rss_refresh = RSSREFRESH;
	prm->reverse_sc = REVERSESCROLLING;
	prm->feed_title = FEEDTITLE;
	str_n_cpy(prm->feed_title_delimiter, FEEDTITLEDELIMITER, DELIMITER_MAXLEN);
	prm->item_title = ITEMTITLE;
	str_n_cpy(prm->item_title_delimiter, ITEMTITLEDELIMITER, DELIMITER_MAXLEN);
	prm->item_description = ITEMDESCRIPTION;
	str_n_cpy(prm->item_description_delimiter, ITEMDESCRIPTIONDELIMITER, DELIMITER_MAXLEN);

	prm->n_items_per_feed = NITEMSPERFEED;
	/*prm->mark_item_action = MARKITEMACTION;*/
	prm->strip_html_tags = STRIPHTMLTAGS;
	prm->upper_case_text = UPPERCASETEXT;
	str_n_cpy(prm->homefeed, HOMEFEED, FILE_NAME_MAXLEN);

	str_n_cpy(prm->open_link_cmd, OPENLINKCMD, FILE_NAME_MAXLEN);
	str_n_cpy(prm->open_link_args, OPENLINKARGS, FILE_NAME_MAXLEN);

	prm->clock = CLOCK;
	prm->clock_sec = CLOCKSEC;
	prm->clock_12h = CLOCK12H;
	prm->clock_date = CLOCKDATE;
	prm->clock_alt_date_form = CLOCKALTDATEFORM;
	get_gdk_color_from_hexstr(CFGCOLOR, &prm->clock_fg_color, &prm->clock_fg_color_alpha);
	get_gdk_color_from_hexstr(CBGCOLOR, &prm->clock_bg_color, &prm->clock_bg_color_alpha);
	prm->set_clock_gradient_bg = SETCLOCKGRADIENTBG;
	get_gdk_color_from_hexstr(CBGCOLOR2, &prm->clock_bg_color2, &prm->clock_bg_color2_alpha);
	compact_font(tmp_font, CFONTNAME, CFONTSIZE);
	str_n_cpy(prm->clock_font_name_size, tmp_font, FONT_MAXLEN);

	prm->disable_popups = DISABLEPOPUPS;
	prm->pause_on_mouseover = PAUSEONMOUSEOVER;
	prm->disable_leftclick = DISABLELEFTCLICK;
	prm->mouse_wheel_action = MOUSEWHEELACTION;
	prm->sfeedpicker_autoclose = SFEEDPICKERAUTOCLOSE;
	prm->enable_feed_ordering = ENABLEFEEDORDERING;

	prm->use_authentication = USEAUTHENTICATION;
	str_n_cpy(prm->user, USER, USER_MAXLEN);
	prm->use_proxy = USEPROXY;
	str_n_cpy(prm->proxy_host, PROXYHOST, PROXY_HOST_MAXLEN);
	str_n_cpy(prm->proxy_port, PROXYPORT, PROXY_PORT_MAXLEN);
	prm->use_proxy_authentication = USEPROXYAUTHENTICATION;
	str_n_cpy(prm->proxy_user, PROXYUSER, PROXY_USER_MAXLEN);
	prm->connect_timeout = CONNECT_TIMEOUT;
	prm->send_recv_timeout = SENDRECV_TIMEOUT;
}

/*
 * Load config file if any & set up - returns 0 if OK.
 * Store options in a string array and initialize a ptr arrays that matches strings.
 * We assume we have up to N_OPTION_MAX options, OPTION_MAXLEN char long max each.
 */
int get_config_file_options(Params *prm)
{
	FILE	*conf_fp;
	char	options_array[N_OPTION_MAX][OPTION_MAXLEN + 2];
	char	*ptr[N_OPTION_MAX + 1];
	int	i, j = 0;

	if ((conf_fp = g_fopen(get_datafile_full_name_from_name(CONFIG_FILE), "rb")) != NULL) {
		/*
		 * Parse config file
		 */
		for (i = 0; ; i++) {
			/*
			 * Here fgets() read OPTION_MAXLEN + 1 char max including
			 * '\n' then add '\0' (if I'm right)
			 */
			if (i >= N_OPTION_MAX) {
				ptr[i] = NULL;
				warning(BLOCK, "Too many lines in configuration file '%s' "
					"(max = %d)\nWon't parse further than limit\n",
					get_datafile_full_name_from_name(CONFIG_FILE), N_OPTION_MAX);
				break;
			}
			/* TODO: Should use getline() instead ? */
			if (fgets(options_array[i], OPTION_MAXLEN + 2, conf_fp) == NULL) {
				ptr[i] = NULL;
				break;
			} else {
				j = strlen(options_array[i]);
				options_array[i][j - 1] = '\0';	/* We remove trailing '\n' */
				ptr[i] = options_array[i];	/* so now options max length = OPTION_MAXLEN. */
			}
		}
		if (i > 0)
			j = parse_options_array(prm, i - 1, (const char **)ptr);
		else
			j = 0;
		fclose(conf_fp);
	} else
		INFO_ERR("Can't read configuration file '%s': %s\n",
			get_datafile_full_name_from_name(CONFIG_FILE), strerror(errno))
	return j;
}

/*
 * Options array parser (NULL terminated ptrs array).
 * We assume we have up to N_OPTION_MAX options, OPTION_MAXLEN char long max each.
 */
int parse_options_array(Params *prm, int n_options, const char *option[])
{
	int	i, j, error_code = OK;

	if (n_options > N_OPTION_MAX) {
		n_options = N_OPTION_MAX;
		INFO_ERR("Too many options (max = %d) - Won't parse further than limit\n",
			N_OPTION_MAX)
		error_code = OPTION_TOO_MANY;
	}
	for (i = 0; i < n_options && option[i] != NULL; i++)
		if ((j = get_name_value_from_option(prm, option[i])) != OK) {
			/* FIXME: Will only get last error code */
			error_code = j;
			/* But if OPTION_ERROR_QUIT is set to TRUE */
			if (OPTION_ERROR_QUIT)
				break;
		}
	return error_code;
}

int get_name_value_from_option(Params *prm, const char *option)
{
	/*
	 * Options are splitted into name and value.
	 * font -> font_name is FONT_NAME_MAXLEN char max long, font_size FONT_SIZE_MAXLEN,
	 * plus 1 extra space so font_name_size is up to
	 * FONT_MAXLEN = FONT_NAME_MAXLEN + 1 + FONT_SIZE_MAXLEN char long.
	 * OPTION_MAXLEN = OPTION_NAME_MAXLEN + OPTION_VALUE_MAXLEN + 4 (-name="value").
	 *
	 * === If unsure, check this in tickr.h ===
	 */
	char	name[OPTION_NAME_MAXLEN + 1], value[OPTION_VALUE_MAXLEN + 1];
	char	font_name[FONT_NAME_MAXLEN + 1], font_size[FONT_SIZE_MAXLEN + 1];
	char	c_font_name[FONT_NAME_MAXLEN + 1], c_font_size[FONT_SIZE_MAXLEN + 1];
	char	tmp[OPTION_MAXLEN + 1];
	char	tmp_font[FONT_MAXLEN + 1];
	int	i, error_code = OK;

	split_font(prm->font_name_size, font_name, font_size);
	split_font(prm->clock_font_name_size, c_font_name, c_font_size);
	/*
	 * Split option into name & value if any
	 */
	name[0] = '\0';
	value[0] = '\0';
	if (option[0] == '-') {
		str_n_cpy(tmp, option + 1, OPTION_MAXLEN);
		for (i = 0; i < OPTION_MAXLEN - 1; i++)
			if (tmp[i] == '=') {
				tmp[i] = '\0';
				str_n_cpy(name, tmp, OPTION_NAME_MAXLEN);
				str_n_cpy(value, tmp + i + 1, OPTION_VALUE_MAXLEN);
				break;
			}
	} else if (option[0] != '\0') {
		INFO_ERR("Invalid option: %s\n", option)
		return (error_code = OPTION_INVALID);
	}
	/*
	 * All *string* values are saved inclosed in a pair of ". When read, enclosing " (or ')
	 * are removed. (This also ensure backward compatibility with previous versions.)
	 */
	str_n_cpy(tmp, value, OPTION_MAXLEN);
	if ((i = strlen(tmp) >= 2) && ((tmp[0] == '\"' && tmp[i - 1] == '\"') ||
			(tmp[0] == '\'' && tmp[i - 1] == '\''))) {
		str_n_cpy(value, tmp + 1, OPTION_VALUE_MAXLEN);
		value[strlen(value) - 1] = '\0';
	}
	/*
	 * Check option name and get value if any
	 */
	if (strcmp(name, "delay") == 0)
		prm->delay = atoi(value);
	else if (strcmp(name, "shiftsize") == 0)
		prm->shift_size = atoi(value);
	else if (strcmp(name, "fontname") == 0)
		str_n_cpy(font_name, value, FONT_NAME_MAXLEN);
	else if (strcmp(name, "fontsize") == 0) {
		str_n_cpy(font_size, value, FONT_SIZE_MAXLEN);
		if (atoi(font_size) > FONT_MAXSIZE) {
			snprintf(font_size, FONT_SIZE_MAXLEN + 1, "%3d", FONT_MAXSIZE);
			INFO_ERR("Font size set to %d (can't be above)\n",
				FONT_MAXSIZE)
		}
	} else if (strcmp(name, "fgcolor") == 0) {
		if (!get_gdk_color_from_hexstr(value, &prm->fg_color,
						&prm->fg_color_alpha)) {
			get_gdk_color_from_hexstr(FGCOLOR, &prm->fg_color,
						&prm->fg_color_alpha);
			error_code = OPTION_VALUE_INVALID;
		}
	} else /*if (strcmp(name, "highlightfgcolor") == 0) {
		if (!get_gdk_color_from_hexstr(value, &prm->highlight_fg_color,
						&prm->highlight_fg_color_alpha)) {
			get_gdk_color_from_hexstr(HIGHLIGHTFGCOLOR, &prm->highlight_fg_color,
						&prm->highlight_fg_color_alpha);
			error_code = OPTION_VALUE_INVALID;
		}
	} else*/ if (strcmp(name, "bgcolor") == 0) {
		if (!get_gdk_color_from_hexstr(value, &prm->bg_color,
						&prm->bg_color_alpha)) {
			get_gdk_color_from_hexstr(BGCOLOR, &prm->bg_color,
						&prm->bg_color_alpha);
			error_code = OPTION_VALUE_INVALID;
		}
	} else if (strcmp(name, "setgradientbg") == 0) {
		if (strcmp(value, "y") != 0 && strcmp(value, "n") != 0) {
			INFO_ERR("Invalid value: %s\n", option)
			error_code = OPTION_VALUE_INVALID;
		} else
			prm->set_gradient_bg = value[0];
	} else if (strcmp(name, "bgcolor2") == 0) {
		if (!get_gdk_color_from_hexstr(value, &prm->bg_color2,
						&prm->bg_color2_alpha)) {
			get_gdk_color_from_hexstr(BGCOLOR, &prm->bg_color2,
						&prm->bg_color2_alpha);
			error_code = OPTION_VALUE_INVALID;
		}
	} else if (strcmp(name, "disablescreenlimits") == 0) {
		if (strcmp(value, "y") != 0 && strcmp(value, "n") != 0) {
			INFO_ERR("Invalid value: %s\n", option)
			error_code = OPTION_VALUE_INVALID;
		} else
			prm->disable_screen_limits = value[0];
	} else if (strcmp(name, "win_x") == 0)
		prm->win_x = atoi(value);
	else if (strcmp(name, "win_y") == 0)
		prm->win_y = atoi(value);
	else if (strcmp(name, "win_w") == 0) {
		prm->win_w = atoi(value);
		/* Same as 'full width' but from command line */
		if (prm->win_w == 0)
			prm->win_w = get_ticker_env()->screen_w;
	} else if (strcmp(name, "win_h") == 0) {
		prm->win_h = atoi(value);
		/*
		* If win_h is set and > 0, it will override requested font size
		* with computed one.
		*/
		if (prm->win_h > 0 && prm->win_h < DRWA_HEIGHT_MIN)
			prm->win_h = DRWA_HEIGHT_MIN;
	} else if (strcmp(name, "windec") == 0) {
		if (strcmp(value, "y") != 0 && strcmp(value, "n") != 0) {
			INFO_ERR("Invalid value: %s\n", option)
			error_code = OPTION_VALUE_INVALID;
		} else
			prm->windec = value[0];
	} else if (strcmp(name, "alwaysontop") == 0) {
		if (strcmp(value, "y") != 0 && strcmp(value, "n") != 0) {
			INFO_ERR("Invalid value: %s\n", option)
			error_code = OPTION_VALUE_INVALID;
		} else
			prm->always_on_top = value[0];
	} else if (strcmp(name, "wintransparency") == 0) {
		/* Compare as integers (because less bug-prone) then set as double.
		 * Saved to / retrieved from config file as [transparency x 10]. */
		prm->win_transparency = atoi(value);
		if ((int)prm->win_transparency < 1) {
			INFO_ERR("Invalid value: %s - will be set to 1\n", option)
			prm->win_transparency = 1.0;
			error_code = OPTION_VALUE_INVALID;
		} else if ((int)prm->win_transparency > 10) {
			INFO_ERR("Invalid value: %s - will be set to 10\n", option)
			prm->win_transparency = 10.0;
			error_code = OPTION_VALUE_INVALID;
		}
		prm->win_transparency /= 10;
	} else if (strcmp(name, "iconintaskbar") == 0) {
		if (strcmp(value, "y") != 0 && strcmp(value, "n") != 0) {
			INFO_ERR("Invalid value: %s\n", option)
			error_code = OPTION_VALUE_INVALID;
		} else
			prm->icon_in_taskbar = value[0];
	} else if (strcmp(name, "winsticky") == 0) {
		if (strcmp(value, "y") != 0 && strcmp(value, "n") != 0) {
			INFO_ERR("Invalid value: %s\n", option)
			error_code = OPTION_VALUE_INVALID;
		} else
			prm->win_sticky = value[0];
	} else if (strcmp(name, "overrideredirect") == 0) {
		if (strcmp(value, "y") != 0 && strcmp(value, "n") != 0) {
			INFO_ERR("Invalid value: %s\n", option)
			error_code = OPTION_VALUE_INVALID;
		} else
			prm->override_redirect = value[0];
	} else if (strcmp(name, "shadow") == 0) {
		if (strcmp(value, "y") != 0 && strcmp(value, "n") != 0) {
			INFO_ERR("Invalid value: %s\n", option)
			error_code = OPTION_VALUE_INVALID;
		} else
			prm->shadow = value[0];
	}
	else if (strcmp(name, "shadowoffset_x") == 0)	/* Should check value here */
		prm->shadow_offset_x = atoi(value);
	else if (strcmp(name, "shadowoffset_y") == 0)	/* Should check value here */
		prm->shadow_offset_y = atoi(value);
	else if (strcmp(name, "shadowfx") == 0)
		prm->shadow_fx = atoi(value);
	else if (strcmp(name, "linedelimiter") == 0)
		str_n_cpy(prm->line_delimiter, value, DELIMITER_MAXLEN);
	else if (strcmp(name, "specialchars") == 0) {
		if (strcmp(value, "y") != 0 && strcmp(value, "n") != 0) {
			INFO_ERR("Invalid value: %s\n", option)
			error_code = OPTION_VALUE_INVALID;
		} else
			prm->special_chars = value[0];
	} else if (strcmp(name, "newpgchar") == 0)
		prm->new_page_char = value[0];
	else if (strcmp(name, "tabchar") == 0)
		prm->tab_char = value[0];
	else if (strcmp(name, "rssrefresh") == 0)
		prm->rss_refresh = atoi(value);
	else if (strcmp(name, "revsc") == 0) {
		if (strcmp(value, "y") != 0 && strcmp(value, "n") != 0) {
			INFO_ERR("Invalid value: %s\n", option)
			error_code = OPTION_VALUE_INVALID;
		} else
			prm->reverse_sc = value[0];
	} else if (strcmp(name, "feedtitle") == 0) {
		if (strcmp(value, "y") != 0 && strcmp(value, "n") != 0) {
			INFO_ERR("Invalid value: %s\n", option)
			error_code = OPTION_VALUE_INVALID;
		} else
			prm->feed_title = value[0];
	} else if (strcmp(name, "feedtitledelimiter") == 0)
		str_n_cpy(prm->feed_title_delimiter, value, DELIMITER_MAXLEN);
	else if (strcmp(name, "itemtitle") == 0) {
		if (strcmp(value, "y") != 0 && strcmp(value, "n") != 0) {
			INFO_ERR("Invalid value: %s\n", option)
			error_code = OPTION_VALUE_INVALID;
		} else
			prm->item_title = value[0];
	} else if (strcmp(name, "itemtitledelimiter") == 0)
		str_n_cpy(prm->item_title_delimiter, value, DELIMITER_MAXLEN);
	else if (strcmp(name, "itemdescription") == 0) {
		if (strcmp(value, "y") != 0 && strcmp(value, "n") != 0) {
			INFO_ERR("Invalid value: %s\n", option)
			error_code = OPTION_VALUE_INVALID;
		} else
			prm->item_description = value[0];
	} else if (strcmp(name, "itemdescriptiondelimiter") == 0)
		str_n_cpy(prm->item_description_delimiter, value, DELIMITER_MAXLEN);
	else if (strcmp(name, "nitemsperfeed") == 0)
		prm->n_items_per_feed = atoi(value);
	else /*if (strcmp(name, "markitemaction") == 0) {
		if (strcmp(value, "h") != 0 && strcmp(value, "c") != 0 \
					&& strcmp(value, "n") != 0) {
			INFO_ERR("Invalid value: %s\n", option)
			error_code = OPTION_VALUE_INVALID;
		} else
			prm->mark_item_action = value[0];
	} else*/ if (strcmp(name, "rmtags") == 0) {
		if (strcmp(value, "y") != 0 && strcmp(value, "n") != 0) {
			INFO_ERR("Invalid value: %s\n", option)
			error_code = OPTION_VALUE_INVALID;
		} else
			prm->strip_html_tags = value[0];
	} else if (strcmp(name, "uppercasetext") == 0) {
		if (strcmp(value, "y") != 0 && strcmp(value, "n") != 0) {
			INFO_ERR("Invalid value: %s\n", option)
			error_code = OPTION_VALUE_INVALID;
		} else
			prm->upper_case_text = value[0];

	} else if (strcmp(name, "homefeed") == 0)
		str_n_cpy(prm->homefeed, value, FILE_NAME_MAXLEN);
	else if (strcmp(name, "openlinkcmd") == 0)
		str_n_cpy(prm->open_link_cmd, value, FILE_NAME_MAXLEN);
	else if (strcmp(name, "openlinkargs") == 0)
		str_n_cpy(prm->open_link_args, value, FILE_NAME_MAXLEN);
	else if (strcmp(name, "clock") == 0) {
		if (strcmp(value, "l") != 0 && strcmp(value, "r") != 0 \
					&& strcmp(value, "n") != 0) {
			INFO_ERR("Invalid value: %s\n", option)
			error_code = OPTION_VALUE_INVALID;
		} else
			prm->clock = value[0];
	} else if (strcmp(name, "clocksec") == 0) {
		if (strcmp(value, "y") != 0 && strcmp(value, "n") != 0) {
			INFO_ERR("Invalid value: %s\n", option)
			error_code = OPTION_VALUE_INVALID;
		} else
			prm->clock_sec = value[0];
	} else if (strcmp(name, "clock12h") == 0) {
		if (strcmp(value, "y") != 0 && strcmp(value, "n") != 0) {
			INFO_ERR("Invalid value: %s\n", option)
			error_code = OPTION_VALUE_INVALID;
		} else
			prm->clock_12h = value[0];
	} else if (strcmp(name, "clockdate") == 0) {
		if (strcmp(value, "y") != 0 && strcmp(value, "n") != 0) {
			INFO_ERR("Invalid value: %s\n", option)
			error_code = OPTION_VALUE_INVALID;
		} else
			prm->clock_date = value[0];
	} else if (strcmp(name, "clockaltdateform") == 0) {
		if (strcmp(value, "y") != 0 && strcmp(value, "n") != 0) {
			INFO_ERR("Invalid value: %s\n", option)
			error_code = OPTION_VALUE_INVALID;
		} else
			prm->clock_alt_date_form = value[0];
	} else if (strcmp(name, "clockfontname") == 0)
		str_n_cpy(c_font_name, value, FONT_NAME_MAXLEN);
	else if (strcmp(name, "clockfontsize") == 0)
		str_n_cpy(c_font_size, value, FONT_SIZE_MAXLEN);
	else if (strcmp(name, "clockfgcolor") == 0) {
		if (!get_gdk_color_from_hexstr(value, &prm->clock_fg_color,
					&prm->clock_fg_color_alpha)) {
			get_gdk_color_from_hexstr(CFGCOLOR, &prm->clock_fg_color,
					&prm->clock_fg_color_alpha);
			error_code = OPTION_VALUE_INVALID;
		}
	} else if (strcmp(name, "clockbgcolor") == 0) {
		if (!get_gdk_color_from_hexstr(value, &prm->clock_bg_color,
					&prm->clock_bg_color_alpha)) {
			get_gdk_color_from_hexstr(CBGCOLOR, &prm->clock_bg_color,
					&prm->clock_bg_color_alpha);
			error_code = OPTION_VALUE_INVALID;
		}
	} else if (strcmp(name, "setclockgradientbg") == 0) {
		if (strcmp(value, "y") != 0 && strcmp(value, "n") != 0) {
			INFO_ERR("Invalid value: %s\n", option)
			error_code = OPTION_VALUE_INVALID;
		} else
			prm->set_clock_gradient_bg = value[0];
	} else if (strcmp(name, "clockbgcolor2") == 0) {
		if (!get_gdk_color_from_hexstr(value, &prm->clock_bg_color2,
						&prm->clock_bg_color2_alpha)) {
			get_gdk_color_from_hexstr(BGCOLOR, &prm->clock_bg_color2,
						&prm->clock_bg_color2_alpha);
			error_code = OPTION_VALUE_INVALID;
		}
	} else if (strcmp(name, "disablepopups") == 0) {
		if (strcmp(value, "y") != 0 && strcmp(value, "n") != 0) {
			INFO_ERR("Invalid value: %s\n", option)
			error_code = OPTION_VALUE_INVALID;
		} else
			prm->disable_popups = value[0];
	} else if (strcmp(name, "pauseonmouseover") == 0) {
		if (strcmp(value, "y") != 0 && strcmp(value, "n") != 0) {
			INFO_ERR("Invalid value: %s\n", option)
			error_code = OPTION_VALUE_INVALID;
		} else
			prm->pause_on_mouseover = value[0];
	} else if (strcmp(name, "disableleftclick") == 0) {
		if (strcmp(value, "y") != 0 && strcmp(value, "n") != 0) {
			INFO_ERR("Invalid value: %s\n", option)
			error_code = OPTION_VALUE_INVALID;
		} else
			prm->disable_leftclick = value[0];
	} else if (strcmp(name, "mousewheelaction") == 0) {
		if (strcmp(value, "s") != 0 && strcmp(value, "f") != 0 \
					&& strcmp(value, "n") != 0) {
			INFO_ERR("Invalid value: %s\n", option)
			error_code = OPTION_VALUE_INVALID;
		} else
			prm->mouse_wheel_action = value[0];
	} else if (strcmp(name, "sfeedpickerautoclose") == 0) {
		if (strcmp(value, "y") != 0 && strcmp(value, "n") != 0) {
			INFO_ERR("Invalid value: %s\n", option)
			error_code = OPTION_VALUE_INVALID;
		} else
			prm->sfeedpicker_autoclose = value[0];
	} else if (strcmp(name, "enablefeedordering") == 0) {
		if (strcmp(value, "y") != 0 && strcmp(value, "n") != 0) {
			INFO_ERR("Invalid value: %s\n", option)
			error_code = OPTION_VALUE_INVALID;
		} else
			prm->enable_feed_ordering = value[0];
	/* Params in connection settings win */
	} else if (strcmp(name, "useauth") == 0) {
		if (strcmp(value, "y") != 0 && strcmp(value, "n") != 0) {
			INFO_ERR("Invalid value: %s\n", option)
			error_code = OPTION_VALUE_INVALID;
		} else {
			prm->use_authentication = value[0];
			set_use_authentication(value[0] == 'y' ? TRUE : FALSE);
		}
	} else if (strcmp(name, "user") == 0) {
		str_n_cpy(prm->user, value, USER_MAXLEN);
		str_n_cpy(get_http_auth_user(), value, USER_MAXLEN);
	/* This option is never saved */
	} else if (strcmp(name, "psw") == 0)
		str_n_cpy(get_http_auth_psw(), value, PSW_MAXLEN);
	else if (strcmp(name, "useproxy") == 0) {
		if (strcmp(value, "y") != 0 && strcmp(value, "n") != 0) {
			INFO_ERR("Invalid value: %s\n", option)
			error_code = OPTION_VALUE_INVALID;
		} else {
			prm->use_proxy = value[0];
			set_use_proxy(value[0] == 'y' ? TRUE : FALSE);
		}
	} else if (strcmp(name, "proxyhost") == 0) {
		str_n_cpy(prm->proxy_host, value, PROXY_HOST_MAXLEN);
		str_n_cpy(get_proxy_host(), value, PROXY_HOST_MAXLEN);
	} else if (strcmp(name, "proxyport") == 0) {
		str_n_cpy(prm->proxy_port, value, PROXY_PORT_MAXLEN);
		str_n_cpy(get_proxy_port(), value, PROXY_PORT_MAXLEN);
	} else if (strcmp(name, "useproxyauth") == 0) {
		if (strcmp(value, "y") != 0 && strcmp(value, "n") != 0) {
			INFO_ERR("Invalid value: %s\n", option)
			error_code = OPTION_VALUE_INVALID;
		} else {
			prm->use_proxy_authentication = value[0];
			set_use_proxy_auth(value[0] == 'y' ? TRUE : FALSE);
		}
	} else if (strcmp(name, "proxyuser") == 0) {
		str_n_cpy(prm->proxy_user, value, PROXY_USER_MAXLEN);
		str_n_cpy(get_proxy_auth_user(), value, PROXY_USER_MAXLEN);
	/* This option is never saved */
	} else if (strcmp(name, "proxypsw") == 0)
		str_n_cpy(get_proxy_auth_psw(), value, PROXY_PSW_MAXLEN);
	else if (strcmp(name, "connect-timeout") == 0) {
		if (atoi(value) < 1 || atoi(value) > 60) {
			INFO_ERR("Invalid value: %s\n", option)
			error_code = OPTION_VALUE_INVALID;
		} else
			prm->connect_timeout = atoi(value);
	} else if (strcmp(name, "sendrecv-timeout") == 0) {
		if (atoi(value) < 1 || atoi(value) > 60) {
			INFO_ERR("Invalid value: %s\n", option)
			error_code = OPTION_VALUE_INVALID;
		} else
			prm->send_recv_timeout = atoi(value);
	/* Connect params until here */
	/* Following options are only set from command line (and never saved) */
	} else if (strcmp(name, "instance-id") == 0) {
		/* Does nothing but avoid an 'unknown option' warning/error */
	} else if (strcmp(name, "no-ui") == 0) {
		/* Does nothing but avoid an 'unknown option' warning/error */
	} else {
		INFO_ERR("Unknown option: %s\n", option)
		error_code = OPTION_UNKNOWN;
	}
	compact_font(tmp_font, font_name, font_size);
	str_n_cpy(prm->font_name_size, tmp_font, FONT_MAXLEN);
	compact_font(tmp_font, c_font_name, c_font_size);
	str_n_cpy(prm->clock_font_name_size, tmp_font, FONT_MAXLEN);
	return error_code;
}

void save_to_config_file(const Params *prm)
{
	FILE		*conf_fp;
	char		font_name[FONT_NAME_MAXLEN + 1], font_size[FONT_SIZE_MAXLEN + 1];
	char		c_font_name[FONT_NAME_MAXLEN + 1], c_font_size[FONT_SIZE_MAXLEN + 1];
	char		tmp[4 * 1024];

	if ((conf_fp = g_fopen(get_datafile_full_name_from_name(CONFIG_FILE), "wb")) != NULL) {
		split_font(prm->font_name_size, font_name, font_size);
		split_font(prm->clock_font_name_size, c_font_name, c_font_size);
		snprintf(tmp, 4 * 1024, "%s%s%s",
			/* All *string* values are saved inclosed in a pair of ".
			 */
			"-delay=%d\n"
			"-shiftsize=%d\n"
			"-fontname=\"%s\"\n"
			"-fontsize=%d\n"
			"-fgcolor=\"%s\"\n"
			/*"-highlightfgcolor=\"%s\"\n"*/
			"-bgcolor=\"%s\"\n"
			"-setgradientbg=%c\n"
			"-bgcolor2=\"%s\"\n"
			"-disablescreenlimits=%c\n"
			"-win_x=%d\n"
			"-win_y=%d\n"
			"-win_w=%d\n"
			"-win_h=%d\n"
			"-windec=%c\n"
			"-alwaysontop=%c\n"
			"-wintransparency=%d\n"
			"-iconintaskbar=%c\n"
			"-winsticky=%c\n"
			"-overrideredirect=%c\n"
			"-shadow=%c\n"
			"-shadowoffset_x=%d\n"
			"-shadowoffset_y=%d\n"
			"-shadowfx=%d\n"
			"-linedelimiter=\"%s\"\n"
			"-specialchars=%c\n",
			"-newpgchar=%c\n"
			"-tabchar=%c\n"
			"-rssrefresh=%d\n"
			"-revsc=%c\n"
			"-feedtitle=%c\n"
			"-feedtitledelimiter=\"%s\"\n"
			"-itemtitle=%c\n"
			"-itemtitledelimiter=\"%s\"\n"
			"-itemdescription=%c\n"
			"-itemdescriptiondelimiter=\"%s\"\n"
			"-nitemsperfeed=%d\n"
			/*"-markitemaction=%c\n"*/
			"-rmtags=%c\n"
			"-uppercasetext=%c\n"
			"-homefeed=\"%s\"\n"
			"-openlinkcmd=\"%s\"\n"
			"-openlinkargs=\"%s\"\n"
			"-clock=%c\n"
			"-clocksec=%c\n"
			"-clock12h=%c\n"
			"-clockdate=%c\n"
			"-clockaltdateform=%c\n"
			"-clockfontname=\"%s\"\n"
			"-clockfontsize=%d\n"
			"-clockfgcolor=\"%s\"\n"
			"-clockbgcolor=\"%s\"\n"
			"-setclockgradientbg=%c\n"
			"-clockbgcolor2=\"%s\"\n",
			"-disablepopups=%c\n"
			"-pauseonmouseover=%c\n"
			"-disableleftclick=%c\n"
			"-mousewheelaction=%c\n"
			"-sfeedpickerautoclose=%c\n"
			"-enablefeedordering=%c\n"
			"-useauth=%c\n"
			"-user=\"%s\"\n"
			"-useproxy=%c\n"
			"-proxyhost=\"%s\"\n"
			"-proxyport=\"%s\"\n"
			"-useproxyauth=%c\n"
			"-proxyuser=\"%s\"\n"
			"-connect-timeout=%d\n"
			"-sendrecv-timeout=%d\n"
			);
		fprintf(conf_fp, tmp,
			prm->delay,
			prm->shift_size,
			font_name, atoi(font_size),
			get_hexstr_from_gdk_color(&prm->fg_color, &prm->fg_color_alpha),
			/*get_hexstr_from_gdk_color(&prm->highlight_fg_color, &prm->highlight_fg_color_alpha),*/
			get_hexstr_from_gdk_color(&prm->bg_color, &prm->bg_color_alpha),
			prm->set_gradient_bg,
			get_hexstr_from_gdk_color(&prm->bg_color2, &prm->bg_color2_alpha),
			prm->disable_screen_limits,
			prm->win_x,
			prm->win_y,
			prm->win_w,
			prm->win_h,
			prm->windec,
			prm->always_on_top,
			(int)(prm->win_transparency * 10),
			prm->icon_in_taskbar,
			prm->win_sticky,
			prm->override_redirect,
			prm->shadow,
			prm->shadow_offset_x,
			prm->shadow_offset_y,
			prm->shadow_fx,
			prm->line_delimiter,
			prm->special_chars,
			prm->new_page_char,
			prm->tab_char,
			prm->rss_refresh,
			prm->reverse_sc,
			prm->feed_title,
			prm->feed_title_delimiter,
			prm->item_title,
			prm->item_title_delimiter,
			prm->item_description,
			prm->item_description_delimiter,
			prm->n_items_per_feed,
			/*prm->mark_item_action,*/
			prm->strip_html_tags,
			prm->upper_case_text,
			prm->homefeed,
			prm->open_link_cmd,
			prm->open_link_args,
			prm->clock,
			prm->clock_sec,
			prm->clock_12h,
			prm->clock_date,
			prm->clock_alt_date_form,
			c_font_name,
			atoi(c_font_size),
			get_hexstr_from_gdk_color(&prm->clock_fg_color, &prm->clock_fg_color_alpha),
			get_hexstr_from_gdk_color(&prm->clock_bg_color, &prm->clock_bg_color_alpha),
			prm->set_clock_gradient_bg,
			get_hexstr_from_gdk_color(&prm->clock_bg_color2, &prm->clock_bg_color2_alpha),
			prm->disable_popups,
			prm->pause_on_mouseover,
			prm->disable_leftclick,
			prm->mouse_wheel_action,
			prm->sfeedpicker_autoclose,
			prm->enable_feed_ordering,
			prm->use_authentication,
			prm->user,
			prm->use_proxy,
			prm->proxy_host,
			prm->proxy_port,
			prm->use_proxy_authentication,
			prm->proxy_user,
			prm->connect_timeout,
			prm->send_recv_timeout
			);
		fclose(conf_fp);
	} else
		warning(BLOCK, "Can't save configuration file '%s': %s",
			get_datafile_full_name_from_name(CONFIG_FILE), strerror(errno));
}

/*
 * Options colors are coded as 4 hexa values -> #rrggbbaa
 */
int get_gdk_color_from_hexstr(const char *str, GdkColor *color, guint16 *color_alpha)
{
	char tmp[3], *tailptr;
	int errflag = 0;

	if (strlen(str) != 9 && str[0] != '#')
		errflag = 1;
	else {
		str_n_cpy(tmp, str + 1, 2);
		color->red = strtoul(tmp, &tailptr, 16) * 0x100;
		if (tailptr == tmp)
		errflag = 1;

		str_n_cpy(tmp, str + 3, 2);
		color->green = strtoul(tmp, &tailptr, 16) * 0x100;
		if (tailptr == tmp)
			errflag = 1;

		str_n_cpy(tmp, str + 5, 2);
		color->blue = strtoul(tmp, &tailptr, 16) * 0x100;
		if (tailptr == tmp)
			errflag = 1;

		str_n_cpy(tmp, str + 7, 2);
		*color_alpha = strtoul(tmp, &tailptr, 16) * 0x100;
		if (tailptr == tmp)
			errflag = 1;
	}
	if (errflag != 0) {
		INFO_ERR("Invalid color: %s\n", str)
		return FALSE;
	} else
		return TRUE;
}

/*
 * Options colors are coded as 4 hexa values -> #rrggbbaa
 * *** allow up to 64 simultaneous calls ***
 */
const char *get_hexstr_from_gdk_color(const GdkColor *color, const guint16 *color_alpha)
{
	static char	str[64][10];
	static int	count = -1;

	count++;
	count &= 63;
	snprintf(str[count], 10, "#%02x%02x%02x%02x", color->red / 256, color->green / 256,
		color->blue / 256, *color_alpha / 256);
	if (strlen(str[count]) != 9)
		return NULL;
	else
		return (const  char *)str[count];
}

/*
 * Adjust size in font_name_and_size str by computing font size from required height
 * (str must be able to store FONT_MAXLEN chars.) Does nothing if height <= 0.
 */
void adjust_font_size_to_rq_height(char *font_name_and_size, int height)
{
	char	f_name[FONT_NAME_MAXLEN + 1];
	char	f_size[FONT_SIZE_MAXLEN + 1];

	if (height > 0) {
		split_font(font_name_and_size, f_name, f_size);
		snprintf(f_size, FONT_SIZE_MAXLEN + 1, "%3d", get_font_size_from_layout_height(height, f_name));
		/* Font size can't be > FONT_MAXSIZE */
		if (atoi(f_size) > FONT_MAXSIZE)
			snprintf(f_size, FONT_SIZE_MAXLEN + 1, "%3d", FONT_MAXSIZE);
		compact_font(font_name_and_size, f_name, f_size);
	} else if (height == 0)
		DEBUG_INFO("Required height = 0\n")
	else
		INFO_ERR("%s() error: Required height < 0\n", __func__)
}

int get_font_size_from_font_name_size(const char *font_name_size)
{
	char font_name[FONT_NAME_MAXLEN + 1], font_size[FONT_SIZE_MAXLEN + 1];

	split_font(font_name_size, font_name, font_size);
	return atoi(font_size);
}

/*
 * Space needed for font_name: FONT_NAME_MAXLEN + 1 bytes / font_size: FONT_SIZE_MAXLEN + 1 bytes
 */
void split_font(const char *font_name_size, char *font_name, char *font_size)
{
	char	tmp[FONT_MAXLEN + 1];
	int	i;

	if (font_name_size[0] == '\0') {
		font_name[0] = '\0';
		font_size[0] = '\0';
	} else {
		str_n_cpy(tmp, font_name_size, FONT_MAXLEN);
		for (i = strlen(tmp) - 1; i > 0 ; i--)
			if (tmp[i] == ' ') {
				tmp[i] = '\0';
				break;
			}
		str_n_cpy(font_name, tmp, FONT_NAME_MAXLEN);
		str_n_cpy(font_size, tmp + strlen(font_name) + 1, FONT_SIZE_MAXLEN);
	}
}

/*
 * Space needed for font_name_size: FONT_NAME_MAXLEN + 1 space + FONTSIZENMAXLEN + 1 bytes
 */
void compact_font(char *font_name_size, const char *font_name, const char *font_size)
{
	if (font_name[0] == '\0' && font_size[0] == '\0') {
		font_name_size[0] = '\0';
	} else {
		str_n_cpy(font_name_size, font_name, FONT_NAME_MAXLEN);
		str_n_cat(font_name_size, " ", 1);
		str_n_cat(font_name_size, font_size, FONT_SIZE_MAXLEN);
	}
}
