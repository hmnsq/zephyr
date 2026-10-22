/*
 * Copyright (c) 2024 Måns Ansgariusson <mansgariusson@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ଅନ୍ତର୍ଭାଵ <zephyr/sys/minmax.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/init.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/kernel.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/internal/syscall_handler.h>
#ଅନ୍ତର୍ଭାଵ <ksched.h>
#ଅନ୍ତର୍ଭାଵ <kthread.h>
#ଅନ୍ତର୍ଭାଵ <wait_q.h>
#ଅନ୍ତର୍ଭାଵ <scheduler.h>

#ifdef CONFIG_OBJ_CORE_PIPE
K_OBJ_TYPE_DEFINE(obj_type_pipe, k_pipe, K_OBJ_TYPE_PIPE_ID, ଖାଲି);
#endif /* CONFIG_OBJ_CORE_PIPE */

ସ୍ଥିର ଧାଡ଼ିଭିତରେ bool pipe_closed(ସଂରଚନା k_pipe *pipe)
{
	ଫେରନ୍ତୁ (pipe->flags & PIPE_FLAG_OPEN) == 0;
}

ସ୍ଥିର ଧାଡ଼ିଭିତରେ bool pipe_resetting(ସଂରଚନା k_pipe *pipe)
{
	ଫେରନ୍ତୁ (pipe->flags & PIPE_FLAG_RESET) != 0;
}

ସ୍ଥିର ଧାଡ଼ିଭିତରେ bool pipe_full(ସଂରଚନା k_pipe *pipe)
{
	ଫେରନ୍ତୁ ring_buf_space_get(&pipe->buf) == 0;
}

ସ୍ଥିର ଧାଡ଼ିଭିତରେ bool pipe_empty(ସଂରଚନା k_pipe *pipe)
{
	ଫେରନ୍ତୁ ring_buf_is_empty(&pipe->buf);
}

ସଂରଚନା pipe_buf_spec {
	uint8_t * ଅଚଳ data;
	ଅଚଳ size_t len;
	size_t used;
};

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା wait_for(_wait_q_t *waitq, ସଂରଚନା k_pipe *pipe, k_spinlock_ଚାବି_t *ଚାବି,
		    k_timepoint_t time_limit, bool *need_resched,
		    ସଂରଚନା pipe_buf_spec *buf_spec)
{
	k_timeout_t timeout = sys_timepoint_timeout(time_limit);
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;

	ଯଦି (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		ଫେରନ୍ତୁ -EAGAIN;
	}

	_current->base.swap_data = buf_spec;

	pipe->waiting++;
	*need_resched = false;
	ଯଦି (waitq == &pipe->space) {
		SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_pipe, ଲିଖ, pipe, timeout);
	} ଅନ୍ୟଥା {
		SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_pipe, ପଠ, pipe, timeout);
	}
	rc = z_pend_curr(&pipe->lock, *ଚାବି, waitq, timeout);
	*ଚାବି = k_spin_lock(&pipe->lock);
	pipe->waiting--;
	ଯଦି (unlikely(pipe_resetting(pipe))) {
		ଯଦି (pipe->waiting == 0) {
			pipe->flags &= ~PIPE_FLAG_RESET;
		}
		rc = -ECANCELED;
	}

	ଫେରନ୍ତୁ rc;
}

