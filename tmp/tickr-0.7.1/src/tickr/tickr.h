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

#ifndef INC_TICKR_H
#define INC_TICKR_H

#define APP_NAME		"Tickr"
#define APP_CMD			"tickr"
#define APP_V_NUM		"0.7.1"
#define COPYRIGHT_STR		"Copyright (C) 2009-2021 Emmanuel Thomas-Maurin"
#define WEBSITE_URL		"https://www.open-tickr.net"
#define DOWNLOAD_URL		WEBSITE_URL "/download.php"
#define SUPPORT_URL		WEBSITE_URL "/help.php"
#define DONATE_WITH_PAYPAL_URL	WEBSITE_URL "/go_to_paypal.php"
#define SUPPORT_EMAIL_ADDR	"manutm007@gmail.com"
#define CHECK4UPDATES_URL	WEBSITE_URL "/app_version_visible.php"
#define CHECK4UPDATES_ID_STR	"LSVN="		/* Stands for 'Last Stable Version Number = ' */

/*#define ABOUT_WIN_QUOTE \
	"We've been told that a million monkeys banging on a\n"\
	"million keyboards will eventually reproduce the entire\n"\
	"works of W. Shakespeare. Now, thanks to the Internet,\n"\
	"we know this is <b>*not*</b> true."*/

#define _GNU_SOURCE
#define _ISOC99_SOURCE

#undef VERBOSE_OUTPUT		/* Always defined in libetm, so always first undefine ... */
#define VERBOSE_OUTPUT		/* Then define again */
/*#define DEBUG_OUTPUT*/

/* For info_win_no_block(..., timeout) */
#define INFO_WIN_WAIT_POS	GTK_WIN_POS_CENTER_ON_PARENT
#define INFO_WIN_WAIT_TIMEOUT	2000		/* in ms */

/* M_S_MOD -> true in multiple selection mode / false otherwise */
#define M_S_MOD			(get_ticker_env()->selection_mode == MULTIPLE)

#define CONNECT_FAIL_MAX	5
#define CONNECT_FAIL_TIMEOUT	1

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/stat.h>
#ifndef G_OS_WIN32
#include <signal.h>
#endif
#include <gtk/gtk.h>				/* Moved here because we need G_OS_WIN32 check early */
#include <gdk/gdkkeysyms.h>
#ifndef G_OS_WIN32
#  include <glib-2.0/glib/gstdio.h>
#  include <gdk/gdkx.h>
#else
#  include <../glib-2.0/glib/gstdio.h>
#endif
#ifndef G_OS_WIN32
#  include <grp.h>
#  include <pwd.h>
#  include <sys/socket.h>
#  include <netdb.h>
#  include <arpa/inet.h>
#  include <sys/select.h>
#else
#  define WINVER		0x0501		/* Win version = XP (5.1) or higher */
#  define _WIN32_WINNT		0x0501
#  include <winsock2.h>
#  include <ws2tcpip.h>
#  include <rpcdce.h>
#  include <iphlpapi.h>
#  include <winreg.h>
#  include <shlobj.h>
#endif
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <gnutls/gnutls.h>
#include <fribidi/fribidi.h>
#include "../libetm-0.5.0/libetm.h"
#include "tickr_list.h"
#include "tickr_error.h"
#include "tickr_tls.h"

#define RESOURCE_ERROR_SSM_QUIT	FALSE		/* To quit or not if not found or invalid resource - only in single selection mode */
#define OPTION_ERROR_QUIT	FALSE		/* To quit or not if unknown or invalid option(s) */

