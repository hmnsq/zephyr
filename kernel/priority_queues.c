/*
 * Copyright (c) 2018,2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#അന്തര്ഭാവ <zephyr/kernel.h>
#അന്തര്ഭാവ <ksched.h>
#അന്തര്ഭാവ <zephyr/sys/math_extras.h>
#അന്തര്ഭാവ <zephyr/sys/dlist.h>

bool z_priq_rb_lessthan(ഘടന rbnode *a, ഘടന rbnode *b)
{
	ഘടന k_thread *thread_a, *thread_b;
	int32_t cmp;

	thread_a = CONTAINER_OF(a, ഘടന k_thread, base.qnode_rb);
	thread_b = CONTAINER_OF(b, ഘടന k_thread, base.qnode_rb);

	cmp = z_sched_prio_cmp(thread_a, thread_b);

	എങ്കിൽ (cmp > 0) {
		തിരികെ true;
	} അല്ലെങ്കിൽ എങ്കിൽ (cmp < 0) {
		തിരികെ false;
	} അല്ലെങ്കിൽ {
		തിരികെ thread_a->base.order_താക്കോൽ < thread_b->base.order_താക്കോൽ
			? 1 : 0;
	}
}
