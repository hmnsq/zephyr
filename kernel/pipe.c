/*
 * Copyright (c) 2024 Måns Ansgariusson <mansgariusson@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#समावेश <zephyr/sys/minmax.h>
#समावेश <zephyr/init.h>
#समावेश <zephyr/kernel.h>
#समावेश <zephyr/internal/syscall_handler.h>
#समावेश <ksched.h>
#समावेश <kthread.h>
#समावेश <wait_q.h>
#समावेश <scheduler.h>

#अगर_घोषित CONFIG_OBJ_CORE_PIPE
K_OBJ_TYPE_DEFINE(obj_type_pipe, k_pipe, K_OBJ_TYPE_PIPE_ID, शून्य);
#पूर्ण_अगर /* CONFIG_OBJ_CORE_PIPE */

अटल अंतरभूत bool pipe_closed(काष्ठा k_pipe *pipe)
{
	वापस (pipe->flags & PIPE_FLAG_OPEN) == 0;
}

अटल अंतरभूत bool pipe_resetting(काष्ठा k_pipe *pipe)
{
	वापस (pipe->flags & PIPE_FLAG_RESET) != 0;
}

अटल अंतरभूत bool pipe_full(काष्ठा k_pipe *pipe)
{
	वापस ring_buf_space_get(&pipe->buf) == 0;
}

अटल अंतरभूत bool pipe_empty(काष्ठा k_pipe *pipe)
{
	वापस ring_buf_is_empty(&pipe->buf);
}

काष्ठा pipe_buf_spec {
	uint8_t * स्थिर तथ्य;
	स्थिर माप_प्रकार len;
	माप_प्रकार used;
};

अटल पूर्णांक wait_for(_wait_q_t *waitq, काष्ठा k_pipe *pipe, k_spinlock_कुंजी_t *कुंजी,
		    k_timepoint_t time_limit, bool *need_resched,
		    काष्ठा pipe_buf_spec *buf_spec)
{
	k_timeout_t timeout = sys_timepoint_timeout(time_limit);
	पूर्णांक rc;

	अगर (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		वापस -EAGAIN;
	}

	_current->base.swap_तथ्य = buf_spec;

	pipe->waiting++;
	*need_resched = false;
	अगर (waitq == &pipe->space) {
		SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_pipe, ग_लिखो, pipe, timeout);
	} अन्यथा {
		SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_pipe, पढ़ो, pipe, timeout);
	}
	rc = z_pend_curr(&pipe->lock, *कुंजी, waitq, timeout);
	*कुंजी = k_spin_lock(&pipe->lock);
	pipe->waiting--;
	अगर (unlikely(pipe_resetting(pipe))) {
		अगर (pipe->waiting == 0) {
			pipe->flags &= ~PIPE_FLAG_RESET;
		}
		rc = -ECANCELED;
	}

	वापस rc;
}

व्योम z_impl_k_pipe_init(काष्ठा k_pipe *pipe, uint8_t *बफर, माप_प्रकार बफर_आकार)
{
	ring_buf_init(&pipe->buf, बफर_आकार, बफर);
	pipe->flags = PIPE_FLAG_OPEN;
	pipe->waiting = 0;

	pipe->lock = (काष्ठा k_spinlock){};
	z_waitq_init(&pipe->तथ्य);
	z_waitq_init(&pipe->space);
	k_object_init(pipe);

#अगर_घोषित CONFIG_POLL
	sys_dlist_init(&pipe->poll_events);
#पूर्ण_अगर /* CONFIG_POLL */
#अगर_घोषित CONFIG_OBJ_CORE_PIPE
	k_obj_core_init_and_link(K_OBJ_CORE(pipe), &obj_type_pipe);
#पूर्ण_अगर /* CONFIG_OBJ_CORE_PIPE */
	SYS_PORT_TRACING_OBJ_INIT(k_pipe, pipe, बफर, बफर_आकार);
}

