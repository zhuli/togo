/*
 * togo_dispatcher.c
 *
 *  Created on: 2015-6-23
 *      Author: zhuli
 */

#include "togo.h"
#include "togo_load.h"

void togo_dispatcher(TOGO_COMMAND_TAG command_tag[],
		TOGO_THREAD_ITEM *socket_item)
{
	BOOL ret = FALSE;

	if (command_tag[0].value == NULL) {
		ret = FALSE;
	}
	/* module Q */
	if (strcmp(command_tag[0].value, "Q") == 0) {

		ret = togo_m_queue_command(command_tag, socket_item);
	}

	/* If fail, We will return "FAIL"! */
	if (ret == FALSE) {
		togo_send_fail(socket_item);
	}
}
