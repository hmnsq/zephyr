/*
 * Copyright (c) 2018 Intel Corporation
 * Copyright (c) 2016 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#شامل <zephyr/kernel.h>

ساکن خالی z_work_user_q_main(خالی *work_q_ptr, خالی *p2, خالی *p3)
{
	ساخت k_work_user_q *work_q = work_q_ptr;

	ARG_غیر_مستعمل(p2);
	ARG_غیر_مستعمل(p3);

	جب (true) {
		ساخت k_work_user *work;
		k_work_user_handler_t handler;

		work = k_قطار_get(&work_q->قطار, K_FOREVER);
		اگر (work == تہی) {
			جاری;
		}

		handler = work->handler;
		__ASSERT(handler != تہی, "handler must be provided");

		/* Reset pending state so it can be resubmitted by handler */
		اگر (atomic_test_and_clear_ثنائی_ہندسہ(&work->flags,
					      K_WORK_USER_STATE_PENDING)) {
			handler(work);
		}

		/* Make sure we don't hog up the CPU if the FIFO never (or
		 * very rarely) gets empty.
		 */
		k_yield();
	}
}

خالی k_work_user_قطار_آغاز(ساخت k_work_user_q *work_q, k_thread_ڈھیر_t *ڈھیر,
			     size_t ڈھیر_سائز, صحیح prio, ثابت حرف *name)
{
	k_قطار_init(&work_q->قطار);

	/* Created worker thread will inherit object permissions and memory
	 * domain configuration of the caller
	 */
	k_thread_create(&work_q->thread, ڈھیر, ڈھیر_سائز, z_work_user_q_main,
			work_q, تہی, تہی, prio, K_USER | K_INHERIT_PERMS,
			K_FOREVER);
	k_object_access_grant(&work_q->قطار, &work_q->thread);
	اگر (name != تہی) {
		k_thread_name_سیٹ(&work_q->thread, name);
	}

	k_thread_آغاز(&work_q->thread);
}