#define DRWA_WIDTH_MIN		100
#define DRWA_HEIGHT_MIN		10
#define TAB_SIZE		8
#define FILE_NAME_MAXLEN	(2 * 1024 - 1)	/* Apply to both FULL FILE NAMES and ***URLs*** */
#define	URL_SCHEME_MAXLEN	15
#define	PORT_STR_MAXLEN		15
#define FONT_NAME_MAXLEN	64
#define FONT_SIZE_MAXLEN	3
#define FONT_MAXLEN		FONT_NAME_MAXLEN + 1 + FONT_SIZE_MAXLEN
#define DELIMITER_MAXLEN	80
#define OPTION_NAME_MAXLEN	31
#define OPTION_VALUE_MAXLEN	MAX(MAX(FILE_NAME_MAXLEN, FONT_MAXLEN), DELIMITER_MAXLEN)
#define OPTION_MAXLEN		OPTION_NAME_MAXLEN + OPTION_VALUE_MAXLEN + 4	/* -name="value" */
#define N_OPTION_MAX		128		/* Max number of options */
#define N_URL_MAX		1024		/* Max number of URLs */
#define NFEEDLINKANDOFFSETMAX	999		/* Max number of "open-able" links per feed (3 digits: 001 -> 999) */
#define LINK_TAG_CHAR		((char)(1))	/* char (ascii 01) used internally for item links, removed from feed text */
#define ITEM_TITLE_TAG_CHAR	((char)(2))	/* char (ascii 02) used internally for item titles, --- */
#define ITEM_DES_TAG_CHAR	((char)(3))	/* char (ascii 03) used internally for item descriptions, --- */
/*#define ENCODING_STR_MAX_LEN	64*/
/*
 * Used in tickr_main.c, tickr_render.c and tickr_feedparser.c
 * Confusingly, L to R scrolling (ie reverse) is for languages written/read from R to L
 */
#define REVERSE_SCROLLING	(get_params()->reverse_sc == 'y')	/* = L to R scrolling for R to L languages*/
#define STANDARD_SCROLLING	(!REVERSE_SCROLLING)			/* = R to L scrolling for L to R languages*/

#define N_RENDER_STR_MAX	256		/* Max number of strings from a stream that can be rendered to cairo surfaces */
#define TMPSTR_SIZE		(2 * 1024 - 1)	/* Used for most tmp strings - must be large enough */
#define FGETS_STR_MAXLEN	(8 * 1024 * 1024)	/* 8 MiB for every fgets() line */

#ifndef G_OS_WIN32
#  define SEPARATOR_CHAR	'/'
#else
#  define SEPARATOR_CHAR	'\\'
#endif

#define FEED_TITLE_MAXLEN	60
/* 7-bit ascii - STH_CHAR and STH_STR ***must*** match */
#define TITLE_TAG_CHAR		'>'
#define TITLE_TAG_STR		">"
#define SELECTED_URL_CHAR	'*'
#define SELECTED_URL_STR	"*"
#define UNSELECTED_URL_CHAR	'-'
#define UNSELECTED_URL_STR	"-"

#define XPIXMAP_MAXWIDTH	(32 * 1024 - 1)	/* 32 K - 1 = max supported width for X pixmaps
						 * (not 64 K - 1 as sometimes incorrectly stated) */
#define FONT_MAXSIZE		200

#define ARBITRARY_TASKBAR_HEIGHT 25		/* Could/should be get from window manager ? */

/*
 * === File names, paths and dirs stuff ===
 *
 * Linux version:	/usr/bin/
 *			/usr/share/APP_CMD/
 *			/usr/share/APP_CMD/pixmpas/
 *			/home/<user_name>/.APP_CMD/
 *
 * Win32 version:	C:\Program Files\APP_NAME\	(also location for pixmaps)
 *			C:\...\Application Data\APP_NAME\
 */
#ifndef G_OS_WIN32
#  define TICKR_DIR_NAME	"." APP_CMD
#else
#  define TICKR_DIR_NAME	APP_NAME
#endif
#define CONFIG_FILE		APP_CMD "-conf"
#define URL_LIST_FILE		APP_CMD "-url-list"
#define RESOURCE_DUMP		APP_CMD "-resrc-dump.xml"
#define XML_DUMP		APP_CMD "-xml-dump"
#define XML_DUMP_EXTRA		APP_CMD "-xml-dump-extra"
#define TMP_FILE		"tmp"
/*#define MARKED_ITEMS_FILE	APP_CMD "-marked-items"*/
/*
 * STD_OUT = stdout on Linux / text file on win32
 * STD_ERR = stderr on Linux / text file on win32
 * (defined in libetm/misc.h)
 */
