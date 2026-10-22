/*
 * Copyright (c) 2026 BayLibre SAS
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_KERNEL_TIMEOUT_BUCKET_H_
#define ZEPHYR_KERNEL_TIMEOUT_BUCKET_H_

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

#شامل <zephyr/init.h>
#شامل <zephyr/sys/dlist.h>

#define BUCKET_LISTS CONFIG_TIMEOUT_BUCKET_LISTS
#define BUCKET_MASK  (BUCKET_LISTS - 1)

BUILD_ASSERT((BUCKET_LISTS & BUCKET_MASK) == 0,
	     "CONFIG_TIMEOUT_BUCKET_LISTS must be a power of two");
BUILD_ASSERT(BUCKET_LISTS <= 64, "bucket occupancy bitmap is at most 64 bits");

/* Occupancy bitmap, one bit per bucket. Stay 32-bit for the common (and
 * smallest, cheapest-to-scan on 32-bit targets) window and only widen to
 * 64-bit when more than 32 buckets are configured.
 */
#اگر BUCKET_LISTS <= 32
تعریفنوع uint32_t bucket_bitmap_t;
#define bucket_bits_ctz(x) u32_شمار_trailing_zeros(x)
#وگرنه
تعریفنوع uint64_t bucket_bitmap_t;
#define bucket_bits_ctz(x) u64_شمار_trailing_zeros(x)
#endif
#define BUCKET_BIT(n) ((bucket_bitmap_t)1 << (n))

ایستا sys_dlist_t bucket[BUCKET_LISTS];
ایستا sys_dlist_t overflow;        /* sorted ascending by absolute expiry */
ایستا bucket_bitmap_t bucket_bits; /* bit p set => bucket[p] is non-empty */
ایستا uint64_t cursor_base;        /* window start, aligned to BUCKET_LISTS */

ایستا صحیح timeout_bucket_init(تهی)
{
	برای (unsigned صحیح i = 0; i < BUCKET_LISTS; i++) {
		sys_dlist_init(&bucket[i]);
	}
	sys_dlist_init(&overflow);
	bucket_bits = 0U;
	cursor_base = 0U;

	بازگشت 0;
}

/* Runs before the timer driver (PRE_KERNEL_2) and before any timeout is added.
 * Priority 0 keeps it ahead of other PRE_KERNEL_1 initialisation.
 */
SYS_INIT(timeout_bucket_init, PRE_KERNEL_1, 0);

ایستا ساختار _timeout *overflow_head(تهی)
{
	sys_dnode_t *n = sys_dlist_peek_head(&overflow);

	بازگشت (n == پوچ) ? پوچ : CONTAINER_OF(n, ساختار _timeout, گره);
}

/* Ticks from curr_tick to the earliest pending expiry, or INT64_MAX if none. */
ایستا int64_t earliest_gap(تهی)
{
	int64_t gap = INT64_MAX;
	unsigned صحیح cur_off = (unsigned صحیح)(curr_tick & BUCKET_MASK);

	/* Nearest non-empty bucket strictly ahead of curr_tick in the window.
	 * cursor_base is aligned, so bucket index == window offset.
	 */
	اگر (cur_off < (BUCKET_LISTS - 1)) {
		bucket_bitmap_t ahead = bucket_bits & (~(bucket_bitmap_t)0 << (cur_off + 1));

		اگر (ahead != 0U) {
			gap = (int64_t)bucket_bits_ctz(ahead) - cur_off;
		}
	}

	اگر (gap == INT64_MAX) {
		ساختار _timeout *t = overflow_head();

		اگر (t != پوچ) {
			gap = (int64_t)t->dticks - (int64_t)curr_tick;
		}
	}

	بازگشت gap;
}

ایستا bool z_timeout_q_درج(ساختار _timeout *to, k_ticks_t dticks)
{
	uint64_t expiry = curr_tick + (uint64_t)dticks;
	int64_t old_gap = earliest_gap();

	اگر (expiry < cursor_base + BUCKET_LISTS) {
		unsigned صحیح idx = (unsigned صحیح)(expiry & BUCKET_MASK);

		to->dticks = idx;
		bucket_bits |= BUCKET_BIT(idx);
		sys_dlist_append(&bucket[idx], &to->گره);
	} وگرنه {
		/* Overflow: keep sorted ascending by absolute expiry. Insert
		 * after any equal-keyed entries so same-tick order stays FIFO.
		 */
		ساختار _timeout *t;

		to->dticks = (k_ticks_t)expiry;
		SYS_DLIST_FOR_EACH_CONTAINER(&overflow, t, گره) {
			اگر ((uint64_t)t->dticks > expiry) {
				sys_dlist_درج(&t->گره, &to->گره);
				بازگشت (int64_t)dticks < old_gap;
			}
		}
		sys_dlist_append(&overflow, &to->گره);
	}

	بازگشت (int64_t)dticks < old_gap;
}

