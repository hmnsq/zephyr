/*
 * Copyright (c) 2018 Intel Corporation
 * Copyright (c) 2016 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ଅନ୍ତର୍ଭାଵ <zephyr/kernel.h>

ସ୍ଥିର ଶୂନ୍ୟ z_work_user_q_main(ଶୂନ୍ୟ *work_q_ptr, ଶୂନ୍ୟ *p2, ଶୂନ୍ୟ *p3)
{
	ସଂରଚନା k_work_user_q *work_q = work_q_ptr;

	ARG_UNUSED(p2);
	ARG_UNUSED(p3);

	ଯେତେବେଳେ (true) {
		ସଂରଚନା k_work_user *work;
		k_work_user_handler_t handler;

		work = k_queue_get(&work_q->queue, K_FOREVER);
		ଯଦି (work == ଖାଲି) {
			ଜାରି;
		}

		handler = work->handler;
		__ASSERT(handler != ଖାଲି, "handler must be provided");

		/* Reset pending state so it can be resubmitted by handler */
		ଯଦି (atomic_test_and_clear_bit(&work->flags,
					      K_WORK_USER_STATE_PENDING)) {
			handler(work);
		}

		/* Make sure we don't hog up the CPU if the FIFO never (or
		 * very rarely) gets empty.
		 */
		k_yield();
	}
}

ଶୂନ୍ୟ k_work_user_queue_start(ସଂରଚନା k_work_user_q *work_q, k_thread_ଗଦା_t *ଗଦା,
			     size_t ଗଦା_size, ପୂର୍ଣ୍ଣସଂଖ୍ୟା prio, ଅଚଳ ଅକ୍ଷର *name)
{
	k_queue_init(&work_q->queue);

	/* Created worker thread will inherit object permissions and memory
	 * domain configuration of the caller
	 */
	k_thread_create(&work_q->thread, ଗଦା, ଗଦା_size, z_work_user_q_main,
			work_q, ଖାଲି, ଖାଲି, prio, K_USER | K_INHERIT_PERMS,
			K_FOREVER);
	k_object_access_grant(&work_q->queue, &work_q->thread);
	ଯଦି (name != ଖାଲି) {
		k_thread_name_set(&work_q->thread, name);
	}

	k_thread_start(&work_q->thread);
}
