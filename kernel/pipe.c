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

#اگر_تعریف_شدہ CONFIG_OBJ_CORE_PIPE
K_OBJ_TYPE_DEFINE(obj_type_pipe, k_pipe, K_OBJ_TYPE_PIPE_ID, تہی);
#اگر_ختم /* CONFIG_OBJ_CORE_PIPE */

ساکن اندرونی bool pipe_closed(ساخت k_pipe *pipe)
{
	واپس (pipe->flags & PIPE_FLAG_OPEN) == 0;
}

ساکن اندرونی bool pipe_resetting(ساخت k_pipe *pipe)
{
	واپس (pipe->flags & PIPE_FLAG_RESET) != 0;
}

ساکن اندرونی bool pipe_بھرا(ساخت k_pipe *pipe)
{
	واپس ring_buf_space_get(&pipe->buf) == 0;
}

ساکن اندرونی bool pipe_خالی_خانے(ساخت k_pipe *pipe)
{
	واپس ring_buf_is_خالی_خانے(&pipe->buf);
}

ساخت pipe_buf_spec {
	uint8_t * ثابت ڈیٹا;
	ثابت size_t len;
	size_t مستعمل;
};

ساکن صحیح wait_for(_wait_q_t *waitq, ساخت k_pipe *pipe, k_spinlock_کلید_t *کلید,
		    k_timepoint_t وقت_حد_بندی, bool *need_resched,
		    ساخت pipe_buf_spec *buf_spec)
{
	k_timeout_t timeout = sys_timepoint_timeout(وقت_حد_بندی);
	صحیح rc;

	اگر (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		واپس -EAGAIN;
	}

	_current->بنیاد.تبادلہ_ڈیٹا = buf_spec;

	pipe->انتظار++;
	*need_resched = false;
	اگر (waitq == &pipe->space) {
		SYS_پورٹ_TRACING_OBJ_FUNC_BLOCKING(k_pipe, اكتب, pipe, timeout);
	} ورنہ {
		SYS_پورٹ_TRACING_OBJ_FUNC_BLOCKING(k_pipe, اقرأ, pipe, timeout);
	}
	rc = z_pend_curr(&pipe->تالا, *کلید, waitq, timeout);
	*کلید = k_spin_تالا(&pipe->تالا);
	pipe->انتظار--;
	اگر (unlikely(pipe_resetting(pipe))) {
		اگر (pipe->انتظار == 0) {
			pipe->flags &= ~PIPE_FLAG_RESET;
		}
		rc = -ECANCELED;
	}

	واپس rc;
}

خالی z_impl_k_pipe_init(ساخت k_pipe *pipe, uint8_t *بفر, size_t بفر_سائز)
{
	ring_buf_init(&pipe->buf, بفر_سائز, بفر);
	pipe->flags = PIPE_FLAG_OPEN;
	pipe->انتظار = 0;

	pipe->تالا = (ساخت k_spinlock){};
	z_waitq_init(&pipe->ڈیٹا);
	z_waitq_init(&pipe->space);
	k_object_init(pipe);

#اگر_تعریف_شدہ CONFIG_POLL
	sys_dlist_init(&pipe->poll_events);
#اگر_ختم /* CONFIG_POLL */
#اگر_تعریف_شدہ CONFIG_OBJ_CORE_PIPE
	k_obj_core_init_and_link(K_OBJ_CORE(pipe), &obj_type_pipe);
#اگر_ختم /* CONFIG_OBJ_CORE_PIPE */
	SYS_پورٹ_TRACING_OBJ_INIT(k_pipe, pipe, بفر, بفر_سائز);
}

ساکن size_t copy_مقصد_pending_readers(ساخت k_pipe *pipe, bool *need_resched,
				      ثابت uint8_t *ڈیٹا, size_t len)
{
	ساخت k_thread *reader = تہی;
	ساخت pipe_buf_spec *reader_buf;
	size_t copy_سائز, written = 0;

	/*
	 * Attempt a direct data copy to waiting readers if any.
	 * The copy has to be done under the scheduler lock to ensure all the
	 * needed data is copied to the target thread whose buffer spec lives
	 * on that thread's stack, and then the thread unpended only if it
	 * received all the data it wanted, without racing with a potential
	 * thread timeout/cancellation event.
	 */
	کرو {
		تالا_SCHED_SPINLOCK {
			reader = priq_wait_بہترین(&pipe->ڈیٹا.waitq);
			اگر (reader == تہی) {
				K_SPINLOCK_BREAK;
			}

			reader_buf = reader->بنیاد.تبادلہ_ڈیٹا;
			copy_سائز = min(len - written,
					reader_buf->len - reader_buf->مستعمل);
			حافظہ_نقل(&reader_buf->ڈیٹا[reader_buf->مستعمل],
			       &ڈیٹا[written], copy_سائز);
			written += copy_سائز;
			reader_buf->مستعمل += copy_سائز;

			اگر (reader_buf->مستعمل < reader_buf->len) {
				/* This reader wants more: don't unpend. */
				reader = تہی;
			} ورنہ {
				/*
				 * This reader has received all the data
				 * it was waiting for. Set its return
				 * value, unpend, abort its timeout, and
				 * ready it, all under the scheduler lock
				 * so a racing timeout handler cannot
				 * observe a half-initialized wake-up.
				 */
				z_thread_return_قدر_سیٹ_with_ڈیٹا(reader, 0, تہی);
				unpend_thread_no_timeout(reader);
				(خالی)z_try_abort_thread_timeout(reader);
				z_sched_ready_locked(reader);
				*need_resched = true;
			}
		}
	} جب (reader != تہی && written < len);

	واپس written;
}

