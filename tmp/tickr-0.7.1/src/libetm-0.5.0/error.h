/*
 *	libetm / error.h - Copyright (C) Emmanuel Thomas-Maurin 2008-2020
 *	<manutm007@gmail.com
 *
 *	- Error handling -
 *
 *	This program is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation, either version 3 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef INC_LIBETM_ERROR_H
#define INC_LIBETM_ERROR_H

typedef struct {
	const int	code;
	const char	*str;
} ErrSt;

/* All libetm error values */

/* Exactly matching this enum and ErrSt array below doesn't matter anymore. */
typedef enum {
	LIBETM_OK,

	/* str_mem.c */
	OUT_OF_MEMORY,
	ZERO_RQ_SIZE,
	NEG_RQ_SIZE,
	NULL_DEST,
	SRC_EQ_DEST,
	STR_OVERLAP,
	NULL_POINTER_FREE,
	RNDSTR_UNKNOWN_MODE,

	/* tcp_socket.c */
	TCP_SOCK_ERROR,
	TCP_SOCK_CANT_CONNECT,
	TCP_SOCK_SHOULD_BE_CLOSED,
	SELECT_ERROR,
	SELECT_TIMED_OUT,
	SELECT_TRUE,
	SELECT_FALSE,
	TCP_SEND_ERROR,
	TCP_RECV_ERROR,

	CONNECTION_CLOSED_BY_SERVER,

	/* win32_specific.c */
#ifdef WIN32_V
	WIN32_ERROR,
	WIN32REGKEY_NOT_FOUND,
	WIN32REGKEY_CREATE_ERROR,
	WIN32REGKEY_SAVE_ERROR,
	WIN32REGKEY_OTHER_ERROR,
#endif

	/* Used by applications to enum error codes above this one. */
	LIBETM_LASTERRORCODE
} libetm_error_code;

/* Exactly matching this ErrSt array and enum above doesn't matter anymore. */
static const ErrSt lib_error[] = {
	{LIBETM_OK,			"OK"},

	/* str_mem.c */
	{OUT_OF_MEMORY,			"Out of memory"},
	{ZERO_RQ_SIZE,			"Zero requested size"},
	{NEG_RQ_SIZE,			"Negative requested size"},
	{NULL_DEST,			"Null destination string"},
	{SRC_EQ_DEST,			"Source string = destination string"},
	{STR_OVERLAP,			"Strings overlap"},
	{NULL_POINTER_FREE,		"Attempting to free a null pointer"},
	{RNDSTR_UNKNOWN_MODE,		"Generate random string: Unknown mode"},

	/* tcp_socket.c */
	{TCP_SOCK_ERROR,		"TCP socket error"},
	{TCP_SOCK_CANT_CONNECT,		"TCP socket error: Can't connect"},
	{TCP_SOCK_SHOULD_BE_CLOSED,	"TCP socket error: Should be closed"},
	{SELECT_ERROR,			"Select error"},
	{SELECT_TIMED_OUT,		"Select error: Timed out"},
	{SELECT_TRUE,			"(Not an error) select TRUE"},
	{SELECT_FALSE,			"(Not an error) select FALSE"},
	{TCP_SEND_ERROR,		"TCP send error"},
	{TCP_RECV_ERROR,		"TCP recv error"},
	{CONNECTION_CLOSED_BY_SERVER,	"Connection closed by server"},

	/* win32_specific.c */
#ifdef WIN32_V
	{WIN32_ERROR,			"Win32 error"},
	{WIN32REGKEY_NOT_FOUND,		"Can't find win32 registry key"},
	{WIN32REGKEY_CREATE_ERROR,	"Can't create win32 registry key"},
	{WIN32REGKEY_SAVE_ERROR,	"Can't save win32 registry key"},
	{WIN32REGKEY_OTHER_ERROR,	"Win32 registry key error (undetermined)"},
#endif

	/* Used by applications to enum error codes above this one. */
	{LIBETM_LASTERRORCODE,		"Libetm last enumerated error code"}
};

/*
 * Generic find error string from error code in an ErrSt array function.
 * Need to pass array's number of elements, as arrays are always passed
 * as pointer: ErrSt array, n_el, error_code.
 */
const char	*error_str_from_error_code(const ErrSt [], int, int);

/*
 * Return error string from error code which apply to libetm only.
 */
const char	*libetm_error_str(libetm_error_code);

/*
 * In-application-defined CRITICAL ERROR handler prototype.
 * Error code from application should enum above LIB_LASTERRORCODE.
 * (See error_handler_example.c.)
 */
int		big_error(int, const char *, ...);

/*
 * In-application-defined WARNING handler prototype.
 * Error code from application should enum above LIB_LASTERRORCODE.
 * (See error_handler_example.c.)
 */
void		warning(int, const char *, ...);

/*
 * This will call big_error() function defined in application.
 */
int		big_error_in_lib(libetm_error_code, const char *);

/*
 * This will call warning() function defined in application.
 */
/* Unused/useless so far... */
void		warning_in_lib(int, libetm_error_code, const char *);

/*
 * Dump libetm error codes and strings.
 */
void		dump_libetm_error_codes();
#endif /* INC_LIBETM_ERROR_H */
