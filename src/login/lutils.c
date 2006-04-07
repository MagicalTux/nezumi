// $Id$

/*------------------------------------------------------------------------
 Module:        Version 1.0.0 - Yor
 Author:        Freya Team Copyrights (c) 2004-2005
 Project:       Project Freya Account Server
 Creation Date: December 6, 2004
 Modified Date: January 8, 2005
 Description:   Ragnarok Online Server Emulator - General functions of login-server
------------------------------------------------------------------------*/

/* Include of configuration script */
#include <config.h>

/* Include of dependances */
#ifdef __WIN32
#define __USE_W32_SOCKETS
#else
#include <sys/time.h>
#endif

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdarg.h> // va_list
#include <stdint.h>

/* Includes of software */
#include <mmo.h>
#include "../common/timer.h" /* gettimeofday for win32 */
#include "lutils.h"
#include "login.h"

/*-----------------
  local variables
-----------------*/
static int log_fp = -1;
static char login_log_filename_used[sizeof(login_log_filename) + 64] = "1"; // +64 for date size
static char login_log_filename_to_use[sizeof(login_log_filename) + 64] = "2"; // must be different to login_log_filename_used

/*-----------------
  Close logs file
-----------------*/
void close_log(void) {
	if (log_fp != -1) {
		close(log_fp);
		log_fp = -1;
	}

	return;
}

/*-------------------------------
  Writing function of logs file
-------------------------------*/
void write_log(char *fmt, ...) {
	static int counter = 0;
	va_list ap;
	struct timeval tv;
	time_t now;
	char tmpstr[2048];
	int tmpstr_len;

	/* if we doesn't want to log */
	if (!log_login)
		return;

	// get time for file name and logs
	gettimeofday(&tv, NULL);
	now = time(NULL);

	// create file name
	memset(login_log_filename_to_use, 0, sizeof(login_log_filename_to_use));
	if (log_file_date == 0) {
		strcpy(login_log_filename_to_use, login_log_filename);
	} else {
		char* last_point;
		char* last_slash; // to avoid name like ../log_file_name_without_point
		// search position of '.'
		last_point = strrchr(login_log_filename, '.');
		last_slash = strrchr(login_log_filename, '/');
		if (last_point == NULL || (last_slash != NULL && last_slash > last_point))
			last_point = login_log_filename + strlen(login_log_filename);
		strncpy(login_log_filename_to_use, login_log_filename, last_point - login_log_filename);
		switch (log_file_date) {
		case 1:
			strftime(login_log_filename_to_use + strlen(login_log_filename_to_use), 63, "-%Y", localtime(&now));
			strcat(login_log_filename_to_use, last_point);
			break;
		case 2:
			strftime(login_log_filename_to_use + strlen(login_log_filename_to_use), 63, "-%m", localtime(&now));
			strcat(login_log_filename_to_use, last_point);
			break;
		case 3:
			strftime(login_log_filename_to_use + strlen(login_log_filename_to_use), 63, "-%Y-%m", localtime(&now));
			strcat(login_log_filename_to_use, last_point);
			break;
		case 4:
			strftime(login_log_filename_to_use + strlen(login_log_filename_to_use), 63, "-%Y-%m-%d", localtime(&now));
			strcat(login_log_filename_to_use, last_point);
			break;
		default: // case 0: 
			strcpy(login_log_filename_to_use, login_log_filename);
			break;
		}
	}

	// if previously used file has different name from the file to use
	if (strcmp(login_log_filename_used, login_log_filename_to_use) != 0) {
		close_log();
		counter = 0;
		memcpy(login_log_filename_used, login_log_filename_to_use, sizeof(login_log_filename_used));
	}

	/* if not already open, try to open log */
	if (log_fp == -1)
		log_fp = open(login_log_filename_used, O_WRONLY | O_CREAT | O_APPEND, 0644);

	if (log_fp != -1) {
		if (fmt[0] == '\0') /* jump a line if no message */
			write(log_fp, RETCODE, strlen(RETCODE));
		else {
			va_start(ap, fmt);
			memset(tmpstr, 0, sizeof(tmpstr));
			tmpstr_len = strftime(tmpstr, 20, date_format, localtime(&now)); // 19 + NULL
			tmpstr_len += sprintf(tmpstr + tmpstr_len, ".%03d: ", (int)tv.tv_usec / 1000);
			tmpstr_len += vsprintf(tmpstr + tmpstr_len, fmt, ap);
			write(log_fp, tmpstr, tmpstr_len);
			va_end(ap);
		}
		counter++;
		if ((counter % 100) == 99) {
			close_log(); /* under cygwin or windows, if software is stopped, data are not written in the file -> periodicaly close file */
			counter = 0;
		}
	}

	return;
}
