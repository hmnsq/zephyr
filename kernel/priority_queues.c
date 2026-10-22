/*
 * Copyright (c) 2018,2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#అన్తర్భావ <zephyr/kernel.h>
#అన్తర్భావ <ksched.h>
#అన్తర్భావ <zephyr/sys/math_extras.h>
#అన్తర్భావ <zephyr/sys/dlist.h>

bool z_priq_rb_lessthan(నిర్మాణం rbnode *a, నిర్మాణం rbnode *b)
{
	నిర్మాణం k_thread *thread_a, *thread_b;
	int32_t cmp;

	thread_a = CONTAINER_యొక్క(a, నిర్మాణం k_thread, పునాది.qnode_rb);
	thread_b = CONTAINER_యొక్క(b, నిర్మాణం k_thread, పునాది.qnode_rb);

	cmp = z_sched_prio_cmp(thread_a, thread_b);

	ఒకవేళ (cmp > 0) {
		తిరిగి true;
	} లేకపోతే ఒకవేళ (cmp < 0) {
		తిరిగి false;
	} లేకపోతే {
		తిరిగి thread_a->పునాది.క్రమం_కీలకం < thread_b->పునాది.క్రమం_కీలకం
			? 1 : 0;
	}
}
