/*
 * Copyright (c) 2024 Måns Ansgariusson <mansgariusson@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#అన్తర్భావ <zephyr/sys/minmax.h>
#అన్తర్భావ <zephyr/init.h>
#అన్తర్భావ <zephyr/kernel.h>
#అన్తర్భావ <zephyr/internal/syscall_handler.h>
#అన్తర్భావ <ksched.h>
#అన్తర్భావ <kthread.h>
#అన్తర్భావ <wait_q.h>
#అన్తర్భావ <scheduler.h>

#నిర్వచితమైతే CONFIG_OBJ_CORE_PIPE
K_OBJ_TYPE_DEFINE(obj_type_pipe, k_pipe, K_OBJ_TYPE_PIPE_ID, ఖాళీ);
#అయితే_ముగింపు /* CONFIG_OBJ_CORE_PIPE */

స్థిర అంతఃవరుస bool pipe_closed(నిర్మాణం k_pipe *pipe)
{
	తిరిగి (pipe->flags & PIPE_FLAG_OPEN) == 0;
}

స్థిర అంతఃవరుస bool pipe_resetting(నిర్మాణం k_pipe *pipe)
{
	తిరిగి (pipe->flags & PIPE_FLAG_RESET) != 0;
}

స్థిర అంతఃవరుస bool pipe_నిండిన(నిర్మాణం k_pipe *pipe)
{
	తిరిగి ring_buf_space_get(&pipe->buf) == 0;
}

స్థిర అంతఃవరుస bool pipe_ఖాళీలు(నిర్మాణం k_pipe *pipe)
{
	తిరిగి ring_buf_is_ఖాళీలు(&pipe->buf);
}

నిర్మాణం pipe_buf_spec {
	uint8_t * స్థిరరాశి దత్తాంశం;
	స్థిరరాశి size_t len;
	size_t వాడిన;
};

స్థిర పూర్ణాంకం wait_for(_wait_q_t *waitq, నిర్మాణం k_pipe *pipe, k_spinlock_కీలకం_t *కీలకం,
		    k_timepoint_t సమయం_హద్దు, bool *need_resched,
		    నిర్మాణం pipe_buf_spec *buf_spec)
{
	k_timeout_t timeout = sys_timepoint_timeout(సమయం_హద్దు);
	పూర్ణాంకం rc;

	ఒకవేళ (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		తిరిగి -EAGAIN;
	}

	_current->పునాది.మార్పిడి_దత్తాంశం = buf_spec;

	pipe->నిరీక్షణ++;
	*need_resched = false;
	ఒకవేళ (waitq == &pipe->space) {
		SYS_పోర్టు_TRACING_OBJ_FUNC_BLOCKING(k_pipe, లిఖ, pipe, timeout);
	} లేకపోతే {
		SYS_పోర్టు_TRACING_OBJ_FUNC_BLOCKING(k_pipe, పఠ, pipe, timeout);
	}
	rc = z_pend_curr(&pipe->తాళం, *కీలకం, waitq, timeout);
	*కీలకం = k_spin_తాళం(&pipe->తాళం);
	pipe->నిరీక్షణ--;
	ఒకవేళ (unlikely(pipe_resetting(pipe))) {
		ఒకవేళ (pipe->నిరీక్షణ == 0) {
			pipe->flags &= ~PIPE_FLAG_RESET;
		}
		rc = -ECANCELED;
	}

	తిరిగి rc;
}

శూన్య z_impl_k_pipe_init(నిర్మాణం k_pipe *pipe, uint8_t *బఫరు, size_t బఫరు_కొలత)
{
	ring_buf_init(&pipe->buf, బఫరు_కొలత, బఫరు);
	pipe->flags = PIPE_FLAG_OPEN;
	pipe->నిరీక్షణ = 0;

	pipe->తాళం = (నిర్మాణం k_spinlock){};
	z_waitq_init(&pipe->దత్తాంశం);
	z_waitq_init(&pipe->space);
	k_object_init(pipe);

#నిర్వచితమైతే CONFIG_POLL
	sys_dlist_init(&pipe->poll_events);
#అయితే_ముగింపు /* CONFIG_POLL */
#నిర్వచితమైతే CONFIG_OBJ_CORE_PIPE
	k_obj_core_init_and_link(K_OBJ_CORE(pipe), &obj_type_pipe);
#అయితే_ముగింపు /* CONFIG_OBJ_CORE_PIPE */
	SYS_పోర్టు_TRACING_OBJ_INIT(k_pipe, pipe, బఫరు, బఫరు_కొలత);
}

