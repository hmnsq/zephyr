/*
 * Copyright (c) 2018,2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#அந்தர்பாவ <zephyr/kernel.h>
#அந்தர்பாவ <ksched.h>
#அந்தர்பாவ <zephyr/sys/math_extras.h>
#அந்தர்பாவ <zephyr/sys/dlist.h>

bool z_priq_rb_lessthan(கட்டமைப்பு rbnode *a, கட்டமைப்பு rbnode *b)
{
	கட்டமைப்பு k_thread *thread_a, *thread_b;
	int32_t cmp;

	thread_a = CONTAINER_OF(a, கட்டமைப்பு k_thread, base.qnode_rb);
	thread_b = CONTAINER_OF(b, கட்டமைப்பு k_thread, base.qnode_rb);

	cmp = z_sched_prio_cmp(thread_a, thread_b);

	எனில் (cmp > 0) {
		திரும்பு true;
	} இல்லையெனில் எனில் (cmp < 0) {
		திரும்பு false;
	} இல்லையெனில் {
		திரும்பு thread_a->base.order_சாவி < thread_b->base.order_சாவி
			? 1 : 0;
	}
}
