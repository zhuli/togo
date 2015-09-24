/*
 * togo_dispatcher.c
 *
 *  Created on: 2015-6-23
 *      Author: zhuli
 */

#include "togo.h"
#include "togo_load.h"

void togo_dispatcher(TOGO_COMMAND_TAG command_tag[],
		TOGO_THREAD_ITEM *socket_item, int ntag)
{
	BOOL ret = FALSE;

	/**
	 * command_tag[0] : Module
	 * command_tag[1] : Action
	 * command_tag[2] : Object
	 * command_tag[3] : Value
	 * command_tag[4] : Option
	 */
	if (ntag < 3 || command_tag[0].value == NULL) {
		ret = FALSE;

		/* module Queue */
	} else if (strcmp(command_tag[0].value, "QUEUE") == 0) {

		ret = togo_m_queue_command(command_tag, socket_item, ntag);

		/* module Count */
	} else if (strcmp(command_tag[0].value, "COUNTER") == 0) {

		ret = togo_m_counter_command(command_tag, socket_item, ntag);
	}

	/* If fail, We will return "TOGO_FAIL"! */
	if (ret == FALSE) {
		togo_send_fail(socket_item);
	}
}
