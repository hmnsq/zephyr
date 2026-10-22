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

#جيڪڏهن_وصف_ٿيل CONFIG_OBJ_CORE_PIPE
K_OBJ_TYPE_DEFINE(obj_type_pipe, k_pipe, K_OBJ_TYPE_PIPE_ID, عدم);
#جيڪڏهن_پورو /* CONFIG_OBJ_CORE_PIPE */

ساكن مضمن bool pipe_closed(بنية k_pipe *pipe)
{
	أرجع (pipe->flags & PIPE_FLAG_OPEN) == 0;
}

ساكن مضمن bool pipe_resetting(بنية k_pipe *pipe)
{
	أرجع (pipe->flags & PIPE_FLAG_RESET) != 0;
}

ساكن مضمن bool pipe_ڀريل(بنية k_pipe *pipe)
{
	أرجع ring_buf_space_get(&pipe->buf) == 0;
}

ساكن مضمن bool pipe_خالي_جايون(بنية k_pipe *pipe)
{
	أرجع ring_buf_is_خالي_جايون(&pipe->buf);
}

بنية pipe_buf_spec {
	uint8_t * ثابت معلومات;
	ثابت size_t len;
	size_t استعمال_ٿيل;
};

ساكن صحيح wait_for(_wait_q_t *waitq, بنية k_pipe *pipe, k_spinlock_ڪنجي_t *ڪنجي,
		    k_timepoint_t وقت_حد_بندي, bool *need_resched,
		    بنية pipe_buf_spec *buf_spec)
{
	k_timeout_t timeout = sys_timepoint_timeout(وقت_حد_بندي);
	صحيح rc;

	إذا (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		أرجع -EAGAIN;
	}

	_current->بنياد.مٽا_سٽا_معلومات = buf_spec;

	pipe->انتظار++;
	*need_resched = false;
	إذا (waitq == &pipe->space) {
		SYS_پورٽ_TRACING_OBJ_FUNC_BLOCKING(k_pipe, اكتب, pipe, timeout);
	} وإلا {
		SYS_پورٽ_TRACING_OBJ_FUNC_BLOCKING(k_pipe, اقرأ, pipe, timeout);
	}
	rc = z_pend_curr(&pipe->تالو, *ڪنجي, waitq, timeout);
	*ڪنجي = k_spin_تالو(&pipe->تالو);
	pipe->انتظار--;
	إذا (unlikely(pipe_resetting(pipe))) {
		إذا (pipe->انتظار == 0) {
			pipe->flags &= ~PIPE_FLAG_RESET;
		}
		rc = -ECANCELED;
	}

	أرجع rc;
}

فراغ z_impl_k_pipe_init(بنية k_pipe *pipe, uint8_t *بفر, size_t بفر_سائيز)
{
	ring_buf_init(&pipe->buf, بفر_سائيز, بفر);
	pipe->flags = PIPE_FLAG_OPEN;
	pipe->انتظار = 0;

	pipe->تالو = (بنية k_spinlock){};
	z_waitq_init(&pipe->معلومات);
	z_waitq_init(&pipe->space);
	k_object_init(pipe);

#جيڪڏهن_وصف_ٿيل CONFIG_POLL
	sys_dlist_init(&pipe->poll_events);
#جيڪڏهن_پورو /* CONFIG_POLL */
#جيڪڏهن_وصف_ٿيل CONFIG_OBJ_CORE_PIPE
	k_obj_core_init_and_link(K_OBJ_CORE(pipe), &obj_type_pipe);
#جيڪڏهن_پورو /* CONFIG_OBJ_CORE_PIPE */
	SYS_پورٽ_TRACING_OBJ_INIT(k_pipe, pipe, بفر, بفر_سائيز);
}

