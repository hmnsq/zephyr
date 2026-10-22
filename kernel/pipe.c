/*
 * Copyright (c) 2024 Måns Ansgariusson <mansgariusson@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ಅನ್ತರ್ಭಾವ <zephyr/sys/minmax.h>
#ಅನ್ತರ್ಭಾವ <zephyr/init.h>
#ಅನ್ತರ್ಭಾವ <zephyr/kernel.h>
#ಅನ್ತರ್ಭಾವ <zephyr/internal/syscall_handler.h>
#ಅನ್ತರ್ಭಾವ <ksched.h>
#ಅನ್ತರ್ಭಾವ <kthread.h>
#ಅನ್ತರ್ಭಾವ <wait_q.h>
#ಅನ್ತರ್ಭಾವ <scheduler.h>

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_OBJ_CORE_PIPE
K_OBJ_TYPE_DEFINE(obj_type_pipe, k_pipe, K_OBJ_TYPE_PIPE_ID, ಖಾಲಿ);
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_OBJ_CORE_PIPE */

ಸ್ಥಿರ ಸಾಲೊಳಗೆ bool pipe_closed(ರಚನೆ k_pipe *pipe)
{
	ಹಿಂತಿರುಗಿ (pipe->flags & PIPE_FLAG_OPEN) == 0;
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ bool pipe_resetting(ರಚನೆ k_pipe *pipe)
{
	ಹಿಂತಿರುಗಿ (pipe->flags & PIPE_FLAG_RESET) != 0;
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ bool pipe_ತುಂಬಿದ(ರಚನೆ k_pipe *pipe)
{
	ಹಿಂತಿರುಗಿ ring_buf_space_get(&pipe->buf) == 0;
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ bool pipe_ಖಾಲಿಗಳು(ರಚನೆ k_pipe *pipe)
{
	ಹಿಂತಿರುಗಿ ring_buf_is_ಖಾಲಿಗಳು(&pipe->buf);
}

ರಚನೆ pipe_buf_spec {
	uint8_t * ಸ್ಥಿರಾಂಕ ದತ್ತಾಂಶ;
	ಸ್ಥಿರಾಂಕ size_t len;
	size_t ಬಳಸಿದ;
};

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ wait_for(_wait_q_t *waitq, ರಚನೆ k_pipe *pipe, k_spinlock_ಕೀಲಿ_t *ಕೀಲಿ,
		    k_timepoint_t ಸಮಯ_ಎಲ್ಲೆ, bool *need_resched,
		    ರಚನೆ pipe_buf_spec *buf_spec)
{
	k_timeout_t timeout = sys_timepoint_timeout(ಸಮಯ_ಎಲ್ಲೆ);
	ಪೂರ್ಣಾಂಕ rc;

	ಆದರೆ (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		ಹಿಂತಿರುಗಿ -EAGAIN;
	}

	_current->ತಳ.ಅದಲುಬದಲು_ದತ್ತಾಂಶ = buf_spec;

	pipe->ಕಾಯುವಿಕೆ++;
	*need_resched = false;
	ಆದರೆ (waitq == &pipe->space) {
		SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_BLOCKING(k_pipe, ಲಿಖ, pipe, timeout);
	} ಇಲ್ಲದಿದ್ದರೆ {
		SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_BLOCKING(k_pipe, ಪಠ, pipe, timeout);
	}
	rc = z_pend_curr(&pipe->ಬೀಗ, *ಕೀಲಿ, waitq, timeout);
	*ಕೀಲಿ = k_spin_ಬೀಗ(&pipe->ಬೀಗ);
	pipe->ಕಾಯುವಿಕೆ--;
	ಆದರೆ (unlikely(pipe_resetting(pipe))) {
		ಆದರೆ (pipe->ಕಾಯುವಿಕೆ == 0) {
			pipe->flags &= ~PIPE_FLAG_RESET;
		}
		rc = -ECANCELED;
	}

	ಹಿಂತಿರುಗಿ rc;
}

ಶೂನ್ಯ z_impl_k_pipe_init(ರಚನೆ k_pipe *pipe, uint8_t *ಬಫರ್, size_t ಬಫರ್_ಗಾತ್ರ)
{
	ring_buf_init(&pipe->buf, ಬಫರ್_ಗಾತ್ರ, ಬಫರ್);
	pipe->flags = PIPE_FLAG_OPEN;
	pipe->ಕಾಯುವಿಕೆ = 0;

	pipe->ಬೀಗ = (ರಚನೆ k_spinlock){};
	z_waitq_init(&pipe->ದತ್ತಾಂಶ);
	z_waitq_init(&pipe->space);
	k_object_init(pipe);

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_POLL
	sys_dlist_init(&pipe->poll_events);
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_POLL */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_OBJ_CORE_PIPE
	k_obj_core_init_and_link(K_OBJ_CORE(pipe), &obj_type_pipe);
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_OBJ_CORE_PIPE */
	SYS_ಪೋರ್ಟ್_TRACING_OBJ_INIT(k_pipe, pipe, ಬಫರ್, ಬಫರ್_ಗಾತ್ರ);
}

ಸ್ಥಿರ size_t copy_ಗಮ್ಯ_pending_readers(ರಚನೆ k_pipe *pipe, bool *need_resched,
				      ಸ್ಥಿರಾಂಕ uint8_t *ದತ್ತಾಂಶ, size_t len)
{
	ರಚನೆ k_thread *reader = ಖಾಲಿ;
	ರಚನೆ pipe_buf_spec *reader_buf;
	size_t copy_ಗಾತ್ರ, written = 0;

	/*
	 * Attempt a direct data copy to waiting readers if any.
	 * The copy has to be done under the scheduler lock to ensure all the
	 * needed data is copied to the target thread whose buffer spec lives
	 * on that thread's stack, and then the thread unpended only if it
	 * received all the data it wanted, without racing with a potential
	 * thread timeout/cancellation event.
	 */
	ಮಾಡು {
		ಬೀಗ_SCHED_SPINLOCK {
			reader = priq_wait_ಉತ್ತಮ(&pipe->ದತ್ತಾಂಶ.waitq);
			ಆದರೆ (reader == ಖಾಲಿ) {
				K_SPINLOCK_BREAK;
			}

			reader_buf = reader->ತಳ.ಅದಲುಬದಲು_ದತ್ತಾಂಶ;
			copy_ಗಾತ್ರ = min(len - written,
					reader_buf->len - reader_buf->ಬಳಸಿದ);
			ಸ್ಮರಣೆ_ನಕಲು(&reader_buf->ದತ್ತಾಂಶ[reader_buf->ಬಳಸಿದ],
			       &ದತ್ತಾಂಶ[written], copy_ಗಾತ್ರ);
			written += copy_ಗಾತ್ರ;
			reader_buf->ಬಳಸಿದ += copy_ಗಾತ್ರ;

			ಆದರೆ (reader_buf->ಬಳಸಿದ < reader_buf->len) {
				/* This reader wants more: don't unpend. */
				reader = ಖಾಲಿ;
			} ಇಲ್ಲದಿದ್ದರೆ {
				/*
				 * This reader has received all the data
				 * it was waiting for. Set its return
				 * value, unpend, abort its timeout, and
				 * ready it, all under the scheduler lock
				 * so a racing timeout handler cannot
				 * observe a half-initialized wake-up.
				 */
				z_thread_return_ಮೌಲ್ಯ_ಗಣ_with_ದತ್ತಾಂಶ(reader, 0, ಖಾಲಿ);
				unpend_thread_no_timeout(reader);
				(ಶೂನ್ಯ)z_try_abort_thread_timeout(reader);
				z_sched_ready_locked(reader);
				*need_resched = true;
			}
		}
	} ಆಗ (reader != ಖಾಲಿ && written < len);

	ಹಿಂತಿರುಗಿ written;
}

ಪೂರ್ಣಾಂಕ z_impl_k_pipe_write(ರಚನೆ k_pipe *pipe, ಸ್ಥಿರಾಂಕ uint8_t *ದತ್ತಾಂಶ, size_t len, k_timeout_t timeout)
{
	ಪೂರ್ಣಾಂಕ rc;
	size_t written = 0;
	size_t added;
	k_timepoint_t ಕೊನೆ = sys_timepoint_calc(timeout);
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_ಬೀಗ(&pipe->ಬೀಗ);
	bool need_resched = false;

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_ENTER(k_pipe, ಲಿಖ, pipe, ದತ್ತಾಂಶ, len, timeout);

	ಆದರೆ (unlikely(len > ಗರಿಷ್ಠ_ಪೂರ್ಣಾಂಕ)) {
		rc = -EOVERFLOW;
		ಹೋಗು ಹೊರಹೋಗು;
	}

	ಆದರೆ (unlikely(pipe_resetting(pipe))) {
		rc = -ECANCELED;
		ಹೋಗು ಹೊರಹೋಗು;
	}

	ಗಾಗಿ (;;) {
		ಆದರೆ (unlikely(pipe_closed(pipe))) {
			rc = -EPIPE;
			ನಿಲ್ಲಿಸು;
		}

		ಆದರೆ (pipe_ಖಾಲಿಗಳು(pipe)) {
			ಆದರೆ (IS_ENABLED(CONFIG_KERNEL_COHERENCE)) {
				/*
				 * Systems that enabled this option don't have
				 * their stacks in coherent memory. Given our
				 * pipe_buf_spec is stored on the stack, and
				 * readers may also have their destination
				 * buffer on their stack too, it is not worth
				 * supporting direct-to-readers copy with them.
				 * Simply wake up all pending readers instead.
				 */
				need_resched = z_sched_wake_all(&pipe->ದತ್ತಾಂಶ, 0, ಖಾಲಿ);
			} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (pipe->ಕಾಯುವಿಕೆ != 0) {
				written += copy_ಗಮ್ಯ_pending_readers(pipe, &need_resched,
								   &ದತ್ತಾಂಶ[written],
								   len - written);
				ಆದರೆ (written >= len) {
					rc = written;
					ನಿಲ್ಲಿಸು;
				}
			}
		}

		added = ring_buf_put(&pipe->buf, &ದತ್ತಾಂಶ[written], len - written);

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_POLL
		ಆದರೆ (added != 0) {
			need_resched |= z_handle_obj_poll_events(&pipe->poll_events,
								 K_POLL_STATE_PIPE_ದತ್ತಾಂಶ_ಲಭ್ಯ);
		}
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_POLL */

		written += added;
		ಆದರೆ (likely(written == len)) {
			rc = written;
			ನಿಲ್ಲಿಸು;
		}

		rc = wait_for(&pipe->space, pipe, &ಕೀಲಿ, ಕೊನೆ, &need_resched, ಖಾಲಿ);
		ಆದರೆ (rc != 0) {
			ಆದರೆ (rc == -EAGAIN) {
				rc = written ? written : -EAGAIN;
			}
			ನಿಲ್ಲಿಸು;
		}
	}
ಹೊರಹೋಗು:
	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_EXIT(k_pipe, ಲಿಖ, pipe, rc);
	ಆದರೆ (need_resched) {
		z_reschedule(&pipe->ಬೀಗ, ಕೀಲಿ);
	} ಇಲ್ಲದಿದ್ದರೆ {
		k_spin_unlock(&pipe->ಬೀಗ, ಕೀಲಿ);
	}
	ಹಿಂತಿರುಗಿ rc;
}

ಪೂರ್ಣಾಂಕ z_impl_k_pipe_read(ರಚನೆ k_pipe *pipe, uint8_t *ದತ್ತಾಂಶ, size_t len, k_timeout_t timeout)
{
	ರಚನೆ pipe_buf_spec buf = { ದತ್ತಾಂಶ, len, 0 };
	ಪೂರ್ಣಾಂಕ rc;
	k_timepoint_t ಕೊನೆ = sys_timepoint_calc(timeout);
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_ಬೀಗ(&pipe->ಬೀಗ);
	bool need_resched = false;

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_ENTER(k_pipe, ಪಠ, pipe, ದತ್ತಾಂಶ, len, timeout);

	ಆದರೆ (unlikely(len > ಗರಿಷ್ಠ_ಪೂರ್ಣಾಂಕ)) {
		rc = -EOVERFLOW;
		ಹೋಗು ಹೊರಹೋಗು;
	}

	ಆದರೆ (unlikely(pipe_resetting(pipe))) {
		rc = -ECANCELED;
		ಹೋಗು ಹೊರಹೋಗು;
	}

	ಗಾಗಿ (;;) {
		ಆದರೆ (pipe_ತುಂಬಿದ(pipe)) {
			/* One or more pending writers may exist. */
			need_resched = z_sched_wake_all(&pipe->space, 0, ಖಾಲಿ);
		}

		buf.ಬಳಸಿದ += ring_buf_get(&pipe->buf, &ದತ್ತಾಂಶ[buf.ಬಳಸಿದ], len - buf.ಬಳಸಿದ);
		ಆದರೆ (likely(buf.ಬಳಸಿದ == len)) {
			rc = buf.ಬಳಸಿದ;
			ನಿಲ್ಲಿಸು;
		}

		ಆದರೆ (unlikely(pipe_closed(pipe))) {
			rc = buf.ಬಳಸಿದ ? buf.ಬಳಸಿದ : -EPIPE;
			ನಿಲ್ಲಿಸು;
		}

		rc = wait_for(&pipe->ದತ್ತಾಂಶ, pipe, &ಕೀಲಿ, ಕೊನೆ, &need_resched, &buf);
		ಆದರೆ (rc != 0) {
			ಆದರೆ (rc == -EAGAIN) {
				rc = buf.ಬಳಸಿದ ? buf.ಬಳಸಿದ : -EAGAIN;
			}
			ನಿಲ್ಲಿಸು;
		}
	}
ಹೊರಹೋಗು:
	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_EXIT(k_pipe, ಪಠ, pipe, rc);
	ಆದರೆ (need_resched) {
		z_reschedule(&pipe->ಬೀಗ, ಕೀಲಿ);
	} ಇಲ್ಲದಿದ್ದರೆ {
		k_spin_unlock(&pipe->ಬೀಗ, ಕೀಲಿ);
	}
	ಹಿಂತಿರುಗಿ rc;
}

ಶೂನ್ಯ z_impl_k_pipe_reset(ರಚನೆ k_pipe *pipe)
{
	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_ENTER(k_pipe, reset, pipe);
	K_SPINLOCK(&pipe->ಬೀಗ) {
		ring_buf_reset(&pipe->buf);
		ಆದರೆ (likely(pipe->ಕಾಯುವಿಕೆ != 0)) {
			pipe->flags |= PIPE_FLAG_RESET;
			z_sched_wake_all(&pipe->ದತ್ತಾಂಶ, 0, ಖಾಲಿ);
			z_sched_wake_all(&pipe->space, 0, ಖಾಲಿ);
		}
	}
	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_EXIT(k_pipe, reset, pipe);
}

ಶೂನ್ಯ z_impl_k_pipe_close(ರಚನೆ k_pipe *pipe)
{
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ;
	bool need_resched;

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_ENTER(k_pipe, close, pipe);

	ಕೀಲಿ = k_spin_ಬೀಗ(&pipe->ಬೀಗ);

	pipe->flags = 0;
	need_resched = z_sched_wake_all(&pipe->ದತ್ತಾಂಶ, 0, ಖಾಲಿ);
	need_resched |= z_sched_wake_all(&pipe->space, 0, ಖಾಲಿ);

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_POLL
	/*
	 * Closing is a broadcast: wake every registered poller. Later
	 * k_poll() calls see the hangup through is_condition_met().
	 */
	ಆಗ (z_handle_obj_poll_events(&pipe->poll_events,
					K_POLL_STATE_PIPE_ದತ್ತಾಂಶ_ಲಭ್ಯ)) {
		need_resched = true;
	}
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_POLL */

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_EXIT(k_pipe, close, pipe);

	ಆದರೆ (need_resched) {
		z_reschedule(&pipe->ಬೀಗ, ಕೀಲಿ);
	} ಇಲ್ಲದಿದ್ದರೆ {
		k_spin_unlock(&pipe->ಬೀಗ, ಕೀಲಿ);
	}
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_USERSPACE
ಶೂನ್ಯ z_vrfy_k_pipe_init(ರಚನೆ k_pipe *pipe, uint8_t *ಬಫರ್, size_t ಬಫರ್_ಗಾತ್ರ)
{
	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(pipe, K_OBJ_PIPE));
	K_OOPS(K_SYSCALL_ಸ್ಮರಣೆ_WRITE(ಬಫರ್, ಬಫರ್_ಗಾತ್ರ));

	z_impl_k_pipe_init(pipe, ಬಫರ್, ಬಫರ್_ಗಾತ್ರ);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_pipe_init_mrsh.c>

ಪೂರ್ಣಾಂಕ z_vrfy_k_pipe_read(ರಚನೆ k_pipe *pipe, uint8_t *ದತ್ತಾಂಶ, size_t len, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(pipe, K_OBJ_PIPE));
	K_OOPS(K_SYSCALL_ಸ್ಮರಣೆ_WRITE(ದತ್ತಾಂಶ, len));

	ಹಿಂತಿರುಗಿ z_impl_k_pipe_read(pipe, ದತ್ತಾಂಶ, len, timeout);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_pipe_read_mrsh.c>

ಪೂರ್ಣಾಂಕ z_vrfy_k_pipe_write(ರಚನೆ k_pipe *pipe, ಸ್ಥಿರಾಂಕ uint8_t *ದತ್ತಾಂಶ, size_t len, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(pipe, K_OBJ_PIPE));
	K_OOPS(K_SYSCALL_ಸ್ಮರಣೆ_READ(ದತ್ತಾಂಶ, len));

	ಹಿಂತಿರುಗಿ z_impl_k_pipe_write(pipe, ದತ್ತಾಂಶ, len, timeout);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_pipe_write_mrsh.c>

ಶೂನ್ಯ z_vrfy_k_pipe_reset(ರಚನೆ k_pipe *pipe)
{
	K_OOPS(K_SYSCALL_OBJ(pipe, K_OBJ_PIPE));
	z_impl_k_pipe_reset(pipe);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_pipe_reset_mrsh.c>

ಶೂನ್ಯ z_vrfy_k_pipe_close(ರಚನೆ k_pipe *pipe)
{
	K_OOPS(K_SYSCALL_OBJ(pipe, K_OBJ_PIPE));
	z_impl_k_pipe_close(pipe);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_pipe_close_mrsh.c>
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_USERSPACE */
