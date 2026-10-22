/*
 * Copyright (c) 2024 Måns Ansgariusson <mansgariusson@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#അന്തര്ഭാവ <zephyr/sys/minmax.h>
#അന്തര്ഭാവ <zephyr/init.h>
#അന്തര്ഭാവ <zephyr/kernel.h>
#അന്തര്ഭാവ <zephyr/internal/syscall_handler.h>
#അന്തര്ഭാവ <ksched.h>
#അന്തര്ഭാവ <kthread.h>
#അന്തര്ഭാവ <wait_q.h>
#അന്തര്ഭാവ <scheduler.h>

#ifdef CONFIG_OBJ_CORE_PIPE
K_OBJ_TYPE_DEFINE(obj_type_pipe, k_pipe, K_OBJ_TYPE_PIPE_ID, ശൂന്യ);
#endif /* CONFIG_OBJ_CORE_PIPE */

സ്ഥിതം വരിയിൽ bool pipe_closed(ഘടന k_pipe *pipe)
{
	തിരികെ (pipe->flags & PIPE_FLAG_OPEN) == 0;
}

സ്ഥിതം വരിയിൽ bool pipe_resetting(ഘടന k_pipe *pipe)
{
	തിരികെ (pipe->flags & PIPE_FLAG_RESET) != 0;
}

സ്ഥിതം വരിയിൽ bool pipe_full(ഘടന k_pipe *pipe)
{
	തിരികെ ring_buf_space_get(&pipe->buf) == 0;
}

സ്ഥിതം വരിയിൽ bool pipe_empty(ഘടന k_pipe *pipe)
{
	തിരികെ ring_buf_is_empty(&pipe->buf);
}

ഘടന pipe_buf_spec {
	uint8_t * സ്ഥിരം data;
	സ്ഥിരം size_t len;
	size_t used;
};

സ്ഥിതം പൂർണ്ണസംഖ്യ wait_for(_wait_q_t *waitq, ഘടന k_pipe *pipe, k_spinlock_താക്കോൽ_t *താക്കോൽ,
		    k_timepoint_t time_limit, bool *need_resched,
		    ഘടന pipe_buf_spec *buf_spec)
{
	k_timeout_t timeout = sys_timepoint_timeout(time_limit);
	പൂർണ്ണസംഖ്യ rc;

	എങ്കിൽ (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		തിരികെ -EAGAIN;
	}

	_current->base.swap_data = buf_spec;

	pipe->waiting++;
	*need_resched = false;
	എങ്കിൽ (waitq == &pipe->space) {
		SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_pipe, ലിഖ, pipe, timeout);
	} അല്ലെങ്കിൽ {
		SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_pipe, പഠ, pipe, timeout);
	}
	rc = z_pend_curr(&pipe->lock, *താക്കോൽ, waitq, timeout);
	*താക്കോൽ = k_spin_lock(&pipe->lock);
	pipe->waiting--;
	എങ്കിൽ (unlikely(pipe_resetting(pipe))) {
		എങ്കിൽ (pipe->waiting == 0) {
			pipe->flags &= ~PIPE_FLAG_RESET;
		}
		rc = -ECANCELED;
	}

	തിരികെ rc;
}

