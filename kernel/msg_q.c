/*
 * Copyright (c) 2016 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief Message queues.
 */


#समावेश <zephyr/kernel.h>

#समावेश <zephyr/toolchain.h>
#समावेश <zephyr/linker/sections.h>
#समावेश <string.h>
#समावेश <ksched.h>
#समावेश <scheduler.h>
#समावेश <wait_q.h>
#समावेश <zephyr/sys/dlist.h>
#समावेश <zephyr/sys/math_extras.h>
#समावेश <zephyr/init.h>
#समावेश <zephyr/internal/syscall_handler.h>
#समावेश <kernel_internal.h>
#समावेश <zephyr/sys/check.h>

#अगर_घोषित CONFIG_OBJ_CORE_MSGQ
K_OBJ_TYPE_DEFINE(obj_type_msgq, k_msgq, K_OBJ_TYPE_MSGQ_ID, शून्य);
#पूर्ण_अगर /* CONFIG_OBJ_CORE_MSGQ */

अटल अंतरभूत bool msgq_handle_poll_events(काष्ठा k_msgq *msgq)
{
#अगर_घोषित CONFIG_POLL
	वापस z_handle_obj_poll_events(&msgq->poll_events,
					K_POLL_STATE_MSGQ_तथ्य_AVAILABLE);
#अन्यथा
	ARG_UNUSED(msgq);
	वापस false;
#पूर्ण_अगर /* CONFIG_POLL */
}

व्योम k_msgq_init(काष्ठा k_msgq *msgq, अक्षर *बफर, माप_प्रकार संदेश_आकार,
		 uint32_t max_msgs)
{
	__ASSERT_NO_MSG(!आकार_mul_overflow(max_msgs, संदेश_आकार, &(माप_प्रकार){0}));
	__ASSERT_NO_MSG(!आकार_add_overflow((माप_प्रकार)(uintptr_t)बफर, max_msgs * संदेश_आकार,
					&(माप_प्रकार){0}));

	msgq->संदेश_आकार = संदेश_आकार;
	msgq->max_msgs = max_msgs;
	msgq->बफर_start = बफर;
	msgq->बफर_end = बफर + (max_msgs * संदेश_आकार);
	msgq->read_ptr = बफर;
	msgq->write_ptr = बफर;
	msgq->used_msgs = 0;
	msgq->flags = 0;
	z_waitq_init(&msgq->wait_q);
	msgq->lock = (काष्ठा k_spinlock) {};
#अगर_घोषित CONFIG_POLL
	sys_dlist_init(&msgq->poll_events);
#पूर्ण_अगर	/* CONFIG_POLL */

#अगर_घोषित CONFIG_OBJ_CORE_MSGQ
	k_obj_core_init_and_link(K_OBJ_CORE(msgq), &obj_type_msgq);
#पूर्ण_अगर /* CONFIG_OBJ_CORE_MSGQ */

	SYS_PORT_TRACING_OBJ_INIT(k_msgq, msgq);

	k_object_init(msgq);
}

पूर्णांक z_impl_k_msgq_alloc_init(काष्ठा k_msgq *msgq, माप_प्रकार संदेश_आकार,
			    uint32_t max_msgs)
{
	व्योम *बफर;
	पूर्णांक ret;
	माप_प्रकार योग_आकार;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_msgq, alloc_init, msgq);

	अगर (आकार_mul_overflow(संदेश_आकार, max_msgs, &योग_आकार)) {
		ret = -EINVAL;
	} अन्यथा {
		बफर = z_thread_malloc(योग_आकार);
		अगर (बफर != शून्य) {
			k_msgq_init(msgq, बफर, संदेश_आकार, max_msgs);
			msgq->flags = K_MSGQ_FLAG_ALLOC;
			ret = 0;
		} अन्यथा {
			ret = -ENOMEM;
		}
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_msgq, alloc_init, msgq, ret);
	वापस ret;
}

#अगर_घोषित CONFIG_USERSPACE
पूर्णांक z_vrfy_k_msgq_alloc_init(काष्ठा k_msgq *msgq, माप_प्रकार संदेश_आकार,
			    uint32_t max_msgs)
{
	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(msgq, K_OBJ_MSGQ));

	वापस z_impl_k_msgq_alloc_init(msgq, संदेश_आकार, max_msgs);
}
#समावेश <zephyr/syscalls/k_msgq_alloc_init_mrsh.c>
#पूर्ण_अगर /* CONFIG_USERSPACE */

