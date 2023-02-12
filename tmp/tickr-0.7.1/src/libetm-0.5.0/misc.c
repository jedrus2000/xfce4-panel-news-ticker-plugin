/*
 *	libetm / misc.c - Copyright (C) Emmanuel Thomas-Maurin 2008-2020
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

#include <stdio.h>
#include <ctype.h>
#include "libetm.h"

/*
 * get_libetm_info(	O ) = version num
 * 			1   = copyright
 *			2   = license 1/3
 *			3   = license 2/3
 *			4   = license 3/3
 *			5   = compiled date
 *			> 5 = NULL
 */
const char *get_libetm_info(unsigned int i)
{
	static const char *libv[] = {
		LIBETM_NAME " version " LIBETM_VERSION_NUM,
		"Copyright (C) Emmanuel Thomas-Maurin 2008-2020 <manu@manutm.net>",
		LIBETM_NAME " is free software: you can redistribute it and/or modify\n"
		"it under the terms of the GNU General Public License as published by\n"
		"the Free Software Foundation, either version 3 of the License, or\n"
		"(at your option) any later version.",
		LIBETM_NAME " is distributed in the hope that it will be useful,\n"
		"but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
		"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
		"GNU General Public License for more details.",
		"You should have received a copy of the GNU General Public License\n"
		/* Fix non-reproducible builds */
		"along with this program.  If not, see <http://www.gnu.org/licenses/>"/*,
		"Compiled on " __DATE__ " - " __TIME__*/
	};
	if (i < sizeof(libv) / sizeof(libv[0]))
		return (const char *)libv[i];
	else
		return NULL;
}

/*
 * Dump libetm info.
 */
void dump_libetm_info()
{
	int i = 0;

	while (get_libetm_info(i) != NULL)
		fprintf(STD_OUT, "%s\n\n", get_libetm_info(i++));

}

/* In CLI mode */
zboolean question(const char *str)
{
	int i, c;

	while (1) {
		fprintf(STD_OUT, "%s [Y/N] ? ", str);
		i = fgetc(stdin);
		c = tolower(i);
		while (i != '\n' && i != EOF)
			(i = fgetc(stdin));
		if (c == 'y')
			return TRUE;
		else if (c == 'n')
			return FALSE;
	}
}
