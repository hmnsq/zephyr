/*
 * Copyright (c) 2026 Aerlync Labs Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ ZEPHYR_KERNEL_TIMEOUT_MINHEAP_H_
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ZEPHYR_KERNEL_TIMEOUT_MINHEAP_H_

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

#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/sys/min_heap_ref.h>

ਸਥਿਰ ਪੂਰਨ timeout_cmp(ਅਚਲ ਬਣਤਰ min_ਢੇਰ_handle *a, ਅਚਲ ਬਣਤਰ min_ਢੇਰ_handle *b)
{
	ਅਚਲ ਬਣਤਰ _timeout *ta = CONTAINER_ਦਾ(a, ਬਣਤਰ _timeout, ਢੇਰ_handle);
	ਅਚਲ ਬਣਤਰ _timeout *tb = CONTAINER_ਦਾ(b, ਬਣਤਰ _timeout, ਢੇਰ_handle);

	ਜੇ (ta->abs_ticks < tb->abs_ticks) {
		ਵਾਪਸ -1;
	}

	ਵਾਪਸ (ta->abs_ticks > tb->abs_ticks) ? 1 : 0;
}

MIN_ਢੇਰ_REF_DEFINE_STATIC(timeout_ਢੇਰ_inst, CONFIG_TIMEOUT_ਢੇਰ_MAX_ENTRIES, timeout_cmp);

ਸਥਿਰ ਸਤਰਅੰਦਰ ਬਣਤਰ _timeout *z_timeout_q_ਜੜ੍ਹ(ਖਾਲੀ)
{
	ਅਚਲ ਬਣਤਰ min_ਢੇਰ_handle *h = min_ਢੇਰ_ref_peek(&timeout_ਢੇਰ_inst);

	ਵਾਪਸ (h == ਨਿਰਾ) ? ਨਿਰਾ : CONTAINER_ਦਾ(h, ਬਣਤਰ _timeout, ਢੇਰ_handle);
}

/* Insert @to so that it expires @dticks ticks after curr_tick. Returns true
 * if @to is now the earliest pending timeout (heap root).
 */
ਸਥਿਰ ਸਤਰਅੰਦਰ bool z_timeout_q_ਪਾਓ(ਬਣਤਰ _timeout *ਮੰਜ਼ਿਲ, k_ticks_t dticks)
{
	ਮੰਜ਼ਿਲ->abs_ticks = (int64_t)curr_tick + dticks;

	ਜੇ (unlikely(min_ਢੇਰ_ref_ਧੱਕੋ(&timeout_ਢੇਰ_inst, &ਮੰਜ਼ਿਲ->ਢੇਰ_handle) != 0)) {
		/*
		 * Heap overflow is always a configuration error: there is no
		 * graceful way to drop a kernel timeout. Increase
		 * CONFIG_TIMEOUT_HEAP_MAX_ENTRIES.
		 */
		k_panic();
	}

	ਵਾਪਸ ਮੰਜ਼ਿਲ->ਢੇਰ_handle.idx == 1U;
}

/* Remove an arbitrary timeout from the heap. Returns true if it was the
 * earliest pending timeout. Used only by the abort path, never from announce.
 */
ਸਥਿਰ ਸਤਰਅੰਦਰ bool z_timeout_q_remove(ਬਣਤਰ _timeout *ਮੰਜ਼ਿਲ)
{
	bool was_ਪਹਿਲਾ = (ਮੰਜ਼ਿਲ->ਢੇਰ_handle.idx == 1U);

	(ਖਾਲੀ)min_ਢੇਰ_ref_remove(&timeout_ਢੇਰ_inst, &ਮੰਜ਼ਿਲ->ਢੇਰ_handle);

	ਵਾਪਸ was_ਪਹਿਲਾ;
}

/* Ticks from curr_tick until @to expires. Caller holds timeout_lock and has
 * verified that @to is active.
 */
ਸਥਿਰ ਸਤਰਅੰਦਰ k_ticks_t z_timeout_q_remainder(ਅਚਲ ਬਣਤਰ _timeout *ਮੰਜ਼ਿਲ)
{
	ਵਾਪਸ (k_ticks_t)(ਮੰਜ਼ਿਲ->abs_ticks - (int64_t)curr_tick);
}

/* Ticks from curr_tick to the earliest pending timeout, or K_TICKS_FOREVER if
 * the heap is empty. The announce-range cap is applied centrally by
 * next_timeout() in timeout.c.
 */
ਸਥਿਰ ਸਤਰਅੰਦਰ k_ticks_t z_timeout_q_ਅਗਲਾ_expiry(ਖਾਲੀ)
{
	ਬਣਤਰ _timeout *t = z_timeout_q_ਜੜ੍ਹ();

	ਵਾਪਸ (t == ਨਿਰਾ) ? K_TICKS_FOREVER
			   : (k_ticks_t)(t->abs_ticks - (int64_t)curr_tick);
}

/* --- announce-loop helpers (see sys_clock_announce_locked() in timeout.c) --- */

/* Ticks from curr_tick until the earliest expiry. A value larger than any
 * announce window when the heap is empty.
 */
ਸਥਿਰ ਸਤਰਅੰਦਰ int32_t z_timeout_q_ਅਗਲਾ_gap(ਖਾਲੀ)
{
	ਬਣਤਰ _timeout *t = z_timeout_q_ਜੜ੍ਹ();

	ਜੇ (t == ਨਿਰਾ) {
		ਵਾਪਸ INT32_MAX;
	}

	ਵਾਪਸ (int32_t)MIN(t->abs_ticks - (int64_t)curr_tick, (int64_t)INT32_MAX);
}

/* Advance backend state by @dt ticks. The heap stores absolute expiry ticks,
 * so the caller's curr_tick += dt is all the advancing required here.
 */
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ z_timeout_q_advance(int32_t dt)
{
	ARG_ਅਣਵਰਤਿਆ(dt);
}

/* Pop and return one timeout that is due at the current curr_tick, or NULL if
 * none is due right now. Tied timeouts share the same abs_ticks and are
 * drained by successive calls.
 */
ਸਥਿਰ ਸਤਰਅੰਦਰ ਬਣਤਰ _timeout *z_timeout_q_ਕੱਢੋ_due(ਖਾਲੀ)
{
	ਬਣਤਰ _timeout *t = z_timeout_q_ਜੜ੍ਹ();

	ਜੇ ((t == ਨਿਰਾ) || (t->abs_ticks > (int64_t)curr_tick)) {
		ਵਾਪਸ ਨਿਰਾ;
	}

	ਵਾਪਸ CONTAINER_ਦਾ(min_ਢੇਰ_ref_ਕੱਢੋ(&timeout_ਢੇਰ_inst), ਬਣਤਰ _timeout, ਢੇਰ_handle);
}

#ਜੇ_ਅੰਤ /* ZEPHYR_KERNEL_TIMEOUT_MINHEAP_H_ */
