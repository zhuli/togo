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

	//togo_cpystrn(u_char *s1, char *s2, size_t len)
	togo_cpystrn(dst, "woshishen", 8);
	if (togo_strcmp(dst, "woshishe") == 0) {
		togo_log(DEBUG,
				"Testing function:togo_cpystrn .............................OK ");
	} else {
		togo_log(DEBUG,
				"Testing function:togo_cpystrn .............................FAIL %s",
				dst);
		togo_exit();
	}

	//togo_strtolower
	togo_cpystrn(dst, "WOSHISHEN", 9);
	char * x111 = togo_strtolower(dst);
	if (togo_strcmp(x111, "woshishen") == 0) {
		togo_log(DEBUG,
				"Testing function:togo_strtolower .............................OK ");
	} else {
		togo_log(DEBUG,
				"Testing function:togo_strtolower .............................FAIL ");
		togo_exit();
	}

	//togo_strtolower
	togo_cpystrn(dst, "woshishen", 9);
	char * x222 = togo_strtoupper(dst);
	if (togo_strcmp(x222, "WOSHISHEN") == 0) {
		togo_log(DEBUG,
				"Testing function:togo_strtoupper .............................OK ");
	} else {
		togo_log(DEBUG,
				"Testing function:togo_strtoupper .............................FAIL ");
		togo_exit();
	}

	//togo_strpos
	char * ddd = "woshishen";
	int ddds = togo_strpos(ddd, 'i');
	if (ddds == 4) {
		togo_log(DEBUG,
				"Testing function:togo_strpos .............................OK ");
	} else {
		togo_log(DEBUG,
				"Testing function:togo_strpos .............................FAIL ");
		togo_exit();
	}

	//togo_strrpos
	int ddds2 = togo_strrpos(ddd, 'i');
	if (ddds2 == 4) {
		togo_log(DEBUG,
				"Testing function:togo_strrpos .............................OK ");
	} else {
		togo_log(DEBUG,
				"Testing function:togo_strrpos .............................FAIL ");
		togo_exit();
	}

	//togo_trim
	togo_cpystrn(dst, "   woshishen   ", 15);
	char * dst1 = togo_trim(dst);
	if (togo_strcmp(dst1, "woshishen") == 0) {
		togo_log(DEBUG,
				"Testing function:togo_trim .............................OK");
	} else {
		togo_log(DEBUG,
				"Testing function:togo_trim .............................FAIL");
		togo_exit();
	}

	//togo_ltrim
	togo_cpystrn(dst, "   woshishen   ", 15);
	char * dst2 = togo_ltrim(dst);
	if (togo_strcmp(dst2, "woshishen   ") == 0) {
		togo_log(DEBUG,
				"Testing function:togo_ltrim .............................OK ");
	} else {
		togo_log(DEBUG,
				"Testing function:togo_ltrim .............................FAIL ");
		togo_exit();
	}

	//togo_rtrim
	togo_cpystrn(dst, "   woshishen   ", 15);
	char * dst3 = togo_rtrim(dst);
	if (togo_strcmp(dst3, "   woshishen") == 0) {
		togo_log(DEBUG,
				"Testing function:togo_rtrim .............................OK");
	} else {
		togo_log(DEBUG,
				"Testing function:togo_rtrim .............................FAIL ");
		togo_exit();
	}

	togo_log(DEBUG, "Testing togo_string.c end ============================");
}

