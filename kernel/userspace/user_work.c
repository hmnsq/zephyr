/*
 * Copyright (c) 2018 Intel Corporation
 * Copyright (c) 2016 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ಅನ್ತರ್ಭಾವ <zephyr/kernel.h>

ಸ್ಥಿರ ಶೂನ್ಯ z_work_user_q_main(ಶೂನ್ಯ *work_q_ptr, ಶೂನ್ಯ *p2, ಶೂನ್ಯ *p3)
{
	ಸಂರಚನಾ k_work_user_q *work_q = work_q_ptr;

	ARG_UNUSED(p2);
	ARG_UNUSED(p3);

	ಯಾವತ್ (true) {
		ಸಂರಚನಾ k_work_user *work;
		k_work_user_handler_t handler;

		work = k_queue_get(&work_q->queue, K_FOREVER);
		ಯದಿ (work == ರಿಕ್ತ) {
			ಅನುವರ್ತ;
		}

		handler = work->handler;
		__ASSERT(handler != ರಿಕ್ತ, "handler must be provided");

		/* Reset pending state so it can be resubmitted by handler */
		ಯದಿ (atomic_test_and_clear_bit(&work->flags,
					      K_WORK_USER_STATE_PENDING)) {
			handler(work);
		}

		/* Make sure we don't hog up the CPU if the FIFO never (or
		 * very rarely) gets empty.
		 */
		k_yield();
	}
}

ಶೂನ್ಯ k_work_user_queue_start(ಸಂರಚನಾ k_work_user_q *work_q, k_thread_ರಾಶಿ_t *ರಾಶಿ,
			     size_t ರಾಶಿ_size, ಪೂರ್ಣಾಙ್ಕ prio, ಧ್ರುವ ವರ್ಣ *name)
{
	k_queue_init(&work_q->queue);

	/* Created worker thread will inherit object permissions and memory
	 * domain configuration of the caller
	 */
	k_thread_create(&work_q->thread, ರಾಶಿ, ರಾಶಿ_size, z_work_user_q_main,
			work_q, ರಿಕ್ತ, ರಿಕ್ತ, prio, K_USER | K_INHERIT_PERMS,
			K_FOREVER);
	k_object_access_grant(&work_q->queue, &work_q->thread);
	ಯದಿ (name != ರಿಕ್ತ) {
		k_thread_name_set(&work_q->thread, name);
	}

	k_thread_start(&work_q->thread);
}
