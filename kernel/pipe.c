/*
 * Copyright (c) 2024 Måns Ansgariusson <mansgariusson@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#अन्तर्भाव <zephyr/sys/minmax.h>
#अन्तर्भाव <zephyr/init.h>
#अन्तर्भाव <zephyr/kernel.h>
#अन्तर्भाव <zephyr/internal/syscall_handler.h>
#अन्तर्भाव <ksched.h>
#अन्तर्भाव <kthread.h>
#अन्तर्भाव <wait_q.h>
#अन्तर्भाव <scheduler.h>

#ifdef CONFIG_OBJ_CORE_PIPE
K_OBJ_TYPE_DEFINE(obj_type_pipe, k_pipe, K_OBJ_TYPE_PIPE_ID, रिक्त);
#endif /* CONFIG_OBJ_CORE_PIPE */

स्थिर अन्तःस्थ bool pipe_closed(संरचना k_pipe *pipe)
{
	प्रत्यागम (pipe->flags & PIPE_FLAG_OPEN) == 0;
}

स्थिर अन्तःस्थ bool pipe_resetting(संरचना k_pipe *pipe)
{
	प्रत्यागम (pipe->flags & PIPE_FLAG_RESET) != 0;
}

स्थिर अन्तःस्थ bool pipe_full(संरचना k_pipe *pipe)
{
	प्रत्यागम ring_buf_space_get(&pipe->buf) == 0;
}

स्थिर अन्तःस्थ bool pipe_empty(संरचना k_pipe *pipe)
{
	प्रत्यागम ring_buf_is_empty(&pipe->buf);
}

संरचना pipe_buf_spec {
	uint8_t * ध्रुव data;
	ध्रुव size_t len;
	size_t used;
};

स्थिर पूर्णाङ्क wait_for(_wait_q_t *waitq, संरचना k_pipe *pipe, k_spinlock_किल्ली_t *किल्ली,
		    k_timepoint_t time_limit, bool *need_resched,
		    संरचना pipe_buf_spec *buf_spec)
{
	k_timeout_t timeout = sys_timepoint_timeout(time_limit);
	पूर्णाङ्क rc;

	यदि (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		प्रत्यागम -EAGAIN;
	}

	_current->base.swap_data = buf_spec;

	pipe->waiting++;
	*need_resched = false;
	यदि (waitq == &pipe->space) {
		SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_pipe, लिख, pipe, timeout);
	} अन्यथा {
		SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_pipe, पठ, pipe, timeout);
	}
	rc = z_pend_curr(&pipe->lock, *किल्ली, waitq, timeout);
	*किल्ली = k_spin_lock(&pipe->lock);
	pipe->waiting--;
	यदि (unlikely(pipe_resetting(pipe))) {
		यदि (pipe->waiting == 0) {
			pipe->flags &= ~PIPE_FLAG_RESET;
		}
		rc = -ECANCELED;
	}

	प्रत्यागम rc;
}

शून्य z_impl_k_pipe_init(संरचना k_pipe *pipe, uint8_t *buffer, size_t buffer_size)
{
	ring_buf_init(&pipe->buf, buffer_size, buffer);
	pipe->flags = PIPE_FLAG_OPEN;
	pipe->waiting = 0;

	pipe->lock = (संरचना k_spinlock){};
	z_waitq_init(&pipe->data);
	z_waitq_init(&pipe->space);
	k_object_init(pipe);

#ifdef CONFIG_POLL
	sys_dlist_init(&pipe->poll_events);
#endif /* CONFIG_POLL */
#ifdef CONFIG_OBJ_CORE_PIPE
	k_obj_core_init_and_link(K_OBJ_CORE(pipe), &obj_type_pipe);
#endif /* CONFIG_OBJ_CORE_PIPE */
	SYS_PORT_TRACING_OBJ_INIT(k_pipe, pipe, buffer, buffer_size);
}

