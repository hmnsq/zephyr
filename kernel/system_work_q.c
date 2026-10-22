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

#ಅನ್ತರ್ಭಾವ <zephyr/kernel.h>
#ಅನ್ತರ್ಭಾವ <kernel_internal.h>

ಸ್ಥಿರ K_KERNEL_ರಾಶಿ_DEFINE(sys_work_q_ರಾಶಿ,
			     CONFIG_SYSTEM_WORKQUEUE_ರಾಶಿ_ಗಾತ್ರ);

ರಚನೆ k_work_q k_sys_work_q;

ಸ್ಥಿರ ಶೂನ್ಯ sys_work_q_init(ಶೂನ್ಯ)
{
	ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ರಚನೆ k_work_ಸರತಿ_config cfg = {
		.name = "sysworkq",
		.no_yield = IS_ENABLED(CONFIG_SYSTEM_WORKQUEUE_NO_YIELD),
		.essential = true,
		.work_timeout_ms = CONFIG_SYSTEM_WORKQUEUE_WORK_TIMEOUT_MS,
	};

	k_work_ಸರತಿ_ಆರಂಭ(&k_sys_work_q,
			    sys_work_q_ರಾಶಿ,
			    K_KERNEL_ರಾಶಿ_SIZEOF(sys_work_q_ರಾಶಿ),
			    CONFIG_SYSTEM_WORKQUEUE_ಆದ್ಯತೆ, &cfg);
}

/* Registered into the kernel post-init section. The entry lives in this TU,
 * so it (and this init) is linked only when something references the system
 * work queue (e.g. k_sys_work_q), preserving pay-per-use linkage.
 */
K_KERNEL_INIT_POST(sys_work_q_init);
