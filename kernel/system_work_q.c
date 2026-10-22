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

#समावेश <zephyr/kernel.h>
#समावेश <kernel_internal.h>

अटल K_KERNEL_ढेर_DEFINE(sys_work_q_ढेर,
			     CONFIG_SYSTEM_WORKQUEUE_ढेर_आकार);

काष्ठा k_work_q k_sys_work_q;

अटल व्योम sys_work_q_init(व्योम)
{
	अटल स्थिर काष्ठा k_work_queue_config cfg = {
		.name = "sysworkq",
		.no_yield = IS_ENABLED(CONFIG_SYSTEM_WORKQUEUE_NO_YIELD),
		.essential = true,
		.work_timeout_ms = CONFIG_SYSTEM_WORKQUEUE_WORK_TIMEOUT_MS,
	};

	k_work_queue_start(&k_sys_work_q,
			    sys_work_q_ढेर,
			    K_KERNEL_ढेर_SIZEOF(sys_work_q_ढेर),
			    CONFIG_SYSTEM_WORKQUEUE_PRIORITY, &cfg);
}

/* Registered into the kernel post-init section. The entry lives in this TU,
 * so it (and this init) is linked only when something references the system
 * work queue (e.g. k_sys_work_q), preserving pay-per-use linkage.
 */
K_KERNEL_INIT_POST(sys_work_q_init);
