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

#که_تعریف_شوی CONFIG_OBJ_CORE_PIPE
K_OBJ_TYPE_DEFINE(obj_type_pipe, k_pipe, K_OBJ_TYPE_PIPE_ID, عدم);
#د_که_پای /* CONFIG_OBJ_CORE_PIPE */

ساكن مضمن bool pipe_closed(بنية k_pipe *pipe)
{
	أرجع (pipe->flags & PIPE_FLAG_OPEN) == 0;
}

ساكن مضمن bool pipe_resetting(بنية k_pipe *pipe)
{
	أرجع (pipe->flags & PIPE_FLAG_RESET) != 0;
}

ساكن مضمن bool pipe_ډک(بنية k_pipe *pipe)
{
	أرجع ring_buf_space_get(&pipe->buf) == 0;
}

ساكن مضمن bool pipe_تشه(بنية k_pipe *pipe)
{
	أرجع ring_buf_is_تشه(&pipe->buf);
}

بنية pipe_buf_spec {
	uint8_t * ثابت معلومات;
	ثابت size_t len;
	size_t کارول_شوی;
};

ساكن صحيح wait_for(_wait_q_t *waitq, بنية k_pipe *pipe, k_spinlock_کلی_t *کلی,
		    k_timepoint_t وخت_حد, bool *need_resched,
		    بنية pipe_buf_spec *buf_spec)
{
	k_timeout_t timeout = sys_timepoint_timeout(وخت_حد);
	صحيح rc;

	إذا (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		أرجع -EAGAIN;
	}

	_current->بنسټ.بدلون_معلومات = buf_spec;

	pipe->انتظار++;
	*need_resched = false;
	إذا (waitq == &pipe->space) {
		SYS_پورټ_TRACING_OBJ_FUNC_BLOCKING(k_pipe, اكتب, pipe, timeout);
	} وإلا {
		SYS_پورټ_TRACING_OBJ_FUNC_BLOCKING(k_pipe, اقرأ, pipe, timeout);
	}
	rc = z_pend_curr(&pipe->قلف, *کلی, waitq, timeout);
	*کلی = k_spin_قلف(&pipe->قلف);
	pipe->انتظار--;
	إذا (unlikely(pipe_resetting(pipe))) {
		إذا (pipe->انتظار == 0) {
			pipe->flags &= ~PIPE_FLAG_RESET;
		}
		rc = -ECANCELED;
	}

	أرجع rc;
}

فراغ z_impl_k_pipe_init(بنية k_pipe *pipe, uint8_t *بفر, size_t بفر_اندازه)
{
	ring_buf_init(&pipe->buf, بفر_اندازه, بفر);
	pipe->flags = PIPE_FLAG_OPEN;
	pipe->انتظار = 0;

	pipe->قلف = (بنية k_spinlock){};
	z_waitq_init(&pipe->معلومات);
	z_waitq_init(&pipe->space);
	k_object_init(pipe);

#که_تعریف_شوی CONFIG_POLL
	sys_dlist_init(&pipe->poll_events);
#د_که_پای /* CONFIG_POLL */
#که_تعریف_شوی CONFIG_OBJ_CORE_PIPE
	k_obj_core_init_and_link(K_OBJ_CORE(pipe), &obj_type_pipe);
#د_که_پای /* CONFIG_OBJ_CORE_PIPE */
	SYS_پورټ_TRACING_OBJ_INIT(k_pipe, pipe, بفر, بفر_اندازه);
}

ساكن size_t copy_منزل_pending_readers(بنية k_pipe *pipe, bool *need_resched,
				      ثابت uint8_t *معلومات, size_t len)
{
	بنية k_thread *reader = عدم;
	بنية pipe_buf_spec *reader_buf;
	size_t copy_اندازه, written = 0;

	/*
	 * Attempt a direct data copy to waiting readers if any.
	 * The copy has to be done under the scheduler lock to ensure all the
	 * needed data is copied to the target thread whose buffer spec lives
	 * on that thread's stack, and then the thread unpended only if it
	 * received all the data it wanted, without racing with a potential
	 * thread timeout/cancellation event.
	 */
	افعل {
		قلف_SCHED_SPINLOCK {
			reader = priq_wait_غوره(&pipe->معلومات.waitq);
			إذا (reader == عدم) {
				K_SPINLOCK_BREAK;
			}

			reader_buf = reader->بنسټ.بدلون_معلومات;
			copy_اندازه = min(len - written,
					reader_buf->len - reader_buf->کارول_شوی);
			حافظه_نقل(&reader_buf->معلومات[reader_buf->کارول_شوی],
			       &معلومات[written], copy_اندازه);
			written += copy_اندازه;
			reader_buf->کارول_شوی += copy_اندازه;

			إذا (reader_buf->کارول_شوی < reader_buf->len) {
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
				z_thread_return_ارزښت_سېټ_with_معلومات(reader, 0, عدم);
				unpend_thread_no_timeout(reader);
				(فراغ)z_try_abort_thread_timeout(reader);
				z_sched_ready_locked(reader);
				*need_resched = true;
			}
		}
	} طالما (reader != عدم && written < len);

	أرجع written;
}

