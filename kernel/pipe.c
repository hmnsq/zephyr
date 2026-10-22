/*
 * Copyright (c) 2024 Måns Ansgariusson <mansgariusson@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#अन्तर्भाव <zephyr/sys/minmax.h>
#अन्तर्भाव <zephyr/init.h>
#अन्तर्भाव <zephyr/kernel.h>
#अन्तर्भाव <zephyr/internal/syscall_handler.h>
#अन्तर्भाव <ksched.h>
#अन्तर्भाव <kthread.h>
#अन्तर्भाव <wait_q.h>
#अन्तर्भाव <scheduler.h>

#यदि_परिभाषितम् CONFIG_OBJ_CORE_PIPE
K_OBJ_TYPE_DEFINE(obj_type_pipe, k_pipe, K_OBJ_TYPE_PIPE_ID, रिक्त);
#यदि_अन्तः /* CONFIG_OBJ_CORE_PIPE */

स्थिर अन्तःस्थ bool pipe_closed(संरचना k_pipe *pipe)
{
	प्रत्यागम (pipe->flags & PIPE_FLAG_OPEN) == 0;
}

स्थिर अन्तःस्थ bool pipe_resetting(संरचना k_pipe *pipe)
{
	प्रत्यागम (pipe->flags & PIPE_FLAG_RESET) != 0;
}

स्थिर अन्तःस्थ bool pipe_पूर्णम्(संरचना k_pipe *pipe)
{
	प्रत्यागम ring_buf_space_get(&pipe->buf) == 0;
}

स्थिर अन्तःस्थ bool pipe_रिक्तानि(संरचना k_pipe *pipe)
{
	प्रत्यागम ring_buf_is_रिक्तानि(&pipe->buf);
}

संरचना pipe_buf_spec {
	uint8_t * ध्रुव दत्त;
	ध्रुव size_t len;
	size_t प्रयुक्तम्;
};

स्थिर पूर्णाङ्क wait_for(_wait_q_t *waitq, संरचना k_pipe *pipe, k_spinlock_कुञ्चिका_t *कुञ्चिका,
		    k_timepoint_t time_सीमा, bool *need_resched,
		    संरचना pipe_buf_spec *buf_spec)
{
	k_timeout_t timeout = sys_timepoint_timeout(time_सीमा);
	पूर्णाङ्क rc;

	यदि (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		प्रत्यागम -EAGAIN;
	}

	_current->आधारः.swap_दत्त = buf_spec;

	pipe->प्रतीक्षा++;
	*need_resched = false;
	यदि (waitq == &pipe->space) {
		SYS_पत्तनम्_TRACING_OBJ_FUNC_BLOCKING(k_pipe, लिख, pipe, timeout);
	} अन्यथा {
		SYS_पत्तनम्_TRACING_OBJ_FUNC_BLOCKING(k_pipe, पठ, pipe, timeout);
	}
	rc = z_pend_curr(&pipe->तालकम्, *कुञ्चिका, waitq, timeout);
	*कुञ्चिका = k_spin_तालकम्(&pipe->तालकम्);
	pipe->प्रतीक्षा--;
	यदि (unlikely(pipe_resetting(pipe))) {
		यदि (pipe->प्रतीक्षा == 0) {
			pipe->flags &= ~PIPE_FLAG_RESET;
		}
		rc = -ECANCELED;
	}

	प्रत्यागम rc;
}

शून्य z_impl_k_pipe_init(संरचना k_pipe *pipe, uint8_t *बफर, size_t बफर_आकार)
{
	ring_buf_init(&pipe->buf, बफर_आकार, बफर);
	pipe->flags = PIPE_FLAG_OPEN;
	pipe->प्रतीक्षा = 0;

	pipe->तालकम् = (संरचना k_spinlock){};
	z_waitq_init(&pipe->दत्त);
	z_waitq_init(&pipe->space);
	k_object_init(pipe);

#यदि_परिभाषितम् CONFIG_POLL
	sys_dlist_init(&pipe->poll_events);
#यदि_अन्तः /* CONFIG_POLL */
#यदि_परिभाषितम् CONFIG_OBJ_CORE_PIPE
	k_obj_core_init_and_link(K_OBJ_CORE(pipe), &obj_type_pipe);
#यदि_अन्तः /* CONFIG_OBJ_CORE_PIPE */
	SYS_पत्तनम्_TRACING_OBJ_INIT(k_pipe, pipe, बफर, बफर_आकार);
}

