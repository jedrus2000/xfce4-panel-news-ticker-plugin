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

	fprintf(STD_ERR, "%s\n", (char *)error_str + strlen(OUCH_STR));
	fflush(NULL);
	info_win(APP_NAME " - Critical error", error_str, INFO_ERROR, FALSE);
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

/* Helpers for warning() */
#define NO_BLOCK	TRUE
#define BLOCK		!NO_BLOCK

void warning(int no_block, const char *format, ...)
{
	char		warning_str[ERROR_WARNING_STR_MAXLEN + 1] = "";
	va_list		a_list;

	va_start(a_list, format);
	vsnprintf(warning_str + strlen(warning_str),
		ERROR_WARNING_STR_MAXLEN + 1 - strlen(warning_str), format, a_list);
	va_end(a_list);

	fprintf(STD_ERR, "%s\n", warning_str);
	fflush(STD_ERR);
	if (no_block)
		info_win_no_block(warning_str, INFO_WIN_WAIT_TIMEOUT);
	else
		info_win("", warning_str, INFO_WARNING, FALSE);
}
