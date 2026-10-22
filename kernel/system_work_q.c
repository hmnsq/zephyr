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

#अन्तर्भाव <zephyr/kernel.h>
#अन्तर्भाव <kernel_internal.h>

स्थिर K_KERNEL_थाक_DEFINE(sys_work_q_थाक,
			     CONFIG_SYSTEM_WORKQUEUE_थाक_SIZE);

संरचना k_work_q k_sys_work_q;

स्थिर शून्य sys_work_q_init(शून्य)
{
	स्थिर अचर संरचना k_work_queue_config cfg = {
		.name = "sysworkq",
		.no_yield = IS_ENABLED(CONFIG_SYSTEM_WORKQUEUE_NO_YIELD),
		.essential = true,
		.work_timeout_ms = CONFIG_SYSTEM_WORKQUEUE_WORK_TIMEOUT_MS,
	};

	k_work_queue_start(&k_sys_work_q,
			    sys_work_q_थाक,
			    K_KERNEL_थाक_SIZEOF(sys_work_q_थाक),
			    CONFIG_SYSTEM_WORKQUEUE_PRIORITY, &cfg);
}

/* Registered into the kernel post-init section. The entry lives in this TU,
 * so it (and this init) is linked only when something references the system
 * work queue (e.g. k_sys_work_q), preserving pay-per-use linkage.
 */
K_KERNEL_INIT_POST(sys_work_q_init);