ശൂന്യം z_impl_k_pipe_init(ഘടന k_pipe *pipe, uint8_t *buffer, size_t buffer_size)
{
	ring_buf_init(&pipe->buf, buffer_size, buffer);
	pipe->flags = PIPE_FLAG_OPEN;
	pipe->waiting = 0;

	pipe->lock = (ഘടന k_spinlock){};
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

സ്ഥിതം size_t copy_to_pending_readers(ഘടന k_pipe *pipe, bool *need_resched,
				      സ്ഥിരം uint8_t *data, size_t len)
{
	ഘടന k_thread *reader = ശൂന്യ;
	ഘടന pipe_buf_spec *reader_buf;
	size_t copy_size, written = 0;

	/*
	 * Attempt a direct data copy to waiting readers if any.
	 * The copy has to be done under the scheduler lock to ensure all the
	 * needed data is copied to the target thread whose buffer spec lives
	 * on that thread's stack, and then the thread unpended only if it
	 * received all the data it wanted, without racing with a potential
	 * thread timeout/cancellation event.
	 */
	ചെയ്യുക {
		LOCK_SCHED_SPINLOCK {
			reader = _priq_wait_best(&pipe->data.waitq);
			എങ്കിൽ (reader == ശൂന്യ) {
				K_SPINLOCK_BREAK;
			}

			reader_buf = reader->base.swap_data;
			copy_size = min(len - written,
					reader_buf->len - reader_buf->used);
			memcpy(&reader_buf->data[reader_buf->used],
			       &data[written], copy_size);
			written += copy_size;
			reader_buf->used += copy_size;

			എങ്കിൽ (reader_buf->used < reader_buf->len) {
				/* This reader wants more: don't unpend. */
				reader = ശൂന്യ;
			} അല്ലെങ്കിൽ {
				/*
				 * This reader has received all the data
				 * it was waiting for. Set its return
				 * value, unpend, abort its timeout, and
				 * ready it, all under the scheduler lock
				 * so a racing timeout handler cannot
				 * observe a half-initialized wake-up.
				 */
				z_thread_return_മൂല്യം_set_with_data(reader, 0, ശൂന്യ);
				unpend_thread_no_timeout(reader);
				(ശൂന്യം)z_try_abort_thread_timeout(reader);
				z_sched_ready_locked(reader);
				*need_resched = true;
			}
		}
	} ആകുമ്പോൾ (reader != ശൂന്യ && written < len);

	തിരികെ written;
}

പൂർണ്ണസംഖ്യ z_impl_k_pipe_write(ഘടന k_pipe *pipe, സ്ഥിരം uint8_t *data, size_t len, k_timeout_t timeout)
{
	പൂർണ്ണസംഖ്യ rc;
	size_t written = 0;
	size_t added;
	k_timepoint_t end = sys_timepoint_calc(timeout);
	k_spinlock_താക്കോൽ_t താക്കോൽ = k_spin_lock(&pipe->lock);
	bool need_resched = false;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_pipe, ലിഖ, pipe, data, len, timeout);

	എങ്കിൽ (unlikely(len > INT_MAX)) {
		rc = -EOVERFLOW;
		പോകുക out;
	}

	എങ്കിൽ (unlikely(pipe_resetting(pipe))) {
		rc = -ECANCELED;
		പോകുക out;
	}

	വേണ്ടി (;;) {
		എങ്കിൽ (unlikely(pipe_closed(pipe))) {
			rc = -EPIPE;
			നിർത്തുക;
		}

		എങ്കിൽ (pipe_empty(pipe)) {
			എങ്കിൽ (IS_ENABLED(CONFIG_KERNEL_COHERENCE)) {
				/*
				 * Systems that enabled this option don't have
				 * their stacks in coherent memory. Given our
				 * pipe_buf_spec is stored on the stack, and
				 * readers may also have their destination
				 * buffer on their stack too, it is not worth
				 * supporting direct-to-readers copy with them.
				 * Simply wake up all pending readers instead.
				 */
				need_resched = z_sched_wake_all(&pipe->data, 0, ശൂന്യ);
			} അല്ലെങ്കിൽ എങ്കിൽ (pipe->waiting != 0) {
				written += copy_to_pending_readers(pipe, &need_resched,
								   &data[written],
								   len - written);
				എങ്കിൽ (written >= len) {
					rc = written;
					നിർത്തുക;
				}
			}
		}

		added = ring_buf_put(&pipe->buf, &data[written], len - written);

#ifdef CONFIG_POLL
		എങ്കിൽ (added != 0) {
			need_resched |= z_handle_obj_poll_events(&pipe->poll_events,
								 K_POLL_STATE_PIPE_DATA_AVAILABLE);
		}
#endif /* CONFIG_POLL */

		written += added;
		എങ്കിൽ (likely(written == len)) {
			rc = written;
			നിർത്തുക;
		}

		rc = wait_for(&pipe->space, pipe, &താക്കോൽ, end, &need_resched, ശൂന്യ);
		എങ്കിൽ (rc != 0) {
			എങ്കിൽ (rc == -EAGAIN) {
				rc = written ? written : -EAGAIN;
			}
			നിർത്തുക;
		}
	}
out:
	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_pipe, ലിഖ, pipe, rc);
	എങ്കിൽ (need_resched) {
		z_reschedule(&pipe->lock, താക്കോൽ);
	} അല്ലെങ്കിൽ {
		k_spin_unlock(&pipe->lock, താക്കോൽ);
	}
	തിരികെ rc;
}

പൂർണ്ണസംഖ്യ z_impl_k_pipe_read(ഘടന k_pipe *pipe, uint8_t *data, size_t len, k_timeout_t timeout)
{
	ഘടന pipe_buf_spec buf = { data, len, 0 };
	പൂർണ്ണസംഖ്യ rc;
	k_timepoint_t end = sys_timepoint_calc(timeout);
	k_spinlock_താക്കോൽ_t താക്കോൽ = k_spin_lock(&pipe->lock);
	bool need_resched = false;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_pipe, പഠ, pipe, data, len, timeout);

	എങ്കിൽ (unlikely(len > INT_MAX)) {
		rc = -EOVERFLOW;
		പോകുക out;
	}

	എങ്കിൽ (unlikely(pipe_resetting(pipe))) {
		rc = -ECANCELED;
		പോകുക out;
	}

	വേണ്ടി (;;) {
		എങ്കിൽ (pipe_full(pipe)) {
			/* One or more pending writers may exist. */
			need_resched = z_sched_wake_all(&pipe->space, 0, ശൂന്യ);
		}

		buf.used += ring_buf_get(&pipe->buf, &data[buf.used], len - buf.used);
		എങ്കിൽ (likely(buf.used == len)) {
			rc = buf.used;
			നിർത്തുക;
		}

		എങ്കിൽ (unlikely(pipe_closed(pipe))) {
			rc = buf.used ? buf.used : -EPIPE;
			നിർത്തുക;
		}

		rc = wait_for(&pipe->data, pipe, &താക്കോൽ, end, &need_resched, &buf);
		എങ്കിൽ (rc != 0) {
			എങ്കിൽ (rc == -EAGAIN) {
				rc = buf.used ? buf.used : -EAGAIN;
			}
			നിർത്തുക;
		}
	}