స్థిర size_t copy_గమ్యం_pending_readers(నిర్మాణం k_pipe *pipe, bool *need_resched,
				      స్థిరరాశి uint8_t *దత్తాంశం, size_t len)
{
	నిర్మాణం k_thread *reader = ఖాళీ;
	నిర్మాణం pipe_buf_spec *reader_buf;
	size_t copy_కొలత, written = 0;

	/*
	 * Attempt a direct data copy to waiting readers if any.
	 * The copy has to be done under the scheduler lock to ensure all the
	 * needed data is copied to the target thread whose buffer spec lives
	 * on that thread's stack, and then the thread unpended only if it
	 * received all the data it wanted, without racing with a potential
	 * thread timeout/cancellation event.
	 */
	చేయి {
		తాళం_SCHED_SPINLOCK {
			reader = priq_wait_ఉత్తమ(&pipe->దత్తాంశం.waitq);
			ఒకవేళ (reader == ఖాళీ) {
				K_SPINLOCK_BREAK;
			}

			reader_buf = reader->పునాది.మార్పిడి_దత్తాంశం;
			copy_కొలత = min(len - written,
					reader_buf->len - reader_buf->వాడిన);
			జ్ఞాపకం_నకలు(&reader_buf->దత్తాంశం[reader_buf->వాడిన],
			       &దత్తాంశం[written], copy_కొలత);
			written += copy_కొలత;
			reader_buf->వాడిన += copy_కొలత;

			ఒకవేళ (reader_buf->వాడిన < reader_buf->len) {
				/* This reader wants more: don't unpend. */
				reader = ఖాళీ;
			} లేకపోతే {
				/*
				 * This reader has received all the data
				 * it was waiting for. Set its return
				 * value, unpend, abort its timeout, and
				 * ready it, all under the scheduler lock
				 * so a racing timeout handler cannot
				 * observe a half-initialized wake-up.
				 */
				z_thread_return_విలువ_సమితి_with_దత్తాంశం(reader, 0, ఖాళీ);
				unpend_thread_no_timeout(reader);
				(శూన్య)z_try_abort_thread_timeout(reader);
				z_sched_ready_locked(reader);
				*need_resched = true;
			}
		}
	} అయితే (reader != ఖాళీ && written < len);

	తిరిగి written;
}

పూర్ణాంకం z_impl_k_pipe_write(నిర్మాణం k_pipe *pipe, స్థిరరాశి uint8_t *దత్తాంశం, size_t len, k_timeout_t timeout)
{
	పూర్ణాంకం rc;
	size_t written = 0;
	size_t added;
	k_timepoint_t చివర = sys_timepoint_calc(timeout);
	k_spinlock_కీలకం_t కీలకం = k_spin_తాళం(&pipe->తాళం);
	bool need_resched = false;

	SYS_పోర్టు_TRACING_OBJ_FUNC_ENTER(k_pipe, లిఖ, pipe, దత్తాంశం, len, timeout);

	ఒకవేళ (unlikely(len > గరిష్ఠ_పూర్ణాంకం)) {
		rc = -EOVERFLOW;
		వెళ్ళు బహిర్గతం;
	}

	ఒకవేళ (unlikely(pipe_resetting(pipe))) {
		rc = -ECANCELED;
		వెళ్ళు బహిర్గతం;
	}

	కొరకు (;;) {
		ఒకవేళ (unlikely(pipe_closed(pipe))) {
			rc = -EPIPE;
			ఆపు;
		}

		ఒకవేళ (pipe_ఖాళీలు(pipe)) {
			ఒకవేళ (IS_ENABLED(CONFIG_KERNEL_COHERENCE)) {
				/*
				 * Systems that enabled this option don't have
				 * their stacks in coherent memory. Given our
				 * pipe_buf_spec is stored on the stack, and
				 * readers may also have their destination
				 * buffer on their stack too, it is not worth
				 * supporting direct-to-readers copy with them.
				 * Simply wake up all pending readers instead.
				 */
				need_resched = z_sched_wake_all(&pipe->దత్తాంశం, 0, ఖాళీ);
			} లేకపోతే ఒకవేళ (pipe->నిరీక్షణ != 0) {
				written += copy_గమ్యం_pending_readers(pipe, &need_resched,
								   &దత్తాంశం[written],
								   len - written);
				ఒకవేళ (written >= len) {
					rc = written;
					ఆపు;
				}
			}
		}

		added = ring_buf_put(&pipe->buf, &దత్తాంశం[written], len - written);

#నిర్వచితమైతే CONFIG_POLL
		ఒకవేళ (added != 0) {
			need_resched |= z_handle_obj_poll_events(&pipe->poll_events,
								 K_POLL_STATE_PIPE_దత్తాంశం_అందుబాటు);
		}
#అయితే_ముగింపు /* CONFIG_POLL */

		written += added;
		ఒకవేళ (likely(written == len)) {
			rc = written;
			ఆపు;
		}

		rc = wait_for(&pipe->space, pipe, &కీలకం, చివర, &need_resched, ఖాళీ);
		ఒకవేళ (rc != 0) {
			ఒకవేళ (rc == -EAGAIN) {
				rc = written ? written : -EAGAIN;
			}
			ఆపు;
		}
	}
బహిర్గతం:
	SYS_పోర్టు_TRACING_OBJ_FUNC_EXIT(k_pipe, లిఖ, pipe, rc);
	ఒకవేళ (need_resched) {
		z_reschedule(&pipe->తాళం, కీలకం);
	} లేకపోతే {
		k_spin_unlock(&pipe->తాళం, కీలకం);
	}
	తిరిగి rc;
}

