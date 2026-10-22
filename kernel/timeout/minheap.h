/*
 * Copyright (c) 2026 Aerlync Labs Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_KERNEL_TIMEOUT_MINHEAP_H_
#define ZEPHYR_KERNEL_TIMEOUT_MINHEAP_H_

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

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ timeout_cmp(ಧ್ರುವ ಸಂರಚನಾ min_heap_handle *a, ಧ್ರುವ ಸಂರಚನಾ min_heap_handle *b)
{
	ಧ್ರುವ ಸಂರಚನಾ _timeout *ta = CONTAINER_OF(a, ಸಂರಚನಾ _timeout, heap_handle);
	ಧ್ರುವ ಸಂರಚನಾ _timeout *tb = CONTAINER_OF(b, ಸಂರಚನಾ _timeout, heap_handle);

	ಯದಿ (ta->abs_ticks < tb->abs_ticks) {
		ಪ್ರತ್ಯಾಗಮ -1;
	}

	ಪ್ರತ್ಯಾಗಮ (ta->abs_ticks > tb->abs_ticks) ? 1 : 0;
}

MIN_HEAP_REF_DEFINE_STATIC(timeout_heap_inst, CONFIG_TIMEOUT_HEAP_MAX_ENTRIES, timeout_cmp);

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಸಂರಚನಾ _timeout *z_timeout_q_ಬೇರು(ಶೂನ್ಯ)
{
	ಧ್ರುವ ಸಂರಚನಾ min_heap_handle *h = min_heap_ref_peek(&timeout_heap_inst);

	ಪ್ರತ್ಯಾಗಮ (h == ರಿಕ್ತ) ? ರಿಕ್ತ : CONTAINER_OF(h, ಸಂರಚನಾ _timeout, heap_handle);
}

/* Insert @to so that it expires @dticks ticks after curr_tick. Returns true
 * if @to is now the earliest pending timeout (heap root).
 */
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ bool z_timeout_q_ಸೇರಿಸು(ಸಂರಚನಾ _timeout *to, k_ticks_t dticks)
{
	to->abs_ticks = (int64_t)curr_tick + dticks;

	ಯದಿ (unlikely(min_heap_ref_ತಳ್ಳು(&timeout_heap_inst, &to->heap_handle) != 0)) {
		/*
		 * Heap overflow is always a configuration error: there is no
		 * graceful way to drop a kernel timeout. Increase
		 * CONFIG_TIMEOUT_HEAP_MAX_ENTRIES.
		 */
		k_panic();
	}

	ಪ್ರತ್ಯಾಗಮ to->heap_handle.idx == 1U;
}

/* Remove an arbitrary timeout from the heap. Returns true if it was the
 * earliest pending timeout. Used only by the abort path, never from announce.
 */
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ bool z_timeout_q_remove(ಸಂರಚನಾ _timeout *to)
{
	bool was_first = (to->heap_handle.idx == 1U);

	(ಶೂನ್ಯ)min_heap_ref_remove(&timeout_heap_inst, &to->heap_handle);

	ಪ್ರತ್ಯಾಗಮ was_first;
}

/* Ticks from curr_tick until @to expires. Caller holds timeout_lock and has
 * verified that @to is active.
 */
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ k_ticks_t z_timeout_q_remainder(ಧ್ರುವ ಸಂರಚನಾ _timeout *to)
{
	ಪ್ರತ್ಯಾಗಮ (k_ticks_t)(to->abs_ticks - (int64_t)curr_tick);
}

/* Ticks from curr_tick to the earliest pending timeout, or K_TICKS_FOREVER if
 * the heap is empty. The announce-range cap is applied centrally by
 * next_timeout() in timeout.c.
 */
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ k_ticks_t z_timeout_q_next_expiry(ಶೂನ್ಯ)
{
	ಸಂರಚನಾ _timeout *t = z_timeout_q_ಬೇರು();

	ಪ್ರತ್ಯಾಗಮ (t == ರಿಕ್ತ) ? K_TICKS_FOREVER
			   : (k_ticks_t)(t->abs_ticks - (int64_t)curr_tick);
}

/* --- announce-loop helpers (see sys_clock_announce_locked() in timeout.c) --- */

/* Ticks from curr_tick until the earliest expiry. A value larger than any
 * announce window when the heap is empty.
 */
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ int32_t z_timeout_q_next_gap(ಶೂನ್ಯ)
{
	ಸಂರಚನಾ _timeout *t = z_timeout_q_ಬೇರು();

	ಯದಿ (t == ರಿಕ್ತ) {
		ಪ್ರತ್ಯಾಗಮ INT32_MAX;
	}

	ಪ್ರತ್ಯಾಗಮ (int32_t)MIN(t->abs_ticks - (int64_t)curr_tick, (int64_t)INT32_MAX);
}

/* Advance backend state by @dt ticks. The heap stores absolute expiry ticks,
 * so the caller's curr_tick += dt is all the advancing required here.
 */
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ z_timeout_q_advance(int32_t dt)
{
	ARG_UNUSED(dt);
}

/* Pop and return one timeout that is due at the current curr_tick, or NULL if
 * none is due right now. Tied timeouts share the same abs_ticks and are
 * drained by successive calls.
 */
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಸಂರಚನಾ _timeout *z_timeout_q_ತೆಗೆ_due(ಶೂನ್ಯ)
{
	ಸಂರಚನಾ _timeout *t = z_timeout_q_ಬೇರು();

	ಯದಿ ((t == ರಿಕ್ತ) || (t->abs_ticks > (int64_t)curr_tick)) {
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}

	ಪ್ರತ್ಯಾಗಮ CONTAINER_OF(min_heap_ref_ತೆಗೆ(&timeout_heap_inst), ಸಂರಚನಾ _timeout, heap_handle);
}

#endif /* ZEPHYR_KERNEL_TIMEOUT_MINHEAP_H_ */
