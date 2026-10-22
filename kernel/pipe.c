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
K_OBJ_TYPE_DEFINE(obj_type_pipe, k_pipe, K_OBJ_TYPE_PIPE_ID, ফাঁকা);
#endif /* CONFIG_OBJ_CORE_PIPE */

স্থির অন্তঃরেখ bool pipe_closed(কাঠামো k_pipe *pipe)
{
	ফেরত (pipe->flags & PIPE_FLAG_OPEN) == 0;
}

স্থির অন্তঃরেখ bool pipe_resetting(কাঠামো k_pipe *pipe)
{
	ফেরত (pipe->flags & PIPE_FLAG_RESET) != 0;
}

স্থির অন্তঃরেখ bool pipe_full(কাঠামো k_pipe *pipe)
{
	ফেরত ring_buf_space_get(&pipe->buf) == 0;
}

স্থির অন্তঃরেখ bool pipe_empty(কাঠামো k_pipe *pipe)
{
	ফেরত ring_buf_is_empty(&pipe->buf);
}

কাঠামো pipe_buf_spec {
	uint8_t * ধ্রুবক তথ্য;
	ধ্রুবক size_t len;
	size_t used;
};

স্থির পূর্ণসংখ্যা wait_for(_wait_q_t *waitq, কাঠামো k_pipe *pipe, k_spinlock_চাবি_t *চাবি,
		    k_timepoint_t time_limit, bool *need_resched,
		    কাঠামো pipe_buf_spec *buf_spec)
{
	k_timeout_t timeout = sys_timepoint_timeout(time_limit);
	পূর্ণসংখ্যা rc;

	যদি (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		ফেরত -EAGAIN;
	}

	_current->base.swap_তথ্য = buf_spec;

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

	ফেরত rc;
}

শূন্য z_impl_k_pipe_init(কাঠামো k_pipe *pipe, uint8_t *বাফার, size_t বাফার_আকার)
{
	ring_buf_init(&pipe->buf, বাফার_আকার, বাফার);
	pipe->flags = PIPE_FLAG_OPEN;
	pipe->waiting = 0;

	pipe->lock = (কাঠামো k_spinlock){};
	z_waitq_init(&pipe->তথ্য);
	z_waitq_init(&pipe->space);
	k_object_init(pipe);

#ifdef CONFIG_POLL
	sys_dlist_init(&pipe->poll_events);
#endif /* CONFIG_POLL */
#ifdef CONFIG_OBJ_CORE_PIPE
	k_obj_core_init_and_link(K_OBJ_CORE(pipe), &obj_type_pipe);
#endif /* CONFIG_OBJ_CORE_PIPE */
	SYS_PORT_TRACING_OBJ_INIT(k_pipe, pipe, বাফার, বাফার_আকার);
}

স্থির size_t copy_to_pending_readers(কাঠামো k_pipe *pipe, bool *need_resched,
				      ধ্রুবক uint8_t *তথ্য, size_t len)
{
	কাঠামো k_thread *reader = ফাঁকা;
	কাঠামো pipe_buf_spec *reader_buf;
	size_t copy_আকার, written = 0;

	/*
	 * Attempt a direct data copy to waiting readers if any.
	 * The copy has to be done under the scheduler lock to ensure all the
	 * needed data is copied to the target thread whose buffer spec lives
	 * on that thread's stack, and then the thread unpended only if it
	 * received all the data it wanted, without racing with a potential
	 * thread timeout/cancellation event.
	 */
	করো {
		LOCK_SCHED_SPINLOCK {
			reader = _priq_wait_best(&pipe->তথ্য.waitq);
			যদি (reader == ফাঁকা) {
				K_SPINLOCK_BREAK;
			}

			reader_buf = reader->base.swap_তথ্য;
			copy_আকার = min(len - written,
					reader_buf->len - reader_buf->used);
			memcpy(&reader_buf->তথ্য[reader_buf->used],
			       &তথ্য[written], copy_আকার);
			written += copy_আকার;
			reader_buf->used += copy_আকার;

			যদি (reader_buf->used < reader_buf->len) {
				/* This reader wants more: don't unpend. */
				reader = ফাঁকা;
			} অন্যথা {
				/*
				 * This reader has received all the data
				 * it was waiting for. Set its return
				 * value, unpend, abort its timeout, and
				 * ready it, all under the scheduler lock
				 * so a racing timeout handler cannot
				 * observe a half-initialized wake-up.
				 */
				z_thread_return_মান_set_with_তথ্য(reader, 0, ফাঁকা);
				unpend_thread_no_timeout(reader);
				(শূন্য)z_try_abort_thread_timeout(reader);
				z_sched_ready_locked(reader);
				*need_resched = true;
			}
		}
	} যখন (reader != ফাঁকা && written < len);

	ফেরত written;
}

