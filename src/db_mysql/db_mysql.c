// $Id$
// MySQL management functions
// Initial sourcecode by Gabuzomeu

#include <config.h>

#ifdef USE_MYSQL

/* Define that to get console output of queries */
#undef MYSQL_DEBUG

#include <stdio.h>
#include <time.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#ifdef __WIN32
#define __USE_W32_SOCKETS
#include <windows.h>
#endif

#ifndef __WIN32
# include <mysql.h>
#else
# include <mysql/mysql.h>
#endif

#include "../common/utils.h"
#include <nezumi_sql.h>

static char last_request[MAX_SQL_BUFFER];

static MYSQL mysql_handle;
static MYSQL_RES* mysql_db_res = NULL;
static MYSQL_ROW mysql_db_row = NULL;

void sql_init() {
#ifdef __DEBUG
	printf("Init mysql connection.\n");
#endif
	memset(last_request, 0, sizeof(last_request));
	mysql_init(&mysql_handle);
#ifdef CLIENT_INTERACTIVE
	if (!mysql_real_connect(&mysql_handle, db_mysql_server_ip, db_mysql_server_id, db_mysql_server_pw,
	    db_mysql_server_db, db_mysql_server_port, (char *)NULL, CLIENT_INTERACTIVE))
#else
	if (!mysql_real_connect(&mysql_handle, db_mysql_server_ip, db_mysql_server_id, db_mysql_server_pw,
	    db_mysql_server_db, db_mysql_server_port, (char *)NULL, 0))
#endif
	{
		/* pointer check */
		printf("Connect DB server error: %s.\n", mysql_error(&mysql_handle));
		exit(1);
	}

	return;
}

void sql_close(void) {
#ifdef __DEBUG
	printf("Closing mysql connection.\n");
#endif
	mysql_close(&mysql_handle);

	return;
}

char inbuf[MAX_SQL_BUFFER];
inline int sql_request(const char *format, ...) {
	int request_with_result;
	va_list args;

	if (format == NULL || *format == '\0')
		return 0;

	va_start(args, format);
	vsprintf(inbuf, format, args);
	va_end(args);

	request_with_result = (strncasecmp(inbuf, "SELECT", 6) == 0 ||
	                       strncasecmp(inbuf, "OPTIMIZE", 8) == 0 ||
	                       strncasecmp(inbuf, "SHOW", 4) == 0);
	if (request_with_result)
		if (mysql_db_res) {
			mysql_free_result(mysql_db_res);
			mysql_db_res = NULL;
			mysql_db_row = NULL;
		}

	strcpy(last_request, inbuf);

#ifdef MYSQL_DEBUG
	printf("Query: %s\n", inbuf);
#endif
	if (mysql_query(&mysql_handle, inbuf)) {
		printf("SQLERR: Req: %s, Error: %s \n", last_request, mysql_error(&mysql_handle));
		return 0;
	}

	if (request_with_result)
		mysql_db_res = mysql_store_result(&mysql_handle);

	return 1;
}

int sql_get_row(void) {
	if (! mysql_db_res)
		return 0;

	mysql_db_row = mysql_fetch_row(mysql_db_res);

	if (!mysql_db_row)
		return 0;

	return 1;
}

char *sql_get_string(int num_col) {
	if (! mysql_db_res)
		return NULL;

	if (!mysql_db_row) {
#ifdef __DEBUG
		printf("SQLERR: access to null sql row ? (col: %d), last req:%s\n", num_col, last_request);
#endif
		return NULL;
	}

	if (mysql_db_row[num_col])
		return mysql_db_row[num_col];

	return NULL;
}

unsigned long db_sql_escape_string(char *to, const char *from, unsigned long from_length) {
	return mysql_escape_string(to, from, from_length);
}

int sql_get_integer(int num_col) {
	if (! mysql_db_res)
		return -1;

	if (!mysql_db_row) {
#ifdef __DEBUG
		printf("SQLERR: access to null sql row ? (col: %d), last req:%s\n", num_col, last_request);
#endif
		return -1;
	}

	if (mysql_db_row[num_col])
		return atoi(mysql_db_row[num_col]);

	return 0;
}

unsigned long sql_num_rows(void) {
	if (mysql_db_res)
		return mysql_num_rows(mysql_db_res);

	return 0;
}
#endif /* USE_MYSQL */

