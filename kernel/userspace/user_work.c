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

	ARG_UNUSED(p2);
	ARG_UNUSED(p3);

	طالما (true) {
		بنية k_work_user *work;
		k_work_user_handler_t handler;

		work = k_queue_get(&work_q->queue, K_FOREVER);
		𐡄𐡍 (work == عدم) {
			استمر;
		}

		handler = work->handler;
		__ASSERT(handler != عدم, "handler must be provided");

		/* Reset pending state so it can be resubmitted by handler */
		𐡄𐡍 (atomic_test_and_clear_bit(&work->flags,
					      K_WORK_USER_STATE_PENDING)) {
			handler(work);
		}

		/* Make sure we don't hog up the CPU if the FIFO never (or
		 * very rarely) gets empty.
		 */
		k_yield();
	}
}

فراغ k_work_user_queue_start(بنية k_work_user_q *work_q, k_thread_stack_t *stack,
			     size_t stack_size, صحيح prio, ثابت حرف *name)
{
	k_queue_init(&work_q->queue);

	/* Created worker thread will inherit object permissions and memory
	 * domain configuration of the caller
	 */
	k_thread_create(&work_q->thread, stack, stack_size, z_work_user_q_main,
			work_q, عدم, عدم, prio, K_USER | K_INHERIT_PERMS,
			K_FOREVER);
	k_object_access_grant(&work_q->queue, &work_q->thread);
	𐡄𐡍 (name != عدم) {
		k_thread_name_set(&work_q->thread, name);
	}

	k_thread_start(&work_q->thread);
}
