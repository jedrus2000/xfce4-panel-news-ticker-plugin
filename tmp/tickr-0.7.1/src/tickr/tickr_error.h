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

#ifndef INC_TICKR_ERROR_H
#define INC_TICKR_ERROR_H

/* Helpers for warning() */
#define NO_BLOCK	TRUE		/* Show for INFO_WIN_WAIT_TIMEOUT ms */
#define BLOCK		!NO_BLOCK	/* Wait for user action */

/* Error codes */

/* Exactly matching this enum and ErrSt array below doesn't matter anymore. */
typedef enum {
	OK = LIBETM_LASTERRORCODE + 1,

	NO_RESOURCE_SPECIFIED,
	RESOURCE_NOT_FOUND,
	RESOURCE_INVALID,
	RESOURCE_ENCODING_ERROR,

	OPTION_TOO_MANY,
	OPTION_INVALID,
	OPTION_UNKNOWN,
	OPTION_VALUE_INVALID,

	FEED_FORMAT_ERROR,
	FEED_UNPARSABLE,
	FEED_EMPTY,
	FEED_NO_ITEM_OR_ENTRY_ELEMENT,

	SELECTION_ERROR,
	SELECTION_EMPTY,

	RENDER_ERROR,
	RENDER_NO_RESOURCE,
	RENDER_CAIRO_IMAGE_SURFACE_TOO_WIDE,
	RENDER_CREATE_CAIRO_IMAGE_SURFACE_ERROR,
	RENDER_FILL_IN_STR_ARRAY_ERROR,
	RENDER_PROCESS_STR_ARRAY_ERROR,
	RENDER_PANGO_LAYOUT_WIDTH_OVERFLOW,

	READ_FROM_STREAM_ERROR,
	WRITE_TO_STREAM_ERROR,

	NOT_UTF8_ENCODED,

	SHIFT2LEFT_NULL_CAIRO_IMAGE_SURFACE,

	FLIST_ERROR,
	LOAD_URL_LIST_ERROR,
	LOAD_URL_LIST_EMPTY_LIST,
	LOAD_URL_LIST_NO_LIST,
	SAVE_URL_LIST_ERROR,

	CREATE_FILE_ERROR,
	OPEN_FILE_ERROR,

	XML_UNPARSABLE,
	XML_EMPTY,

	OPML_ERROR,

	CONNECT_TOO_MANY_ERRORS,

	TLS_ERROR,
	TLS_SEND_ERROR,
	TLS_RECV_ERROR,

	HTTP_ERROR,
	HTTP_UNSUPPORTED_SCHEME,
	HTTP_NO_AUTH_CREDENTIALS,
	HTTP_NO_PROXY_AUTH_CREDENTIALS,
	HTTP_INVALID_PORT_NUM,
	HTTP_NO_STATUS_CODE,
	HTTP_TOO_MANY_REDIRECTS,

	/* http status codes */
	HTTP_CONTINUE,
	HTTP_SWITCH_PROTO,
	HTTP_MOVED,			/* All moved status codes but permamently */
	HTTP_MOVED_PERMANENTLY,
	HTTP_USE_PROXY,
	HTTP_UNAUTHORIZED,
	HTTP_BAD_REQUEST,
	HTTP_FORBIDDEN,
	HTTP_NOT_FOUND,
	HTTP_PROXY_AUTH_REQUIRED,
	HTTP_GONE,
	HTTP_INT_SERVER_ERROR,

	NO_BROWSER_SET_ERROR,

	SEGFAULT,

#ifdef G_OS_WIN32
	WIN32V_ERROR,
#endif

	TICKR_LASTERRORCODE
} tickr_error_code;

