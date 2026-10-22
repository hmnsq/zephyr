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

#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/kernel.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <kernel_internal.h>

ਸਥਿਰ K_KERNEL_ਢੇਰ_DEFINE(sys_work_q_ਢੇਰ,
			     CONFIG_SYSTEM_WORKQUEUE_ਢੇਰ_ਮਾਪ);

ਬਣਤਰ k_work_q k_sys_work_q;

ਸਥਿਰ ਖਾਲੀ sys_work_q_init(ਖਾਲੀ)
{
	ਸਥਿਰ ਅਚਲ ਬਣਤਰ k_work_ਕਤਾਰ_config cfg = {
		.name = "sysworkq",
		.no_yield = IS_ENABLED(CONFIG_SYSTEM_WORKQUEUE_NO_YIELD),
		.essential = true,
		.work_timeout_ms = CONFIG_SYSTEM_WORKQUEUE_WORK_TIMEOUT_MS,
	};

	k_work_ਕਤਾਰ_ਸ਼ੁਰੂ(&k_sys_work_q,
			    sys_work_q_ਢੇਰ,
			    K_KERNEL_ਢੇਰ_SIZEOF(sys_work_q_ਢੇਰ),
			    CONFIG_SYSTEM_WORKQUEUE_ਤਰਜੀਹ, &cfg);
}

/* Registered into the kernel post-init section. The entry lives in this TU,
 * so it (and this init) is linked only when something references the system
 * work queue (e.g. k_sys_work_q), preserving pay-per-use linkage.
 */
K_KERNEL_INIT_POST(sys_work_q_init);
