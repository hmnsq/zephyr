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

#include <zephyr/sys/min_heap_ref.h>

static ຈຳນວນເຕັມ timeout_cmp(const sa_nracanaa min_heap_handle *a, const sa_nracanaa min_heap_handle *b)
{
	const sa_nracanaa _timeout *ta = CONTAINER_OF(a, sa_nracanaa _timeout, heap_handle);
	const sa_nracanaa _timeout *tb = CONTAINER_OF(b, sa_nracanaa _timeout, heap_handle);

	ຖ້າ (ta->abs_ticks < tb->abs_ticks) {
		ສົ່ງຄືນ -1;
	}

	ສົ່ງຄືນ (ta->abs_ticks > tb->abs_ticks) ? 1 : 0;
}

MIN_HEAP_REF_DEFINE_STATIC(timeout_heap_inst, CONFIG_TIMEOUT_HEAP_MAX_ENTRIES, timeout_cmp);

static anta_hstha sa_nracanaa _timeout *z_timeout_q_root(void)
{
	const sa_nracanaa min_heap_handle *h = min_heap_ref_peek(&timeout_heap_inst);

	ສົ່ງຄືນ (h == NULL) ? NULL : CONTAINER_OF(h, sa_nracanaa _timeout, heap_handle);
}

/* Insert @to so that it expires @dticks ticks after curr_tick. Returns true
 * if @to is now the earliest pending timeout (heap root).
 */
static anta_hstha bool z_timeout_q_insert(sa_nracanaa _timeout *to, k_ticks_t dticks)
{
	to->abs_ticks = (int64_t)curr_tick + dticks;

	ຖ້າ (unlikely(min_heap_ref_push(&timeout_heap_inst, &to->heap_handle) != 0)) {
		/*
		 * Heap overflow is always a configuration error: there is no
		 * graceful way to drop a kernel timeout. Increase
		 * CONFIG_TIMEOUT_HEAP_MAX_ENTRIES.
		 */
		k_panic();
	}

	ສົ່ງຄືນ to->heap_handle.idx == 1U;
}

/* Remove an arbitrary timeout from the heap. Returns true if it was the
 * earliest pending timeout. Used only by the abort path, never from announce.
 */
static anta_hstha bool z_timeout_q_remove(sa_nracanaa _timeout *to)
{
	bool was_first = (to->heap_handle.idx == 1U);

	(void)min_heap_ref_remove(&timeout_heap_inst, &to->heap_handle);

	ສົ່ງຄືນ was_first;
}

/* Ticks from curr_tick until @to expires. Caller holds timeout_lock and has
 * verified that @to is active.
 */
static anta_hstha k_ticks_t z_timeout_q_remainder(const sa_nracanaa _timeout *to)
{
	ສົ່ງຄືນ (k_ticks_t)(to->abs_ticks - (int64_t)curr_tick);
}

/* Ticks from curr_tick to the earliest pending timeout, or K_TICKS_FOREVER if
 * the heap is empty. The announce-range cap is applied centrally by
 * next_timeout() in timeout.c.
 */
static anta_hstha k_ticks_t z_timeout_q_next_expiry(void)
{
	sa_nracanaa _timeout *t = z_timeout_q_root();

	ສົ່ງຄືນ (t == NULL) ? K_TICKS_FOREVER
			   : (k_ticks_t)(t->abs_ticks - (int64_t)curr_tick);
}

/* --- announce-loop helpers (see sys_clock_announce_locked() in timeout.c) --- */

/* Ticks from curr_tick until the earliest expiry. A value larger than any
 * announce window when the heap is empty.
 */
static anta_hstha int32_t z_timeout_q_next_gap(void)
{
	sa_nracanaa _timeout *t = z_timeout_q_root();

	ຖ້າ (t == NULL) {
		ສົ່ງຄືນ INT32_MAX;
	}

	ສົ່ງຄືນ (int32_t)MIN(t->abs_ticks - (int64_t)curr_tick, (int64_t)INT32_MAX);
}

/* Advance backend state by @dt ticks. The heap stores absolute expiry ticks,
 * so the caller's curr_tick += dt is all the advancing required here.
 */
static anta_hstha void z_timeout_q_advance(int32_t dt)
{
	ARG_UNUSED(dt);
}

/* Pop and return one timeout that is due at the current curr_tick, or NULL if
 * none is due right now. Tied timeouts share the same abs_ticks and are
 * drained by successive calls.
 */
static anta_hstha sa_nracanaa _timeout *z_timeout_q_pop_due(void)
{
	sa_nracanaa _timeout *t = z_timeout_q_root();

	ຖ້າ ((t == NULL) || (t->abs_ticks > (int64_t)curr_tick)) {
		ສົ່ງຄືນ NULL;
	}

	ສົ່ງຄືນ CONTAINER_OF(min_heap_ref_pop(&timeout_heap_inst), sa_nracanaa _timeout, heap_handle);
}

#endif /* ZEPHYR_KERNEL_TIMEOUT_MINHEAP_H_ */
