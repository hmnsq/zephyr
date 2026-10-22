/*
 * Copyright (c) 2015 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#که_نه_تعریف_شوی ZEPHYR_KERNEL_INCLUDE_TIMEOUT_Q_H_
#تعریف_کړه ZEPHYR_KERNEL_INCLUDE_TIMEOUT_Q_H_

/**
 * @file
 * @brief timeout queue for threads on kernel objects
 */

#ضمن <zephyr/kernel.h>

#ضمن <stdbool.h>

#که_تعریف_شوی __cplusplus
خارجي "C" {
#د_که_پای

#که_تعریف_شوی CONFIG_SYS_CLOCK_EXISTS

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

ساكن مضمن فراغ z_init_timeout(بنية _timeout *منزل)
{
	منزل->ډېرۍ_handle.idx = 0U;
	منزل->abs_ticks = 0;
}

ساكن مضمن bool z_is_inactive_timeout(ثابت بنية _timeout *منزل)
{
	أرجع منزل->ډېرۍ_handle.idx == 0U;
}

#وإلاإذا defined(CONFIG_TIMEOUT_BACKEND_WHEEL)

ساكن مضمن فراغ z_init_timeout(بنية _timeout *منزل)
{
	sys_dnode_init(&منزل->غوټه);
	منزل->flags = 0U;
	منزل->dticks = 0;
}

ساكن مضمن bool z_is_inactive_timeout(ثابت بنية _timeout *منزل)
{
	أرجع !sys_dnode_is_linked(&منزل->غوټه);
}

#وإلاإذا defined(CONFIG_TIMEOUT_BACKEND_SKIPLIST)

ساكن مضمن فراغ z_init_timeout(بنية _timeout *منزل)
{
	uint8_t i;

	منزل->لوړوالی = 0;
	منزل->abs_ticks = 0;

	لكل (i = 0; i < CONFIG_TIMEOUT_SKIPLIST_MAX_LEVEL; i++) {
		منزل->forward[i] = عدم;
	}
}

ساكن مضمن bool z_is_inactive_timeout(ثابت بنية _timeout *منزل)
{
	أرجع منزل->لوړوالی == 0;
}

#وإلا /* CONFIG_TIMEOUT_BACKEND_DLIST or _BUCKET */

ساكن مضمن فراغ z_init_timeout(بنية _timeout *منزل)
{
	sys_dnode_init(&منزل->غوټه);
	منزل->dticks = 0;
}

ساكن مضمن bool z_is_inactive_timeout(ثابت بنية _timeout *منزل)
{
	أرجع !sys_dnode_is_linked(&منزل->غوټه);
}

#د_که_پای

/* Adds the timeout to the queue.
 *
 * @return Absolute tick value when timeout will expire.
 */
k_ticks_t z_جمع_کړه_timeout(بنية _timeout *منزل, _timeout_func_t fn, k_timeout_t timeout);

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
صحيح z_try_abort_timeout(بنية _timeout *منزل);

/* True if @to is the currently in-flight timeout and a same-CPU aborter
 * has marked the slot as superseded. Handlers with non-idempotent side
 * effects (e.g. k_timer's expiry_fn) should call this at entry, after
 * taking their own lock, and bail if it returns true.
 */
bool z_timeout_inflight_superseded(ثابت بنية _timeout *منزل);

ساكن مضمن فراغ z_init_thread_timeout(بنية thread_بنسټ *thread_بنسټ_2)
{
	z_init_timeout(&thread_بنسټ_2->timeout);
}

خارجي فراغ z_thread_timeout(بنية _timeout *timeout);

ساكن مضمن k_ticks_t z_جمع_کړه_thread_timeout(بنية k_thread *thread, k_timeout_t ticks)
{
	أرجع z_جمع_کړه_timeout(&thread->بنسټ.timeout, z_thread_timeout, ticks);
}

ساكن مضمن صحيح z_try_abort_thread_timeout(بنية k_thread *thread)
{
	أرجع z_try_abort_timeout(&thread->بنسټ.timeout);
}

uint32_t z_get_بل_timeout_expiry(فراغ);

k_ticks_t z_timeout_پاتې_شوی(ثابت بنية _timeout *timeout);

#وإلا

/* Stubs when !CONFIG_SYS_CLOCK_EXISTS */
#تعریف_کړه z_init_thread_timeout(thread_بنسټ_2) افعل {} طالما (false)
#تعریف_کړه z_try_abort_thread_timeout(منزل) 0
#تعریف_کړه z_is_inactive_timeout(منزل) 1
#تعریف_کړه z_get_بل_timeout_expiry() ((uint32_t) K_TICKS_FOREVER)
#تعریف_کړه z_سېټ_timeout_expiry(ticks, is_idle) افعل {} طالما (false)

ساكن مضمن k_ticks_t z_جمع_کړه_thread_timeout(بنية k_thread *thread, k_timeout_t ticks)
{
	ARG_ناکارول_شوی(thread);
	ARG_ناکارول_شوی(ticks);
	أرجع 0;
}

#د_که_پای /* CONFIG_SYS_CLOCK_EXISTS */

#که_تعریف_شوی __cplusplus
}
#د_که_پای

#د_که_پای /* ZEPHYR_KERNEL_INCLUDE_TIMEOUT_Q_H_ */
