/*
 * Watchdog management.
 *
 * Written by Vladik Goytin
 */

#ifndef CONFIG_SPL_BUILD

#include <common.h>

static int do_wdog(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int			rc = 0;

	/* need at least two arguments */
	if (argc >= 2) {
		const char	*cmd = argv[1];
		unsigned long	timeout;

		if (strcmp(cmd, "off") == 0)
			hw_watchdog_disable();
	}
	else
		rc = CMD_RET_USAGE;

	return rc;
}

U_BOOT_CMD(
	wdog,	2,	1,	do_wdog,
	"Watchdog management",
	"wdog off		- disable watchdog"
);

#endif
