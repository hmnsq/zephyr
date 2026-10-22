/*
 * Copyright (c) 2015 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#اگر_غیر_تعریف_شدہ ZEPHYR_KERNEL_INCLUDE_TIMEOUT_Q_H_
#تعریف_کرو ZEPHYR_KERNEL_INCLUDE_TIMEOUT_Q_H_

/**
 * @file
 * @brief timeout queue for threads on kernel objects
 */

#شامل <zephyr/kernel.h>

#شامل <stdbool.h>

#اگر_تعریف_شدہ __cplusplus
خارجی "C" {
#اگر_ختم

#اگر_تعریف_شدہ CONFIG_SYS_CLOCK_EXISTS

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

ساکن اندرونی خالی z_init_timeout(ساخت _timeout *مقصد)
{
	مقصد->ڈھیر_handle.idx = 0U;
	مقصد->abs_ticks = 0;
}

ساکن اندرونی bool z_is_inactive_timeout(ثابت ساخت _timeout *مقصد)
{
	واپس مقصد->ڈھیر_handle.idx == 0U;
}

#وگرنہ_اگر defined(CONFIG_TIMEOUT_BACKEND_WHEEL)

ساکن اندرونی خالی z_init_timeout(ساخت _timeout *مقصد)
{
	sys_dnode_init(&مقصد->گرہ);
	مقصد->flags = 0U;
	مقصد->dticks = 0;
}

ساکن اندرونی bool z_is_inactive_timeout(ثابت ساخت _timeout *مقصد)
{
	واپس !sys_dnode_is_linked(&مقصد->گرہ);
}

#وگرنہ_اگر defined(CONFIG_TIMEOUT_BACKEND_SKIPLIST)

ساکن اندرونی خالی z_init_timeout(ساخت _timeout *مقصد)
{
	uint8_t i;

	مقصد->اونچائی = 0;
	مقصد->abs_ticks = 0;

	برائے (i = 0; i < CONFIG_TIMEOUT_SKIPLIST_MAX_LEVEL; i++) {
		مقصد->forward[i] = تہی;
	}
}

ساکن اندرونی bool z_is_inactive_timeout(ثابت ساخت _timeout *مقصد)
{
	واپس مقصد->اونچائی == 0;
}

#ورنہ /* CONFIG_TIMEOUT_BACKEND_DLIST or _BUCKET */

ساکن اندرونی خالی z_init_timeout(ساخت _timeout *مقصد)
{
	sys_dnode_init(&مقصد->گرہ);
	مقصد->dticks = 0;
}

ساکن اندرونی bool z_is_inactive_timeout(ثابت ساخت _timeout *مقصد)
{
	واپس !sys_dnode_is_linked(&مقصد->گرہ);
}

#اگر_ختم

/* Adds the timeout to the queue.
 *
 * @return Absolute tick value when timeout will expire.
 */
k_ticks_t z_جوڑو_timeout(ساخت _timeout *مقصد, _timeout_func_t fn, k_timeout_t timeout);

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
صحیح z_try_abort_timeout(ساخت _timeout *مقصد);

/* True if @to is the currently in-flight timeout and a same-CPU aborter
 * has marked the slot as superseded. Handlers with non-idempotent side
 * effects (e.g. k_timer's expiry_fn) should call this at entry, after
 * taking their own lock, and bail if it returns true.
 */
bool z_timeout_inflight_superseded(ثابت ساخت _timeout *مقصد);

ساکن اندرونی خالی z_init_thread_timeout(ساخت thread_بنیاد *thread_بنیاد_2)
{
	z_init_timeout(&thread_بنیاد_2->timeout);
}

خارجی خالی z_thread_timeout(ساخت _timeout *timeout);

ساکن اندرونی k_ticks_t z_جوڑو_thread_timeout(ساخت k_thread *thread, k_timeout_t ticks)
{
	واپس z_جوڑو_timeout(&thread->بنیاد.timeout, z_thread_timeout, ticks);
}

ساکن اندرونی صحیح z_try_abort_thread_timeout(ساخت k_thread *thread)
{
	واپس z_try_abort_timeout(&thread->بنیاد.timeout);
}

uint32_t z_get_اگلا_timeout_expiry(خالی);

k_ticks_t z_timeout_بقایا(ثابت ساخت _timeout *timeout);

#ورنہ

/* Stubs when !CONFIG_SYS_CLOCK_EXISTS */
#تعریف_کرو z_init_thread_timeout(thread_بنیاد_2) کرو {} جب (false)
#تعریف_کرو z_try_abort_thread_timeout(مقصد) 0
#تعریف_کرو z_is_inactive_timeout(مقصد) 1
#تعریف_کرو z_get_اگلا_timeout_expiry() ((uint32_t) K_TICKS_FOREVER)
#تعریف_کرو z_سیٹ_timeout_expiry(ticks, is_idle) کرو {} جب (false)

ساکن اندرونی k_ticks_t z_جوڑو_thread_timeout(ساخت k_thread *thread, k_timeout_t ticks)
{
	ARG_غیر_مستعمل(thread);
	ARG_غیر_مستعمل(ticks);
	واپس 0;
}

#اگر_ختم /* CONFIG_SYS_CLOCK_EXISTS */

#اگر_تعریف_شدہ __cplusplus
}
#اگر_ختم

#اگر_ختم /* ZEPHYR_KERNEL_INCLUDE_TIMEOUT_Q_H_ */
