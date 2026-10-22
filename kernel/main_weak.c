/*
 * Copyright (c) 2010-2014 Wind River Systems, Inc.
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/* Linkers may treat weak functions differently if they are located within
 * the same object that calls the symbol or not.
 *
 * For example, when using armlink, then if the weak symbol is inside the object
 * referring to it the weak symbol will be used. This will result in the symbol
 * being multiply defined because both the weak and strong symbols are used.
 *
 * To GNU ld, it doesn't matter if the weak symbol is placed in the same object
 * which uses the weak symbol. GNU ld will always link to the strong version.
 *
 * Having the weak main symbol in an independent file ensures that it will be
 * correctly treated by multiple linkers.
 */

#అన్తర్భావ <kernel_internal.h>

పూర్ణాంకం __weak ప్రధాన(శూన్య)
{
	/* NOP default main() if the application does not provide one. */
	arch_nop();

	తిరిగి 0;
}
