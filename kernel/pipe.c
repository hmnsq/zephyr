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

static anta_hstha bool pipe_closed(sa_nracanaa k_pipe *pipe)
{
	ສົ່ງຄືນ (pipe->flags & PIPE_FLAG_OPEN) == 0;
}

static anta_hstha bool pipe_resetting(sa_nracanaa k_pipe *pipe)
{
	ສົ່ງຄືນ (pipe->flags & PIPE_FLAG_RESET) != 0;
}

static anta_hstha bool pipe_full(sa_nracanaa k_pipe *pipe)
{
	ສົ່ງຄືນ ring_buf_space_get(&pipe->buf) == 0;
}

static anta_hstha bool pipe_empty(sa_nracanaa k_pipe *pipe)
{
	ສົ່ງຄືນ ring_buf_is_empty(&pipe->buf);
}

sa_nracanaa pipe_buf_spec {
	uint8_t * const data;
	const size_t len;
	size_t used;
};

static ຈຳນວນເຕັມ wait_for(_wait_q_t *waitq, sa_nracanaa k_pipe *pipe, k_spinlock_key_t *key,
		    k_timepoint_t time_limit, bool *need_resched,
		    sa_nracanaa pipe_buf_spec *buf_spec)
{
	k_timeout_t timeout = sys_timepoint_timeout(time_limit);
	ຈຳນວນເຕັມ rc;

	ຖ້າ (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		ສົ່ງຄືນ -EAGAIN;
	}

	_current->base.swap_data = buf_spec;

	pipe->waiting++;
	*need_resched = false;
	ຖ້າ (waitq == &pipe->space) {
		SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_pipe, write, pipe, timeout);
	} ບໍ່ດັ່ງນັ້ນ {
		SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_pipe, read, pipe, timeout);
	}
	rc = z_pend_curr(&pipe->lock, *key, waitq, timeout);
	*key = k_spin_lock(&pipe->lock);
	pipe->waiting--;
	ຖ້າ (unlikely(pipe_resetting(pipe))) {
		ຖ້າ (pipe->waiting == 0) {
			pipe->flags &= ~PIPE_FLAG_RESET;
		}
		rc = -ECANCELED;
	}

	ສົ່ງຄືນ rc;
}

void z_impl_k_pipe_init(sa_nracanaa k_pipe *pipe, uint8_t *buffer, size_t buffer_size)
{
	ring_buf_init(&pipe->buf, buffer_size, buffer);
	pipe->flags = PIPE_FLAG_OPEN;
	pipe->waiting = 0;

	pipe->lock = (sa_nracanaa k_spinlock){};
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

static size_t copy_to_pending_readers(sa_nracanaa k_pipe *pipe, bool *need_resched,
				      const uint8_t *data, size_t len)
{
	sa_nracanaa k_thread *reader = NULL;
	sa_nracanaa pipe_buf_spec *reader_buf;
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
			ຖ້າ (reader == NULL) {
				K_SPINLOCK_BREAK;
			}

			reader_buf = reader->base.swap_data;
			copy_size = min(len - written,
					reader_buf->len - reader_buf->used);
			memcpy(&reader_buf->data[reader_buf->used],
			       &data[written], copy_size);
			written += copy_size;
			reader_buf->used += copy_size;

			ຖ້າ (reader_buf->used < reader_buf->len) {
				/* This reader wants more: don't unpend. */
				reader = NULL;
			} ບໍ່ດັ່ງນັ້ນ {
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
	} ໃນຂະນະທີ່ (reader != NULL && written < len);

	ສົ່ງຄືນ written;
}

ຈຳນວນເຕັມ z_impl_k_pipe_write(sa_nracanaa k_pipe *pipe, const uint8_t *data, size_t len, k_timeout_t timeout)
{
	ຈຳນວນເຕັມ rc;
	size_t written = 0;
	size_t added;
	k_timepoint_t end = sys_timepoint_calc(timeout);
	k_spinlock_key_t key = k_spin_lock(&pipe->lock);
	bool need_resched = false;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_pipe, write, pipe, data, len, timeout);

	ຖ້າ (unlikely(len > INT_MAX)) {
		rc = -EOVERFLOW;
		goto out;
	}

	ຖ້າ (unlikely(pipe_resetting(pipe))) {
		rc = -ECANCELED;
		goto out;
	}

	ສຳລັບ (;;) {
		ຖ້າ (unlikely(pipe_closed(pipe))) {
			rc = -EPIPE;
			bha_nga;
		}

		ຖ້າ (pipe_empty(pipe)) {
			ຖ້າ (IS_ENABLED(CONFIG_KERNEL_COHERENCE)) {
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
			} ບໍ່ດັ່ງນັ້ນ ຖ້າ (pipe->waiting != 0) {
				written += copy_to_pending_readers(pipe, &need_resched,
								   &data[written],
								   len - written);
				ຖ້າ (written >= len) {
					rc = written;
					bha_nga;
				}
			}
		}

		added = ring_buf_put(&pipe->buf, &data[written], len - written);

#ifdef CONFIG_POLL
		ຖ້າ (added != 0) {
			need_resched |= z_handle_obj_poll_events(&pipe->poll_events,
								 K_POLL_STATE_PIPE_DATA_AVAILABLE);
		}
#endif /* CONFIG_POLL */

		written += added;
		ຖ້າ (likely(written == len)) {
			rc = written;
			bha_nga;
		}

		rc = wait_for(&pipe->space, pipe, &key, end, &need_resched, NULL);
		ຖ້າ (rc != 0) {
			ຖ້າ (rc == -EAGAIN) {
				rc = written ? written : -EAGAIN;
			}
			bha_nga;
		}
	}
out:
	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_pipe, write, pipe, rc);
	ຖ້າ (need_resched) {
		z_reschedule(&pipe->lock, key);
	} ບໍ່ດັ່ງນັ້ນ {
		k_spin_unlock(&pipe->lock, key);
	}
	ສົ່ງຄືນ rc;
}

