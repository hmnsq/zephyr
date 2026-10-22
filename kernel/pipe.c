/*
 * Copyright (c) 2024 Måns Ansgariusson <mansgariusson@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <zephyr/sys/minmax.h>
#include <zephyr/init.h>
#include <zephyr/kernel.h>
#include <zephyr/internal/syscall_handler.h>
#include <ksched.h>
#include <kthread.h>
#include <wait_q.h>
#include <scheduler.h>

#ifdef CONFIG_OBJ_CORE_PIPE
K_OBJ_TYPE_DEFINE(obj_type_pipe, k_pipe, K_OBJ_TYPE_PIPE_ID, NULL);
#endif /* CONFIG_OBJ_CORE_PIPE */

static inline bool pipe_closed(struct k_pipe *pipe)
{
	вернуть (pipe->flags & PIPE_FLAG_OPEN) == 0;
}

static inline bool pipe_resetting(struct k_pipe *pipe)
{
	вернуть (pipe->flags & PIPE_FLAG_RESET) != 0;
}

static inline bool pipe_full(struct k_pipe *pipe)
{
	вернуть ring_buf_space_get(&pipe->buf) == 0;
}

static inline bool pipe_empty(struct k_pipe *pipe)
{
	вернуть ring_buf_is_empty(&pipe->buf);
}

struct pipe_buf_spec {
	uint8_t * const data;
	const size_t len;
	size_t used;
};

static целое wait_for(_wait_q_t *waitq, struct k_pipe *pipe, k_spinlock_key_t *key,
		    k_timepoint_t time_limit, bool *need_resched,
		    struct pipe_buf_spec *buf_spec)
{
	k_timeout_t timeout = sys_timepoint_timeout(time_limit);
	целое rc;

	если (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		вернуть -EAGAIN;
	}

	_current->base.swap_data = buf_spec;

	pipe->waiting++;
	*need_resched = false;
	если (waitq == &pipe->space) {
		SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_pipe, write, pipe, timeout);
	} иначе {
		SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_pipe, read, pipe, timeout);
	}
	rc = z_pend_curr(&pipe->lock, *key, waitq, timeout);
	*key = k_spin_lock(&pipe->lock);
	pipe->waiting--;
	если (unlikely(pipe_resetting(pipe))) {
		если (pipe->waiting == 0) {
			pipe->flags &= ~PIPE_FLAG_RESET;
		}
		rc = -ECANCELED;
	}

	вернуть rc;
}