ساكن size_t copy_منزل_pending_readers(بنية k_pipe *pipe, bool *need_resched,
				      ثابت uint8_t *معلومات, size_t len)
{
	بنية k_thread *reader = عدم;
	بنية pipe_buf_spec *reader_buf;
	size_t copy_سائيز, written = 0;

	/*
	 * Attempt a direct data copy to waiting readers if any.
	 * The copy has to be done under the scheduler lock to ensure all the
	 * needed data is copied to the target thread whose buffer spec lives
	 * on that thread's stack, and then the thread unpended only if it
	 * received all the data it wanted, without racing with a potential
	 * thread timeout/cancellation event.
	 */
	افعل {
		تالو_SCHED_SPINLOCK {
			reader = priq_wait_بهترين(&pipe->معلومات.waitq);
			إذا (reader == عدم) {
				K_SPINLOCK_BREAK;
			}

			reader_buf = reader->بنياد.مٽا_سٽا_معلومات;
			copy_سائيز = min(len - written,
					reader_buf->len - reader_buf->استعمال_ٿيل);
			يادگيري_نقل(&reader_buf->معلومات[reader_buf->استعمال_ٿيل],
			       &معلومات[written], copy_سائيز);
			written += copy_سائيز;
			reader_buf->استعمال_ٿيل += copy_سائيز;

			إذا (reader_buf->استعمال_ٿيل < reader_buf->len) {
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
				z_thread_return_قدر_سيٽ_with_معلومات(reader, 0, عدم);
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
	k_timepoint_t پڇاڙي = sys_timepoint_calc(timeout);
	k_spinlock_ڪنجي_t ڪنجي = k_spin_تالو(&pipe->تالو);
	bool need_resched = false;

	SYS_پورٽ_TRACING_OBJ_FUNC_ENTER(k_pipe, اكتب, pipe, معلومات, len, timeout);

	إذا (unlikely(len > وڌ_کان_وڌ_عدد)) {
		rc = -EOVERFLOW;
		اذهب ٻاهريون;
	}

	إذا (unlikely(pipe_resetting(pipe))) {
		rc = -ECANCELED;
		اذهب ٻاهريون;
	}

	لكل (;;) {
		إذا (unlikely(pipe_closed(pipe))) {
			rc = -EPIPE;
			اكسر;
		}

		إذا (pipe_خالي_جايون(pipe)) {
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

#جيڪڏهن_وصف_ٿيل CONFIG_POLL
		إذا (added != 0) {
			need_resched |= z_handle_obj_poll_events(&pipe->poll_events,
								 K_POLL_STATE_PIPE_معلومات_موجود);
		}
#جيڪڏهن_پورو /* CONFIG_POLL */

		written += added;
		إذا (likely(written == len)) {
			rc = written;
			اكسر;
		}

		rc = wait_for(&pipe->space, pipe, &ڪنجي, پڇاڙي, &need_resched, عدم);
		إذا (rc != 0) {
			إذا (rc == -EAGAIN) {
				rc = written ? written : -EAGAIN;
			}
			اكسر;
		}
	}
ٻاهريون:
	SYS_پورٽ_TRACING_OBJ_FUNC_EXIT(k_pipe, اكتب, pipe, rc);
	إذا (need_resched) {
		z_reschedule(&pipe->تالو, ڪنجي);
	} وإلا {
		k_spin_unlock(&pipe->تالو, ڪنجي);
	}
	أرجع rc;
}

صحيح z_impl_k_pipe_read(بنية k_pipe *pipe, uint8_t *معلومات, size_t len, k_timeout_t timeout)
{
	بنية pipe_buf_spec buf = { معلومات, len, 0 };
	صحيح rc;
	k_timepoint_t پڇاڙي = sys_timepoint_calc(timeout);
	k_spinlock_ڪنجي_t ڪنجي = k_spin_تالو(&pipe->تالو);
	bool need_resched = false;

	SYS_پورٽ_TRACING_OBJ_FUNC_ENTER(k_pipe, اقرأ, pipe, معلومات, len, timeout);

	إذا (unlikely(len > وڌ_کان_وڌ_عدد)) {
		rc = -EOVERFLOW;
		اذهب ٻاهريون;
	}

	إذا (unlikely(pipe_resetting(pipe))) {
		rc = -ECANCELED;
		اذهب ٻاهريون;
	}

	لكل (;;) {
		إذا (pipe_ڀريل(pipe)) {
			/* One or more pending writers may exist. */
			need_resched = z_sched_wake_all(&pipe->space, 0, عدم);
		}

		buf.استعمال_ٿيل += ring_buf_get(&pipe->buf, &معلومات[buf.استعمال_ٿيل], len - buf.استعمال_ٿيل);
		إذا (likely(buf.استعمال_ٿيل == len)) {
			rc = buf.استعمال_ٿيل;
			اكسر;
		}

		إذا (unlikely(pipe_closed(pipe))) {
			rc = buf.استعمال_ٿيل ? buf.استعمال_ٿيل : -EPIPE;
			اكسر;
		}

		rc = wait_for(&pipe->معلومات, pipe, &ڪنجي, پڇاڙي, &need_resched, &buf);
		إذا (rc != 0) {
			إذا (rc == -EAGAIN) {
				rc = buf.استعمال_ٿيل ? buf.استعمال_ٿيل : -EAGAIN;
			}
			اكسر;
		}
	}
ٻاهريون:
	SYS_پورٽ_TRACING_OBJ_FUNC_EXIT(k_pipe, اقرأ, pipe, rc);
	إذا (need_resched) {
		z_reschedule(&pipe->تالو, ڪنجي);
	} وإلا {
		k_spin_unlock(&pipe->تالو, ڪنجي);
	}
	أرجع rc;
}

فراغ z_impl_k_pipe_reset(بنية k_pipe *pipe)
{
	SYS_پورٽ_TRACING_OBJ_FUNC_ENTER(k_pipe, reset, pipe);
	K_SPINLOCK(&pipe->تالو) {
		ring_buf_reset(&pipe->buf);
		إذا (likely(pipe->انتظار != 0)) {
			pipe->flags |= PIPE_FLAG_RESET;
			z_sched_wake_all(&pipe->معلومات, 0, عدم);
			z_sched_wake_all(&pipe->space, 0, عدم);
		}
	}
	SYS_پورٽ_TRACING_OBJ_FUNC_EXIT(k_pipe, reset, pipe);
}

فراغ z_impl_k_pipe_close(بنية k_pipe *pipe)
{
	k_spinlock_ڪنجي_t ڪنجي;
	bool need_resched;

	SYS_پورٽ_TRACING_OBJ_FUNC_ENTER(k_pipe, close, pipe);

	ڪنجي = k_spin_تالو(&pipe->تالو);

	pipe->flags = 0;
	need_resched = z_sched_wake_all(&pipe->معلومات, 0, عدم);
	need_resched |= z_sched_wake_all(&pipe->space, 0, عدم);

#جيڪڏهن_وصف_ٿيل CONFIG_POLL
	/*
	 * Closing is a broadcast: wake every registered poller. Later
	 * k_poll() calls see the hangup through is_condition_met().
	 */
	طالما (z_handle_obj_poll_events(&pipe->poll_events,
					K_POLL_STATE_PIPE_معلومات_موجود)) {
		need_resched = true;
	}
#جيڪڏهن_پورو /* CONFIG_POLL */

	SYS_پورٽ_TRACING_OBJ_FUNC_EXIT(k_pipe, close, pipe);

	إذا (need_resched) {
		z_reschedule(&pipe->تالو, ڪنجي);
	} وإلا {
		k_spin_unlock(&pipe->تالو, ڪنجي);
	}
}

#جيڪڏهن_وصف_ٿيل CONFIG_USERSPACE
فراغ z_vrfy_k_pipe_init(بنية k_pipe *pipe, uint8_t *بفر, size_t بفر_سائيز)
{
	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(pipe, K_OBJ_PIPE));
	K_OOPS(K_SYSCALL_يادگيري_WRITE(بفر, بفر_سائيز));

	z_impl_k_pipe_init(pipe, بفر, بفر_سائيز);
}
#ضمن <zephyr/syscalls/k_pipe_init_mrsh.c>

صحيح z_vrfy_k_pipe_read(بنية k_pipe *pipe, uint8_t *معلومات, size_t len, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(pipe, K_OBJ_PIPE));
	K_OOPS(K_SYSCALL_يادگيري_WRITE(معلومات, len));

	أرجع z_impl_k_pipe_read(pipe, معلومات, len, timeout);
}
#ضمن <zephyr/syscalls/k_pipe_read_mrsh.c>

صحيح z_vrfy_k_pipe_write(بنية k_pipe *pipe, ثابت uint8_t *معلومات, size_t len, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(pipe, K_OBJ_PIPE));
	K_OOPS(K_SYSCALL_يادگيري_READ(معلومات, len));

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
#جيڪڏهن_پورو /* CONFIG_USERSPACE */
