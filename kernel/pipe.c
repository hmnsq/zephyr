/*
 * Copyright (c) 2024 Måns Ansgariusson <mansgariusson@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/sys/minmax.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/init.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/kernel.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/internal/syscall_handler.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <ksched.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <kthread.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <wait_q.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <scheduler.h>

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_OBJ_CORE_PIPE
K_OBJ_TYPE_DEFINE(obj_type_pipe, k_pipe, K_OBJ_TYPE_PIPE_ID, ਨਿਰਾ);
#ਜੇ_ਅੰਤ /* CONFIG_OBJ_CORE_PIPE */

ਸਥਿਰ ਸਤਰਅੰਦਰ bool pipe_closed(ਬਣਤਰ k_pipe *pipe)
{
	ਵਾਪਸ (pipe->flags & PIPE_FLAG_OPEN) == 0;
}

ਸਥਿਰ ਸਤਰਅੰਦਰ bool pipe_resetting(ਬਣਤਰ k_pipe *pipe)
{
	ਵਾਪਸ (pipe->flags & PIPE_FLAG_RESET) != 0;
}

ਸਥਿਰ ਸਤਰਅੰਦਰ bool pipe_ਭਰਿਆ(ਬਣਤਰ k_pipe *pipe)
{
	ਵਾਪਸ ring_buf_space_get(&pipe->buf) == 0;
}

ਸਥਿਰ ਸਤਰਅੰਦਰ bool pipe_ਖਾਲੀ_ਥਾਵਾਂ(ਬਣਤਰ k_pipe *pipe)
{
	ਵਾਪਸ ring_buf_is_ਖਾਲੀ_ਥਾਵਾਂ(&pipe->buf);
}

ਬਣਤਰ pipe_buf_spec {
	uint8_t * ਅਚਲ ਅੰਕੜੇ;
	ਅਚਲ size_t len;
	size_t ਵਰਤਿਆ;
};

ਸਥਿਰ ਪੂਰਨ wait_for(_wait_q_t *waitq, ਬਣਤਰ k_pipe *pipe, k_spinlock_ਕੁੰਜੀ_t *ਕੁੰਜੀ,
		    k_timepoint_t ਸਮਾਂ_ਹੱਦਬੰਦੀ, bool *need_resched,
		    ਬਣਤਰ pipe_buf_spec *buf_spec)
{
	k_timeout_t timeout = sys_timepoint_timeout(ਸਮਾਂ_ਹੱਦਬੰਦੀ);
	ਪੂਰਨ rc;

	ਜੇ (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		ਵਾਪਸ -EAGAIN;
	}

	_current->ਅਧਾਰ.ਅਦਲਾ_ਬਦਲੀ_ਅੰਕੜੇ = buf_spec;

	pipe->ਉਡੀਕ++;
	*need_resched = false;
	ਜੇ (waitq == &pipe->space) {
		SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_BLOCKING(k_pipe, ਲਿਖ, pipe, timeout);
	} ਨਹੀਂਤਾਂ {
		SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_BLOCKING(k_pipe, ਪਠ, pipe, timeout);
	}
	rc = z_pend_curr(&pipe->ਤਾਲਾ, *ਕੁੰਜੀ, waitq, timeout);
	*ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&pipe->ਤਾਲਾ);
	pipe->ਉਡੀਕ--;
	ਜੇ (unlikely(pipe_resetting(pipe))) {
		ਜੇ (pipe->ਉਡੀਕ == 0) {
			pipe->flags &= ~PIPE_FLAG_RESET;
		}
		rc = -ECANCELED;
	}

	ਵਾਪਸ rc;
}

ਖਾਲੀ z_impl_k_pipe_init(ਬਣਤਰ k_pipe *pipe, uint8_t *ਬਫਰ, size_t ਬਫਰ_ਮਾਪ)
{
	ring_buf_init(&pipe->buf, ਬਫਰ_ਮਾਪ, ਬਫਰ);
	pipe->flags = PIPE_FLAG_OPEN;
	pipe->ਉਡੀਕ = 0;

	pipe->ਤਾਲਾ = (ਬਣਤਰ k_spinlock){};
	z_waitq_init(&pipe->ਅੰਕੜੇ);
	z_waitq_init(&pipe->space);
	k_object_init(pipe);

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_POLL
	sys_dlist_init(&pipe->poll_events);
#ਜੇ_ਅੰਤ /* CONFIG_POLL */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_OBJ_CORE_PIPE
	k_obj_core_init_and_link(K_OBJ_CORE(pipe), &obj_type_pipe);
#ਜੇ_ਅੰਤ /* CONFIG_OBJ_CORE_PIPE */
	SYS_ਪੋਰਟ_TRACING_OBJ_INIT(k_pipe, pipe, ਬਫਰ, ਬਫਰ_ਮਾਪ);
}