ଶୂନ୍ୟ z_impl_k_pipe_init(ସଂରଚନା k_pipe *pipe, uint8_t *buffer, size_t buffer_size)
{
	ring_buf_init(&pipe->buf, buffer_size, buffer);
	pipe->flags = PIPE_FLAG_OPEN;
	pipe->waiting = 0;

	pipe->lock = (ସଂରଚନା k_spinlock){};
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

ସ୍ଥିର size_t copy_to_pending_readers(ସଂରଚନା k_pipe *pipe, bool *need_resched,
				      ଅଚଳ uint8_t *data, size_t len)
{
	ସଂରଚନା k_thread *reader = ଖାଲି;
	ସଂରଚନା pipe_buf_spec *reader_buf;
	size_t copy_size, written = 0;

	/*
	 * Attempt a direct data copy to waiting readers if any.
	 * The copy has to be done under the scheduler lock to ensure all the
	 * needed data is copied to the target thread whose buffer spec lives
	 * on that thread's stack, and then the thread unpended only if it
	 * received all the data it wanted, without racing with a potential
	 * thread timeout/cancellation event.
	 */
	କର {
		LOCK_SCHED_SPINLOCK {
			reader = _priq_wait_best(&pipe->data.waitq);
			ଯଦି (reader == ଖାଲି) {
				K_SPINLOCK_BREAK;
			}

			reader_buf = reader->base.swap_data;
			copy_size = min(len - written,
					reader_buf->len - reader_buf->used);
			memcpy(&reader_buf->data[reader_buf->used],
			       &data[written], copy_size);
			written += copy_size;
			reader_buf->used += copy_size;

			ଯଦି (reader_buf->used < reader_buf->len) {
				/* This reader wants more: don't unpend. */
				reader = ଖାଲି;
			} ଅନ୍ୟଥା {
				/*
				 * This reader has received all the data
				 * it was waiting for. Set its return
				 * value, unpend, abort its timeout, and
				 * ready it, all under the scheduler lock
				 * so a racing timeout handler cannot
				 * observe a half-initialized wake-up.
				 */
				z_thread_return_ମୂଲ୍ୟ_set_with_data(reader, 0, ଖାଲି);
				unpend_thread_no_timeout(reader);
				(ଶୂନ୍ୟ)z_try_abort_thread_timeout(reader);
				z_sched_ready_locked(reader);
				*need_resched = true;
			}
		}
	} ଯେତେବେଳେ (reader != ଖାଲି && written < len);

	ଫେରନ୍ତୁ written;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା z_impl_k_pipe_write(ସଂରଚନା k_pipe *pipe, ଅଚଳ uint8_t *data, size_t len, k_timeout_t timeout)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
	size_t written = 0;
	size_t added;
	k_timepoint_t end = sys_timepoint_calc(timeout);
	k_spinlock_ଚାବି_t ଚାବି = k_spin_lock(&pipe->lock);
	bool need_resched = false;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_pipe, ଲିଖ, pipe, data, len, timeout);

	ଯଦି (unlikely(len > INT_MAX)) {
		rc = -EOVERFLOW;
		ଯାଅ out;
	}

	ଯଦି (unlikely(pipe_resetting(pipe))) {
		rc = -ECANCELED;
		ଯାଅ out;
	}

	ପାଇଁ (;;) {
		ଯଦି (unlikely(pipe_closed(pipe))) {
			rc = -EPIPE;
			ଭାଙ୍ଗ;
		}

		ଯଦି (pipe_empty(pipe)) {
			ଯଦି (IS_ENABLED(CONFIG_KERNEL_COHERENCE)) {
				/*
				 * Systems that enabled this option don't have
				 * their stacks in coherent memory. Given our
				 * pipe_buf_spec is stored on the stack, and
				 * readers may also have their destination
				 * buffer on their stack too, it is not worth
				 * supporting direct-to-readers copy with them.
				 * Simply wake up all pending readers instead.
				 */
				need_resched = z_sched_wake_all(&pipe->data, 0, ଖାଲି);
			} ଅନ୍ୟଥା ଯଦି (pipe->waiting != 0) {
				written += copy_to_pending_readers(pipe, &need_resched,
								   &data[written],
								   len - written);
				ଯଦି (written >= len) {
					rc = written;
					ଭାଙ୍ଗ;
				}
			}
		}

		added = ring_buf_put(&pipe->buf, &data[written], len - written);

#ifdef CONFIG_POLL
		ଯଦି (added != 0) {
			need_resched |= z_handle_obj_poll_events(&pipe->poll_events,
								 K_POLL_STATE_PIPE_DATA_AVAILABLE);
		}
#endif /* CONFIG_POLL */

		written += added;
		ଯଦି (likely(written == len)) {
			rc = written;
			ଭାଙ୍ଗ;
		}

		rc = wait_for(&pipe->space, pipe, &ଚାବି, end, &need_resched, ଖାଲି);
		ଯଦି (rc != 0) {
			ଯଦି (rc == -EAGAIN) {
				rc = written ? written : -EAGAIN;
			}
			ଭାଙ୍ଗ;
		}
	}
out:
	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_pipe, ଲିଖ, pipe, rc);
	ଯଦି (need_resched) {
		z_reschedule(&pipe->lock, ଚାବି);
	} ଅନ୍ୟଥା {
		k_spin_unlock(&pipe->lock, ଚାବି);
	}
	ଫେରନ୍ତୁ rc;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା z_impl_k_pipe_read(ସଂରଚନା k_pipe *pipe, uint8_t *data, size_t len, k_timeout_t timeout)
{
	ସଂରଚନା pipe_buf_spec buf = { data, len, 0 };
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
	k_timepoint_t end = sys_timepoint_calc(timeout);
	k_spinlock_ଚାବି_t ଚାବି = k_spin_lock(&pipe->lock);
	bool need_resched = false;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_pipe, ପଠ, pipe, data, len, timeout);

	ଯଦି (unlikely(len > INT_MAX)) {
		rc = -EOVERFLOW;
		ଯାଅ out;
	}

	ଯଦି (unlikely(pipe_resetting(pipe))) {
		rc = -ECANCELED;
		ଯାଅ out;
	}

	ପାଇଁ (;;) {
		ଯଦି (pipe_full(pipe)) {
			/* One or more pending writers may exist. */
			need_resched = z_sched_wake_all(&pipe->space, 0, ଖାଲି);
		}

		buf.used += ring_buf_get(&pipe->buf, &data[buf.used], len - buf.used);
		ଯଦି (likely(buf.used == len)) {
			rc = buf.used;
			ଭାଙ୍ଗ;
		}

		ଯଦି (unlikely(pipe_closed(pipe))) {
			rc = buf.used ? buf.used : -EPIPE;
			ଭାଙ୍ଗ;
		}

		rc = wait_for(&pipe->data, pipe, &ଚାବି, end, &need_resched, &buf);
		ଯଦି (rc != 0) {
			ଯଦି (rc == -EAGAIN) {
				rc = buf.used ? buf.used : -EAGAIN;
			}
			ଭାଙ୍ଗ;
		}
	}
