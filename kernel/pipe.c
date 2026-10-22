/*
 * Copyright (c) 2024 Måns Ansgariusson <mansgariusson@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ضمن <zephyr/sys/minmax.h>
#ضمن <zephyr/init.h>
#ضمن <zephyr/kernel.h>
#ضمن <zephyr/internal/syscall_handler.h>
#ضمن <ksched.h>
#ضمن <kthread.h>
#ضمن <wait_q.h>
#ضمن <scheduler.h>

#ifdef CONFIG_OBJ_CORE_PIPE
K_OBJ_TYPE_DEFINE(obj_type_pipe, k_pipe, K_OBJ_TYPE_PIPE_ID, عدم);
#endif /* CONFIG_OBJ_CORE_PIPE */

ساكن مضمن bool pipe_closed(بنية k_pipe *pipe)
{
	أرجع (pipe->flags & PIPE_FLAG_OPEN) == 0;
}

ساكن مضمن bool pipe_resetting(بنية k_pipe *pipe)
{
	أرجع (pipe->flags & PIPE_FLAG_RESET) != 0;
}

ساكن مضمن bool pipe_full(بنية k_pipe *pipe)
{
	أرجع ring_buf_space_get(&pipe->buf) == 0;
}

ساكن مضمن bool pipe_empty(بنية k_pipe *pipe)
{
	أرجع ring_buf_is_empty(&pipe->buf);
}

بنية pipe_buf_spec {
	uint8_t * ثابت data;
	ثابت size_t len;
	size_t used;
};

ساكن صحيح wait_for(_wait_q_t *waitq, بنية k_pipe *pipe, k_spinlock_ئاچقۇچ_t *ئاچقۇچ,
		    k_timepoint_t time_limit, bool *need_resched,
		    بنية pipe_buf_spec *buf_spec)
{
	k_timeout_t timeout = sys_timepoint_timeout(time_limit);
	صحيح rc;

	إذا (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		أرجع -EAGAIN;
	}

	_current->base.swap_data = buf_spec;

	pipe->waiting++;
	*need_resched = false;
	إذا (waitq == &pipe->space) {
		SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_pipe, اكتب, pipe, timeout);
	} وإلا {
		SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_pipe, اقرأ, pipe, timeout);
	}
	rc = z_pend_curr(&pipe->lock, *ئاچقۇچ, waitq, timeout);
	*ئاچقۇچ = k_spin_lock(&pipe->lock);
	pipe->waiting--;
	إذا (unlikely(pipe_resetting(pipe))) {
		إذا (pipe->waiting == 0) {
			pipe->flags &= ~PIPE_FLAG_RESET;
		}
		rc = -ECANCELED;
	}

	أرجع rc;
}

