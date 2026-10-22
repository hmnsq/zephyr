/*
 * Copyright (c) 2024 Måns Ansgariusson <mansgariusson@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#અન્તર્ભાવ <zephyr/sys/minmax.h>
#અન્તર્ભાવ <zephyr/init.h>
#અન્તર્ભાવ <zephyr/kernel.h>
#અન્તર્ભાવ <zephyr/internal/syscall_handler.h>
#અન્તર્ભાવ <ksched.h>
#અન્તર્ભાવ <kthread.h>
#અન્તર્ભાવ <wait_q.h>
#અન્તર્ભાવ <scheduler.h>

#ifdef CONFIG_OBJ_CORE_PIPE
K_OBJ_TYPE_DEFINE(obj_type_pipe, k_pipe, K_OBJ_TYPE_PIPE_ID, ખાલી);
#endif /* CONFIG_OBJ_CORE_PIPE */

સ્થિર અંતઃરેખ bool pipe_closed(સંરચના k_pipe *pipe)
{
	પાછા (pipe->flags & PIPE_FLAG_OPEN) == 0;
}

સ્થિર અંતઃરેખ bool pipe_resetting(સંરચના k_pipe *pipe)
{
	પાછા (pipe->flags & PIPE_FLAG_RESET) != 0;
}

સ્થિર અંતઃરેખ bool pipe_full(સંરચના k_pipe *pipe)
{
	પાછા ring_buf_space_get(&pipe->buf) == 0;
}

સ્થિર અંતઃરેખ bool pipe_empty(સંરચના k_pipe *pipe)
{
	પાછા ring_buf_is_empty(&pipe->buf);
}

સંરચના pipe_buf_spec {
	uint8_t * અચલ data;
	અચલ size_t len;
	size_t used;
};

સ્થિર પૂર્ણાંક wait_for(_wait_q_t *waitq, સંરચના k_pipe *pipe, k_spinlock_ચાવી_t *ચાવી,
		    k_timepoint_t time_limit, bool *need_resched,
		    સંરચના pipe_buf_spec *buf_spec)
{
	k_timeout_t timeout = sys_timepoint_timeout(time_limit);
	પૂર્ણાંક rc;

	જો (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		પાછા -EAGAIN;
	}

	_current->base.swap_data = buf_spec;

	pipe->waiting++;
	*need_resched = false;
	જો (waitq == &pipe->space) {
		SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_pipe, લિખ, pipe, timeout);
	} નહીંતર {
		SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_pipe, પઠ, pipe, timeout);
	}
	rc = z_pend_curr(&pipe->lock, *ચાવી, waitq, timeout);
	*ચાવી = k_spin_lock(&pipe->lock);
	pipe->waiting--;
	જો (unlikely(pipe_resetting(pipe))) {
		જો (pipe->waiting == 0) {
			pipe->flags &= ~PIPE_FLAG_RESET;
		}
		rc = -ECANCELED;
	}

	પાછા rc;
}

