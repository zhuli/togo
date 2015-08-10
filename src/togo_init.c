/*
 * init.c
 *
 * Created on: 2015-6-12
 * Author: zhuli
 */
#include "togo.h"
#include "togo_load.h"
//#include "togo_test.c"

static void togo_argv_init(int argc, char *argv[]);
static void togo_config_init();
static void togo_c_init();
static void togo_daemon_init();
static void togo_pool_init();
static void togo_module_init();

void togo_init(int argc, char *argv[])
{
	//togo_test();
	togo_pool_init();
	togo_argv_init(argc, argv);
	togo_c_init();
	togo_daemon_init();
	togo_hashtable_init(togo_pool);
	togo_module_init();
	togo_server_init();
}

static void togo_config_init()
{
	BOOL ret = togo_read_config(TOGO_C_PATH);

	if (ret == FALSE) {

		BOOL retTry = togo_read_config(TOGO_C_DEFAULT);

		if (retTry == FALSE) {
			togo_log(ERROR, "Read config file Fail");
			togo_exit();
		}
	}
}

static void togo_argv_init(int argc, char *argv[])
{
	int oc;
	char ec;
	char * config_path = NULL;
	char * port = NULL;
	char * daemon = NULL;
	char * worker_thread_num = NULL;

	/* Process command-line arguments */
	while ((oc = getopt(argc, argv, "c:" /* Set configuration file */
			"p:" /* Set Tcp port*/
			"n:" /* Set the number of worker thread*/
			"d" /* Set the programs runs in the backgroud */
	)) != -1) {
		switch (oc) {
		case 'c':
			if (optarg != NULL) {
				config_path = optarg;
			}
			break;

		case 'p':
			if (optarg != NULL) {
				port = optarg;
			}
			break;

		case 'd':
			daemon = "1";
			break;

		case 'n':
			if (optarg != NULL) {
				worker_thread_num = optarg;
			}
			break;

		default:
			ec = (char) optopt;
			togo_log(ERROR, "Illegal argument.");
			togo_exit();
			break;

		}
	}

	/* Read the config file */
	if (config_path != NULL) {
		BOOL isRet = togo_read_config(config_path);
		if (isRet == FALSE) {
			togo_config_init(config_path);
		}
	} else {
		togo_config_init(config_path);
	}

	if (port != NULL) {
		togo_set_config(TOGO_C_PORT, port);
	}
	if (daemon != NULL) {
		togo_set_config(TOGO_C_DAEMON, "1");
	}
	if (worker_thread_num != NULL) {
		togo_set_config(TOGO_C_WTN, worker_thread_num);
	}
}

static void togo_c_init()
{
	char * version = togo_get_config(TOGO_C_VERSION);
	togo_c.version = version;

	char * ip = togo_get_config(TOGO_C_IP);
	togo_c.ip = ip;

	int port = togo_get_config_to_number(TOGO_C_PORT, TOGO_C_DEFAULT_PORT);
	togo_c.port = port;

	int daemon = togo_get_config_to_number(TOGO_C_DAEMON, TRUE);
	togo_c.daemon = daemon;

	int worker_thread_num = togo_get_config_to_number(TOGO_C_WTN,
			TOGO_C_DEFAULT_WTN);
	togo_c.worker_thread_num = worker_thread_num;
}

static void togo_daemon_init()
{
	if (togo_c.daemon == TRUE) {
		daemon(1, 1);
	}
}

static void togo_pool_init()
{
	togo_pool = togo_pool_create(togo_pool_size(TOGO_POOL_SIZE));
	togo_m_queue_init();
}

static void togo_module_init()
{
	togo_m_queue_init(); /* Queue module*/
}