فراغ z_impl_k_pipe_init(بنية k_pipe *pipe, uint8_t *buffer, size_t buffer_size)
{
	ring_buf_init(&pipe->buf, buffer_size, buffer);
	pipe->flags = PIPE_FLAG_OPEN;
	pipe->waiting = 0;

	pipe->lock = (بنية k_spinlock){};
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

ساكن size_t copy_to_pending_readers(بنية k_pipe *pipe, bool *need_resched,
				      ثابت uint8_t *data, size_t len)
{
	بنية k_thread *reader = عدم;
	بنية pipe_buf_spec *reader_buf;
	size_t copy_size, written = 0;

	/*
	 * Attempt a direct data copy to waiting readers if any.
	 * The copy has to be done under the scheduler lock to ensure all the
	 * needed data is copied to the target thread whose buffer spec lives
	 * on that thread's stack, and then the thread unpended only if it
	 * received all the data it wanted, without racing with a potential
	 * thread timeout/cancellation event.
	 */
	افعل {
		LOCK_SCHED_SPINLOCK {
			reader = _priq_wait_best(&pipe->data.waitq);
			إذا (reader == عدم) {
				K_SPINLOCK_BREAK;
			}

			reader_buf = reader->base.swap_data;
			copy_size = min(len - written,
					reader_buf->len - reader_buf->used);
			memcpy(&reader_buf->data[reader_buf->used],
			       &data[written], copy_size);
			written += copy_size;
			reader_buf->used += copy_size;

			إذا (reader_buf->used < reader_buf->len) {
				/* This reader wants more: don't unpend. */
				reader = عدم;
			} وإلا {
				/*
				 * This reader has received all the data
				 * it was waiting for. Set its return
				 * value, unpend, abort its timeout, and
				 * ready it, all under the scheduler lock
				 * so a racing timeout handler cannot
				 * observe a half-initialized wake-up.
				 */
				z_thread_return_قىممەت_set_with_data(reader, 0, عدم);
				unpend_thread_no_timeout(reader);
				(فراغ)z_try_abort_thread_timeout(reader);
				z_sched_ready_locked(reader);
				*need_resched = true;
			}
		}
	} طالما (reader != عدم && written < len);

	أرجع written;
}

صحيح z_impl_k_pipe_write(بنية k_pipe *pipe, ثابت uint8_t *data, size_t len, k_timeout_t timeout)
{
	صحيح rc;
	size_t written = 0;
	size_t added;
	k_timepoint_t end = sys_timepoint_calc(timeout);
	k_spinlock_ئاچقۇچ_t ئاچقۇچ = k_spin_lock(&pipe->lock);
	bool need_resched = false;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_pipe, اكتب, pipe, data, len, timeout);

	إذا (unlikely(len > INT_MAX)) {
		rc = -EOVERFLOW;
		اذهب out;
	}

	إذا (unlikely(pipe_resetting(pipe))) {
		rc = -ECANCELED;
		اذهب out;
	}

	لكل (;;) {
		إذا (unlikely(pipe_closed(pipe))) {
			rc = -EPIPE;
			اكسر;
		}

		إذا (pipe_empty(pipe)) {
			إذا (IS_ENABLED(CONFIG_KERNEL_COHERENCE)) {
				/*
				 * Systems that enabled this option don't have
				 * their stacks in coherent memory. Given our
				 * pipe_buf_spec is stored on the stack, and
				 * readers may also have their destination
				 * buffer on their stack too, it is not worth
				 * supporting direct-to-readers copy with them.
				 * Simply wake up all pending readers instead.
				 */
				need_resched = z_sched_wake_all(&pipe->data, 0, عدم);
			} وإلا إذا (pipe->waiting != 0) {
				written += copy_to_pending_readers(pipe, &need_resched,
								   &data[written],
								   len - written);
				إذا (written >= len) {
					rc = written;
					اكسر;
				}
			}
		}

		added = ring_buf_put(&pipe->buf, &data[written], len - written);

#ifdef CONFIG_POLL
		إذا (added != 0) {
			need_resched |= z_handle_obj_poll_events(&pipe->poll_events,
								 K_POLL_STATE_PIPE_DATA_AVAILABLE);
		}
#endif /* CONFIG_POLL */

		written += added;
		إذا (likely(written == len)) {
			rc = written;
			اكسر;
		}

		rc = wait_for(&pipe->space, pipe, &ئاچقۇچ, end, &need_resched, عدم);
		إذا (rc != 0) {
			إذا (rc == -EAGAIN) {
				rc = written ? written : -EAGAIN;
			}
			اكسر;
		}
	}
out:
	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_pipe, اكتب, pipe, rc);
	إذا (need_resched) {
		z_reschedule(&pipe->lock, ئاچقۇچ);
	} وإلا {
		k_spin_unlock(&pipe->lock, ئاچقۇچ);
	}
	أرجع rc;
}

صحيح z_impl_k_pipe_read(بنية k_pipe *pipe, uint8_t *data, size_t len, k_timeout_t timeout)
{
	بنية pipe_buf_spec buf = { data, len, 0 };
	صحيح rc;
	k_timepoint_t end = sys_timepoint_calc(timeout);
	k_spinlock_ئاچقۇچ_t ئاچقۇچ = k_spin_lock(&pipe->lock);
	bool need_resched = false;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_pipe, اقرأ, pipe, data, len, timeout);

	إذا (unlikely(len > INT_MAX)) {
		rc = -EOVERFLOW;
		اذهب out;
	}

	إذا (unlikely(pipe_resetting(pipe))) {
		rc = -ECANCELED;
		اذهب out;
	}

	لكل (;;) {
		إذا (pipe_full(pipe)) {
			/* One or more pending writers may exist. */
			need_resched = z_sched_wake_all(&pipe->space, 0, عدم);
		}

		buf.used += ring_buf_get(&pipe->buf, &data[buf.used], len - buf.used);
		إذا (likely(buf.used == len)) {
			rc = buf.used;
			اكسر;
		}

		إذا (unlikely(pipe_closed(pipe))) {
			rc = buf.used ? buf.used : -EPIPE;
			اكسر;
		}

		rc = wait_for(&pipe->data, pipe, &ئاچقۇچ, end, &need_resched, &buf);
		إذا (rc != 0) {
			إذا (rc == -EAGAIN) {
				rc = buf.used ? buf.used : -EAGAIN;
			}
			اكسر;
		}
	}