صحيح z_impl_k_pipe_write(بنية k_pipe *pipe, ثابت uint8_t *معلومات, size_t len, k_timeout_t timeout)
{
	صحيح rc;
	size_t written = 0;
	size_t added;
	k_timepoint_t پای_ټکی = sys_timepoint_calc(timeout);
	k_spinlock_کلی_t کلی = k_spin_قلف(&pipe->قلف);
	bool need_resched = false;

	SYS_پورټ_TRACING_OBJ_FUNC_ENTER(k_pipe, اكتب, pipe, معلومات, len, timeout);

	إذا (unlikely(len > لوی_عدد)) {
		rc = -EOVERFLOW;
		اذهب بهر;
	}

	إذا (unlikely(pipe_resetting(pipe))) {
		rc = -ECANCELED;
		اذهب بهر;
	}

	لكل (;;) {
		إذا (unlikely(pipe_closed(pipe))) {
			rc = -EPIPE;
			اكسر;
		}

		إذا (pipe_تشه(pipe)) {
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
				need_resched = z_sched_wake_all(&pipe->معلومات, 0, عدم);
			} وإلا إذا (pipe->انتظار != 0) {
				written += copy_منزل_pending_readers(pipe, &need_resched,
								   &معلومات[written],
								   len - written);
				إذا (written >= len) {
					rc = written;
					اكسر;
				}
			}
		}

		added = ring_buf_put(&pipe->buf, &معلومات[written], len - written);

#که_تعریف_شوی CONFIG_POLL
		إذا (added != 0) {
			need_resched |= z_handle_obj_poll_events(&pipe->poll_events,
								 K_POLL_STATE_PIPE_معلومات_شته);
		}
#د_که_پای /* CONFIG_POLL */

		written += added;
		إذا (likely(written == len)) {
			rc = written;
			اكسر;
		}

		rc = wait_for(&pipe->space, pipe, &کلی, پای_ټکی, &need_resched, عدم);
		إذا (rc != 0) {
			إذا (rc == -EAGAIN) {
				rc = written ? written : -EAGAIN;
			}
			اكسر;
		}
	}
بهر:
	SYS_پورټ_TRACING_OBJ_FUNC_EXIT(k_pipe, اكتب, pipe, rc);
	إذا (need_resched) {
		z_reschedule(&pipe->قلف, کلی);
	} وإلا {
		k_spin_unlock(&pipe->قلف, کلی);
	}
	أرجع rc;
}

صحيح z_impl_k_pipe_read(بنية k_pipe *pipe, uint8_t *معلومات, size_t len, k_timeout_t timeout)
{
	بنية pipe_buf_spec buf = { معلومات, len, 0 };
	صحيح rc;
	k_timepoint_t پای_ټکی = sys_timepoint_calc(timeout);
	k_spinlock_کلی_t کلی = k_spin_قلف(&pipe->قلف);
	bool need_resched = false;

	SYS_پورټ_TRACING_OBJ_FUNC_ENTER(k_pipe, اقرأ, pipe, معلومات, len, timeout);

	إذا (unlikely(len > لوی_عدد)) {
		rc = -EOVERFLOW;
		اذهب بهر;
	}

	إذا (unlikely(pipe_resetting(pipe))) {
		rc = -ECANCELED;
		اذهب بهر;
	}

	لكل (;;) {
		إذا (pipe_ډک(pipe)) {
			/* One or more pending writers may exist. */
			need_resched = z_sched_wake_all(&pipe->space, 0, عدم);
		}

		buf.کارول_شوی += ring_buf_get(&pipe->buf, &معلومات[buf.کارول_شوی], len - buf.کارول_شوی);
		إذا (likely(buf.کارول_شوی == len)) {
			rc = buf.کارول_شوی;
			اكسر;
		}

		إذا (unlikely(pipe_closed(pipe))) {
			rc = buf.کارول_شوی ? buf.کارول_شوی : -EPIPE;
			اكسر;
		}

		rc = wait_for(&pipe->معلومات, pipe, &کلی, پای_ټکی, &need_resched, &buf);
		إذا (rc != 0) {
			إذا (rc == -EAGAIN) {
				rc = buf.کارول_شوی ? buf.کارول_شوی : -EAGAIN;
			}
			اكسر;
		}
	}
بهر:
	SYS_پورټ_TRACING_OBJ_FUNC_EXIT(k_pipe, اقرأ, pipe, rc);
	إذا (need_resched) {
		z_reschedule(&pipe->قلف, کلی);
	} وإلا {
		k_spin_unlock(&pipe->قلف, کلی);
	}
	أرجع rc;
}