out:
	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_pipe, പഠ, pipe, rc);
	എങ്കിൽ (need_resched) {
		z_reschedule(&pipe->lock, താക്കോൽ);
	} അല്ലെങ്കിൽ {
		k_spin_unlock(&pipe->lock, താക്കോൽ);
	}
	തിരികെ rc;
}

ശൂന്യം z_impl_k_pipe_reset(ഘടന k_pipe *pipe)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_pipe, reset, pipe);
	K_SPINLOCK(&pipe->lock) {
		ring_buf_reset(&pipe->buf);
		എങ്കിൽ (likely(pipe->waiting != 0)) {
			pipe->flags |= PIPE_FLAG_RESET;
			z_sched_wake_all(&pipe->data, 0, ശൂന്യ);
			z_sched_wake_all(&pipe->space, 0, ശൂന്യ);
		}
	}
	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_pipe, reset, pipe);
}

ശൂന്യം z_impl_k_pipe_close(ഘടന k_pipe *pipe)
{
	k_spinlock_താക്കോൽ_t താക്കോൽ;
	bool need_resched;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_pipe, close, pipe);

	താക്കോൽ = k_spin_lock(&pipe->lock);

	pipe->flags = 0;
	need_resched = z_sched_wake_all(&pipe->data, 0, ശൂന്യ);
	need_resched |= z_sched_wake_all(&pipe->space, 0, ശൂന്യ);

#ifdef CONFIG_POLL
	/*
	 * Closing is a broadcast: wake every registered poller. Later
	 * k_poll() calls see the hangup through is_condition_met().
	 */
	ആകുമ്പോൾ (z_handle_obj_poll_events(&pipe->poll_events,
					K_POLL_STATE_PIPE_DATA_AVAILABLE)) {
		need_resched = true;
	}
#endif /* CONFIG_POLL */

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_pipe, close, pipe);

	എങ്കിൽ (need_resched) {
		z_reschedule(&pipe->lock, താക്കോൽ);
	} അല്ലെങ്കിൽ {
		k_spin_unlock(&pipe->lock, താക്കോൽ);
	}
}

#ifdef CONFIG_USERSPACE
ശൂന്യം z_vrfy_k_pipe_init(ഘടന k_pipe *pipe, uint8_t *buffer, size_t buffer_size)
{
	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(pipe, K_OBJ_PIPE));
	K_OOPS(K_SYSCALL_MEMORY_WRITE(buffer, buffer_size));

	z_impl_k_pipe_init(pipe, buffer, buffer_size);
}
#അന്തര്ഭാവ <zephyr/syscalls/k_pipe_init_mrsh.c>

പൂർണ്ണസംഖ്യ z_vrfy_k_pipe_read(ഘടന k_pipe *pipe, uint8_t *data, size_t len, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(pipe, K_OBJ_PIPE));
	K_OOPS(K_SYSCALL_MEMORY_WRITE(data, len));

	തിരികെ z_impl_k_pipe_read(pipe, data, len, timeout);
}
#അന്തര്ഭാവ <zephyr/syscalls/k_pipe_read_mrsh.c>

പൂർണ്ണസംഖ്യ z_vrfy_k_pipe_write(ഘടന k_pipe *pipe, സ്ഥിരം uint8_t *data, size_t len, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(pipe, K_OBJ_PIPE));
	K_OOPS(K_SYSCALL_MEMORY_READ(data, len));

	തിരികെ z_impl_k_pipe_write(pipe, data, len, timeout);
}
#അന്തര്ഭാവ <zephyr/syscalls/k_pipe_write_mrsh.c>

ശൂന്യം z_vrfy_k_pipe_reset(ഘടന k_pipe *pipe)
{
	K_OOPS(K_SYSCALL_OBJ(pipe, K_OBJ_PIPE));
	z_impl_k_pipe_reset(pipe);
}
#അന്തര്ഭാവ <zephyr/syscalls/k_pipe_reset_mrsh.c>

ശൂന്യം z_vrfy_k_pipe_close(ഘടന k_pipe *pipe)
{
	K_OOPS(K_SYSCALL_OBJ(pipe, K_OBJ_PIPE));
	z_impl_k_pipe_close(pipe);
}
#അന്തര്ഭാവ <zephyr/syscalls/k_pipe_close_mrsh.c>
#endif /* CONFIG_USERSPACE */
