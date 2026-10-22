/*
 * Copyright (c) 2018,2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#অন্তর্ভাব <zephyr/kernel.h>
#অন্তর্ভাব <ksched.h>
#অন্তর্ভাব <zephyr/sys/math_extras.h>
#অন্তর্ভাব <zephyr/sys/dlist.h>

bool z_priq_rb_lessthan(সংরচনা rbnode *a, সংরচনা rbnode *b)
{
	সংরচনা k_thread *thread_a, *thread_b;
	int32_t cmp;

	thread_a = CONTAINER_OF(a, সংরচনা k_thread, base.qnode_rb);
	thread_b = CONTAINER_OF(b, সংরচনা k_thread, base.qnode_rb);

	cmp = z_sched_prio_cmp(thread_a, thread_b);

	যদি (cmp > 0) {
		প্রত্যাগম true;
	} অন্যথা যদি (cmp < 0) {
		প্রত্যাগম false;
	} অন্যথা {
		প্রত্যাগম thread_a->base.order_চাবি < thread_b->base.order_চাবি
			? 1 : 0;
	}
}
