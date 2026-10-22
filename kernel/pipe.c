/*
 * Copyright (c) 2024 Måns Ansgariusson <mansgariusson@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ಅನ್ತರ್ಭಾವ <zephyr/sys/minmax.h>
#ಅನ್ತರ್ಭಾವ <zephyr/init.h>
#ಅನ್ತರ್ಭಾವ <zephyr/kernel.h>
#ಅನ್ತರ್ಭಾವ <zephyr/internal/syscall_handler.h>
#ಅನ್ತರ್ಭಾವ <ksched.h>
#ಅನ್ತರ್ಭಾವ <kthread.h>
#ಅನ್ತರ್ಭಾವ <wait_q.h>
#ಅನ್ತರ್ಭಾವ <scheduler.h>

#ifdef CONFIG_OBJ_CORE_PIPE
K_OBJ_TYPE_DEFINE(obj_type_pipe, k_pipe, K_OBJ_TYPE_PIPE_ID, ರಿಕ್ತ);
#endif /* CONFIG_OBJ_CORE_PIPE */

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ bool pipe_closed(ಸಂರಚನಾ k_pipe *pipe)
{
	ಪ್ರತ್ಯಾಗಮ (pipe->flags & PIPE_FLAG_OPEN) == 0;
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ bool pipe_resetting(ಸಂರಚನಾ k_pipe *pipe)
{
	ಪ್ರತ್ಯಾಗಮ (pipe->flags & PIPE_FLAG_RESET) != 0;
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ bool pipe_full(ಸಂರಚನಾ k_pipe *pipe)
{
	ಪ್ರತ್ಯಾಗಮ ring_buf_space_get(&pipe->buf) == 0;
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ bool pipe_empty(ಸಂರಚನಾ k_pipe *pipe)
{
	ಪ್ರತ್ಯಾಗಮ ring_buf_is_empty(&pipe->buf);
}

ಸಂರಚನಾ pipe_buf_spec {
	uint8_t * ಧ್ರುವ data;
	ಧ್ರುವ size_t len;
	size_t used;
};

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ wait_for(_wait_q_t *waitq, ಸಂರಚನಾ k_pipe *pipe, k_spinlock_ಕೀಲಿ_t *ಕೀಲಿ,
		    k_timepoint_t time_limit, bool *need_resched,
		    ಸಂರಚನಾ pipe_buf_spec *buf_spec)
{
	k_timeout_t timeout = sys_timepoint_timeout(time_limit);
	ಪೂರ್ಣಾಙ್ಕ rc;

	ಯದಿ (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		ಪ್ರತ್ಯಾಗಮ -EAGAIN;
	}

	_current->base.swap_data = buf_spec;

	pipe->waiting++;
	*need_resched = false;
	ಯದಿ (waitq == &pipe->space) {
		SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_pipe, ಲಿಖ, pipe, timeout);
	} ಅನ್ಯಥಾ {
		SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_pipe, ಪಠ, pipe, timeout);
	}
	rc = z_pend_curr(&pipe->lock, *ಕೀಲಿ, waitq, timeout);
	*ಕೀಲಿ = k_spin_lock(&pipe->lock);
	pipe->waiting--;
	ಯದಿ (unlikely(pipe_resetting(pipe))) {
		ಯದಿ (pipe->waiting == 0) {
			pipe->flags &= ~PIPE_FLAG_RESET;
		}
		rc = -ECANCELED;
	}

	ಪ್ರತ್ಯಾಗಮ rc;
}

ಶೂನ್ಯ z_impl_k_pipe_init(ಸಂರಚನಾ k_pipe *pipe, uint8_t *buffer, size_t buffer_size)
{
	ring_buf_init(&pipe->buf, buffer_size, buffer);
	pipe->flags = PIPE_FLAG_OPEN;
	pipe->waiting = 0;

	pipe->lock = (ಸಂರಚನಾ k_spinlock){};
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

ಸ್ಥಿರ size_t copy_to_pending_readers(ಸಂರಚನಾ k_pipe *pipe, bool *need_resched,
				      ಧ್ರುವ uint8_t *data, size_t len)
{
	ಸಂರಚನಾ k_thread *reader = ರಿಕ್ತ;
	ಸಂರಚನಾ pipe_buf_spec *reader_buf;
	size_t copy_size, written = 0;

	/*
	 * Attempt a direct data copy to waiting readers if any.
	 * The copy has to be done under the scheduler lock to ensure all the
	 * needed data is copied to the target thread whose buffer spec lives
	 * on that thread's stack, and then the thread unpended only if it
	 * received all the data it wanted, without racing with a potential
	 * thread timeout/cancellation event.
	 */
	ಕುರು {
		LOCK_SCHED_SPINLOCK {
			reader = _priq_wait_best(&pipe->data.waitq);
			ಯದಿ (reader == ರಿಕ್ತ) {
				K_SPINLOCK_BREAK;
			}

			reader_buf = reader->base.swap_data;
			copy_size = min(len - written,
					reader_buf->len - reader_buf->used);
			memcpy(&reader_buf->data[reader_buf->used],
			       &data[written], copy_size);
			written += copy_size;
			reader_buf->used += copy_size;

			ಯದಿ (reader_buf->used < reader_buf->len) {
				/* This reader wants more: don't unpend. */
				reader = ರಿಕ್ತ;
			} ಅನ್ಯಥಾ {
				/*
				 * This reader has received all the data
				 * it was waiting for. Set its return
				 * value, unpend, abort its timeout, and
				 * ready it, all under the scheduler lock
				 * so a racing timeout handler cannot
				 * observe a half-initialized wake-up.
				 */
				z_thread_return_ಮೌಲ್ಯ_set_with_data(reader, 0, ರಿಕ್ತ);
				unpend_thread_no_timeout(reader);
				(ಶೂನ್ಯ)z_try_abort_thread_timeout(reader);
				z_sched_ready_locked(reader);
				*need_resched = true;
			}
		}
	} ಯಾವತ್ (reader != ರಿಕ್ತ && written < len);

	ಪ್ರತ್ಯಾಗಮ written;
}

ಪೂರ್ಣಾಙ್ಕ z_impl_k_pipe_write(ಸಂರಚನಾ k_pipe *pipe, ಧ್ರುವ uint8_t *data, size_t len, k_timeout_t timeout)
{
	ಪೂರ್ಣಾಙ್ಕ rc;
	size_t written = 0;
	size_t added;
	k_timepoint_t end = sys_timepoint_calc(timeout);
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_lock(&pipe->lock);
	bool need_resched = false;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_pipe, ಲಿಖ, pipe, data, len, timeout);

	ಯದಿ (unlikely(len > INT_MAX)) {
		rc = -EOVERFLOW;
		ಗಚ್ಛ out;
	}

	ಯದಿ (unlikely(pipe_resetting(pipe))) {
		rc = -ECANCELED;
		ಗಚ್ಛ out;
	}

	ಪ್ರತಿ (;;) {
		ಯದಿ (unlikely(pipe_closed(pipe))) {
			rc = -EPIPE;
			ಭಙ್ಗ;
		}

		ಯದಿ (pipe_empty(pipe)) {
			ಯದಿ (IS_ENABLED(CONFIG_KERNEL_COHERENCE)) {
				/*
				 * Systems that enabled this option don't have
				 * their stacks in coherent memory. Given our
				 * pipe_buf_spec is stored on the stack, and
				 * readers may also have their destination
				 * buffer on their stack too, it is not worth
				 * supporting direct-to-readers copy with them.
				 * Simply wake up all pending readers instead.
				 */
				need_resched = z_sched_wake_all(&pipe->data, 0, ರಿಕ್ತ);
			} ಅನ್ಯಥಾ ಯದಿ (pipe->waiting != 0) {
				written += copy_to_pending_readers(pipe, &need_resched,
								   &data[written],
								   len - written);
				ಯದಿ (written >= len) {
					rc = written;
					ಭಙ್ಗ;
				}
			}
		}

		added = ring_buf_put(&pipe->buf, &data[written], len - written);

#ifdef CONFIG_POLL
		ಯದಿ (added != 0) {
			need_resched |= z_handle_obj_poll_events(&pipe->poll_events,
								 K_POLL_STATE_PIPE_DATA_AVAILABLE);
		}
#endif /* CONFIG_POLL */

		written += added;
		ಯದಿ (likely(written == len)) {
			rc = written;
			ಭಙ್ಗ;
		}

		rc = wait_for(&pipe->space, pipe, &ಕೀಲಿ, end, &need_resched, ರಿಕ್ತ);
		ಯದಿ (rc != 0) {
			ಯದಿ (rc == -EAGAIN) {
				rc = written ? written : -EAGAIN;
			}
			ಭಙ್ಗ;
		}
	}
out:
	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_pipe, ಲಿಖ, pipe, rc);
	ಯದಿ (need_resched) {
		z_reschedule(&pipe->lock, ಕೀಲಿ);
	} ಅನ್ಯಥಾ {
		k_spin_unlock(&pipe->lock, ಕೀಲಿ);
	}
	ಪ್ರತ್ಯಾಗಮ rc;
}

