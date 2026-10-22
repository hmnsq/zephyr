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

#ಅನ್ತರ್ಭಾವ <zephyr/kernel.h>

#ಅನ್ತರ್ಭಾವ <stdbool.h>

#ifdef __cplusplus
ಬಾಹ್ಯ "C" {
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
#ಯದಿ defined(CONFIG_TIMEOUT_BACKEND_MINHEAP)

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ z_init_timeout(ಸಂರಚನಾ _timeout *to)
{
	to->heap_handle.idx = 0U;
	to->abs_ticks = 0;
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ bool z_is_inactive_timeout(ಧ್ರುವ ಸಂರಚನಾ _timeout *to)
{
	ಪ್ರತ್ಯಾಗಮ to->heap_handle.idx == 0U;
}

#ನೋಚೇದ್ಯದಿ defined(CONFIG_TIMEOUT_BACKEND_WHEEL)

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ z_init_timeout(ಸಂರಚನಾ _timeout *to)
{
	sys_dnode_init(&to->ಗಂಟು);
	to->flags = 0U;
	to->dticks = 0;
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ bool z_is_inactive_timeout(ಧ್ರುವ ಸಂರಚನಾ _timeout *to)
{
	ಪ್ರತ್ಯಾಗಮ !sys_dnode_is_linked(&to->ಗಂಟು);
}

#ನೋಚೇದ್ಯದಿ defined(CONFIG_TIMEOUT_BACKEND_SKIPLIST)

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ z_init_timeout(ಸಂರಚನಾ _timeout *to)
{
	uint8_t i;

	to->height = 0;
	to->abs_ticks = 0;

	ಪ್ರತಿ (i = 0; i < CONFIG_TIMEOUT_SKIPLIST_MAX_LEVEL; i++) {
		to->forward[i] = ರಿಕ್ತ;
	}
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ bool z_is_inactive_timeout(ಧ್ರುವ ಸಂರಚನಾ _timeout *to)
{
	ಪ್ರತ್ಯಾಗಮ to->height == 0;
}

#ಅನ್ಯಥಾ /* CONFIG_TIMEOUT_BACKEND_DLIST or _BUCKET */

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ z_init_timeout(ಸಂರಚನಾ _timeout *to)
{
	sys_dnode_init(&to->ಗಂಟು);
	to->dticks = 0;
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ bool z_is_inactive_timeout(ಧ್ರುವ ಸಂರಚನಾ _timeout *to)
{
	ಪ್ರತ್ಯಾಗಮ !sys_dnode_is_linked(&to->ಗಂಟು);
}

#endif

/* Adds the timeout to the queue.
 *
 * @return Absolute tick value when timeout will expire.
 */
k_ticks_t z_add_timeout(ಸಂರಚನಾ _timeout *to, _timeout_func_t fn, k_timeout_t timeout);

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
ಪೂರ್ಣಾಙ್ಕ z_try_abort_timeout(ಸಂರಚನಾ _timeout *to);

/* True if @to is the currently in-flight timeout and a same-CPU aborter
 * has marked the slot as superseded. Handlers with non-idempotent side
 * effects (e.g. k_timer's expiry_fn) should call this at entry, after
 * taking their own lock, and bail if it returns true.
 */
bool z_timeout_inflight_superseded(ಧ್ರುವ ಸಂರಚನಾ _timeout *to);

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ z_init_thread_timeout(ಸಂರಚನಾ _thread_base *thread_base)
{
	z_init_timeout(&thread_base->timeout);
}

ಬಾಹ್ಯ ಶೂನ್ಯ z_thread_timeout(ಸಂರಚನಾ _timeout *timeout);

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ k_ticks_t z_add_thread_timeout(ಸಂರಚನಾ k_thread *thread, k_timeout_t ticks)
{
	ಪ್ರತ್ಯಾಗಮ z_add_timeout(&thread->base.timeout, z_thread_timeout, ticks);
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಪೂರ್ಣಾಙ್ಕ z_try_abort_thread_timeout(ಸಂರಚನಾ k_thread *thread)
{
	ಪ್ರತ್ಯಾಗಮ z_try_abort_timeout(&thread->base.timeout);
}

uint32_t z_get_next_timeout_expiry(ಶೂನ್ಯ);

k_ticks_t z_timeout_remaining(ಧ್ರುವ ಸಂರಚನಾ _timeout *timeout);

#ಅನ್ಯಥಾ

/* Stubs when !CONFIG_SYS_CLOCK_EXISTS */
#define z_init_thread_timeout(thread_base) ಕುರು {} ಯಾವತ್ (false)
#define z_try_abort_thread_timeout(to) 0
#define z_is_inactive_timeout(to) 1
#define z_get_next_timeout_expiry() ((uint32_t) K_TICKS_FOREVER)
#define z_set_timeout_expiry(ticks, is_idle) ಕುರು {} ಯಾವತ್ (false)

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ k_ticks_t z_add_thread_timeout(ಸಂರಚನಾ k_thread *thread, k_timeout_t ticks)
{
	ARG_UNUSED(thread);
	ARG_UNUSED(ticks);
	ಪ್ರತ್ಯಾಗಮ 0;
}

#endif /* CONFIG_SYS_CLOCK_EXISTS */

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_KERNEL_INCLUDE_TIMEOUT_Q_H_ */
