/*
 * Copyright (c) 2018 Intel Corporation
 * Copyright (c) 2016 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#অন্তর্ভাব <zephyr/kernel.h>

স্থির শূন্য z_work_user_q_main(শূন্য *work_q_ptr, শূন্য *p2, শূন্য *p3)
{
	কাঠামো k_work_user_q *work_q = work_q_ptr;

	ARG_UNUSED(p2);
	ARG_UNUSED(p3);

	যখন (true) {
		কাঠামো k_work_user *work;
		k_work_user_handler_t handler;

		work = k_queue_get(&work_q->queue, K_FOREVER);
		যদি (work == ফাঁকা) {
			চালু;
		}

		handler = work->handler;
		__ASSERT(handler != ফাঁকা, "handler must be provided");

		/* Reset pending state so it can be resubmitted by handler */
		যদি (atomic_test_and_clear_bit(&work->flags,
					      K_WORK_USER_STATE_PENDING)) {
			handler(work);
		}

		/* Make sure we don't hog up the CPU if the FIFO never (or
		 * very rarely) gets empty.
		 */
		k_yield();
	}
}

শূন্য k_work_user_queue_start(কাঠামো k_work_user_q *work_q, k_thread_স্তূপ_t *স্তূপ,
			     size_t স্তূপ_আকার, পূর্ণসংখ্যা prio, ধ্রুবক অক্ষর *name)
{
	k_queue_init(&work_q->queue);

	/* Created worker thread will inherit object permissions and memory
	 * domain configuration of the caller
	 */
	k_thread_create(&work_q->thread, স্তূপ, স্তূপ_আকার, z_work_user_q_main,
			work_q, ফাঁকা, ফাঁকা, prio, K_USER | K_INHERIT_PERMS,
			K_FOREVER);
	k_object_access_grant(&work_q->queue, &work_q->thread);
	যদি (name != ফাঁকা) {
		k_thread_name_set(&work_q->thread, name);
	}

	k_thread_start(&work_q->thread);
}
