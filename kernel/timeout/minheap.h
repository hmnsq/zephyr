/*
 * Copyright (c) 2026 Aerlync Labs Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#अगर_अघोषित ZEPHYR_KERNEL_TIMEOUT_MINHEAP_H_
#घोषणा ZEPHYR_KERNEL_TIMEOUT_MINHEAP_H_

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

#समावेश <zephyr/sys/min_heap_ref.h>

अटल पूर्णांक timeout_cmp(स्थिर काष्ठा min_heap_handle *a, स्थिर काष्ठा min_heap_handle *b)
{
	स्थिर काष्ठा _timeout *ta = CONTAINER_OF(a, काष्ठा _timeout, heap_handle);
	स्थिर काष्ठा _timeout *tb = CONTAINER_OF(b, काष्ठा _timeout, heap_handle);

	अगर (ta->abs_ticks < tb->abs_ticks) {
		वापस -1;
	}

	वापस (ta->abs_ticks > tb->abs_ticks) ? 1 : 0;
}

MIN_HEAP_REF_DEFINE_STATIC(timeout_heap_inst, CONFIG_TIMEOUT_HEAP_MAX_ENTRIES, timeout_cmp);

अटल अंतरभूत काष्ठा _timeout *z_timeout_q_मूल(व्योम)
{
	स्थिर काष्ठा min_heap_handle *h = min_heap_ref_peek(&timeout_heap_inst);

	वापस (h == शून्य) ? शून्य : CONTAINER_OF(h, काष्ठा _timeout, heap_handle);
}

/* Insert @to so that it expires @dticks ticks after curr_tick. Returns true
 * if @to is now the earliest pending timeout (heap root).
 */
अटल अंतरभूत bool z_timeout_q_जोड़ो(काष्ठा _timeout *to, k_ticks_t dticks)
{
	to->abs_ticks = (int64_t)curr_tick + dticks;

	अगर (unlikely(min_heap_ref_धकेलो(&timeout_heap_inst, &to->heap_handle) != 0)) {
		/*
		 * Heap overflow is always a configuration error: there is no
		 * graceful way to drop a kernel timeout. Increase
		 * CONFIG_TIMEOUT_HEAP_MAX_ENTRIES.
		 */
		k_panic();
	}

	वापस to->heap_handle.idx == 1U;
}

/* Remove an arbitrary timeout from the heap. Returns true if it was the
 * earliest pending timeout. Used only by the abort path, never from announce.
 */
अटल अंतरभूत bool z_timeout_q_remove(काष्ठा _timeout *to)
{
	bool was_first = (to->heap_handle.idx == 1U);

	(व्योम)min_heap_ref_remove(&timeout_heap_inst, &to->heap_handle);

	वापस was_first;
}

/* Ticks from curr_tick until @to expires. Caller holds timeout_lock and has
 * verified that @to is active.
 */
अटल अंतरभूत k_ticks_t z_timeout_q_remainder(स्थिर काष्ठा _timeout *to)
{
	वापस (k_ticks_t)(to->abs_ticks - (int64_t)curr_tick);
}

/* Ticks from curr_tick to the earliest pending timeout, or K_TICKS_FOREVER if
 * the heap is empty. The announce-range cap is applied centrally by
 * next_timeout() in timeout.c.
 */
अटल अंतरभूत k_ticks_t z_timeout_q_अग्र_expiry(व्योम)
{
	काष्ठा _timeout *t = z_timeout_q_मूल();

	वापस (t == शून्य) ? K_TICKS_FOREVER
			   : (k_ticks_t)(t->abs_ticks - (int64_t)curr_tick);
}

/* --- announce-loop helpers (see sys_clock_announce_locked() in timeout.c) --- */

/* Ticks from curr_tick until the earliest expiry. A value larger than any
 * announce window when the heap is empty.
 */
अटल अंतरभूत int32_t z_timeout_q_अग्र_gap(व्योम)
{
	काष्ठा _timeout *t = z_timeout_q_मूल();

	अगर (t == शून्य) {
		वापस INT32_MAX;
	}

	वापस (int32_t)MIN(t->abs_ticks - (int64_t)curr_tick, (int64_t)INT32_MAX);
}

/* Advance backend state by @dt ticks. The heap stores absolute expiry ticks,
 * so the caller's curr_tick += dt is all the advancing required here.
 */
अटल अंतरभूत व्योम z_timeout_q_advance(int32_t dt)
{
	ARG_UNUSED(dt);
}

/* Pop and return one timeout that is due at the current curr_tick, or NULL if
 * none is due right now. Tied timeouts share the same abs_ticks and are
 * drained by successive calls.
 */
अटल अंतरभूत काष्ठा _timeout *z_timeout_q_निकालो_due(व्योम)
{
	काष्ठा _timeout *t = z_timeout_q_मूल();

	अगर ((t == शून्य) || (t->abs_ticks > (int64_t)curr_tick)) {
		वापस शून्य;
	}

	वापस CONTAINER_OF(min_heap_ref_निकालो(&timeout_heap_inst), काष्ठा _timeout, heap_handle);
}

#पूर्ण_अगर /* ZEPHYR_KERNEL_TIMEOUT_MINHEAP_H_ */
