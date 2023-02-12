/*
 *	libetm / win32_specific.h - Copyright (C) Emmanuel Thomas-Maurin 2008-2020
 *	<manutm007@gmail.com>
 *
 *	- Win32 specific functions -
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

#ifndef INC_LIBETM_WIN32_SPECIFIC_H
#define INC_LIBETM_WIN32_SPECIFIC_H

#ifdef WIN32_V
extern FILE *std_out, *std_err;

/*
 * On Linux, STD_OUT = stdout and STD_ERR = stderr / on win32, we must
 * first create specific text files then pass them to this function.
 */
void		libetm_init_win32_stdout_stderr(FILE *, FILE *);

void		libetm_init_win32_sockets();

void		libetm_cleanup_win32_sockets();

/* Return NULL if error */
const char	*get_appdata_dir();

const char	*get_appdata_dir_w();

const char	*get_progfiles_dir();

/* key_value must be able to store 255 chars */
int		get_key_value_from_win32registry(const char *, char *);

int		save_key_value_into_win32registry(const char *, const char *);

/* Return NULL if error */
const char	*get_default_browser_from_win32registry();

/* Return -1 if error */
int		get_win32_taskbar_height();

/* Unused so commented out */
/*zboolean	is_vista_or_higher();*/

/* Unused so commented out */
/*
 * Find up to 15 mac addresses for this computer
 * Return NULL if error
 */
/*const char 	**find_mac_addresses();*/

const char	*win32_error_msg(int);
#endif
#endif /* INC_LIBETM_WIN32_SPECIFIC_H */
