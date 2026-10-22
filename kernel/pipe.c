/*
 * Copyright (c) 2024 Måns Ansgariusson <mansgariusson@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#அந்தர்பாவ <zephyr/sys/minmax.h>
#அந்தர்பாவ <zephyr/init.h>
#அந்தர்பாவ <zephyr/kernel.h>
#அந்தர்பாவ <zephyr/internal/syscall_handler.h>
#அந்தர்பாவ <ksched.h>
#அந்தர்பாவ <kthread.h>
#அந்தர்பாவ <wait_q.h>
#அந்தர்பாவ <scheduler.h>

#ifdef CONFIG_OBJ_CORE_PIPE
K_OBJ_TYPE_DEFINE(obj_type_pipe, k_pipe, K_OBJ_TYPE_PIPE_ID, வெற்று);
#endif /* CONFIG_OBJ_CORE_PIPE */

நிலையான உள்வரி bool pipe_closed(கட்டமைப்பு k_pipe *pipe)
{
	திரும்பு (pipe->flags & PIPE_FLAG_OPEN) == 0;
}

நிலையான உள்வரி bool pipe_resetting(கட்டமைப்பு k_pipe *pipe)
{
	திரும்பு (pipe->flags & PIPE_FLAG_RESET) != 0;
}

நிலையான உள்வரி bool pipe_full(கட்டமைப்பு k_pipe *pipe)
{
	திரும்பு ring_buf_space_get(&pipe->buf) == 0;
}

நிலையான உள்வரி bool pipe_empty(கட்டமைப்பு k_pipe *pipe)
{
	திரும்பு ring_buf_is_empty(&pipe->buf);
}

கட்டமைப்பு pipe_buf_spec {
	uint8_t * மாறிலி data;
	மாறிலி size_t len;
	size_t used;
};

நிலையான முழு wait_for(_wait_q_t *waitq, கட்டமைப்பு k_pipe *pipe, k_spinlock_சாவி_t *சாவி,
		    k_timepoint_t time_limit, bool *need_resched,
		    கட்டமைப்பு pipe_buf_spec *buf_spec)
{
	k_timeout_t timeout = sys_timepoint_timeout(time_limit);
	முழு rc;

	எனில் (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		திரும்பு -EAGAIN;
	}

	_current->base.swap_data = buf_spec;

	pipe->waiting++;
	*need_resched = false;
	எனில் (waitq == &pipe->space) {
		SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_pipe, லிக, pipe, timeout);
	} இல்லையெனில் {
		SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_pipe, பட, pipe, timeout);
	}
	rc = z_pend_curr(&pipe->lock, *சாவி, waitq, timeout);
	*சாவி = k_spin_lock(&pipe->lock);
	pipe->waiting--;
	எனில் (unlikely(pipe_resetting(pipe))) {
		எனில் (pipe->waiting == 0) {
			pipe->flags &= ~PIPE_FLAG_RESET;
		}
		rc = -ECANCELED;
	}

	திரும்பு rc;
}

