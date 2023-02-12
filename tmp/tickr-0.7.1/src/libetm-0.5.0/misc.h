/*
 *	libetm / misc.h - Copyright (C) Emmanuel Thomas-Maurin 2008-2020
 *	<manutm007@gmail.com>
 *
 *	- Misc functions -
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

#ifndef INC_LIBETM_MISC_H
#define INC_LIBETM_MISC_H

/*
 * get_libetm_info(	O ) = version num
 * 			1   = copyright
 *			2   = license 1/3
 *			3   = license 2/3
 *			4   = license 3/3
 *			5   = compiled date
 *			> 5 = NULL
 */
const char	*get_libetm_info(unsigned int);


/*
 * Dump libetm info.
 */
void		dump_libetm_info();

/* In CLI mode */
zboolean	question(const char *);
#endif /* INC_LIBETM_MISC_H */
