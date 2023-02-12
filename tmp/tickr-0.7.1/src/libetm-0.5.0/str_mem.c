/*
 *	libetm / str_mem.c - Copyright (C) Emmanuel Thomas-Maurin 2008-2020
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <math.h>
#include "libetm.h"

/*
 * Copy n bytes max from src to dest then add '\0' at end of dest.
 */
char *str_n_cpy(char *dest, const char *src, size_t length)
{
	char *dest_bak = dest;

	if (dest == NULL)
		big_error_in_lib(NULL_DEST, __func__);

	while (length-- > 0 && *src != '\0')
		*dest++ = *src++;

	*dest = '\0';

	return dest_bak;
}

/*
 * Concanate n bytes max of src to dest then add '\0' at end of dest.
 * Strings may not be identical and should not overlap.
 */
char *str_n_cat(char *dest, const char *src, size_t length)
{
	char *dest_bak = dest;

	if (dest == NULL)
		big_error_in_lib(NULL_DEST, __func__);
	else if (src == dest)
		big_error_in_lib(SRC_EQ_DEST, __func__);

	while (*dest++ != '\0');
	dest--;

	while (length-- > 0 && *src != '\0')
		*dest++ = *src++;

	*dest = '\0';

	return dest_bak;
}

/*
 * Create new_str (allocate memory) and copy str (can be NULL) to new_str.
 */
char *l_str_new(const char *str)
{
	char	*new_str;
	size_t	str_len;

	if (str == NULL)
		str_len = 0;
	else
		str_len = strlen(str);

	new_str = malloc2(sizeof(char) * (str_len + 1));
	if (new_str == NULL)
		big_error_in_lib(OUT_OF_MEMORY, __func__);

	return str_n_cpy(new_str, str, str_len);
}

/*
 * Append src (can be NULL) to dest (re-allocate memory as necessary).
 * dest must have been created by l_str_new()/malloc() - strings may overlap.
 * Note: faster than l_str_insert_at_b().
 */
char *l_str_cat(char *dest, const char *src)
{
	char	*new_str, *src2;
	size_t	src_len, dest_len;

	if (dest == NULL)
		big_error_in_lib(NULL_DEST, __func__);
	else if (src == NULL)
		return dest;

	src_len = strlen(src);
	if (src_len == 0)
		return dest;
	dest_len = strlen(dest);

	src2 = l_str_new(src);
	new_str = realloc(dest, sizeof(char) * (dest_len + src_len + 1));
	if (new_str == NULL)
		big_error_in_lib(OUT_OF_MEMORY, __func__);

	str_n_cpy(new_str + dest_len, src2, src_len);
	l_str_free(src2);

	return new_str;
}

/*
 * Insert src (can be NULL) at the beginning of dest (re-allocate memory as necessary).
 * dest must have been created by l_str_new()/malloc() - strings may overlap.
 * Note: slower than l_str_cat().
 */
char *l_str_insert_at_b(char *dest, const char *src)
{
	char	*new_str;
	size_t	src_len, dest_len;

	if (dest == NULL)
		big_error_in_lib(NULL_DEST, __func__);
	else if (src == NULL)
		return dest;

	src_len = strlen(src);
	if (src_len == 0)
		return dest;
	dest_len = strlen(dest);

	new_str = malloc(sizeof(char) * (src_len + dest_len + 1));
	if (new_str == NULL)
		big_error_in_lib(OUT_OF_MEMORY, __func__);

	str_n_cpy(new_str, src, src_len);
	str_n_cpy(new_str + src_len, dest, dest_len);
	l_str_free(dest);

	return new_str;
}

/*
 * Free string created by l_str_new(), l_str_cat() or l_str_insert_at_b().
 * Also set ptr to NULL.
 */
void l_str_free(char *str)
{
	if (str == NULL)
		big_error_in_lib(NULL_POINTER_FREE, __func__);

	free(str);
	str = NULL;
}

/*
 * Wrappers for malloc(), realloc(), calloc() and free() which check returned value.
 */
void *malloc2(size_t size)
{
	void *mem_block = NULL;

	if (size == 0)
		big_error_in_lib(ZERO_RQ_SIZE, __func__);
	else if ((mem_block = malloc(size)) == NULL)
		big_error_in_lib(OUT_OF_MEMORY, __func__);

	return mem_block;
}

void *realloc2(void *mem_block, size_t size)
{
	void *mem_block2 = mem_block;

	if (size == 0)
		big_error_in_lib(ZERO_RQ_SIZE, __func__);
	else if ((mem_block2 = realloc(mem_block, size)) == NULL)
		big_error_in_lib(OUT_OF_MEMORY, __func__);

	return mem_block2;
}

void *calloc2(size_t n_elements, size_t element_size)
{
	void	*mem_block = NULL;
	size_t	size = n_elements * element_size;

	if (size == 0)
		big_error_in_lib(ZERO_RQ_SIZE, __func__);
	else if ((mem_block = malloc(size)) == NULL)
		big_error_in_lib(OUT_OF_MEMORY, __func__);
	else
		memset(mem_block, 0, size);

	return mem_block;
}

