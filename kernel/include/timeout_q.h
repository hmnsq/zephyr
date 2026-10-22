/*
 * Copyright (c) 2015 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ ZEPHYR_KERNEL_INCLUDE_TIMEOUT_Q_H_
#ವ್ಯಾಖ್ಯಾನಿಸು ZEPHYR_KERNEL_INCLUDE_TIMEOUT_Q_H_

/**
 * @file
 * @brief timeout queue for threads on kernel objects
 */

#ಅನ್ತರ್ಭಾವ <zephyr/kernel.h>

#ಅನ್ತರ್ಭಾವ <stdbool.h>

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __cplusplus
ಬಾಹ್ಯ "C" {
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SYS_CLOCK_EXISTS

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
#ಆದರೆ defined(CONFIG_TIMEOUT_BACKEND_MINHEAP)

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ z_init_timeout(ರಚನೆ _timeout *ಗಮ್ಯ)
{
	ಗಮ್ಯ->ರಾಶಿ_handle.idx = 0U;
	ಗಮ್ಯ->abs_ticks = 0;
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ bool z_is_inactive_timeout(ಸ್ಥಿರಾಂಕ ರಚನೆ _timeout *ಗಮ್ಯ)
{
	ಹಿಂತಿರುಗಿ ಗಮ್ಯ->ರಾಶಿ_handle.idx == 0U;
}

#ಅಥವಾಆದರೆ defined(CONFIG_TIMEOUT_BACKEND_WHEEL)

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ z_init_timeout(ರಚನೆ _timeout *ಗಮ್ಯ)
{
	sys_dnode_init(&ಗಮ್ಯ->ಗಂಟು);
	ಗಮ್ಯ->flags = 0U;
	ಗಮ್ಯ->dticks = 0;
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ bool z_is_inactive_timeout(ಸ್ಥಿರಾಂಕ ರಚನೆ _timeout *ಗಮ್ಯ)
{
	ಹಿಂತಿರುಗಿ !sys_dnode_is_linked(&ಗಮ್ಯ->ಗಂಟು);
}

#ಅಥವಾಆದರೆ defined(CONFIG_TIMEOUT_BACKEND_SKIPLIST)

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ z_init_timeout(ರಚನೆ _timeout *ಗಮ್ಯ)
{
	uint8_t i;

	ಗಮ್ಯ->ಎತ್ತರ = 0;
	ಗಮ್ಯ->abs_ticks = 0;

	ಗಾಗಿ (i = 0; i < CONFIG_TIMEOUT_SKIPLIST_MAX_LEVEL; i++) {
		ಗಮ್ಯ->forward[i] = ಖಾಲಿ;
	}
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ bool z_is_inactive_timeout(ಸ್ಥಿರಾಂಕ ರಚನೆ _timeout *ಗಮ್ಯ)
{
	ಹಿಂತಿರುಗಿ ಗಮ್ಯ->ಎತ್ತರ == 0;
}

#ಇಲ್ಲದಿದ್ದರೆ /* CONFIG_TIMEOUT_BACKEND_DLIST or _BUCKET */

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ z_init_timeout(ರಚನೆ _timeout *ಗಮ್ಯ)
{
	sys_dnode_init(&ಗಮ್ಯ->ಗಂಟು);
	ಗಮ್ಯ->dticks = 0;
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ bool z_is_inactive_timeout(ಸ್ಥಿರಾಂಕ ರಚನೆ _timeout *ಗಮ್ಯ)
{
	ಹಿಂತಿರುಗಿ !sys_dnode_is_linked(&ಗಮ್ಯ->ಗಂಟು);
}

#ಆದರೆ_ಅಂತ್ಯ

/* Adds the timeout to the queue.
 *
 * @return Absolute tick value when timeout will expire.
 */
k_ticks_t z_ಕೂಡಿಸು_timeout(ರಚನೆ _timeout *ಗಮ್ಯ, _timeout_func_t fn, k_timeout_t timeout);

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
ಪೂರ್ಣಾಂಕ z_try_abort_timeout(ರಚನೆ _timeout *ಗಮ್ಯ);

/* True if @to is the currently in-flight timeout and a same-CPU aborter
 * has marked the slot as superseded. Handlers with non-idempotent side
 * effects (e.g. k_timer's expiry_fn) should call this at entry, after
 * taking their own lock, and bail if it returns true.
 */
bool z_timeout_inflight_superseded(ಸ್ಥಿರಾಂಕ ರಚನೆ _timeout *ಗಮ್ಯ);

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ z_init_thread_timeout(ರಚನೆ thread_ತಳ *thread_ತಳ_2)
{
	z_init_timeout(&thread_ತಳ_2->timeout);
}

ಬಾಹ್ಯ ಶೂನ್ಯ z_thread_timeout(ರಚನೆ _timeout *timeout);

ಸ್ಥಿರ ಸಾಲೊಳಗೆ k_ticks_t z_ಕೂಡಿಸು_thread_timeout(ರಚನೆ k_thread *thread, k_timeout_t ticks)
{
	ಹಿಂತಿರುಗಿ z_ಕೂಡಿಸು_timeout(&thread->ತಳ.timeout, z_thread_timeout, ticks);
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಪೂರ್ಣಾಂಕ z_try_abort_thread_timeout(ರಚನೆ k_thread *thread)
{
	ಹಿಂತಿರುಗಿ z_try_abort_timeout(&thread->ತಳ.timeout);
}

uint32_t z_get_ಮುಂದಿನ_timeout_expiry(ಶೂನ್ಯ);

k_ticks_t z_timeout_ಉಳಿದ(ಸ್ಥಿರಾಂಕ ರಚನೆ _timeout *timeout);

#ಇಲ್ಲದಿದ್ದರೆ

/* Stubs when !CONFIG_SYS_CLOCK_EXISTS */
#ವ್ಯಾಖ್ಯಾನಿಸು z_init_thread_timeout(thread_ತಳ_2) ಮಾಡು {} ಆಗ (false)
#ವ್ಯಾಖ್ಯಾನಿಸು z_try_abort_thread_timeout(ಗಮ್ಯ) 0
#ವ್ಯಾಖ್ಯಾನಿಸು z_is_inactive_timeout(ಗಮ್ಯ) 1
#ವ್ಯಾಖ್ಯಾನಿಸು z_get_ಮುಂದಿನ_timeout_expiry() ((uint32_t) K_TICKS_FOREVER)
#ವ್ಯಾಖ್ಯಾನಿಸು z_ಗಣ_timeout_expiry(ticks, is_idle) ಮಾಡು {} ಆಗ (false)

ಸ್ಥಿರ ಸಾಲೊಳಗೆ k_ticks_t z_ಕೂಡಿಸು_thread_timeout(ರಚನೆ k_thread *thread, k_timeout_t ticks)
{
	ARG_ಬಳಸದ(thread);
	ARG_ಬಳಸದ(ticks);
	ಹಿಂತಿರುಗಿ 0;
}

#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_SYS_CLOCK_EXISTS */

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __cplusplus
}
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ_ಅಂತ್ಯ /* ZEPHYR_KERNEL_INCLUDE_TIMEOUT_Q_H_ */
