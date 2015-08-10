/*
 * togo_string.h
 *
 *  Created on: 2015-7-28
 *      Author: zhuli
 */

#ifndef TOGO_STRING_H_
#define TOGO_STRING_H_

#define togo_tolower(c)  (u_char) ((c >= 'A' && c <= 'Z') ? (c | 0x20) : c)
#define togo_toupper(c)  (u_char) ((c >= 'a' && c <= 'z') ? (c & ~0x20) : c)
#define togo_strcmp(s1, s2)  strcmp((const char *) s1, (const char *) s2)
#define togo_strncmp(s1, s2, n) strncmp((const char *) s1, (const char *) s2, n)
#define togo_strchr(s1, c)   strchr((const char *) s1, (int) c)
#define togo_strstr(s1, s2)  strstr((const char *) s1, (const char *) s2)
#define togo_strlen(s)       strlen((const char *) s)
#define togo_strcpy strcpy
#define togo_strcat strcat

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