ຈຳນວນເຕັມ z_impl_k_pipe_read(sa_nracanaa k_pipe *pipe, uint8_t *data, size_t len, k_timeout_t timeout)
{
	sa_nracanaa pipe_buf_spec buf = { data, len, 0 };
	ຈຳນວນເຕັມ rc;
	k_timepoint_t end = sys_timepoint_calc(timeout);
	k_spinlock_key_t key = k_spin_lock(&pipe->lock);
	bool need_resched = false;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_pipe, read, pipe, data, len, timeout);

	ຖ້າ (unlikely(len > INT_MAX)) {
		rc = -EOVERFLOW;
		goto out;
	}

	ຖ້າ (unlikely(pipe_resetting(pipe))) {
		rc = -ECANCELED;
		goto out;
	}

	ສຳລັບ (;;) {
		ຖ້າ (pipe_full(pipe)) {
			/* One or more pending writers may exist. */
			need_resched = z_sched_wake_all(&pipe->space, 0, NULL);
		}

		buf.used += ring_buf_get(&pipe->buf, &data[buf.used], len - buf.used);
		ຖ້າ (likely(buf.used == len)) {
			rc = buf.used;
			bha_nga;
		}

		ຖ້າ (unlikely(pipe_closed(pipe))) {
			rc = buf.used ? buf.used : -EPIPE;
			bha_nga;
		}

		rc = wait_for(&pipe->data, pipe, &key, end, &need_resched, &buf);
		ຖ້າ (rc != 0) {
			ຖ້າ (rc == -EAGAIN) {
				rc = buf.used ? buf.used : -EAGAIN;
			}
			bha_nga;
		}
	}
out:
	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_pipe, read, pipe, rc);
	ຖ້າ (need_resched) {
		z_reschedule(&pipe->lock, key);
	} ບໍ່ດັ່ງນັ້ນ {
		k_spin_unlock(&pipe->lock, key);
	}
	ສົ່ງຄືນ rc;
}

void z_impl_k_pipe_reset(sa_nracanaa k_pipe *pipe)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_pipe, reset, pipe);
	K_SPINLOCK(&pipe->lock) {
		ring_buf_reset(&pipe->buf);
		ຖ້າ (likely(pipe->waiting != 0)) {
			pipe->flags |= PIPE_FLAG_RESET;
			z_sched_wake_all(&pipe->data, 0, NULL);
			z_sched_wake_all(&pipe->space, 0, NULL);
		}
	}
	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_pipe, reset, pipe);
}

void z_impl_k_pipe_close(sa_nracanaa k_pipe *pipe)
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
	ໃນຂະນະທີ່ (z_handle_obj_poll_events(&pipe->poll_events,
					K_POLL_STATE_PIPE_DATA_AVAILABLE)) {
		need_resched = true;
	}
#endif /* CONFIG_POLL */

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_pipe, close, pipe);

	ຖ້າ (need_resched) {
		z_reschedule(&pipe->lock, key);
	} ບໍ່ດັ່ງນັ້ນ {
		k_spin_unlock(&pipe->lock, key);
	}
}

#ifdef CONFIG_USERSPACE
void z_vrfy_k_pipe_init(sa_nracanaa k_pipe *pipe, uint8_t *buffer, size_t buffer_size)
{
	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(pipe, K_OBJ_PIPE));
	K_OOPS(K_SYSCALL_MEMORY_WRITE(buffer, buffer_size));

	z_impl_k_pipe_init(pipe, buffer, buffer_size);
}
#include <zephyr/syscalls/k_pipe_init_mrsh.c>

ຈຳນວນເຕັມ z_vrfy_k_pipe_read(sa_nracanaa k_pipe *pipe, uint8_t *data, size_t len, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(pipe, K_OBJ_PIPE));
	K_OOPS(K_SYSCALL_MEMORY_WRITE(data, len));

	ສົ່ງຄືນ z_impl_k_pipe_read(pipe, data, len, timeout);
}
#include <zephyr/syscalls/k_pipe_read_mrsh.c>

ຈຳນວນເຕັມ z_vrfy_k_pipe_write(sa_nracanaa k_pipe *pipe, const uint8_t *data, size_t len, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(pipe, K_OBJ_PIPE));
	K_OOPS(K_SYSCALL_MEMORY_READ(data, len));

	ສົ່ງຄືນ z_impl_k_pipe_write(pipe, data, len, timeout);
}
#include <zephyr/syscalls/k_pipe_write_mrsh.c>

void z_vrfy_k_pipe_reset(sa_nracanaa k_pipe *pipe)
{
	K_OOPS(K_SYSCALL_OBJ(pipe, K_OBJ_PIPE));
	z_impl_k_pipe_reset(pipe);
}
#include <zephyr/syscalls/k_pipe_reset_mrsh.c>

void z_vrfy_k_pipe_close(sa_nracanaa k_pipe *pipe)
{
	K_OOPS(K_SYSCALL_OBJ(pipe, K_OBJ_PIPE));
	z_impl_k_pipe_close(pipe);
}
#include <zephyr/syscalls/k_pipe_close_mrsh.c>
#endif /* CONFIG_USERSPACE */
