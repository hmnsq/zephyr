/*
 * Copyright (c) 2018 Intel Corporation
 * Copyright (c) 2016 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#అన్తర్భావ <zephyr/kernel.h>

స్థిర శూన్య z_work_user_q_main(శూన్య *work_q_ptr, శూన్య *p2, శూన్య *p3)
{
	నిర్మాణం k_work_user_q *work_q = work_q_ptr;

	ARG_వాడని(p2);
	ARG_వాడని(p3);

	అయితే (true) {
		నిర్మాణం k_work_user *work;
		k_work_user_handler_t handler;

		work = k_వరుస_get(&work_q->వరుస, K_FOREVER);
		ఒకవేళ (work == ఖాళీ) {
			కొనసాగు;
		}

		handler = work->handler;
		__ASSERT(handler != ఖాళీ, "handler must be provided");

		/* Reset pending state so it can be resubmitted by handler */
		ఒకవేళ (atomic_test_and_clear_బిట్టు(&work->flags,
					      K_WORK_USER_STATE_PENDING)) {
			handler(work);
		}

		/* Make sure we don't hog up the CPU if the FIFO never (or
		 * very rarely) gets empty.
		 */
		k_yield();
	}
}

శూన్య k_work_user_వరుస_ప్రారంభం(నిర్మాణం k_work_user_q *work_q, k_thread_దొంతర_t *దొంతర,
			     size_t దొంతర_కొలత, పూర్ణాంకం prio, స్థిరరాశి అక్షరం *name)
{
	k_వరుస_init(&work_q->వరుస);

	/* Created worker thread will inherit object permissions and memory
	 * domain configuration of the caller
	 */
	k_thread_create(&work_q->thread, దొంతర, దొంతర_కొలత, z_work_user_q_main,
			work_q, ఖాళీ, ఖాళీ, prio, K_USER | K_INHERIT_PERMS,
			K_FOREVER);
	k_object_access_grant(&work_q->వరుస, &work_q->thread);
	ఒకవేళ (name != ఖాళీ) {
		k_thread_name_సమితి(&work_q->thread, name);
	}

	k_thread_ప్రారంభం(&work_q->thread);
}