શૂન્ય z_impl_k_pipe_init(સંરચના k_pipe *pipe, uint8_t *buffer, size_t buffer_size)
{
	ring_buf_init(&pipe->buf, buffer_size, buffer);
	pipe->flags = PIPE_FLAG_OPEN;
	pipe->waiting = 0;

	pipe->lock = (સંરચના k_spinlock){};
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

સ્થિર size_t copy_to_pending_readers(સંરચના k_pipe *pipe, bool *need_resched,
				      અચલ uint8_t *data, size_t len)
{
	સંરચના k_thread *reader = ખાલી;
	સંરચના pipe_buf_spec *reader_buf;
	size_t copy_size, written = 0;

	/*
	 * Attempt a direct data copy to waiting readers if any.
	 * The copy has to be done under the scheduler lock to ensure all the
	 * needed data is copied to the target thread whose buffer spec lives
	 * on that thread's stack, and then the thread unpended only if it
	 * received all the data it wanted, without racing with a potential
	 * thread timeout/cancellation event.
	 */
	કરો {
		LOCK_SCHED_SPINLOCK {
			reader = _priq_wait_best(&pipe->data.waitq);
			જો (reader == ખાલી) {
				K_SPINLOCK_BREAK;
			}

			reader_buf = reader->base.swap_data;
			copy_size = min(len - written,
					reader_buf->len - reader_buf->used);
			memcpy(&reader_buf->data[reader_buf->used],
			       &data[written], copy_size);
			written += copy_size;
			reader_buf->used += copy_size;

			જો (reader_buf->used < reader_buf->len) {
				/* This reader wants more: don't unpend. */
				reader = ખાલી;
			} નહીંતર {
				/*
				 * This reader has received all the data
				 * it was waiting for. Set its return
				 * value, unpend, abort its timeout, and
				 * ready it, all under the scheduler lock
				 * so a racing timeout handler cannot
				 * observe a half-initialized wake-up.
				 */
				z_thread_return_કિંમત_set_with_data(reader, 0, ખાલી);
				unpend_thread_no_timeout(reader);
				(શૂન્ય)z_try_abort_thread_timeout(reader);
				z_sched_ready_locked(reader);
				*need_resched = true;
			}
		}
	} જ્યારે (reader != ખાલી && written < len);

	પાછા written;
}

પૂર્ણાંક z_impl_k_pipe_write(સંરચના k_pipe *pipe, અચલ uint8_t *data, size_t len, k_timeout_t timeout)
{
	પૂર્ણાંક rc;
	size_t written = 0;
	size_t added;
	k_timepoint_t end = sys_timepoint_calc(timeout);
	k_spinlock_ચાવી_t ચાવી = k_spin_lock(&pipe->lock);
	bool need_resched = false;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_pipe, લિખ, pipe, data, len, timeout);

	જો (unlikely(len > INT_MAX)) {
		rc = -EOVERFLOW;
		જાઓ out;
	}

	જો (unlikely(pipe_resetting(pipe))) {
		rc = -ECANCELED;
		જાઓ out;
	}

	માટે (;;) {
		જો (unlikely(pipe_closed(pipe))) {
			rc = -EPIPE;
			તોડો;
		}

		જો (pipe_empty(pipe)) {
			જો (IS_ENABLED(CONFIG_KERNEL_COHERENCE)) {
				/*
				 * Systems that enabled this option don't have
				 * their stacks in coherent memory. Given our
				 * pipe_buf_spec is stored on the stack, and
				 * readers may also have their destination
				 * buffer on their stack too, it is not worth
				 * supporting direct-to-readers copy with them.
				 * Simply wake up all pending readers instead.
				 */
				need_resched = z_sched_wake_all(&pipe->data, 0, ખાલી);
			} નહીંતર જો (pipe->waiting != 0) {
				written += copy_to_pending_readers(pipe, &need_resched,
								   &data[written],
								   len - written);
				જો (written >= len) {
					rc = written;
					તોડો;
				}
			}
		}

		added = ring_buf_put(&pipe->buf, &data[written], len - written);

#ifdef CONFIG_POLL
		જો (added != 0) {
			need_resched |= z_handle_obj_poll_events(&pipe->poll_events,
								 K_POLL_STATE_PIPE_DATA_AVAILABLE);
		}
#endif /* CONFIG_POLL */

		written += added;
		જો (likely(written == len)) {
			rc = written;
			તોડો;
		}

		rc = wait_for(&pipe->space, pipe, &ચાવી, end, &need_resched, ખાલી);
		જો (rc != 0) {
			જો (rc == -EAGAIN) {
				rc = written ? written : -EAGAIN;
			}
			તોડો;
		}
	}
out:
	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_pipe, લિખ, pipe, rc);
	જો (need_resched) {
		z_reschedule(&pipe->lock, ચાવી);
	} નહીંતર {
		k_spin_unlock(&pipe->lock, ચાવી);
	}
	પાછા rc;
}

પૂર્ણાંક z_impl_k_pipe_read(સંરચના k_pipe *pipe, uint8_t *data, size_t len, k_timeout_t timeout)
{
	સંરચના pipe_buf_spec buf = { data, len, 0 };
	પૂર્ણાંક rc;
	k_timepoint_t end = sys_timepoint_calc(timeout);
	k_spinlock_ચાવી_t ચાવી = k_spin_lock(&pipe->lock);
	bool need_resched = false;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_pipe, પઠ, pipe, data, len, timeout);

	જો (unlikely(len > INT_MAX)) {
		rc = -EOVERFLOW;
		જાઓ out;
	}

	જો (unlikely(pipe_resetting(pipe))) {
		rc = -ECANCELED;
		જાઓ out;
	}

	માટે (;;) {
		જો (pipe_full(pipe)) {
			/* One or more pending writers may exist. */
			need_resched = z_sched_wake_all(&pipe->space, 0, ખાલી);
		}

		buf.used += ring_buf_get(&pipe->buf, &data[buf.used], len - buf.used);
		જો (likely(buf.used == len)) {
			rc = buf.used;
			તોડો;
		}

		જો (unlikely(pipe_closed(pipe))) {
			rc = buf.used ? buf.used : -EPIPE;
			તોડો;
		}

		rc = wait_for(&pipe->data, pipe, &ચાવી, end, &need_resched, &buf);
		જો (rc != 0) {
			જો (rc == -EAGAIN) {
				rc = buf.used ? buf.used : -EAGAIN;
			}
			તોડો;
		}
	}