स्थिर size_t copy_to_pending_readers(संरचना k_pipe *pipe, bool *need_resched,
				      ध्रुव uint8_t *data, size_t len)
{
	संरचना k_thread *reader = रिक्त;
	संरचना pipe_buf_spec *reader_buf;
	size_t copy_size, written = 0;

	/*
	 * Attempt a direct data copy to waiting readers if any.
	 * The copy has to be done under the scheduler lock to ensure all the
	 * needed data is copied to the target thread whose buffer spec lives
	 * on that thread's stack, and then the thread unpended only if it
	 * received all the data it wanted, without racing with a potential
	 * thread timeout/cancellation event.
	 */
	कुरु {
		LOCK_SCHED_SPINLOCK {
			reader = _priq_wait_best(&pipe->data.waitq);
			यदि (reader == रिक्त) {
				K_SPINLOCK_BREAK;
			}

			reader_buf = reader->base.swap_data;
			copy_size = min(len - written,
					reader_buf->len - reader_buf->used);
			memcpy(&reader_buf->data[reader_buf->used],
			       &data[written], copy_size);
			written += copy_size;
			reader_buf->used += copy_size;

			यदि (reader_buf->used < reader_buf->len) {
				/* This reader wants more: don't unpend. */
				reader = रिक्त;
			} अन्यथा {
				/*
				 * This reader has received all the data
				 * it was waiting for. Set its return
				 * value, unpend, abort its timeout, and
				 * ready it, all under the scheduler lock
				 * so a racing timeout handler cannot
				 * observe a half-initialized wake-up.
				 */
				z_thread_return_मूल्य_set_with_data(reader, 0, रिक्त);
				unpend_thread_no_timeout(reader);
				(शून्य)z_try_abort_thread_timeout(reader);
				z_sched_ready_locked(reader);
				*need_resched = true;
			}
		}
	} यावत् (reader != रिक्त && written < len);

	प्रत्यागम written;
}

पूर्णाङ्क z_impl_k_pipe_write(संरचना k_pipe *pipe, ध्रुव uint8_t *data, size_t len, k_timeout_t timeout)
{
	पूर्णाङ्क rc;
	size_t written = 0;
	size_t added;
	k_timepoint_t end = sys_timepoint_calc(timeout);
	k_spinlock_किल्ली_t किल्ली = k_spin_lock(&pipe->lock);
	bool need_resched = false;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_pipe, लिख, pipe, data, len, timeout);

	यदि (unlikely(len > INT_MAX)) {
		rc = -EOVERFLOW;
		गच्छ out;
	}

	यदि (unlikely(pipe_resetting(pipe))) {
		rc = -ECANCELED;
		गच्छ out;
	}

	प्रति (;;) {
		यदि (unlikely(pipe_closed(pipe))) {
			rc = -EPIPE;
			भङ्ग;
		}

		यदि (pipe_empty(pipe)) {
			यदि (IS_ENABLED(CONFIG_KERNEL_COHERENCE)) {
				/*
				 * Systems that enabled this option don't have
				 * their stacks in coherent memory. Given our
				 * pipe_buf_spec is stored on the stack, and
				 * readers may also have their destination
				 * buffer on their stack too, it is not worth
				 * supporting direct-to-readers copy with them.
				 * Simply wake up all pending readers instead.
				 */
				need_resched = z_sched_wake_all(&pipe->data, 0, रिक्त);
			} अन्यथा यदि (pipe->waiting != 0) {
				written += copy_to_pending_readers(pipe, &need_resched,
								   &data[written],
								   len - written);
				यदि (written >= len) {
					rc = written;
					भङ्ग;
				}
			}
		}

		added = ring_buf_put(&pipe->buf, &data[written], len - written);

#ifdef CONFIG_POLL
		यदि (added != 0) {
			need_resched |= z_handle_obj_poll_events(&pipe->poll_events,
								 K_POLL_STATE_PIPE_DATA_AVAILABLE);
		}
#endif /* CONFIG_POLL */

		written += added;
		यदि (likely(written == len)) {
			rc = written;
			भङ्ग;
		}

		rc = wait_for(&pipe->space, pipe, &किल्ली, end, &need_resched, रिक्त);
		यदि (rc != 0) {
			यदि (rc == -EAGAIN) {
				rc = written ? written : -EAGAIN;
			}
			भङ्ग;
		}
	}
out:
	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_pipe, लिख, pipe, rc);
	यदि (need_resched) {
		z_reschedule(&pipe->lock, किल्ली);
	} अन्यथा {
		k_spin_unlock(&pipe->lock, किल्ली);
	}
	प्रत्यागम rc;
}

पूर्णाङ्क z_impl_k_pipe_read(संरचना k_pipe *pipe, uint8_t *data, size_t len, k_timeout_t timeout)
{
	संरचना pipe_buf_spec buf = { data, len, 0 };
	पूर्णाङ्क rc;
	k_timepoint_t end = sys_timepoint_calc(timeout);
	k_spinlock_किल्ली_t किल्ली = k_spin_lock(&pipe->lock);
	bool need_resched = false;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_pipe, पठ, pipe, data, len, timeout);

	यदि (unlikely(len > INT_MAX)) {
		rc = -EOVERFLOW;
		गच्छ out;
	}

	यदि (unlikely(pipe_resetting(pipe))) {
		rc = -ECANCELED;
		गच्छ out;
	}

	प्रति (;;) {
		यदि (pipe_full(pipe)) {
			/* One or more pending writers may exist. */
			need_resched = z_sched_wake_all(&pipe->space, 0, रिक्त);
		}

		buf.used += ring_buf_get(&pipe->buf, &data[buf.used], len - buf.used);
		यदि (likely(buf.used == len)) {
			rc = buf.used;
			भङ्ग;
		}

		यदि (unlikely(pipe_closed(pipe))) {
			rc = buf.used ? buf.used : -EPIPE;
			भङ्ग;
		}

		rc = wait_for(&pipe->data, pipe, &किल्ली, end, &need_resched, &buf);
		यदि (rc != 0) {
			यदि (rc == -EAGAIN) {
				rc = buf.used ? buf.used : -EAGAIN;
			}
			भङ्ग;
		}
	}