పూర్ణాంకం z_impl_k_pipe_read(నిర్మాణం k_pipe *pipe, uint8_t *దత్తాంశం, size_t len, k_timeout_t timeout)
{
	నిర్మాణం pipe_buf_spec buf = { దత్తాంశం, len, 0 };
	పూర్ణాంకం rc;
	k_timepoint_t చివర = sys_timepoint_calc(timeout);
	k_spinlock_కీలకం_t కీలకం = k_spin_తాళం(&pipe->తాళం);
	bool need_resched = false;

	SYS_పోర్టు_TRACING_OBJ_FUNC_ENTER(k_pipe, పఠ, pipe, దత్తాంశం, len, timeout);

	ఒకవేళ (unlikely(len > గరిష్ఠ_పూర్ణాంకం)) {
		rc = -EOVERFLOW;
		వెళ్ళు బహిర్గతం;
	}

	ఒకవేళ (unlikely(pipe_resetting(pipe))) {
		rc = -ECANCELED;
		వెళ్ళు బహిర్గతం;
	}

	కొరకు (;;) {
		ఒకవేళ (pipe_నిండిన(pipe)) {
			/* One or more pending writers may exist. */
			need_resched = z_sched_wake_all(&pipe->space, 0, ఖాళీ);
		}

		buf.వాడిన += ring_buf_get(&pipe->buf, &దత్తాంశం[buf.వాడిన], len - buf.వాడిన);
		ఒకవేళ (likely(buf.వాడిన == len)) {
			rc = buf.వాడిన;
			ఆపు;
		}

		ఒకవేళ (unlikely(pipe_closed(pipe))) {
			rc = buf.వాడిన ? buf.వాడిన : -EPIPE;
			ఆపు;
		}

		rc = wait_for(&pipe->దత్తాంశం, pipe, &కీలకం, చివర, &need_resched, &buf);
		ఒకవేళ (rc != 0) {
			ఒకవేళ (rc == -EAGAIN) {
				rc = buf.వాడిన ? buf.వాడిన : -EAGAIN;
			}
			ఆపు;
		}
	}
బహిర్గతం:
	SYS_పోర్టు_TRACING_OBJ_FUNC_EXIT(k_pipe, పఠ, pipe, rc);
	ఒకవేళ (need_resched) {
		z_reschedule(&pipe->తాళం, కీలకం);
	} లేకపోతే {
		k_spin_unlock(&pipe->తాళం, కీలకం);
	}
	తిరిగి rc;
}