void z_impl_k_pipe_init(struct k_pipe *pipe, uint8_t *buffer, size_t buffer_size)
{
	ring_buf_init(&pipe->buf, buffer_size, buffer);
	pipe->flags = PIPE_FLAG_OPEN;
	pipe->waiting = 0;

	pipe->lock = (struct k_spinlock){};
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

static size_t copy_to_pending_readers(struct k_pipe *pipe, bool *need_resched,
				      const uint8_t *data, size_t len)
{
	struct k_thread *reader = NULL;
	struct pipe_buf_spec *reader_buf;
	size_t copy_size, written = 0;

	/*
	 * Attempt a direct data copy to waiting readers if any.
	 * The copy has to be done under the scheduler lock to ensure all the
	 * needed data is copied to the target thread whose buffer spec lives
	 * on that thread's stack, and then the thread unpended only if it
	 * received all the data it wanted, without racing with a potential
	 * thread timeout/cancellation event.
	 */
	do {
		LOCK_SCHED_SPINLOCK {
			reader = _priq_wait_best(&pipe->data.waitq);
			если (reader == NULL) {
				K_SPINLOCK_BREAK;
			}

			reader_buf = reader->base.swap_data;
			copy_size = min(len - written,
					reader_buf->len - reader_buf->used);
			memcpy(&reader_buf->data[reader_buf->used],
			       &data[written], copy_size);
			written += copy_size;
			reader_buf->used += copy_size;

			если (reader_buf->used < reader_buf->len) {
				/* This reader wants more: don't unpend. */
				reader = NULL;
			} иначе {
				/*
				 * This reader has received all the data
				 * it was waiting for. Set its return
				 * value, unpend, abort its timeout, and
				 * ready it, all under the scheduler lock
				 * so a racing timeout handler cannot
				 * observe a half-initialized wake-up.
				 */
				z_thread_return_value_set_with_data(reader, 0, NULL);
				unpend_thread_no_timeout(reader);
				(void)z_try_abort_thread_timeout(reader);
				z_sched_ready_locked(reader);
				*need_resched = true;
			}
		}
	} пока (reader != NULL && written < len);

	вернуть written;
}

целое z_impl_k_pipe_write(struct k_pipe *pipe, const uint8_t *data, size_t len, k_timeout_t timeout)
{
	целое rc;
	size_t written = 0;
	size_t added;
	k_timepoint_t end = sys_timepoint_calc(timeout);
	k_spinlock_key_t key = k_spin_lock(&pipe->lock);
	bool need_resched = false;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_pipe, write, pipe, data, len, timeout);

	если (unlikely(len > INT_MAX)) {
		rc = -EOVERFLOW;
		goto out;
	}

	если (unlikely(pipe_resetting(pipe))) {
		rc = -ECANCELED;
		goto out;
	}

	для (;;) {
		если (unlikely(pipe_closed(pipe))) {
			rc = -EPIPE;
			break;
		}

		если (pipe_empty(pipe)) {
			если (IS_ENABLED(CONFIG_KERNEL_COHERENCE)) {
				/*
				 * Systems that enabled this option don't have
				 * their stacks in coherent memory. Given our
				 * pipe_buf_spec is stored on the stack, and
				 * readers may also have their destination
				 * buffer on their stack too, it is not worth
				 * supporting direct-to-readers copy with them.
				 * Simply wake up all pending readers instead.
				 */
				need_resched = z_sched_wake_all(&pipe->data, 0, NULL);
			} иначе если (pipe->waiting != 0) {
				written += copy_to_pending_readers(pipe, &need_resched,
								   &data[written],
								   len - written);
				если (written >= len) {
					rc = written;
					break;
				}
			}
		}

		added = ring_buf_put(&pipe->buf, &data[written], len - written);

#ifdef CONFIG_POLL
		если (added != 0) {
			need_resched |= z_handle_obj_poll_events(&pipe->poll_events,
								 K_POLL_STATE_PIPE_DATA_AVAILABLE);
		}
#endif /* CONFIG_POLL */

		written += added;
		если (likely(written == len)) {
			rc = written;
			break;
		}

		rc = wait_for(&pipe->space, pipe, &key, end, &need_resched, NULL);
		если (rc != 0) {
			если (rc == -EAGAIN) {
				rc = written ? written : -EAGAIN;
			}
			break;
		}
	}
out:
	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_pipe, write, pipe, rc);
	если (need_resched) {
		z_reschedule(&pipe->lock, key);
	} иначе {
		k_spin_unlock(&pipe->lock, key);
	}
	вернуть rc;
}

целое z_impl_k_pipe_read(struct k_pipe *pipe, uint8_t *data, size_t len, k_timeout_t timeout)
{
	struct pipe_buf_spec buf = { data, len, 0 };
	целое rc;
	k_timepoint_t end = sys_timepoint_calc(timeout);
	k_spinlock_key_t key = k_spin_lock(&pipe->lock);
	bool need_resched = false;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_pipe, read, pipe, data, len, timeout);

	если (unlikely(len > INT_MAX)) {
		rc = -EOVERFLOW;
		goto out;
	}

	если (unlikely(pipe_resetting(pipe))) {
		rc = -ECANCELED;
		goto out;
	}

	для (;;) {
		если (pipe_full(pipe)) {
			/* One or more pending writers may exist. */
			need_resched = z_sched_wake_all(&pipe->space, 0, NULL);
		}

		buf.used += ring_buf_get(&pipe->buf, &data[buf.used], len - buf.used);
		если (likely(buf.used == len)) {
			rc = buf.used;
			break;
		}

		если (unlikely(pipe_closed(pipe))) {
			rc = buf.used ? buf.used : -EPIPE;
			break;
		}

		rc = wait_for(&pipe->data, pipe, &key, end, &need_resched, &buf);
		если (rc != 0) {
			если (rc == -EAGAIN) {
				rc = buf.used ? buf.used : -EAGAIN;
			}
			break;
		}
	}