صحیح z_impl_k_pipe_write(ساخت k_pipe *pipe, ثابت uint8_t *ڈیٹا, size_t len, k_timeout_t timeout)
{
	صحیح rc;
	size_t written = 0;
	size_t added;
	k_timepoint_t انتہا = sys_timepoint_calc(timeout);
	k_spinlock_کلید_t کلید = k_spin_تالا(&pipe->تالا);
	bool need_resched = false;

	SYS_پورٹ_TRACING_OBJ_FUNC_ENTER(k_pipe, اكتب, pipe, ڈیٹا, len, timeout);

	اگر (unlikely(len > زیادہ_سے_زیادہ_عدد)) {
		rc = -EOVERFLOW;
		جاؤ برآمد;
	}

	اگر (unlikely(pipe_resetting(pipe))) {
		rc = -ECANCELED;
		جاؤ برآمد;
	}

	برائے (;;) {
		اگر (unlikely(pipe_closed(pipe))) {
			rc = -EPIPE;
			توڑو;
		}

		اگر (pipe_خالی_خانے(pipe)) {
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
				need_resched = z_sched_wake_all(&pipe->ڈیٹا, 0, تہی);
			} ورنہ اگر (pipe->انتظار != 0) {
				written += copy_مقصد_pending_readers(pipe, &need_resched,
								   &ڈیٹا[written],
								   len - written);
				اگر (written >= len) {
					rc = written;
					توڑو;
				}
			}
		}

		added = ring_buf_put(&pipe->buf, &ڈیٹا[written], len - written);

#اگر_تعریف_شدہ CONFIG_POLL
		اگر (added != 0) {
			need_resched |= z_handle_obj_poll_events(&pipe->poll_events,
								 K_POLL_STATE_PIPE_ڈیٹا_دستیاب);
		}
#اگر_ختم /* CONFIG_POLL */

		written += added;
		اگر (likely(written == len)) {
			rc = written;
			توڑو;
		}

		rc = wait_for(&pipe->space, pipe, &کلید, انتہا, &need_resched, تہی);
		اگر (rc != 0) {
			اگر (rc == -EAGAIN) {
				rc = written ? written : -EAGAIN;
			}
			توڑو;
		}
	}
برآمد:
	SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_pipe, اكتب, pipe, rc);
	اگر (need_resched) {
		z_reschedule(&pipe->تالا, کلید);
	} ورنہ {
		k_spin_unlock(&pipe->تالا, کلید);
	}
	واپس rc;
}

صحیح z_impl_k_pipe_read(ساخت k_pipe *pipe, uint8_t *ڈیٹا, size_t len, k_timeout_t timeout)
{
	ساخت pipe_buf_spec buf = { ڈیٹا, len, 0 };
	صحیح rc;
	k_timepoint_t انتہا = sys_timepoint_calc(timeout);
	k_spinlock_کلید_t کلید = k_spin_تالا(&pipe->تالا);
	bool need_resched = false;

	SYS_پورٹ_TRACING_OBJ_FUNC_ENTER(k_pipe, اقرأ, pipe, ڈیٹا, len, timeout);

	اگر (unlikely(len > زیادہ_سے_زیادہ_عدد)) {
		rc = -EOVERFLOW;
		جاؤ برآمد;
	}

	اگر (unlikely(pipe_resetting(pipe))) {
		rc = -ECANCELED;
		جاؤ برآمد;
	}

	برائے (;;) {
		اگر (pipe_بھرا(pipe)) {
			/* One or more pending writers may exist. */
			need_resched = z_sched_wake_all(&pipe->space, 0, تہی);
		}

		buf.مستعمل += ring_buf_get(&pipe->buf, &ڈیٹا[buf.مستعمل], len - buf.مستعمل);
		اگر (likely(buf.مستعمل == len)) {
			rc = buf.مستعمل;
			توڑو;
		}

		اگر (unlikely(pipe_closed(pipe))) {
			rc = buf.مستعمل ? buf.مستعمل : -EPIPE;
			توڑو;
		}

		rc = wait_for(&pipe->ڈیٹا, pipe, &کلید, انتہا, &need_resched, &buf);
		اگر (rc != 0) {
			اگر (rc == -EAGAIN) {
				rc = buf.مستعمل ? buf.مستعمل : -EAGAIN;
			}
			توڑو;
		}
	}
برآمد:
	SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_pipe, اقرأ, pipe, rc);
	اگر (need_resched) {
		z_reschedule(&pipe->تالا, کلید);
	} ورنہ {
		k_spin_unlock(&pipe->تالا, کلید);
	}
	واپس rc;
}

