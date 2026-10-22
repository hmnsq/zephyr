/*
 * Copyright (c) 2018,2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ಅನ್ತರ್ಭಾವ <zephyr/kernel.h>
#ಅನ್ತರ್ಭಾವ <ksched.h>
#ಅನ್ತರ್ಭಾವ <zephyr/sys/math_extras.h>
#ಅನ್ತರ್ಭಾವ <zephyr/sys/dlist.h>

bool z_priq_rb_lessthan(ಸಂರಚನಾ rbnode *a, ಸಂರಚನಾ rbnode *b)
{
	ಸಂರಚನಾ k_thread *thread_a, *thread_b;
	int32_t cmp;

	thread_a = CONTAINER_OF(a, ಸಂರಚನಾ k_thread, base.qnode_rb);
	thread_b = CONTAINER_OF(b, ಸಂರಚನಾ k_thread, base.qnode_rb);

	cmp = z_sched_prio_cmp(thread_a, thread_b);

	ಯದಿ (cmp > 0) {
		ಪ್ರತ್ಯಾಗಮ true;
	} ಅನ್ಯಥಾ ಯದಿ (cmp < 0) {
		ಪ್ರತ್ಯಾಗಮ false;
	} ಅನ್ಯಥಾ {
		ಪ್ರತ್ಯಾಗಮ thread_a->base.order_ಕೀಲಿ < thread_b->base.order_ಕೀಲಿ
			? 1 : 0;
	}
}