स्थिर size_t copy_गन्तव्यम्_pending_readers(संरचना k_pipe *pipe, bool *need_resched,
				      ध्रुव uint8_t *दत्त, size_t len)
{
	संरचना k_thread *reader = रिक्त;
	संरचना pipe_buf_spec *reader_buf;
	size_t copy_आकार, written = 0;

	/*
	 * Attempt a direct data copy to waiting readers if any.
	 * The copy has to be done under the scheduler lock to ensure all the
	 * needed data is copied to the target thread whose buffer spec lives
	 * on that thread's stack, and then the thread unpended only if it
	 * received all the data it wanted, without racing with a potential
	 * thread timeout/cancellation event.
	 */
	कुरु {
		तालकम्_SCHED_SPINLOCK {
			reader = _priq_wait_best(&pipe->दत्त.waitq);
			यदि (reader == रिक्त) {
				K_SPINLOCK_BREAK;
			}

			reader_buf = reader->आधारः.swap_दत्त;
			copy_आकार = min(len - written,
					reader_buf->len - reader_buf->प्रयुक्तम्);
			स्मृतिं_प्रतिलिख(&reader_buf->दत्त[reader_buf->प्रयुक्तम्],
			       &दत्त[written], copy_आकार);
			written += copy_आकार;
			reader_buf->प्रयुक्तम् += copy_आकार;

			यदि (reader_buf->प्रयुक्तम् < reader_buf->len) {
				/* This reader wants more: don't unpend. */
				reader = रिक्त;
			} अन्यथा {
				/*
				 * This reader has received all the data
				 * it was waiting for. Set its return
				 * value, unpend, abort its timeout, and
				 * ready it, all under the scheduler lock
				 * so a racing timeout handler cannot
				 * observe a half-initialized wake-up.
				 */
				z_thread_return_मानम्_समुच्चयः_with_दत्त(reader, 0, रिक्त);
				unpend_thread_no_timeout(reader);
				(शून्य)z_try_abort_thread_timeout(reader);
				z_sched_ready_locked(reader);
				*need_resched = true;
			}
		}
	} यावत् (reader != रिक्त && written < len);

	प्रत्यागम written;
}

पूर्णाङ्क z_impl_k_pipe_write(संरचना k_pipe *pipe, ध्रुव uint8_t *दत्त, size_t len, k_timeout_t timeout)
{
	पूर्णाङ्क rc;
	size_t written = 0;
	size_t added;
	k_timepoint_t अन्तः = sys_timepoint_calc(timeout);
	k_spinlock_कुञ्चिका_t कुञ्चिका = k_spin_तालकम्(&pipe->तालकम्);
	bool need_resched = false;

	SYS_पत्तनम्_TRACING_OBJ_FUNC_ENTER(k_pipe, लिख, pipe, दत्त, len, timeout);

	यदि (unlikely(len > अधिकतम_पूर्णाङ्कः)) {
		rc = -EOVERFLOW;
		गच्छ बहिर्गमनम्;
	}

	यदि (unlikely(pipe_resetting(pipe))) {
		rc = -ECANCELED;
		गच्छ बहिर्गमनम्;
	}

	प्रति (;;) {
		यदि (unlikely(pipe_closed(pipe))) {
			rc = -EPIPE;
			भङ्ग;
		}

		यदि (pipe_रिक्तानि(pipe)) {
			यदि (IS_ENABLED(CONFIG_KERNEL_COHERENCE)) {
				/*
				 * Systems that enabled this option don't have
				 * their stacks in coherent memory. Given our
				 * pipe_buf_spec is stored on the stack, and
				 * readers may also have their destination
				 * buffer on their stack too, it is not worth
				 * supporting direct-to-readers copy with them.
				 * Simply wake up all pending readers instead.
				 */
				need_resched = z_sched_wake_all(&pipe->दत्त, 0, रिक्त);
			} अन्यथा यदि (pipe->प्रतीक्षा != 0) {
				written += copy_गन्तव्यम्_pending_readers(pipe, &need_resched,
								   &दत्त[written],
								   len - written);
				यदि (written >= len) {
					rc = written;
					भङ्ग;
				}
			}
		}

		added = ring_buf_put(&pipe->buf, &दत्त[written], len - written);

#यदि_परिभाषितम् CONFIG_POLL
		यदि (added != 0) {
			need_resched |= z_handle_obj_poll_events(&pipe->poll_events,
								 K_POLL_STATE_PIPE_दत्त_उपलब्धम्);
		}
#यदि_अन्तः /* CONFIG_POLL */

		written += added;
		यदि (likely(written == len)) {
			rc = written;
			भङ्ग;
		}

		rc = wait_for(&pipe->space, pipe, &कुञ्चिका, अन्तः, &need_resched, रिक्त);
		यदि (rc != 0) {
			यदि (rc == -EAGAIN) {
				rc = written ? written : -EAGAIN;
			}
			भङ्ग;
		}
	}
बहिर्गमनम्:
	SYS_पत्तनम्_TRACING_OBJ_FUNC_EXIT(k_pipe, लिख, pipe, rc);
	यदि (need_resched) {
		z_reschedule(&pipe->तालकम्, कुञ्चिका);
	} अन्यथा {
		k_spin_unlock(&pipe->तालकम्, कुञ्चिका);
	}
	प्रत्यागम rc;
}

