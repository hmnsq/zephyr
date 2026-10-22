/*
 * Copyright (c) 2026 BayLibre SAS
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ ZEPHYR_KERNEL_TIMEOUT_ਬਾਲਟੀ_H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ZEPHYR_KERNEL_TIMEOUT_ਬਾਲਟੀ_H

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

#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/init.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/sys/dlist.h>

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਬਾਲਟੀ_LISTS CONFIG_TIMEOUT_ਬਾਲਟੀ_LISTS
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਬਾਲਟੀ_ਨਕਾਬ  (ਬਾਲਟੀ_LISTS - 1)

BUILD_ASSERT((ਬਾਲਟੀ_LISTS & ਬਾਲਟੀ_ਨਕਾਬ) == 0,
	     "CONFIG_TIMEOUT_BUCKET_LISTS must be a power of two");
BUILD_ASSERT(ਬਾਲਟੀ_LISTS <= 64, "bucket occupancy bitmap is at most 64 bits");

/* Occupancy bitmap, one bit per bucket. Stay 32-bit for the common (and
 * smallest, cheapest-to-scan on 32-bit targets) window and only widen to
 * 64-bit when more than 32 buckets are configured.
 */
#ਜੇ ਬਾਲਟੀ_LISTS <= 32
ਕਿਸਮਪਰਿਭਾਸ਼ਾ uint32_t ਬਾਲਟੀ_bitmap_t;
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਬਾਲਟੀ_ਬਿੱਟ_ctz(x) u32_ਗਿਣਤੀ_trailing_zeros(x)
#ਨਹੀਂਤਾਂ
ਕਿਸਮਪਰਿਭਾਸ਼ਾ uint64_t ਬਾਲਟੀ_bitmap_t;
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਬਾਲਟੀ_ਬਿੱਟ_ctz(x) u64_ਗਿਣਤੀ_trailing_zeros(x)
#ਜੇ_ਅੰਤ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਬਾਲਟੀ_ਦੋਆਧਾਰੀ_ਅੰਕ(n) ((ਬਾਲਟੀ_bitmap_t)1 << (n))

ਸਥਿਰ sys_dlist_t ਬਾਲਟੀ[ਬਾਲਟੀ_LISTS];
ਸਥਿਰ sys_dlist_t ਉਛਾਲ;        /* sorted ascending by absolute expiry */
ਸਥਿਰ ਬਾਲਟੀ_bitmap_t ਬਾਲਟੀ_ਬਿੱਟ; /* bit p set => bucket[p] is non-empty */
ਸਥਿਰ uint64_t cursor_ਅਧਾਰ;        /* window start, aligned to BUCKET_LISTS */

ਸਥਿਰ ਪੂਰਨ timeout_ਬਾਲਟੀ_init(ਖਾਲੀ)
{
	ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ i = 0; i < ਬਾਲਟੀ_LISTS; i++) {
		sys_dlist_init(&ਬਾਲਟੀ[i]);
	}
	sys_dlist_init(&ਉਛਾਲ);
	ਬਾਲਟੀ_ਬਿੱਟ = 0U;
	cursor_ਅਧਾਰ = 0U;

	ਵਾਪਸ 0;
}

/* Runs before the timer driver (PRE_KERNEL_2) and before any timeout is added.
 * Priority 0 keeps it ahead of other PRE_KERNEL_1 initialisation.
 */
SYS_INIT(timeout_ਬਾਲਟੀ_init, PRE_KERNEL_1, 0);

ਸਥਿਰ ਬਣਤਰ _timeout *ਉਛਾਲ_ਸਿਰ(ਖਾਲੀ)
{
	sys_dnode_t *n = sys_dlist_peek_ਸਿਰ(&ਉਛਾਲ);

	ਵਾਪਸ (n == ਨਿਰਾ) ? ਨਿਰਾ : CONTAINER_ਦਾ(n, ਬਣਤਰ _timeout, ਗੰਢ);
}

