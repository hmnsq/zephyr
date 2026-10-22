/*
 * Copyright (c) 2024 Måns Ansgariusson <mansgariusson@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#অন্তর্ভাব <zephyr/sys/minmax.h>
#অন্তর্ভাব <zephyr/init.h>
#অন্তর্ভাব <zephyr/kernel.h>
#অন্তর্ভাব <zephyr/internal/syscall_handler.h>
#অন্তর্ভাব <ksched.h>
#অন্তর্ভাব <kthread.h>
#অন্তর্ভাব <wait_q.h>
#অন্তর্ভাব <scheduler.h>

#ifdef CONFIG_OBJ_CORE_PIPE
K_OBJ_TYPE_DEFINE(obj_type_pipe, k_pipe, K_OBJ_TYPE_PIPE_ID, রিক্ত);
#endif /* CONFIG_OBJ_CORE_PIPE */

স্থির অন্তঃস্থ bool pipe_closed(সংরচনা k_pipe *pipe)
{
	প্রত্যাগম (pipe->flags & PIPE_FLAG_OPEN) == 0;
}

স্থির অন্তঃস্থ bool pipe_resetting(সংরচনা k_pipe *pipe)
{
	প্রত্যাগম (pipe->flags & PIPE_FLAG_RESET) != 0;
}

স্থির অন্তঃস্থ bool pipe_full(সংরচনা k_pipe *pipe)
{
	প্রত্যাগম ring_buf_space_get(&pipe->buf) == 0;
}

স্থির অন্তঃস্থ bool pipe_empty(সংরচনা k_pipe *pipe)
{
	প্রত্যাগম ring_buf_is_empty(&pipe->buf);
}

সংরচনা pipe_buf_spec {
	uint8_t * ধ্রুব data;
	ধ্রুব size_t len;
	size_t used;
};

স্থির পূর্ণাঙ্ক wait_for(_wait_q_t *waitq, সংরচনা k_pipe *pipe, k_spinlock_চাবি_t *চাবি,
		    k_timepoint_t time_limit, bool *need_resched,
		    সংরচনা pipe_buf_spec *buf_spec)
{
	k_timeout_t timeout = sys_timepoint_timeout(time_limit);
	পূর্ণাঙ্ক rc;

	যদি (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		প্রত্যাগম -EAGAIN;
	}

	_current->base.swap_data = buf_spec;

	pipe->waiting++;
	*need_resched = false;
	যদি (waitq == &pipe->space) {
		SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_pipe, লিখ, pipe, timeout);
	} অন্যথা {
		SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_pipe, পঠ, pipe, timeout);
	}
	rc = z_pend_curr(&pipe->lock, *চাবি, waitq, timeout);
	*চাবি = k_spin_lock(&pipe->lock);
	pipe->waiting--;
	যদি (unlikely(pipe_resetting(pipe))) {
		যদি (pipe->waiting == 0) {
			pipe->flags &= ~PIPE_FLAG_RESET;
		}
		rc = -ECANCELED;
	}

	প্রত্যাগম rc;
}

