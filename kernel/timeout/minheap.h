/*
 * Copyright (c) 2026 Aerlync Labs Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#جيڪڏهن_وصف_نه_ٿيل ZEPHYR_KERNEL_TIMEOUT_MINHEAP_H_
#وصف_ڪريو ZEPHYR_KERNEL_TIMEOUT_MINHEAP_H_

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

#ضمن <zephyr/sys/min_heap_ref.h>

ساكن صحيح timeout_cmp(ثابت بنية min_ڍير_handle *a, ثابت بنية min_ڍير_handle *b)
{
	ثابت بنية _timeout *ta = CONTAINER_جو(a, بنية _timeout, ڍير_handle);
	ثابت بنية _timeout *tb = CONTAINER_جو(b, بنية _timeout, ڍير_handle);

	إذا (ta->abs_ticks < tb->abs_ticks) {
		أرجع -1;
	}

	أرجع (ta->abs_ticks > tb->abs_ticks) ? 1 : 0;
}

MIN_ڍير_REF_DEFINE_STATIC(timeout_ڍير_inst, CONFIG_TIMEOUT_ڍير_MAX_ENTRIES, timeout_cmp);

ساكن مضمن بنية _timeout *z_timeout_q_پاڙ(فراغ)
{
	ثابت بنية min_ڍير_handle *h = min_ڍير_ref_peek(&timeout_ڍير_inst);

	أرجع (h == عدم) ? عدم : CONTAINER_جو(h, بنية _timeout, ڍير_handle);
}

/* Insert @to so that it expires @dticks ticks after curr_tick. Returns true
 * if @to is now the earliest pending timeout (heap root).
 */
ساكن مضمن bool z_timeout_q_وجهو(بنية _timeout *منزل, k_ticks_t dticks)
{
	منزل->abs_ticks = (int64_t)curr_tick + dticks;

	إذا (unlikely(min_ڍير_ref_ڌڪو(&timeout_ڍير_inst, &منزل->ڍير_handle) != 0)) {
		/*
		 * Heap overflow is always a configuration error: there is no
		 * graceful way to drop a kernel timeout. Increase
		 * CONFIG_TIMEOUT_HEAP_MAX_ENTRIES.
		 */
		k_panic();
	}

	أرجع منزل->ڍير_handle.idx == 1U;
}

/* Remove an arbitrary timeout from the heap. Returns true if it was the
 * earliest pending timeout. Used only by the abort path, never from announce.
 */
ساكن مضمن bool z_timeout_q_remove(بنية _timeout *منزل)
{
	bool was_پهريون = (منزل->ڍير_handle.idx == 1U);

	(فراغ)min_ڍير_ref_remove(&timeout_ڍير_inst, &منزل->ڍير_handle);

	أرجع was_پهريون;
}

/* Ticks from curr_tick until @to expires. Caller holds timeout_lock and has
 * verified that @to is active.
 */
ساكن مضمن k_ticks_t z_timeout_q_remainder(ثابت بنية _timeout *منزل)
{
	أرجع (k_ticks_t)(منزل->abs_ticks - (int64_t)curr_tick);
}

/* Ticks from curr_tick to the earliest pending timeout, or K_TICKS_FOREVER if
 * the heap is empty. The announce-range cap is applied centrally by
 * next_timeout() in timeout.c.
 */
ساكن مضمن k_ticks_t z_timeout_q_ايندڙ_expiry(فراغ)
{
	بنية _timeout *t = z_timeout_q_پاڙ();

	أرجع (t == عدم) ? K_TICKS_FOREVER
			   : (k_ticks_t)(t->abs_ticks - (int64_t)curr_tick);
}

/* --- announce-loop helpers (see sys_clock_announce_locked() in timeout.c) --- */

/* Ticks from curr_tick until the earliest expiry. A value larger than any
 * announce window when the heap is empty.
 */
ساكن مضمن int32_t z_timeout_q_ايندڙ_gap(فراغ)
{
	بنية _timeout *t = z_timeout_q_پاڙ();

	إذا (t == عدم) {
		أرجع INT32_MAX;
	}

	أرجع (int32_t)MIN(t->abs_ticks - (int64_t)curr_tick, (int64_t)INT32_MAX);
}

/* Advance backend state by @dt ticks. The heap stores absolute expiry ticks,
 * so the caller's curr_tick += dt is all the advancing required here.
 */
ساكن مضمن فراغ z_timeout_q_advance(int32_t dt)
{
	ARG_اڻ_استعمال_ٿيل(dt);
}

/* Pop and return one timeout that is due at the current curr_tick, or NULL if
 * none is due right now. Tied timeouts share the same abs_ticks and are
 * drained by successive calls.
 */
ساكن مضمن بنية _timeout *z_timeout_q_ڪڍو_due(فراغ)
{
	بنية _timeout *t = z_timeout_q_پاڙ();

	إذا ((t == عدم) || (t->abs_ticks > (int64_t)curr_tick)) {
		أرجع عدم;
	}

	أرجع CONTAINER_جو(min_ڍير_ref_ڪڍو(&timeout_ڍير_inst), بنية _timeout, ڍير_handle);
}

#جيڪڏهن_پورو /* ZEPHYR_KERNEL_TIMEOUT_MINHEAP_H_ */
