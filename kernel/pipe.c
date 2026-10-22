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

#جے_تعریف_شدہ CONFIG_OBJ_CORE_PIPE
K_OBJ_TYPE_DEFINE(obj_type_pipe, k_pipe, K_OBJ_TYPE_PIPE_ID, عدم);
#جے_انت /* CONFIG_OBJ_CORE_PIPE */

ساكن مضمن bool pipe_closed(بنية k_pipe *pipe)
{
	أرجع (pipe->flags & PIPE_FLAG_OPEN) == 0;
}

ساكن مضمن bool pipe_resetting(بنية k_pipe *pipe)
{
	أرجع (pipe->flags & PIPE_FLAG_RESET) != 0;
}

ساكن مضمن bool pipe_بھریا(بنية k_pipe *pipe)
{
	أرجع ring_buf_space_get(&pipe->buf) == 0;
}

ساكن مضمن bool pipe_خالی_تھاواں(بنية k_pipe *pipe)
{
	أرجع ring_buf_is_خالی_تھاواں(&pipe->buf);
}

بنية pipe_buf_spec {
	uint8_t * ثابت انکڑے;
	ثابت size_t len;
	size_t ورتیا;
};

ساكن صحيح wait_for(_wait_q_t *waitq, بنية k_pipe *pipe, k_spinlock_کلید_t *کلید,
		    k_timepoint_t ویلا_حد_بندی, bool *need_resched,
		    بنية pipe_buf_spec *buf_spec)
{
	k_timeout_t timeout = sys_timepoint_timeout(ویلا_حد_بندی);
	صحيح rc;

	إذا (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		أرجع -EAGAIN;
	}

	_current->ادھار.وٹاندرا_انکڑے = buf_spec;

	pipe->اڈیک++;
	*need_resched = false;
	إذا (waitq == &pipe->space) {
		SYS_پورٹ_TRACING_OBJ_FUNC_BLOCKING(k_pipe, اكتب, pipe, timeout);
	} وإلا {
		SYS_پورٹ_TRACING_OBJ_FUNC_BLOCKING(k_pipe, اقرأ, pipe, timeout);
	}
	rc = z_pend_curr(&pipe->جندرا, *کلید, waitq, timeout);
	*کلید = k_spin_جندرا(&pipe->جندرا);
	pipe->اڈیک--;
	إذا (unlikely(pipe_resetting(pipe))) {
		إذا (pipe->اڈیک == 0) {
			pipe->flags &= ~PIPE_FLAG_RESET;
		}
		rc = -ECANCELED;
	}

	أرجع rc;
}

فراغ z_impl_k_pipe_init(بنية k_pipe *pipe, uint8_t *بفر, size_t بفر_ناپ)
{
	ring_buf_init(&pipe->buf, بفر_ناپ, بفر);
	pipe->flags = PIPE_FLAG_OPEN;
	pipe->اڈیک = 0;

	pipe->جندرا = (بنية k_spinlock){};
	z_waitq_init(&pipe->انکڑے);
	z_waitq_init(&pipe->space);
	k_object_init(pipe);

#جے_تعریف_شدہ CONFIG_POLL
	sys_dlist_init(&pipe->poll_events);
#جے_انت /* CONFIG_POLL */
#جے_تعریف_شدہ CONFIG_OBJ_CORE_PIPE
	k_obj_core_init_and_link(K_OBJ_CORE(pipe), &obj_type_pipe);
#جے_انت /* CONFIG_OBJ_CORE_PIPE */
	SYS_پورٹ_TRACING_OBJ_INIT(k_pipe, pipe, بفر, بفر_ناپ);
}