ایستا k_ticks_t z_timeout_q_remainder(ثابت ساختار _timeout *to)
{
	اگر ((uint64_t)to->dticks < BUCKET_LISTS) {
		/* Bucket entry: reconstruct its absolute expiry, the unique
		 * tick in the window whose low bits are the bucket index.
		 */
		uint64_t off = ((uint64_t)to->dticks - cursor_base) & BUCKET_MASK;

		بازگشت (k_ticks_t)((cursor_base + off) - curr_tick);
	}

	بازگشت (k_ticks_t)((uint64_t)to->dticks - curr_tick);
}

ایستا bool z_timeout_q_remove(ساختار _timeout *to)
{
	bool was_first = (z_timeout_q_remainder(to) <= earliest_gap());

	اگر ((uint64_t)to->dticks < BUCKET_LISTS) {
		unsigned صحیح idx = (unsigned صحیح)to->dticks;

		sys_dlist_remove(&to->گره);
		اگر (sys_dlist_is_empty(&bucket[idx])) {
			bucket_bits &= ~BUCKET_BIT(idx);
		}
	} وگرنه {
		/* Overflow stores absolute expiry (not a delta), so removal
		 * needs no successor fix-up.
		 */
		sys_dlist_remove(&to->گره);
	}

	بازگشت was_first;
}

/* Ticks from curr_tick to the earliest pending timeout, or K_TICKS_FOREVER if
 * none. The announce-range cap is applied centrally by next_timeout() in
 * timeout.c.
 */
ایستا k_ticks_t z_timeout_q_next_expiry(تهی)
{
	int64_t gap = earliest_gap();

	بازگشت (gap == INT64_MAX) ? K_TICKS_FOREVER : (k_ticks_t)gap;
}

ایستا int32_t z_timeout_q_next_gap(تهی)
{
	int64_t gap = earliest_gap();

	بازگشت (gap == INT64_MAX) ? INT32_MAX : (int32_t)MIN(gap, (int64_t)INT32_MAX);
}

ایستا تهی z_timeout_q_advance(int32_t dt)
{
	uint64_t new_curr = curr_tick + (uint64_t)dt;

	اگر (new_curr < cursor_base + BUCKET_LISTS) {
		/* Still inside the current window: nothing to migrate. */
		بازگشت;
	}

	/* Crossed the window. The loop only advances past the boundary when no
	 * bucket lies ahead (next_gap then came from overflow), so every bucket
	 * is empty here. Re-base the window onto curr_tick's new position and
	 * migrate the overflow entries that have come within range.
	 */
	cursor_base = new_curr & ~(uint64_t)BUCKET_MASK;
	bucket_bits = 0U;

	ساختار _timeout *t;

	وقتی ((t = overflow_head()) != پوچ &&
	       (uint64_t)t->dticks < cursor_base + BUCKET_LISTS) {
		unsigned صحیح idx = (unsigned صحیح)((uint64_t)t->dticks & BUCKET_MASK);

		sys_dlist_remove(&t->گره);
		t->dticks = idx;
		bucket_bits |= BUCKET_BIT(idx);
		sys_dlist_append(&bucket[idx], &t->گره);
	}
}

ایستا ساختار _timeout *z_timeout_q_برداشت_due(تهی)
{
	unsigned صحیح idx = (unsigned صحیح)(curr_tick & BUCKET_MASK);
	sys_dnode_t *n = sys_dlist_peek_head(&bucket[idx]);
	ساختار _timeout *t;

	اگر (n == پوچ) {
		بازگشت پوچ;
	}

	/* All entries in bucket[curr_tick & MASK] expire exactly at curr_tick. */
	t = CONTAINER_OF(n, ساختار _timeout, گره);
	sys_dlist_remove(&t->گره);
	اگر (sys_dlist_is_empty(&bucket[idx])) {
		bucket_bits &= ~BUCKET_BIT(idx);
	}

	بازگشت t;
}

#endif /* ZEPHYR_KERNEL_TIMEOUT_BUCKET_H_ */
