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

	thread_a = CONTAINER_सम्बन्धि(a, संरचना k_thread, आधारः.qnode_rb);
	thread_b = CONTAINER_सम्बन्धि(b, संरचना k_thread, आधारः.qnode_rb);

	cmp = z_sched_prio_cmp(thread_a, thread_b);

	यदि (cmp > 0) {
		प्रत्यागम true;
	} अन्यथा यदि (cmp < 0) {
		प्रत्यागम false;
	} अन्यथा {
		प्रत्यागम thread_a->आधारः.क्रमः_कुञ्चिका < thread_b->आधारः.क्रमः_कुञ्चिका
			? 1 : 0;
	}
}
