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

	जर (cmp > 0) {
		परत true;
	} अन्यथा जर (cmp < 0) {
		परत false;
	} अन्यथा {
		परत thread_a->base.order_किल्ली < thread_b->base.order_किल्ली
			? 1 : 0;
	}
}