पूर्णांक z_msgq_cleanup(काष्ठा k_msgq *msgq, __maybe_unused bool locked)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_msgq, cleanup, msgq);

	पूर्णांक ret = 0;
	bool freed = false;
	k_spinlock_कुंजी_t कुंजी = k_spin_lock(&msgq->lock);

	CHECKIF(locked && (z_waitq_शीर्ष_locked(&msgq->wait_q) != शून्य)) {
		ret = -EBUSY;
		जाओ out;
	}

	CHECKIF(!locked && (z_waitq_शीर्ष(&msgq->wait_q) != शून्य)) {
		ret = -EBUSY;
		जाओ out;
	}

	अगर ((msgq->flags & K_MSGQ_FLAG_ALLOC) != 0U) {
		k_free(msgq->बफर_start);
		msgq->flags &= ~K_MSGQ_FLAG_ALLOC;
		freed = true;
	}

out:
	k_spin_unlock(&msgq->lock, कुंजी);

#अगर_घोषित CONFIG_OBJ_CORE_MSGQ
	अगर (freed) {
		k_obj_core_unlink(K_OBJ_CORE(msgq));
	}
#पूर्ण_अगर /* CONFIG_OBJ_CORE_MSGQ */

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_msgq, cleanup, msgq, ret);
	वापस ret;
}

पूर्णांक k_msgq_cleanup(काष्ठा k_msgq *msgq)
{
	वापस z_msgq_cleanup(msgq, false);
}

अटल अंतरभूत पूर्णांक put_संदेश_in_queue(काष्ठा k_msgq *msgq, स्थिर व्योम *तथ्य,
			k_timeout_t timeout, bool put_at_back)
{
	__ASSERT(!arch_is_in_isr() || K_TIMEOUT_EQ(timeout, K_NO_WAIT), "");

	काष्ठा k_thread *pending_thread = शून्य;
	k_spinlock_कुंजी_t कुंजी;
	पूर्णांक परिणाम;
	bool resched = false;

	कुंजी = k_spin_lock(&msgq->lock);

	अगर (put_at_back) {
		SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_msgq, put, msgq, timeout);
	} अन्यथा {
		SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_msgq, put_front, msgq, timeout);
	}

	अगर (msgq->used_msgs < msgq->max_msgs) {
		/* message queue isn't full. Try to hand the message
		 * directly to the longest-waiting receiver, atomically
		 * under the scheduler's spinlock so a racing in-flight timeout
		 * handler cannot wake the receiver before the message
		 * has been copied into its buffer.
		 */
		LOCK_SCHED_SPINLOCK {
			pending_thread = z_unpend_first_thread_locked(&msgq->wait_q);
			अगर (pending_thread != शून्य) {
				/* copy into the receiver's buffer */
				(व्योम)स_नकल(pending_thread->base.swap_तथ्य, तथ्य,
					     msgq->संदेश_आकार);
				arch_thread_return_मूल्य_set(pending_thread, 0);
				z_sched_ready_locked(pending_thread);
				resched = true;
			}
		}
		अगर (pending_thread == शून्य) {
			माप_प्रकार संदेश_आकार = msgq->संदेश_आकार;
			अक्षर *slot;

			__ASSERT_NO_MSG((msgq->write_ptr >= msgq->बफर_start) &&
					(msgq->write_ptr <= (msgq->बफर_end - 1)) &&
					((माप_प्रकार)(uintptr_t)(msgq->बफर_end - msgq->write_ptr) >=
						msgq->संदेश_आकार));
			/*
			 * Pick the slot and update the ring state from
			 * registers first: the compiler cannot tell that the
			 * copy leaves the queue fields alone, so copying last
			 * saves it re-reading them afterwards.
			 */
			अगर (put_at_back) {
				/*
				 * to write a message to the back of the queue,
				 * take the slot write_ptr points at and
				 * advance write_ptr past it
				 */
				अक्षर *अग्र;

				slot = msgq->write_ptr;
				अग्र = slot + संदेश_आकार;
				अगर (अग्र == msgq->बफर_end) {
					अग्र = msgq->बफर_start;
				}
				msgq->write_ptr = अग्र;
			} अन्यथा {
				/*
				 * to write a message to the head of the queue,
				 * first decrement the read pointer (to open
				 * space at the front of the queue) then copy
				 * the message to the newly created space.
				 */
				slot = msgq->read_ptr;
				अगर (slot == msgq->बफर_start) {
					slot = msgq->बफर_end;
				}
				slot -= संदेश_आकार;
				msgq->read_ptr = slot;
			}
			msgq->used_msgs++;
			(व्योम)स_नकल(slot, तथ्य, संदेश_आकार);
			resched = msgq_handle_poll_events(msgq);
		}
		परिणाम = 0;
	} अन्यथा अगर (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		/* don't wait for message space to become available */
		परिणाम = -ENOMSG;
	} अन्यथा {
		अगर (put_at_back) {
			SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_msgq, put, msgq, timeout);
		} अन्यथा {
			SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_msgq, put_front, msgq, timeout);
		}

		/* wait for put message success, failure, or timeout */
		_current->base.swap_तथ्य = (व्योम *) तथ्य;

		परिणाम = z_pend_curr(&msgq->lock, कुंजी, &msgq->wait_q, timeout);

		अगर (put_at_back) {
			SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_msgq, put, msgq, timeout, परिणाम);
		} अन्यथा {
			SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_msgq, put_front, msgq, timeout, परिणाम);
		}

		वापस परिणाम;
	}

	अगर (put_at_back) {
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_msgq, put, msgq, timeout, परिणाम);
	} अन्यथा {
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_msgq, put_front, msgq, timeout, परिणाम);
	}

	अगर (resched) {
		z_reschedule(&msgq->lock, कुंजी);
	} अन्यथा {
		k_spin_unlock(&msgq->lock, कुंजी);
	}

	वापस परिणाम;
}


