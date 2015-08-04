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

	if (command_tag[0].value == NULL) {
		return;

		/* module Q */
	} else if (strcmp(command_tag[0].value, "Q") == 0) {

		togo_m_queue_command(command_tag, socket_item);
	}

}
