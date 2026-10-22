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

#அந்தர்பாவ <zephyr/kernel.h>
#அந்தர்பாவ <kernel_internal.h>

நிலையான K_KERNEL_அடுக்கு_DEFINE(sys_work_q_அடுக்கு,
			     CONFIG_SYSTEM_WORKQUEUE_அடுக்கு_SIZE);

கட்டமைப்பு k_work_q k_sys_work_q;

நிலையான வெறுமை sys_work_q_init(வெறுமை)
{
	நிலையான மாறிலி கட்டமைப்பு k_work_queue_config cfg = {
		.name = "sysworkq",
		.no_yield = IS_ENABLED(CONFIG_SYSTEM_WORKQUEUE_NO_YIELD),
		.essential = true,
		.work_timeout_ms = CONFIG_SYSTEM_WORKQUEUE_WORK_TIMEOUT_MS,
	};

	k_work_queue_start(&k_sys_work_q,
			    sys_work_q_அடுக்கு,
			    K_KERNEL_அடுக்கு_SIZEOF(sys_work_q_அடுக்கு),
			    CONFIG_SYSTEM_WORKQUEUE_PRIORITY, &cfg);
}

/* Registered into the kernel post-init section. The entry lives in this TU,
 * so it (and this init) is linked only when something references the system
 * work queue (e.g. k_sys_work_q), preserving pay-per-use linkage.
 */
K_KERNEL_INIT_POST(sys_work_q_init);
