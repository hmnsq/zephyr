/*
 * Copyright (c) 2018,2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ଅନ୍ତର୍ଭାଵ <zephyr/kernel.h>
#ଅନ୍ତର୍ଭାଵ <ksched.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/sys/math_extras.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/sys/dlist.h>

bool z_priq_rb_lessthan(ସଂରଚନା rbnode *a, ସଂରଚନା rbnode *b)
{
	ସଂରଚନା k_thread *thread_a, *thread_b;
	int32_t cmp;

	thread_a = CONTAINER_OF(a, ସଂରଚନା k_thread, base.qnode_rb);
	thread_b = CONTAINER_OF(b, ସଂରଚନା k_thread, base.qnode_rb);

	cmp = z_sched_prio_cmp(thread_a, thread_b);

	ଯଦି (cmp > 0) {
		ଫେରନ୍ତୁ true;
	} ଅନ୍ୟଥା ଯଦି (cmp < 0) {
		ଫେରନ୍ତୁ false;
	} ଅନ୍ୟଥା {
		ଫେରନ୍ତୁ thread_a->base.order_ଚାବି < thread_b->base.order_ଚାବି
			? 1 : 0;
	}
}