out:
	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_pipe, પઠ, pipe, rc);
	જો (need_resched) {
		z_reschedule(&pipe->lock, ચાવી);
	} નહીંતર {
		k_spin_unlock(&pipe->lock, ચાવી);
	}
	પાછા rc;
}

શૂન્ય z_impl_k_pipe_reset(સંરચના k_pipe *pipe)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_pipe, reset, pipe);
	K_SPINLOCK(&pipe->lock) {
		ring_buf_reset(&pipe->buf);
		જો (likely(pipe->waiting != 0)) {
			pipe->flags |= PIPE_FLAG_RESET;
			z_sched_wake_all(&pipe->data, 0, ખાલી);
			z_sched_wake_all(&pipe->space, 0, ખાલી);
		}
	}
	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_pipe, reset, pipe);
}

શૂન્ય z_impl_k_pipe_close(સંરચના k_pipe *pipe)
{
	k_spinlock_ચાવી_t ચાવી;
	bool need_resched;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_pipe, close, pipe);

	ચાવી = k_spin_lock(&pipe->lock);

	pipe->flags = 0;
	need_resched = z_sched_wake_all(&pipe->data, 0, ખાલી);
	need_resched |= z_sched_wake_all(&pipe->space, 0, ખાલી);

#ifdef CONFIG_POLL
	/*
	 * Closing is a broadcast: wake every registered poller. Later
	 * k_poll() calls see the hangup through is_condition_met().
	 */
	જ્યારે (z_handle_obj_poll_events(&pipe->poll_events,
					K_POLL_STATE_PIPE_DATA_AVAILABLE)) {
		need_resched = true;
	}
#endif /* CONFIG_POLL */

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_pipe, close, pipe);

	જો (need_resched) {
		z_reschedule(&pipe->lock, ચાવી);
	} નહીંતર {
		k_spin_unlock(&pipe->lock, ચાવી);
	}
}

#ifdef CONFIG_USERSPACE
શૂન્ય z_vrfy_k_pipe_init(સંરચના k_pipe *pipe, uint8_t *buffer, size_t buffer_size)
{
	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(pipe, K_OBJ_PIPE));
	K_OOPS(K_SYSCALL_MEMORY_WRITE(buffer, buffer_size));

	z_impl_k_pipe_init(pipe, buffer, buffer_size);
}
#અન્તર્ભાવ <zephyr/syscalls/k_pipe_init_mrsh.c>

પૂર્ણાંક z_vrfy_k_pipe_read(સંરચના k_pipe *pipe, uint8_t *data, size_t len, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(pipe, K_OBJ_PIPE));
	K_OOPS(K_SYSCALL_MEMORY_WRITE(data, len));

	પાછા z_impl_k_pipe_read(pipe, data, len, timeout);
}
#અન્તર્ભાવ <zephyr/syscalls/k_pipe_read_mrsh.c>

પૂર્ણાંક z_vrfy_k_pipe_write(સંરચના k_pipe *pipe, અચલ uint8_t *data, size_t len, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(pipe, K_OBJ_PIPE));
	K_OOPS(K_SYSCALL_MEMORY_READ(data, len));

	પાછા z_impl_k_pipe_write(pipe, data, len, timeout);
}
#અન્તર્ભાવ <zephyr/syscalls/k_pipe_write_mrsh.c>

શૂન્ય z_vrfy_k_pipe_reset(સંરચના k_pipe *pipe)
{
	K_OOPS(K_SYSCALL_OBJ(pipe, K_OBJ_PIPE));
	z_impl_k_pipe_reset(pipe);
}
#અન્તર્ભાવ <zephyr/syscalls/k_pipe_reset_mrsh.c>

શૂન્ય z_vrfy_k_pipe_close(સંરચના k_pipe *pipe)
{
	K_OOPS(K_SYSCALL_OBJ(pipe, K_OBJ_PIPE));
	z_impl_k_pipe_close(pipe);
}
#અન્તર્ભાવ <zephyr/syscalls/k_pipe_close_mrsh.c>
#endif /* CONFIG_USERSPACE */