अटल माप_प्रकार copy_to_pending_readers(काष्ठा k_pipe *pipe, bool *need_resched,
				      स्थिर uint8_t *तथ्य, माप_प्रकार len)
{
	काष्ठा k_thread *reader = शून्य;
	काष्ठा pipe_buf_spec *reader_buf;
	माप_प्रकार copy_आकार, written = 0;

	/*
	 * Attempt a direct data copy to waiting readers if any.
	 * The copy has to be done under the scheduler lock to ensure all the
	 * needed data is copied to the target thread whose buffer spec lives
	 * on that thread's stack, and then the thread unpended only if it
	 * received all the data it wanted, without racing with a potential
	 * thread timeout/cancellation event.
	 */
	करो {
		LOCK_SCHED_SPINLOCK {
			reader = _priq_wait_best(&pipe->तथ्य.waitq);
			अगर (reader == शून्य) {
				K_SPINLOCK_BREAK;
			}

			reader_buf = reader->base.swap_तथ्य;
			copy_आकार = min(len - written,
					reader_buf->len - reader_buf->used);
			स_नकल(&reader_buf->तथ्य[reader_buf->used],
			       &तथ्य[written], copy_आकार);
			written += copy_आकार;
			reader_buf->used += copy_आकार;

			अगर (reader_buf->used < reader_buf->len) {
				/* This reader wants more: don't unpend. */
				reader = शून्य;
			} अन्यथा {
				/*
				 * This reader has received all the data
				 * it was waiting for. Set its return
				 * value, unpend, abort its timeout, and
				 * ready it, all under the scheduler lock
				 * so a racing timeout handler cannot
				 * observe a half-initialized wake-up.
				 */
				z_thread_return_मूल्य_set_with_तथ्य(reader, 0, शून्य);
				unpend_thread_no_timeout(reader);
				(व्योम)z_try_abort_thread_timeout(reader);
				z_sched_ready_locked(reader);
				*need_resched = true;
			}
		}
	} जबतक (reader != शून्य && written < len);

	वापस written;
}

पूर्णांक z_impl_k_pipe_write(काष्ठा k_pipe *pipe, स्थिर uint8_t *तथ्य, माप_प्रकार len, k_timeout_t timeout)
{
	पूर्णांक rc;
	माप_प्रकार written = 0;
	माप_प्रकार added;
	k_timepoint_t end = sys_timepoint_calc(timeout);
	k_spinlock_कुंजी_t कुंजी = k_spin_lock(&pipe->lock);
	bool need_resched = false;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_pipe, ग_लिखो, pipe, तथ्य, len, timeout);

	अगर (unlikely(len > पूर्णांक_उच्च)) {
		rc = -EOVERFLOW;
		जाओ out;
	}

	अगर (unlikely(pipe_resetting(pipe))) {
		rc = -ECANCELED;
		जाओ out;
	}

	क्रम (;;) {
		अगर (unlikely(pipe_closed(pipe))) {
			rc = -EPIPE;
			अवरोध;
		}

		अगर (pipe_empty(pipe)) {
			अगर (IS_ENABLED(CONFIG_KERNEL_COHERENCE)) {
				/*
				 * Systems that enabled this option don't have
				 * their stacks in coherent memory. Given our
				 * pipe_buf_spec is stored on the stack, and
				 * readers may also have their destination
				 * buffer on their stack too, it is not worth
				 * supporting direct-to-readers copy with them.
				 * Simply wake up all pending readers instead.
				 */
				need_resched = z_sched_wake_all(&pipe->तथ्य, 0, शून्य);
			} अन्यथा अगर (pipe->waiting != 0) {
				written += copy_to_pending_readers(pipe, &need_resched,
								   &तथ्य[written],
								   len - written);
				अगर (written >= len) {
					rc = written;
					अवरोध;
				}
			}
		}

		added = ring_buf_put(&pipe->buf, &तथ्य[written], len - written);

#अगर_घोषित CONFIG_POLL
		अगर (added != 0) {
			need_resched |= z_handle_obj_poll_events(&pipe->poll_events,
								 K_POLL_STATE_PIPE_तथ्य_AVAILABLE);
		}
#पूर्ण_अगर /* CONFIG_POLL */

		written += added;
		अगर (likely(written == len)) {
			rc = written;
			अवरोध;
		}

		rc = wait_for(&pipe->space, pipe, &कुंजी, end, &need_resched, शून्य);
		अगर (rc != 0) {
			अगर (rc == -EAGAIN) {
				rc = written ? written : -EAGAIN;
			}
			अवरोध;
		}
	}
out:
	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_pipe, ग_लिखो, pipe, rc);
	अगर (need_resched) {
		z_reschedule(&pipe->lock, कुंजी);
	} अन्यथा {
		k_spin_unlock(&pipe->lock, कुंजी);
	}
	वापस rc;
}

पूर्णांक z_impl_k_pipe_read(काष्ठा k_pipe *pipe, uint8_t *तथ्य, माप_प्रकार len, k_timeout_t timeout)
{
	काष्ठा pipe_buf_spec buf = { तथ्य, len, 0 };
	पूर्णांक rc;
	k_timepoint_t end = sys_timepoint_calc(timeout);
	k_spinlock_कुंजी_t कुंजी = k_spin_lock(&pipe->lock);
	bool need_resched = false;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_pipe, पढ़ो, pipe, तथ्य, len, timeout);

	अगर (unlikely(len > पूर्णांक_उच्च)) {
		rc = -EOVERFLOW;
		जाओ out;
	}

	अगर (unlikely(pipe_resetting(pipe))) {
		rc = -ECANCELED;
		जाओ out;
	}

	क्रम (;;) {
		अगर (pipe_full(pipe)) {
			/* One or more pending writers may exist. */
			need_resched = z_sched_wake_all(&pipe->space, 0, शून्य);
		}

		buf.used += ring_buf_get(&pipe->buf, &तथ्य[buf.used], len - buf.used);
		अगर (likely(buf.used == len)) {
			rc = buf.used;
			अवरोध;
		}

		अगर (unlikely(pipe_closed(pipe))) {
			rc = buf.used ? buf.used : -EPIPE;
			अवरोध;
		}

		rc = wait_for(&pipe->तथ्य, pipe, &कुंजी, end, &need_resched, &buf);
		अगर (rc != 0) {
			अगर (rc == -EAGAIN) {
				rc = buf.used ? buf.used : -EAGAIN;
			}
			अवरोध;
		}
	}
