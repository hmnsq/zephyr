/*
 * Copyright (c) 2026 Aerlync Labs Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#اگر_تعریف_نشده ZEPHYR_KERNEL_TIMEOUT_MINHEAP_H_
#تعریف_کن ZEPHYR_KERNEL_TIMEOUT_MINHEAP_H_

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

ساكن صحيح timeout_cmp(ثابت بنية min_انباشته_handle *a, ثابت بنية min_انباشته_handle *b)
{
	ثابت بنية _timeout *ta = CONTAINER_از(a, بنية _timeout, انباشته_handle);
	ثابت بنية _timeout *tb = CONTAINER_از(b, بنية _timeout, انباشته_handle);

	إذا (ta->abs_ticks < tb->abs_ticks) {
		أرجع -1;
	}

	أرجع (ta->abs_ticks > tb->abs_ticks) ? 1 : 0;
}

MIN_انباشته_REF_DEFINE_STATIC(timeout_انباشته_inst, CONFIG_TIMEOUT_انباشته_MAX_ENTRIES, timeout_cmp);

ساكن مضمن بنية _timeout *z_timeout_q_ریشه(فراغ)
{
	ثابت بنية min_انباشته_handle *h = min_انباشته_ref_peek(&timeout_انباشته_inst);

	أرجع (h == عدم) ? عدم : CONTAINER_از(h, بنية _timeout, انباشته_handle);
}

/* Insert @to so that it expires @dticks ticks after curr_tick. Returns true
 * if @to is now the earliest pending timeout (heap root).
 */
ساكن مضمن bool z_timeout_q_درج(بنية _timeout *مقصد, k_ticks_t dticks)
{
	مقصد->abs_ticks = (int64_t)curr_tick + dticks;

	إذا (unlikely(min_انباشته_ref_افزودن(&timeout_انباشته_inst, &مقصد->انباشته_handle) != 0)) {
		/*
		 * Heap overflow is always a configuration error: there is no
		 * graceful way to drop a kernel timeout. Increase
		 * CONFIG_TIMEOUT_HEAP_MAX_ENTRIES.
		 */
		k_panic();
	}

	أرجع مقصد->انباشته_handle.idx == 1U;
}

/* Remove an arbitrary timeout from the heap. Returns true if it was the
 * earliest pending timeout. Used only by the abort path, never from announce.
 */
ساكن مضمن bool z_timeout_q_remove(بنية _timeout *مقصد)
{
	bool was_اولی = (مقصد->انباشته_handle.idx == 1U);

	(فراغ)min_انباشته_ref_remove(&timeout_انباشته_inst, &مقصد->انباشته_handle);

	أرجع was_اولی;
}

/* Ticks from curr_tick until @to expires. Caller holds timeout_lock and has
 * verified that @to is active.
 */
ساكن مضمن k_ticks_t z_timeout_q_remainder(ثابت بنية _timeout *مقصد)
{
	أرجع (k_ticks_t)(مقصد->abs_ticks - (int64_t)curr_tick);
}

/* Ticks from curr_tick to the earliest pending timeout, or K_TICKS_FOREVER if
 * the heap is empty. The announce-range cap is applied centrally by
 * next_timeout() in timeout.c.
 */
ساكن مضمن k_ticks_t z_timeout_q_بعدی_expiry(فراغ)
{
	بنية _timeout *t = z_timeout_q_ریشه();

	أرجع (t == عدم) ? K_TICKS_FOREVER
			   : (k_ticks_t)(t->abs_ticks - (int64_t)curr_tick);
}

/* --- announce-loop helpers (see sys_clock_announce_locked() in timeout.c) --- */

/* Ticks from curr_tick until the earliest expiry. A value larger than any
 * announce window when the heap is empty.
 */
ساكن مضمن int32_t z_timeout_q_بعدی_gap(فراغ)
{
	بنية _timeout *t = z_timeout_q_ریشه();

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
	ARG_استفادهنشده(dt);
}

/* Pop and return one timeout that is due at the current curr_tick, or NULL if
 * none is due right now. Tied timeouts share the same abs_ticks and are
 * drained by successive calls.
 */
ساكن مضمن بنية _timeout *z_timeout_q_برداشت_due(فراغ)
{
	بنية _timeout *t = z_timeout_q_ریشه();

	إذا ((t == عدم) || (t->abs_ticks > (int64_t)curr_tick)) {
		أرجع عدم;
	}

	أرجع CONTAINER_از(min_انباشته_ref_برداشت(&timeout_انباشته_inst), بنية _timeout, انباشته_handle);
}

#پایان_اگر /* ZEPHYR_KERNEL_TIMEOUT_MINHEAP_H_ */
