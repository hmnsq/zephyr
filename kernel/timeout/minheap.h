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

#अन्तर्भाव <zephyr/sys/min_heap_ref.h>

स्थिर पूर्णाङ्क timeout_cmp(ध्रुव संरचना min_heap_handle *a, ध्रुव संरचना min_heap_handle *b)
{
	ध्रुव संरचना _timeout *ta = CONTAINER_OF(a, संरचना _timeout, heap_handle);
	ध्रुव संरचना _timeout *tb = CONTAINER_OF(b, संरचना _timeout, heap_handle);

	यदि (ta->abs_ticks < tb->abs_ticks) {
		प्रत्यागम -1;
	}

	प्रत्यागम (ta->abs_ticks > tb->abs_ticks) ? 1 : 0;
}

MIN_HEAP_REF_DEFINE_STATIC(timeout_heap_inst, CONFIG_TIMEOUT_HEAP_MAX_ENTRIES, timeout_cmp);

स्थिर अन्तःस्थ संरचना _timeout *z_timeout_q_मूल(शून्य)
{
	ध्रुव संरचना min_heap_handle *h = min_heap_ref_peek(&timeout_heap_inst);

	प्रत्यागम (h == रिक्त) ? रिक्त : CONTAINER_OF(h, संरचना _timeout, heap_handle);
}

/* Insert @to so that it expires @dticks ticks after curr_tick. Returns true
 * if @to is now the earliest pending timeout (heap root).
 */
स्थिर अन्तःस्थ bool z_timeout_q_जोड़ो(संरचना _timeout *to, k_ticks_t dticks)
{
	to->abs_ticks = (int64_t)curr_tick + dticks;

	यदि (unlikely(min_heap_ref_धकेलो(&timeout_heap_inst, &to->heap_handle) != 0)) {
		/*
		 * Heap overflow is always a configuration error: there is no
		 * graceful way to drop a kernel timeout. Increase
		 * CONFIG_TIMEOUT_HEAP_MAX_ENTRIES.
		 */
		k_panic();
	}

	प्रत्यागम to->heap_handle.idx == 1U;
}

/* Remove an arbitrary timeout from the heap. Returns true if it was the
 * earliest pending timeout. Used only by the abort path, never from announce.
 */
स्थिर अन्तःस्थ bool z_timeout_q_remove(संरचना _timeout *to)
{
	bool was_first = (to->heap_handle.idx == 1U);

	(शून्य)min_heap_ref_remove(&timeout_heap_inst, &to->heap_handle);

	प्रत्यागम was_first;
}

/* Ticks from curr_tick until @to expires. Caller holds timeout_lock and has
 * verified that @to is active.
 */
स्थिर अन्तःस्थ k_ticks_t z_timeout_q_remainder(ध्रुव संरचना _timeout *to)
{
	प्रत्यागम (k_ticks_t)(to->abs_ticks - (int64_t)curr_tick);
}

/* Ticks from curr_tick to the earliest pending timeout, or K_TICKS_FOREVER if
 * the heap is empty. The announce-range cap is applied centrally by
 * next_timeout() in timeout.c.
 */
स्थिर अन्तःस्थ k_ticks_t z_timeout_q_next_expiry(शून्य)
{
	संरचना _timeout *t = z_timeout_q_मूल();

	प्रत्यागम (t == रिक्त) ? K_TICKS_FOREVER
			   : (k_ticks_t)(t->abs_ticks - (int64_t)curr_tick);
}

/* --- announce-loop helpers (see sys_clock_announce_locked() in timeout.c) --- */

/* Ticks from curr_tick until the earliest expiry. A value larger than any
 * announce window when the heap is empty.
 */
स्थिर अन्तःस्थ int32_t z_timeout_q_next_gap(शून्य)
{
	संरचना _timeout *t = z_timeout_q_मूल();

	यदि (t == रिक्त) {
		प्रत्यागम INT32_MAX;
	}

	प्रत्यागम (int32_t)MIN(t->abs_ticks - (int64_t)curr_tick, (int64_t)INT32_MAX);
}

/* Advance backend state by @dt ticks. The heap stores absolute expiry ticks,
 * so the caller's curr_tick += dt is all the advancing required here.
 */
स्थिर अन्तःस्थ शून्य z_timeout_q_advance(int32_t dt)
{
	ARG_UNUSED(dt);
}

/* Pop and return one timeout that is due at the current curr_tick, or NULL if
 * none is due right now. Tied timeouts share the same abs_ticks and are
 * drained by successive calls.
 */
स्थिर अन्तःस्थ संरचना _timeout *z_timeout_q_निकालो_due(शून्य)
{
	संरचना _timeout *t = z_timeout_q_मूल();

	यदि ((t == रिक्त) || (t->abs_ticks > (int64_t)curr_tick)) {
		प्रत्यागम रिक्त;
	}

	प्रत्यागम CONTAINER_OF(min_heap_ref_निकालो(&timeout_heap_inst), संरचना _timeout, heap_handle);
}

#endif /* ZEPHYR_KERNEL_TIMEOUT_MINHEAP_H_ */