ਸਥਿਰ size_t copy_ਮੰਜ਼ਿਲ_pending_readers(ਬਣਤਰ k_pipe *pipe, bool *need_resched,
				      ਅਚਲ uint8_t *ਅੰਕੜੇ, size_t len)
{
	ਬਣਤਰ k_thread *reader = ਨਿਰਾ;
	ਬਣਤਰ pipe_buf_spec *reader_buf;
	size_t copy_ਮਾਪ, written = 0;

	/*
	 * Attempt a direct data copy to waiting readers if any.
	 * The copy has to be done under the scheduler lock to ensure all the
	 * needed data is copied to the target thread whose buffer spec lives
	 * on that thread's stack, and then the thread unpended only if it
	 * received all the data it wanted, without racing with a potential
	 * thread timeout/cancellation event.
	 */
	ਕਰੋ {
		ਤਾਲਾ_SCHED_SPINLOCK {
			reader = priq_wait_ਉੱਤਮ(&pipe->ਅੰਕੜੇ.waitq);
			ਜੇ (reader == ਨਿਰਾ) {
				K_SPINLOCK_BREAK;
			}

			reader_buf = reader->ਅਧਾਰ.ਅਦਲਾ_ਬਦਲੀ_ਅੰਕੜੇ;
			copy_ਮਾਪ = min(len - written,
					reader_buf->len - reader_buf->ਵਰਤਿਆ);
			ਸਮ੍ਰਿਤੀ_ਨਕਲ(&reader_buf->ਅੰਕੜੇ[reader_buf->ਵਰਤਿਆ],
			       &ਅੰਕੜੇ[written], copy_ਮਾਪ);
			written += copy_ਮਾਪ;
			reader_buf->ਵਰਤਿਆ += copy_ਮਾਪ;

			ਜੇ (reader_buf->ਵਰਤਿਆ < reader_buf->len) {
				/* This reader wants more: don't unpend. */
				reader = ਨਿਰਾ;
			} ਨਹੀਂਤਾਂ {
				/*
				 * This reader has received all the data
				 * it was waiting for. Set its return
				 * value, unpend, abort its timeout, and
				 * ready it, all under the scheduler lock
				 * so a racing timeout handler cannot
				 * observe a half-initialized wake-up.
				 */
				z_thread_return_ਮੁੱਲ_ਸੈੱਟ_with_ਅੰਕੜੇ(reader, 0, ਨਿਰਾ);
				unpend_thread_no_timeout(reader);
				(ਖਾਲੀ)z_try_abort_thread_timeout(reader);
				z_sched_ready_locked(reader);
				*need_resched = true;
			}
		}
	} ਜਦੋਂ (reader != ਨਿਰਾ && written < len);

	ਵਾਪਸ written;
}

ਪੂਰਨ z_impl_k_pipe_write(ਬਣਤਰ k_pipe *pipe, ਅਚਲ uint8_t *ਅੰਕੜੇ, size_t len, k_timeout_t timeout)
{
	ਪੂਰਨ rc;
	size_t written = 0;
	size_t added;
	k_timepoint_t ਸਿਰਾ = sys_timepoint_calc(timeout);
	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&pipe->ਤਾਲਾ);
	bool need_resched = false;

	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_ENTER(k_pipe, ਲਿਖ, pipe, ਅੰਕੜੇ, len, timeout);

	ਜੇ (unlikely(len > ਵੱਧ_ਤੋਂ_ਵੱਧ_ਅੰਕ)) {
		rc = -EOVERFLOW;
		ਜਾਓ ਬਾਹਰਲਾ;
	}

	ਜੇ (unlikely(pipe_resetting(pipe))) {
		rc = -ECANCELED;
		ਜਾਓ ਬਾਹਰਲਾ;
	}

	ਲਈ (;;) {
		ਜੇ (unlikely(pipe_closed(pipe))) {
			rc = -EPIPE;
			ਰੋਕੋ;
		}

		ਜੇ (pipe_ਖਾਲੀ_ਥਾਵਾਂ(pipe)) {
			ਜੇ (IS_ENABLED(CONFIG_KERNEL_COHERENCE)) {
				/*
				 * Systems that enabled this option don't have
				 * their stacks in coherent memory. Given our
				 * pipe_buf_spec is stored on the stack, and
				 * readers may also have their destination
				 * buffer on their stack too, it is not worth
				 * supporting direct-to-readers copy with them.
				 * Simply wake up all pending readers instead.
				 */
				need_resched = z_sched_wake_all(&pipe->ਅੰਕੜੇ, 0, ਨਿਰਾ);
			} ਨਹੀਂਤਾਂ ਜੇ (pipe->ਉਡੀਕ != 0) {
				written += copy_ਮੰਜ਼ਿਲ_pending_readers(pipe, &need_resched,
								   &ਅੰਕੜੇ[written],
								   len - written);
				ਜੇ (written >= len) {
					rc = written;
					ਰੋਕੋ;
				}
			}
		}

		added = ring_buf_put(&pipe->buf, &ਅੰਕੜੇ[written], len - written);

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_POLL
		ਜੇ (added != 0) {
			need_resched |= z_handle_obj_poll_events(&pipe->poll_events,
								 K_POLL_STATE_PIPE_ਅੰਕੜੇ_ਉਪਲਬਧ);
		}
#ਜੇ_ਅੰਤ /* CONFIG_POLL */

		written += added;
		ਜੇ (likely(written == len)) {
			rc = written;
			ਰੋਕੋ;
		}

		rc = wait_for(&pipe->space, pipe, &ਕੁੰਜੀ, ਸਿਰਾ, &need_resched, ਨਿਰਾ);
		ਜੇ (rc != 0) {
			ਜੇ (rc == -EAGAIN) {
				rc = written ? written : -EAGAIN;
			}
			ਰੋਕੋ;
		}
	}