শূন্য z_impl_k_pipe_init(সংরচনা k_pipe *pipe, uint8_t *buffer, size_t buffer_size)
{
	ring_buf_init(&pipe->buf, buffer_size, buffer);
	pipe->flags = PIPE_FLAG_OPEN;
	pipe->waiting = 0;

	pipe->lock = (সংরচনা k_spinlock){};
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

স্থির size_t copy_to_pending_readers(সংরচনা k_pipe *pipe, bool *need_resched,
				      ধ্রুব uint8_t *data, size_t len)
{
	সংরচনা k_thread *reader = রিক্ত;
	সংরচনা pipe_buf_spec *reader_buf;
	size_t copy_size, written = 0;

	/*
	 * Attempt a direct data copy to waiting readers if any.
	 * The copy has to be done under the scheduler lock to ensure all the
	 * needed data is copied to the target thread whose buffer spec lives
	 * on that thread's stack, and then the thread unpended only if it
	 * received all the data it wanted, without racing with a potential
	 * thread timeout/cancellation event.
	 */
	কুরু {
		LOCK_SCHED_SPINLOCK {
			reader = _priq_wait_best(&pipe->data.waitq);
			যদি (reader == রিক্ত) {
				K_SPINLOCK_BREAK;
			}

			reader_buf = reader->base.swap_data;
			copy_size = min(len - written,
					reader_buf->len - reader_buf->used);
			memcpy(&reader_buf->data[reader_buf->used],
			       &data[written], copy_size);
			written += copy_size;
			reader_buf->used += copy_size;

			যদি (reader_buf->used < reader_buf->len) {
				/* This reader wants more: don't unpend. */
				reader = রিক্ত;
			} অন্যথা {
				/*
				 * This reader has received all the data
				 * it was waiting for. Set its return
				 * value, unpend, abort its timeout, and
				 * ready it, all under the scheduler lock
				 * so a racing timeout handler cannot
				 * observe a half-initialized wake-up.
				 */
				z_thread_return_মান_set_with_data(reader, 0, রিক্ত);
				unpend_thread_no_timeout(reader);
				(শূন্য)z_try_abort_thread_timeout(reader);
				z_sched_ready_locked(reader);
				*need_resched = true;
			}
		}
	} যাবত্ (reader != রিক্ত && written < len);

	প্রত্যাগম written;
}

পূর্ণাঙ্ক z_impl_k_pipe_write(সংরচনা k_pipe *pipe, ধ্রুব uint8_t *data, size_t len, k_timeout_t timeout)
{
	পূর্ণাঙ্ক rc;
	size_t written = 0;
	size_t added;
	k_timepoint_t end = sys_timepoint_calc(timeout);
	k_spinlock_চাবি_t চাবি = k_spin_lock(&pipe->lock);
	bool need_resched = false;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_pipe, লিখ, pipe, data, len, timeout);

	যদি (unlikely(len > INT_MAX)) {
		rc = -EOVERFLOW;
		গচ্ছ out;
	}

	যদি (unlikely(pipe_resetting(pipe))) {
		rc = -ECANCELED;
		গচ্ছ out;
	}

	প্রতি (;;) {
		যদি (unlikely(pipe_closed(pipe))) {
			rc = -EPIPE;
			ভঙ্গ;
		}

		যদি (pipe_empty(pipe)) {
			যদি (IS_ENABLED(CONFIG_KERNEL_COHERENCE)) {
				/*
				 * Systems that enabled this option don't have
				 * their stacks in coherent memory. Given our
				 * pipe_buf_spec is stored on the stack, and
				 * readers may also have their destination
				 * buffer on their stack too, it is not worth
				 * supporting direct-to-readers copy with them.
				 * Simply wake up all pending readers instead.
				 */
				need_resched = z_sched_wake_all(&pipe->data, 0, রিক্ত);
			} অন্যথা যদি (pipe->waiting != 0) {
				written += copy_to_pending_readers(pipe, &need_resched,
								   &data[written],
								   len - written);
				যদি (written >= len) {
					rc = written;
					ভঙ্গ;
				}
			}
		}

		added = ring_buf_put(&pipe->buf, &data[written], len - written);

#ifdef CONFIG_POLL
		যদি (added != 0) {
			need_resched |= z_handle_obj_poll_events(&pipe->poll_events,
								 K_POLL_STATE_PIPE_DATA_AVAILABLE);
		}
#endif /* CONFIG_POLL */

		written += added;
		যদি (likely(written == len)) {
			rc = written;
			ভঙ্গ;
		}

		rc = wait_for(&pipe->space, pipe, &চাবি, end, &need_resched, রিক্ত);
		যদি (rc != 0) {
			যদি (rc == -EAGAIN) {
				rc = written ? written : -EAGAIN;
			}
			ভঙ্গ;
		}
	}
out:
	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_pipe, লিখ, pipe, rc);
	যদি (need_resched) {
		z_reschedule(&pipe->lock, চাবি);
	} অন্যথা {
		k_spin_unlock(&pipe->lock, চাবি);
	}
	প্রত্যাগম rc;
}

পূর্ণাঙ্ক z_impl_k_pipe_read(সংরচনা k_pipe *pipe, uint8_t *data, size_t len, k_timeout_t timeout)
{
	সংরচনা pipe_buf_spec buf = { data, len, 0 };
	পূর্ণাঙ্ক rc;
	k_timepoint_t end = sys_timepoint_calc(timeout);
	k_spinlock_চাবি_t চাবি = k_spin_lock(&pipe->lock);
	bool need_resched = false;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_pipe, পঠ, pipe, data, len, timeout);

	যদি (unlikely(len > INT_MAX)) {
		rc = -EOVERFLOW;
		গচ্ছ out;
	}

	যদি (unlikely(pipe_resetting(pipe))) {
		rc = -ECANCELED;
		গচ্ছ out;
	}

	প্রতি (;;) {
		যদি (pipe_full(pipe)) {
			/* One or more pending writers may exist. */
			need_resched = z_sched_wake_all(&pipe->space, 0, রিক্ত);
		}

		buf.used += ring_buf_get(&pipe->buf, &data[buf.used], len - buf.used);
		যদি (likely(buf.used == len)) {
			rc = buf.used;
			ভঙ্গ;
		}

		যদি (unlikely(pipe_closed(pipe))) {
			rc = buf.used ? buf.used : -EPIPE;
			ভঙ্গ;
		}

		rc = wait_for(&pipe->data, pipe, &চাবি, end, &need_resched, &buf);
		যদি (rc != 0) {
			যদি (rc == -EAGAIN) {
				rc = buf.used ? buf.used : -EAGAIN;
			}
			ভঙ্গ;
		}
	}