ಪೂರ್ಣಾಙ್ಕ z_impl_k_pipe_read(ಸಂರಚನಾ k_pipe *pipe, uint8_t *data, size_t len, k_timeout_t timeout)
{
	ಸಂರಚನಾ pipe_buf_spec buf = { data, len, 0 };
	ಪೂರ್ಣಾಙ್ಕ rc;
	k_timepoint_t end = sys_timepoint_calc(timeout);
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_lock(&pipe->lock);
	bool need_resched = false;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_pipe, ಪಠ, pipe, data, len, timeout);

	ಯದಿ (unlikely(len > INT_MAX)) {
		rc = -EOVERFLOW;
		ಗಚ್ಛ out;
	}

	ಯದಿ (unlikely(pipe_resetting(pipe))) {
		rc = -ECANCELED;
		ಗಚ್ಛ out;
	}

	ಪ್ರತಿ (;;) {
		ಯದಿ (pipe_full(pipe)) {
			/* One or more pending writers may exist. */
			need_resched = z_sched_wake_all(&pipe->space, 0, ರಿಕ್ತ);
		}

		buf.used += ring_buf_get(&pipe->buf, &data[buf.used], len - buf.used);
		ಯದಿ (likely(buf.used == len)) {
			rc = buf.used;
			ಭಙ್ಗ;
		}

		ಯದಿ (unlikely(pipe_closed(pipe))) {
			rc = buf.used ? buf.used : -EPIPE;
			ಭಙ್ಗ;
		}

		rc = wait_for(&pipe->data, pipe, &ಕೀಲಿ, end, &need_resched, &buf);
		ಯದಿ (rc != 0) {
			ಯದಿ (rc == -EAGAIN) {
				rc = buf.used ? buf.used : -EAGAIN;
			}
			ಭಙ್ಗ;
		}
	}