ਬਾਹਰਲਾ:
	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_EXIT(k_pipe, ਲਿਖ, pipe, rc);
	ਜੇ (need_resched) {
		z_reschedule(&pipe->ਤਾਲਾ, ਕੁੰਜੀ);
	} ਨਹੀਂਤਾਂ {
		k_spin_unlock(&pipe->ਤਾਲਾ, ਕੁੰਜੀ);
	}
	ਵਾਪਸ rc;
}

ਪੂਰਨ z_impl_k_pipe_read(ਬਣਤਰ k_pipe *pipe, uint8_t *ਅੰਕੜੇ, size_t len, k_timeout_t timeout)
{
	ਬਣਤਰ pipe_buf_spec buf = { ਅੰਕੜੇ, len, 0 };
	ਪੂਰਨ rc;
	k_timepoint_t ਸਿਰਾ = sys_timepoint_calc(timeout);
	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&pipe->ਤਾਲਾ);
	bool need_resched = false;

	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_ENTER(k_pipe, ਪਠ, pipe, ਅੰਕੜੇ, len, timeout);

	ਜੇ (unlikely(len > ਵੱਧ_ਤੋਂ_ਵੱਧ_ਅੰਕ)) {
		rc = -EOVERFLOW;
		ਜਾਓ ਬਾਹਰਲਾ;
	}

	ਜੇ (unlikely(pipe_resetting(pipe))) {
		rc = -ECANCELED;
		ਜਾਓ ਬਾਹਰਲਾ;
	}

	ਲਈ (;;) {
		ਜੇ (pipe_ਭਰਿਆ(pipe)) {
			/* One or more pending writers may exist. */
			need_resched = z_sched_wake_all(&pipe->space, 0, ਨਿਰਾ);
		}

		buf.ਵਰਤਿਆ += ring_buf_get(&pipe->buf, &ਅੰਕੜੇ[buf.ਵਰਤਿਆ], len - buf.ਵਰਤਿਆ);
		ਜੇ (likely(buf.ਵਰਤਿਆ == len)) {
			rc = buf.ਵਰਤਿਆ;
			ਰੋਕੋ;
		}

		ਜੇ (unlikely(pipe_closed(pipe))) {
			rc = buf.ਵਰਤਿਆ ? buf.ਵਰਤਿਆ : -EPIPE;
			ਰੋਕੋ;
		}

		rc = wait_for(&pipe->ਅੰਕੜੇ, pipe, &ਕੁੰਜੀ, ਸਿਰਾ, &need_resched, &buf);
		ਜੇ (rc != 0) {
			ਜੇ (rc == -EAGAIN) {
				rc = buf.ਵਰਤਿਆ ? buf.ਵਰਤਿਆ : -EAGAIN;
			}
			ਰੋਕੋ;
		}
	}
ਬਾਹਰਲਾ:
	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_EXIT(k_pipe, ਪਠ, pipe, rc);
	ਜੇ (need_resched) {
		z_reschedule(&pipe->ਤਾਲਾ, ਕੁੰਜੀ);
	} ਨਹੀਂਤਾਂ {
		k_spin_unlock(&pipe->ਤਾਲਾ, ਕੁੰਜੀ);
	}
	ਵਾਪਸ rc;
}

