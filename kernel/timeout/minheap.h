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

#അന്തര്ഭാവ <zephyr/sys/min_heap_ref.h>

സ്ഥിതം പൂർണ്ണസംഖ്യ timeout_cmp(സ്ഥിരം ഘടന min_heap_handle *a, സ്ഥിരം ഘടന min_heap_handle *b)
{
	സ്ഥിരം ഘടന _timeout *ta = CONTAINER_OF(a, ഘടന _timeout, heap_handle);
	സ്ഥിരം ഘടന _timeout *tb = CONTAINER_OF(b, ഘടന _timeout, heap_handle);

	എങ്കിൽ (ta->abs_ticks < tb->abs_ticks) {
		തിരികെ -1;
	}

	തിരികെ (ta->abs_ticks > tb->abs_ticks) ? 1 : 0;
}

MIN_HEAP_REF_DEFINE_STATIC(timeout_heap_inst, CONFIG_TIMEOUT_HEAP_MAX_ENTRIES, timeout_cmp);

സ്ഥിതം വരിയിൽ ഘടന _timeout *z_timeout_q_വേര്(ശൂന്യം)
{
	സ്ഥിരം ഘടന min_heap_handle *h = min_heap_ref_peek(&timeout_heap_inst);

	തിരികെ (h == ശൂന്യ) ? ശൂന്യ : CONTAINER_OF(h, ഘടന _timeout, heap_handle);
}

/* Insert @to so that it expires @dticks ticks after curr_tick. Returns true
 * if @to is now the earliest pending timeout (heap root).
 */
സ്ഥിതം വരിയിൽ bool z_timeout_q_തിരുകുക(ഘടന _timeout *to, k_ticks_t dticks)
{
	to->abs_ticks = (int64_t)curr_tick + dticks;

	എങ്കിൽ (unlikely(min_heap_ref_തള്ളുക(&timeout_heap_inst, &to->heap_handle) != 0)) {
		/*
		 * Heap overflow is always a configuration error: there is no
		 * graceful way to drop a kernel timeout. Increase
		 * CONFIG_TIMEOUT_HEAP_MAX_ENTRIES.
		 */
		k_panic();
	}

	തിരികെ to->heap_handle.idx == 1U;
}

/* Remove an arbitrary timeout from the heap. Returns true if it was the
 * earliest pending timeout. Used only by the abort path, never from announce.
 */
സ്ഥിതം വരിയിൽ bool z_timeout_q_remove(ഘടന _timeout *to)
{
	bool was_first = (to->heap_handle.idx == 1U);

	(ശൂന്യം)min_heap_ref_remove(&timeout_heap_inst, &to->heap_handle);

	തിരികെ was_first;
}

/* Ticks from curr_tick until @to expires. Caller holds timeout_lock and has
 * verified that @to is active.
 */
സ്ഥിതം വരിയിൽ k_ticks_t z_timeout_q_remainder(സ്ഥിരം ഘടന _timeout *to)
{
	തിരികെ (k_ticks_t)(to->abs_ticks - (int64_t)curr_tick);
}

/* Ticks from curr_tick to the earliest pending timeout, or K_TICKS_FOREVER if
 * the heap is empty. The announce-range cap is applied centrally by
 * next_timeout() in timeout.c.
 */
സ്ഥിതം വരിയിൽ k_ticks_t z_timeout_q_next_expiry(ശൂന്യം)
{
	ഘടന _timeout *t = z_timeout_q_വേര്();

	തിരികെ (t == ശൂന്യ) ? K_TICKS_FOREVER
			   : (k_ticks_t)(t->abs_ticks - (int64_t)curr_tick);
}

/* --- announce-loop helpers (see sys_clock_announce_locked() in timeout.c) --- */

/* Ticks from curr_tick until the earliest expiry. A value larger than any
 * announce window when the heap is empty.
 */
സ്ഥിതം വരിയിൽ int32_t z_timeout_q_next_gap(ശൂന്യം)
{
	ഘടന _timeout *t = z_timeout_q_വേര്();

	എങ്കിൽ (t == ശൂന്യ) {
		തിരികെ INT32_MAX;
	}

	തിരികെ (int32_t)MIN(t->abs_ticks - (int64_t)curr_tick, (int64_t)INT32_MAX);
}

/* Advance backend state by @dt ticks. The heap stores absolute expiry ticks,
 * so the caller's curr_tick += dt is all the advancing required here.
 */
സ്ഥിതം വരിയിൽ ശൂന്യം z_timeout_q_advance(int32_t dt)
{
	ARG_UNUSED(dt);
}

/* Pop and return one timeout that is due at the current curr_tick, or NULL if
 * none is due right now. Tied timeouts share the same abs_ticks and are
 * drained by successive calls.
 */
സ്ഥിതം വരിയിൽ ഘടന _timeout *z_timeout_q_എടുക്കുക_due(ശൂന്യം)
{
	ഘടന _timeout *t = z_timeout_q_വേര്();

	എങ്കിൽ ((t == ശൂന്യ) || (t->abs_ticks > (int64_t)curr_tick)) {
		തിരികെ ശൂന്യ;
	}

	തിരികെ CONTAINER_OF(min_heap_ref_എടുക്കുക(&timeout_heap_inst), ഘടന _timeout, heap_handle);
}

#endif /* ZEPHYR_KERNEL_TIMEOUT_MINHEAP_H_ */
