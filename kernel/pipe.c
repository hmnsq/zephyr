/*
 * Copyright (c) 2024 Måns Ansgariusson <mansgariusson@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#شامل <zephyr/sys/minmax.h>
#شامل <zephyr/init.h>
#شامل <zephyr/kernel.h>
#شامل <zephyr/internal/syscall_handler.h>
#شامل <ksched.h>
#شامل <kthread.h>
#شامل <wait_q.h>
#شامل <scheduler.h>

#ifdef CONFIG_OBJ_CORE_PIPE
K_OBJ_TYPE_DEFINE(obj_type_pipe, k_pipe, K_OBJ_TYPE_PIPE_ID, پوچ);
#endif /* CONFIG_OBJ_CORE_PIPE */

ایستا درونخطی bool pipe_closed(ساختار k_pipe *pipe)
{
	بازگشت (pipe->flags & PIPE_FLAG_OPEN) == 0;
}

ایستا درونخطی bool pipe_resetting(ساختار k_pipe *pipe)
{
	بازگشت (pipe->flags & PIPE_FLAG_RESET) != 0;
}

ایستا درونخطی bool pipe_full(ساختار k_pipe *pipe)
{
	بازگشت ring_buf_space_get(&pipe->buf) == 0;
}

ایستا درونخطی bool pipe_empty(ساختار k_pipe *pipe)
{
	بازگشت ring_buf_is_empty(&pipe->buf);
}

ساختار pipe_buf_spec {
	uint8_t * ثابت data;
	ثابت size_t len;
	size_t used;
};

ایستا صحیح wait_for(_wait_q_t *waitq, ساختار k_pipe *pipe, k_spinlock_کلید_t *کلید,
		    k_timepoint_t time_limit, bool *need_resched,
		    ساختار pipe_buf_spec *buf_spec)
{
	k_timeout_t timeout = sys_timepoint_timeout(time_limit);
	صحیح rc;

	اگر (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		بازگشت -EAGAIN;
	}

	_current->base.swap_data = buf_spec;

	pipe->waiting++;
	*need_resched = false;
	اگر (waitq == &pipe->space) {
		SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_pipe, اكتب, pipe, timeout);
	} وگرنه {
		SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_pipe, اقرأ, pipe, timeout);
	}
	rc = z_pend_curr(&pipe->lock, *کلید, waitq, timeout);
	*کلید = k_spin_lock(&pipe->lock);
	pipe->waiting--;
	اگر (unlikely(pipe_resetting(pipe))) {
		اگر (pipe->waiting == 0) {
			pipe->flags &= ~PIPE_FLAG_RESET;
		}
		rc = -ECANCELED;
	}

	بازگشت rc;
}