ਖਾਲੀ z_impl_k_pipe_reset(ਬਣਤਰ k_pipe *pipe)
{
	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_ENTER(k_pipe, reset, pipe);
	K_SPINLOCK(&pipe->ਤਾਲਾ) {
		ring_buf_reset(&pipe->buf);
		ਜੇ (likely(pipe->ਉਡੀਕ != 0)) {
			pipe->flags |= PIPE_FLAG_RESET;
			z_sched_wake_all(&pipe->ਅੰਕੜੇ, 0, ਨਿਰਾ);
			z_sched_wake_all(&pipe->space, 0, ਨਿਰਾ);
		}
	}
	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_EXIT(k_pipe, reset, pipe);
}

ਖਾਲੀ z_impl_k_pipe_close(ਬਣਤਰ k_pipe *pipe)
{
	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ;
	bool need_resched;

	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_ENTER(k_pipe, close, pipe);

	ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&pipe->ਤਾਲਾ);

	pipe->flags = 0;
	need_resched = z_sched_wake_all(&pipe->ਅੰਕੜੇ, 0, ਨਿਰਾ);
	need_resched |= z_sched_wake_all(&pipe->space, 0, ਨਿਰਾ);

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_POLL
	/*
	 * Closing is a broadcast: wake every registered poller. Later
	 * k_poll() calls see the hangup through is_condition_met().
	 */
	ਜਦੋਂ (z_handle_obj_poll_events(&pipe->poll_events,
					K_POLL_STATE_PIPE_ਅੰਕੜੇ_ਉਪਲਬਧ)) {
		need_resched = true;
	}
#ਜੇ_ਅੰਤ /* CONFIG_POLL */

	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_EXIT(k_pipe, close, pipe);

	ਜੇ (need_resched) {
		z_reschedule(&pipe->ਤਾਲਾ, ਕੁੰਜੀ);
	} ਨਹੀਂਤਾਂ {
		k_spin_unlock(&pipe->ਤਾਲਾ, ਕੁੰਜੀ);
	}
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_USERSPACE
ਖਾਲੀ z_vrfy_k_pipe_init(ਬਣਤਰ k_pipe *pipe, uint8_t *ਬਫਰ, size_t ਬਫਰ_ਮਾਪ)
{
	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(pipe, K_OBJ_PIPE));
	K_OOPS(K_SYSCALL_ਸਮ੍ਰਿਤੀ_WRITE(ਬਫਰ, ਬਫਰ_ਮਾਪ));

	z_impl_k_pipe_init(pipe, ਬਫਰ, ਬਫਰ_ਮਾਪ);
}
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/syscalls/k_pipe_init_mrsh.c>

ਪੂਰਨ z_vrfy_k_pipe_read(ਬਣਤਰ k_pipe *pipe, uint8_t *ਅੰਕੜੇ, size_t len, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(pipe, K_OBJ_PIPE));
	K_OOPS(K_SYSCALL_ਸਮ੍ਰਿਤੀ_WRITE(ਅੰਕੜੇ, len));

	ਵਾਪਸ z_impl_k_pipe_read(pipe, ਅੰਕੜੇ, len, timeout);
}
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/syscalls/k_pipe_read_mrsh.c>

ਪੂਰਨ z_vrfy_k_pipe_write(ਬਣਤਰ k_pipe *pipe, ਅਚਲ uint8_t *ਅੰਕੜੇ, size_t len, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(pipe, K_OBJ_PIPE));
	K_OOPS(K_SYSCALL_ਸਮ੍ਰਿਤੀ_READ(ਅੰਕੜੇ, len));

	ਵਾਪਸ z_impl_k_pipe_write(pipe, ਅੰਕੜੇ, len, timeout);
}
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/syscalls/k_pipe_write_mrsh.c>

ਖਾਲੀ z_vrfy_k_pipe_reset(ਬਣਤਰ k_pipe *pipe)
{
	K_OOPS(K_SYSCALL_OBJ(pipe, K_OBJ_PIPE));
	z_impl_k_pipe_reset(pipe);
}
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/syscalls/k_pipe_reset_mrsh.c>

ਖਾਲੀ z_vrfy_k_pipe_close(ਬਣਤਰ k_pipe *pipe)
{
	K_OOPS(K_SYSCALL_OBJ(pipe, K_OBJ_PIPE));
	z_impl_k_pipe_close(pipe);
}
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/syscalls/k_pipe_close_mrsh.c>
#ਜੇ_ਅੰਤ /* CONFIG_USERSPACE */
