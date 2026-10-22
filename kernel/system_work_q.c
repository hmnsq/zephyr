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

#ضمن <zephyr/kernel.h>
#ضمن <kernel_internal.h>

ساكن K_KERNEL_ڍير_DEFINE(sys_work_q_ڍير,
			     CONFIG_SYSTEM_WORKQUEUE_ڍير_سائيز);

بنية k_work_q k_sys_work_q;

ساكن فراغ sys_work_q_init(فراغ)
{
	ساكن ثابت بنية k_work_قطار_config cfg = {
		.name = "sysworkq",
		.no_yield = IS_ENABLED(CONFIG_SYSTEM_WORKQUEUE_NO_YIELD),
		.essential = true,
		.work_timeout_ms = CONFIG_SYSTEM_WORKQUEUE_WORK_TIMEOUT_MS,
	};

	k_work_قطار_شروعات(&k_sys_work_q,
			    sys_work_q_ڍير,
			    K_KERNEL_ڍير_SIZEOF(sys_work_q_ڍير),
			    CONFIG_SYSTEM_WORKQUEUE_ترجيح, &cfg);
}

/* Registered into the kernel post-init section. The entry lives in this TU,
 * so it (and this init) is linked only when something references the system
 * work queue (e.g. k_sys_work_q), preserving pay-per-use linkage.
 */
K_KERNEL_INIT_POST(sys_work_q_init);
