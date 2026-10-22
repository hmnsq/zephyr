/*
 * Copyright (c) 2026 BayLibre SAS
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#که_نه_تعریف_شوی ZEPHYR_KERNEL_TIMEOUT_لوښی_H
#تعریف_کړه ZEPHYR_KERNEL_TIMEOUT_لوښی_H

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

#تعریف_کړه لوښی_LISTS CONFIG_TIMEOUT_لوښی_LISTS
#تعریف_کړه لوښی_ماسک  (لوښی_LISTS - 1)

BUILD_ASSERT((لوښی_LISTS & لوښی_ماسک) == 0,
	     "CONFIG_TIMEOUT_BUCKET_LISTS must be a power of two");
BUILD_ASSERT(لوښی_LISTS <= 64, "bucket occupancy bitmap is at most 64 bits");

/* Occupancy bitmap, one bit per bucket. Stay 32-bit for the common (and
 * smallest, cheapest-to-scan on 32-bit targets) window and only widen to
 * 64-bit when more than 32 buckets are configured.
 */
#إذا لوښی_LISTS <= 32
تعريفنوع uint32_t لوښی_bitmap_t;
#تعریف_کړه لوښی_بټونه_ctz(x) u32_شمېر_trailing_zeros(x)
#وإلا
تعريفنوع uint64_t لوښی_bitmap_t;
#تعریف_کړه لوښی_بټونه_ctz(x) u64_شمېر_trailing_zeros(x)
#د_که_پای
#تعریف_کړه لوښی_بټ(n) ((لوښی_bitmap_t)1 << (n))

ساكن sys_dlist_t لوښی[لوښی_LISTS];
ساكن sys_dlist_t ډکېدنه;        /* sorted ascending by absolute expiry */
ساكن لوښی_bitmap_t لوښی_بټونه; /* bit p set => bucket[p] is non-empty */
ساكن uint64_t cursor_بنسټ;        /* window start, aligned to BUCKET_LISTS */

ساكن صحيح timeout_لوښی_init(فراغ)
{
	لكل (بینښه صحيح i = 0; i < لوښی_LISTS; i++) {
		sys_dlist_init(&لوښی[i]);
	}
	sys_dlist_init(&ډکېدنه);
	لوښی_بټونه = 0U;
	cursor_بنسټ = 0U;

	أرجع 0;
}

/* Runs before the timer driver (PRE_KERNEL_2) and before any timeout is added.
 * Priority 0 keeps it ahead of other PRE_KERNEL_1 initialisation.
 */
SYS_INIT(timeout_لوښی_init, PRE_KERNEL_1, 0);

ساكن بنية _timeout *ډکېدنه_سر(فراغ)
{
	sys_dnode_t *n = sys_dlist_peek_سر(&ډکېدنه);

	أرجع (n == عدم) ? عدم : CONTAINER_د(n, بنية _timeout, غوټه);
}

/* Ticks from curr_tick to the earliest pending expiry, or INT64_MAX if none. */
ساكن int64_t earliest_gap(فراغ)
{
	int64_t gap = INT64_MAX;
	بینښه صحيح cur_off = (بینښه صحيح)(curr_tick & لوښی_ماسک);

	/* Nearest non-empty bucket strictly ahead of curr_tick in the window.
	 * cursor_base is aligned, so bucket index == window offset.
	 */
	إذا (cur_off < (لوښی_LISTS - 1)) {
		لوښی_bitmap_t ahead = لوښی_بټونه & (~(لوښی_bitmap_t)0 << (cur_off + 1));

		إذا (ahead != 0U) {
			gap = (int64_t)لوښی_بټونه_ctz(ahead) - cur_off;
		}
	}

	إذا (gap == INT64_MAX) {
		بنية _timeout *t = ډکېدنه_سر();

		إذا (t != عدم) {
			gap = (int64_t)t->dticks - (int64_t)curr_tick;
		}
	}

	أرجع gap;
}

ساكن bool z_timeout_q_ورننباسه(بنية _timeout *منزل, k_ticks_t dticks)
{
	uint64_t expiry = curr_tick + (uint64_t)dticks;
	int64_t old_gap = earliest_gap();

	إذا (expiry < cursor_بنسټ + لوښی_LISTS) {
		بینښه صحيح idx = (بینښه صحيح)(expiry & لوښی_ماسک);

		منزل->dticks = idx;
		لوښی_بټونه |= لوښی_بټ(idx);
		sys_dlist_append(&لوښی[idx], &منزل->غوټه);
	} وإلا {
		/* Overflow: keep sorted ascending by absolute expiry. Insert
		 * after any equal-keyed entries so same-tick order stays FIFO.
		 */
		بنية _timeout *t;

		منزل->dticks = (k_ticks_t)expiry;
		SYS_DLIST_FOR_EACH_CONTAINER(&ډکېدنه, t, غوټه) {
			إذا ((uint64_t)t->dticks > expiry) {
				sys_dlist_ورننباسه(&t->غوټه, &منزل->غوټه);
				أرجع (int64_t)dticks < old_gap;
			}
		}
		sys_dlist_append(&ډکېدنه, &منزل->غوټه);
	}

	أرجع (int64_t)dticks < old_gap;
}