out:
	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_pipe, اقرأ, pipe, rc);
	إذا (need_resched) {
		z_reschedule(&pipe->lock, ئاچقۇچ);
	} وإلا {
		k_spin_unlock(&pipe->lock, ئاچقۇچ);
	}
	أرجع rc;
}

فراغ z_impl_k_pipe_reset(بنية k_pipe *pipe)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_pipe, reset, pipe);
	K_SPINLOCK(&pipe->lock) {
		ring_buf_reset(&pipe->buf);
		إذا (likely(pipe->waiting != 0)) {
			pipe->flags |= PIPE_FLAG_RESET;
			z_sched_wake_all(&pipe->data, 0, عدم);
			z_sched_wake_all(&pipe->space, 0, عدم);
		}
	}
	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_pipe, reset, pipe);
}

فراغ z_impl_k_pipe_close(بنية k_pipe *pipe)
{
	k_spinlock_ئاچقۇچ_t ئاچقۇچ;
	bool need_resched;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_pipe, close, pipe);

	ئاچقۇچ = k_spin_lock(&pipe->lock);

	pipe->flags = 0;
	need_resched = z_sched_wake_all(&pipe->data, 0, عدم);
	need_resched |= z_sched_wake_all(&pipe->space, 0, عدم);

#ifdef CONFIG_POLL
	/*
	 * Closing is a broadcast: wake every registered poller. Later
	 * k_poll() calls see the hangup through is_condition_met().
	 */
	طالما (z_handle_obj_poll_events(&pipe->poll_events,
					K_POLL_STATE_PIPE_DATA_AVAILABLE)) {
		need_resched = true;
	}
#endif /* CONFIG_POLL */

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_pipe, close, pipe);

	إذا (need_resched) {
		z_reschedule(&pipe->lock, ئاچقۇچ);
	} وإلا {
		k_spin_unlock(&pipe->lock, ئاچقۇچ);
	}
}

#ifdef CONFIG_USERSPACE
فراغ z_vrfy_k_pipe_init(بنية k_pipe *pipe, uint8_t *buffer, size_t buffer_size)
{
	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(pipe, K_OBJ_PIPE));
	K_OOPS(K_SYSCALL_MEMORY_WRITE(buffer, buffer_size));

	z_impl_k_pipe_init(pipe, buffer, buffer_size);
}
#ضمن <zephyr/syscalls/k_pipe_init_mrsh.c>

صحيح z_vrfy_k_pipe_read(بنية k_pipe *pipe, uint8_t *data, size_t len, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(pipe, K_OBJ_PIPE));
	K_OOPS(K_SYSCALL_MEMORY_WRITE(data, len));

	أرجع z_impl_k_pipe_read(pipe, data, len, timeout);
}
#ضمن <zephyr/syscalls/k_pipe_read_mrsh.c>

صحيح z_vrfy_k_pipe_write(بنية k_pipe *pipe, ثابت uint8_t *data, size_t len, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(pipe, K_OBJ_PIPE));
	K_OOPS(K_SYSCALL_MEMORY_READ(data, len));

	أرجع z_impl_k_pipe_write(pipe, data, len, timeout);
}
#ضمن <zephyr/syscalls/k_pipe_write_mrsh.c>

فراغ z_vrfy_k_pipe_reset(بنية k_pipe *pipe)
{
	K_OOPS(K_SYSCALL_OBJ(pipe, K_OBJ_PIPE));
	z_impl_k_pipe_reset(pipe);
}
#ضمن <zephyr/syscalls/k_pipe_reset_mrsh.c>

فراغ z_vrfy_k_pipe_close(بنية k_pipe *pipe)
{
	K_OOPS(K_SYSCALL_OBJ(pipe, K_OBJ_PIPE));
	z_impl_k_pipe_close(pipe);
}
#ضمن <zephyr/syscalls/k_pipe_close_mrsh.c>
#endif /* CONFIG_USERSPACE */