#ifdef G_OS_WIN32
#  define STDOUT_FILENAME1	"stdout1.txt"
#  define STDOUT_FILENAME2	"stdout2.txt"
#  define STDERR_FILENAME1	"stderr1.txt"
#  define STDERR_FILENAME2	"stderr2.txt"
#endif
#ifndef G_OS_WIN32
#  define INSTALL_PATH		"/usr/share/" APP_CMD
#  define IMAGES_PATH		INSTALL_PATH "/pixmaps"
#else
#  define IMAGES_PATH		APP_NAME	/* Actually not a path but a dir name */
#endif
#define TICKR_ICON		APP_CMD "-icon.png"
#define TICKR_LOGO		APP_CMD "-logo.png"
#define RSS_ICON		APP_CMD "-rss-icon.png"

/*
 * Default config values
 */
#define DELAY			8
#define SHIFTSIZE		1
#ifndef G_OS_WIN32
#  define FONTNAME		"Sans"		/* Ubuntu */
#else
#  define FONTNAME		"Arial Unicode MS"
#endif
#define FONTSIZE		"12"		/* 14 */
#define FGCOLOR			"#ffffffff"
/*#define HIGHLIGHTFGCOLOR	"#ff0000ff"	/ Red ???? */
#define BGCOLOR			"#40404080"
#define SETGRADIENTBG		'y'
#define BGCOLOR2		"#20202080"
#define DISABLESCREENLIMITS	'n'
#define WIN_X			0
#define WIN_Y			0
/*#define WIN_W			1024*/		/* Unused, we use get_ticker_env()->screen_w instead */
#define WIN_H			0		/* If = 0, determined by font size */
/* These arbitrary values are supposed to be big enough */
#define WIN_MAX_X		15000
#define WIN_MAX_Y		10000
#define WIN_MAX_W		15000
#define WINDEC			'n'
#define ALWAYSONTOP		'n'
#ifndef G_OS_WIN32
#  define WINTRANSPARENCY	1.0
#else
#  define WINTRANSPARENCY	0.8
#endif
#define ICONINTASKBAR		'y'
#define WINSTICKY		'n'
#define OVERRIDE_REDIRECT	'n'
#define SHADOW			'y'
#define SHADOWOFFSET_X		4
#define SHADOWOFFSET_Y		2
#define SHADOWFX		2
#define LINEDELIMITER		" "
#define SPECIALCHARS		'n'
#define NEWPG			'`'
#define TABCH			'~'
#define RSSREFRESH		15
#define REVERSESCROLLING	'n'
#define FEEDTITLE		'n'
#define FEEDTITLEDELIMITER	"                "
#define ITEMTITLE		'y'
#define ITEMTITLEDELIMITER	"        |        "
#define ITEMDESCRIPTION		'n'
#define ITEMDESCRIPTIONDELIMITER "        ***        "
#define NITEMSPERFEED		5
/*#define MARKITEMACTION		'c'*/
#define STRIPHTMLTAGS		'y'
#define UPPERCASETEXT		'n'
#define HOMEFEED		"http://rss.cnn.com/rss/edition.rss"
#ifndef G_OS_WIN32
#  define OPENLINKCMD		"x-www-browser"/*"sensible-browser"*/	/* Get default browser on Linux */
#else
#  define OPENLINKCMD		""
#endif
#define OPENLINKARGS		""
#define CLOCK			'n'
#define CLOCKSEC		'n'
#define CLOCK12H		'y'
#define CLOCKDATE		'n'
#define CLOCKALTDATEFORM	'n'
#define CFONTNAME		FONTNAME
#define CFONTSIZE		FONTSIZE
#define CFGCOLOR		FGCOLOR
#define CBGCOLOR		BGCOLOR
#define SETCLOCKGRADIENTBG	SETGRADIENTBG
#define CBGCOLOR2		BGCOLOR2
#define DISABLEPOPUPS		'n'
#define PAUSEONMOUSEOVER	'y'
#define DISABLELEFTCLICK	'n'
#define MOUSEWHEELACTION	'f'
#ifndef G_OS_WIN32
#  define SFEEDPICKERAUTOCLOSE	'y'
#else
#  define SFEEDPICKERAUTOCLOSE	'n'
#endif
#define ENABLEFEEDORDERING	'n'
#define USEAUTHENTICATION	'n'
#define USER			""
#define USEPROXY		'n'
#define PROXYHOST		""
#define PROXYPORT		"8080"
#define USEPROXYAUTHENTICATION	'n'
#define PROXYUSER		""
#define CONNECT_TIMEOUT		5
#define SENDRECV_TIMEOUT	1

