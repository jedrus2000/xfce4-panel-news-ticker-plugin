/*
 *	libetm / str_mem.h - Copyright (C) Emmanuel Thomas-Maurin 2008-2020
 *	<manutm007@gmail.com>
 *
 *	- A few strings and memory management functions -
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

#ifndef INC_LIBETM_STR_MEM_H
#define INC_LIBETM_STR_MEM_H

/*
 * Copy n bytes max from src to dest then add '\0' at end of dest.
 */
char		*str_n_cpy(char *, const char *, size_t);

/*
 * Concanate n bytes max of src to dest then add '\0' at end of dest.
 * Strings may not be identical and should not overlap.
 */
char		*str_n_cat(char *, const char *, size_t);

/*
 * Create new_str (allocate memory) and copy str (can be NULL) to new_str.
 */
char		*l_str_new(const char *str);

/*
 * Append src (can be NULL) to dest (re-allocate memory as necessary).
 * dest must have been created by l_str_new()/malloc() - strings may overlap.
 * Note: faster than l_str_insert_at_b().
 */
char		*l_str_cat(char *, const char *);

/*
 * Insert src (can be NULL) at the beginning of dest (re-allocate memory as necessary).
 * dest must have been created by l_str_new()/malloc() - strings may overlap.
 * Note: slower than l_str_cat().
 */
char		*l_str_insert_at_b(char *, const char *);

/*
 * Free string created by l_str_new(), l_str_cat() or l_str_insert_at_b().
 * Also set ptr to NULL.
 */
void		l_str_free(char *);

/*
 * Wrappers for malloc(), realloc(), calloc() and free() which check returned value.
 */
void		*malloc2(size_t);
void		*realloc2(void *, size_t);
void		*calloc2(size_t, size_t);
/* Also set ptr to NULL. */
void		free2(void *);

/*
 * Return size in readable format (KiB, MiB, GiB, TiB, ...)
 * *** Allow up to 16 simultaneous calls ***
 * Convention: we assume 2.5 MiB = 2 MiB + [0.5 x 1024 = 512] KiB, not 500 KiB.
 * Otherwise, there is no way to express a value in the range 1000 - 1023,
 * so x.y MiB = x MiB + 0.y MiB, that is: not y x 100 KiB but y x 102.4 KiB.
 */
const char	*readable_size(double);

/*
 * itoa() is not ANSI C so this one could be useful.
 * *** Allow up to 16 simultaneous calls ***
 */
const char	*itoa2(long int);

/*
 * Modify string in place.
 */
char		*remove_char_from_str(char *, char);

/*
 * Modify string in place.
 */
char		*remove_leading_whitespaces_from_str(char *);

/*
 * Modify string in place.
 */
char		*remove_trailing_whitespaces_from_str(char *);

/*
 * Modify string in place.
 */
char		*remove_surrounding_whitespaces_from_str(char *);

/*
 * Check that str contains only digits (at least one) and whitespaces
 * (may start and/or end with whitespaces), meaning numerical value is
 * integer >= 0.
 */
zboolean	str_is_num(const char *);

/*
 * Check that str contains only whitespaces or is empty.
 */
zboolean	str_is_blank(const char *);

/*
 * Generate a random string up to 1023 chars long.
 * mode = a -> alpha / d -> digits / b ->  both
 */
const char	*rnd_str(char, int);

#ifdef LIBETM_EXPERIMENTAL_STUFF
/*
 * Very basic xor sym encryption for small strings (up to 511 chars).
 * *** Key must be ascii / if 2 chars match (in str and key) the string is cut off ***
 * *** Allow up to 16 simultaneous calls ***
 * Works with ascii strings, probably not otherwise so don't try with
 * exotic things...
 */
const char	*str_crypt(const char *, const char *);

/*
 * Very basic crypto hash.
 */
unsigned long	str_crypto_hash(const char *, const char*);
#endif
#endif /* INC_LIBETM_STR_MEM_H */