पूर्णांक z_impl_k_msgq_put(काष्ठा k_msgq *msgq, स्थिर व्योम *तथ्य, k_timeout_t timeout)
{
	वापस put_संदेश_in_queue(msgq, तथ्य, timeout, true);
}

पूर्णांक z_impl_k_msgq_put_front(काष्ठा k_msgq *msgq, स्थिर व्योम *तथ्य)
{
	वापस put_संदेश_in_queue(msgq, तथ्य, K_NO_WAIT, false);
}

#अगर_घोषित CONFIG_USERSPACE
अटल अंतरभूत पूर्णांक z_vrfy_k_msgq_put(काष्ठा k_msgq *msgq, स्थिर व्योम *तथ्य,
				    k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	K_OOPS(K_SYSCALL_MEMORY_READ(तथ्य, msgq->संदेश_आकार));

	वापस z_impl_k_msgq_put(msgq, तथ्य, timeout);
}
#समावेश <zephyr/syscalls/k_msgq_put_mrsh.c>

अटल अंतरभूत पूर्णांक z_vrfy_k_msgq_put_front(काष्ठा k_msgq *msgq, स्थिर व्योम *तथ्य)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	K_OOPS(K_SYSCALL_MEMORY_READ(तथ्य, msgq->संदेश_आकार));

	वापस z_impl_k_msgq_put_front(msgq, तथ्य);
}
#समावेश <zephyr/syscalls/k_msgq_put_front_mrsh.c>
#पूर्ण_अगर /* CONFIG_USERSPACE */

व्योम z_impl_k_msgq_get_attrs(काष्ठा k_msgq *msgq, काष्ठा k_msgq_attrs *attrs)
{
	attrs->संदेश_आकार = msgq->संदेश_आकार;
	attrs->max_msgs = msgq->max_msgs;
	attrs->used_msgs = msgq->used_msgs;
}

#अगर_घोषित CONFIG_USERSPACE
अटल अंतरभूत व्योम z_vrfy_k_msgq_get_attrs(काष्ठा k_msgq *msgq,
					   काष्ठा k_msgq_attrs *attrs)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	K_OOPS(K_SYSCALL_MEMORY_WRITE(attrs, माप(काष्ठा k_msgq_attrs)));
	z_impl_k_msgq_get_attrs(msgq, attrs);
}
#समावेश <zephyr/syscalls/k_msgq_get_attrs_mrsh.c>
#पूर्ण_अगर /* CONFIG_USERSPACE */

