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

#అన్తర్భావ <zephyr/kernel.h>
#అన్తర్భావ <kernel_internal.h>

స్థిర K_KERNEL_దొంతర_DEFINE(sys_work_q_దొంతర,
			     CONFIG_SYSTEM_WORKQUEUE_దొంతర_కొలత);

నిర్మాణం k_work_q k_sys_work_q;

స్థిర శూన్య sys_work_q_init(శూన్య)
{
	స్థిర స్థిరరాశి నిర్మాణం k_work_వరుస_config cfg = {
		.name = "sysworkq",
		.no_yield = IS_ENABLED(CONFIG_SYSTEM_WORKQUEUE_NO_YIELD),
		.essential = true,
		.work_timeout_ms = CONFIG_SYSTEM_WORKQUEUE_WORK_TIMEOUT_MS,
	};

	k_work_వరుస_ప్రారంభం(&k_sys_work_q,
			    sys_work_q_దొంతర,
			    K_KERNEL_దొంతర_SIZEOF(sys_work_q_దొంతర),
			    CONFIG_SYSTEM_WORKQUEUE_ప్రాధాన్యత, &cfg);
}

/* Registered into the kernel post-init section. The entry lives in this TU,
 * so it (and this init) is linked only when something references the system
 * work queue (e.g. k_sys_work_q), preserving pay-per-use linkage.
 */
K_KERNEL_INIT_POST(sys_work_q_init);
