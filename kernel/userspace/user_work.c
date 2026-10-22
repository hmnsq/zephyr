/*
 * Copyright (c) 2018 Intel Corporation
 * Copyright (c) 2016 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#அந்தர்பாவ <zephyr/kernel.h>

நிலையான வெறுமை z_work_user_q_main(வெறுமை *work_q_ptr, வெறுமை *p2, வெறுமை *p3)
{
	கட்டமைப்பு k_work_user_q *work_q = work_q_ptr;

	ARG_UNUSED(p2);
	ARG_UNUSED(p3);

	போது (true) {
		கட்டமைப்பு k_work_user *work;
		k_work_user_handler_t handler;

		work = k_queue_get(&work_q->queue, K_FOREVER);
		எனில் (work == வெற்று) {
			தொடர்;
		}

		handler = work->handler;
		__ASSERT(handler != வெற்று, "handler must be provided");

		/* Reset pending state so it can be resubmitted by handler */
		எனில் (atomic_test_and_clear_bit(&work->flags,
					      K_WORK_USER_STATE_PENDING)) {
			handler(work);
		}

		/* Make sure we don't hog up the CPU if the FIFO never (or
		 * very rarely) gets empty.
		 */
		k_yield();
	}
}

வெறுமை k_work_user_queue_start(கட்டமைப்பு k_work_user_q *work_q, k_thread_அடுக்கு_t *அடுக்கு,
			     size_t அடுக்கு_size, முழு prio, மாறிலி எழுத்து *name)
{
	k_queue_init(&work_q->queue);

	/* Created worker thread will inherit object permissions and memory
	 * domain configuration of the caller
	 */
	k_thread_create(&work_q->thread, அடுக்கு, அடுக்கு_size, z_work_user_q_main,
			work_q, வெற்று, வெற்று, prio, K_USER | K_INHERIT_PERMS,
			K_FOREVER);
	k_object_access_grant(&work_q->queue, &work_q->thread);
	எனில் (name != வெற்று) {
		k_thread_name_set(&work_q->thread, name);
	}

	k_thread_start(&work_q->thread);
}
