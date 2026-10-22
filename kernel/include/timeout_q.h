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

#ضمن <zephyr/kernel.h>

#ضمن <stdbool.h>

#ifdef __cplusplus
خارجي "C" {
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
#إذا defined(CONFIG_TIMEOUT_BACKEND_MINHEAP)

ساكن مضمن فراغ z_init_timeout(بنية _timeout *to)
{
	to->heap_handle.idx = 0U;
	to->abs_ticks = 0;
}

ساكن مضمن bool z_is_inactive_timeout(ثابت بنية _timeout *to)
{
	أرجع to->heap_handle.idx == 0U;
}

#وإلاإذا defined(CONFIG_TIMEOUT_BACKEND_WHEEL)

ساكن مضمن فراغ z_init_timeout(بنية _timeout *to)
{
	sys_dnode_init(&to->گرێ);
	to->flags = 0U;
	to->dticks = 0;
}

ساكن مضمن bool z_is_inactive_timeout(ثابت بنية _timeout *to)
{
	أرجع !sys_dnode_is_linked(&to->گرێ);
}

#وإلاإذا defined(CONFIG_TIMEOUT_BACKEND_SKIPLIST)

ساكن مضمن فراغ z_init_timeout(بنية _timeout *to)
{
	uint8_t i;

	to->height = 0;
	to->abs_ticks = 0;

	لكل (i = 0; i < CONFIG_TIMEOUT_SKIPLIST_MAX_LEVEL; i++) {
		to->forward[i] = عدم;
	}
}

ساكن مضمن bool z_is_inactive_timeout(ثابت بنية _timeout *to)
{
	أرجع to->height == 0;
}

#وإلا /* CONFIG_TIMEOUT_BACKEND_DLIST or _BUCKET */

ساكن مضمن فراغ z_init_timeout(بنية _timeout *to)
{
	sys_dnode_init(&to->گرێ);
	to->dticks = 0;
}

ساكن مضمن bool z_is_inactive_timeout(ثابت بنية _timeout *to)
{
	أرجع !sys_dnode_is_linked(&to->گرێ);
}

#endif

/* Adds the timeout to the queue.
 *
 * @return Absolute tick value when timeout will expire.
 */
k_ticks_t z_add_timeout(بنية _timeout *to, _timeout_func_t fn, k_timeout_t timeout);

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
صحيح z_try_abort_timeout(بنية _timeout *to);

/* True if @to is the currently in-flight timeout and a same-CPU aborter
 * has marked the slot as superseded. Handlers with non-idempotent side
 * effects (e.g. k_timer's expiry_fn) should call this at entry, after
 * taking their own lock, and bail if it returns true.
 */
bool z_timeout_inflight_superseded(ثابت بنية _timeout *to);

ساكن مضمن فراغ z_init_thread_timeout(بنية _thread_base *thread_base)
{
	z_init_timeout(&thread_base->timeout);
}

خارجي فراغ z_thread_timeout(بنية _timeout *timeout);

ساكن مضمن k_ticks_t z_add_thread_timeout(بنية k_thread *thread, k_timeout_t ticks)
{
	أرجع z_add_timeout(&thread->base.timeout, z_thread_timeout, ticks);
}

ساكن مضمن صحيح z_try_abort_thread_timeout(بنية k_thread *thread)
{
	أرجع z_try_abort_timeout(&thread->base.timeout);
}

uint32_t z_get_next_timeout_expiry(فراغ);

k_ticks_t z_timeout_remaining(ثابت بنية _timeout *timeout);

#وإلا

/* Stubs when !CONFIG_SYS_CLOCK_EXISTS */
#define z_init_thread_timeout(thread_base) افعل {} طالما (false)
#define z_try_abort_thread_timeout(to) 0
#define z_is_inactive_timeout(to) 1
#define z_get_next_timeout_expiry() ((uint32_t) K_TICKS_FOREVER)
#define z_set_timeout_expiry(ticks, is_idle) افعل {} طالما (false)

ساكن مضمن k_ticks_t z_add_thread_timeout(بنية k_thread *thread, k_timeout_t ticks)
{
	ARG_UNUSED(thread);
	ARG_UNUSED(ticks);
	أرجع 0;
}

#endif /* CONFIG_SYS_CLOCK_EXISTS */

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_KERNEL_INCLUDE_TIMEOUT_Q_H_ */