ساكن size_t copy_منزل_pending_readers(بنية k_pipe *pipe, bool *need_resched,
				      ثابت uint8_t *انکڑے, size_t len)
{
	بنية k_thread *reader = عدم;
	بنية pipe_buf_spec *reader_buf;
	size_t copy_ناپ, written = 0;

	/*
	 * Attempt a direct data copy to waiting readers if any.
	 * The copy has to be done under the scheduler lock to ensure all the
	 * needed data is copied to the target thread whose buffer spec lives
	 * on that thread's stack, and then the thread unpended only if it
	 * received all the data it wanted, without racing with a potential
	 * thread timeout/cancellation event.
	 */
	افعل {
		جندرا_SCHED_SPINLOCK {
			reader = priq_wait_بہترین(&pipe->انکڑے.waitq);
			إذا (reader == عدم) {
				K_SPINLOCK_BREAK;
			}

			reader_buf = reader->ادھار.وٹاندرا_انکڑے;
			copy_ناپ = min(len - written,
					reader_buf->len - reader_buf->ورتیا);
			حافظہ_نقل(&reader_buf->انکڑے[reader_buf->ورتیا],
			       &انکڑے[written], copy_ناپ);
			written += copy_ناپ;
			reader_buf->ورتیا += copy_ناپ;

			إذا (reader_buf->ورتیا < reader_buf->len) {
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
				z_thread_return_قدر_سیٹ_with_انکڑے(reader, 0, عدم);
				unpend_thread_no_timeout(reader);
				(فراغ)z_try_abort_thread_timeout(reader);
				z_sched_ready_locked(reader);
				*need_resched = true;
			}
		}
	} طالما (reader != عدم && written < len);

	أرجع written;
}

صحيح z_impl_k_pipe_write(بنية k_pipe *pipe, ثابت uint8_t *انکڑے, size_t len, k_timeout_t timeout)
{
	صحيح rc;
	size_t written = 0;
	size_t added;
	k_timepoint_t سرا = sys_timepoint_calc(timeout);
	k_spinlock_کلید_t کلید = k_spin_جندرا(&pipe->جندرا);
	bool need_resched = false;

	SYS_پورٹ_TRACING_OBJ_FUNC_ENTER(k_pipe, اكتب, pipe, انکڑے, len, timeout);

	إذا (unlikely(len > ودھ_توں_ودھ_عدد)) {
		rc = -EOVERFLOW;
		اذهب باہرلا;
	}

	إذا (unlikely(pipe_resetting(pipe))) {
		rc = -ECANCELED;
		اذهب باہرلا;
	}

	لكل (;;) {
		إذا (unlikely(pipe_closed(pipe))) {
			rc = -EPIPE;
			اكسر;
		}

		إذا (pipe_خالی_تھاواں(pipe)) {
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
				need_resched = z_sched_wake_all(&pipe->انکڑے, 0, عدم);
			} وإلا إذا (pipe->اڈیک != 0) {
				written += copy_منزل_pending_readers(pipe, &need_resched,
								   &انکڑے[written],
								   len - written);
				إذا (written >= len) {
					rc = written;
					اكسر;
				}
			}
		}

		added = ring_buf_put(&pipe->buf, &انکڑے[written], len - written);

#جے_تعریف_شدہ CONFIG_POLL
		إذا (added != 0) {
			need_resched |= z_handle_obj_poll_events(&pipe->poll_events,
								 K_POLL_STATE_PIPE_انکڑے_دستیاب);
		}
#جے_انت /* CONFIG_POLL */

		written += added;
		إذا (likely(written == len)) {
			rc = written;
			اكسر;
		}

		rc = wait_for(&pipe->space, pipe, &کلید, سرا, &need_resched, عدم);
		إذا (rc != 0) {
			إذا (rc == -EAGAIN) {
				rc = written ? written : -EAGAIN;
			}
			اكسر;
		}
	}
باہرلا:
	SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_pipe, اكتب, pipe, rc);
	إذا (need_resched) {
		z_reschedule(&pipe->جندرا, کلید);
	} وإلا {
		k_spin_unlock(&pipe->جندرا, کلید);
	}
	أرجع rc;
}

