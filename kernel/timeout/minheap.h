/*
 * Copyright (c) 2026 Aerlync Labs Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#నిర్వచితం_కాకపోతే ZEPHYR_KERNEL_TIMEOUT_MINHEAP_H_
#నిర్వచించు ZEPHYR_KERNEL_TIMEOUT_MINHEAP_H_

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

#అన్తర్భావ <zephyr/sys/min_heap_ref.h>

స్థిర పూర్ణాంకం timeout_cmp(స్థిరరాశి నిర్మాణం min_కుప్ప_handle *a, స్థిరరాశి నిర్మాణం min_కుప్ప_handle *b)
{
	స్థిరరాశి నిర్మాణం _timeout *ta = CONTAINER_యొక్క(a, నిర్మాణం _timeout, కుప్ప_handle);
	స్థిరరాశి నిర్మాణం _timeout *tb = CONTAINER_యొక్క(b, నిర్మాణం _timeout, కుప్ప_handle);

	ఒకవేళ (ta->abs_ticks < tb->abs_ticks) {
		తిరిగి -1;
	}

	తిరిగి (ta->abs_ticks > tb->abs_ticks) ? 1 : 0;
}

MIN_కుప్ప_REF_DEFINE_STATIC(timeout_కుప్ప_inst, CONFIG_TIMEOUT_కుప్ప_MAX_ENTRIES, timeout_cmp);

స్థిర అంతఃవరుస నిర్మాణం _timeout *z_timeout_q_మూలం(శూన్య)
{
	స్థిరరాశి నిర్మాణం min_కుప్ప_handle *h = min_కుప్ప_ref_peek(&timeout_కుప్ప_inst);

	తిరిగి (h == ఖాళీ) ? ఖాళీ : CONTAINER_యొక్క(h, నిర్మాణం _timeout, కుప్ప_handle);
}

/* Insert @to so that it expires @dticks ticks after curr_tick. Returns true
 * if @to is now the earliest pending timeout (heap root).
 */
స్థిర అంతఃవరుస bool z_timeout_q_చొప్పించు(నిర్మాణం _timeout *గమ్యం, k_ticks_t dticks)
{
	గమ్యం->abs_ticks = (int64_t)curr_tick + dticks;

	ఒకవేళ (unlikely(min_కుప్ప_ref_నెట్టు(&timeout_కుప్ప_inst, &గమ్యం->కుప్ప_handle) != 0)) {
		/*
		 * Heap overflow is always a configuration error: there is no
		 * graceful way to drop a kernel timeout. Increase
		 * CONFIG_TIMEOUT_HEAP_MAX_ENTRIES.
		 */
		k_panic();
	}

	తిరిగి గమ్యం->కుప్ప_handle.idx == 1U;
}

/* Remove an arbitrary timeout from the heap. Returns true if it was the
 * earliest pending timeout. Used only by the abort path, never from announce.
 */
స్థిర అంతఃవరుస bool z_timeout_q_remove(నిర్మాణం _timeout *గమ్యం)
{
	bool was_మొదటి = (గమ్యం->కుప్ప_handle.idx == 1U);

	(శూన్య)min_కుప్ప_ref_remove(&timeout_కుప్ప_inst, &గమ్యం->కుప్ప_handle);

	తిరిగి was_మొదటి;
}

/* Ticks from curr_tick until @to expires. Caller holds timeout_lock and has
 * verified that @to is active.
 */
స్థిర అంతఃవరుస k_ticks_t z_timeout_q_remainder(స్థిరరాశి నిర్మాణం _timeout *గమ్యం)
{
	తిరిగి (k_ticks_t)(గమ్యం->abs_ticks - (int64_t)curr_tick);
}

/* Ticks from curr_tick to the earliest pending timeout, or K_TICKS_FOREVER if
 * the heap is empty. The announce-range cap is applied centrally by
 * next_timeout() in timeout.c.
 */
స్థిర అంతఃవరుస k_ticks_t z_timeout_q_తదుపరి_expiry(శూన్య)
{
	నిర్మాణం _timeout *t = z_timeout_q_మూలం();

	తిరిగి (t == ఖాళీ) ? K_TICKS_FOREVER
			   : (k_ticks_t)(t->abs_ticks - (int64_t)curr_tick);
}

/* --- announce-loop helpers (see sys_clock_announce_locked() in timeout.c) --- */

/* Ticks from curr_tick until the earliest expiry. A value larger than any
 * announce window when the heap is empty.
 */
స్థిర అంతఃవరుస int32_t z_timeout_q_తదుపరి_gap(శూన్య)
{
	నిర్మాణం _timeout *t = z_timeout_q_మూలం();

	ఒకవేళ (t == ఖాళీ) {
		తిరిగి INT32_MAX;
	}

	తిరిగి (int32_t)MIN(t->abs_ticks - (int64_t)curr_tick, (int64_t)INT32_MAX);
}

/* Advance backend state by @dt ticks. The heap stores absolute expiry ticks,
 * so the caller's curr_tick += dt is all the advancing required here.
 */
స్థిర అంతఃవరుస శూన్య z_timeout_q_advance(int32_t dt)
{
	ARG_వాడని(dt);
}

/* Pop and return one timeout that is due at the current curr_tick, or NULL if
 * none is due right now. Tied timeouts share the same abs_ticks and are
 * drained by successive calls.
 */
స్థిర అంతఃవరుస నిర్మాణం _timeout *z_timeout_q_తీయి_due(శూన్య)
{
	నిర్మాణం _timeout *t = z_timeout_q_మూలం();

	ఒకవేళ ((t == ఖాళీ) || (t->abs_ticks > (int64_t)curr_tick)) {
		తిరిగి ఖాళీ;
	}

	తిరిగి CONTAINER_యొక్క(min_కుప్ప_ref_తీయి(&timeout_కుప్ప_inst), నిర్మాణం _timeout, కుప్ప_handle);
}

#అయితే_ముగింపు /* ZEPHYR_KERNEL_TIMEOUT_MINHEAP_H_ */
