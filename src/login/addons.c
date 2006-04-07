// $Id$
/* Modules support... */

#include <config.h>

#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "../common/addons.h"
#include "../common/timer.h"
#include "login.h"
#include "addons.h"

void init_localcalltable(void) {
#ifdef DYNAMIC_LINKING
	local_table = malloc(LFNC_COUNT * 4);
	/* put here list of exported functions... */
	LFNC_SERVER(void);
	LFNC_SERVER_FD(void);
	LFNC_LOGIN_PORT(void);
	LFNC_GETTICK_CACHE(void);
#endif
}