శూన్య z_impl_k_pipe_reset(నిర్మాణం k_pipe *pipe)
{
	SYS_పోర్టు_TRACING_OBJ_FUNC_ENTER(k_pipe, reset, pipe);
	K_SPINLOCK(&pipe->తాళం) {
		ring_buf_reset(&pipe->buf);
		ఒకవేళ (likely(pipe->నిరీక్షణ != 0)) {
			pipe->flags |= PIPE_FLAG_RESET;
			z_sched_wake_all(&pipe->దత్తాంశం, 0, ఖాళీ);
			z_sched_wake_all(&pipe->space, 0, ఖాళీ);
		}
	}
	SYS_పోర్టు_TRACING_OBJ_FUNC_EXIT(k_pipe, reset, pipe);
}

శూన్య z_impl_k_pipe_close(నిర్మాణం k_pipe *pipe)
{
	k_spinlock_కీలకం_t కీలకం;
	bool need_resched;

	SYS_పోర్టు_TRACING_OBJ_FUNC_ENTER(k_pipe, close, pipe);

	కీలకం = k_spin_తాళం(&pipe->తాళం);

	pipe->flags = 0;
	need_resched = z_sched_wake_all(&pipe->దత్తాంశం, 0, ఖాళీ);
	need_resched |= z_sched_wake_all(&pipe->space, 0, ఖాళీ);

#నిర్వచితమైతే CONFIG_POLL
	/*
	 * Closing is a broadcast: wake every registered poller. Later
	 * k_poll() calls see the hangup through is_condition_met().
	 */
	అయితే (z_handle_obj_poll_events(&pipe->poll_events,
					K_POLL_STATE_PIPE_దత్తాంశం_అందుబాటు)) {
		need_resched = true;
	}
#అయితే_ముగింపు /* CONFIG_POLL */

	SYS_పోర్టు_TRACING_OBJ_FUNC_EXIT(k_pipe, close, pipe);

	ఒకవేళ (need_resched) {
		z_reschedule(&pipe->తాళం, కీలకం);
	} లేకపోతే {
		k_spin_unlock(&pipe->తాళం, కీలకం);
	}
}

#నిర్వచితమైతే CONFIG_USERSPACE
శూన్య z_vrfy_k_pipe_init(నిర్మాణం k_pipe *pipe, uint8_t *బఫరు, size_t బఫరు_కొలత)
{
	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(pipe, K_OBJ_PIPE));
	K_OOPS(K_SYSCALL_జ్ఞాపకం_WRITE(బఫరు, బఫరు_కొలత));

	z_impl_k_pipe_init(pipe, బఫరు, బఫరు_కొలత);
}
#అన్తర్భావ <zephyr/syscalls/k_pipe_init_mrsh.c>

పూర్ణాంకం z_vrfy_k_pipe_read(నిర్మాణం k_pipe *pipe, uint8_t *దత్తాంశం, size_t len, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(pipe, K_OBJ_PIPE));
	K_OOPS(K_SYSCALL_జ్ఞాపకం_WRITE(దత్తాంశం, len));

	తిరిగి z_impl_k_pipe_read(pipe, దత్తాంశం, len, timeout);
}
#అన్తర్భావ <zephyr/syscalls/k_pipe_read_mrsh.c>

పూర్ణాంకం z_vrfy_k_pipe_write(నిర్మాణం k_pipe *pipe, స్థిరరాశి uint8_t *దత్తాంశం, size_t len, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(pipe, K_OBJ_PIPE));
	K_OOPS(K_SYSCALL_జ్ఞాపకం_READ(దత్తాంశం, len));

	తిరిగి z_impl_k_pipe_write(pipe, దత్తాంశం, len, timeout);
}
#అన్తర్భావ <zephyr/syscalls/k_pipe_write_mrsh.c>

శూన్య z_vrfy_k_pipe_reset(నిర్మాణం k_pipe *pipe)
{
	K_OOPS(K_SYSCALL_OBJ(pipe, K_OBJ_PIPE));
	z_impl_k_pipe_reset(pipe);
}
#అన్తర్భావ <zephyr/syscalls/k_pipe_reset_mrsh.c>

శూన్య z_vrfy_k_pipe_close(నిర్మాణం k_pipe *pipe)
{
	K_OOPS(K_SYSCALL_OBJ(pipe, K_OBJ_PIPE));
	z_impl_k_pipe_close(pipe);
}
#అన్తర్భావ <zephyr/syscalls/k_pipe_close_mrsh.c>
#అయితే_ముగింపు /* CONFIG_USERSPACE */
