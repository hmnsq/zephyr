/*
 * Copyright (c) 2016 Wind River Systems, Inc.
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 *
 * System workqueue.
 */

#അന്തര്ഭാവ <zephyr/kernel.h>
#അന്തര്ഭാവ <kernel_internal.h>

സ്ഥിതം K_KERNEL_അട്ടി_DEFINE(sys_work_q_അട്ടി,
			     CONFIG_SYSTEM_WORKQUEUE_അട്ടി_SIZE);

ഘടന k_work_q k_sys_work_q;

സ്ഥിതം ശൂന്യം sys_work_q_init(ശൂന്യം)
{
	സ്ഥിതം സ്ഥിരം ഘടന k_work_queue_config cfg = {
		.name = "sysworkq",
		.no_yield = IS_ENABLED(CONFIG_SYSTEM_WORKQUEUE_NO_YIELD),
		.essential = true,
		.work_timeout_ms = CONFIG_SYSTEM_WORKQUEUE_WORK_TIMEOUT_MS,
	};

	k_work_queue_start(&k_sys_work_q,
			    sys_work_q_അട്ടി,
			    K_KERNEL_അട്ടി_SIZEOF(sys_work_q_അട്ടി),
			    CONFIG_SYSTEM_WORKQUEUE_PRIORITY, &cfg);
}

/* Registered into the kernel post-init section. The entry lives in this TU,
 * so it (and this init) is linked only when something references the system
 * work queue (e.g. k_sys_work_q), preserving pay-per-use linkage.
 */
K_KERNEL_INIT_POST(sys_work_q_init);
