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

	thread_a = CONTAINER_از(a, بنية k_thread, پایه.qnode_rb);
	thread_b = CONTAINER_از(b, بنية k_thread, پایه.qnode_rb);

	cmp = z_sched_prio_cmp(thread_a, thread_b);

	إذا (cmp > 0) {
		أرجع true;
	} وإلا إذا (cmp < 0) {
		أرجع false;
	} وإلا {
		أرجع thread_a->پایه.ترتیب_کلید < thread_b->پایه.ترتیب_کلید
			? 1 : 0;
	}
}
