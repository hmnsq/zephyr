/*
 * Copyright (c) 2015 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_KERNEL_INCLUDE_TIMEOUT_Q_H_
#define ZEPHYR_KERNEL_INCLUDE_TIMEOUT_Q_H_

/**
 * @file
 * @brief timeout queue for threads on kernel objects
 */

#شامل <zephyr/kernel.h>

#شامل <stdbool.h>

#ifdef __cplusplus
بیرونی "C" {
#endif

#ifdef CONFIG_SYS_CLOCK_EXISTS

/*
 * Per-node timeout helpers (Kconfig choice TIMEOUT_BACKEND).
 *
 * z_init_timeout() and z_is_inactive_timeout() operate on a single
 * struct _timeout and are needed tree-wide (timer.c, work.c, poll.c, ...),
 * so they live here and depend only on the chosen backend's struct _timeout
 * fields (see kernel_structs.h). The queue itself (the z_timeout_q_*()
 * operations and the backend instance) is private to kernel/timeout.c, which
 * includes the matching backend implementation header directly.
 */
#اگر defined(CONFIG_TIMEOUT_BACKEND_MINHEAP)

ایستا درونخطی تهی z_init_timeout(ساختار _timeout *to)
{
	to->heap_handle.idx = 0U;
	to->abs_ticks = 0;
}

ایستا درونخطی bool z_is_inactive_timeout(ثابت ساختار _timeout *to)
{
	بازگشت to->heap_handle.idx == 0U;
}

#وگرنهاگر defined(CONFIG_TIMEOUT_BACKEND_WHEEL)

ایستا درونخطی تهی z_init_timeout(ساختار _timeout *to)
{
	sys_dnode_init(&to->گره);
	to->flags = 0U;
	to->dticks = 0;
}

ایستا درونخطی bool z_is_inactive_timeout(ثابت ساختار _timeout *to)
{
	بازگشت !sys_dnode_is_linked(&to->گره);
}

#وگرنهاگر defined(CONFIG_TIMEOUT_BACKEND_SKIPLIST)

ایستا درونخطی تهی z_init_timeout(ساختار _timeout *to)
{
	uint8_t i;

	to->height = 0;
	to->abs_ticks = 0;

	برای (i = 0; i < CONFIG_TIMEOUT_SKIPLIST_MAX_LEVEL; i++) {
		to->forward[i] = پوچ;
	}
}

ایستا درونخطی bool z_is_inactive_timeout(ثابت ساختار _timeout *to)
{
	بازگشت to->height == 0;
}

#وگرنه /* CONFIG_TIMEOUT_BACKEND_DLIST or _BUCKET */

ایستا درونخطی تهی z_init_timeout(ساختار _timeout *to)
{
	sys_dnode_init(&to->گره);
	to->dticks = 0;
}

ایستا درونخطی bool z_is_inactive_timeout(ثابت ساختار _timeout *to)
{
	بازگشت !sys_dnode_is_linked(&to->گره);
}

#endif

/* Adds the timeout to the queue.
 *
 * @return Absolute tick value when timeout will expire.
 */
k_ticks_t z_add_timeout(ساختار _timeout *to, _timeout_func_t fn, k_timeout_t timeout);

/* Attempt to abort a timeout. Safe to use for callers that need to be
 * certain the handler is not in flight (e.g. when about to free the
 * timeout's storage), as long as the caller is prepared to drop any
 * locks the handler may need and retry.
 *
 * Returns:
 *   0       — the timeout was active and has been removed from the queue.
 *   -EINVAL — the timeout was not active (never linked, already done, or
 *             a same-CPU IRQ aborted while the handler was paused on this
 *             CPU; in that case the in-flight slot is marked superseded
 *             so handlers that need to bail may check via
 *             z_timeout_inflight_superseded()).
 *   -EAGAIN — the handler is currently in flight on another CPU. The caller
 *             must drop any lock that the handler may need and retry. A short
 *             arch_spin_relax() has already been done before this return.
 *
 * On -EAGAIN, the only correct response is to drop outer locks and call again.
 * After a non-EAGAIN return, the handler is guaranteed not to be in flight.
 */
صحیح z_try_abort_timeout(ساختار _timeout *to);

/* True if @to is the currently in-flight timeout and a same-CPU aborter
 * has marked the slot as superseded. Handlers with non-idempotent side
 * effects (e.g. k_timer's expiry_fn) should call this at entry, after
 * taking their own lock, and bail if it returns true.
 */
bool z_timeout_inflight_superseded(ثابت ساختار _timeout *to);

ایستا درونخطی تهی z_init_thread_timeout(ساختار _thread_base *thread_base)
{
	z_init_timeout(&thread_base->timeout);
}

بیرونی تهی z_thread_timeout(ساختار _timeout *timeout);

ایستا درونخطی k_ticks_t z_add_thread_timeout(ساختار k_thread *thread, k_timeout_t ticks)
{
	بازگشت z_add_timeout(&thread->base.timeout, z_thread_timeout, ticks);
}

ایستا درونخطی صحیح z_try_abort_thread_timeout(ساختار k_thread *thread)
{
	بازگشت z_try_abort_timeout(&thread->base.timeout);
}

uint32_t z_get_next_timeout_expiry(تهی);

k_ticks_t z_timeout_remaining(ثابت ساختار _timeout *timeout);

#وگرنه

/* Stubs when !CONFIG_SYS_CLOCK_EXISTS */
#define z_init_thread_timeout(thread_base) انجامبده {} وقتی (false)
#define z_try_abort_thread_timeout(to) 0
#define z_is_inactive_timeout(to) 1
#define z_get_next_timeout_expiry() ((uint32_t) K_TICKS_FOREVER)
#define z_set_timeout_expiry(ticks, is_idle) انجامبده {} وقتی (false)

ایستا درونخطی k_ticks_t z_add_thread_timeout(ساختار k_thread *thread, k_timeout_t ticks)
{
	ARG_UNUSED(thread);
	ARG_UNUSED(ticks);
	بازگشت 0;
}

#endif /* CONFIG_SYS_CLOCK_EXISTS */

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_KERNEL_INCLUDE_TIMEOUT_Q_H_ */
