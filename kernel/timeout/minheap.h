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

#شامل <zephyr/sys/min_heap_ref.h>

ایستا صحیح timeout_cmp(ثابت ساختار min_heap_handle *a, ثابت ساختار min_heap_handle *b)
{
	ثابت ساختار _timeout *ta = CONTAINER_OF(a, ساختار _timeout, heap_handle);
	ثابت ساختار _timeout *tb = CONTAINER_OF(b, ساختار _timeout, heap_handle);

	اگر (ta->abs_ticks < tb->abs_ticks) {
		بازگشت -1;
	}

	بازگشت (ta->abs_ticks > tb->abs_ticks) ? 1 : 0;
}

MIN_HEAP_REF_DEFINE_STATIC(timeout_heap_inst, CONFIG_TIMEOUT_HEAP_MAX_ENTRIES, timeout_cmp);

ایستا درونخطی ساختار _timeout *z_timeout_q_ریشه(تهی)
{
	ثابت ساختار min_heap_handle *h = min_heap_ref_peek(&timeout_heap_inst);

	بازگشت (h == پوچ) ? پوچ : CONTAINER_OF(h, ساختار _timeout, heap_handle);
}

/* Insert @to so that it expires @dticks ticks after curr_tick. Returns true
 * if @to is now the earliest pending timeout (heap root).
 */
ایستا درونخطی bool z_timeout_q_درج(ساختار _timeout *to, k_ticks_t dticks)
{
	to->abs_ticks = (int64_t)curr_tick + dticks;

	اگر (unlikely(min_heap_ref_افزودن(&timeout_heap_inst, &to->heap_handle) != 0)) {
		/*
		 * Heap overflow is always a configuration error: there is no
		 * graceful way to drop a kernel timeout. Increase
		 * CONFIG_TIMEOUT_HEAP_MAX_ENTRIES.
		 */
		k_panic();
	}

	بازگشت to->heap_handle.idx == 1U;
}

/* Remove an arbitrary timeout from the heap. Returns true if it was the
 * earliest pending timeout. Used only by the abort path, never from announce.
 */
ایستا درونخطی bool z_timeout_q_remove(ساختار _timeout *to)
{
	bool was_first = (to->heap_handle.idx == 1U);

	(تهی)min_heap_ref_remove(&timeout_heap_inst, &to->heap_handle);

	بازگشت was_first;
}

/* Ticks from curr_tick until @to expires. Caller holds timeout_lock and has
 * verified that @to is active.
 */
ایستا درونخطی k_ticks_t z_timeout_q_remainder(ثابت ساختار _timeout *to)
{
	بازگشت (k_ticks_t)(to->abs_ticks - (int64_t)curr_tick);
}

/* Ticks from curr_tick to the earliest pending timeout, or K_TICKS_FOREVER if
 * the heap is empty. The announce-range cap is applied centrally by
 * next_timeout() in timeout.c.
 */
ایستا درونخطی k_ticks_t z_timeout_q_next_expiry(تهی)
{
	ساختار _timeout *t = z_timeout_q_ریشه();

	بازگشت (t == پوچ) ? K_TICKS_FOREVER
			   : (k_ticks_t)(t->abs_ticks - (int64_t)curr_tick);
}

/* --- announce-loop helpers (see sys_clock_announce_locked() in timeout.c) --- */

/* Ticks from curr_tick until the earliest expiry. A value larger than any
 * announce window when the heap is empty.
 */
ایستا درونخطی int32_t z_timeout_q_next_gap(تهی)
{
	ساختار _timeout *t = z_timeout_q_ریشه();

	اگر (t == پوچ) {
		بازگشت INT32_MAX;
	}

	بازگشت (int32_t)MIN(t->abs_ticks - (int64_t)curr_tick, (int64_t)INT32_MAX);
}

/* Advance backend state by @dt ticks. The heap stores absolute expiry ticks,
 * so the caller's curr_tick += dt is all the advancing required here.
 */
ایستا درونخطی تهی z_timeout_q_advance(int32_t dt)
{
	ARG_UNUSED(dt);
}

/* Pop and return one timeout that is due at the current curr_tick, or NULL if
 * none is due right now. Tied timeouts share the same abs_ticks and are
 * drained by successive calls.
 */
ایستا درونخطی ساختار _timeout *z_timeout_q_برداشت_due(تهی)
{
	ساختار _timeout *t = z_timeout_q_ریشه();

	اگر ((t == پوچ) || (t->abs_ticks > (int64_t)curr_tick)) {
		بازگشت پوچ;
	}

	بازگشت CONTAINER_OF(min_heap_ref_برداشت(&timeout_heap_inst), ساختار _timeout, heap_handle);
}

#endif /* ZEPHYR_KERNEL_TIMEOUT_MINHEAP_H_ */
