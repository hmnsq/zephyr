/*
 * Copyright (c) 2026 BayLibre SAS
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ ZEPHYR_KERNEL_TIMEOUT_ಪಾತ್ರೆ_H
#ವ್ಯಾಖ್ಯಾನಿಸು ZEPHYR_KERNEL_TIMEOUT_ಪಾತ್ರೆ_H

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

#ವ್ಯಾಖ್ಯಾನಿಸು ಪಾತ್ರೆ_LISTS CONFIG_TIMEOUT_ಪಾತ್ರೆ_LISTS
#ವ್ಯಾಖ್ಯಾನಿಸು ಪಾತ್ರೆ_ಮುಖವಾಡ  (ಪಾತ್ರೆ_LISTS - 1)

BUILD_ASSERT((ಪಾತ್ರೆ_LISTS & ಪಾತ್ರೆ_ಮುಖವಾಡ) == 0,
	     "CONFIG_TIMEOUT_BUCKET_LISTS must be a power of two");
BUILD_ASSERT(ಪಾತ್ರೆ_LISTS <= 64, "bucket occupancy bitmap is at most 64 bits");

/* Occupancy bitmap, one bit per bucket. Stay 32-bit for the common (and
 * smallest, cheapest-to-scan on 32-bit targets) window and only widen to
 * 64-bit when more than 32 buckets are configured.
 */
#ಆದರೆ ಪಾತ್ರೆ_LISTS <= 32
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ uint32_t ಪಾತ್ರೆ_bitmap_t;
#ವ್ಯಾಖ್ಯಾನಿಸು ಪಾತ್ರೆ_ಬಿಟ್ಟುಗಳು_ctz(x) u32_ಎಣಿಕೆ_trailing_zeros(x)
#ಇಲ್ಲದಿದ್ದರೆ
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ uint64_t ಪಾತ್ರೆ_bitmap_t;
#ವ್ಯಾಖ್ಯಾನಿಸು ಪಾತ್ರೆ_ಬಿಟ್ಟುಗಳು_ctz(x) u64_ಎಣಿಕೆ_trailing_zeros(x)
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿಸು ಪಾತ್ರೆ_ಬಿಟ್(n) ((ಪಾತ್ರೆ_bitmap_t)1 << (n))

ಸ್ಥಿರ sys_dlist_t ಪಾತ್ರೆ[ಪಾತ್ರೆ_LISTS];
ಸ್ಥಿರ sys_dlist_t ಅತಿಪ್ರವಾಹ;        /* sorted ascending by absolute expiry */
ಸ್ಥಿರ ಪಾತ್ರೆ_bitmap_t ಪಾತ್ರೆ_ಬಿಟ್ಟುಗಳು; /* bit p set => bucket[p] is non-empty */
ಸ್ಥಿರ uint64_t cursor_ತಳ;        /* window start, aligned to BUCKET_LISTS */

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ timeout_ಪಾತ್ರೆ_init(ಶೂನ್ಯ)
{
	ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ i = 0; i < ಪಾತ್ರೆ_LISTS; i++) {
		sys_dlist_init(&ಪಾತ್ರೆ[i]);
	}
	sys_dlist_init(&ಅತಿಪ್ರವಾಹ);
	ಪಾತ್ರೆ_ಬಿಟ್ಟುಗಳು = 0U;
	cursor_ತಳ = 0U;

	ಹಿಂತಿರುಗಿ 0;
}

/* Runs before the timer driver (PRE_KERNEL_2) and before any timeout is added.
 * Priority 0 keeps it ahead of other PRE_KERNEL_1 initialisation.
 */
SYS_INIT(timeout_ಪಾತ್ರೆ_init, PRE_KERNEL_1, 0);

ಸ್ಥಿರ ರಚನೆ _timeout *ಅತಿಪ್ರವಾಹ_ತಲೆ(ಶೂನ್ಯ)
{
	sys_dnode_t *n = sys_dlist_peek_ತಲೆ(&ಅತಿಪ್ರವಾಹ);

	ಹಿಂತಿರುಗಿ (n == ಖಾಲಿ) ? ಖಾಲಿ : CONTAINER_ದ(n, ರಚನೆ _timeout, ಗಂಟು);
}