/* Ticks from curr_tick to the earliest pending expiry, or INT64_MAX if none. */
ਸਥਿਰ int64_t earliest_gap(ਖਾਲੀ)
{
	int64_t gap = INT64_MAX;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ cur_off = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ)(curr_tick & ਬਾਲਟੀ_ਨਕਾਬ);

	/* Nearest non-empty bucket strictly ahead of curr_tick in the window.
	 * cursor_base is aligned, so bucket index == window offset.
	 */
	ਜੇ (cur_off < (ਬਾਲਟੀ_LISTS - 1)) {
		ਬਾਲਟੀ_bitmap_t ahead = ਬਾਲਟੀ_ਬਿੱਟ & (~(ਬਾਲਟੀ_bitmap_t)0 << (cur_off + 1));

		ਜੇ (ahead != 0U) {
			gap = (int64_t)ਬਾਲਟੀ_ਬਿੱਟ_ctz(ahead) - cur_off;
		}
	}

	ਜੇ (gap == INT64_MAX) {
		ਬਣਤਰ _timeout *t = ਉਛਾਲ_ਸਿਰ();

		ਜੇ (t != ਨਿਰਾ) {
			gap = (int64_t)t->dticks - (int64_t)curr_tick;
		}
	}

	ਵਾਪਸ gap;
}

ਸਥਿਰ bool z_timeout_q_ਪਾਓ(ਬਣਤਰ _timeout *ਮੰਜ਼ਿਲ, k_ticks_t dticks)
{
	uint64_t expiry = curr_tick + (uint64_t)dticks;
	int64_t old_gap = earliest_gap();

	ਜੇ (expiry < cursor_ਅਧਾਰ + ਬਾਲਟੀ_LISTS) {
		ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ idx = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ)(expiry & ਬਾਲਟੀ_ਨਕਾਬ);

		ਮੰਜ਼ਿਲ->dticks = idx;
		ਬਾਲਟੀ_ਬਿੱਟ |= ਬਾਲਟੀ_ਦੋਆਧਾਰੀ_ਅੰਕ(idx);
		sys_dlist_append(&ਬਾਲਟੀ[idx], &ਮੰਜ਼ਿਲ->ਗੰਢ);
	} ਨਹੀਂਤਾਂ {
		/* Overflow: keep sorted ascending by absolute expiry. Insert
		 * after any equal-keyed entries so same-tick order stays FIFO.
		 */
		ਬਣਤਰ _timeout *t;

		ਮੰਜ਼ਿਲ->dticks = (k_ticks_t)expiry;
		SYS_DLIST_FOR_EACH_CONTAINER(&ਉਛਾਲ, t, ਗੰਢ) {
			ਜੇ ((uint64_t)t->dticks > expiry) {
				sys_dlist_ਪਾਓ(&t->ਗੰਢ, &ਮੰਜ਼ਿਲ->ਗੰਢ);
				ਵਾਪਸ (int64_t)dticks < old_gap;
			}
		}
		sys_dlist_append(&ਉਛਾਲ, &ਮੰਜ਼ਿਲ->ਗੰਢ);
	}

	ਵਾਪਸ (int64_t)dticks < old_gap;
}

ਸਥਿਰ k_ticks_t z_timeout_q_remainder(ਅਚਲ ਬਣਤਰ _timeout *ਮੰਜ਼ਿਲ)
{
	ਜੇ ((uint64_t)ਮੰਜ਼ਿਲ->dticks < ਬਾਲਟੀ_LISTS) {
		/* Bucket entry: reconstruct its absolute expiry, the unique
		 * tick in the window whose low bits are the bucket index.
		 */
		uint64_t off = ((uint64_t)ਮੰਜ਼ਿਲ->dticks - cursor_ਅਧਾਰ) & ਬਾਲਟੀ_ਨਕਾਬ;

		ਵਾਪਸ (k_ticks_t)((cursor_ਅਧਾਰ + off) - curr_tick);
	}

	ਵਾਪਸ (k_ticks_t)((uint64_t)ਮੰਜ਼ਿਲ->dticks - curr_tick);
}

ਸਥਿਰ bool z_timeout_q_remove(ਬਣਤਰ _timeout *ਮੰਜ਼ਿਲ)
{
	bool was_ਪਹਿਲਾ = (z_timeout_q_remainder(ਮੰਜ਼ਿਲ) <= earliest_gap());

	ਜੇ ((uint64_t)ਮੰਜ਼ਿਲ->dticks < ਬਾਲਟੀ_LISTS) {
		ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ idx = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ)ਮੰਜ਼ਿਲ->dticks;

		sys_dlist_remove(&ਮੰਜ਼ਿਲ->ਗੰਢ);
		ਜੇ (sys_dlist_is_ਖਾਲੀ_ਥਾਵਾਂ(&ਬਾਲਟੀ[idx])) {
			ਬਾਲਟੀ_ਬਿੱਟ &= ~ਬਾਲਟੀ_ਦੋਆਧਾਰੀ_ਅੰਕ(idx);
		}
	} ਨਹੀਂਤਾਂ {
		/* Overflow stores absolute expiry (not a delta), so removal
		 * needs no successor fix-up.
		 */
		sys_dlist_remove(&ਮੰਜ਼ਿਲ->ਗੰਢ);
	}

	ਵਾਪਸ was_ਪਹਿਲਾ;
}

