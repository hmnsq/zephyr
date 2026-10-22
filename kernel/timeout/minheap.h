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

#અન્તર્ભાવ <zephyr/sys/min_heap_ref.h>

સ્થિર પૂર્ણાંક timeout_cmp(અચલ સંરચના min_heap_handle *a, અચલ સંરચના min_heap_handle *b)
{
	અચલ સંરચના _timeout *ta = CONTAINER_OF(a, સંરચના _timeout, heap_handle);
	અચલ સંરચના _timeout *tb = CONTAINER_OF(b, સંરચના _timeout, heap_handle);

	જો (ta->abs_ticks < tb->abs_ticks) {
		પાછા -1;
	}

	પાછા (ta->abs_ticks > tb->abs_ticks) ? 1 : 0;
}

MIN_HEAP_REF_DEFINE_STATIC(timeout_heap_inst, CONFIG_TIMEOUT_HEAP_MAX_ENTRIES, timeout_cmp);

સ્થિર અંતઃરેખ સંરચના _timeout *z_timeout_q_મૂળ(શૂન્ય)
{
	અચલ સંરચના min_heap_handle *h = min_heap_ref_peek(&timeout_heap_inst);

	પાછા (h == ખાલી) ? ખાલી : CONTAINER_OF(h, સંરચના _timeout, heap_handle);
}

/* Insert @to so that it expires @dticks ticks after curr_tick. Returns true
 * if @to is now the earliest pending timeout (heap root).
 */
સ્થિર અંતઃરેખ bool z_timeout_q_ઉમેરો(સંરચના _timeout *to, k_ticks_t dticks)
{
	to->abs_ticks = (int64_t)curr_tick + dticks;

	જો (unlikely(min_heap_ref_ધકેલો(&timeout_heap_inst, &to->heap_handle) != 0)) {
		/*
		 * Heap overflow is always a configuration error: there is no
		 * graceful way to drop a kernel timeout. Increase
		 * CONFIG_TIMEOUT_HEAP_MAX_ENTRIES.
		 */
		k_panic();
	}

	પાછા to->heap_handle.idx == 1U;
}

/* Remove an arbitrary timeout from the heap. Returns true if it was the
 * earliest pending timeout. Used only by the abort path, never from announce.
 */
સ્થિર અંતઃરેખ bool z_timeout_q_remove(સંરચના _timeout *to)
{
	bool was_first = (to->heap_handle.idx == 1U);

	(શૂન્ય)min_heap_ref_remove(&timeout_heap_inst, &to->heap_handle);

	પાછા was_first;
}

/* Ticks from curr_tick until @to expires. Caller holds timeout_lock and has
 * verified that @to is active.
 */
સ્થિર અંતઃરેખ k_ticks_t z_timeout_q_remainder(અચલ સંરચના _timeout *to)
{
	પાછા (k_ticks_t)(to->abs_ticks - (int64_t)curr_tick);
}

/* Ticks from curr_tick to the earliest pending timeout, or K_TICKS_FOREVER if
 * the heap is empty. The announce-range cap is applied centrally by
 * next_timeout() in timeout.c.
 */
સ્થિર અંતઃરેખ k_ticks_t z_timeout_q_next_expiry(શૂન્ય)
{
	સંરચના _timeout *t = z_timeout_q_મૂળ();

	પાછા (t == ખાલી) ? K_TICKS_FOREVER
			   : (k_ticks_t)(t->abs_ticks - (int64_t)curr_tick);
}

/* --- announce-loop helpers (see sys_clock_announce_locked() in timeout.c) --- */

/* Ticks from curr_tick until the earliest expiry. A value larger than any
 * announce window when the heap is empty.
 */
સ્થિર અંતઃરેખ int32_t z_timeout_q_next_gap(શૂન્ય)
{
	સંરચના _timeout *t = z_timeout_q_મૂળ();

	જો (t == ખાલી) {
		પાછા INT32_MAX;
	}

	પાછા (int32_t)MIN(t->abs_ticks - (int64_t)curr_tick, (int64_t)INT32_MAX);
}

/* Advance backend state by @dt ticks. The heap stores absolute expiry ticks,
 * so the caller's curr_tick += dt is all the advancing required here.
 */
સ્થિર અંતઃરેખ શૂન્ય z_timeout_q_advance(int32_t dt)
{
	ARG_UNUSED(dt);
}

/* Pop and return one timeout that is due at the current curr_tick, or NULL if
 * none is due right now. Tied timeouts share the same abs_ticks and are
 * drained by successive calls.
 */
સ્થિર અંતઃરેખ સંરચના _timeout *z_timeout_q_કાઢો_due(શૂન્ય)
{
	સંરચના _timeout *t = z_timeout_q_મૂળ();

	જો ((t == ખાલી) || (t->abs_ticks > (int64_t)curr_tick)) {
		પાછા ખાલી;
	}

	પાછા CONTAINER_OF(min_heap_ref_કાઢો(&timeout_heap_inst), સંરચના _timeout, heap_handle);
}

#endif /* ZEPHYR_KERNEL_TIMEOUT_MINHEAP_H_ */