out:
	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_pipe, read, pipe, rc);
	если (need_resched) {
		z_reschedule(&pipe->lock, key);
	} иначе {
		k_spin_unlock(&pipe->lock, key);
	}
	вернуть rc;
}

void z_impl_k_pipe_reset(struct k_pipe *pipe)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_pipe, reset, pipe);
	K_SPINLOCK(&pipe->lock) {
		ring_buf_reset(&pipe->buf);
		если (likely(pipe->waiting != 0)) {
			pipe->flags |= PIPE_FLAG_RESET;
			z_sched_wake_all(&pipe->data, 0, NULL);
			z_sched_wake_all(&pipe->space, 0, NULL);
		}
	}
	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_pipe, reset, pipe);
}

void z_impl_k_pipe_close(struct k_pipe *pipe)
{
	k_spinlock_key_t key;
	bool need_resched;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_pipe, close, pipe);

	key = k_spin_lock(&pipe->lock);

	pipe->flags = 0;
	need_resched = z_sched_wake_all(&pipe->data, 0, NULL);
	need_resched |= z_sched_wake_all(&pipe->space, 0, NULL);

#ifdef CONFIG_POLL
	/*
	 * Closing is a broadcast: wake every registered poller. Later
	 * k_poll() calls see the hangup through is_condition_met().
	 */
	пока (z_handle_obj_poll_events(&pipe->poll_events,
					K_POLL_STATE_PIPE_DATA_AVAILABLE)) {
		need_resched = true;
	}
#endif /* CONFIG_POLL */

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_pipe, close, pipe);

	если (need_resched) {
		z_reschedule(&pipe->lock, key);
	} иначе {
		k_spin_unlock(&pipe->lock, key);
	}
}

#ifdef CONFIG_USERSPACE
void z_vrfy_k_pipe_init(struct k_pipe *pipe, uint8_t *buffer, size_t buffer_size)
{
	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(pipe, K_OBJ_PIPE));
	K_OOPS(K_SYSCALL_MEMORY_WRITE(buffer, buffer_size));

	z_impl_k_pipe_init(pipe, buffer, buffer_size);
}
#include <zephyr/syscalls/k_pipe_init_mrsh.c>

целое z_vrfy_k_pipe_read(struct k_pipe *pipe, uint8_t *data, size_t len, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(pipe, K_OBJ_PIPE));
	K_OOPS(K_SYSCALL_MEMORY_WRITE(data, len));

	вернуть z_impl_k_pipe_read(pipe, data, len, timeout);
}
#include <zephyr/syscalls/k_pipe_read_mrsh.c>

целое z_vrfy_k_pipe_write(struct k_pipe *pipe, const uint8_t *data, size_t len, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(pipe, K_OBJ_PIPE));
	K_OOPS(K_SYSCALL_MEMORY_READ(data, len));

	вернуть z_impl_k_pipe_write(pipe, data, len, timeout);
}
#include <zephyr/syscalls/k_pipe_write_mrsh.c>

void z_vrfy_k_pipe_reset(struct k_pipe *pipe)
{
	K_OOPS(K_SYSCALL_OBJ(pipe, K_OBJ_PIPE));
	z_impl_k_pipe_reset(pipe);
}
#include <zephyr/syscalls/k_pipe_reset_mrsh.c>

void z_vrfy_k_pipe_close(struct k_pipe *pipe)
{
	K_OOPS(K_SYSCALL_OBJ(pipe, K_OBJ_PIPE));
	z_impl_k_pipe_close(pipe);
}
#include <zephyr/syscalls/k_pipe_close_mrsh.c>
#endif /* CONFIG_USERSPACE */