வெறுமை z_impl_k_pipe_init(கட்டமைப்பு k_pipe *pipe, uint8_t *buffer, size_t buffer_size)
{
	ring_buf_init(&pipe->buf, buffer_size, buffer);
	pipe->flags = PIPE_FLAG_OPEN;
	pipe->waiting = 0;

	pipe->lock = (கட்டமைப்பு k_spinlock){};
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

நிலையான size_t copy_to_pending_readers(கட்டமைப்பு k_pipe *pipe, bool *need_resched,
				      மாறிலி uint8_t *data, size_t len)
{
	கட்டமைப்பு k_thread *reader = வெற்று;
	கட்டமைப்பு pipe_buf_spec *reader_buf;
	size_t copy_size, written = 0;

	/*
	 * Attempt a direct data copy to waiting readers if any.
	 * The copy has to be done under the scheduler lock to ensure all the
	 * needed data is copied to the target thread whose buffer spec lives
	 * on that thread's stack, and then the thread unpended only if it
	 * received all the data it wanted, without racing with a potential
	 * thread timeout/cancellation event.
	 */
	செய் {
		LOCK_SCHED_SPINLOCK {
			reader = _priq_wait_best(&pipe->data.waitq);
			எனில் (reader == வெற்று) {
				K_SPINLOCK_BREAK;
			}

			reader_buf = reader->base.swap_data;
			copy_size = min(len - written,
					reader_buf->len - reader_buf->used);
			memcpy(&reader_buf->data[reader_buf->used],
			       &data[written], copy_size);
			written += copy_size;
			reader_buf->used += copy_size;

			எனில் (reader_buf->used < reader_buf->len) {
				/* This reader wants more: don't unpend. */
				reader = வெற்று;
			} இல்லையெனில் {
				/*
				 * This reader has received all the data
				 * it was waiting for. Set its return
				 * value, unpend, abort its timeout, and
				 * ready it, all under the scheduler lock
				 * so a racing timeout handler cannot
				 * observe a half-initialized wake-up.
				 */
				z_thread_return_மதிப்பு_set_with_data(reader, 0, வெற்று);
				unpend_thread_no_timeout(reader);
				(வெறுமை)z_try_abort_thread_timeout(reader);
				z_sched_ready_locked(reader);
				*need_resched = true;
			}
		}
	} போது (reader != வெற்று && written < len);

	திரும்பு written;
}

முழு z_impl_k_pipe_write(கட்டமைப்பு k_pipe *pipe, மாறிலி uint8_t *data, size_t len, k_timeout_t timeout)
{
	முழு rc;
	size_t written = 0;
	size_t added;
	k_timepoint_t end = sys_timepoint_calc(timeout);
	k_spinlock_சாவி_t சாவி = k_spin_lock(&pipe->lock);
	bool need_resched = false;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_pipe, லிக, pipe, data, len, timeout);

	எனில் (unlikely(len > INT_MAX)) {
		rc = -EOVERFLOW;
		செல் out;
	}

	எனில் (unlikely(pipe_resetting(pipe))) {
		rc = -ECANCELED;
		செல் out;
	}

	க்கு (;;) {
		எனில் (unlikely(pipe_closed(pipe))) {
			rc = -EPIPE;
			நிறுத்து;
		}

		எனில் (pipe_empty(pipe)) {
			எனில் (IS_ENABLED(CONFIG_KERNEL_COHERENCE)) {
				/*
				 * Systems that enabled this option don't have
				 * their stacks in coherent memory. Given our
				 * pipe_buf_spec is stored on the stack, and
				 * readers may also have their destination
				 * buffer on their stack too, it is not worth
				 * supporting direct-to-readers copy with them.
				 * Simply wake up all pending readers instead.
				 */
				need_resched = z_sched_wake_all(&pipe->data, 0, வெற்று);
			} இல்லையெனில் எனில் (pipe->waiting != 0) {
				written += copy_to_pending_readers(pipe, &need_resched,
								   &data[written],
								   len - written);
				எனில் (written >= len) {
					rc = written;
					நிறுத்து;
				}
			}
		}

		added = ring_buf_put(&pipe->buf, &data[written], len - written);

#ifdef CONFIG_POLL
		எனில் (added != 0) {
			need_resched |= z_handle_obj_poll_events(&pipe->poll_events,
								 K_POLL_STATE_PIPE_DATA_AVAILABLE);
		}
#endif /* CONFIG_POLL */

		written += added;
		எனில் (likely(written == len)) {
			rc = written;
			நிறுத்து;
		}

		rc = wait_for(&pipe->space, pipe, &சாவி, end, &need_resched, வெற்று);
		எனில் (rc != 0) {
			எனில் (rc == -EAGAIN) {
				rc = written ? written : -EAGAIN;
			}
			நிறுத்து;
		}
	}
out:
	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_pipe, லிக, pipe, rc);
	எனில் (need_resched) {
		z_reschedule(&pipe->lock, சாவி);
	} இல்லையெனில் {
		k_spin_unlock(&pipe->lock, சாவி);
	}
	திரும்பு rc;
}

முழு z_impl_k_pipe_read(கட்டமைப்பு k_pipe *pipe, uint8_t *data, size_t len, k_timeout_t timeout)
{
	கட்டமைப்பு pipe_buf_spec buf = { data, len, 0 };
	முழு rc;
	k_timepoint_t end = sys_timepoint_calc(timeout);
	k_spinlock_சாவி_t சாவி = k_spin_lock(&pipe->lock);
	bool need_resched = false;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_pipe, பட, pipe, data, len, timeout);

	எனில் (unlikely(len > INT_MAX)) {
		rc = -EOVERFLOW;
		செல் out;
	}

	எனில் (unlikely(pipe_resetting(pipe))) {
		rc = -ECANCELED;
		செல் out;
	}

	க்கு (;;) {
		எனில் (pipe_full(pipe)) {
			/* One or more pending writers may exist. */
			need_resched = z_sched_wake_all(&pipe->space, 0, வெற்று);
		}

		buf.used += ring_buf_get(&pipe->buf, &data[buf.used], len - buf.used);
		எனில் (likely(buf.used == len)) {
			rc = buf.used;
			நிறுத்து;
		}

		எனில் (unlikely(pipe_closed(pipe))) {
			rc = buf.used ? buf.used : -EPIPE;
			நிறுத்து;
		}

		rc = wait_for(&pipe->data, pipe, &சாவி, end, &need_resched, &buf);
		எனில் (rc != 0) {
			எனில் (rc == -EAGAIN) {
				rc = buf.used ? buf.used : -EAGAIN;
			}
			நிறுத்து;
		}
	}