/* Exactly matching this ErrSt array and enum above doesn't matter anymore. */
static const ErrSt e_a[] = {
	{OK,					"OK"},

	{NO_RESOURCE_SPECIFIED,			"No resource specified"},
	{RESOURCE_NOT_FOUND,			"Resource not found"},
	{RESOURCE_INVALID,			"Invalid resource"},
	{RESOURCE_ENCODING_ERROR,		"Resource is *not* UTF-8 encoded"},

	{OPTION_TOO_MANY,			"Too many options"},
	{OPTION_INVALID,			"Invalid option"},
	{OPTION_UNKNOWN,			"Unknown option"},
	{OPTION_VALUE_INVALID,			"Invalid option value"},

	{FEED_FORMAT_ERROR,			"Feed format error (RSS 1.0/2.0 or ATOM expected)"},
	{FEED_UNPARSABLE,			"Feed is unparsable"},
	{FEED_EMPTY,				"Feed is empty"},
	{FEED_NO_ITEM_OR_ENTRY_ELEMENT,		"No item or entry element"},

	{SELECTION_ERROR,			"Feed selection error (no more info)"},
	{SELECTION_EMPTY,			"Feed selection is empty"},

	{RENDER_ERROR,				"Render error (no more info)"},
	{RENDER_NO_RESOURCE,			"Render: No resource"},
	{RENDER_CAIRO_IMAGE_SURFACE_TOO_WIDE,	"Cairo image surface is too wide (> 32 K - 1 pixels)"},
	{RENDER_CREATE_CAIRO_IMAGE_SURFACE_ERROR,	"Can't create cairo image surface"},
	{RENDER_FILL_IN_STR_ARRAY_ERROR,	"Error while filling in RenderString array - Check out log for more info"},
	{RENDER_PROCESS_STR_ARRAY_ERROR,	"Error while processing RenderString array - Check out log for more info"},
	{RENDER_PANGO_LAYOUT_WIDTH_OVERFLOW,	"Overflow - Pango can't compute layout width of text with these font name and size.\n"
						"Please either decrease text length or font size."},

	{READ_FROM_STREAM_ERROR,		"Error while reading from stream"},
	{WRITE_TO_STREAM_ERROR,			"Error while writing to stream"},

	{NOT_UTF8_ENCODED,			"String is *not* UTF-8 encoded"},

	{SHIFT2LEFT_NULL_CAIRO_IMAGE_SURFACE,	"Null cairo image surface"},

	{FLIST_ERROR,				"FList error (no more info)"},
	{LOAD_URL_LIST_ERROR,			"Can't load URL list"},
	{LOAD_URL_LIST_EMPTY_LIST,		"URL list is empty"},
	{LOAD_URL_LIST_NO_LIST,			"URL list doesn't exist"},
	{SAVE_URL_LIST_ERROR,			"Can't save URL list"},

	{CREATE_FILE_ERROR,			"Can't create file"},
	{OPEN_FILE_ERROR,			"Can't open file"},

	{XML_UNPARSABLE,			"XML file is unparsable"},
	{XML_EMPTY,				"XML file is empty"},

	{OPML_ERROR,				"OPML error (no more info)"},

	{CONNECT_TOO_MANY_ERRORS,		"Too many connection attempts"},

	{TLS_ERROR,				"TLS error (no more info)"},
	{TLS_SEND_ERROR,			"TLS send error"},
	{TLS_RECV_ERROR,			"TLS recv error"},

	{HTTP_ERROR,				"HTTP error (no more info)"},
	{HTTP_UNSUPPORTED_SCHEME,		"Unsupported scheme in URL"},
	{HTTP_NO_AUTH_CREDENTIALS,		"No authentication credentials"},
	{HTTP_NO_PROXY_AUTH_CREDENTIALS,	"No proxy authentication credentials"},
	{HTTP_INVALID_PORT_NUM,			"Invalid port number in URL"},
	{HTTP_NO_STATUS_CODE,			"No HTTP status code returned"},
	{HTTP_TOO_MANY_REDIRECTS,		"Too many HTTP redirects"},

	/* http status codes */
	{HTTP_CONTINUE,				"Continue"},
	{HTTP_SWITCH_PROTO,			"Switch protocol"},
	{HTTP_MOVED,				"Moved (not permanently)"},
	{HTTP_MOVED_PERMANENTLY,		"Moved permanently"},
	{HTTP_USE_PROXY,			"Must be accessed through proxy"},
	{HTTP_UNAUTHORIZED,			"HTTP authentication required"},
	{HTTP_BAD_REQUEST,			"Bad request"},
	{HTTP_FORBIDDEN,			"Forbidden"},
	{HTTP_NOT_FOUND,			"Not found"},
	{HTTP_PROXY_AUTH_REQUIRED,		"Proxy authentication required"},
	{HTTP_GONE,				"Gone"},
	{HTTP_INT_SERVER_ERROR,			"Internal server error"},

	{NO_BROWSER_SET_ERROR,			"No Browser is set"},

	{SEGFAULT,				"Segfault (this is a *bug*)"},

#ifdef G_OS_WIN32
	{WIN32V_ERROR,				"Win32 version error (no more info)"},
#endif

	{TICKR_LASTERRORCODE,			"Tickr last enumerated error code"}
};

const char	*tickr_error_str(tickr_error_code);
const char	*global_error_str(int);
void		dump_error_codes();
#endif /* INC_TICKR_ERROR_H */
