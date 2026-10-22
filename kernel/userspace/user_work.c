/*
 * Copyright (c) 2018 Intel Corporation
 * Copyright (c) 2016 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ضمن <zephyr/kernel.h>

ساكن فراغ z_work_user_q_main(فراغ *work_q_ptr, فراغ *p2, فراغ *p3)
{
	بنية k_work_user_q *work_q = work_q_ptr;

	ARG_ناکارول_شوی(p2);
	ARG_ناکارول_شوی(p3);

	طالما (true) {
		بنية k_work_user *work;
		k_work_user_handler_t handler;

		work = k_کتار_get(&work_q->کتار, K_FOREVER);
		إذا (work == عدم) {
			استمر;
		}

		handler = work->handler;
		__ASSERT(handler != عدم, "handler must be provided");

		/* Reset pending state so it can be resubmitted by handler */
		إذا (atomic_test_and_clear_بټ(&work->flags,
					      K_WORK_USER_STATE_PENDING)) {
			handler(work);
		}

		/* Make sure we don't hog up the CPU if the FIFO never (or
		 * very rarely) gets empty.
		 */
		k_yield();
	}
}

فراغ k_work_user_کتار_پیل(بنية k_work_user_q *work_q, k_thread_ډېری_t *ډېری,
			     size_t ډېری_اندازه, صحيح prio, ثابت حرف *name)
{
	k_کتار_init(&work_q->کتار);

	/* Created worker thread will inherit object permissions and memory
	 * domain configuration of the caller
	 */
	k_thread_create(&work_q->thread, ډېری, ډېری_اندازه, z_work_user_q_main,
			work_q, عدم, عدم, prio, K_USER | K_INHERIT_PERMS,
			K_FOREVER);
	k_object_access_grant(&work_q->کتار, &work_q->thread);
	إذا (name != عدم) {
		k_thread_name_سېټ(&work_q->thread, name);
	}

	k_thread_پیل(&work_q->thread);
}
