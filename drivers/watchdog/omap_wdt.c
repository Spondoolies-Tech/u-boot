/*
 * U-Boot OMAP watchdog driver.
 * Written by Vladik Goytin
 *
 * Based on Linux drivers/watchdog/omap_wdt.c
 * ==========================================
 * omap_wdt.c
 *
 * Watchdog driver for the TI OMAP 16xx & 24xx/34xx 32KHz (non-secure) watchdog
 *
 * Author: MontaVista Software, Inc.
 *	 <gdavis@mvista.com> or <source@mvista.com>
 *
 * 2003 (c) MontaVista Software, Inc. This file is licensed under the
 * terms of the GNU General Public License version 2. This program is
 * licensed "as is" without any warranty of any kind, whether express
 * or implied.
 *
 * History:
 *
 * 20030527: George G. Davis <gdavis@mvista.com>
 *	Initially based on linux-2.4.19-rmk7-pxa1/drivers/char/sa1100_wdt.c
 *	(c) Copyright 2000 Oleg Drokin <green@crimea.edu>
 *	Based on SoftDog driver by Alan Cox <alan@lxorguk.ukuu.org.uk>
 *
 * Copyright (c) 2004 Texas Instruments.
 *	1. Modified to support OMAP1610 32-KHz watchdog timer
 *	2. Ported to 2.6 kernel
 *
 * Copyright (c) 2005 David Brownell
 *	Use the driver model and standard identifiers; handle bigger timeouts.
 */


#include <common.h>
#include <asm/io.h>
#include <asm/processor.h>
#include <watchdog.h>

#include "omap_wdt.h"


static void __iomem		*base = (void __iomem *)CONFIG_WATCHDOG_BASEADDR;


static void omap_wdt_reload(void)
{
	/* wait for posted write to complete */
	while ((__raw_readl(base + OMAP_WATCHDOG_WPS)) & 0x08)
		cpu_relax();

	/* OMAP35x Applications Processor TRM, Table 16-85. WTGR, p.2658:
	 *	Writing a different value than the one already written
	 *	in this register causes a watchdog counter reload.
	 */
	__raw_writel( ~__raw_readl(base + OMAP_WATCHDOG_TGR),
		base + OMAP_WATCHDOG_TGR);

	/* wait for posted write to complete */
	while ((__raw_readl(base + OMAP_WATCHDOG_WPS)) & 0x08)
		cpu_relax();
	/* reloaded WCRR from WLDR */
}

static void omap_wdt_enable(void)
{
	/* Sequence to enable the watchdog */
	__raw_writel(0xBBBB, base + OMAP_WATCHDOG_SPR);
	while ((__raw_readl(base + OMAP_WATCHDOG_WPS)) & 0x10)
		cpu_relax();

	__raw_writel(0x4444, base + OMAP_WATCHDOG_SPR);
	while ((__raw_readl(base + OMAP_WATCHDOG_WPS)) & 0x10)
		cpu_relax();
}

static void omap_wdt_disable(void)
{
	/* sequence required to disable watchdog */
	__raw_writel(0xAAAA, base + OMAP_WATCHDOG_SPR);	/* TIMER_MODE */
	while (__raw_readl(base + OMAP_WATCHDOG_WPS) & 0x10)
		cpu_relax();

	__raw_writel(0x5555, base + OMAP_WATCHDOG_SPR);	/* TIMER_MODE */
	while (__raw_readl(base + OMAP_WATCHDOG_WPS) & 0x10)
		cpu_relax();
}

static void omap_wdt_set_timer(unsigned int timeout)
{
	u32 pre_margin = GET_WLDR_VAL(timeout);

	/* just count up at 32 KHz */
	while (__raw_readl(base + OMAP_WATCHDOG_WPS) & 0x04)
		cpu_relax();

	__raw_writel(pre_margin, base + OMAP_WATCHDOG_LDR);
	while (__raw_readl(base + OMAP_WATCHDOG_WPS) & 0x04)
		cpu_relax();
}

static void omap_wdt_start(const unsigned timeout)
{
	/* initialize prescaler */
	while (__raw_readl(base + OMAP_WATCHDOG_WPS) & 0x01)
		cpu_relax();

	__raw_writel((1 << 5) | (PTV << 2), base + OMAP_WATCHDOG_CNTRL);
	while (__raw_readl(base + OMAP_WATCHDOG_WPS) & 0x01)
		cpu_relax();

	omap_wdt_set_timer(timeout);
	omap_wdt_reload(); /* trigger loading of new timeout value */
	omap_wdt_enable();
}

static void omap_wdt_stop(void)
{
	omap_wdt_disable();
}

void hw_watchdog_disable(void)
{
	omap_wdt_stop();
}

void hw_watchdog_init(void)
{
	omap_wdt_start(120);
	printf("\nWARNING: HW watchdog resets the system in 2 mins!\n"
		"         To disable watchdog issue command 'wdog off'\n\n");
}