خالی z_impl_k_pipe_reset(ساخت k_pipe *pipe)
{
	SYS_پورٹ_TRACING_OBJ_FUNC_ENTER(k_pipe, reset, pipe);
	K_SPINLOCK(&pipe->تالا) {
		ring_buf_reset(&pipe->buf);
		اگر (likely(pipe->انتظار != 0)) {
			pipe->flags |= PIPE_FLAG_RESET;
			z_sched_wake_all(&pipe->ڈیٹا, 0, تہی);
			z_sched_wake_all(&pipe->space, 0, تہی);
		}
	}
	SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_pipe, reset, pipe);
}

خالی z_impl_k_pipe_close(ساخت k_pipe *pipe)
{
	k_spinlock_کلید_t کلید;
	bool need_resched;

	SYS_پورٹ_TRACING_OBJ_FUNC_ENTER(k_pipe, close, pipe);

	کلید = k_spin_تالا(&pipe->تالا);

	pipe->flags = 0;
	need_resched = z_sched_wake_all(&pipe->ڈیٹا, 0, تہی);
	need_resched |= z_sched_wake_all(&pipe->space, 0, تہی);

#اگر_تعریف_شدہ CONFIG_POLL
	/*
	 * Closing is a broadcast: wake every registered poller. Later
	 * k_poll() calls see the hangup through is_condition_met().
	 */
	جب (z_handle_obj_poll_events(&pipe->poll_events,
					K_POLL_STATE_PIPE_ڈیٹا_دستیاب)) {
		need_resched = true;
	}
#اگر_ختم /* CONFIG_POLL */

	SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_pipe, close, pipe);

	اگر (need_resched) {
		z_reschedule(&pipe->تالا, کلید);
	} ورنہ {
		k_spin_unlock(&pipe->تالا, کلید);
	}
}

#اگر_تعریف_شدہ CONFIG_USERSPACE
خالی z_vrfy_k_pipe_init(ساخت k_pipe *pipe, uint8_t *بفر, size_t بفر_سائز)
{
	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(pipe, K_OBJ_PIPE));
	K_OOPS(K_SYSCALL_حافظہ_WRITE(بفر, بفر_سائز));

	z_impl_k_pipe_init(pipe, بفر, بفر_سائز);
}
#شامل <zephyr/syscalls/k_pipe_init_mrsh.c>

صحیح z_vrfy_k_pipe_read(ساخت k_pipe *pipe, uint8_t *ڈیٹا, size_t len, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(pipe, K_OBJ_PIPE));
	K_OOPS(K_SYSCALL_حافظہ_WRITE(ڈیٹا, len));

	واپس z_impl_k_pipe_read(pipe, ڈیٹا, len, timeout);
}
#شامل <zephyr/syscalls/k_pipe_read_mrsh.c>

صحیح z_vrfy_k_pipe_write(ساخت k_pipe *pipe, ثابت uint8_t *ڈیٹا, size_t len, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(pipe, K_OBJ_PIPE));
	K_OOPS(K_SYSCALL_حافظہ_READ(ڈیٹا, len));

	واپس z_impl_k_pipe_write(pipe, ڈیٹا, len, timeout);
}
#شامل <zephyr/syscalls/k_pipe_write_mrsh.c>

خالی z_vrfy_k_pipe_reset(ساخت k_pipe *pipe)
{
	K_OOPS(K_SYSCALL_OBJ(pipe, K_OBJ_PIPE));
	z_impl_k_pipe_reset(pipe);
}
#شامل <zephyr/syscalls/k_pipe_reset_mrsh.c>

خالی z_vrfy_k_pipe_close(ساخت k_pipe *pipe)
{
	K_OOPS(K_SYSCALL_OBJ(pipe, K_OBJ_PIPE));
	z_impl_k_pipe_close(pipe);
}
#شامل <zephyr/syscalls/k_pipe_close_mrsh.c>
#اگر_ختم /* CONFIG_USERSPACE */