out:
	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_pipe, ಪಠ, pipe, rc);
	ಯದಿ (need_resched) {
		z_reschedule(&pipe->lock, ಕೀಲಿ);
	} ಅನ್ಯಥಾ {
		k_spin_unlock(&pipe->lock, ಕೀಲಿ);
	}
	ಪ್ರತ್ಯಾಗಮ rc;
}

ಶೂನ್ಯ z_impl_k_pipe_reset(ಸಂರಚನಾ k_pipe *pipe)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_pipe, reset, pipe);
	K_SPINLOCK(&pipe->lock) {
		ring_buf_reset(&pipe->buf);
		ಯದಿ (likely(pipe->waiting != 0)) {
			pipe->flags |= PIPE_FLAG_RESET;
			z_sched_wake_all(&pipe->data, 0, ರಿಕ್ತ);
			z_sched_wake_all(&pipe->space, 0, ರಿಕ್ತ);
		}
	}
	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_pipe, reset, pipe);
}

ಶೂನ್ಯ z_impl_k_pipe_close(ಸಂರಚನಾ k_pipe *pipe)
{
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ;
	bool need_resched;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_pipe, close, pipe);

	ಕೀಲಿ = k_spin_lock(&pipe->lock);

	pipe->flags = 0;
	need_resched = z_sched_wake_all(&pipe->data, 0, ರಿಕ್ತ);
	need_resched |= z_sched_wake_all(&pipe->space, 0, ರಿಕ್ತ);

#ifdef CONFIG_POLL
	/*
	 * Closing is a broadcast: wake every registered poller. Later
	 * k_poll() calls see the hangup through is_condition_met().
	 */
	ಯಾವತ್ (z_handle_obj_poll_events(&pipe->poll_events,
					K_POLL_STATE_PIPE_DATA_AVAILABLE)) {
		need_resched = true;
	}
#endif /* CONFIG_POLL */

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_pipe, close, pipe);

	ಯದಿ (need_resched) {
		z_reschedule(&pipe->lock, ಕೀಲಿ);
	} ಅನ್ಯಥಾ {
		k_spin_unlock(&pipe->lock, ಕೀಲಿ);
	}
}

#ifdef CONFIG_USERSPACE
ಶೂನ್ಯ z_vrfy_k_pipe_init(ಸಂರಚನಾ k_pipe *pipe, uint8_t *buffer, size_t buffer_size)
{
	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(pipe, K_OBJ_PIPE));
	K_OOPS(K_SYSCALL_MEMORY_WRITE(buffer, buffer_size));

	z_impl_k_pipe_init(pipe, buffer, buffer_size);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_pipe_init_mrsh.c>

ಪೂರ್ಣಾಙ್ಕ z_vrfy_k_pipe_read(ಸಂರಚನಾ k_pipe *pipe, uint8_t *data, size_t len, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(pipe, K_OBJ_PIPE));
	K_OOPS(K_SYSCALL_MEMORY_WRITE(data, len));

	ಪ್ರತ್ಯಾಗಮ z_impl_k_pipe_read(pipe, data, len, timeout);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_pipe_read_mrsh.c>

ಪೂರ್ಣಾಙ್ಕ z_vrfy_k_pipe_write(ಸಂರಚನಾ k_pipe *pipe, ಧ್ರುವ uint8_t *data, size_t len, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(pipe, K_OBJ_PIPE));
	K_OOPS(K_SYSCALL_MEMORY_READ(data, len));

	ಪ್ರತ್ಯಾಗಮ z_impl_k_pipe_write(pipe, data, len, timeout);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_pipe_write_mrsh.c>

ಶೂನ್ಯ z_vrfy_k_pipe_reset(ಸಂರಚನಾ k_pipe *pipe)
{
	K_OOPS(K_SYSCALL_OBJ(pipe, K_OBJ_PIPE));
	z_impl_k_pipe_reset(pipe);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_pipe_reset_mrsh.c>

ಶೂನ್ಯ z_vrfy_k_pipe_close(ಸಂರಚನಾ k_pipe *pipe)
{
	K_OOPS(K_SYSCALL_OBJ(pipe, K_OBJ_PIPE));
	z_impl_k_pipe_close(pipe);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_pipe_close_mrsh.c>
#endif /* CONFIG_USERSPACE */
