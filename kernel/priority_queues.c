/*
 * Copyright (c) 2018,2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ضمن <zephyr/kernel.h>
#ضمن <ksched.h>
#ضمن <zephyr/sys/math_extras.h>
#ضمن <zephyr/sys/dlist.h>

bool z_priq_rb_lessthan(بنية rbnode *a, بنية rbnode *b)
{
	بنية k_thread *thread_a, *thread_b;
	int32_t cmp;

	thread_a = CONTAINER_OF(a, بنية k_thread, base.qnode_rb);
	thread_b = CONTAINER_OF(b, بنية k_thread, base.qnode_rb);

	cmp = z_sched_prio_cmp(thread_a, thread_b);

	إذا (cmp > 0) {
		إرجاع true;
	} وإلا إذا (cmp < 0) {
		إرجاع false;
	} وإلا {
		إرجاع thread_a->base.order_مفتاح < thread_b->base.order_مفتاح
			? 1 : 0;
	}
}