पूर्णांक z_impl_k_msgq_get(काष्ठा k_msgq *msgq, व्योम *तथ्य, k_timeout_t timeout)
{
	__ASSERT(!arch_is_in_isr() || K_TIMEOUT_EQ(timeout, K_NO_WAIT), "");

	k_spinlock_कुंजी_t कुंजी;
	काष्ठा k_thread *pending_thread;
	पूर्णांक परिणाम;
	bool resched = false;

	कुंजी = k_spin_lock(&msgq->lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_msgq, get, msgq, timeout);

	अगर (likely(msgq->used_msgs > 0U)) {
		माप_प्रकार संदेश_आकार = msgq->संदेश_आकार;
		अक्षर *slot = msgq->read_ptr;
		अक्षर *अग्र = slot + संदेश_आकार;

		/* drop the message from the queue, then copy it out */
		अगर (अग्र == msgq->बफर_end) {
			अग्र = msgq->बफर_start;
		}
		msgq->read_ptr = अग्र;
		msgq->used_msgs--;

		(व्योम)स_नकल(तथ्य, slot, संदेश_आकार);

		/* sanity-check write_ptr in case we hand the slot to a sender */
		__ASSERT_NO_MSG((msgq->write_ptr >= msgq->बफर_start) &&
				(msgq->write_ptr <= (msgq->बफर_end - 1)) &&
				((माप_प्रकार)(uintptr_t)(msgq->बफर_end - msgq->write_ptr) >=
					msgq->संदेश_आकार));

		/* handle first thread waiting to write (if any). Done
		 * atomically under the scheduler's spinlock so we read the
		 * sender's swap_data and complete the wake before any
		 * racing in-flight timeout handler can wake the sender.
		 */
		LOCK_SCHED_SPINLOCK {
			pending_thread = z_unpend_first_thread_locked(&msgq->wait_q);
			अगर (pending_thread != शून्य) {
				SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_msgq, get, msgq, timeout);

				/* add the sender's pending message to the queue */
				slot = msgq->write_ptr;
				अग्र = slot + संदेश_आकार;
				अगर (अग्र == msgq->बफर_end) {
					अग्र = msgq->बफर_start;
				}
				msgq->write_ptr = अग्र;
				msgq->used_msgs++;

				(व्योम)स_नकल(slot, pending_thread->base.swap_तथ्य, संदेश_आकार);

				arch_thread_return_मूल्य_set(pending_thread, 0);
				z_sched_ready_locked(pending_thread);
				resched = true;
			}
		}
		परिणाम = 0;
	} अन्यथा अगर (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		/* don't wait for a message to become available */
		परिणाम = -ENOMSG;
	} अन्यथा {
		SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_msgq, get, msgq, timeout);

		/* wait for get message success or timeout */
		_current->base.swap_तथ्य = तथ्य;

		परिणाम = z_pend_curr(&msgq->lock, कुंजी, &msgq->wait_q, timeout);
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_msgq, get, msgq, timeout, परिणाम);
		वापस परिणाम;
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_msgq, get, msgq, timeout, परिणाम);

	अगर (resched) {
		z_reschedule(&msgq->lock, कुंजी);
	} अन्यथा {
		k_spin_unlock(&msgq->lock, कुंजी);
	}

	वापस परिणाम;
}

#अगर_घोषित CONFIG_USERSPACE
अटल अंतरभूत पूर्णांक z_vrfy_k_msgq_get(काष्ठा k_msgq *msgq, व्योम *तथ्य,
				    k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	K_OOPS(K_SYSCALL_MEMORY_WRITE(तथ्य, msgq->संदेश_आकार));

	वापस z_impl_k_msgq_get(msgq, तथ्य, timeout);
}
#समावेश <zephyr/syscalls/k_msgq_get_mrsh.c>
#पूर्ण_अगर /* CONFIG_USERSPACE */

पूर्णांक z_impl_k_msgq_peek(काष्ठा k_msgq *msgq, व्योम *तथ्य)
{
	k_spinlock_कुंजी_t कुंजी;
	पूर्णांक परिणाम;

	कुंजी = k_spin_lock(&msgq->lock);

	अगर (msgq->used_msgs > 0U) {
		/* take first available message from queue */
		(व्योम)स_नकल(तथ्य, msgq->read_ptr, msgq->संदेश_आकार);
		परिणाम = 0;
	} अन्यथा {
		/* don't wait for a message to become available */
		परिणाम = -ENOMSG;
	}

	SYS_PORT_TRACING_OBJ_FUNC(k_msgq, peek, msgq, परिणाम);

	k_spin_unlock(&msgq->lock, कुंजी);

	वापस परिणाम;
}