ساكن k_ticks_t z_timeout_q_remainder(ثابت بنية _timeout *منزل)
{
	إذا ((uint64_t)منزل->dticks < لوښی_LISTS) {
		/* Bucket entry: reconstruct its absolute expiry, the unique
		 * tick in the window whose low bits are the bucket index.
		 */
		uint64_t off = ((uint64_t)منزل->dticks - cursor_بنسټ) & لوښی_ماسک;

		أرجع (k_ticks_t)((cursor_بنسټ + off) - curr_tick);
	}

	أرجع (k_ticks_t)((uint64_t)منزل->dticks - curr_tick);
}

ساكن bool z_timeout_q_remove(بنية _timeout *منزل)
{
	bool was_لومړی = (z_timeout_q_remainder(منزل) <= earliest_gap());

	إذا ((uint64_t)منزل->dticks < لوښی_LISTS) {
		بینښه صحيح idx = (بینښه صحيح)منزل->dticks;

		sys_dlist_remove(&منزل->غوټه);
		إذا (sys_dlist_is_تشه(&لوښی[idx])) {
			لوښی_بټونه &= ~لوښی_بټ(idx);
		}
	} وإلا {
		/* Overflow stores absolute expiry (not a delta), so removal
		 * needs no successor fix-up.
		 */
		sys_dlist_remove(&منزل->غوټه);
	}

	أرجع was_لومړی;
}

/* Ticks from curr_tick to the earliest pending timeout, or K_TICKS_FOREVER if
 * none. The announce-range cap is applied centrally by next_timeout() in
 * timeout.c.
 */
ساكن k_ticks_t z_timeout_q_بل_expiry(فراغ)
{
	int64_t gap = earliest_gap();

	أرجع (gap == INT64_MAX) ? K_TICKS_FOREVER : (k_ticks_t)gap;
}

ساكن int32_t z_timeout_q_بل_gap(فراغ)
{
	int64_t gap = earliest_gap();

	أرجع (gap == INT64_MAX) ? INT32_MAX : (int32_t)MIN(gap, (int64_t)INT32_MAX);
}

ساكن فراغ z_timeout_q_advance(int32_t dt)
{
	uint64_t new_curr = curr_tick + (uint64_t)dt;

	إذا (new_curr < cursor_بنسټ + لوښی_LISTS) {
		/* Still inside the current window: nothing to migrate. */
		أرجع;
	}

	/* Crossed the window. The loop only advances past the boundary when no
	 * bucket lies ahead (next_gap then came from overflow), so every bucket
	 * is empty here. Re-base the window onto curr_tick's new position and
	 * migrate the overflow entries that have come within range.
	 */
	cursor_بنسټ = new_curr & ~(uint64_t)لوښی_ماسک;
	لوښی_بټونه = 0U;

	بنية _timeout *t;

	طالما ((t = ډکېدنه_سر()) != عدم &&
	       (uint64_t)t->dticks < cursor_بنسټ + لوښی_LISTS) {
		بینښه صحيح idx = (بینښه صحيح)((uint64_t)t->dticks & لوښی_ماسک);

		sys_dlist_remove(&t->غوټه);
		t->dticks = idx;
		لوښی_بټونه |= لوښی_بټ(idx);
		sys_dlist_append(&لوښی[idx], &t->غوټه);
	}
}

ساكن بنية _timeout *z_timeout_q_راوباسه_due(فراغ)
{
	بینښه صحيح idx = (بینښه صحيح)(curr_tick & لوښی_ماسک);
	sys_dnode_t *n = sys_dlist_peek_سر(&لوښی[idx]);
	بنية _timeout *t;

	إذا (n == عدم) {
		أرجع عدم;
	}

	/* All entries in bucket[curr_tick & MASK] expire exactly at curr_tick. */
	t = CONTAINER_د(n, بنية _timeout, غوټه);
	sys_dlist_remove(&t->غوټه);
	إذا (sys_dlist_is_تشه(&لوښی[idx])) {
		لوښی_بټونه &= ~لوښی_بټ(idx);
	}

	أرجع t;
}

#د_که_پای /* ZEPHYR_KERNEL_TIMEOUT_BUCKET_H_ */
