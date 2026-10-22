/*
 * Copyright (c) 2015 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#నిర్వచితం_కాకపోతే ZEPHYR_KERNEL_INCLUDE_TIMEOUT_Q_H_
#నిర్వచించు ZEPHYR_KERNEL_INCLUDE_TIMEOUT_Q_H_

/**
 * @file
 * @brief timeout queue for threads on kernel objects
 */

#అన్తర్భావ <zephyr/kernel.h>

#అన్తర్భావ <stdbool.h>

#నిర్వచితమైతే __cplusplus
బాహ్య "C" {
#అయితే_ముగింపు

#నిర్వచితమైతే CONFIG_SYS_CLOCK_EXISTS

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
#ఒకవేళ defined(CONFIG_TIMEOUT_BACKEND_MINHEAP)

స్థిర అంతఃవరుస శూన్య z_init_timeout(నిర్మాణం _timeout *గమ్యం)
{
	గమ్యం->కుప్ప_handle.idx = 0U;
	గమ్యం->abs_ticks = 0;
}

స్థిర అంతఃవరుస bool z_is_inactive_timeout(స్థిరరాశి నిర్మాణం _timeout *గమ్యం)
{
	తిరిగి గమ్యం->కుప్ప_handle.idx == 0U;
}

#లేదావేళ defined(CONFIG_TIMEOUT_BACKEND_WHEEL)

స్థిర అంతఃవరుస శూన్య z_init_timeout(నిర్మాణం _timeout *గమ్యం)
{
	sys_dnode_init(&గమ్యం->కణుపు);
	గమ్యం->flags = 0U;
	గమ్యం->dticks = 0;
}

స్థిర అంతఃవరుస bool z_is_inactive_timeout(స్థిరరాశి నిర్మాణం _timeout *గమ్యం)
{
	తిరిగి !sys_dnode_is_linked(&గమ్యం->కణుపు);
}

#లేదావేళ defined(CONFIG_TIMEOUT_BACKEND_SKIPLIST)

స్థిర అంతఃవరుస శూన్య z_init_timeout(నిర్మాణం _timeout *గమ్యం)
{
	uint8_t i;

	గమ్యం->ఎత్తు = 0;
	గమ్యం->abs_ticks = 0;

	కొరకు (i = 0; i < CONFIG_TIMEOUT_SKIPLIST_MAX_LEVEL; i++) {
		గమ్యం->forward[i] = ఖాళీ;
	}
}

స్థిర అంతఃవరుస bool z_is_inactive_timeout(స్థిరరాశి నిర్మాణం _timeout *గమ్యం)
{
	తిరిగి గమ్యం->ఎత్తు == 0;
}

#లేకపోతే /* CONFIG_TIMEOUT_BACKEND_DLIST or _BUCKET */

స్థిర అంతఃవరుస శూన్య z_init_timeout(నిర్మాణం _timeout *గమ్యం)
{
	sys_dnode_init(&గమ్యం->కణుపు);
	గమ్యం->dticks = 0;
}

స్థిర అంతఃవరుస bool z_is_inactive_timeout(స్థిరరాశి నిర్మాణం _timeout *గమ్యం)
{
	తిరిగి !sys_dnode_is_linked(&గమ్యం->కణుపు);
}

#అయితే_ముగింపు

/* Adds the timeout to the queue.
 *
 * @return Absolute tick value when timeout will expire.
 */
k_ticks_t z_కూడు_timeout(నిర్మాణం _timeout *గమ్యం, _timeout_func_t fn, k_timeout_t timeout);

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
పూర్ణాంకం z_try_abort_timeout(నిర్మాణం _timeout *గమ్యం);

/* True if @to is the currently in-flight timeout and a same-CPU aborter
 * has marked the slot as superseded. Handlers with non-idempotent side
 * effects (e.g. k_timer's expiry_fn) should call this at entry, after
 * taking their own lock, and bail if it returns true.
 */
bool z_timeout_inflight_superseded(స్థిరరాశి నిర్మాణం _timeout *గమ్యం);

స్థిర అంతఃవరుస శూన్య z_init_thread_timeout(నిర్మాణం thread_పునాది *thread_పునాది_2)
{
	z_init_timeout(&thread_పునాది_2->timeout);
}

బాహ్య శూన్య z_thread_timeout(నిర్మాణం _timeout *timeout);

స్థిర అంతఃవరుస k_ticks_t z_కూడు_thread_timeout(నిర్మాణం k_thread *thread, k_timeout_t ticks)
{
	తిరిగి z_కూడు_timeout(&thread->పునాది.timeout, z_thread_timeout, ticks);
}

స్థిర అంతఃవరుస పూర్ణాంకం z_try_abort_thread_timeout(నిర్మాణం k_thread *thread)
{
	తిరిగి z_try_abort_timeout(&thread->పునాది.timeout);
}

uint32_t z_get_తదుపరి_timeout_expiry(శూన్య);

k_ticks_t z_timeout_మిగిలిన(స్థిరరాశి నిర్మాణం _timeout *timeout);

#లేకపోతే

/* Stubs when !CONFIG_SYS_CLOCK_EXISTS */
#నిర్వచించు z_init_thread_timeout(thread_పునాది_2) చేయి {} అయితే (false)
#నిర్వచించు z_try_abort_thread_timeout(గమ్యం) 0
#నిర్వచించు z_is_inactive_timeout(గమ్యం) 1
#నిర్వచించు z_get_తదుపరి_timeout_expiry() ((uint32_t) K_TICKS_FOREVER)
#నిర్వచించు z_సమితి_timeout_expiry(ticks, is_idle) చేయి {} అయితే (false)

స్థిర అంతఃవరుస k_ticks_t z_కూడు_thread_timeout(నిర్మాణం k_thread *thread, k_timeout_t ticks)
{
	ARG_వాడని(thread);
	ARG_వాడని(ticks);
	తిరిగి 0;
}

#అయితే_ముగింపు /* CONFIG_SYS_CLOCK_EXISTS */

#నిర్వచితమైతే __cplusplus
}
#అయితే_ముగింపు

#అయితే_ముగింపు /* ZEPHYR_KERNEL_INCLUDE_TIMEOUT_Q_H_ */