/* Ticks from curr_tick to the earliest pending expiry, or INT64_MAX if none. */
ಸ್ಥಿರ int64_t earliest_gap(ಶೂನ್ಯ)
{
	int64_t gap = INT64_MAX;
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ cur_off = (ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ)(curr_tick & ಪಾತ್ರೆ_ಮುಖವಾಡ);

	/* Nearest non-empty bucket strictly ahead of curr_tick in the window.
	 * cursor_base is aligned, so bucket index == window offset.
	 */
	ಆದರೆ (cur_off < (ಪಾತ್ರೆ_LISTS - 1)) {
		ಪಾತ್ರೆ_bitmap_t ahead = ಪಾತ್ರೆ_ಬಿಟ್ಟುಗಳು & (~(ಪಾತ್ರೆ_bitmap_t)0 << (cur_off + 1));

		ಆದರೆ (ahead != 0U) {
			gap = (int64_t)ಪಾತ್ರೆ_ಬಿಟ್ಟುಗಳು_ctz(ahead) - cur_off;
		}
	}

	ಆದರೆ (gap == INT64_MAX) {
		ರಚನೆ _timeout *t = ಅತಿಪ್ರವಾಹ_ತಲೆ();

		ಆದರೆ (t != ಖಾಲಿ) {
			gap = (int64_t)t->dticks - (int64_t)curr_tick;
		}
	}

	ಹಿಂತಿರುಗಿ gap;
}

ಸ್ಥಿರ bool z_timeout_q_ಸೇರಿಸು(ರಚನೆ _timeout *ಗಮ್ಯ, k_ticks_t dticks)
{
	uint64_t expiry = curr_tick + (uint64_t)dticks;
	int64_t old_gap = earliest_gap();

	ಆದರೆ (expiry < cursor_ತಳ + ಪಾತ್ರೆ_LISTS) {
		ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ idx = (ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ)(expiry & ಪಾತ್ರೆ_ಮುಖವಾಡ);

		ಗಮ್ಯ->dticks = idx;
		ಪಾತ್ರೆ_ಬಿಟ್ಟುಗಳು |= ಪಾತ್ರೆ_ಬಿಟ್(idx);
		sys_dlist_append(&ಪಾತ್ರೆ[idx], &ಗಮ್ಯ->ಗಂಟು);
	} ಇಲ್ಲದಿದ್ದರೆ {
		/* Overflow: keep sorted ascending by absolute expiry. Insert
		 * after any equal-keyed entries so same-tick order stays FIFO.
		 */
		ರಚನೆ _timeout *t;

		ಗಮ್ಯ->dticks = (k_ticks_t)expiry;
		SYS_DLIST_FOR_EACH_CONTAINER(&ಅತಿಪ್ರವಾಹ, t, ಗಂಟು) {
			ಆದರೆ ((uint64_t)t->dticks > expiry) {
				sys_dlist_ಸೇರಿಸು(&t->ಗಂಟು, &ಗಮ್ಯ->ಗಂಟು);
				ಹಿಂತಿರುಗಿ (int64_t)dticks < old_gap;
			}
		}
		sys_dlist_append(&ಅತಿಪ್ರವಾಹ, &ಗಮ್ಯ->ಗಂಟು);
	}

	ಹಿಂತಿರುಗಿ (int64_t)dticks < old_gap;
}

ಸ್ಥಿರ k_ticks_t z_timeout_q_remainder(ಸ್ಥಿರಾಂಕ ರಚನೆ _timeout *ಗಮ್ಯ)
{
	ಆದರೆ ((uint64_t)ಗಮ್ಯ->dticks < ಪಾತ್ರೆ_LISTS) {
		/* Bucket entry: reconstruct its absolute expiry, the unique
		 * tick in the window whose low bits are the bucket index.
		 */
		uint64_t off = ((uint64_t)ಗಮ್ಯ->dticks - cursor_ತಳ) & ಪಾತ್ರೆ_ಮುಖವಾಡ;

		ಹಿಂತಿರುಗಿ (k_ticks_t)((cursor_ತಳ + off) - curr_tick);
	}

	ಹಿಂತಿರುಗಿ (k_ticks_t)((uint64_t)ಗಮ್ಯ->dticks - curr_tick);
}

