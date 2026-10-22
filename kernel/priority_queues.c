/*
 * Copyright (c) 2018,2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#समावेश <zephyr/kernel.h>
#समावेश <ksched.h>
#समावेश <zephyr/sys/math_extras.h>
#समावेश <zephyr/sys/dlist.h>

bool z_priq_rb_lessthan(काष्ठा rbnode *a, काष्ठा rbnode *b)
{
	काष्ठा k_thread *thread_a, *thread_b;
	int32_t cmp;

	thread_a = CONTAINER_OF(a, काष्ठा k_thread, base.qnode_rb);
	thread_b = CONTAINER_OF(b, काष्ठा k_thread, base.qnode_rb);

	cmp = z_sched_prio_cmp(thread_a, thread_b);

	अगर (cmp > 0) {
		वापस true;
	} अन्यथा अगर (cmp < 0) {
		वापस false;
	} अन्यथा {
		वापस thread_a->base.order_कुंजी < thread_b->base.order_कुंजी
			? 1 : 0;
	}
}
