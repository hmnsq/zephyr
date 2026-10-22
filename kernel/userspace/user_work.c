/*
 * Copyright (c) 2018 Intel Corporation
 * Copyright (c) 2016 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#അന്തര്ഭാവ <zephyr/kernel.h>

സ്ഥിതം ശൂന്യം z_work_user_q_main(ശൂന്യം *work_q_ptr, ശൂന്യം *p2, ശൂന്യം *p3)
{
	ഘടന k_work_user_q *work_q = work_q_ptr;

	ARG_UNUSED(p2);
	ARG_UNUSED(p3);

	ആകുമ്പോൾ (true) {
		ഘടന k_work_user *work;
		k_work_user_handler_t handler;

		work = k_queue_get(&work_q->queue, K_FOREVER);
		എങ്കിൽ (work == ശൂന്യ) {
			തുടരുക;
		}

		handler = work->handler;
		__ASSERT(handler != ശൂന്യ, "handler must be provided");

		/* Reset pending state so it can be resubmitted by handler */
		എങ്കിൽ (atomic_test_and_clear_bit(&work->flags,
					      K_WORK_USER_STATE_PENDING)) {
			handler(work);
		}

		/* Make sure we don't hog up the CPU if the FIFO never (or
		 * very rarely) gets empty.
		 */
		k_yield();
	}
}

ശൂന്യം k_work_user_queue_start(ഘടന k_work_user_q *work_q, k_thread_അട്ടി_t *അട്ടി,
			     size_t അട്ടി_size, പൂർണ്ണസംഖ്യ prio, സ്ഥിരം അക്ഷരം *name)
{
	k_queue_init(&work_q->queue);

	/* Created worker thread will inherit object permissions and memory
	 * domain configuration of the caller
	 */
	k_thread_create(&work_q->thread, അട്ടി, അട്ടി_size, z_work_user_q_main,
			work_q, ശൂന്യ, ശൂന്യ, prio, K_USER | K_INHERIT_PERMS,
			K_FOREVER);
	k_object_access_grant(&work_q->queue, &work_q->thread);
	എങ്കിൽ (name != ശൂന്യ) {
		k_thread_name_set(&work_q->thread, name);
	}

	k_thread_start(&work_q->thread);
}
