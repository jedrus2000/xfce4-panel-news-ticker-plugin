/*
 *	libetm / error.c - Copyright (C) Emmanuel Thomas-Maurin 2008-2020
 *	<manutm007@gmail.com>
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

#include <stdlib.h>
#include <stdio.h>
#include "libetm.h"

/*
 * Generic find error string from error code in an ErrSt array function.
 * Need to pass array's number of elements, as arrays are always passed
 * as pointer: ErrSt array, n_el, error_code.
 */
const char *error_str_from_error_code(const ErrSt e_a[], int n_el, int e_c)
{
	static const char	*unknown = "(Unregistered error code)";
	int			i;

	for (i = 0; i < n_el; i++)
		if (e_a[i].code == e_c)
			return e_a[i].str;
	return unknown;
}

/*
 * Return error string from error code which apply to libetm only.
 */
const char *libetm_error_str(libetm_error_code e_c)
{
	if (e_c >= LIBETM_OK && e_c <= LIBETM_LASTERRORCODE)
		return error_str_from_error_code(lib_error,
			sizeof(lib_error) / sizeof(lib_error[0]), e_c);
	else
		return "";
}

/*
 * Critical error handler for libetm that will call big_error() function
 * defined in app, so that appropiate behaviour can be choosen.
 */
int big_error_in_lib(libetm_error_code e_c, const char *str)
{
	/* Only call big_error() in app */
	big_error(e_c, "%s-%s: %s(): %s",
		LIBETM_NAME, LIBETM_VERSION_NUM, str, libetm_error_str(e_c));
	return e_c;
}

/*
 * Warning handler for libetm that will call warning() function
 * defined in app, so that appropiate behaviour can be choosen.
 */
void warning_in_lib(int wait, libetm_error_code e_c, const char *str)
{
	/* Only call warning() in app */
	warning(wait, "%s-%s: %s(): %s",
		LIBETM_NAME, LIBETM_VERSION_NUM, str, libetm_error_str(e_c));
}

/*
 * Dump libetm error codes and strings.
 */
void dump_libetm_error_codes()
{
	int i;

	fprintf(STD_OUT, "%s-%s error codes and strings:\n", LIBETM_NAME, LIBETM_VERSION_NUM);
	for (i = 0; i < (int)(sizeof(lib_error) / sizeof(lib_error[0])); i++)
		fprintf(STD_OUT, "%d %s\n", i, libetm_error_str(i));
}