/* typedef enum's */
typedef enum {
	SINGLE,
	MULTIPLE
} _selection_mode;

typedef enum {
	RESRC_TYPE_UNDETERMINED,
	RESRC_UNSPECIFIED,
	RESRC_URL,				/* Will be fetched and 'xml-parsed' */
	RESRC_FILE				/* Will be processed as non-xml text file */
} _resrc_type;

typedef enum {
	RSS_FORMAT_UNDETERMINED,
	RSS_1_0,				/* Should use RDF or RSS_RDF instead ? */
	RSS_2_0,
	RSS_ATOM
} _rss_format;

typedef enum {
	TIME_RESET,
	TIME_CHECK
} check_time_mode;

typedef enum {
	INFO,
	INFO_WARNING,
	INFO_ERROR
} info_type;

typedef enum {
	WIN_WITH_PROGRESS_BAR_OPEN,
	WIN_WITH_PROGRESS_BAR_PULSE,
	WIN_WITH_PROGRESS_BAR_CLOSE
} win_with_progress_bar_mode;

typedef enum {
	AUTH_PAGE,
	PROXY_PAGE
} connection_settings_page;

/* Predefined GTK_RESPONSE_ are < 0, those ones are app level defined and >= 0 (>= 100 actually) */
typedef enum {
	GTK_RESPONSE_CANCEL_CLOSE = 100,

	/* tickr_feedpicker.c */
	GTK_RESPONSE_SELECT_ALL, GTK_RESPONSE_UNSELECT_ALL, GTK_RESPONSE_CLEAR_RANKING, GTK_RESPONSE_TOP,
	GTK_RESPONSE_CURRENT, GTK_RESPONSE_HOME, GTK_RESPONSE_REMOVE, GTK_RESPONSE_ADD_UPD, GTK_RESPONSE_SINGLE,
	GTK_RESPONSE_SELECTION,

	/* tickr_prefwin.c */
	GTK_RESPONSE_RESET, GTK_RESPONSE_FULL_SETTINGS, GTK_RESPONSE_CONN_SETTINGS
} gtk_custom_response;

typedef enum {
	/* tickr_main.c and tickr_prefwin.c */
	DO_NEXT_NOTHING,
	DO_NEXT_REOPEN,
	DO_NEXT_OPEN_FULL_SETTINGS,
	DO_NEXT_OPEN_CONN_SETTINGS
} next_action;

/* typedef struct's */
/*
 * === Useful graphical (and other) ticker 'environment' values ===
 */
typedef struct {
	GdkScreen	*screen;			/* GDK screen for win */
	GdkVisual	*visual;
	int		screen_w, screen_h;		/* Screen width & height */
	int		depth;				/* Screen depth */
	GtkWidget 	*win;				/* Top level window */
	GtkWidget	*drw_a, *drwa_clock;		/* Drawing areas (main/clock) */
	int		drwa_width, drwa_height;	/* Main drawing area dimensions */
	int		drwa_clock_width;		/* Clock drawing area width */
	cairo_surface_t	*c_surf;			/* Cairo image surface onto which text is rendered */
	int 		surf_width, surf_height;	/* Cairo image surface dimensions */
	int		shift_counter;
	char		active_link[FILE_NAME_MAXLEN + 1];
	int		mouse_x_in_drwa;
	/* These flags are checked (mainly) by shift2left_callback() */
	zboolean	suspend_rq;			/* Request for doing nothing */
	zboolean	compute_rq;			/* Request for (re)computing everything (surface and win) */
	zboolean	reload_rq;			/* Request for (re)loading resource */
	_selection_mode	selection_mode;			/* default = MULTIPLE, set to SINGLE internally if resource
							 * is a command line argument or if selection is unavailable/empty
							 * or after explicitely picking a single feed */
	zboolean	feed_fully_rendered;		/* Used when a stream is rendered to several cairo surfaces,
							 * set/checked only in render_stream_to_surface(), stream_to_htext()
							 * and shift2left_callback() */
	zboolean	https_support;
} TickerEnv;

typedef struct {
	char		url[FILE_NAME_MAXLEN + 1];
	int		offset_in_surface;
/* TODO: check/complete this
	char		item_title[FILE_NAME_MAXLEN + 1];
	char		GUUI[FILE_NAME_MAXLEN + 1];*/
} FeedLinkAndOffset;