تهی z_impl_k_pipe_init(ساختار k_pipe *pipe, uint8_t *buffer, size_t buffer_size)
{
	ring_buf_init(&pipe->buf, buffer_size, buffer);
	pipe->flags = PIPE_FLAG_OPEN;
	pipe->waiting = 0;

	pipe->lock = (ساختار k_spinlock){};
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

ایستا size_t copy_to_pending_readers(ساختار k_pipe *pipe, bool *need_resched,
				      ثابت uint8_t *data, size_t len)
{
	ساختار k_thread *reader = پوچ;
	ساختار pipe_buf_spec *reader_buf;
	size_t copy_size, written = 0;

	/*
	 * Attempt a direct data copy to waiting readers if any.
	 * The copy has to be done under the scheduler lock to ensure all the
	 * needed data is copied to the target thread whose buffer spec lives
	 * on that thread's stack, and then the thread unpended only if it
	 * received all the data it wanted, without racing with a potential
	 * thread timeout/cancellation event.
	 */
	انجامبده {
		LOCK_SCHED_SPINLOCK {
			reader = _priq_wait_best(&pipe->data.waitq);
			اگر (reader == پوچ) {
				K_SPINLOCK_BREAK;
			}

			reader_buf = reader->base.swap_data;
			copy_size = min(len - written,
					reader_buf->len - reader_buf->used);
			memcpy(&reader_buf->data[reader_buf->used],
			       &data[written], copy_size);
			written += copy_size;
			reader_buf->used += copy_size;

			اگر (reader_buf->used < reader_buf->len) {
				/* This reader wants more: don't unpend. */
				reader = پوچ;
			} وگرنه {
				/*
				 * This reader has received all the data
				 * it was waiting for. Set its return
				 * value, unpend, abort its timeout, and
				 * ready it, all under the scheduler lock
				 * so a racing timeout handler cannot
				 * observe a half-initialized wake-up.
				 */
				z_thread_return_مقدار_set_with_data(reader, 0, پوچ);
				unpend_thread_no_timeout(reader);
				(تهی)z_try_abort_thread_timeout(reader);
				z_sched_ready_locked(reader);
				*need_resched = true;
			}
		}
	} وقتی (reader != پوچ && written < len);

	بازگشت written;
}

صحیح z_impl_k_pipe_write(ساختار k_pipe *pipe, ثابت uint8_t *data, size_t len, k_timeout_t timeout)
{
	صحیح rc;
	size_t written = 0;
	size_t added;
	k_timepoint_t end = sys_timepoint_calc(timeout);
	k_spinlock_کلید_t کلید = k_spin_lock(&pipe->lock);
	bool need_resched = false;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_pipe, اكتب, pipe, data, len, timeout);

	اگر (unlikely(len > INT_MAX)) {
		rc = -EOVERFLOW;
		برو out;
	}

	اگر (unlikely(pipe_resetting(pipe))) {
		rc = -ECANCELED;
		برو out;
	}

	برای (;;) {
		اگر (unlikely(pipe_closed(pipe))) {
			rc = -EPIPE;
			بشکن;
		}

		اگر (pipe_empty(pipe)) {
			اگر (IS_ENABLED(CONFIG_KERNEL_COHERENCE)) {
				/*
				 * Systems that enabled this option don't have
				 * their stacks in coherent memory. Given our
				 * pipe_buf_spec is stored on the stack, and
				 * readers may also have their destination
				 * buffer on their stack too, it is not worth
				 * supporting direct-to-readers copy with them.
				 * Simply wake up all pending readers instead.
				 */
				need_resched = z_sched_wake_all(&pipe->data, 0, پوچ);
			} وگرنه اگر (pipe->waiting != 0) {
				written += copy_to_pending_readers(pipe, &need_resched,
								   &data[written],
								   len - written);
				اگر (written >= len) {
					rc = written;
					بشکن;
				}
			}
		}

		added = ring_buf_put(&pipe->buf, &data[written], len - written);

#ifdef CONFIG_POLL
		اگر (added != 0) {
			need_resched |= z_handle_obj_poll_events(&pipe->poll_events,
								 K_POLL_STATE_PIPE_DATA_AVAILABLE);
		}
#endif /* CONFIG_POLL */

		written += added;
		اگر (likely(written == len)) {
			rc = written;
			بشکن;
		}

		rc = wait_for(&pipe->space, pipe, &کلید, end, &need_resched, پوچ);
		اگر (rc != 0) {
			اگر (rc == -EAGAIN) {
				rc = written ? written : -EAGAIN;
			}
			بشکن;
		}
	}
out:
	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_pipe, اكتب, pipe, rc);
	اگر (need_resched) {
		z_reschedule(&pipe->lock, کلید);
	} وگرنه {
		k_spin_unlock(&pipe->lock, کلید);
	}
	بازگشت rc;
}

صحیح z_impl_k_pipe_read(ساختار k_pipe *pipe, uint8_t *data, size_t len, k_timeout_t timeout)
{
	ساختار pipe_buf_spec buf = { data, len, 0 };
	صحیح rc;
	k_timepoint_t end = sys_timepoint_calc(timeout);
	k_spinlock_کلید_t کلید = k_spin_lock(&pipe->lock);
	bool need_resched = false;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_pipe, اقرأ, pipe, data, len, timeout);

	اگر (unlikely(len > INT_MAX)) {
		rc = -EOVERFLOW;
		برو out;
	}

	اگر (unlikely(pipe_resetting(pipe))) {
		rc = -ECANCELED;
		برو out;
	}

	برای (;;) {
		اگر (pipe_full(pipe)) {
			/* One or more pending writers may exist. */
			need_resched = z_sched_wake_all(&pipe->space, 0, پوچ);
		}

		buf.used += ring_buf_get(&pipe->buf, &data[buf.used], len - buf.used);
		اگر (likely(buf.used == len)) {
			rc = buf.used;
			بشکن;
		}

		اگر (unlikely(pipe_closed(pipe))) {
			rc = buf.used ? buf.used : -EPIPE;
			بشکن;
		}

		rc = wait_for(&pipe->data, pipe, &کلید, end, &need_resched, &buf);
		اگر (rc != 0) {
			اگر (rc == -EAGAIN) {
				rc = buf.used ? buf.used : -EAGAIN;
			}
			بشکن;
		}
	}
