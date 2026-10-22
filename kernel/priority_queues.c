/*
 * Copyright (c) 2018,2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#شامل <zephyr/kernel.h>
#شامل <ksched.h>
#شامل <zephyr/sys/math_extras.h>
#شامل <zephyr/sys/dlist.h>

bool z_priq_rb_lessthan(ساخت rbnode *a, ساخت rbnode *b)
{
	ساخت k_thread *thread_a, *thread_b;
	int32_t cmp;

	thread_a = CONTAINER_کا(a, ساخت k_thread, بنیاد.qnode_rb);
	thread_b = CONTAINER_کا(b, ساخت k_thread, بنیاد.qnode_rb);

	cmp = z_sched_prio_cmp(thread_a, thread_b);

	اگر (cmp > 0) {
		واپس true;
	} ورنہ اگر (cmp < 0) {
		واپس false;
	} ورنہ {
		واپس thread_a->بنیاد.ترتیب_کلید < thread_b->بنیاد.ترتیب_کلید
			? 1 : 0;
	}
}
