/*
 * togo_string.c
 *
 *  Created on: 2015-7-28
 *      Author: zhuli
 */

#include "togo.h"
#include "togo_load.h"

TOGO_STRING * togo_string_init(TOGO_POOL * pool, size_t size)
{
	TOGO_STRING * togo_str;
	size_t all_size;
	void * p;

	all_size = sizeof(TOGO_STRING) + size;
	p = togo_pool_alloc(pool, all_size);
	if (p == NULL) {
		return NULL;
	}

	togo_str = (TOGO_STRING *) p;
	togo_str->buf = (u_char *) (p + sizeof(TOGO_STRING));
	togo_str->buf_size = size;
	togo_str->pool = pool;
	togo_str->str_size = 0;

	return togo_str;
}

void togo_string_append(TOGO_STRING ** togo_str, u_char * str, size_t len)
{
	size_t space;
	u_char * curr;

	if ((*togo_str) == NULL || str == NULL || len == 0) {
		return;
	}

	space = (*togo_str)->buf_size - (*togo_str)->str_size;

	/* There is no enough space, Alloc a new TOGO_STRING */
	if (space < len) {

		size_t old_size, new_size, str_size;
		TOGO_POOL * pool;
		TOGO_STRING * new_str;
		void * p;

		str_size = (*togo_str)->str_size;
		pool = (*togo_str)->pool;
		old_size = sizeof(TOGO_STRING) + (*togo_str)->buf_size;
		new_size = old_size * 2;
		p = togo_pool_realloc((*togo_str)->pool, (void *) (*togo_str), old_size,
				new_size);
		if (p == NULL) {
			return;
		}

		new_str = (TOGO_STRING *) p;
		new_str->buf = (u_char *) (p + sizeof(TOGO_STRING));
		new_str->buf_size = new_size;
		new_str->pool = pool;
		new_str->str_size = str_size;

		(*togo_str) = new_str;
	}

	curr = (u_char *) ((*togo_str)->buf + (*togo_str)->str_size);
	togo_memcpy(curr, str, len);
	(*togo_str)->str_size += len;

	return;
}

void togo_string_append_i(TOGO_STRING ** togo_str, uint32_t val) {
	char int_str[12];
	togo_itoa(val, int_str, 10);
	togo_string_append(togo_str, int_str, togo_strlen(int_str));
	return;
}

void togo_string_append_s(TOGO_STRING ** togo_str, u_char * str) {
	togo_string_append(togo_str, str, togo_strlen(str));
	return;
}

void togo_string_destroy(TOGO_STRING * togo_str)
{
	togo_pool_free_data(togo_str->pool, (void *) togo_str);
}

int togo_encode_base64(u_char *dst, u_char *src)
{
	u_char *d, *s;
	size_t len;
	static u_char basis64[] =
			"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

	len = togo_strlen(src);
	s = src;
	d = dst;

	while (len > 2) {
		*d++ = basis64[(s[0] >> 2) & 0x3f];
		*d++ = basis64[((s[0] & 3) << 4) | (s[1] >> 4)];
		*d++ = basis64[((s[1] & 0x0f) << 2) | (s[2] >> 6)];
		*d++ = basis64[s[2] & 0x3f];

		s += 3;
		len -= 3;
	}

	if (len) {
		*d++ = basis64[(s[0] >> 2) & 0x3f];

		if (len == 1) {
			*d++ = basis64[(s[0] & 3) << 4];
			*d++ = '=';

		} else {
			*d++ = basis64[((s[0] & 3) << 4) | (s[1] >> 4)];
			*d++ = basis64[(s[1] & 0x0f) << 2];
		}

		*d++ = '=';
	}

	return d - dst;
}

int togo_decode_base64(u_char *dst, u_char *src)
{
	static u_char basis[] = { 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
			77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
			77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 62, 77, 77,
			77, 63, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 77, 77, 77, 77, 77,
			77, 77, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
			17, 18, 19, 20, 21, 22, 23, 24, 25, 77, 77, 77, 77, 77, 77, 26, 27,
			28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44,
			45, 46, 47, 48, 49, 50, 51, 77, 77, 77, 77, 77,

			77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
			77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
			77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
			77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
			77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
			77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
			77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
			77, 77, 77, 77, 77, 77, 77, 77, 77 };

	size_t len;
	u_char *d, *s;

	int src_len = togo_strlen(src);

	for (len = 0; len < src_len; len++) {
		if (src[len] == '=') {
			break;
		}

		if (basis[src[len]] == 77) {
			return -1;
		}
	}

	if (len % 4 == 1) {
		return -1;
	}

	s = src;
	d = dst;

	while (len > 3) {
		*d++ = (u_char)(basis[s[0]] << 2 | basis[s[1]] >> 4);
		*d++ = (u_char)(basis[s[1]] << 4 | basis[s[2]] >> 2);
		*d++ = (u_char)(basis[s[2]] << 6 | basis[s[3]]);

		s += 4;
		len -= 4;
	}

	if (len > 1) {
		*d++ = (u_char)(basis[s[0]] << 2 | basis[s[1]] >> 4);
	}

	if (len > 2) {
		*d++ = (u_char)(basis[s[1]] << 4 | basis[s[2]] >> 2);
	}

	return d - dst;
}