out:
	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_pipe, पठ, pipe, rc);
	यदि (need_resched) {
		z_reschedule(&pipe->lock, किल्ली);
	} अन्यथा {
		k_spin_unlock(&pipe->lock, किल्ली);
	}
	प्रत्यागम rc;
}

शून्य z_impl_k_pipe_reset(संरचना k_pipe *pipe)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_pipe, reset, pipe);
	K_SPINLOCK(&pipe->lock) {
		ring_buf_reset(&pipe->buf);
		यदि (likely(pipe->waiting != 0)) {
			pipe->flags |= PIPE_FLAG_RESET;
			z_sched_wake_all(&pipe->data, 0, रिक्त);
			z_sched_wake_all(&pipe->space, 0, रिक्त);
		}
	}
	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_pipe, reset, pipe);
}

शून्य z_impl_k_pipe_close(संरचना k_pipe *pipe)
{
	k_spinlock_किल्ली_t किल्ली;
	bool need_resched;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_pipe, close, pipe);

	किल्ली = k_spin_lock(&pipe->lock);

	pipe->flags = 0;
	need_resched = z_sched_wake_all(&pipe->data, 0, रिक्त);
	need_resched |= z_sched_wake_all(&pipe->space, 0, रिक्त);

#ifdef CONFIG_POLL
	/*
	 * Closing is a broadcast: wake every registered poller. Later
	 * k_poll() calls see the hangup through is_condition_met().
	 */
	यावत् (z_handle_obj_poll_events(&pipe->poll_events,
					K_POLL_STATE_PIPE_DATA_AVAILABLE)) {
		need_resched = true;
	}
#endif /* CONFIG_POLL */

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_pipe, close, pipe);

	यदि (need_resched) {
		z_reschedule(&pipe->lock, किल्ली);
	} अन्यथा {
		k_spin_unlock(&pipe->lock, किल्ली);
	}
}

#ifdef CONFIG_USERSPACE
शून्य z_vrfy_k_pipe_init(संरचना k_pipe *pipe, uint8_t *buffer, size_t buffer_size)
{
	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(pipe, K_OBJ_PIPE));
	K_OOPS(K_SYSCALL_MEMORY_WRITE(buffer, buffer_size));

	z_impl_k_pipe_init(pipe, buffer, buffer_size);
}
#अन्तर्भाव <zephyr/syscalls/k_pipe_init_mrsh.c>

पूर्णाङ्क z_vrfy_k_pipe_read(संरचना k_pipe *pipe, uint8_t *data, size_t len, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(pipe, K_OBJ_PIPE));
	K_OOPS(K_SYSCALL_MEMORY_WRITE(data, len));

	प्रत्यागम z_impl_k_pipe_read(pipe, data, len, timeout);
}
#अन्तर्भाव <zephyr/syscalls/k_pipe_read_mrsh.c>

पूर्णाङ्क z_vrfy_k_pipe_write(संरचना k_pipe *pipe, ध्रुव uint8_t *data, size_t len, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(pipe, K_OBJ_PIPE));
	K_OOPS(K_SYSCALL_MEMORY_READ(data, len));

	प्रत्यागम z_impl_k_pipe_write(pipe, data, len, timeout);
}
#अन्तर्भाव <zephyr/syscalls/k_pipe_write_mrsh.c>

शून्य z_vrfy_k_pipe_reset(संरचना k_pipe *pipe)
{
	K_OOPS(K_SYSCALL_OBJ(pipe, K_OBJ_PIPE));
	z_impl_k_pipe_reset(pipe);
}
#अन्तर्भाव <zephyr/syscalls/k_pipe_reset_mrsh.c>

शून्य z_vrfy_k_pipe_close(संरचना k_pipe *pipe)
{
	K_OOPS(K_SYSCALL_OBJ(pipe, K_OBJ_PIPE));
	z_impl_k_pipe_close(pipe);
}
#अन्तर्भाव <zephyr/syscalls/k_pipe_close_mrsh.c>
#endif /* CONFIG_USERSPACE */
