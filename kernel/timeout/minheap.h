/*
 * Copyright (c) 2026 Aerlync Labs Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#اگر_غیر_تعریف_شدہ ZEPHYR_KERNEL_TIMEOUT_MINHEAP_H_
#تعریف_کرو ZEPHYR_KERNEL_TIMEOUT_MINHEAP_H_

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

ساکن صحیح timeout_cmp(ثابت ساخت min_ڈھیر_handle *a, ثابت ساخت min_ڈھیر_handle *b)
{
	ثابت ساخت _timeout *ta = CONTAINER_کا(a, ساخت _timeout, ڈھیر_handle);
	ثابت ساخت _timeout *tb = CONTAINER_کا(b, ساخت _timeout, ڈھیر_handle);

	اگر (ta->abs_ticks < tb->abs_ticks) {
		واپس -1;
	}

	واپس (ta->abs_ticks > tb->abs_ticks) ? 1 : 0;
}

MIN_ڈھیر_REF_DEFINE_STATIC(timeout_ڈھیر_inst, CONFIG_TIMEOUT_ڈھیر_MAX_ENTRIES, timeout_cmp);

ساکن اندرونی ساخت _timeout *z_timeout_q_جڑ(خالی)
{
	ثابت ساخت min_ڈھیر_handle *h = min_ڈھیر_ref_peek(&timeout_ڈھیر_inst);

	واپس (h == تہی) ? تہی : CONTAINER_کا(h, ساخت _timeout, ڈھیر_handle);
}

/* Insert @to so that it expires @dticks ticks after curr_tick. Returns true
 * if @to is now the earliest pending timeout (heap root).
 */
ساکن اندرونی bool z_timeout_q_داخل_کرو(ساخت _timeout *مقصد, k_ticks_t dticks)
{
	مقصد->abs_ticks = (int64_t)curr_tick + dticks;

	اگر (unlikely(min_ڈھیر_ref_ڈالو(&timeout_ڈھیر_inst, &مقصد->ڈھیر_handle) != 0)) {
		/*
		 * Heap overflow is always a configuration error: there is no
		 * graceful way to drop a kernel timeout. Increase
		 * CONFIG_TIMEOUT_HEAP_MAX_ENTRIES.
		 */
		k_panic();
	}

	واپس مقصد->ڈھیر_handle.idx == 1U;
}

/* Remove an arbitrary timeout from the heap. Returns true if it was the
 * earliest pending timeout. Used only by the abort path, never from announce.
 */
ساکن اندرونی bool z_timeout_q_remove(ساخت _timeout *مقصد)
{
	bool was_پہلا = (مقصد->ڈھیر_handle.idx == 1U);

	(خالی)min_ڈھیر_ref_remove(&timeout_ڈھیر_inst, &مقصد->ڈھیر_handle);

	واپس was_پہلا;
}

/* Ticks from curr_tick until @to expires. Caller holds timeout_lock and has
 * verified that @to is active.
 */
ساکن اندرونی k_ticks_t z_timeout_q_remainder(ثابت ساخت _timeout *مقصد)
{
	واپس (k_ticks_t)(مقصد->abs_ticks - (int64_t)curr_tick);
}

/* Ticks from curr_tick to the earliest pending timeout, or K_TICKS_FOREVER if
 * the heap is empty. The announce-range cap is applied centrally by
 * next_timeout() in timeout.c.
 */
ساکن اندرونی k_ticks_t z_timeout_q_اگلا_expiry(خالی)
{
	ساخت _timeout *t = z_timeout_q_جڑ();

	واپس (t == تہی) ? K_TICKS_FOREVER
			   : (k_ticks_t)(t->abs_ticks - (int64_t)curr_tick);
}

/* --- announce-loop helpers (see sys_clock_announce_locked() in timeout.c) --- */

/* Ticks from curr_tick until the earliest expiry. A value larger than any
 * announce window when the heap is empty.
 */
ساکن اندرونی int32_t z_timeout_q_اگلا_gap(خالی)
{
	ساخت _timeout *t = z_timeout_q_جڑ();

	اگر (t == تہی) {
		واپس INT32_MAX;
	}

	واپس (int32_t)MIN(t->abs_ticks - (int64_t)curr_tick, (int64_t)INT32_MAX);
}

/* Advance backend state by @dt ticks. The heap stores absolute expiry ticks,
 * so the caller's curr_tick += dt is all the advancing required here.
 */
ساکن اندرونی خالی z_timeout_q_advance(int32_t dt)
{
	ARG_غیر_مستعمل(dt);
}

/* Pop and return one timeout that is due at the current curr_tick, or NULL if
 * none is due right now. Tied timeouts share the same abs_ticks and are
 * drained by successive calls.
 */
ساکن اندرونی ساخت _timeout *z_timeout_q_نکالو_due(خالی)
{
	ساخت _timeout *t = z_timeout_q_جڑ();

	اگر ((t == تہی) || (t->abs_ticks > (int64_t)curr_tick)) {
		واپس تہی;
	}

	واپس CONTAINER_کا(min_ڈھیر_ref_نکالو(&timeout_ڈھیر_inst), ساخت _timeout, ڈھیر_handle);
}

#اگر_ختم /* ZEPHYR_KERNEL_TIMEOUT_MINHEAP_H_ */