out:
	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_pipe, اقرأ, pipe, rc);
	اگر (need_resched) {
		z_reschedule(&pipe->lock, کلید);
	} وگرنه {
		k_spin_unlock(&pipe->lock, کلید);
	}
	بازگشت rc;
}

تهی z_impl_k_pipe_reset(ساختار k_pipe *pipe)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_pipe, reset, pipe);
	K_SPINLOCK(&pipe->lock) {
		ring_buf_reset(&pipe->buf);
		اگر (likely(pipe->waiting != 0)) {
			pipe->flags |= PIPE_FLAG_RESET;
			z_sched_wake_all(&pipe->data, 0, پوچ);
			z_sched_wake_all(&pipe->space, 0, پوچ);
		}
	}
	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_pipe, reset, pipe);
}

تهی z_impl_k_pipe_close(ساختار k_pipe *pipe)
{
	k_spinlock_کلید_t کلید;
	bool need_resched;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_pipe, close, pipe);

	کلید = k_spin_lock(&pipe->lock);

	pipe->flags = 0;
	need_resched = z_sched_wake_all(&pipe->data, 0, پوچ);
	need_resched |= z_sched_wake_all(&pipe->space, 0, پوچ);

#ifdef CONFIG_POLL
	/*
	 * Closing is a broadcast: wake every registered poller. Later
	 * k_poll() calls see the hangup through is_condition_met().
	 */
	وقتی (z_handle_obj_poll_events(&pipe->poll_events,
					K_POLL_STATE_PIPE_DATA_AVAILABLE)) {
		need_resched = true;
	}
#endif /* CONFIG_POLL */

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_pipe, close, pipe);

	اگر (need_resched) {
		z_reschedule(&pipe->lock, کلید);
	} وگرنه {
		k_spin_unlock(&pipe->lock, کلید);
	}
}

#ifdef CONFIG_USERSPACE
تهی z_vrfy_k_pipe_init(ساختار k_pipe *pipe, uint8_t *buffer, size_t buffer_size)
{
	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(pipe, K_OBJ_PIPE));
	K_OOPS(K_SYSCALL_MEMORY_WRITE(buffer, buffer_size));

	z_impl_k_pipe_init(pipe, buffer, buffer_size);
}
#شامل <zephyr/syscalls/k_pipe_init_mrsh.c>

صحیح z_vrfy_k_pipe_read(ساختار k_pipe *pipe, uint8_t *data, size_t len, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(pipe, K_OBJ_PIPE));
	K_OOPS(K_SYSCALL_MEMORY_WRITE(data, len));

	بازگشت z_impl_k_pipe_read(pipe, data, len, timeout);
}
#شامل <zephyr/syscalls/k_pipe_read_mrsh.c>

صحیح z_vrfy_k_pipe_write(ساختار k_pipe *pipe, ثابت uint8_t *data, size_t len, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(pipe, K_OBJ_PIPE));
	K_OOPS(K_SYSCALL_MEMORY_READ(data, len));

	بازگشت z_impl_k_pipe_write(pipe, data, len, timeout);
}
#شامل <zephyr/syscalls/k_pipe_write_mrsh.c>

تهی z_vrfy_k_pipe_reset(ساختار k_pipe *pipe)
{
	K_OOPS(K_SYSCALL_OBJ(pipe, K_OBJ_PIPE));
	z_impl_k_pipe_reset(pipe);
}
#شامل <zephyr/syscalls/k_pipe_reset_mrsh.c>

تهی z_vrfy_k_pipe_close(ساختار k_pipe *pipe)
{
	K_OOPS(K_SYSCALL_OBJ(pipe, K_OBJ_PIPE));
	z_impl_k_pipe_close(pipe);
}
#شامل <zephyr/syscalls/k_pipe_close_mrsh.c>
#endif /* CONFIG_USERSPACE */
