/*
 * Copyright (c) 2018 Intel Corporation
 * Copyright (c) 2016 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ಅನ್ತರ್ಭಾವ <zephyr/kernel.h>

ಸ್ಥಿರ ಶೂನ್ಯ z_work_user_q_main(ಶೂನ್ಯ *work_q_ptr, ಶೂನ್ಯ *p2, ಶೂನ್ಯ *p3)
{
	ರಚನೆ k_work_user_q *work_q = work_q_ptr;

	ARG_ಬಳಸದ(p2);
	ARG_ಬಳಸದ(p3);

	ಆಗ (true) {
		ರಚನೆ k_work_user *work;
		k_work_user_handler_t handler;

		work = k_ಸರತಿ_get(&work_q->ಸರತಿ, K_FOREVER);
		ಆದರೆ (work == ಖಾಲಿ) {
			ಮುಂದುವರಿ;
		}

		handler = work->handler;
		__ASSERT(handler != ಖಾಲಿ, "handler must be provided");

		/* Reset pending state so it can be resubmitted by handler */
		ಆದರೆ (atomic_test_and_clear_ಬಿಟ್(&work->flags,
					      K_WORK_USER_STATE_PENDING)) {
			handler(work);
		}

		/* Make sure we don't hog up the CPU if the FIFO never (or
		 * very rarely) gets empty.
		 */
		k_yield();
	}
}

ಶೂನ್ಯ k_work_user_ಸರತಿ_ಆರಂಭ(ರಚನೆ k_work_user_q *work_q, k_thread_ರಾಶಿ_t *ರಾಶಿ,
			     size_t ರಾಶಿ_ಗಾತ್ರ, ಪೂರ್ಣಾಂಕ prio, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name)
{
	k_ಸರತಿ_init(&work_q->ಸರತಿ);

	/* Created worker thread will inherit object permissions and memory
	 * domain configuration of the caller
	 */
	k_thread_create(&work_q->thread, ರಾಶಿ, ರಾಶಿ_ಗಾತ್ರ, z_work_user_q_main,
			work_q, ಖಾಲಿ, ಖಾಲಿ, prio, K_USER | K_INHERIT_PERMS,
			K_FOREVER);
	k_object_access_grant(&work_q->ಸರತಿ, &work_q->thread);
	ಆದರೆ (name != ಖಾಲಿ) {
		k_thread_name_ಗಣ(&work_q->thread, name);
	}

	k_thread_ಆರಂಭ(&work_q->thread);
}
