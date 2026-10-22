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

#ಅನ್ತರ್ಭಾವ <zephyr/init.h>
#ಅನ್ತರ್ಭಾವ <zephyr/sys/dlist.h>

#define BUCKET_LISTS CONFIG_TIMEOUT_BUCKET_LISTS
#define BUCKET_MASK  (BUCKET_LISTS - 1)

BUILD_ASSERT((BUCKET_LISTS & BUCKET_MASK) == 0,
	     "CONFIG_TIMEOUT_BUCKET_LISTS must be a power of two");
BUILD_ASSERT(BUCKET_LISTS <= 64, "bucket occupancy bitmap is at most 64 bits");

/* Occupancy bitmap, one bit per bucket. Stay 32-bit for the common (and
 * smallest, cheapest-to-scan on 32-bit targets) window and only widen to
 * 64-bit when more than 32 buckets are configured.
 */
#ಯದಿ BUCKET_LISTS <= 32
ಪ್ರಕಾರನಿರ್ದೇಶ uint32_t bucket_bitmap_t;
#define bucket_bits_ctz(x) u32_ಎಣಿಕೆ_trailing_zeros(x)
#ಅನ್ಯಥಾ
ಪ್ರಕಾರನಿರ್ದೇಶ uint64_t bucket_bitmap_t;
#define bucket_bits_ctz(x) u64_ಎಣಿಕೆ_trailing_zeros(x)
#endif
#define BUCKET_BIT(n) ((bucket_bitmap_t)1 << (n))

ಸ್ಥಿರ sys_dlist_t bucket[BUCKET_LISTS];
ಸ್ಥಿರ sys_dlist_t overflow;        /* sorted ascending by absolute expiry */
ಸ್ಥಿರ bucket_bitmap_t bucket_bits; /* bit p set => bucket[p] is non-empty */
ಸ್ಥಿರ uint64_t cursor_base;        /* window start, aligned to BUCKET_LISTS */

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ timeout_bucket_init(ಶೂನ್ಯ)
{
	ಪ್ರತಿ (unsigned ಪೂರ್ಣಾಙ್ಕ i = 0; i < BUCKET_LISTS; i++) {
		sys_dlist_init(&bucket[i]);
	}
	sys_dlist_init(&overflow);
	bucket_bits = 0U;
	cursor_base = 0U;

	ಪ್ರತ್ಯಾಗಮ 0;
}

/* Runs before the timer driver (PRE_KERNEL_2) and before any timeout is added.
 * Priority 0 keeps it ahead of other PRE_KERNEL_1 initialisation.
 */
SYS_INIT(timeout_bucket_init, PRE_KERNEL_1, 0);

ಸ್ಥಿರ ಸಂರಚನಾ _timeout *overflow_head(ಶೂನ್ಯ)
{
	sys_dnode_t *n = sys_dlist_peek_head(&overflow);

	ಪ್ರತ್ಯಾಗಮ (n == ರಿಕ್ತ) ? ರಿಕ್ತ : CONTAINER_OF(n, ಸಂರಚನಾ _timeout, ಗಂಟು);
}

/* Ticks from curr_tick to the earliest pending expiry, or INT64_MAX if none. */
ಸ್ಥಿರ int64_t earliest_gap(ಶೂನ್ಯ)
{
	int64_t gap = INT64_MAX;
	unsigned ಪೂರ್ಣಾಙ್ಕ cur_off = (unsigned ಪೂರ್ಣಾಙ್ಕ)(curr_tick & BUCKET_MASK);

	/* Nearest non-empty bucket strictly ahead of curr_tick in the window.
	 * cursor_base is aligned, so bucket index == window offset.
	 */
	ಯದಿ (cur_off < (BUCKET_LISTS - 1)) {
		bucket_bitmap_t ahead = bucket_bits & (~(bucket_bitmap_t)0 << (cur_off + 1));

		ಯದಿ (ahead != 0U) {
			gap = (int64_t)bucket_bits_ctz(ahead) - cur_off;
		}
	}

	ಯದಿ (gap == INT64_MAX) {
		ಸಂರಚನಾ _timeout *t = overflow_head();

		ಯದಿ (t != ರಿಕ್ತ) {
			gap = (int64_t)t->dticks - (int64_t)curr_tick;
		}
	}

	ಪ್ರತ್ಯಾಗಮ gap;
}

ಸ್ಥಿರ bool z_timeout_q_ಸೇರಿಸು(ಸಂರಚನಾ _timeout *to, k_ticks_t dticks)
{
	uint64_t expiry = curr_tick + (uint64_t)dticks;
	int64_t old_gap = earliest_gap();

	ಯದಿ (expiry < cursor_base + BUCKET_LISTS) {
		unsigned ಪೂರ್ಣಾಙ್ಕ idx = (unsigned ಪೂರ್ಣಾಙ್ಕ)(expiry & BUCKET_MASK);

		to->dticks = idx;
		bucket_bits |= BUCKET_BIT(idx);
		sys_dlist_append(&bucket[idx], &to->ಗಂಟು);
	} ಅನ್ಯಥಾ {
		/* Overflow: keep sorted ascending by absolute expiry. Insert
		 * after any equal-keyed entries so same-tick order stays FIFO.
		 */
		ಸಂರಚನಾ _timeout *t;

		to->dticks = (k_ticks_t)expiry;
		SYS_DLIST_FOR_EACH_CONTAINER(&overflow, t, ಗಂಟು) {
			ಯದಿ ((uint64_t)t->dticks > expiry) {
				sys_dlist_ಸೇರಿಸು(&t->ಗಂಟು, &to->ಗಂಟು);
				ಪ್ರತ್ಯಾಗಮ (int64_t)dticks < old_gap;
			}
		}
		sys_dlist_append(&overflow, &to->ಗಂಟು);
	}

	ಪ್ರತ್ಯಾಗಮ (int64_t)dticks < old_gap;
}