out:
	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_pipe, ପଠ, pipe, rc);
	ଯଦି (need_resched) {
		z_reschedule(&pipe->lock, ଚାବି);
	} ଅନ୍ୟଥା {
		k_spin_unlock(&pipe->lock, ଚାବି);
	}
	ଫେରନ୍ତୁ rc;
}

ଶୂନ୍ୟ z_impl_k_pipe_reset(ସଂରଚନା k_pipe *pipe)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_pipe, reset, pipe);
	K_SPINLOCK(&pipe->lock) {
		ring_buf_reset(&pipe->buf);
		ଯଦି (likely(pipe->waiting != 0)) {
			pipe->flags |= PIPE_FLAG_RESET;
			z_sched_wake_all(&pipe->data, 0, ଖାଲି);
			z_sched_wake_all(&pipe->space, 0, ଖାଲି);
		}
	}
	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_pipe, reset, pipe);
}

ଶୂନ୍ୟ z_impl_k_pipe_close(ସଂରଚନା k_pipe *pipe)
{
	k_spinlock_ଚାବି_t ଚାବି;
	bool need_resched;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_pipe, close, pipe);

	ଚାବି = k_spin_lock(&pipe->lock);

	pipe->flags = 0;
	need_resched = z_sched_wake_all(&pipe->data, 0, ଖାଲି);
	need_resched |= z_sched_wake_all(&pipe->space, 0, ଖାଲି);

#ifdef CONFIG_POLL
	/*
	 * Closing is a broadcast: wake every registered poller. Later
	 * k_poll() calls see the hangup through is_condition_met().
	 */
	ଯେତେବେଳେ (z_handle_obj_poll_events(&pipe->poll_events,
					K_POLL_STATE_PIPE_DATA_AVAILABLE)) {
		need_resched = true;
	}
#endif /* CONFIG_POLL */

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_pipe, close, pipe);

	ଯଦି (need_resched) {
		z_reschedule(&pipe->lock, ଚାବି);
	} ଅନ୍ୟଥା {
		k_spin_unlock(&pipe->lock, ଚାବି);
	}
}

#ifdef CONFIG_USERSPACE
ଶୂନ୍ୟ z_vrfy_k_pipe_init(ସଂରଚନା k_pipe *pipe, uint8_t *buffer, size_t buffer_size)
{
	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(pipe, K_OBJ_PIPE));
	K_OOPS(K_SYSCALL_MEMORY_WRITE(buffer, buffer_size));

	z_impl_k_pipe_init(pipe, buffer, buffer_size);
}
#ଅନ୍ତର୍ଭାଵ <zephyr/syscalls/k_pipe_init_mrsh.c>

ପୂର୍ଣ୍ଣସଂଖ୍ୟା z_vrfy_k_pipe_read(ସଂରଚନା k_pipe *pipe, uint8_t *data, size_t len, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(pipe, K_OBJ_PIPE));
	K_OOPS(K_SYSCALL_MEMORY_WRITE(data, len));

	ଫେରନ୍ତୁ z_impl_k_pipe_read(pipe, data, len, timeout);
}
#ଅନ୍ତର୍ଭାଵ <zephyr/syscalls/k_pipe_read_mrsh.c>

ପୂର୍ଣ୍ଣସଂଖ୍ୟା z_vrfy_k_pipe_write(ସଂରଚନା k_pipe *pipe, ଅଚଳ uint8_t *data, size_t len, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(pipe, K_OBJ_PIPE));
	K_OOPS(K_SYSCALL_MEMORY_READ(data, len));

	ଫେରନ୍ତୁ z_impl_k_pipe_write(pipe, data, len, timeout);
}
#ଅନ୍ତର୍ଭାଵ <zephyr/syscalls/k_pipe_write_mrsh.c>

ଶୂନ୍ୟ z_vrfy_k_pipe_reset(ସଂରଚନା k_pipe *pipe)
{
	K_OOPS(K_SYSCALL_OBJ(pipe, K_OBJ_PIPE));
	z_impl_k_pipe_reset(pipe);
}
#ଅନ୍ତର୍ଭାଵ <zephyr/syscalls/k_pipe_reset_mrsh.c>

ଶୂନ୍ୟ z_vrfy_k_pipe_close(ସଂରଚନା k_pipe *pipe)
{
	K_OOPS(K_SYSCALL_OBJ(pipe, K_OBJ_PIPE));
	z_impl_k_pipe_close(pipe);
}
#ଅନ୍ତର୍ଭାଵ <zephyr/syscalls/k_pipe_close_mrsh.c>
#endif /* CONFIG_USERSPACE */
