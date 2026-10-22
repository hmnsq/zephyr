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

#அந்தர்பாவ <zephyr/kernel.h>

#அந்தர்பாவ <stdbool.h>

#ifdef __cplusplus
வெளி "C" {
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
#எனில் defined(CONFIG_TIMEOUT_BACKEND_MINHEAP)

நிலையான உள்வரி வெறுமை z_init_timeout(கட்டமைப்பு _timeout *to)
{
	to->heap_handle.idx = 0U;
	to->abs_ticks = 0;
}

நிலையான உள்வரி bool z_is_inactive_timeout(மாறிலி கட்டமைப்பு _timeout *to)
{
	திரும்பு to->heap_handle.idx == 0U;
}

#அல்லதெனில் defined(CONFIG_TIMEOUT_BACKEND_WHEEL)

நிலையான உள்வரி வெறுமை z_init_timeout(கட்டமைப்பு _timeout *to)
{
	sys_dnode_init(&to->முனை);
	to->flags = 0U;
	to->dticks = 0;
}

நிலையான உள்வரி bool z_is_inactive_timeout(மாறிலி கட்டமைப்பு _timeout *to)
{
	திரும்பு !sys_dnode_is_linked(&to->முனை);
}

#அல்லதெனில் defined(CONFIG_TIMEOUT_BACKEND_SKIPLIST)

நிலையான உள்வரி வெறுமை z_init_timeout(கட்டமைப்பு _timeout *to)
{
	uint8_t i;

	to->height = 0;
	to->abs_ticks = 0;

	க்கு (i = 0; i < CONFIG_TIMEOUT_SKIPLIST_MAX_LEVEL; i++) {
		to->forward[i] = வெற்று;
	}
}

நிலையான உள்வரி bool z_is_inactive_timeout(மாறிலி கட்டமைப்பு _timeout *to)
{
	திரும்பு to->height == 0;
}

#இல்லையெனில் /* CONFIG_TIMEOUT_BACKEND_DLIST or _BUCKET */

நிலையான உள்வரி வெறுமை z_init_timeout(கட்டமைப்பு _timeout *to)
{
	sys_dnode_init(&to->முனை);
	to->dticks = 0;
}

நிலையான உள்வரி bool z_is_inactive_timeout(மாறிலி கட்டமைப்பு _timeout *to)
{
	திரும்பு !sys_dnode_is_linked(&to->முனை);
}

#endif

/* Adds the timeout to the queue.
 *
 * @return Absolute tick value when timeout will expire.
 */
k_ticks_t z_add_timeout(கட்டமைப்பு _timeout *to, _timeout_func_t fn, k_timeout_t timeout);

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
முழு z_try_abort_timeout(கட்டமைப்பு _timeout *to);

/* True if @to is the currently in-flight timeout and a same-CPU aborter
 * has marked the slot as superseded. Handlers with non-idempotent side
 * effects (e.g. k_timer's expiry_fn) should call this at entry, after
 * taking their own lock, and bail if it returns true.
 */
bool z_timeout_inflight_superseded(மாறிலி கட்டமைப்பு _timeout *to);

நிலையான உள்வரி வெறுமை z_init_thread_timeout(கட்டமைப்பு _thread_base *thread_base)
{
	z_init_timeout(&thread_base->timeout);
}

வெளி வெறுமை z_thread_timeout(கட்டமைப்பு _timeout *timeout);

நிலையான உள்வரி k_ticks_t z_add_thread_timeout(கட்டமைப்பு k_thread *thread, k_timeout_t ticks)
{
	திரும்பு z_add_timeout(&thread->base.timeout, z_thread_timeout, ticks);
}

நிலையான உள்வரி முழு z_try_abort_thread_timeout(கட்டமைப்பு k_thread *thread)
{
	திரும்பு z_try_abort_timeout(&thread->base.timeout);
}

uint32_t z_get_next_timeout_expiry(வெறுமை);

k_ticks_t z_timeout_remaining(மாறிலி கட்டமைப்பு _timeout *timeout);

#இல்லையெனில்

/* Stubs when !CONFIG_SYS_CLOCK_EXISTS */
#define z_init_thread_timeout(thread_base) செய் {} போது (false)
#define z_try_abort_thread_timeout(to) 0
#define z_is_inactive_timeout(to) 1
#define z_get_next_timeout_expiry() ((uint32_t) K_TICKS_FOREVER)
#define z_set_timeout_expiry(ticks, is_idle) செய் {} போது (false)

நிலையான உள்வரி k_ticks_t z_add_thread_timeout(கட்டமைப்பு k_thread *thread, k_timeout_t ticks)
{
	ARG_UNUSED(thread);
	ARG_UNUSED(ticks);
	திரும்பு 0;
}

#endif /* CONFIG_SYS_CLOCK_EXISTS */

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_KERNEL_INCLUDE_TIMEOUT_Q_H_ */