/* Also set ptr to NULL. */
void free2(void *mem_block)
{
	if (mem_block == NULL)
		big_error_in_lib(NULL_POINTER_FREE, __func__);

	free(mem_block);
	mem_block = NULL;
}

/*
 * Return size in readable format (KiB, MiB, GiB, TiB, ...)
 * *** Allow up to N_SIMULTANEOUS_CALLS simultaneous calls ***
 * Convention: we assume 2.5 MiB = 2 MiB + [0.5 x 1024 = 512] KiB, not 500 KiB.
 * Otherwise, there is no way to express a value in the range 1000 - 1023,
 * so x.y MiB = x MiB + 0.y MiB, that is: not y x 100 KiB but y x 102.4 KiB.
 */
const char *readable_size(double size_bytes)
{
	/*
	 * We use an array of N_SIMULTANEOUS_CALLS strings to store the returned string
	 * in order to allow N_SIMULTANEOUS_CALLS simultaneous calls.
	 * === TODO: Should we use mutex locks instead ? ===
	 * Otherwise, sth like:
	 * printf("size1 = %s / size2 = %s\n", readable_size(size1), readable_size(size2))
	 * would produce unpredictable / false results (like readable sizes are
	 * equal whereas sizes are different).
	 */
	static char	lib_static_str[N_SIMULTANEOUS_CALLS][128];
	double		size_KiB, size_MiB, size_GiB, size_TiB, size_PiB, size_EiB, size_ZiB, size_YiB;
	double		remainder_bytes, remainder_KiB, remainder_MiB, remainder_GiB, remainder_TiB,
				remainder_PiB, remainder_EiB, remainder_ZiB;
	static int	count = -1;

	count++;
	count &= N_SIMULTANEOUS_CALLS_MASK;

	if (size_bytes < 0.0f)
		return "[Negative size error] bytes";

	size_bytes = floor(size_bytes);		/* OK ? */

	size_KiB = floor(size_bytes / 1024);
	remainder_bytes = size_bytes - size_KiB * 1024;

	size_MiB = floor(size_KiB / 1024);
	remainder_KiB = size_KiB - size_MiB * 1024;

	size_GiB = floor(size_MiB / 1024);
	remainder_MiB = size_MiB - size_GiB * 1024;

	size_TiB = floor(size_GiB / 1024);
	remainder_GiB = size_GiB - size_TiB * 1024;

	size_PiB = floor(size_TiB / 1024);
	remainder_TiB = size_TiB - size_PiB * 1024;

	size_EiB = floor(size_PiB / 1024);
	remainder_PiB = size_PiB - size_EiB * 1024;

	size_ZiB = floor(size_EiB / 1024);
	remainder_EiB = size_EiB - size_ZiB * 1024;

	size_YiB = floor(size_ZiB / 1024);
	remainder_ZiB = size_ZiB - size_YiB * 1024;

	if (size_YiB >= 1)
		snprintf(lib_static_str[count], 128, "%.0f.%.0f YiB",
			size_YiB, floor(remainder_ZiB / 102.4));
	else if (size_ZiB >= 1)
		snprintf(lib_static_str[count], 128, "%.0f.%.0f ZiB",
			size_ZiB, floor(remainder_EiB / 102.4));
	else if (size_EiB >= 1)
		snprintf(lib_static_str[count], 128, "%.0f.%.0f EiB",
			size_EiB, floor(remainder_PiB / 102.4));
	else if (size_PiB >= 1)
		snprintf(lib_static_str[count], 128, "%.0f.%.0f PiB",
			size_PiB, floor(remainder_TiB / 102.4));
	else if (size_TiB >= 1)
		snprintf(lib_static_str[count], 128, "%.0f.%.0f TiB",
			size_TiB, floor(remainder_GiB / 102.4));
	else if (size_GiB >= 1)
		snprintf(lib_static_str[count], 128, "%.0f.%.0f GiB",
			size_GiB, floor(remainder_MiB / 102.4));
	else if (size_MiB >= 1)
		snprintf(lib_static_str[count], 128, "%.0f.%.0f MiB",
			size_MiB, floor(remainder_KiB / 102.4));
	else if (size_KiB >= 1)
		snprintf(lib_static_str[count], 128, "%.0f.%.0f KiB",
			size_KiB, floor(remainder_bytes / 102.4));
	else
		snprintf(lib_static_str[count], 128, "%.0f bytes", size_bytes);

	return (const char *)lib_static_str[count];
}

/*
 * itoa() is not ANSI C so this one could be useful.
 * *** Allow up to N_SIMULTANEOUS_CALLS simultaneous calls ***
 */
const char *itoa2(long int n)
{
	/* Array of N_SIMULTANEOUS_CALLS strings (like for readable_size() - see above) */
	static char	lib_static_str[N_SIMULTANEOUS_CALLS][128];
	static int	count = -1;

	count++;
	count &= N_SIMULTANEOUS_CALLS_MASK;

	snprintf(lib_static_str[count], 128, "%ld", n);
	return (const char *)lib_static_str[count];
}