ಸ್ಥಿರ k_ticks_t z_timeout_q_remainder(ಧ್ರುವ ಸಂರಚನಾ _timeout *to)
{
	ಯದಿ ((uint64_t)to->dticks < BUCKET_LISTS) {
		/* Bucket entry: reconstruct its absolute expiry, the unique
		 * tick in the window whose low bits are the bucket index.
		 */
		uint64_t off = ((uint64_t)to->dticks - cursor_base) & BUCKET_MASK;

		ಪ್ರತ್ಯಾಗಮ (k_ticks_t)((cursor_base + off) - curr_tick);
	}

	ಪ್ರತ್ಯಾಗಮ (k_ticks_t)((uint64_t)to->dticks - curr_tick);
}

ಸ್ಥಿರ bool z_timeout_q_remove(ಸಂರಚನಾ _timeout *to)
{
	bool was_first = (z_timeout_q_remainder(to) <= earliest_gap());

	ಯದಿ ((uint64_t)to->dticks < BUCKET_LISTS) {
		unsigned ಪೂರ್ಣಾಙ್ಕ idx = (unsigned ಪೂರ್ಣಾಙ್ಕ)to->dticks;

		sys_dlist_remove(&to->ಗಂಟು);
		ಯದಿ (sys_dlist_is_empty(&bucket[idx])) {
			bucket_bits &= ~BUCKET_BIT(idx);
		}
	} ಅನ್ಯಥಾ {
		/* Overflow stores absolute expiry (not a delta), so removal
		 * needs no successor fix-up.
		 */
		sys_dlist_remove(&to->ಗಂಟು);
	}

	ಪ್ರತ್ಯಾಗಮ was_first;
}

/* Ticks from curr_tick to the earliest pending timeout, or K_TICKS_FOREVER if
 * none. The announce-range cap is applied centrally by next_timeout() in
 * timeout.c.
 */
ಸ್ಥಿರ k_ticks_t z_timeout_q_next_expiry(ಶೂನ್ಯ)
{
	int64_t gap = earliest_gap();

	ಪ್ರತ್ಯಾಗಮ (gap == INT64_MAX) ? K_TICKS_FOREVER : (k_ticks_t)gap;
}

ಸ್ಥಿರ int32_t z_timeout_q_next_gap(ಶೂನ್ಯ)
{
	int64_t gap = earliest_gap();

	ಪ್ರತ್ಯಾಗಮ (gap == INT64_MAX) ? INT32_MAX : (int32_t)MIN(gap, (int64_t)INT32_MAX);
}

ಸ್ಥಿರ ಶೂನ್ಯ z_timeout_q_advance(int32_t dt)
{
	uint64_t new_curr = curr_tick + (uint64_t)dt;

	ಯದಿ (new_curr < cursor_base + BUCKET_LISTS) {
		/* Still inside the current window: nothing to migrate. */
		ಪ್ರತ್ಯಾಗಮ;
	}

	/* Crossed the window. The loop only advances past the boundary when no
	 * bucket lies ahead (next_gap then came from overflow), so every bucket
	 * is empty here. Re-base the window onto curr_tick's new position and
	 * migrate the overflow entries that have come within range.
	 */
	cursor_base = new_curr & ~(uint64_t)BUCKET_MASK;
	bucket_bits = 0U;

	ಸಂರಚನಾ _timeout *t;

	ಯಾವತ್ ((t = overflow_head()) != ರಿಕ್ತ &&
	       (uint64_t)t->dticks < cursor_base + BUCKET_LISTS) {
		unsigned ಪೂರ್ಣಾಙ್ಕ idx = (unsigned ಪೂರ್ಣಾಙ್ಕ)((uint64_t)t->dticks & BUCKET_MASK);

		sys_dlist_remove(&t->ಗಂಟು);
		t->dticks = idx;
		bucket_bits |= BUCKET_BIT(idx);
		sys_dlist_append(&bucket[idx], &t->ಗಂಟು);
	}
}

ಸ್ಥಿರ ಸಂರಚನಾ _timeout *z_timeout_q_ತೆಗೆ_due(ಶೂನ್ಯ)
{
	unsigned ಪೂರ್ಣಾಙ್ಕ idx = (unsigned ಪೂರ್ಣಾಙ್ಕ)(curr_tick & BUCKET_MASK);
	sys_dnode_t *n = sys_dlist_peek_head(&bucket[idx]);
	ಸಂರಚನಾ _timeout *t;

	ಯದಿ (n == ರಿಕ್ತ) {
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}

	/* All entries in bucket[curr_tick & MASK] expire exactly at curr_tick. */
	t = CONTAINER_OF(n, ಸಂರಚನಾ _timeout, ಗಂಟು);
	sys_dlist_remove(&t->ಗಂಟು);
	ಯದಿ (sys_dlist_is_empty(&bucket[idx])) {
		bucket_bits &= ~BUCKET_BIT(idx);
	}

	ಪ್ರತ್ಯಾಗಮ t;
}

#endif /* ZEPHYR_KERNEL_TIMEOUT_BUCKET_H_ */
