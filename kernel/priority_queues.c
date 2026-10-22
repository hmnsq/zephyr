/*
 * Copyright (c) 2018,2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ಅನ್ತರ್ಭಾವ <zephyr/kernel.h>
#ಅನ್ತರ್ಭಾವ <ksched.h>
#ಅನ್ತರ್ಭಾವ <zephyr/sys/math_extras.h>
#ಅನ್ತರ್ಭಾವ <zephyr/sys/dlist.h>

bool z_priq_rb_lessthan(ರಚನೆ rbnode *a, ರಚನೆ rbnode *b)
{
	ರಚನೆ k_thread *thread_a, *thread_b;
	int32_t cmp;

	thread_a = CONTAINER_ದ(a, ರಚನೆ k_thread, ತಳ.qnode_rb);
	thread_b = CONTAINER_ದ(b, ರಚನೆ k_thread, ತಳ.qnode_rb);

	cmp = z_sched_prio_cmp(thread_a, thread_b);

	ಆದರೆ (cmp > 0) {
		ಹಿಂತಿರುಗಿ true;
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (cmp < 0) {
		ಹಿಂತಿರುಗಿ false;
	} ಇಲ್ಲದಿದ್ದರೆ {
		ಹಿಂತಿರುಗಿ thread_a->ತಳ.ಕ್ರಮ_ಕೀಲಿ < thread_b->ತಳ.ಕ್ರಮ_ಕೀಲಿ
			? 1 : 0;
	}
}