पूर्णाङ्क z_impl_k_pipe_read(संरचना k_pipe *pipe, uint8_t *दत्त, size_t len, k_timeout_t timeout)
{
	संरचना pipe_buf_spec buf = { दत्त, len, 0 };
	पूर्णाङ्क rc;
	k_timepoint_t अन्तः = sys_timepoint_calc(timeout);
	k_spinlock_कुञ्चिका_t कुञ्चिका = k_spin_तालकम्(&pipe->तालकम्);
	bool need_resched = false;

	SYS_पत्तनम्_TRACING_OBJ_FUNC_ENTER(k_pipe, पठ, pipe, दत्त, len, timeout);

	यदि (unlikely(len > अधिकतम_पूर्णाङ्कः)) {
		rc = -EOVERFLOW;
		गच्छ बहिर्गमनम्;
	}

	यदि (unlikely(pipe_resetting(pipe))) {
		rc = -ECANCELED;
		गच्छ बहिर्गमनम्;
	}

	प्रति (;;) {
		यदि (pipe_पूर्णम्(pipe)) {
			/* One or more pending writers may exist. */
			need_resched = z_sched_wake_all(&pipe->space, 0, रिक्त);
		}

		buf.प्रयुक्तम् += ring_buf_get(&pipe->buf, &दत्त[buf.प्रयुक्तम्], len - buf.प्रयुक्तम्);
		यदि (likely(buf.प्रयुक्तम् == len)) {
			rc = buf.प्रयुक्तम्;
			भङ्ग;
		}

		यदि (unlikely(pipe_closed(pipe))) {
			rc = buf.प्रयुक्तम् ? buf.प्रयुक्तम् : -EPIPE;
			भङ्ग;
		}

		rc = wait_for(&pipe->दत्त, pipe, &कुञ्चिका, अन्तः, &need_resched, &buf);
		यदि (rc != 0) {
			यदि (rc == -EAGAIN) {
				rc = buf.प्रयुक्तम् ? buf.प्रयुक्तम् : -EAGAIN;
			}
			भङ्ग;
		}
	}
बहिर्गमनम्:
	SYS_पत्तनम्_TRACING_OBJ_FUNC_EXIT(k_pipe, पठ, pipe, rc);
	यदि (need_resched) {
		z_reschedule(&pipe->तालकम्, कुञ्चिका);
	} अन्यथा {
		k_spin_unlock(&pipe->तालकम्, कुञ्चिका);
	}
	प्रत्यागम rc;
}