out:
	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_pipe, பட, pipe, rc);
	எனில் (need_resched) {
		z_reschedule(&pipe->lock, சாவி);
	} இல்லையெனில் {
		k_spin_unlock(&pipe->lock, சாவி);
	}
	திரும்பு rc;
}

வெறுமை z_impl_k_pipe_reset(கட்டமைப்பு k_pipe *pipe)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_pipe, reset, pipe);
	K_SPINLOCK(&pipe->lock) {
		ring_buf_reset(&pipe->buf);
		எனில் (likely(pipe->waiting != 0)) {
			pipe->flags |= PIPE_FLAG_RESET;
			z_sched_wake_all(&pipe->data, 0, வெற்று);
			z_sched_wake_all(&pipe->space, 0, வெற்று);
		}
	}
	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_pipe, reset, pipe);
}

வெறுமை z_impl_k_pipe_close(கட்டமைப்பு k_pipe *pipe)
{
	k_spinlock_சாவி_t சாவி;
	bool need_resched;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_pipe, close, pipe);

	சாவி = k_spin_lock(&pipe->lock);

	pipe->flags = 0;
	need_resched = z_sched_wake_all(&pipe->data, 0, வெற்று);
	need_resched |= z_sched_wake_all(&pipe->space, 0, வெற்று);

#ifdef CONFIG_POLL
	/*
	 * Closing is a broadcast: wake every registered poller. Later
	 * k_poll() calls see the hangup through is_condition_met().
	 */
	போது (z_handle_obj_poll_events(&pipe->poll_events,
					K_POLL_STATE_PIPE_DATA_AVAILABLE)) {
		need_resched = true;
	}
#endif /* CONFIG_POLL */

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_pipe, close, pipe);

	எனில் (need_resched) {
		z_reschedule(&pipe->lock, சாவி);
	} இல்லையெனில் {
		k_spin_unlock(&pipe->lock, சாவி);
	}
}

#ifdef CONFIG_USERSPACE
வெறுமை z_vrfy_k_pipe_init(கட்டமைப்பு k_pipe *pipe, uint8_t *buffer, size_t buffer_size)
{
	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(pipe, K_OBJ_PIPE));
	K_OOPS(K_SYSCALL_MEMORY_WRITE(buffer, buffer_size));

	z_impl_k_pipe_init(pipe, buffer, buffer_size);
}
#அந்தர்பாவ <zephyr/syscalls/k_pipe_init_mrsh.c>

முழு z_vrfy_k_pipe_read(கட்டமைப்பு k_pipe *pipe, uint8_t *data, size_t len, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(pipe, K_OBJ_PIPE));
	K_OOPS(K_SYSCALL_MEMORY_WRITE(data, len));

	திரும்பு z_impl_k_pipe_read(pipe, data, len, timeout);
}
#அந்தர்பாவ <zephyr/syscalls/k_pipe_read_mrsh.c>

முழு z_vrfy_k_pipe_write(கட்டமைப்பு k_pipe *pipe, மாறிலி uint8_t *data, size_t len, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(pipe, K_OBJ_PIPE));
	K_OOPS(K_SYSCALL_MEMORY_READ(data, len));

	திரும்பு z_impl_k_pipe_write(pipe, data, len, timeout);
}
#அந்தர்பாவ <zephyr/syscalls/k_pipe_write_mrsh.c>

வெறுமை z_vrfy_k_pipe_reset(கட்டமைப்பு k_pipe *pipe)
{
	K_OOPS(K_SYSCALL_OBJ(pipe, K_OBJ_PIPE));
	z_impl_k_pipe_reset(pipe);
}
#அந்தர்பாவ <zephyr/syscalls/k_pipe_reset_mrsh.c>

வெறுமை z_vrfy_k_pipe_close(கட்டமைப்பு k_pipe *pipe)
{
	K_OOPS(K_SYSCALL_OBJ(pipe, K_OBJ_PIPE));
	z_impl_k_pipe_close(pipe);
}
#அந்தர்பாவ <zephyr/syscalls/k_pipe_close_mrsh.c>
#endif /* CONFIG_USERSPACE */
