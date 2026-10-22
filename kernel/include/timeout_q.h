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

#अन्तर्भाव <zephyr/kernel.h>

#अन्तर्भाव <stdbool.h>

#ifdef __cplusplus
बाह्य "C" {
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
#यदि defined(CONFIG_TIMEOUT_BACKEND_MINHEAP)

स्थिर अन्तःस्थ शून्य z_init_timeout(संरचना _timeout *to)
{
	to->heap_handle.idx = 0U;
	to->abs_ticks = 0;
}

स्थिर अन्तःस्थ bool z_is_inactive_timeout(ध्रुव संरचना _timeout *to)
{
	प्रत्यागम to->heap_handle.idx == 0U;
}

#नोचेद्यदि defined(CONFIG_TIMEOUT_BACKEND_WHEEL)

स्थिर अन्तःस्थ शून्य z_init_timeout(संरचना _timeout *to)
{
	sys_dnode_init(&to->गाठ);
	to->flags = 0U;
	to->dticks = 0;
}

स्थिर अन्तःस्थ bool z_is_inactive_timeout(ध्रुव संरचना _timeout *to)
{
	प्रत्यागम !sys_dnode_is_linked(&to->गाठ);
}

#नोचेद्यदि defined(CONFIG_TIMEOUT_BACKEND_SKIPLIST)

स्थिर अन्तःस्थ शून्य z_init_timeout(संरचना _timeout *to)
{
	uint8_t i;

	to->height = 0;
	to->abs_ticks = 0;

	प्रति (i = 0; i < CONFIG_TIMEOUT_SKIPLIST_MAX_LEVEL; i++) {
		to->forward[i] = रिक्त;
	}
}

स्थिर अन्तःस्थ bool z_is_inactive_timeout(ध्रुव संरचना _timeout *to)
{
	प्रत्यागम to->height == 0;
}

#अन्यथा /* CONFIG_TIMEOUT_BACKEND_DLIST or _BUCKET */

स्थिर अन्तःस्थ शून्य z_init_timeout(संरचना _timeout *to)
{
	sys_dnode_init(&to->गाठ);
	to->dticks = 0;
}

स्थिर अन्तःस्थ bool z_is_inactive_timeout(ध्रुव संरचना _timeout *to)
{
	प्रत्यागम !sys_dnode_is_linked(&to->गाठ);
}

#endif

/* Adds the timeout to the queue.
 *
 * @return Absolute tick value when timeout will expire.
 */
k_ticks_t z_add_timeout(संरचना _timeout *to, _timeout_func_t fn, k_timeout_t timeout);

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
पूर्णाङ्क z_try_abort_timeout(संरचना _timeout *to);

/* True if @to is the currently in-flight timeout and a same-CPU aborter
 * has marked the slot as superseded. Handlers with non-idempotent side
 * effects (e.g. k_timer's expiry_fn) should call this at entry, after
 * taking their own lock, and bail if it returns true.
 */
bool z_timeout_inflight_superseded(ध्रुव संरचना _timeout *to);

स्थिर अन्तःस्थ शून्य z_init_thread_timeout(संरचना _thread_base *thread_base)
{
	z_init_timeout(&thread_base->timeout);
}

बाह्य शून्य z_thread_timeout(संरचना _timeout *timeout);

स्थिर अन्तःस्थ k_ticks_t z_add_thread_timeout(संरचना k_thread *thread, k_timeout_t ticks)
{
	प्रत्यागम z_add_timeout(&thread->base.timeout, z_thread_timeout, ticks);
}

स्थिर अन्तःस्थ पूर्णाङ्क z_try_abort_thread_timeout(संरचना k_thread *thread)
{
	प्रत्यागम z_try_abort_timeout(&thread->base.timeout);
}

uint32_t z_get_next_timeout_expiry(शून्य);

k_ticks_t z_timeout_remaining(ध्रुव संरचना _timeout *timeout);

#अन्यथा

/* Stubs when !CONFIG_SYS_CLOCK_EXISTS */
#define z_init_thread_timeout(thread_base) कुरु {} यावत् (false)
#define z_try_abort_thread_timeout(to) 0
#define z_is_inactive_timeout(to) 1
#define z_get_next_timeout_expiry() ((uint32_t) K_TICKS_FOREVER)
#define z_set_timeout_expiry(ticks, is_idle) कुरु {} यावत् (false)

स्थिर अन्तःस्थ k_ticks_t z_add_thread_timeout(संरचना k_thread *thread, k_timeout_t ticks)
{
	ARG_UNUSED(thread);
	ARG_UNUSED(ticks);
	प्रत्यागम 0;
}

#endif /* CONFIG_SYS_CLOCK_EXISTS */

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_KERNEL_INCLUDE_TIMEOUT_Q_H_ */
