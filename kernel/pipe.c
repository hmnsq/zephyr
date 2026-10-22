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

#اگر_تعریف_شده CONFIG_OBJ_CORE_PIPE
K_OBJ_TYPE_DEFINE(obj_type_pipe, k_pipe, K_OBJ_TYPE_PIPE_ID, عدم);
#پایان_اگر /* CONFIG_OBJ_CORE_PIPE */

ساكن مضمن bool pipe_closed(بنية k_pipe *pipe)
{
	أرجع (pipe->flags & PIPE_FLAG_OPEN) == 0;
}

ساكن مضمن bool pipe_resetting(بنية k_pipe *pipe)
{
	أرجع (pipe->flags & PIPE_FLAG_RESET) != 0;
}

ساكن مضمن bool pipe_پر(بنية k_pipe *pipe)
{
	أرجع ring_buf_space_get(&pipe->buf) == 0;
}

ساكن مضمن bool pipe_خالی(بنية k_pipe *pipe)
{
	أرجع ring_buf_is_خالی(&pipe->buf);
}

بنية pipe_buf_spec {
	uint8_t * ثابت داده;
	ثابت size_t len;
	size_t استفاده_شده;
};

ساكن صحيح wait_for(_wait_q_t *waitq, بنية k_pipe *pipe, k_spinlock_کلید_t *کلید,
		    k_timepoint_t زمان_حد, bool *need_resched,
		    بنية pipe_buf_spec *buf_spec)
{
	k_timeout_t timeout = sys_timepoint_timeout(زمان_حد);
	صحيح rc;

	إذا (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		أرجع -EAGAIN;
	}

	_current->پایه.جابجا_داده = buf_spec;

	pipe->انتظار++;
	*need_resched = false;
	إذا (waitq == &pipe->space) {
		SYS_پورت_TRACING_OBJ_FUNC_BLOCKING(k_pipe, اكتب, pipe, timeout);
	} وإلا {
		SYS_پورت_TRACING_OBJ_FUNC_BLOCKING(k_pipe, اقرأ, pipe, timeout);
	}
	rc = z_pend_curr(&pipe->قفل, *کلید, waitq, timeout);
	*کلید = k_spin_قفل(&pipe->قفل);
	pipe->انتظار--;
	إذا (unlikely(pipe_resetting(pipe))) {
		إذا (pipe->انتظار == 0) {
			pipe->flags &= ~PIPE_FLAG_RESET;
		}
		rc = -ECANCELED;
	}

	أرجع rc;
}

فراغ z_impl_k_pipe_init(بنية k_pipe *pipe, uint8_t *میانگیر, size_t میانگیر_اندازه)
{
	ring_buf_init(&pipe->buf, میانگیر_اندازه, میانگیر);
	pipe->flags = PIPE_FLAG_OPEN;
	pipe->انتظار = 0;

	pipe->قفل = (بنية k_spinlock){};
	z_waitq_init(&pipe->داده);
	z_waitq_init(&pipe->space);
	k_object_init(pipe);

#اگر_تعریف_شده CONFIG_POLL
	sys_dlist_init(&pipe->poll_events);
#پایان_اگر /* CONFIG_POLL */
#اگر_تعریف_شده CONFIG_OBJ_CORE_PIPE
	k_obj_core_init_and_link(K_OBJ_CORE(pipe), &obj_type_pipe);
#پایان_اگر /* CONFIG_OBJ_CORE_PIPE */
	SYS_پورت_TRACING_OBJ_INIT(k_pipe, pipe, میانگیر, میانگیر_اندازه);
}

ساكن size_t copy_مقصد_pending_readers(بنية k_pipe *pipe, bool *need_resched,
				      ثابت uint8_t *داده, size_t len)
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
		قفل_SCHED_SPINLOCK {
			reader = priq_wait_بهترین(&pipe->داده.waitq);
			إذا (reader == عدم) {
				K_SPINLOCK_BREAK;
			}

			reader_buf = reader->پایه.جابجا_داده;
			copy_اندازه = min(len - written,
					reader_buf->len - reader_buf->استفاده_شده);
			کپی_حافظه(&reader_buf->داده[reader_buf->استفاده_شده],
			       &داده[written], copy_اندازه);
			written += copy_اندازه;
			reader_buf->استفاده_شده += copy_اندازه;

			إذا (reader_buf->استفاده_شده < reader_buf->len) {
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
				z_thread_return_مقدار_مجموعه_with_داده(reader, 0, عدم);
				unpend_thread_no_timeout(reader);
				(فراغ)z_try_abort_thread_timeout(reader);
				z_sched_ready_locked(reader);
				*need_resched = true;
			}
		}
	} طالما (reader != عدم && written < len);

	أرجع written;
}