/*
 * Modify string in place.
 */
char *remove_char_from_str(char *str, char c)
{
	int i, len = strlen(str);

	for (i = 0; i < len && str[i] != '\0';) {
		if (str[i] == c)
			str_n_cpy(str + i, (const char *)(str + i + 1), len - i);
		else
			i++;
	}
	return str;
}

/*
 * Modify string in place.
 */
char *remove_leading_whitespaces_from_str(char *str)
{
	int i, len = strlen(str);

	for (i = 0; i < len;)
		if (isspace((int)str[i]))
			i++;
		else
			break;
	if (i > 0 && i <= len)
		str_n_cpy(str, (const char *)(str + i), len - i);
	return str;
}

/*
 * Modify string in place.
 */
char *remove_trailing_whitespaces_from_str(char *str)
{
	int i = strlen(str) - 1;

	while (isspace((int)str[i]) && i >= 0)
		i--;
	str[i + 1] = '\0';
	return str;
}

/*
 * Modify string in place.
 */
char *remove_surrounding_whitespaces_from_str(char *str)
{
	return remove_trailing_whitespaces_from_str(remove_leading_whitespaces_from_str(str));
}

/*
 * Check that str contains only digits (at least one) and whitespaces
 * (may start and/or end with whitespaces), meaning numerical value is
 * integer >= 0.
 */
zboolean str_is_num(const char *str2)
{
	char	*str, *s;
	int	i, len;

	if (*str2 == '\0')
		return FALSE;
	s = str = l_str_new(str2);
	len = strlen(str);
	i = 0;
	while (i < len)
		if (isspace((int)*s)) {
			i++;
			s++;
		} else
			break;
	if (i == len) {
		l_str_free(str);
		return FALSE;
	}
	while (i < len)
		if (isdigit((int)*s)) {
			i++;
			s++;
		} else
			break;
	if (i == len) {
		l_str_free(str);
		return TRUE;
	}
	while (i < len)
		if (isspace((int)*s)) {
			i++;
			s++;
		} else
			break;
	l_str_free(str);
	if (i == len)
		return TRUE;
	else
		return FALSE;
}

/*
 * Check that str contains only whitespaces or is empty.
 */
zboolean str_is_blank(const char *str)
{
	int i = 0, len = strlen(str);

	if (*str == '\0')
		return TRUE;
	while (i < len)
		if (isspace((int)*str)) {
			i++;
			str++;
		} else
			break;
	if (i == len)
		return TRUE;
	else
		return FALSE;
}

/*
 * Generate a random string up to 1023 chars long.
 * mode = a -> alpha / d -> digits / b ->  both
 */
const char *rnd_str(char mode, int length)
{
	static char	str[1024];
	char		str_a[65] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijkl";
	char		str_d[17] = "1234567890123456";
	char		str_b[65] = "1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ00";
	int		i = 0;

	if (length > 1023)
		length = 1023;
	srand(time(0));
	if (mode == 'a') {
		while (i < length)
			str[i++] = str_a[rand() & 63];
	} else if (mode == 'd') {
		while (i < length)
			str[i++] = str_d[rand() & 15];
	} else if (mode == 'b') {
		while (i < length)
			str[i++] = str_b[rand() & 63];
	} else
		big_error_in_lib(RNDSTR_UNKNOWN_MODE, __func__);
	str[i] = '\0';
	return str;
}

#ifdef LIBETM_EXPERIMENTAL_STUFF
/*
 * Very basic xor sym encryption for small strings (up to 511 chars).
 * *** Key must be ascii / if 2 chars match (in str and key) the string is cut off ***
 * *** Allow up to N_SIMULTANEOUS_CALLS simultaneous calls ***
 * Works with ascii strings, probably not otherwise so don't try with
 * exotic things...
 */
const char *str_crypt(const char *str, const char *key)
{
	/* Array of N_SIMULTANEOUS_CALLS strings (like for readable_size() - see above) */
	static char	str2[N_SIMULTANEOUS_CALLS][512];
	static int	count = -1;
	int		i, j;

	count++;
	count &= N_SIMULTANEOUS_CALLS_MASK;
	for (i = 0, j = 0; i < 511 && str[i] != '\0'; i++) {
		if (key[j] == '\0')
			j = 0;
		str2[count][i] = str[i] ^ key[j++];
	}
	str2[count][i] = '\0';
	return (const char *)str2[count];
}

/*
 * Very basic crypto hash.
 */
unsigned long str_crypto_hash(const char *str, const char* salt)
{
	char		*str2;
	unsigned long	hash = 0;
	int		i = 0;

	str2 = (char *)str_crypt(str, salt);
	while (str2[i] != '\0') {
		hash += str2[i] * 1024 + (str2[i] + 128) * (32 + i) + str2[i] - 1;
		i ++;
	}
	return hash;
}
#endif