/* Ticks from curr_tick to the earliest pending timeout, or K_TICKS_FOREVER if
 * none. The announce-range cap is applied centrally by next_timeout() in
 * timeout.c.
 */
ਸਥਿਰ k_ticks_t z_timeout_q_ਅਗਲਾ_expiry(ਖਾਲੀ)
{
	int64_t gap = earliest_gap();

	ਵਾਪਸ (gap == INT64_MAX) ? K_TICKS_FOREVER : (k_ticks_t)gap;
}

ਸਥਿਰ int32_t z_timeout_q_ਅਗਲਾ_gap(ਖਾਲੀ)
{
	int64_t gap = earliest_gap();

	ਵਾਪਸ (gap == INT64_MAX) ? INT32_MAX : (int32_t)MIN(gap, (int64_t)INT32_MAX);
}

ਸਥਿਰ ਖਾਲੀ z_timeout_q_advance(int32_t dt)
{
	uint64_t new_curr = curr_tick + (uint64_t)dt;

	ਜੇ (new_curr < cursor_ਅਧਾਰ + ਬਾਲਟੀ_LISTS) {
		/* Still inside the current window: nothing to migrate. */
		ਵਾਪਸ;
	}

	/* Crossed the window. The loop only advances past the boundary when no
	 * bucket lies ahead (next_gap then came from overflow), so every bucket
	 * is empty here. Re-base the window onto curr_tick's new position and
	 * migrate the overflow entries that have come within range.
	 */
	cursor_ਅਧਾਰ = new_curr & ~(uint64_t)ਬਾਲਟੀ_ਨਕਾਬ;
	ਬਾਲਟੀ_ਬਿੱਟ = 0U;

	ਬਣਤਰ _timeout *t;

	ਜਦੋਂ ((t = ਉਛਾਲ_ਸਿਰ()) != ਨਿਰਾ &&
	       (uint64_t)t->dticks < cursor_ਅਧਾਰ + ਬਾਲਟੀ_LISTS) {
		ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ idx = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ)((uint64_t)t->dticks & ਬਾਲਟੀ_ਨਕਾਬ);

		sys_dlist_remove(&t->ਗੰਢ);
		t->dticks = idx;
		ਬਾਲਟੀ_ਬਿੱਟ |= ਬਾਲਟੀ_ਦੋਆਧਾਰੀ_ਅੰਕ(idx);
		sys_dlist_append(&ਬਾਲਟੀ[idx], &t->ਗੰਢ);
	}
}

ਸਥਿਰ ਬਣਤਰ _timeout *z_timeout_q_ਕੱਢੋ_due(ਖਾਲੀ)
{
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ idx = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ)(curr_tick & ਬਾਲਟੀ_ਨਕਾਬ);
	sys_dnode_t *n = sys_dlist_peek_ਸਿਰ(&ਬਾਲਟੀ[idx]);
	ਬਣਤਰ _timeout *t;

	ਜੇ (n == ਨਿਰਾ) {
		ਵਾਪਸ ਨਿਰਾ;
	}

	/* All entries in bucket[curr_tick & MASK] expire exactly at curr_tick. */
	t = CONTAINER_ਦਾ(n, ਬਣਤਰ _timeout, ਗੰਢ);
	sys_dlist_remove(&t->ਗੰਢ);
	ਜੇ (sys_dlist_is_ਖਾਲੀ_ਥਾਵਾਂ(&ਬਾਲਟੀ[idx])) {
		ਬਾਲਟੀ_ਬਿੱਟ &= ~ਬਾਲਟੀ_ਦੋਆਧਾਰੀ_ਅੰਕ(idx);
	}

	ਵਾਪਸ t;
}

#ਜੇ_ਅੰਤ /* ZEPHYR_KERNEL_TIMEOUT_BUCKET_H_ */