صحيح z_impl_k_pipe_write(بنية k_pipe *pipe, ثابت uint8_t *داده, size_t len, k_timeout_t timeout)
{
	صحيح rc;
	size_t written = 0;
	size_t added;
	k_timepoint_t انتها = sys_timepoint_calc(timeout);
	k_spinlock_کلید_t کلید = k_spin_قفل(&pipe->قفل);
	bool need_resched = false;

	SYS_پورت_TRACING_OBJ_FUNC_ENTER(k_pipe, اكتب, pipe, داده, len, timeout);

	إذا (unlikely(len > بیشترین_صحیح)) {
		rc = -EOVERFLOW;
		اذهب بیرون;
	}

	إذا (unlikely(pipe_resetting(pipe))) {
		rc = -ECANCELED;
		اذهب بیرون;
	}

	لكل (;;) {
		إذا (unlikely(pipe_closed(pipe))) {
			rc = -EPIPE;
			اكسر;
		}

		إذا (pipe_خالی(pipe)) {
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
				need_resched = z_sched_wake_all(&pipe->داده, 0, عدم);
			} وإلا إذا (pipe->انتظار != 0) {
				written += copy_مقصد_pending_readers(pipe, &need_resched,
								   &داده[written],
								   len - written);
				إذا (written >= len) {
					rc = written;
					اكسر;
				}
			}
		}

		added = ring_buf_put(&pipe->buf, &داده[written], len - written);

#اگر_تعریف_شده CONFIG_POLL
		إذا (added != 0) {
			need_resched |= z_handle_obj_poll_events(&pipe->poll_events,
								 K_POLL_STATE_PIPE_داده_موجود);
		}
#پایان_اگر /* CONFIG_POLL */

		written += added;
		إذا (likely(written == len)) {
			rc = written;
			اكسر;
		}

		rc = wait_for(&pipe->space, pipe, &کلید, انتها, &need_resched, عدم);
		إذا (rc != 0) {
			إذا (rc == -EAGAIN) {
				rc = written ? written : -EAGAIN;
			}
			اكسر;
		}
	}
بیرون:
	SYS_پورت_TRACING_OBJ_FUNC_EXIT(k_pipe, اكتب, pipe, rc);
	إذا (need_resched) {
		z_reschedule(&pipe->قفل, کلید);
	} وإلا {
		k_spin_unlock(&pipe->قفل, کلید);
	}
	أرجع rc;
}

صحيح z_impl_k_pipe_read(بنية k_pipe *pipe, uint8_t *داده, size_t len, k_timeout_t timeout)
{
	بنية pipe_buf_spec buf = { داده, len, 0 };
	صحيح rc;
	k_timepoint_t انتها = sys_timepoint_calc(timeout);
	k_spinlock_کلید_t کلید = k_spin_قفل(&pipe->قفل);
	bool need_resched = false;

	SYS_پورت_TRACING_OBJ_FUNC_ENTER(k_pipe, اقرأ, pipe, داده, len, timeout);

	إذا (unlikely(len > بیشترین_صحیح)) {
		rc = -EOVERFLOW;
		اذهب بیرون;
	}

	إذا (unlikely(pipe_resetting(pipe))) {
		rc = -ECANCELED;
		اذهب بیرون;
	}

	لكل (;;) {
		إذا (pipe_پر(pipe)) {
			/* One or more pending writers may exist. */
			need_resched = z_sched_wake_all(&pipe->space, 0, عدم);
		}

		buf.استفاده_شده += ring_buf_get(&pipe->buf, &داده[buf.استفاده_شده], len - buf.استفاده_شده);
		إذا (likely(buf.استفاده_شده == len)) {
			rc = buf.استفاده_شده;
			اكسر;
		}

		إذا (unlikely(pipe_closed(pipe))) {
			rc = buf.استفاده_شده ? buf.استفاده_شده : -EPIPE;
			اكسر;
		}

		rc = wait_for(&pipe->داده, pipe, &کلید, انتها, &need_resched, &buf);
		إذا (rc != 0) {
			إذا (rc == -EAGAIN) {
				rc = buf.استفاده_شده ? buf.استفاده_شده : -EAGAIN;
			}
			اكسر;
		}
	}
بیرون:
	SYS_پورت_TRACING_OBJ_FUNC_EXIT(k_pipe, اقرأ, pipe, rc);
	إذا (need_resched) {
		z_reschedule(&pipe->قفل, کلید);
	} وإلا {
		k_spin_unlock(&pipe->قفل, کلید);
	}
	أرجع rc;
}

