/*
 * Copyright (c) 2026 BayLibre SAS
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#اگر_تعریف_نشده ZEPHYR_KERNEL_TIMEOUT_سطل_H
#تعریف_کن ZEPHYR_KERNEL_TIMEOUT_سطل_H

/**
 * @file
 * @brief Single-level bucketed delta-list timeout backend (implementation).
 *
 * Timeouts expiring within the next CONFIG_TIMEOUT_BUCKET_LISTS ticks go into a
 * per-tick "bucket" list (O(1) insert and remove); everything beyond goes into
 * a single "overflow" list sorted by absolute expiry (O(n) insert, O(1)
 * remove). A bitmap of non-empty buckets makes "ticks until the next expiry"
 * O(1). As curr_tick crosses the end of the current bucket window, the overflow
 * entries that have come within range are migrated into buckets.
 *
 * Migration is on demand against the actual next event, so an idle system with
 * a distant timeout sleeps straight to it rather than waking periodically.
 * Overflow stores absolute expiry, so 64-bit timeouts are required.
 *
 * The per-node representation is node + dticks: a bucket entry stores its
 * bucket index (< BUCKET_LISTS) in dticks; an overflow entry stores its
 * absolute expiry (>= BUCKET_LISTS) in dticks. The two ranges never overlap,
 * so no flag is needed to tell them apart.
 *
 * Window invariant: buckets cover the half-open tick range
 * [cursor_base, cursor_base + BUCKET_LISTS). cursor_base is kept aligned to
 * BUCKET_LISTS, so a bucket's index equals both its tick offset within the
 * window and (absolute expiry & (BUCKET_LISTS - 1)). curr_tick is always
 * within the window. Everything expiring at or beyond cursor_base +
 * BUCKET_LISTS lives in the overflow list, sorted by absolute expiry.
 *
 * Included only by kernel/timeout.c, after its shared state (curr_tick), so
 * the bucket state and the z_timeout_q_*() operations below are private to
 * that translation unit. The per-node helpers live in
 * kernel/include/timeout_q.h.
 */

#ضمن <zephyr/init.h>
#ضمن <zephyr/sys/dlist.h>

#تعریف_کن سطل_LISTS CONFIG_TIMEOUT_سطل_LISTS
#تعریف_کن سطل_ماسک  (سطل_LISTS - 1)

BUILD_ASSERT((سطل_LISTS & سطل_ماسک) == 0,
	     "CONFIG_TIMEOUT_BUCKET_LISTS must be a power of two");
BUILD_ASSERT(سطل_LISTS <= 64, "bucket occupancy bitmap is at most 64 bits");

/* Occupancy bitmap, one bit per bucket. Stay 32-bit for the common (and
 * smallest, cheapest-to-scan on 32-bit targets) window and only widen to
 * 64-bit when more than 32 buckets are configured.
 */
#إذا سطل_LISTS <= 32
تعريفنوع uint32_t سطل_bitmap_t;
#تعریف_کن سطل_بیتها_ctz(x) u32_شمار_trailing_zeros(x)
#وإلا
تعريفنوع uint64_t سطل_bitmap_t;
#تعریف_کن سطل_بیتها_ctz(x) u64_شمار_trailing_zeros(x)
#پایان_اگر
#تعریف_کن سطل_بیت(n) ((سطل_bitmap_t)1 << (n))

ساكن sys_dlist_t سطل[سطل_LISTS];
ساكن sys_dlist_t سرریز;        /* sorted ascending by absolute expiry */
ساكن سطل_bitmap_t سطل_بیتها; /* bit p set => bucket[p] is non-empty */
ساكن uint64_t cursor_پایه;        /* window start, aligned to BUCKET_LISTS */

ساكن صحيح timeout_سطل_init(فراغ)
{
	لكل (بیعلامت صحيح i = 0; i < سطل_LISTS; i++) {
		sys_dlist_init(&سطل[i]);
	}
	sys_dlist_init(&سرریز);
	سطل_بیتها = 0U;
	cursor_پایه = 0U;

	أرجع 0;
}

/* Runs before the timer driver (PRE_KERNEL_2) and before any timeout is added.
 * Priority 0 keeps it ahead of other PRE_KERNEL_1 initialisation.
 */
SYS_INIT(timeout_سطل_init, PRE_KERNEL_1, 0);

ساكن بنية _timeout *سرریز_سر(فراغ)
{
	sys_dnode_t *n = sys_dlist_peek_سر(&سرریز);

	أرجع (n == عدم) ? عدم : CONTAINER_از(n, بنية _timeout, گره);
}

/* Ticks from curr_tick to the earliest pending expiry, or INT64_MAX if none. */
ساكن int64_t earliest_gap(فراغ)
{
	int64_t gap = INT64_MAX;
	بیعلامت صحيح cur_off = (بیعلامت صحيح)(curr_tick & سطل_ماسک);

	/* Nearest non-empty bucket strictly ahead of curr_tick in the window.
	 * cursor_base is aligned, so bucket index == window offset.
	 */
	إذا (cur_off < (سطل_LISTS - 1)) {
		سطل_bitmap_t ahead = سطل_بیتها & (~(سطل_bitmap_t)0 << (cur_off + 1));

		إذا (ahead != 0U) {
			gap = (int64_t)سطل_بیتها_ctz(ahead) - cur_off;
		}
	}

	إذا (gap == INT64_MAX) {
		بنية _timeout *t = سرریز_سر();

		إذا (t != عدم) {
			gap = (int64_t)t->dticks - (int64_t)curr_tick;
		}
	}

	أرجع gap;
}

