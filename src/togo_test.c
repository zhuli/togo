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
	}

	//togo_decode_base64
	i = togo_decode_base64(str2, dst);
	if (strcmp(str, str2) == 0 && strlen(str2) == i) {
		togo_log(DEBUG,
				"Testing function:togo_decode_base64 .............................OK ");
	}

	//togo_tolower
	c = togo_tolower('X');
	if (c == 'x') {
		togo_log(DEBUG,
				"Testing function:togo_tolower .............................OK ");
	}

	togo_log(DEBUG, "Testing togo_string.c end ============================");
}

