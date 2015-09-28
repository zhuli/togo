/*
 * togo_string.h
 *
 *  Created on: 2015-7-28
 *      Author: zhuli
 */

#ifndef TOGO_STRING_H_
#define TOGO_STRING_H_

#define TOGO_STRING_DEFAULT_SIZE 256

#define togo_tolower(c)  (u_char) ((c >= 'A' && c <= 'Z') ? (c | 0x20) : c)
#define togo_toupper(c)  (u_char) ((c >= 'a' && c <= 'z') ? (c & ~0x20) : c)
#define togo_strcmp(s1, s2)  strcmp((const char *) s1, (const char *) s2)
#define togo_strncmp(s1, s2, n) strncmp((const char *) s1, (const char *) s2, n)
#define togo_strchr(s1, c)   strchr((const char *) s1, (int) c)
#define togo_strstr(s1, s2)  strstr((const char *) s1, (const char *) s2)
#define togo_strlen(s)       strlen((const char *) s)
#define togo_strcpy strcpy
#define togo_strcat strcat

typedef struct togo_string TOGO_STRING;
#define togo_string_get(buf)  (buf - sizeof(TOGO_STRING))

struct togo_string {
	size_t buf_size;
	size_t str_size;
	u_char * buf;
	TOGO_POOL * pool;
};

TOGO_STRING * togo_string_init(TOGO_POOL * pool, size_t size);
void togo_string_append(TOGO_STRING ** togo_str, u_char * str, size_t len);
void togo_string_append_i(TOGO_STRING ** togo_str, uint32_t val);
void togo_string_append_s(TOGO_STRING ** togo_str, u_char * str);
void togo_string_destroy(TOGO_STRING * togo_str);


int togo_encode_base64(u_char *dst, u_char *src);
int togo_decode_base64(u_char *dst, u_char *src);

/**
 * togo_atofp("10.5", 4, 2) return 1050
 */
int togo_atofp(u_char *line, size_t n, size_t point);

/**
 * togo_atoi("1024", 4) return 1024
 */
int togo_atoi(u_char *line, size_t n);

u_char * togo_itoa(int num, u_char * str, int radix);
u_char * togo_cpystrn(u_char *dst, u_char *src, size_t n);
u_char * togo_strtolower(u_char *s);
u_char * togo_strtoupper(u_char *s);
int togo_strpos(const u_char *s, u_char c);
int togo_strrpos(const u_char *s, u_char c);
u_char *togo_trim(u_char *str);
u_char *togo_ltrim(u_char *str);
u_char *togo_rtrim(u_char *str);

#endif /* TOGO_STRING_H_ */
