/*
 * Copyright (c) 2018 Intel Corporation
 * Copyright (c) 2016 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/kernel.h>

ਸਥਿਰ ਖਾਲੀ z_work_user_q_main(ਖਾਲੀ *work_q_ptr, ਖਾਲੀ *p2, ਖਾਲੀ *p3)
{
	ਬਣਤਰ k_work_user_q *work_q = work_q_ptr;

	ARG_ਅਣਵਰਤਿਆ(p2);
	ARG_ਅਣਵਰਤਿਆ(p3);

	ਜਦੋਂ (true) {
		ਬਣਤਰ k_work_user *work;
		k_work_user_handler_t handler;

		work = k_ਕਤਾਰ_get(&work_q->ਕਤਾਰ, K_FOREVER);
		ਜੇ (work == ਨਿਰਾ) {
			ਜਾਰੀ;
		}

		handler = work->handler;
		__ASSERT(handler != ਨਿਰਾ, "handler must be provided");

		/* Reset pending state so it can be resubmitted by handler */
		ਜੇ (atomic_test_and_clear_ਦੋਆਧਾਰੀ_ਅੰਕ(&work->flags,
					      K_WORK_USER_STATE_PENDING)) {
			handler(work);
		}

		/* Make sure we don't hog up the CPU if the FIFO never (or
		 * very rarely) gets empty.
		 */
		k_yield();
	}
}

ਖਾਲੀ k_work_user_ਕਤਾਰ_ਸ਼ੁਰੂ(ਬਣਤਰ k_work_user_q *work_q, k_thread_ਢੇਰ_t *ਢੇਰ,
			     size_t ਢੇਰ_ਮਾਪ, ਪੂਰਨ prio, ਅਚਲ ਅੱਖਰ *name)
{
	k_ਕਤਾਰ_init(&work_q->ਕਤਾਰ);

	/* Created worker thread will inherit object permissions and memory
	 * domain configuration of the caller
	 */
	k_thread_create(&work_q->thread, ਢੇਰ, ਢੇਰ_ਮਾਪ, z_work_user_q_main,
			work_q, ਨਿਰਾ, ਨਿਰਾ, prio, K_USER | K_INHERIT_PERMS,
			K_FOREVER);
	k_object_access_grant(&work_q->ਕਤਾਰ, &work_q->thread);
	ਜੇ (name != ਨਿਰਾ) {
		k_thread_name_ਸੈੱਟ(&work_q->thread, name);
	}

	k_thread_ਸ਼ੁਰੂ(&work_q->thread);
}