/*
 * === Resource is the current feed/file being fetched, processed and displayed ===
 */
typedef struct {
	_resrc_type	type;					/* URL (including local URL) or text file */
	char		id[FILE_NAME_MAXLEN + 1];		/* URL (including local URL) or full path/file name */
	char		orig_url[FILE_NAME_MAXLEN + 1];		/* In case of HTTP redirects */
	/* Apply only to URL/RSS */
	_rss_format	format;					/* RSS 1.0, RSS 2.0, or Atom */
	char		feed_title[FEED_TITLE_MAXLEN + 1];
	char		resrc_dump[FILE_NAME_MAXLEN + 1];	/* Downloaded resource (= text file) */
	char		xml_dump[FILE_NAME_MAXLEN + 1];		/* Output of XML/RSS processing of downloaded resource */
	FILE		*fp;					/* Stream to be read (xml_dump = file name) */
	FeedLinkAndOffset link_and_offset[NFEEDLINKANDOFFSETMAX];
	char		xml_dump_extra[FILE_NAME_MAXLEN + 1];
	FILE		*fp_extra;				/* Stream with extra info: item titles (/ descriptions) generated
								 * when ticker displays only descriptions (/ titles) */
	int		rss_ttl;
} Resource;

#define USER_MAXLEN		63
#define PSW_MAXLEN		63
#define AUTH_STR_MAXLEN		127

/* TODO: Do we need both these auth/proxy structs and auth/proxy params ? redundant - check this out */
typedef struct {
	zboolean	use_authentication;
	char		user[USER_MAXLEN + 1];
	char		psw[PSW_MAXLEN + 1];
	char		auth_str[AUTH_STR_MAXLEN + 1];
} Authentication;

#define PROXY_HOST_MAXLEN	127
#define PROXY_PORT_MAXLEN	PORT_STR_MAXLEN
#define PROXY_USER_MAXLEN	63
#define PROXY_PSW_MAXLEN	63
#define PROXY_AUTH_STR_MAXLEN	127
#define PROXY_STR_MAXLEN	255

typedef struct {
	zboolean	use_proxy;
	char		host[PROXY_HOST_MAXLEN + 1];
	char		port[PROXY_PORT_MAXLEN + 1];
	char		proxy_str[PROXY_STR_MAXLEN + 1];
	int		use_proxy_authentication;
	char		user[PROXY_USER_MAXLEN + 1];
	char		psw[PROXY_PSW_MAXLEN + 1];
	char		auth_str[PROXY_AUTH_STR_MAXLEN + 1];
} Proxy;

