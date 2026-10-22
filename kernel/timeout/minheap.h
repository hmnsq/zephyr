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

#অন্তর্ভাব <zephyr/sys/min_heap_ref.h>

স্থির পূর্ণসংখ্যা timeout_cmp(ধ্রুবক কাঠামো min_heap_handle *a, ধ্রুবক কাঠামো min_heap_handle *b)
{
	ধ্রুবক কাঠামো _timeout *ta = CONTAINER_OF(a, কাঠামো _timeout, heap_handle);
	ধ্রুবক কাঠামো _timeout *tb = CONTAINER_OF(b, কাঠামো _timeout, heap_handle);

	যদি (ta->abs_ticks < tb->abs_ticks) {
		ফেরত -1;
	}

	ফেরত (ta->abs_ticks > tb->abs_ticks) ? 1 : 0;
}

MIN_HEAP_REF_DEFINE_STATIC(timeout_heap_inst, CONFIG_TIMEOUT_HEAP_MAX_ENTRIES, timeout_cmp);

স্থির অন্তঃরেখ কাঠামো _timeout *z_timeout_q_মূল(শূন্য)
{
	ধ্রুবক কাঠামো min_heap_handle *h = min_heap_ref_peek(&timeout_heap_inst);

	ফেরত (h == ফাঁকা) ? ফাঁকা : CONTAINER_OF(h, কাঠামো _timeout, heap_handle);
}

/* Insert @to so that it expires @dticks ticks after curr_tick. Returns true
 * if @to is now the earliest pending timeout (heap root).
 */
স্থির অন্তঃরেখ bool z_timeout_q_ঢোকাও(কাঠামো _timeout *to, k_ticks_t dticks)
{
	to->abs_ticks = (int64_t)curr_tick + dticks;

	যদি (unlikely(min_heap_ref_ঠেলো(&timeout_heap_inst, &to->heap_handle) != 0)) {
		/*
		 * Heap overflow is always a configuration error: there is no
		 * graceful way to drop a kernel timeout. Increase
		 * CONFIG_TIMEOUT_HEAP_MAX_ENTRIES.
		 */
		k_panic();
	}

	ফেরত to->heap_handle.idx == 1U;
}

/* Remove an arbitrary timeout from the heap. Returns true if it was the
 * earliest pending timeout. Used only by the abort path, never from announce.
 */
স্থির অন্তঃরেখ bool z_timeout_q_remove(কাঠামো _timeout *to)
{
	bool was_first = (to->heap_handle.idx == 1U);

	(শূন্য)min_heap_ref_remove(&timeout_heap_inst, &to->heap_handle);

	ফেরত was_first;
}

/* Ticks from curr_tick until @to expires. Caller holds timeout_lock and has
 * verified that @to is active.
 */
স্থির অন্তঃরেখ k_ticks_t z_timeout_q_remainder(ধ্রুবক কাঠামো _timeout *to)
{
	ফেরত (k_ticks_t)(to->abs_ticks - (int64_t)curr_tick);
}

/* Ticks from curr_tick to the earliest pending timeout, or K_TICKS_FOREVER if
 * the heap is empty. The announce-range cap is applied centrally by
 * next_timeout() in timeout.c.
 */
স্থির অন্তঃরেখ k_ticks_t z_timeout_q_পরবর্তী_expiry(শূন্য)
{
	কাঠামো _timeout *t = z_timeout_q_মূল();

	ফেরত (t == ফাঁকা) ? K_TICKS_FOREVER
			   : (k_ticks_t)(t->abs_ticks - (int64_t)curr_tick);
}

/* --- announce-loop helpers (see sys_clock_announce_locked() in timeout.c) --- */

/* Ticks from curr_tick until the earliest expiry. A value larger than any
 * announce window when the heap is empty.
 */
স্থির অন্তঃরেখ int32_t z_timeout_q_পরবর্তী_gap(শূন্য)
{
	কাঠামো _timeout *t = z_timeout_q_মূল();

	যদি (t == ফাঁকা) {
		ফেরত INT32_MAX;
	}

	ফেরত (int32_t)MIN(t->abs_ticks - (int64_t)curr_tick, (int64_t)INT32_MAX);
}

/* Advance backend state by @dt ticks. The heap stores absolute expiry ticks,
 * so the caller's curr_tick += dt is all the advancing required here.
 */
স্থির অন্তঃরেখ শূন্য z_timeout_q_advance(int32_t dt)
{
	ARG_UNUSED(dt);
}

/* Pop and return one timeout that is due at the current curr_tick, or NULL if
 * none is due right now. Tied timeouts share the same abs_ticks and are
 * drained by successive calls.
 */
স্থির অন্তঃরেখ কাঠামো _timeout *z_timeout_q_বের_করো_due(শূন্য)
{
	কাঠামো _timeout *t = z_timeout_q_মূল();

	যদি ((t == ফাঁকা) || (t->abs_ticks > (int64_t)curr_tick)) {
		ফেরত ফাঁকা;
	}

	ফেরত CONTAINER_OF(min_heap_ref_বের_করো(&timeout_heap_inst), কাঠামো _timeout, heap_handle);
}

#endif /* ZEPHYR_KERNEL_TIMEOUT_MINHEAP_H_ */