পূর্ণসংখ্যা z_impl_k_pipe_write(কাঠামো k_pipe *pipe, ধ্রুবক uint8_t *তথ্য, size_t len, k_timeout_t timeout)
{
	পূর্ণসংখ্যা rc;
	size_t written = 0;
	size_t added;
	k_timepoint_t end = sys_timepoint_calc(timeout);
	k_spinlock_চাবি_t চাবি = k_spin_lock(&pipe->lock);
	bool need_resched = false;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_pipe, লিখ, pipe, তথ্য, len, timeout);

	যদি (unlikely(len > INT_MAX)) {
		rc = -EOVERFLOW;
		যাও out;
	}

	যদি (unlikely(pipe_resetting(pipe))) {
		rc = -ECANCELED;
		যাও out;
	}

	জন্য (;;) {
		যদি (unlikely(pipe_closed(pipe))) {
			rc = -EPIPE;
			বিরতি;
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
				need_resched = z_sched_wake_all(&pipe->তথ্য, 0, ফাঁকা);
			} অন্যথা যদি (pipe->waiting != 0) {
				written += copy_to_pending_readers(pipe, &need_resched,
								   &তথ্য[written],
								   len - written);
				যদি (written >= len) {
					rc = written;
					বিরতি;
				}
			}
		}

		added = ring_buf_put(&pipe->buf, &তথ্য[written], len - written);

#ifdef CONFIG_POLL
		যদি (added != 0) {
			need_resched |= z_handle_obj_poll_events(&pipe->poll_events,
								 K_POLL_STATE_PIPE_তথ্য_AVAILABLE);
		}
#endif /* CONFIG_POLL */

		written += added;
		যদি (likely(written == len)) {
			rc = written;
			বিরতি;
		}

		rc = wait_for(&pipe->space, pipe, &চাবি, end, &need_resched, ফাঁকা);
		যদি (rc != 0) {
			যদি (rc == -EAGAIN) {
				rc = written ? written : -EAGAIN;
			}
			বিরতি;
		}
	}
out:
	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_pipe, লিখ, pipe, rc);
	যদি (need_resched) {
		z_reschedule(&pipe->lock, চাবি);
	} অন্যথা {
		k_spin_unlock(&pipe->lock, চাবি);
	}
	ফেরত rc;
}

পূর্ণসংখ্যা z_impl_k_pipe_read(কাঠামো k_pipe *pipe, uint8_t *তথ্য, size_t len, k_timeout_t timeout)
{
	কাঠামো pipe_buf_spec buf = { তথ্য, len, 0 };
	পূর্ণসংখ্যা rc;
	k_timepoint_t end = sys_timepoint_calc(timeout);
	k_spinlock_চাবি_t চাবি = k_spin_lock(&pipe->lock);
	bool need_resched = false;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_pipe, পঠ, pipe, তথ্য, len, timeout);

	যদি (unlikely(len > INT_MAX)) {
		rc = -EOVERFLOW;
		যাও out;
	}

	যদি (unlikely(pipe_resetting(pipe))) {
		rc = -ECANCELED;
		যাও out;
	}

	জন্য (;;) {
		যদি (pipe_full(pipe)) {
			/* One or more pending writers may exist. */
			need_resched = z_sched_wake_all(&pipe->space, 0, ফাঁকা);
		}

		buf.used += ring_buf_get(&pipe->buf, &তথ্য[buf.used], len - buf.used);
		যদি (likely(buf.used == len)) {
			rc = buf.used;
			বিরতি;
		}

		যদি (unlikely(pipe_closed(pipe))) {
			rc = buf.used ? buf.used : -EPIPE;
			বিরতি;
		}

		rc = wait_for(&pipe->তথ্য, pipe, &চাবি, end, &need_resched, &buf);
		যদি (rc != 0) {
			যদি (rc == -EAGAIN) {
				rc = buf.used ? buf.used : -EAGAIN;
			}
			বিরতি;
		}
	}
