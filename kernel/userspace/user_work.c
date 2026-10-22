/*
 * Copyright (c) 2018 Intel Corporation
 * Copyright (c) 2016 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#અન્તર્ભાવ <zephyr/kernel.h>

સ્થિર શૂન્ય z_work_user_q_main(શૂન્ય *work_q_ptr, શૂન્ય *p2, શૂન્ય *p3)
{
	સંરચના k_work_user_q *work_q = work_q_ptr;

	ARG_UNUSED(p2);
	ARG_UNUSED(p3);

	જ્યારે (true) {
		સંરચના k_work_user *work;
		k_work_user_handler_t handler;

		work = k_queue_get(&work_q->queue, K_FOREVER);
		જો (work == ખાલી) {
			ચાલુ;
		}

		handler = work->handler;
		__ASSERT(handler != ખાલી, "handler must be provided");

		/* Reset pending state so it can be resubmitted by handler */
		જો (atomic_test_and_clear_bit(&work->flags,
					      K_WORK_USER_STATE_PENDING)) {
			handler(work);
		}

		/* Make sure we don't hog up the CPU if the FIFO never (or
		 * very rarely) gets empty.
		 */
		k_yield();
	}
}

શૂન્ય k_work_user_queue_start(સંરચના k_work_user_q *work_q, k_thread_ઢગલો_t *ઢગલો,
			     size_t ઢગલો_size, પૂર્ણાંક prio, અચલ અક્ષર *name)
{
	k_queue_init(&work_q->queue);

	/* Created worker thread will inherit object permissions and memory
	 * domain configuration of the caller
	 */
	k_thread_create(&work_q->thread, ઢગલો, ઢગલો_size, z_work_user_q_main,
			work_q, ખાલી, ખાલી, prio, K_USER | K_INHERIT_PERMS,
			K_FOREVER);
	k_object_access_grant(&work_q->queue, &work_q->thread);
	જો (name != ખાલી) {
		k_thread_name_set(&work_q->thread, name);
	}

	k_thread_start(&work_q->thread);
}