int togo_atofp(u_char *line, size_t n, size_t point)
{
	int value;
	int dot;

	if (n == 0) {
		return -1;
	}
	dot = 0;

	for (value = 0; n--; line++) {

		if (point == 0) {
			return -1;
		}
		if (*line == '.') {
			if (dot) {
				return -1;
			}
			dot = 1;
			continue;
		}

		if (*line < '0' || *line > '9') {
			return -1;
		}

		value = value * 10 + (*line - '0');
		point -= dot;
	}

	while (point--) {
		value = value * 10;
	}

	if (value < 0) {
		return -1;
	} else {
		return value;
	}
}

int togo_atoi(u_char *line, size_t n)
{
	int value;

	if (n == 0) {
		return -1;
	}

	for (value = 0; n--; line++) {
		if (*line < '0' || *line > '9') {
			return -1;
		}

		value = value * 10 + (*line - '0');
	}

	if (value < 0) {
		return -1;

	} else {
		return value;
	}
}

u_char * togo_itoa(int num, u_char * str, int radix)
{
	u_char index[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	unsigned unum;
	int i = 0, j, k;

	if (radix == 10 && num < 0) {
		unum = (unsigned) -num;
		str[i++] = '-';
	} else
		unum = (unsigned) num;
	do {
		str[i++] = index[unum % (unsigned) radix];
		unum /= radix;
	} while (unum);
	str[i] = '\0';
	if (str[0] == '-')
		k = 1;
	else
		k = 0;
	u_char temp;
	for (j = k; j <= (i - 1) / 2; j++) {
		temp = str[j];
		str[j] = str[i - 1 + k - j];
		str[i - 1 + k - j] = temp;
	}
	return str;
}

u_char *togo_cpystrn(u_char *dst, u_char *src, size_t n)
{
	if (n == 0) {
		return dst;
	}

	do {
		if (n == 0) {
			return dst;
		}

		*dst = *src;

		if (*dst == '\0') {
			return dst;
		}

		dst++;
		src++;
	} while (--n);

	*dst = '\0';

	return dst;
}

u_char * togo_strtolower(u_char *s)
{
	int i, len = togo_strlen(s);
	for (i = 0; i < len; i++) {
		s[i] = togo_tolower(s[i]);
	}
	return (s);
}

u_char * togo_strtoupper(u_char *s)
{
	int i, len = togo_strlen(s);
	for (i = 0; i < len; i++) {
		s[i] = togo_toupper(s[i]);
	}
	return (s);
}

int togo_strpos(const u_char *s, u_char c)
{
	int i, len;
	if (!s || !c)
		return -1;
	len = strlen(s);
	for (i = 0; i < len; i++) {
		if (s[i] == c)
			return i;
	}
	return -1;
}

int togo_strrpos(const u_char *s, u_char c)
{
	int i, len;
	if (!s || !c)
		return -1;
	len = strlen(s);
	for (i = len; i >= 0; i--) {
		if (s[i] == c)
			return i;
	}
	return -1;
}

u_char *togo_trim(u_char *str)
{
	u_char *pfirst = str;
	u_char *plast = NULL;
	if (pfirst) {
		plast = pfirst + strlen(str) - 1;
		while (*pfirst && isspace(*pfirst)) {
			pfirst++;
		}
		while (plast > pfirst && isspace(*plast)) {
			*plast-- = '\0';
		}
	}
	return pfirst;
}

u_char *togo_ltrim(u_char *str)
{
	u_char *pfirst = str;
	if (pfirst) {
		while (*pfirst && isspace(*pfirst)) {
			pfirst++;
		}
	}
	return pfirst;
}

u_char *togo_rtrim(u_char *str)
{
	u_char *pfirst = str;
	u_char *plast = NULL;
	if (pfirst) {
		plast = pfirst + strlen(str) - 1;
		while (plast > pfirst && isspace(*plast)) {
			*plast-- = '\0';
		}
	}
	return pfirst;
}
