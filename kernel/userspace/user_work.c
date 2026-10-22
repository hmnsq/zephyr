/*
 * Copyright (c) 2018 Intel Corporation
 * Copyright (c) 2016 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#شامل <zephyr/kernel.h>

ایستا تهی z_work_user_q_main(تهی *work_q_ptr, تهی *p2, تهی *p3)
{
	ساختار k_work_user_q *work_q = work_q_ptr;

	ARG_UNUSED(p2);
	ARG_UNUSED(p3);

	وقتی (true) {
		ساختار k_work_user *work;
		k_work_user_handler_t handler;

		work = k_queue_get(&work_q->queue, K_FOREVER);
		اگر (work == پوچ) {
			ادامه;
		}

		handler = work->handler;
		__ASSERT(handler != پوچ, "handler must be provided");

		/* Reset pending state so it can be resubmitted by handler */
		اگر (atomic_test_and_clear_bit(&work->flags,
					      K_WORK_USER_STATE_PENDING)) {
			handler(work);
		}

		/* Make sure we don't hog up the CPU if the FIFO never (or
		 * very rarely) gets empty.
		 */
		k_yield();
	}
}

تهی k_work_user_queue_start(ساختار k_work_user_q *work_q, k_thread_پشته_t *پشته,
			     size_t پشته_size, صحیح prio, ثابت نویسه *name)
{
	k_queue_init(&work_q->queue);

	/* Created worker thread will inherit object permissions and memory
	 * domain configuration of the caller
	 */
	k_thread_create(&work_q->thread, پشته, پشته_size, z_work_user_q_main,
			work_q, پوچ, پوچ, prio, K_USER | K_INHERIT_PERMS,
			K_FOREVER);
	k_object_access_grant(&work_q->queue, &work_q->thread);
	اگر (name != پوچ) {
		k_thread_name_set(&work_q->thread, name);
	}

	k_thread_start(&work_q->thread);
}