out:
	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_pipe, পঠ, pipe, rc);
	যদি (need_resched) {
		z_reschedule(&pipe->lock, চাবি);
	} অন্যথা {
		k_spin_unlock(&pipe->lock, চাবি);
	}
	ফেরত rc;
}

শূন্য z_impl_k_pipe_reset(কাঠামো k_pipe *pipe)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_pipe, reset, pipe);
	K_SPINLOCK(&pipe->lock) {
		ring_buf_reset(&pipe->buf);
		যদি (likely(pipe->waiting != 0)) {
			pipe->flags |= PIPE_FLAG_RESET;
			z_sched_wake_all(&pipe->তথ্য, 0, ফাঁকা);
			z_sched_wake_all(&pipe->space, 0, ফাঁকা);
		}
	}
	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_pipe, reset, pipe);
}

শূন্য z_impl_k_pipe_close(কাঠামো k_pipe *pipe)
{
	k_spinlock_চাবি_t চাবি;
	bool need_resched;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_pipe, close, pipe);

	চাবি = k_spin_lock(&pipe->lock);

	pipe->flags = 0;
	need_resched = z_sched_wake_all(&pipe->তথ্য, 0, ফাঁকা);
	need_resched |= z_sched_wake_all(&pipe->space, 0, ফাঁকা);

#ifdef CONFIG_POLL
	/*
	 * Closing is a broadcast: wake every registered poller. Later
	 * k_poll() calls see the hangup through is_condition_met().
	 */
	যখন (z_handle_obj_poll_events(&pipe->poll_events,
					K_POLL_STATE_PIPE_তথ্য_AVAILABLE)) {
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
শূন্য z_vrfy_k_pipe_init(কাঠামো k_pipe *pipe, uint8_t *বাফার, size_t বাফার_আকার)
{
	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(pipe, K_OBJ_PIPE));
	K_OOPS(K_SYSCALL_MEMORY_WRITE(বাফার, বাফার_আকার));

	z_impl_k_pipe_init(pipe, বাফার, বাফার_আকার);
}
#অন্তর্ভাব <zephyr/syscalls/k_pipe_init_mrsh.c>

পূর্ণসংখ্যা z_vrfy_k_pipe_read(কাঠামো k_pipe *pipe, uint8_t *তথ্য, size_t len, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(pipe, K_OBJ_PIPE));
	K_OOPS(K_SYSCALL_MEMORY_WRITE(তথ্য, len));

	ফেরত z_impl_k_pipe_read(pipe, তথ্য, len, timeout);
}
#অন্তর্ভাব <zephyr/syscalls/k_pipe_read_mrsh.c>

পূর্ণসংখ্যা z_vrfy_k_pipe_write(কাঠামো k_pipe *pipe, ধ্রুবক uint8_t *তথ্য, size_t len, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(pipe, K_OBJ_PIPE));
	K_OOPS(K_SYSCALL_MEMORY_READ(তথ্য, len));

	ফেরত z_impl_k_pipe_write(pipe, তথ্য, len, timeout);
}
#অন্তর্ভাব <zephyr/syscalls/k_pipe_write_mrsh.c>

শূন্য z_vrfy_k_pipe_reset(কাঠামো k_pipe *pipe)
{
	K_OOPS(K_SYSCALL_OBJ(pipe, K_OBJ_PIPE));
	z_impl_k_pipe_reset(pipe);
}
#অন্তর্ভাব <zephyr/syscalls/k_pipe_reset_mrsh.c>

শূন্য z_vrfy_k_pipe_close(কাঠামো k_pipe *pipe)
{
	K_OOPS(K_SYSCALL_OBJ(pipe, K_OBJ_PIPE));
	z_impl_k_pipe_close(pipe);
}
#অন্তর্ভাব <zephyr/syscalls/k_pipe_close_mrsh.c>
#endif /* CONFIG_USERSPACE */
