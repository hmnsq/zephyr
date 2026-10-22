/*
 * Copyright (c) 2015 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ ZEPHYR_KERNEL_INCLUDE_TIMEOUT_Q_H_
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ZEPHYR_KERNEL_INCLUDE_TIMEOUT_Q_H_

/**
 * @file
 * @brief timeout queue for threads on kernel objects
 */

#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/kernel.h>

#ਅਨ੍ਤਰ੍ਭਾਵ <stdbool.h>

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __cplusplus
ਬਾਹਰੀ "C" {
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SYS_CLOCK_EXISTS

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
#ਜੇ defined(CONFIG_TIMEOUT_BACKEND_MINHEAP)

ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ z_init_timeout(ਬਣਤਰ _timeout *ਮੰਜ਼ਿਲ)
{
	ਮੰਜ਼ਿਲ->ਢੇਰ_handle.idx = 0U;
	ਮੰਜ਼ਿਲ->abs_ticks = 0;
}

ਸਥਿਰ ਸਤਰਅੰਦਰ bool z_is_inactive_timeout(ਅਚਲ ਬਣਤਰ _timeout *ਮੰਜ਼ਿਲ)
{
	ਵਾਪਸ ਮੰਜ਼ਿਲ->ਢੇਰ_handle.idx == 0U;
}

#ਜਾਂਜੇ defined(CONFIG_TIMEOUT_BACKEND_WHEEL)

ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ z_init_timeout(ਬਣਤਰ _timeout *ਮੰਜ਼ਿਲ)
{
	sys_dnode_init(&ਮੰਜ਼ਿਲ->ਗੰਢ);
	ਮੰਜ਼ਿਲ->flags = 0U;
	ਮੰਜ਼ਿਲ->dticks = 0;
}

ਸਥਿਰ ਸਤਰਅੰਦਰ bool z_is_inactive_timeout(ਅਚਲ ਬਣਤਰ _timeout *ਮੰਜ਼ਿਲ)
{
	ਵਾਪਸ !sys_dnode_is_linked(&ਮੰਜ਼ਿਲ->ਗੰਢ);
}

#ਜਾਂਜੇ defined(CONFIG_TIMEOUT_BACKEND_SKIPLIST)

ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ z_init_timeout(ਬਣਤਰ _timeout *ਮੰਜ਼ਿਲ)
{
	uint8_t i;

	ਮੰਜ਼ਿਲ->ਉਚਾਈ = 0;
	ਮੰਜ਼ਿਲ->abs_ticks = 0;

	ਲਈ (i = 0; i < CONFIG_TIMEOUT_SKIPLIST_MAX_LEVEL; i++) {
		ਮੰਜ਼ਿਲ->forward[i] = ਨਿਰਾ;
	}
}

ਸਥਿਰ ਸਤਰਅੰਦਰ bool z_is_inactive_timeout(ਅਚਲ ਬਣਤਰ _timeout *ਮੰਜ਼ਿਲ)
{
	ਵਾਪਸ ਮੰਜ਼ਿਲ->ਉਚਾਈ == 0;
}

#ਨਹੀਂਤਾਂ /* CONFIG_TIMEOUT_BACKEND_DLIST or _BUCKET */

ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ z_init_timeout(ਬਣਤਰ _timeout *ਮੰਜ਼ਿਲ)
{
	sys_dnode_init(&ਮੰਜ਼ਿਲ->ਗੰਢ);
	ਮੰਜ਼ਿਲ->dticks = 0;
}

ਸਥਿਰ ਸਤਰਅੰਦਰ bool z_is_inactive_timeout(ਅਚਲ ਬਣਤਰ _timeout *ਮੰਜ਼ਿਲ)
{
	ਵਾਪਸ !sys_dnode_is_linked(&ਮੰਜ਼ਿਲ->ਗੰਢ);
}

#ਜੇ_ਅੰਤ

/* Adds the timeout to the queue.
 *
 * @return Absolute tick value when timeout will expire.
 */
k_ticks_t z_ਜੋੜੋ_timeout(ਬਣਤਰ _timeout *ਮੰਜ਼ਿਲ, _timeout_func_t fn, k_timeout_t timeout);

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
ਪੂਰਨ z_try_abort_timeout(ਬਣਤਰ _timeout *ਮੰਜ਼ਿਲ);

/* True if @to is the currently in-flight timeout and a same-CPU aborter
 * has marked the slot as superseded. Handlers with non-idempotent side
 * effects (e.g. k_timer's expiry_fn) should call this at entry, after
 * taking their own lock, and bail if it returns true.
 */
bool z_timeout_inflight_superseded(ਅਚਲ ਬਣਤਰ _timeout *ਮੰਜ਼ਿਲ);

ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ z_init_thread_timeout(ਬਣਤਰ thread_ਅਧਾਰ *thread_ਅਧਾਰ_2)
{
	z_init_timeout(&thread_ਅਧਾਰ_2->timeout);
}

ਬਾਹਰੀ ਖਾਲੀ z_thread_timeout(ਬਣਤਰ _timeout *timeout);

ਸਥਿਰ ਸਤਰਅੰਦਰ k_ticks_t z_ਜੋੜੋ_thread_timeout(ਬਣਤਰ k_thread *thread, k_timeout_t ticks)
{
	ਵਾਪਸ z_ਜੋੜੋ_timeout(&thread->ਅਧਾਰ.timeout, z_thread_timeout, ticks);
}

ਸਥਿਰ ਸਤਰਅੰਦਰ ਪੂਰਨ z_try_abort_thread_timeout(ਬਣਤਰ k_thread *thread)
{
	ਵਾਪਸ z_try_abort_timeout(&thread->ਅਧਾਰ.timeout);
}

uint32_t z_get_ਅਗਲਾ_timeout_expiry(ਖਾਲੀ);

k_ticks_t z_timeout_ਬਚਿਆ(ਅਚਲ ਬਣਤਰ _timeout *timeout);

#ਨਹੀਂਤਾਂ

/* Stubs when !CONFIG_SYS_CLOCK_EXISTS */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ z_init_thread_timeout(thread_ਅਧਾਰ_2) ਕਰੋ {} ਜਦੋਂ (false)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ z_try_abort_thread_timeout(ਮੰਜ਼ਿਲ) 0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ z_is_inactive_timeout(ਮੰਜ਼ਿਲ) 1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ z_get_ਅਗਲਾ_timeout_expiry() ((uint32_t) K_TICKS_FOREVER)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ z_ਸੈੱਟ_timeout_expiry(ticks, is_idle) ਕਰੋ {} ਜਦੋਂ (false)

ਸਥਿਰ ਸਤਰਅੰਦਰ k_ticks_t z_ਜੋੜੋ_thread_timeout(ਬਣਤਰ k_thread *thread, k_timeout_t ticks)
{
	ARG_ਅਣਵਰਤਿਆ(thread);
	ARG_ਅਣਵਰਤਿਆ(ticks);
	ਵਾਪਸ 0;
}

#ਜੇ_ਅੰਤ /* CONFIG_SYS_CLOCK_EXISTS */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __cplusplus
}
#ਜੇ_ਅੰਤ

#ਜੇ_ਅੰਤ /* ZEPHYR_KERNEL_INCLUDE_TIMEOUT_Q_H_ */
