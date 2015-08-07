/*
 * test.c
 *
 *  Created on: 2015-7-28
 *      Author: zhuli
 */

#include "src/togo_init.h"

static void togo_string_test(void);

void togo_test()
{
	togo_string_test();
}

void togo_string_test()
{
	togo_log(DEBUG, "Testing togo_string.c start ============================");
	u_char c;
	char * dst = togo_alloc(100);
	char * str2 = togo_alloc(100);

	//togo_encode_base64
	int i;
	char * str = "SAODOSADO988766556asdsad";
	i = togo_encode_base64(dst, str);
	if (strcmp(dst, "U0FPRE9TQURPOTg4NzY2NTU2YXNkc2Fk") == 0
			&& strlen("U0FPRE9TQURPOTg4NzY2NTU2YXNkc2Fk") == i) {
		togo_log(DEBUG,
				"Testing function:togo_encode_base64 .............................OK");
	} else {
		togo_log(DEBUG,
				"Testing function:togo_encode_base64 .............................FAIL");
		togo_exit();
	}

	//togo_decode_base64
	i = togo_decode_base64(str2, dst);
	if (strcmp(str, str2) == 0 && strlen(str2) == i) {
		togo_log(DEBUG,
				"Testing function:togo_decode_base64 .............................OK ");
	} else {
		togo_log(DEBUG,
				"Testing function:togo_decode_base64 .............................FAIL ");
		togo_exit();
	}

	//togo_tolower
	c = togo_tolower('X');
	if (c == 'x') {
		togo_log(DEBUG,
				"Testing function:togo_tolower .............................OK ");
	} else {
		togo_log(DEBUG,
				"Testing function:togo_tolower .............................FAIL ");
		togo_exit();
	}

	//togo_toupper
	c = togo_toupper('a');
	if (c == 'A') {
		togo_log(DEBUG,
				"Testing function:togo_toupper .............................OK ");
	} else {
		togo_log(DEBUG,
				"Testing function:togo_toupper .............................FAIL ");
		togo_exit();
	}

	//togo_strcmp
	char * s1 = "woshishen";
	char * s2 = "woshishen";
	i = togo_strcmp(s1, s2);
	if (i == 0) {
		togo_log(DEBUG,
				"Testing function:togo_strcmp .............................OK ");
	} else {
		togo_log(DEBUG,
				"Testing function:togo_strcmp .............................FAIL ");
		togo_exit();
	}

	//togo_strncmp
	char * s11 = "woshishen";
	char * s22 = "woshidfdf";
	i = togo_strncmp(s11, s22, 5);
	if (i == 0) {
		togo_log(DEBUG,
				"Testing function:togo_strncmp .............................OK ");
	} else {
		togo_log(DEBUG,
				"Testing function:togo_strncmp .............................FAIL ");
		togo_exit();
	}

	//togo_strchr
	char * strchr_p = togo_strchr("woshishen", 's');
	if (*strchr_p == 's') {
		togo_log(DEBUG,
				"Testing function:togo_strchr .............................OK ");
	} else {
		togo_log(DEBUG,
				"Testing function:togo_strchr .............................FAIL ");
		togo_exit();
	}

	//togo_strstr
	char * strstr_p = togo_strstr("woshishen", "shi");
	if (*strstr_p == 's') {
		togo_log(DEBUG,
				"Testing function:togo_strstr .............................OK ");
	} else {
		togo_log(DEBUG,
				"Testing function:togo_strstr .............................FAIL ");
		togo_exit();
	}

	//togo_strlen
	i = togo_strlen("woshishen");
	if (i == 9) {
		togo_log(DEBUG,
				"Testing function:togo_strlen .............................OK ");
	} else {
		togo_log(DEBUG,
				"Testing function:togo_strlen .............................FAIL ");
		togo_exit();
	}

	//togo_atofp(u_char *line, size_t n, size_t point)
	i = togo_atofp("10.98", 5, 2);
	if (i == 1098) {
		togo_log(DEBUG,
				"Testing function:togo_atofp .............................OK ");
	} else {
		togo_log(DEBUG,
				"Testing function:togo_atofp .............................FAIL ");
		togo_exit();
	}

	//togo_atoi(u_char *line, size_t n);
	i = togo_atoi("1024", 4);
	if (i == 1024) {
		togo_log(DEBUG,
				"Testing function:togo_atoi .............................OK ");
	} else {
		togo_log(DEBUG,
				"Testing function:togo_atoi .............................FAIL ");
		togo_exit();
	}

	//togo_strnstr(u_char *s1, char *s2, size_t len)
	char * xxx = togo_strnstr("woshishen", "shi", 3);
	if (*xxx == 's') {
		togo_log(DEBUG,
				"Testing function:togo_strnstr .............................OK ");
	} else {
		togo_log(DEBUG,
				"Testing function:togo_strnstr .............................FAIL ");
		togo_exit();
	}

	togo_log(DEBUG, "Testing togo_string.c end ============================");
}

