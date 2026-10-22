/*
 * Copyright (c) 2018,2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#شامل <zephyr/kernel.h>
#شامل <ksched.h>
#شامل <zephyr/sys/math_extras.h>
#شامل <zephyr/sys/dlist.h>

bool z_priq_rb_lessthan(ساختار rbnode *a, ساختار rbnode *b)
{
	ساختار k_thread *thread_a, *thread_b;
	int32_t cmp;

	thread_a = CONTAINER_OF(a, ساختار k_thread, base.qnode_rb);
	thread_b = CONTAINER_OF(b, ساختار k_thread, base.qnode_rb);

	cmp = z_sched_prio_cmp(thread_a, thread_b);

	اگر (cmp > 0) {
		بازگشت true;
	} وگرنه اگر (cmp < 0) {
		بازگشت false;
	} وگرنه {
		بازگشت thread_a->base.order_کلید < thread_b->base.order_کلید
			? 1 : 0;
	}
}
