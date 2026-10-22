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

#অন্তর্ভাৱ <zephyr/kernel.h>
#অন্তর্ভাৱ <kernel_internal.h>

স্থির K_KERNEL_স্তূপ_DEFINE(sys_work_q_স্তূপ,
			     CONFIG_SYSTEM_WORKQUEUE_স্তূপ_SIZE);

সংরচনা k_work_q k_sys_work_q;

স্থির শূন্য sys_work_q_init(শূন্য)
{
	স্থির ধ্রুৱ সংরচনা k_work_queue_config cfg = {
		.name = "sysworkq",
		.no_yield = IS_ENABLED(CONFIG_SYSTEM_WORKQUEUE_NO_YIELD),
		.essential = true,
		.work_timeout_ms = CONFIG_SYSTEM_WORKQUEUE_WORK_TIMEOUT_MS,
	};

	k_work_queue_start(&k_sys_work_q,
			    sys_work_q_স্তূপ,
			    K_KERNEL_স্তূপ_SIZEOF(sys_work_q_স্তূপ),
			    CONFIG_SYSTEM_WORKQUEUE_PRIORITY, &cfg);
}

/* Registered into the kernel post-init section. The entry lives in this TU,
 * so it (and this init) is linked only when something references the system
 * work queue (e.g. k_sys_work_q), preserving pay-per-use linkage.
 */
K_KERNEL_INIT_POST(sys_work_q_init);