typedef struct {
	int		delay;
	int		shift_size;
	char		font_name_size[FONT_MAXLEN + 1];
	GdkColor	fg_color;
	guint16		fg_color_alpha;
	/*GdkColor	highlight_fg_color;
	guint16		highlight_fg_color_alpha;*/
	GdkColor	bg_color;
	guint16		bg_color_alpha;
	char		set_gradient_bg;
	GdkColor	bg_color2;
	guint16		bg_color2_alpha;
	char		disable_screen_limits;
	int		win_x;
	int		win_y;
	int		win_w;
	int		win_h;
	char		windec;
	char		always_on_top;
	double		win_transparency;	/* 0.1 -> 1.0 */
	char		icon_in_taskbar;
	char		win_sticky;
	char		override_redirect;
	char		shadow;
	int		shadow_offset_x;
	int		shadow_offset_y;
	int		shadow_fx;
	char		line_delimiter[DELIMITER_MAXLEN + 1];
	char		special_chars;
	char		new_page_char;
	char		tab_char;
	int		rss_refresh;
	char		reverse_sc;		/* Reverse scrolling (= L to R) */
	char		feed_title;
	char		feed_title_delimiter[DELIMITER_MAXLEN + 1];
	char		item_title;
	char		item_title_delimiter[DELIMITER_MAXLEN + 1];
	char		item_description;
	char		item_description_delimiter[DELIMITER_MAXLEN + 1];
	/* POSSIBLE NEW FEATURE
	char		feed_delimiter[DELIMITER_MAXLEN + 1];*/
	/* (until here) */
	int		n_items_per_feed;
	/*char		mark_item_action;	/ (h) hide / c (color = highlight) / n (none) */
	char		strip_html_tags;
	char		upper_case_text;
	char		homefeed[FILE_NAME_MAXLEN + 1];
	char		open_link_cmd[FILE_NAME_MAXLEN + 1];
	char		open_link_args[FILE_NAME_MAXLEN + 1];
	char		clock;
	char		clock_sec;
	char		clock_12h;
	char		clock_date;
	char		clock_alt_date_form;	/* Use alternative date format, ie "Mon 01 Jan" instead of "Mon Jan 01" */
	char		clock_font_name_size[FONT_MAXLEN + 1];
	GdkColor	clock_fg_color;
	guint16		clock_fg_color_alpha;
	GdkColor	clock_bg_color;
	guint16		clock_bg_color_alpha;
	char		set_clock_gradient_bg;
	GdkColor	clock_bg_color2;
	guint16		clock_bg_color2_alpha;
	char		disable_popups;
	char		pause_on_mouseover;
	char		disable_leftclick;
	char		mouse_wheel_action;	/* Acts on s (speed) / f (feed) / n (none) */
	char		sfeedpicker_autoclose;
	char		enable_feed_ordering;
	/*char		alt_encoding[ENCODING_STR_MAX_LEN + 1];*/
/* TODO: Do we need both these auth/proxy structs and auth/proxy params ? redundant - check this out */
	char		use_authentication;
	char		user[USER_MAXLEN + 1];
	char		use_proxy;
	char		proxy_host[PROXY_HOST_MAXLEN + 1];
	char		proxy_port[PROXY_PORT_MAXLEN + 1];
	char		use_proxy_authentication;
	char		proxy_user[PROXY_USER_MAXLEN + 1];
	int		connect_timeout;
	int		send_recv_timeout;
} Params;

/* Prototypes for most source files are below */
/* tickr_main.c */
TickerEnv	*get_ticker_env();
Resource	*get_resource();
FList		*get_feed_list();
void		set_feed_list(FList *);
FList		*get_feed_selection();
void		set_feed_selection(FList *);
Params		*get_params();
int		get_instance_id();
char		*get_feed_extra_info(int);
int		easily_link_with_browser();
void		check_main_win_always_on_top();
zboolean	win_params_have_been_changed(Params *, int);
void		free_all();
void		quit_with_cli_info(tickr_error_code);

/* tickr_resource.c */
int		build_feed_selection_from_feed_list();
void		current_feed();
void		first_feed();
void		last_feed();
zboolean	previous_feed();
zboolean	next_feed();
int		load_resource_from_selection(Resource *, FList *);
int		format_resource(Resource *, const char *);
char		*format_resource_str(char *);
char		*convert_str_to_utf8(const char *, const char *);
int		get_stream_contents(FILE *, char **, zboolean);
void		set_tickr_icon_to_dialog(GtkWindow *);
FILE		*open_new_datafile_with_name(const char *, const char *);
char		*get_datafile_full_name_from_name(const char *);
char		*get_imagefile_full_name_from_name(const char *);
char		*get_datadir_full_path();
char		*usr_home_dir();
#ifdef G_OS_WIN32
const char	*get_appdata_dir_utf8();
#endif
const char	*get_sample_url_list_full_name();

/* tickr_render.c */
void		init_render_string_array();
void		free_all_render_strings_in_array();
/*RenderStringArray *get_render_str_array();*/
int		get_links_extra_offset();
cairo_surface_t *render_stream_to_surface(FILE *, FeedLinkAndOffset *, const Params *, int *);
int		get_font_size_from_layout_height(int, const char *);
int		get_layout_height_from_font_name_size(const char *);
char		spinning_shape();
void		show_str_beginning_and_end(char *);

/* tickr_params.c */
void		set_default_options(Params *);
int		get_config_file_options(Params *);
int		parse_options_array(Params *, int, const char *[]);
int		get_name_value_from_option(Params *, const char*);
void		save_to_config_file(const Params *);
int		get_gdk_color_from_hexstr(const char *, GdkColor *, guint16 *);
const char	*get_hexstr_from_gdk_color(const GdkColor *, const guint16 *);
void		adjust_font_size_to_rq_height(char *, int);
int		get_font_size_from_font_name_size(const char *);
void		split_font(const char *, char *, char *);
void		compact_font(char *, const char *, const char *);

