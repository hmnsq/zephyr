/*
 * Copyright (c) 2026 Aerlync Labs Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ ZEPHYR_KERNEL_TIMEOUT_MINHEAP_H_
#ವ್ಯಾಖ್ಯಾನಿಸು ZEPHYR_KERNEL_TIMEOUT_MINHEAP_H_

/**
 * @file
 * @brief Binary min-heap timeout backend (implementation).
 *
 * Pending timeouts are kept in a binary min-heap keyed on absolute expiry
 * tick (struct _timeout's abs_ticks). Insertion and arbitrary removal are
 * O(log n); the earliest timeout is always at the root. A timeout that is
 * not queued has heap_handle.idx == 0.
 *
 * Included only by kernel/timeout.c, after its shared state (curr_tick), so
 * the heap instance and the z_timeout_q_*() operations below are private to
 * that translation unit. The per-node helpers live in
 * kernel/include/timeout_q.h.
 */

#ಅನ್ತರ್ಭಾವ <zephyr/sys/min_heap_ref.h>

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ timeout_cmp(ಸ್ಥಿರಾಂಕ ರಚನೆ min_ರಾಶಿ_handle *a, ಸ್ಥಿರಾಂಕ ರಚನೆ min_ರಾಶಿ_handle *b)
{
	ಸ್ಥಿರಾಂಕ ರಚನೆ _timeout *ta = CONTAINER_ದ(a, ರಚನೆ _timeout, ರಾಶಿ_handle);
	ಸ್ಥಿರಾಂಕ ರಚನೆ _timeout *tb = CONTAINER_ದ(b, ರಚನೆ _timeout, ರಾಶಿ_handle);

	ಆದರೆ (ta->abs_ticks < tb->abs_ticks) {
		ಹಿಂತಿರುಗಿ -1;
	}

	ಹಿಂತಿರುಗಿ (ta->abs_ticks > tb->abs_ticks) ? 1 : 0;
}

MIN_ರಾಶಿ_REF_DEFINE_STATIC(timeout_ರಾಶಿ_inst, CONFIG_TIMEOUT_ರಾಶಿ_MAX_ENTRIES, timeout_cmp);

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ರಚನೆ _timeout *z_timeout_q_ಬೇರು(ಶೂನ್ಯ)
{
	ಸ್ಥಿರಾಂಕ ರಚನೆ min_ರಾಶಿ_handle *h = min_ರಾಶಿ_ref_peek(&timeout_ರಾಶಿ_inst);

	ಹಿಂತಿರುಗಿ (h == ಖಾಲಿ) ? ಖಾಲಿ : CONTAINER_ದ(h, ರಚನೆ _timeout, ರಾಶಿ_handle);
}

/* Insert @to so that it expires @dticks ticks after curr_tick. Returns true
 * if @to is now the earliest pending timeout (heap root).
 */
ಸ್ಥಿರ ಸಾಲೊಳಗೆ bool z_timeout_q_ಸೇರಿಸು(ರಚನೆ _timeout *ಗಮ್ಯ, k_ticks_t dticks)
{
	ಗಮ್ಯ->abs_ticks = (int64_t)curr_tick + dticks;

	ಆದರೆ (unlikely(min_ರಾಶಿ_ref_ತಳ್ಳು(&timeout_ರಾಶಿ_inst, &ಗಮ್ಯ->ರಾಶಿ_handle) != 0)) {
		/*
		 * Heap overflow is always a configuration error: there is no
		 * graceful way to drop a kernel timeout. Increase
		 * CONFIG_TIMEOUT_HEAP_MAX_ENTRIES.
		 */
		k_panic();
	}

	ಹಿಂತಿರುಗಿ ಗಮ್ಯ->ರಾಶಿ_handle.idx == 1U;
}

/* Remove an arbitrary timeout from the heap. Returns true if it was the
 * earliest pending timeout. Used only by the abort path, never from announce.
 */
ಸ್ಥಿರ ಸಾಲೊಳಗೆ bool z_timeout_q_remove(ರಚನೆ _timeout *ಗಮ್ಯ)
{
	bool was_ಮೊದಲ = (ಗಮ್ಯ->ರಾಶಿ_handle.idx == 1U);

	(ಶೂನ್ಯ)min_ರಾಶಿ_ref_remove(&timeout_ರಾಶಿ_inst, &ಗಮ್ಯ->ರಾಶಿ_handle);

	ಹಿಂತಿರುಗಿ was_ಮೊದಲ;
}

/* Ticks from curr_tick until @to expires. Caller holds timeout_lock and has
 * verified that @to is active.
 */
ಸ್ಥಿರ ಸಾಲೊಳಗೆ k_ticks_t z_timeout_q_remainder(ಸ್ಥಿರಾಂಕ ರಚನೆ _timeout *ಗಮ್ಯ)
{
	ಹಿಂತಿರುಗಿ (k_ticks_t)(ಗಮ್ಯ->abs_ticks - (int64_t)curr_tick);
}

/* Ticks from curr_tick to the earliest pending timeout, or K_TICKS_FOREVER if
 * the heap is empty. The announce-range cap is applied centrally by
 * next_timeout() in timeout.c.
 */
ಸ್ಥಿರ ಸಾಲೊಳಗೆ k_ticks_t z_timeout_q_ಮುಂದಿನ_expiry(ಶೂನ್ಯ)
{
	ರಚನೆ _timeout *t = z_timeout_q_ಬೇರು();

	ಹಿಂತಿರುಗಿ (t == ಖಾಲಿ) ? K_TICKS_FOREVER
			   : (k_ticks_t)(t->abs_ticks - (int64_t)curr_tick);
}

/* --- announce-loop helpers (see sys_clock_announce_locked() in timeout.c) --- */

/* Ticks from curr_tick until the earliest expiry. A value larger than any
 * announce window when the heap is empty.
 */
ಸ್ಥಿರ ಸಾಲೊಳಗೆ int32_t z_timeout_q_ಮುಂದಿನ_gap(ಶೂನ್ಯ)
{
	ರಚನೆ _timeout *t = z_timeout_q_ಬೇರು();

	ಆದರೆ (t == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ INT32_MAX;
	}

	ಹಿಂತಿರುಗಿ (int32_t)MIN(t->abs_ticks - (int64_t)curr_tick, (int64_t)INT32_MAX);
}

/* Advance backend state by @dt ticks. The heap stores absolute expiry ticks,
 * so the caller's curr_tick += dt is all the advancing required here.
 */
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ z_timeout_q_advance(int32_t dt)
{
	ARG_ಬಳಸದ(dt);
}

/* Pop and return one timeout that is due at the current curr_tick, or NULL if
 * none is due right now. Tied timeouts share the same abs_ticks and are
 * drained by successive calls.
 */
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ರಚನೆ _timeout *z_timeout_q_ತೆಗೆ_due(ಶೂನ್ಯ)
{
	ರಚನೆ _timeout *t = z_timeout_q_ಬೇರು();

	ಆದರೆ ((t == ಖಾಲಿ) || (t->abs_ticks > (int64_t)curr_tick)) {
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}

	ಹಿಂತಿರುಗಿ CONTAINER_ದ(min_ರಾಶಿ_ref_ತೆಗೆ(&timeout_ರಾಶಿ_inst), ರಚನೆ _timeout, ರಾಶಿ_handle);
}

#ಆದರೆ_ಅಂತ್ಯ /* ZEPHYR_KERNEL_TIMEOUT_MINHEAP_H_ */