शून्य z_impl_k_pipe_reset(संरचना k_pipe *pipe)
{
	SYS_पत्तनम्_TRACING_OBJ_FUNC_ENTER(k_pipe, reset, pipe);
	K_SPINLOCK(&pipe->तालकम्) {
		ring_buf_reset(&pipe->buf);
		यदि (likely(pipe->प्रतीक्षा != 0)) {
			pipe->flags |= PIPE_FLAG_RESET;
			z_sched_wake_all(&pipe->दत्त, 0, रिक्त);
			z_sched_wake_all(&pipe->space, 0, रिक्त);
		}
	}
	SYS_पत्तनम्_TRACING_OBJ_FUNC_EXIT(k_pipe, reset, pipe);
}

शून्य z_impl_k_pipe_close(संरचना k_pipe *pipe)
{
	k_spinlock_कुञ्चिका_t कुञ्चिका;
	bool need_resched;

	SYS_पत्तनम्_TRACING_OBJ_FUNC_ENTER(k_pipe, close, pipe);

	कुञ्चिका = k_spin_तालकम्(&pipe->तालकम्);

	pipe->flags = 0;
	need_resched = z_sched_wake_all(&pipe->दत्त, 0, रिक्त);
	need_resched |= z_sched_wake_all(&pipe->space, 0, रिक्त);

#यदि_परिभाषितम् CONFIG_POLL
	/*
	 * Closing is a broadcast: wake every registered poller. Later
	 * k_poll() calls see the hangup through is_condition_met().
	 */
	यावत् (z_handle_obj_poll_events(&pipe->poll_events,
					K_POLL_STATE_PIPE_दत्त_उपलब्धम्)) {
		need_resched = true;
	}
#यदि_अन्तः /* CONFIG_POLL */

	SYS_पत्तनम्_TRACING_OBJ_FUNC_EXIT(k_pipe, close, pipe);

	यदि (need_resched) {
		z_reschedule(&pipe->तालकम्, कुञ्चिका);
	} अन्यथा {
		k_spin_unlock(&pipe->तालकम्, कुञ्चिका);
	}
}

#यदि_परिभाषितम् CONFIG_USERSPACE
शून्य z_vrfy_k_pipe_init(संरचना k_pipe *pipe, uint8_t *बफर, size_t बफर_आकार)
{
	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(pipe, K_OBJ_PIPE));
	K_OOPS(K_SYSCALL_स्मृतिः_WRITE(बफर, बफर_आकार));

	z_impl_k_pipe_init(pipe, बफर, बफर_आकार);
}
#अन्तर्भाव <zephyr/syscalls/k_pipe_init_mrsh.c>

पूर्णाङ्क z_vrfy_k_pipe_read(संरचना k_pipe *pipe, uint8_t *दत्त, size_t len, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(pipe, K_OBJ_PIPE));
	K_OOPS(K_SYSCALL_स्मृतिः_WRITE(दत्त, len));

	प्रत्यागम z_impl_k_pipe_read(pipe, दत्त, len, timeout);
}
#अन्तर्भाव <zephyr/syscalls/k_pipe_read_mrsh.c>

पूर्णाङ्क z_vrfy_k_pipe_write(संरचना k_pipe *pipe, ध्रुव uint8_t *दत्त, size_t len, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(pipe, K_OBJ_PIPE));
	K_OOPS(K_SYSCALL_स्मृतिः_READ(दत्त, len));

	प्रत्यागम z_impl_k_pipe_write(pipe, दत्त, len, timeout);
}
#अन्तर्भाव <zephyr/syscalls/k_pipe_write_mrsh.c>

शून्य z_vrfy_k_pipe_reset(संरचना k_pipe *pipe)
{
	K_OOPS(K_SYSCALL_OBJ(pipe, K_OBJ_PIPE));
	z_impl_k_pipe_reset(pipe);
}
#अन्तर्भाव <zephyr/syscalls/k_pipe_reset_mrsh.c>

शून्य z_vrfy_k_pipe_close(संरचना k_pipe *pipe)
{
	K_OOPS(K_SYSCALL_OBJ(pipe, K_OBJ_PIPE));
	z_impl_k_pipe_close(pipe);
}
#अन्तर्भाव <zephyr/syscalls/k_pipe_close_mrsh.c>
#यदि_अन्तः /* CONFIG_USERSPACE */
