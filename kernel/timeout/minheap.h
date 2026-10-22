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

#ଅନ୍ତର୍ଭାଵ <zephyr/sys/min_heap_ref.h>

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା timeout_cmp(ଅଚଳ ସଂରଚନା min_heap_handle *a, ଅଚଳ ସଂରଚନା min_heap_handle *b)
{
	ଅଚଳ ସଂରଚନା _timeout *ta = CONTAINER_OF(a, ସଂରଚନା _timeout, heap_handle);
	ଅଚଳ ସଂରଚନା _timeout *tb = CONTAINER_OF(b, ସଂରଚନା _timeout, heap_handle);

	ଯଦି (ta->abs_ticks < tb->abs_ticks) {
		ଫେରନ୍ତୁ -1;
	}

	ଫେରନ୍ତୁ (ta->abs_ticks > tb->abs_ticks) ? 1 : 0;
}

MIN_HEAP_REF_DEFINE_STATIC(timeout_heap_inst, CONFIG_TIMEOUT_HEAP_MAX_ENTRIES, timeout_cmp);

ସ୍ଥିର ଧାଡ଼ିଭିତରେ ସଂରଚନା _timeout *z_timeout_q_ମୂଳ(ଶୂନ୍ୟ)
{
	ଅଚଳ ସଂରଚନା min_heap_handle *h = min_heap_ref_peek(&timeout_heap_inst);

	ଫେରନ୍ତୁ (h == ଖାଲି) ? ଖାଲି : CONTAINER_OF(h, ସଂରଚନା _timeout, heap_handle);
}

/* Insert @to so that it expires @dticks ticks after curr_tick. Returns true
 * if @to is now the earliest pending timeout (heap root).
 */
ସ୍ଥିର ଧାଡ଼ିଭିତରେ bool z_timeout_q_ଭର୍ତ୍ତି(ସଂରଚନା _timeout *to, k_ticks_t dticks)
{
	to->abs_ticks = (int64_t)curr_tick + dticks;

	ଯଦି (unlikely(min_heap_ref_ଠେଲ(&timeout_heap_inst, &to->heap_handle) != 0)) {
		/*
		 * Heap overflow is always a configuration error: there is no
		 * graceful way to drop a kernel timeout. Increase
		 * CONFIG_TIMEOUT_HEAP_MAX_ENTRIES.
		 */
		k_panic();
	}

	ଫେରନ୍ତୁ to->heap_handle.idx == 1U;
}

/* Remove an arbitrary timeout from the heap. Returns true if it was the
 * earliest pending timeout. Used only by the abort path, never from announce.
 */
ସ୍ଥିର ଧାଡ଼ିଭିତରେ bool z_timeout_q_remove(ସଂରଚନା _timeout *to)
{
	bool was_first = (to->heap_handle.idx == 1U);

	(ଶୂନ୍ୟ)min_heap_ref_remove(&timeout_heap_inst, &to->heap_handle);

	ଫେରନ୍ତୁ was_first;
}

/* Ticks from curr_tick until @to expires. Caller holds timeout_lock and has
 * verified that @to is active.
 */
ସ୍ଥିର ଧାଡ଼ିଭିତରେ k_ticks_t z_timeout_q_remainder(ଅଚଳ ସଂରଚନା _timeout *to)
{
	ଫେରନ୍ତୁ (k_ticks_t)(to->abs_ticks - (int64_t)curr_tick);
}

/* Ticks from curr_tick to the earliest pending timeout, or K_TICKS_FOREVER if
 * the heap is empty. The announce-range cap is applied centrally by
 * next_timeout() in timeout.c.
 */
ସ୍ଥିର ଧାଡ଼ିଭିତରେ k_ticks_t z_timeout_q_next_expiry(ଶୂନ୍ୟ)
{
	ସଂରଚନା _timeout *t = z_timeout_q_ମୂଳ();

	ଫେରନ୍ତୁ (t == ଖାଲି) ? K_TICKS_FOREVER
			   : (k_ticks_t)(t->abs_ticks - (int64_t)curr_tick);
}

/* --- announce-loop helpers (see sys_clock_announce_locked() in timeout.c) --- */

/* Ticks from curr_tick until the earliest expiry. A value larger than any
 * announce window when the heap is empty.
 */
ସ୍ଥିର ଧାଡ଼ିଭିତରେ int32_t z_timeout_q_next_gap(ଶୂନ୍ୟ)
{
	ସଂରଚନା _timeout *t = z_timeout_q_ମୂଳ();

	ଯଦି (t == ଖାଲି) {
		ଫେରନ୍ତୁ INT32_MAX;
	}

	ଫେରନ୍ତୁ (int32_t)MIN(t->abs_ticks - (int64_t)curr_tick, (int64_t)INT32_MAX);
}

/* Advance backend state by @dt ticks. The heap stores absolute expiry ticks,
 * so the caller's curr_tick += dt is all the advancing required here.
 */
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ z_timeout_q_advance(int32_t dt)
{
	ARG_UNUSED(dt);
}

/* Pop and return one timeout that is due at the current curr_tick, or NULL if
 * none is due right now. Tied timeouts share the same abs_ticks and are
 * drained by successive calls.
 */
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ସଂରଚନା _timeout *z_timeout_q_କାଢ଼_due(ଶୂନ୍ୟ)
{
	ସଂରଚନା _timeout *t = z_timeout_q_ମୂଳ();

	ଯଦି ((t == ଖାଲି) || (t->abs_ticks > (int64_t)curr_tick)) {
		ଫେରନ୍ତୁ ଖାଲି;
	}

	ଫେରନ୍ତୁ CONTAINER_OF(min_heap_ref_କାଢ଼(&timeout_heap_inst), ସଂରଚନା _timeout, heap_handle);
}

#endif /* ZEPHYR_KERNEL_TIMEOUT_MINHEAP_H_ */
