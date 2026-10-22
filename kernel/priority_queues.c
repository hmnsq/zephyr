/*
 * Copyright (c) 2018,2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#અન્તર્ભાવ <zephyr/kernel.h>
#અન્તર્ભાવ <ksched.h>
#અન્તર્ભાવ <zephyr/sys/math_extras.h>
#અન્તર્ભાવ <zephyr/sys/dlist.h>

bool z_priq_rb_lessthan(સંરચના rbnode *a, સંરચના rbnode *b)
{
	સંરચના k_thread *thread_a, *thread_b;
	int32_t cmp;

	thread_a = CONTAINER_OF(a, સંરચના k_thread, base.qnode_rb);
	thread_b = CONTAINER_OF(b, સંરચના k_thread, base.qnode_rb);

	cmp = z_sched_prio_cmp(thread_a, thread_b);

	જો (cmp > 0) {
		પાછા true;
	} નહીંતર જો (cmp < 0) {
		પાછા false;
	} નહીંતર {
		પાછા thread_a->base.order_ચાવી < thread_b->base.order_ચાવી
			? 1 : 0;
	}
}