صحيح z_impl_k_pipe_read(بنية k_pipe *pipe, uint8_t *انکڑے, size_t len, k_timeout_t timeout)
{
	بنية pipe_buf_spec buf = { انکڑے, len, 0 };
	صحيح rc;
	k_timepoint_t سرا = sys_timepoint_calc(timeout);
	k_spinlock_کلید_t کلید = k_spin_جندرا(&pipe->جندرا);
	bool need_resched = false;

	SYS_پورٹ_TRACING_OBJ_FUNC_ENTER(k_pipe, اقرأ, pipe, انکڑے, len, timeout);

	إذا (unlikely(len > ودھ_توں_ودھ_عدد)) {
		rc = -EOVERFLOW;
		اذهب باہرلا;
	}

	إذا (unlikely(pipe_resetting(pipe))) {
		rc = -ECANCELED;
		اذهب باہرلا;
	}

	لكل (;;) {
		إذا (pipe_بھریا(pipe)) {
			/* One or more pending writers may exist. */
			need_resched = z_sched_wake_all(&pipe->space, 0, عدم);
		}

		buf.ورتیا += ring_buf_get(&pipe->buf, &انکڑے[buf.ورتیا], len - buf.ورتیا);
		إذا (likely(buf.ورتیا == len)) {
			rc = buf.ورتیا;
			اكسر;
		}

		إذا (unlikely(pipe_closed(pipe))) {
			rc = buf.ورتیا ? buf.ورتیا : -EPIPE;
			اكسر;
		}

		rc = wait_for(&pipe->انکڑے, pipe, &کلید, سرا, &need_resched, &buf);
		إذا (rc != 0) {
			إذا (rc == -EAGAIN) {
				rc = buf.ورتیا ? buf.ورتیا : -EAGAIN;
			}
			اكسر;
		}
	}
باہرلا:
	SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_pipe, اقرأ, pipe, rc);
	إذا (need_resched) {
		z_reschedule(&pipe->جندرا, کلید);
	} وإلا {
		k_spin_unlock(&pipe->جندرا, کلید);
	}
	أرجع rc;
}

فراغ z_impl_k_pipe_reset(بنية k_pipe *pipe)
{
	SYS_پورٹ_TRACING_OBJ_FUNC_ENTER(k_pipe, reset, pipe);
	K_SPINLOCK(&pipe->جندرا) {
		ring_buf_reset(&pipe->buf);
		إذا (likely(pipe->اڈیک != 0)) {
			pipe->flags |= PIPE_FLAG_RESET;
			z_sched_wake_all(&pipe->انکڑے, 0, عدم);
			z_sched_wake_all(&pipe->space, 0, عدم);
		}
	}
	SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_pipe, reset, pipe);
}

فراغ z_impl_k_pipe_close(بنية k_pipe *pipe)
{
	k_spinlock_کلید_t کلید;
	bool need_resched;

	SYS_پورٹ_TRACING_OBJ_FUNC_ENTER(k_pipe, close, pipe);

	کلید = k_spin_جندرا(&pipe->جندرا);

	pipe->flags = 0;
	need_resched = z_sched_wake_all(&pipe->انکڑے, 0, عدم);
	need_resched |= z_sched_wake_all(&pipe->space, 0, عدم);

#جے_تعریف_شدہ CONFIG_POLL
	/*
	 * Closing is a broadcast: wake every registered poller. Later
	 * k_poll() calls see the hangup through is_condition_met().
	 */
	طالما (z_handle_obj_poll_events(&pipe->poll_events,
					K_POLL_STATE_PIPE_انکڑے_دستیاب)) {
		need_resched = true;
	}
#جے_انت /* CONFIG_POLL */

	SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_pipe, close, pipe);

	إذا (need_resched) {
		z_reschedule(&pipe->جندرا, کلید);
	} وإلا {
		k_spin_unlock(&pipe->جندرا, کلید);
	}
}

#جے_تعریف_شدہ CONFIG_USERSPACE
فراغ z_vrfy_k_pipe_init(بنية k_pipe *pipe, uint8_t *بفر, size_t بفر_ناپ)
{
	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(pipe, K_OBJ_PIPE));
	K_OOPS(K_SYSCALL_حافظہ_WRITE(بفر, بفر_ناپ));

	z_impl_k_pipe_init(pipe, بفر, بفر_ناپ);
}
#ضمن <zephyr/syscalls/k_pipe_init_mrsh.c>

صحيح z_vrfy_k_pipe_read(بنية k_pipe *pipe, uint8_t *انکڑے, size_t len, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(pipe, K_OBJ_PIPE));
	K_OOPS(K_SYSCALL_حافظہ_WRITE(انکڑے, len));

	أرجع z_impl_k_pipe_read(pipe, انکڑے, len, timeout);
}
#ضمن <zephyr/syscalls/k_pipe_read_mrsh.c>

صحيح z_vrfy_k_pipe_write(بنية k_pipe *pipe, ثابت uint8_t *انکڑے, size_t len, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(pipe, K_OBJ_PIPE));
	K_OOPS(K_SYSCALL_حافظہ_READ(انکڑے, len));

	أرجع z_impl_k_pipe_write(pipe, انکڑے, len, timeout);
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
#جے_انت /* CONFIG_USERSPACE */
