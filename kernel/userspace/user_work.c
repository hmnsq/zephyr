/*
 * Copyright (c) 2018 Intel Corporation
 * Copyright (c) 2016 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#अन्तर्भाव <zephyr/kernel.h>

स्थिर शून्य z_work_user_q_main(शून्य *work_q_ptr, शून्य *p2, शून्य *p3)
{
	संरचना k_work_user_q *work_q = work_q_ptr;

	ARG_UNUSED(p2);
	ARG_UNUSED(p3);

	जाव (true) {
		संरचना k_work_user *work;
		k_work_user_handler_t handler;

		work = k_queue_get(&work_q->queue, K_FOREVER);
		जइ (work == रिक्त) {
			अनुवर्त;
		}

		handler = work->handler;
		__ASSERT(handler != रिक्त, "handler must be provided");

		/* Reset pending state so it can be resubmitted by handler */
		जइ (atomic_test_and_clear_bit(&work->flags,
					      K_WORK_USER_STATE_PENDING)) {
			handler(work);
		}

		/* Make sure we don't hog up the CPU if the FIFO never (or
		 * very rarely) gets empty.
		 */
		k_yield();
	}
}

शून्य k_work_user_queue_start(संरचना k_work_user_q *work_q, k_thread_राशिः_t *राशिः,
			     size_t राशिः_size, पूर्णाङ्क prio, ध्रुव वर्ण *name)
{
	k_queue_init(&work_q->queue);

	/* Created worker thread will inherit object permissions and memory
	 * domain configuration of the caller
	 */
	k_thread_create(&work_q->thread, राशिः, राशिः_size, z_work_user_q_main,
			work_q, रिक्त, रिक्त, prio, K_USER | K_INHERIT_PERMS,
			K_FOREVER);
	k_object_access_grant(&work_q->queue, &work_q->thread);
	जइ (name != रिक्त) {
		k_thread_name_set(&work_q->thread, name);
	}

	k_thread_start(&work_q->thread);
}
