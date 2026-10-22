/*
 * Copyright (c) 2018,2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/kernel.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <ksched.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/sys/math_extras.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/sys/dlist.h>

bool z_priq_rb_lessthan(ਬਣਤਰ rbnode *a, ਬਣਤਰ rbnode *b)
{
	ਬਣਤਰ k_thread *thread_a, *thread_b;
	int32_t cmp;

	thread_a = CONTAINER_ਦਾ(a, ਬਣਤਰ k_thread, ਅਧਾਰ.qnode_rb);
	thread_b = CONTAINER_ਦਾ(b, ਬਣਤਰ k_thread, ਅਧਾਰ.qnode_rb);

	cmp = z_sched_prio_cmp(thread_a, thread_b);

	ਜੇ (cmp > 0) {
		ਵਾਪਸ true;
	} ਨਹੀਂਤਾਂ ਜੇ (cmp < 0) {
		ਵਾਪਸ false;
	} ਨਹੀਂਤਾਂ {
		ਵਾਪਸ thread_a->ਅਧਾਰ.ਕ੍ਰਮ_ਕੁੰਜੀ < thread_b->ਅਧਾਰ.ਕ੍ਰਮ_ਕੁੰਜੀ
			? 1 : 0;
	}
}
