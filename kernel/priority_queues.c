/*
 * Copyright (c) 2018,2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <ksched.h>
#include <zephyr/sys/math_extras.h>
#include <zephyr/sys/dlist.h>

bool z_priq_rb_lessthan(sa_nracanaa rbnode *a, sa_nracanaa rbnode *b)
{
	sa_nracanaa k_thread *thread_a, *thread_b;
	int32_t cmp;

	thread_a = CONTAINER_OF(a, sa_nracanaa k_thread, base.qnode_rb);
	thread_b = CONTAINER_OF(b, sa_nracanaa k_thread, base.qnode_rb);

	cmp = z_sched_prio_cmp(thread_a, thread_b);

	གལ་སྲིད (cmp > 0) {
		ལོག true;
	} དེ་མིན གལ་སྲིད (cmp < 0) {
		ལོག false;
	} དེ་མིན {
		ལོག thread_a->base.order_key < thread_b->base.order_key
			? 1 : 0;
	}
}
