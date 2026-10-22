/*
 * Copyright (c) 2018,2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#अन्तर्भाव <zephyr/kernel.h>
#अन्तर्भाव <ksched.h>
#अन्तर्भाव <zephyr/sys/math_extras.h>
#अन्तर्भाव <zephyr/sys/dlist.h>

bool z_priq_rb_lessthan(संरचना rbnode *a, संरचना rbnode *b)
{
	संरचना k_thread *thread_a, *thread_b;
	int32_t cmp;

	thread_a = CONTAINER_OF(a, संरचना k_thread, base.qnode_rb);
	thread_b = CONTAINER_OF(b, संरचना k_thread, base.qnode_rb);

	cmp = z_sched_prio_cmp(thread_a, thread_b);

	जइ (cmp > 0) {
		प्रत्यागम true;
	} अण्णहा जइ (cmp < 0) {
		प्रत्यागम false;
	} अण्णहा {
		प्रत्यागम thread_a->base.order_कुञ्चिका < thread_b->base.order_कुञ्चिका
			? 1 : 0;
	}
}