out:
	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_pipe, পঠ, pipe, rc);
	যদি (need_resched) {
		z_reschedule(&pipe->lock, চাবি);
	} অন্যথা {
		k_spin_unlock(&pipe->lock, চাবি);
	}
	প্রত্যাগম rc;
}

শূন্য z_impl_k_pipe_reset(সংরচনা k_pipe *pipe)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_pipe, reset, pipe);
	K_SPINLOCK(&pipe->lock) {
		ring_buf_reset(&pipe->buf);
		যদি (likely(pipe->waiting != 0)) {
			pipe->flags |= PIPE_FLAG_RESET;
			z_sched_wake_all(&pipe->data, 0, রিক্ত);
			z_sched_wake_all(&pipe->space, 0, রিক্ত);
		}
	}
	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_pipe, reset, pipe);
}

শূন্য z_impl_k_pipe_close(সংরচনা k_pipe *pipe)
{
	k_spinlock_চাবি_t চাবি;
	bool need_resched;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_pipe, close, pipe);

	চাবি = k_spin_lock(&pipe->lock);

	pipe->flags = 0;
	need_resched = z_sched_wake_all(&pipe->data, 0, রিক্ত);
	need_resched |= z_sched_wake_all(&pipe->space, 0, রিক্ত);

#ifdef CONFIG_POLL
	/*
	 * Closing is a broadcast: wake every registered poller. Later
	 * k_poll() calls see the hangup through is_condition_met().
	 */
	যাবত্ (z_handle_obj_poll_events(&pipe->poll_events,
					K_POLL_STATE_PIPE_DATA_AVAILABLE)) {
		need_resched = true;
	}
#endif /* CONFIG_POLL */

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_pipe, close, pipe);

	যদি (need_resched) {
		z_reschedule(&pipe->lock, চাবি);
	} অন্যথা {
		k_spin_unlock(&pipe->lock, চাবি);
	}
}

#ifdef CONFIG_USERSPACE
শূন্য z_vrfy_k_pipe_init(সংরচনা k_pipe *pipe, uint8_t *buffer, size_t buffer_size)
{
	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(pipe, K_OBJ_PIPE));
	K_OOPS(K_SYSCALL_MEMORY_WRITE(buffer, buffer_size));

	z_impl_k_pipe_init(pipe, buffer, buffer_size);
}
#অন্তর্ভাব <zephyr/syscalls/k_pipe_init_mrsh.c>

পূর্ণাঙ্ক z_vrfy_k_pipe_read(সংরচনা k_pipe *pipe, uint8_t *data, size_t len, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(pipe, K_OBJ_PIPE));
	K_OOPS(K_SYSCALL_MEMORY_WRITE(data, len));

	প্রত্যাগম z_impl_k_pipe_read(pipe, data, len, timeout);
}
#অন্তর্ভাব <zephyr/syscalls/k_pipe_read_mrsh.c>

পূর্ণাঙ্ক z_vrfy_k_pipe_write(সংরচনা k_pipe *pipe, ধ্রুব uint8_t *data, size_t len, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(pipe, K_OBJ_PIPE));
	K_OOPS(K_SYSCALL_MEMORY_READ(data, len));

	প্রত্যাগম z_impl_k_pipe_write(pipe, data, len, timeout);
}
#অন্তর্ভাব <zephyr/syscalls/k_pipe_write_mrsh.c>

শূন্য z_vrfy_k_pipe_reset(সংরচনা k_pipe *pipe)
{
	K_OOPS(K_SYSCALL_OBJ(pipe, K_OBJ_PIPE));
	z_impl_k_pipe_reset(pipe);
}
#অন্তর্ভাব <zephyr/syscalls/k_pipe_reset_mrsh.c>

শূন্য z_vrfy_k_pipe_close(সংরচনা k_pipe *pipe)
{
	K_OOPS(K_SYSCALL_OBJ(pipe, K_OBJ_PIPE));
	z_impl_k_pipe_close(pipe);
}
#অন্তর্ভাব <zephyr/syscalls/k_pipe_close_mrsh.c>
#endif /* CONFIG_USERSPACE */
