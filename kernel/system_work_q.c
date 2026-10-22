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

#ଅନ୍ତର୍ଭାଵ <zephyr/kernel.h>
#ଅନ୍ତର୍ଭାଵ <kernel_internal.h>

ସ୍ଥିର K_KERNEL_ଗଦା_DEFINE(sys_work_q_ଗଦା,
			     CONFIG_SYSTEM_WORKQUEUE_ଗଦା_SIZE);

ସଂରଚନା k_work_q k_sys_work_q;

ସ୍ଥିର ଶୂନ୍ୟ sys_work_q_init(ଶୂନ୍ୟ)
{
	ସ୍ଥିର ଅଚଳ ସଂରଚନା k_work_queue_config cfg = {
		.name = "sysworkq",
		.no_yield = IS_ENABLED(CONFIG_SYSTEM_WORKQUEUE_NO_YIELD),
		.essential = true,
		.work_timeout_ms = CONFIG_SYSTEM_WORKQUEUE_WORK_TIMEOUT_MS,
	};

	k_work_queue_start(&k_sys_work_q,
			    sys_work_q_ଗଦା,
			    K_KERNEL_ଗଦା_SIZEOF(sys_work_q_ଗଦା),
			    CONFIG_SYSTEM_WORKQUEUE_PRIORITY, &cfg);
}

/* Registered into the kernel post-init section. The entry lives in this TU,
 * so it (and this init) is linked only when something references the system
 * work queue (e.g. k_sys_work_q), preserving pay-per-use linkage.
 */
K_KERNEL_INIT_POST(sys_work_q_init);