/* tickr_clock.c */
void		display_time(const Params *);
int		get_clock_width(const Params *);

/* tickr_feedparser.c */
int		get_feed(Resource *, const Params *);
int		parse_xml_file(int, const char *, FeedLinkAndOffset *link_and_offset,
			const Params *);
void		get_rss20_selected_elements1(xmlNode *, xmlDoc *);
void		get_rss10_selected_elements1(xmlNode *, xmlDoc *);
void		get_atom_selected_elements1(xmlNode *, xmlDoc *);
void		get_feed_selected_elements2(int, xmlNode *, xmlDoc *,
			FeedLinkAndOffset *link_and_offset, const Params *prm);
int		get_feed_info(int *, const char *, char *, char *, char *);
void		get_xml_first_element(xmlNode *, xmlDoc *, char *, char *, int);
char		*log2vis_utf8(const char *);

/* tickr_feedpicker.c */
void		highlight_and_go_to_row(GtkTreeView *, GtkTreeSelection *, int);
int		check_and_update_feed_url(char *, char *);
void		manage_list_and_selection(Resource *);

/* tickr_prefwin.c */
next_action 	easily_modify_params(Params *);
next_action 	modify_params(Params *);

/* tickr_otherwins.c */
int		esc_key_pressed(GtkWidget *, GdkEventKey *);
void		force_quit_dialog(GtkWidget *);
void		open_txt_file(Resource *resrc);
void		show_resource_info(Resource *resrc);
void		help_win();
void		about_win();
void		info_win(const char *, const char *, info_type, zboolean);
void		minimalistic_info_win(const char *, const char *);
void		info_win_no_block(const char *, int);
int		question_win(const char *, int);
int		question_win_at(const char *, int, int);
/*void		win_with_spinner(win_with_spinner_mode, const char *);*/
void		win_with_progress_bar(win_with_progress_bar_mode, const char *);
#ifndef G_OS_WIN32
void		nanosleep2(long);
#endif

/* tickr_misc.c */
void		import_params();
void		export_params();
void		open_url_in_browser(char *);
void		online_help();
void		go_to_paypal();
void		dump_font_list();
void		dump_config();

/* tickr_hlptxt.c */
const char	**get_help_str0();
const char	**get_help_str1();
const char	**get_license_str1();
const char	*get_license_str2();

/* tickr_ompl.c */
int		import_opml_file();
void		export_opml_file();

/* tickr_http.c */
void		remove_chunk_info(char **);
int		connect_with_url(sockt *, char *);
int		fetch_resource(char *, const char *, char *);
const char	*build_http_request(const char *, const char *, const char *,
			const char *);
int 		get_http_response(sockt, const char *, const char *, const char *,
			char **, char **, int *);
int		get_http_status_code(const char *);
const char	*get_http_header_value(const char *, const char *);
int		go_after_next_cr_lf(const char *);
int		go_after_next_empty_line(const char *);
int		get_http_chunk_size(const char *);
const char	*get_scheme_from_url(const char *);
const char	*get_host_and_port_from_url(const char *, char *);
zboolean	port_num_is_valid(const char *);
const char	*get_path_from_url(const char *);

/* tickr_connectwin.c */
void		init_authentication();
void		init_proxy();
void		compute_auth_and_proxy_str();
int		connection_settings(connection_settings_page);
void		set_use_authentication(zboolean);
zboolean	get_use_authentication();
char		*get_http_auth_user();
char		*get_http_auth_psw();
char		*get_http_auth_str();
void		set_use_proxy(zboolean);
zboolean	get_use_proxy();
char		*get_proxy_host();
char		*get_proxy_port();
char		*get_proxy_str();
void		set_use_proxy_auth(zboolean);
zboolean	get_use_proxy_auth();
char		*get_proxy_auth_user();
char		*get_proxy_auth_psw();
char		*get_proxy_auth_str();

/* tickr_quickfeedpicker.c */
void		quick_feed_picker();

/* tickr_quicksetup.c */
void		quick_setup(Params *);

/* tickr_check4updates.c */
void		check_for_updates();
#endif /* INC_TICKR_H */