فراغ z_impl_k_pipe_reset(بنية k_pipe *pipe)
{
	SYS_پورت_TRACING_OBJ_FUNC_ENTER(k_pipe, reset, pipe);
	K_SPINLOCK(&pipe->قفل) {
		ring_buf_reset(&pipe->buf);
		إذا (likely(pipe->انتظار != 0)) {
			pipe->flags |= PIPE_FLAG_RESET;
			z_sched_wake_all(&pipe->داده, 0, عدم);
			z_sched_wake_all(&pipe->space, 0, عدم);
		}
	}
	SYS_پورت_TRACING_OBJ_FUNC_EXIT(k_pipe, reset, pipe);
}

فراغ z_impl_k_pipe_close(بنية k_pipe *pipe)
{
	k_spinlock_کلید_t کلید;
	bool need_resched;

	SYS_پورت_TRACING_OBJ_FUNC_ENTER(k_pipe, close, pipe);

	کلید = k_spin_قفل(&pipe->قفل);

	pipe->flags = 0;
	need_resched = z_sched_wake_all(&pipe->داده, 0, عدم);
	need_resched |= z_sched_wake_all(&pipe->space, 0, عدم);

#اگر_تعریف_شده CONFIG_POLL
	/*
	 * Closing is a broadcast: wake every registered poller. Later
	 * k_poll() calls see the hangup through is_condition_met().
	 */
	طالما (z_handle_obj_poll_events(&pipe->poll_events,
					K_POLL_STATE_PIPE_داده_موجود)) {
		need_resched = true;
	}
#پایان_اگر /* CONFIG_POLL */

	SYS_پورت_TRACING_OBJ_FUNC_EXIT(k_pipe, close, pipe);

	إذا (need_resched) {
		z_reschedule(&pipe->قفل, کلید);
	} وإلا {
		k_spin_unlock(&pipe->قفل, کلید);
	}
}

#اگر_تعریف_شده CONFIG_USERSPACE
فراغ z_vrfy_k_pipe_init(بنية k_pipe *pipe, uint8_t *میانگیر, size_t میانگیر_اندازه)
{
	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(pipe, K_OBJ_PIPE));
	K_OOPS(K_SYSCALL_حافظه_WRITE(میانگیر, میانگیر_اندازه));

	z_impl_k_pipe_init(pipe, میانگیر, میانگیر_اندازه);
}
#ضمن <zephyr/syscalls/k_pipe_init_mrsh.c>

صحيح z_vrfy_k_pipe_read(بنية k_pipe *pipe, uint8_t *داده, size_t len, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(pipe, K_OBJ_PIPE));
	K_OOPS(K_SYSCALL_حافظه_WRITE(داده, len));

	أرجع z_impl_k_pipe_read(pipe, داده, len, timeout);
}
#ضمن <zephyr/syscalls/k_pipe_read_mrsh.c>

صحيح z_vrfy_k_pipe_write(بنية k_pipe *pipe, ثابت uint8_t *داده, size_t len, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(pipe, K_OBJ_PIPE));
	K_OOPS(K_SYSCALL_حافظه_READ(داده, len));

	أرجع z_impl_k_pipe_write(pipe, داده, len, timeout);
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
#پایان_اگر /* CONFIG_USERSPACE */
