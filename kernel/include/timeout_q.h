/*
 * Copyright (c) 2015 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#جيڪڏهن_وصف_نه_ٿيل ZEPHYR_KERNEL_INCLUDE_TIMEOUT_Q_H_
#وصف_ڪريو ZEPHYR_KERNEL_INCLUDE_TIMEOUT_Q_H_

/**
 * @file
 * @brief timeout queue for threads on kernel objects
 */

#ضمن <zephyr/kernel.h>

#ضمن <stdbool.h>

#جيڪڏهن_وصف_ٿيل __cplusplus
خارجي "C" {
#جيڪڏهن_پورو

#جيڪڏهن_وصف_ٿيل CONFIG_SYS_CLOCK_EXISTS

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
	منزل->ڍير_handle.idx = 0U;
	منزل->abs_ticks = 0;
}

ساكن مضمن bool z_is_inactive_timeout(ثابت بنية _timeout *منزل)
{
	أرجع منزل->ڍير_handle.idx == 0U;
}

#وإلاإذا defined(CONFIG_TIMEOUT_BACKEND_WHEEL)

ساكن مضمن فراغ z_init_timeout(بنية _timeout *منزل)
{
	sys_dnode_init(&منزل->ڳنڍ);
	منزل->flags = 0U;
	منزل->dticks = 0;
}

ساكن مضمن bool z_is_inactive_timeout(ثابت بنية _timeout *منزل)
{
	أرجع !sys_dnode_is_linked(&منزل->ڳنڍ);
}

#وإلاإذا defined(CONFIG_TIMEOUT_BACKEND_SKIPLIST)

ساكن مضمن فراغ z_init_timeout(بنية _timeout *منزل)
{
	uint8_t i;

	منزل->اوچائي = 0;
	منزل->abs_ticks = 0;

	لكل (i = 0; i < CONFIG_TIMEOUT_SKIPLIST_MAX_LEVEL; i++) {
		منزل->forward[i] = عدم;
	}
}

ساكن مضمن bool z_is_inactive_timeout(ثابت بنية _timeout *منزل)
{
	أرجع منزل->اوچائي == 0;
}

#وإلا /* CONFIG_TIMEOUT_BACKEND_DLIST or _BUCKET */

ساكن مضمن فراغ z_init_timeout(بنية _timeout *منزل)
{
	sys_dnode_init(&منزل->ڳنڍ);
	منزل->dticks = 0;
}

ساكن مضمن bool z_is_inactive_timeout(ثابت بنية _timeout *منزل)
{
	أرجع !sys_dnode_is_linked(&منزل->ڳنڍ);
}

#جيڪڏهن_پورو

/* Adds the timeout to the queue.
 *
 * @return Absolute tick value when timeout will expire.
 */
k_ticks_t z_جوڙيو_timeout(بنية _timeout *منزل, _timeout_func_t fn, k_timeout_t timeout);

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

ساكن مضمن فراغ z_init_thread_timeout(بنية thread_بنياد *thread_بنياد_2)
{
	z_init_timeout(&thread_بنياد_2->timeout);
}

خارجي فراغ z_thread_timeout(بنية _timeout *timeout);

ساكن مضمن k_ticks_t z_جوڙيو_thread_timeout(بنية k_thread *thread, k_timeout_t ticks)
{
	أرجع z_جوڙيو_timeout(&thread->بنياد.timeout, z_thread_timeout, ticks);
}

ساكن مضمن صحيح z_try_abort_thread_timeout(بنية k_thread *thread)
{
	أرجع z_try_abort_timeout(&thread->بنياد.timeout);
}

uint32_t z_get_ايندڙ_timeout_expiry(فراغ);

k_ticks_t z_timeout_بچيل(ثابت بنية _timeout *timeout);

#وإلا

/* Stubs when !CONFIG_SYS_CLOCK_EXISTS */
#وصف_ڪريو z_init_thread_timeout(thread_بنياد_2) افعل {} طالما (false)
#وصف_ڪريو z_try_abort_thread_timeout(منزل) 0
#وصف_ڪريو z_is_inactive_timeout(منزل) 1
#وصف_ڪريو z_get_ايندڙ_timeout_expiry() ((uint32_t) K_TICKS_FOREVER)
#وصف_ڪريو z_سيٽ_timeout_expiry(ticks, is_idle) افعل {} طالما (false)

ساكن مضمن k_ticks_t z_جوڙيو_thread_timeout(بنية k_thread *thread, k_timeout_t ticks)
{
	ARG_اڻ_استعمال_ٿيل(thread);
	ARG_اڻ_استعمال_ٿيل(ticks);
	أرجع 0;
}

#جيڪڏهن_پورو /* CONFIG_SYS_CLOCK_EXISTS */

#جيڪڏهن_وصف_ٿيل __cplusplus
}
#جيڪڏهن_پورو

#جيڪڏهن_پورو /* ZEPHYR_KERNEL_INCLUDE_TIMEOUT_Q_H_ */
