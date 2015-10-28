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

	if (togo_strcmp(command_tag[0].value, "VERSION") == 0) {

		togo_send_data(socket_item, TOGO_VERSION, togo_strlen(TOGO_VERSION));
		ret = TRUE;

	} else if (togo_strcmp(command_tag[0].value, "QUIT") == 0) {

		togo_server_disconnect(socket_item);
		ret = TRUE;
	} else {

		/**
		 * command_tag[0] : Module
		 * command_tag[1] : Action
		 * command_tag[2] : Object
		 * command_tag[3] : Value
		 * command_tag[4] : Option
		 */
		if (ntag < 3 || command_tag[0].value == NULL) {
			ret = FALSE;

		} else if (togo_strcmp(command_tag[0].value, "QUEUE") == 0) {

			/**
			 * command_tag[0] : Module  QUEUE
			 * command_tag[1] : Action  RPUSH|LPUSH|LPOP|RPOP|COUNT|STATUS
			 * command_tag[2] : Object  Queue name
			 * command_tag[3] : Value   value
			 * command_tag[4] : Option  priority:1|2|3
			 */
			ret = togo_m_queue_command(command_tag, socket_item, ntag);

		} else if (togo_strcmp(command_tag[0].value, "COUNTER") == 0) {

			/**
			 * command_tag[0] : Module  COUNTER
			 * command_tag[1] : Action  PLUS|MINUS|GET|RESET
			 * command_tag[2] : Object  Count name
			 * command_tag[3] : Step    1  MAX:99999999
			 */
			ret = togo_m_counter_command(command_tag, socket_item, ntag);

		} else if (togo_strcmp(command_tag[0].value, "LOCK") == 0) {

			/**
			 * command_tag[0] : Module  LOCK
			 * command_tag[1] : Action  LOCK|UNLOCK|STATUS
			 * command_tag[2] : Object  Count name
			 */
			ret = togo_m_lock_command(command_tag, socket_item, ntag);
		}
	}

	/* If fail, We will return "TOGO_FAIL"! */
	if (ret == FALSE) {
		togo_send_fail(socket_item);
	}
}
