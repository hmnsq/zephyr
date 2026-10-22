/*
 * Copyright (c) 2018 Intel Corporation
 * Copyright (c) 2016 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#समावेश <zephyr/kernel.h>

अटल व्योम z_work_user_q_main(व्योम *work_q_ptr, व्योम *p2, व्योम *p3)
{
	काष्ठा k_work_user_q *work_q = work_q_ptr;

	ARG_UNUSED(p2);
	ARG_UNUSED(p3);

	जबतक (true) {
		काष्ठा k_work_user *work;
		k_work_user_handler_t handler;

		work = k_queue_get(&work_q->queue, K_FOREVER);
		अगर (work == शून्य) {
			जारी;
		}

		handler = work->handler;
		__ASSERT(handler != शून्य, "handler must be provided");

		/* Reset pending state so it can be resubmitted by handler */
		अगर (atomic_test_and_clear_bit(&work->flags,
					      K_WORK_USER_STATE_PENDING)) {
			handler(work);
		}

		/* Make sure we don't hog up the CPU if the FIFO never (or
		 * very rarely) gets empty.
		 */
		k_yield();
	}
}

व्योम k_work_user_queue_start(काष्ठा k_work_user_q *work_q, k_thread_ढेर_t *ढेर,
			     माप_प्रकार ढेर_आकार, पूर्णांक prio, स्थिर अक्षर *name)
{
	k_queue_init(&work_q->queue);

	/* Created worker thread will inherit object permissions and memory
	 * domain configuration of the caller
	 */
	k_thread_create(&work_q->thread, ढेर, ढेर_आकार, z_work_user_q_main,
			work_q, शून्य, शून्य, prio, K_USER | K_INHERIT_PERMS,
			K_FOREVER);
	k_object_access_grant(&work_q->queue, &work_q->thread);
	अगर (name != शून्य) {
		k_thread_name_set(&work_q->thread, name);
	}

	k_thread_start(&work_q->thread);
}