فراغ z_impl_k_pipe_reset(بنية k_pipe *pipe)
{
	SYS_پورټ_TRACING_OBJ_FUNC_ENTER(k_pipe, reset, pipe);
	K_SPINLOCK(&pipe->قلف) {
		ring_buf_reset(&pipe->buf);
		إذا (likely(pipe->انتظار != 0)) {
			pipe->flags |= PIPE_FLAG_RESET;
			z_sched_wake_all(&pipe->معلومات, 0, عدم);
			z_sched_wake_all(&pipe->space, 0, عدم);
		}
	}
	SYS_پورټ_TRACING_OBJ_FUNC_EXIT(k_pipe, reset, pipe);
}

فراغ z_impl_k_pipe_close(بنية k_pipe *pipe)
{
	k_spinlock_کلی_t کلی;
	bool need_resched;

	SYS_پورټ_TRACING_OBJ_FUNC_ENTER(k_pipe, close, pipe);

	کلی = k_spin_قلف(&pipe->قلف);

	pipe->flags = 0;
	need_resched = z_sched_wake_all(&pipe->معلومات, 0, عدم);
	need_resched |= z_sched_wake_all(&pipe->space, 0, عدم);

#که_تعریف_شوی CONFIG_POLL
	/*
	 * Closing is a broadcast: wake every registered poller. Later
	 * k_poll() calls see the hangup through is_condition_met().
	 */
	طالما (z_handle_obj_poll_events(&pipe->poll_events,
					K_POLL_STATE_PIPE_معلومات_شته)) {
		need_resched = true;
	}
#د_که_پای /* CONFIG_POLL */

	SYS_پورټ_TRACING_OBJ_FUNC_EXIT(k_pipe, close, pipe);

	إذا (need_resched) {
		z_reschedule(&pipe->قلف, کلی);
	} وإلا {
		k_spin_unlock(&pipe->قلف, کلی);
	}
}

#که_تعریف_شوی CONFIG_USERSPACE
فراغ z_vrfy_k_pipe_init(بنية k_pipe *pipe, uint8_t *بفر, size_t بفر_اندازه)
{
	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(pipe, K_OBJ_PIPE));
	K_OOPS(K_SYSCALL_حافظه_WRITE(بفر, بفر_اندازه));

	z_impl_k_pipe_init(pipe, بفر, بفر_اندازه);
}
#ضمن <zephyr/syscalls/k_pipe_init_mrsh.c>

صحيح z_vrfy_k_pipe_read(بنية k_pipe *pipe, uint8_t *معلومات, size_t len, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(pipe, K_OBJ_PIPE));
	K_OOPS(K_SYSCALL_حافظه_WRITE(معلومات, len));

	أرجع z_impl_k_pipe_read(pipe, معلومات, len, timeout);
}
#ضمن <zephyr/syscalls/k_pipe_read_mrsh.c>

صحيح z_vrfy_k_pipe_write(بنية k_pipe *pipe, ثابت uint8_t *معلومات, size_t len, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(pipe, K_OBJ_PIPE));
	K_OOPS(K_SYSCALL_حافظه_READ(معلومات, len));

	أرجع z_impl_k_pipe_write(pipe, معلومات, len, timeout);
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
#د_که_پای /* CONFIG_USERSPACE */