#अगर_घोषित CONFIG_USERSPACE
अटल अंतरभूत पूर्णांक z_vrfy_k_msgq_peek(काष्ठा k_msgq *msgq, व्योम *तथ्य)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	K_OOPS(K_SYSCALL_MEMORY_WRITE(तथ्य, msgq->संदेश_आकार));

	वापस z_impl_k_msgq_peek(msgq, तथ्य);
}
#समावेश <zephyr/syscalls/k_msgq_peek_mrsh.c>
#पूर्ण_अगर /* CONFIG_USERSPACE */

पूर्णांक z_impl_k_msgq_peek_at(काष्ठा k_msgq *msgq, व्योम *तथ्य, uint32_t idx)
{
	k_spinlock_कुंजी_t कुंजी;
	पूर्णांक परिणाम;
	uint32_t bytes_to_end;
	uint32_t byte_offset;
	अक्षर *start_addr;

	कुंजी = k_spin_lock(&msgq->lock);

	अगर (msgq->used_msgs > idx) {
		bytes_to_end = (msgq->बफर_end - msgq->read_ptr);
		byte_offset = idx * msgq->संदेश_आकार;
		start_addr = msgq->read_ptr;
		/* check item available in start/end of ring buffer */
		अगर (bytes_to_end <= byte_offset) {
			/* Tweak the values in case */
			byte_offset -= bytes_to_end;
			/* wrap-around is required */
			start_addr = msgq->बफर_start;
		}
		(व्योम)स_नकल(तथ्य, start_addr + byte_offset, msgq->संदेश_आकार);
		परिणाम = 0;
	} अन्यथा {
		/* don't wait for a message to become available */
		परिणाम = -ENOMSG;
	}

	SYS_PORT_TRACING_OBJ_FUNC(k_msgq, peek, msgq, परिणाम);

	k_spin_unlock(&msgq->lock, कुंजी);

	वापस परिणाम;
}

#अगर_घोषित CONFIG_USERSPACE
अटल अंतरभूत पूर्णांक z_vrfy_k_msgq_peek_at(काष्ठा k_msgq *msgq, व्योम *तथ्य, uint32_t idx)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	K_OOPS(K_SYSCALL_MEMORY_WRITE(तथ्य, msgq->संदेश_आकार));

	वापस z_impl_k_msgq_peek_at(msgq, तथ्य, idx);
}
#समावेश <zephyr/syscalls/k_msgq_peek_at_mrsh.c>
#पूर्ण_अगर /* CONFIG_USERSPACE */

व्योम z_impl_k_msgq_purge(काष्ठा k_msgq *msgq)
{
	k_spinlock_कुंजी_t कुंजी;
	bool resched = false;

	कुंजी = k_spin_lock(&msgq->lock);

	SYS_PORT_TRACING_OBJ_FUNC(k_msgq, purge, msgq);

	/* wake up any threads that are waiting to write */
	जबतक (z_sched_wake(&msgq->wait_q, -ENOMSG, शून्य)) {
		resched = true;
	}

	msgq->used_msgs = 0;
	msgq->read_ptr = msgq->write_ptr;

	अगर (resched) {
		z_reschedule(&msgq->lock, कुंजी);
	} अन्यथा {
		k_spin_unlock(&msgq->lock, कुंजी);
	}
}

#अगर_घोषित CONFIG_USERSPACE
अटल अंतरभूत व्योम z_vrfy_k_msgq_purge(काष्ठा k_msgq *msgq)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	z_impl_k_msgq_purge(msgq);
}
#समावेश <zephyr/syscalls/k_msgq_purge_mrsh.c>

अटल अंतरभूत uint32_t z_vrfy_k_msgq_अंक_free_get(काष्ठा k_msgq *msgq)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	वापस z_impl_k_msgq_अंक_free_get(msgq);
}
#समावेश <zephyr/syscalls/k_msgq_num_free_get_mrsh.c>

अटल अंतरभूत uint32_t z_vrfy_k_msgq_अंक_used_get(काष्ठा k_msgq *msgq)
{
	K_OOPS(K_SYSCALL_OBJ(msgq, K_OBJ_MSGQ));
	वापस z_impl_k_msgq_अंक_used_get(msgq);
}
#समावेश <zephyr/syscalls/k_msgq_num_used_get_mrsh.c>

#पूर्ण_अगर /* CONFIG_USERSPACE */
