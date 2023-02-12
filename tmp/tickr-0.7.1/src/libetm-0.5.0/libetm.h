/*
 *	libetm / libetm.h - Copyright (C) Emmanuel Thomas-Maurin 2008-2020
 *	<manutm007@gmail.com>
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

#ifndef INC_LIBETM_H
#define INC_LIBETM_H

#define LIBETM_NAME		"Libetm"
#define LIBETM_VERSION_NUM	"0.5.0"

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#ifndef _ISOC99_SOURCE
#define _ISOC99_SOURCE
#endif

#define VERBOSE_OUTPUT
/*#define LIBETM_EXPERIMENTAL_STUFF*/

/* stdout and stderr on Linux / text files on win32 (see win32_specific.c) */
#ifndef WIN32_V
#  define STD_OUT	stdout
#  define STD_ERR	stderr
#else
#  define STD_OUT	std_out
#  define STD_ERR	std_err
#endif

#define INFO_OUT(...) \
{\
	fprintf(STD_OUT, __VA_ARGS__);\
	fflush(STD_OUT);\
}
#define INFO_ERR(...) \
{\
	fprintf(STD_ERR, __VA_ARGS__);\
	fflush(STD_ERR);\
}

/*
 * Following macros depends on definition of VERBOSE_OUTPUT and DEBUG_OUTPUT
 *
 * === NOTE: As VERBOSE_OUTPUT is actually defined here, to skip, well, verbose
 * output in application compiled against libetm, you must always *undefine*
 * VERBOSE_OUTPUT in code of application ===
 */
#ifdef VERBOSE_OUTPUT
#  define VERBOSE_INFO_OUT(...)	INFO_OUT(__VA_ARGS__)
#  define VERBOSE_INFO_ERR(...) INFO_ERR(__VA_ARGS__)
#else
#  define VERBOSE_INFO_OUT(...) {}
#  define VERBOSE_INFO_ERR(...) {}
#endif

#ifdef DEBUG_OUTPUT
#  define DEBUG_INFO(...) \
{\
	fprintf(STD_OUT, "[%s: %d] %s(): ", __FILE__, __LINE__, __func__);\
	INFO_OUT(__VA_ARGS__)\
}
#else
#  define DEBUG_INFO(...) {}
#endif

/* === TODO: Should we use mutex locks instead ? ===*/
#define N_SIMULTANEOUS_CALLS		64
#define N_SIMULTANEOUS_CALLS_MASK	63

/* Should zboolean type be sig_atomic_t ???? */
#undef TRUE
#undef FALSE
typedef enum {
	FALSE = (0), TRUE = (!FALSE)
} zboolean;	/* Couldn't find anything better at the moment */

#define YES	(1)
#define NO	(0)

#include "str_mem.h"
#include "tcp_socket.h"
#include "error.h"
#include "misc.h"
/*#include "dllist.h"*/
#ifdef WIN32_V
#include "win32_specific.h"
#endif

#ifndef MAX
#  define MAX(x,y)	((x) > (y) ? (x) : (y))
#endif

#ifndef MIN
#  define MIN(x,y)	((x) < (y) ? (x) : (y))
#endif

#ifndef ABS
#  define ABS(x)	((x) > 0 ? (x) : -(x))
#endif

/* Do more testing for the 2 following ones */
#ifndef SIGN
#  define SIGN(x)	((x) != 0 ? ((x) > 0 ? 1 : -1) : 0)
#endif

/* Only if f > 0 */
#ifndef APPROX
#  define APPROX(f)	(ABS(((f) - (int)(f)) > 0.5) ? (int)(f) + SIGN((f)) : (int)(f))
#endif
#endif /* INC_LIBETM_H */
