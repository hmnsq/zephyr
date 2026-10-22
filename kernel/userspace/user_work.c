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

	ARG_اڻ_استعمال_ٿيل(p2);
	ARG_اڻ_استعمال_ٿيل(p3);

	طالما (true) {
		بنية k_work_user *work;
		k_work_user_handler_t handler;

		work = k_قطار_get(&work_q->قطار, K_FOREVER);
		إذا (work == عدم) {
			استمر;
		}

		handler = work->handler;
		__ASSERT(handler != عدم, "handler must be provided");

		/* Reset pending state so it can be resubmitted by handler */
		إذا (atomic_test_and_clear_ٻٽو_انگ(&work->flags,
					      K_WORK_USER_STATE_PENDING)) {
			handler(work);
		}

		/* Make sure we don't hog up the CPU if the FIFO never (or
		 * very rarely) gets empty.
		 */
		k_yield();
	}
}

فراغ k_work_user_قطار_شروعات(بنية k_work_user_q *work_q, k_thread_ڍير_t *ڍير,
			     size_t ڍير_سائيز, صحيح prio, ثابت حرف *name)
{
	k_قطار_init(&work_q->قطار);

	/* Created worker thread will inherit object permissions and memory
	 * domain configuration of the caller
	 */
	k_thread_create(&work_q->thread, ڍير, ڍير_سائيز, z_work_user_q_main,
			work_q, عدم, عدم, prio, K_USER | K_INHERIT_PERMS,
			K_FOREVER);
	k_object_access_grant(&work_q->قطار, &work_q->thread);
	إذا (name != عدم) {
		k_thread_name_سيٽ(&work_q->thread, name);
	}

	k_thread_شروعات(&work_q->thread);
}
