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
#include "tickr_error.h"

const char *tickr_error_str(tickr_error_code e_c)
{
	return error_str_from_error_code(e_a, sizeof(e_a) / sizeof(e_a[0]), e_c);
}

const char *global_error_str(int error_code)
{
	if (error_code >= LIBETM_OK && error_code <= LIBETM_LASTERRORCODE)
		return libetm_error_str((libetm_error_code)error_code);
	else
		return tickr_error_str((tickr_error_code)error_code);
}

/*
 * This func's prototype is in libetm/error.h.
 * This func is not defined in libetm but in application and it handles
 * CRITICAL ERRORS from both libetm and application.
 */

#define ERROR_WARNING_STR_MAXLEN	(8 * 1024 - 1)
#define OUCH_STR			"\nOuch !!  Something went wrong ...\n\n"

int big_error(int big_error_code, const char *format, ...)
{
	char		error_str[ERROR_WARNING_STR_MAXLEN + 1] = "";
	va_list		a_list;

	if (get_ticker_env() != NULL)
		get_ticker_env()->suspend_rq = TRUE;

	str_n_cpy(error_str, OUCH_STR, 100);
	str_n_cat(error_str, "CRITICAL ERROR in ", 100);
	if (big_error_code > LIBETM_LASTERRORCODE)
		str_n_cat(error_str, APP_NAME ": ", 100);
	va_start(a_list, format);
	vsnprintf(error_str + strlen(error_str),
		ERROR_WARNING_STR_MAXLEN + 1 - strlen(error_str), format, a_list);
	va_end(a_list);
	snprintf(error_str + strlen(error_str),
		ERROR_WARNING_STR_MAXLEN + 1 - strlen(error_str), " - Will quit now");

	if (STD_ERR != NULL)
		INFO_ERR("%s\n", (char *)error_str + strlen(OUCH_STR))
	/* We want this win to always popup */
	if (get_params() != NULL && get_ticker_env() != NULL) {
		get_params()->disable_popups = 'n';
		info_win(APP_NAME " - Critical error", error_str, INFO_ERROR, FALSE);
	} else
		minimalistic_info_win(APP_NAME " - Critical error", error_str);
	free_all();
	exit(big_error_code);
}

/*
 * This func's prototype is in libetm/error.h.
 * This func is not defined in libetm but in application and it handles
 * WARNINGS from both libetm and application.
 *
 * Will popup and wait for INFO_WIN_WAIT_TIMEOUT ms if no_block == TRUE, then close
 * otherwise, will block until an appropriate keyboard/mouse action happens.
 *
 * You can use BLOCK/NO_BLOCK helpers.
 */
void warning(int no_block, const char *format, ...)
{
	char		warning_str[ERROR_WARNING_STR_MAXLEN + 1] = "";
	va_list		a_list;

	va_start(a_list, format);
	vsnprintf(warning_str + strlen(warning_str),
		ERROR_WARNING_STR_MAXLEN + 1 - strlen(warning_str), format, a_list);
	va_end(a_list);

	INFO_ERR("%s\n", warning_str)
	if (no_block)
		info_win_no_block(warning_str, INFO_WIN_WAIT_TIMEOUT);
	else
		info_win("", warning_str, INFO_WARNING, FALSE);
}

void dump_error_codes()
{
	int i, j;

	dump_libetm_error_codes();
	fprintf(STD_OUT, "\n%s-%s error codes and strings:\n", APP_NAME, APP_V_NUM);
	for (i = 0; i < (int)(sizeof(e_a) / sizeof(e_a[0])); i++) {
		j = i + LIBETM_LASTERRORCODE + 1;
		fprintf(STD_OUT, "%d %s\n", j , tickr_error_str(j));
	}
}