ساكن bool z_timeout_q_درج(بنية _timeout *مقصد, k_ticks_t dticks)
{
	uint64_t expiry = curr_tick + (uint64_t)dticks;
	int64_t old_gap = earliest_gap();

	إذا (expiry < cursor_پایه + سطل_LISTS) {
		بیعلامت صحيح idx = (بیعلامت صحيح)(expiry & سطل_ماسک);

		مقصد->dticks = idx;
		سطل_بیتها |= سطل_بیت(idx);
		sys_dlist_append(&سطل[idx], &مقصد->گره);
	} وإلا {
		/* Overflow: keep sorted ascending by absolute expiry. Insert
		 * after any equal-keyed entries so same-tick order stays FIFO.
		 */
		بنية _timeout *t;

		مقصد->dticks = (k_ticks_t)expiry;
		SYS_DLIST_FOR_EACH_CONTAINER(&سرریز, t, گره) {
			إذا ((uint64_t)t->dticks > expiry) {
				sys_dlist_درج(&t->گره, &مقصد->گره);
				أرجع (int64_t)dticks < old_gap;
			}
		}
		sys_dlist_append(&سرریز, &مقصد->گره);
	}

	أرجع (int64_t)dticks < old_gap;
}

ساكن k_ticks_t z_timeout_q_remainder(ثابت بنية _timeout *مقصد)
{
	إذا ((uint64_t)مقصد->dticks < سطل_LISTS) {
		/* Bucket entry: reconstruct its absolute expiry, the unique
		 * tick in the window whose low bits are the bucket index.
		 */
		uint64_t off = ((uint64_t)مقصد->dticks - cursor_پایه) & سطل_ماسک;

		أرجع (k_ticks_t)((cursor_پایه + off) - curr_tick);
	}

	أرجع (k_ticks_t)((uint64_t)مقصد->dticks - curr_tick);
}

ساكن bool z_timeout_q_remove(بنية _timeout *مقصد)
{
	bool was_اولی = (z_timeout_q_remainder(مقصد) <= earliest_gap());

	إذا ((uint64_t)مقصد->dticks < سطل_LISTS) {
		بیعلامت صحيح idx = (بیعلامت صحيح)مقصد->dticks;

		sys_dlist_remove(&مقصد->گره);
		إذا (sys_dlist_is_خالی(&سطل[idx])) {
			سطل_بیتها &= ~سطل_بیت(idx);
		}
	} وإلا {
		/* Overflow stores absolute expiry (not a delta), so removal
		 * needs no successor fix-up.
		 */
		sys_dlist_remove(&مقصد->گره);
	}

	أرجع was_اولی;
}

/* Ticks from curr_tick to the earliest pending timeout, or K_TICKS_FOREVER if
 * none. The announce-range cap is applied centrally by next_timeout() in
 * timeout.c.
 */
ساكن k_ticks_t z_timeout_q_بعدی_expiry(فراغ)
{
	int64_t gap = earliest_gap();

	أرجع (gap == INT64_MAX) ? K_TICKS_FOREVER : (k_ticks_t)gap;
}

ساكن int32_t z_timeout_q_بعدی_gap(فراغ)
{
	int64_t gap = earliest_gap();

	أرجع (gap == INT64_MAX) ? INT32_MAX : (int32_t)MIN(gap, (int64_t)INT32_MAX);
}

ساكن فراغ z_timeout_q_advance(int32_t dt)
{
	uint64_t new_curr = curr_tick + (uint64_t)dt;

	إذا (new_curr < cursor_پایه + سطل_LISTS) {
		/* Still inside the current window: nothing to migrate. */
		أرجع;
	}

	/* Crossed the window. The loop only advances past the boundary when no
	 * bucket lies ahead (next_gap then came from overflow), so every bucket
	 * is empty here. Re-base the window onto curr_tick's new position and
	 * migrate the overflow entries that have come within range.
	 */
	cursor_پایه = new_curr & ~(uint64_t)سطل_ماسک;
	سطل_بیتها = 0U;

	بنية _timeout *t;

	طالما ((t = سرریز_سر()) != عدم &&
	       (uint64_t)t->dticks < cursor_پایه + سطل_LISTS) {
		بیعلامت صحيح idx = (بیعلامت صحيح)((uint64_t)t->dticks & سطل_ماسک);

		sys_dlist_remove(&t->گره);
		t->dticks = idx;
		سطل_بیتها |= سطل_بیت(idx);
		sys_dlist_append(&سطل[idx], &t->گره);
	}
}

ساكن بنية _timeout *z_timeout_q_برداشت_due(فراغ)
{
	بیعلامت صحيح idx = (بیعلامت صحيح)(curr_tick & سطل_ماسک);
	sys_dnode_t *n = sys_dlist_peek_سر(&سطل[idx]);
	بنية _timeout *t;

	إذا (n == عدم) {
		أرجع عدم;
	}

	/* All entries in bucket[curr_tick & MASK] expire exactly at curr_tick. */
	t = CONTAINER_از(n, بنية _timeout, گره);
	sys_dlist_remove(&t->گره);
	إذا (sys_dlist_is_خالی(&سطل[idx])) {
		سطل_بیتها &= ~سطل_بیت(idx);
	}

	أرجع t;
}

#پایان_اگر /* ZEPHYR_KERNEL_TIMEOUT_BUCKET_H_ */