out:
	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_pipe, पढ़ो, pipe, rc);
	अगर (need_resched) {
		z_reschedule(&pipe->lock, कुंजी);
	} अन्यथा {
		k_spin_unlock(&pipe->lock, कुंजी);
	}
	वापस rc;
}

व्योम z_impl_k_pipe_reset(काष्ठा k_pipe *pipe)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_pipe, reset, pipe);
	K_SPINLOCK(&pipe->lock) {
		ring_buf_reset(&pipe->buf);
		अगर (likely(pipe->waiting != 0)) {
			pipe->flags |= PIPE_FLAG_RESET;
			z_sched_wake_all(&pipe->तथ्य, 0, शून्य);
			z_sched_wake_all(&pipe->space, 0, शून्य);
		}
	}
	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_pipe, reset, pipe);
}

व्योम z_impl_k_pipe_close(काष्ठा k_pipe *pipe)
{
	k_spinlock_कुंजी_t कुंजी;
	bool need_resched;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_pipe, बंद, pipe);

	कुंजी = k_spin_lock(&pipe->lock);

	pipe->flags = 0;
	need_resched = z_sched_wake_all(&pipe->तथ्य, 0, शून्य);
	need_resched |= z_sched_wake_all(&pipe->space, 0, शून्य);

#अगर_घोषित CONFIG_POLL
	/*
	 * Closing is a broadcast: wake every registered poller. Later
	 * k_poll() calls see the hangup through is_condition_met().
	 */
	जबतक (z_handle_obj_poll_events(&pipe->poll_events,
					K_POLL_STATE_PIPE_तथ्य_AVAILABLE)) {
		need_resched = true;
	}
#पूर्ण_अगर /* CONFIG_POLL */

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_pipe, बंद, pipe);

	अगर (need_resched) {
		z_reschedule(&pipe->lock, कुंजी);
	} अन्यथा {
		k_spin_unlock(&pipe->lock, कुंजी);
	}
}

#अगर_घोषित CONFIG_USERSPACE
व्योम z_vrfy_k_pipe_init(काष्ठा k_pipe *pipe, uint8_t *बफर, माप_प्रकार बफर_आकार)
{
	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(pipe, K_OBJ_PIPE));
	K_OOPS(K_SYSCALL_MEMORY_WRITE(बफर, बफर_आकार));

	z_impl_k_pipe_init(pipe, बफर, बफर_आकार);
}
#समावेश <zephyr/syscalls/k_pipe_init_mrsh.c>

पूर्णांक z_vrfy_k_pipe_read(काष्ठा k_pipe *pipe, uint8_t *तथ्य, माप_प्रकार len, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(pipe, K_OBJ_PIPE));
	K_OOPS(K_SYSCALL_MEMORY_WRITE(तथ्य, len));

	वापस z_impl_k_pipe_read(pipe, तथ्य, len, timeout);
}
#समावेश <zephyr/syscalls/k_pipe_read_mrsh.c>

पूर्णांक z_vrfy_k_pipe_write(काष्ठा k_pipe *pipe, स्थिर uint8_t *तथ्य, माप_प्रकार len, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(pipe, K_OBJ_PIPE));
	K_OOPS(K_SYSCALL_MEMORY_READ(तथ्य, len));

	वापस z_impl_k_pipe_write(pipe, तथ्य, len, timeout);
}
#समावेश <zephyr/syscalls/k_pipe_write_mrsh.c>

व्योम z_vrfy_k_pipe_reset(काष्ठा k_pipe *pipe)
{
	K_OOPS(K_SYSCALL_OBJ(pipe, K_OBJ_PIPE));
	z_impl_k_pipe_reset(pipe);
}
#समावेश <zephyr/syscalls/k_pipe_reset_mrsh.c>

व्योम z_vrfy_k_pipe_close(काष्ठा k_pipe *pipe)
{
	K_OOPS(K_SYSCALL_OBJ(pipe, K_OBJ_PIPE));
	z_impl_k_pipe_close(pipe);
}
#समावेश <zephyr/syscalls/k_pipe_close_mrsh.c>
#पूर्ण_अगर /* CONFIG_USERSPACE */