ಸ್ಥಿರ bool z_timeout_q_remove(ರಚನೆ _timeout *ಗಮ್ಯ)
{
	bool was_ಮೊದಲ = (z_timeout_q_remainder(ಗಮ್ಯ) <= earliest_gap());

	ಆದರೆ ((uint64_t)ಗಮ್ಯ->dticks < ಪಾತ್ರೆ_LISTS) {
		ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ idx = (ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ)ಗಮ್ಯ->dticks;

		sys_dlist_remove(&ಗಮ್ಯ->ಗಂಟು);
		ಆದರೆ (sys_dlist_is_ಖಾಲಿಗಳು(&ಪಾತ್ರೆ[idx])) {
			ಪಾತ್ರೆ_ಬಿಟ್ಟುಗಳು &= ~ಪಾತ್ರೆ_ಬಿಟ್(idx);
		}
	} ಇಲ್ಲದಿದ್ದರೆ {
		/* Overflow stores absolute expiry (not a delta), so removal
		 * needs no successor fix-up.
		 */
		sys_dlist_remove(&ಗಮ್ಯ->ಗಂಟು);
	}

	ಹಿಂತಿರುಗಿ was_ಮೊದಲ;
}

/* Ticks from curr_tick to the earliest pending timeout, or K_TICKS_FOREVER if
 * none. The announce-range cap is applied centrally by next_timeout() in
 * timeout.c.
 */
ಸ್ಥಿರ k_ticks_t z_timeout_q_ಮುಂದಿನ_expiry(ಶೂನ್ಯ)
{
	int64_t gap = earliest_gap();

	ಹಿಂತಿರುಗಿ (gap == INT64_MAX) ? K_TICKS_FOREVER : (k_ticks_t)gap;
}

ಸ್ಥಿರ int32_t z_timeout_q_ಮುಂದಿನ_gap(ಶೂನ್ಯ)
{
	int64_t gap = earliest_gap();

	ಹಿಂತಿರುಗಿ (gap == INT64_MAX) ? INT32_MAX : (int32_t)MIN(gap, (int64_t)INT32_MAX);
}

ಸ್ಥಿರ ಶೂನ್ಯ z_timeout_q_advance(int32_t dt)
{
	uint64_t new_curr = curr_tick + (uint64_t)dt;

	ಆದರೆ (new_curr < cursor_ತಳ + ಪಾತ್ರೆ_LISTS) {
		/* Still inside the current window: nothing to migrate. */
		ಹಿಂತಿರುಗಿ;
	}

	/* Crossed the window. The loop only advances past the boundary when no
	 * bucket lies ahead (next_gap then came from overflow), so every bucket
	 * is empty here. Re-base the window onto curr_tick's new position and
	 * migrate the overflow entries that have come within range.
	 */
	cursor_ತಳ = new_curr & ~(uint64_t)ಪಾತ್ರೆ_ಮುಖವಾಡ;
	ಪಾತ್ರೆ_ಬಿಟ್ಟುಗಳು = 0U;

	ರಚನೆ _timeout *t;

	ಆಗ ((t = ಅತಿಪ್ರವಾಹ_ತಲೆ()) != ಖಾಲಿ &&
	       (uint64_t)t->dticks < cursor_ತಳ + ಪಾತ್ರೆ_LISTS) {
		ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ idx = (ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ)((uint64_t)t->dticks & ಪಾತ್ರೆ_ಮುಖವಾಡ);

		sys_dlist_remove(&t->ಗಂಟು);
		t->dticks = idx;
		ಪಾತ್ರೆ_ಬಿಟ್ಟುಗಳು |= ಪಾತ್ರೆ_ಬಿಟ್(idx);
		sys_dlist_append(&ಪಾತ್ರೆ[idx], &t->ಗಂಟು);
	}
}

ಸ್ಥಿರ ರಚನೆ _timeout *z_timeout_q_ತೆಗೆ_due(ಶೂನ್ಯ)
{
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ idx = (ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ)(curr_tick & ಪಾತ್ರೆ_ಮುಖವಾಡ);
	sys_dnode_t *n = sys_dlist_peek_ತಲೆ(&ಪಾತ್ರೆ[idx]);
	ರಚನೆ _timeout *t;

	ಆದರೆ (n == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}

	/* All entries in bucket[curr_tick & MASK] expire exactly at curr_tick. */
	t = CONTAINER_ದ(n, ರಚನೆ _timeout, ಗಂಟು);
	sys_dlist_remove(&t->ಗಂಟು);
	ಆದರೆ (sys_dlist_is_ಖಾಲಿಗಳು(&ಪಾತ್ರೆ[idx])) {
		ಪಾತ್ರೆ_ಬಿಟ್ಟುಗಳು &= ~ಪಾತ್ರೆ_ಬಿಟ್(idx);
	}

	ಹಿಂತಿರುಗಿ t;
}

#ಆದರೆ_ಅಂತ್ಯ /* ZEPHYR_KERNEL_TIMEOUT_BUCKET_H_ */
