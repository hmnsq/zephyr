/*
 * Copyright (c) 2026 BayLibre SAS
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#నిర్వచితం_కాకపోతే ZEPHYR_KERNEL_TIMEOUT_పాత్ర_H
#నిర్వచించు ZEPHYR_KERNEL_TIMEOUT_పాత్ర_H

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

#అన్తర్భావ <zephyr/init.h>
#అన్తర్భావ <zephyr/sys/dlist.h>

#నిర్వచించు పాత్ర_LISTS CONFIG_TIMEOUT_పాత్ర_LISTS
#నిర్వచించు పాత్ర_ముసుగు  (పాత్ర_LISTS - 1)

BUILD_ASSERT((పాత్ర_LISTS & పాత్ర_ముసుగు) == 0,
	     "CONFIG_TIMEOUT_BUCKET_LISTS must be a power of two");
BUILD_ASSERT(పాత్ర_LISTS <= 64, "bucket occupancy bitmap is at most 64 bits");

/* Occupancy bitmap, one bit per bucket. Stay 32-bit for the common (and
 * smallest, cheapest-to-scan on 32-bit targets) window and only widen to
 * 64-bit when more than 32 buckets are configured.
 */
#ఒకవేళ పాత్ర_LISTS <= 32
రకనిర్వచనం uint32_t పాత్ర_bitmap_t;
#నిర్వచించు పాత్ర_బిట్లు_ctz(x) u32_లెక్క_trailing_zeros(x)
#లేకపోతే
రకనిర్వచనం uint64_t పాత్ర_bitmap_t;
#నిర్వచించు పాత్ర_బిట్లు_ctz(x) u64_లెక్క_trailing_zeros(x)
#అయితే_ముగింపు
#నిర్వచించు పాత్ర_బిట్టు(n) ((పాత్ర_bitmap_t)1 << (n))

స్థిర sys_dlist_t పాత్ర[పాత్ర_LISTS];
స్థిర sys_dlist_t అధికప్రవాహం;        /* sorted ascending by absolute expiry */
స్థిర పాత్ర_bitmap_t పాత్ర_బిట్లు; /* bit p set => bucket[p] is non-empty */
స్థిర uint64_t cursor_పునాది;        /* window start, aligned to BUCKET_LISTS */

స్థిర పూర్ణాంకం timeout_పాత్ర_init(శూన్య)
{
	కొరకు (గుర్తులేని పూర్ణాంకం i = 0; i < పాత్ర_LISTS; i++) {
		sys_dlist_init(&పాత్ర[i]);
	}
	sys_dlist_init(&అధికప్రవాహం);
	పాత్ర_బిట్లు = 0U;
	cursor_పునాది = 0U;

	తిరిగి 0;
}

/* Runs before the timer driver (PRE_KERNEL_2) and before any timeout is added.
 * Priority 0 keeps it ahead of other PRE_KERNEL_1 initialisation.
 */
SYS_INIT(timeout_పాత్ర_init, PRE_KERNEL_1, 0);

స్థిర నిర్మాణం _timeout *అధికప్రవాహం_తల(శూన్య)
{
	sys_dnode_t *n = sys_dlist_peek_తల(&అధికప్రవాహం);

	తిరిగి (n == ఖాళీ) ? ఖాళీ : CONTAINER_యొక్క(n, నిర్మాణం _timeout, కణుపు);
}

/* Ticks from curr_tick to the earliest pending expiry, or INT64_MAX if none. */
స్థిర int64_t earliest_gap(శూన్య)
{
	int64_t gap = INT64_MAX;
	గుర్తులేని పూర్ణాంకం cur_off = (గుర్తులేని పూర్ణాంకం)(curr_tick & పాత్ర_ముసుగు);

	/* Nearest non-empty bucket strictly ahead of curr_tick in the window.
	 * cursor_base is aligned, so bucket index == window offset.
	 */
	ఒకవేళ (cur_off < (పాత్ర_LISTS - 1)) {
		పాత్ర_bitmap_t ahead = పాత్ర_బిట్లు & (~(పాత్ర_bitmap_t)0 << (cur_off + 1));

		ఒకవేళ (ahead != 0U) {
			gap = (int64_t)పాత్ర_బిట్లు_ctz(ahead) - cur_off;
		}
	}

	ఒకవేళ (gap == INT64_MAX) {
		నిర్మాణం _timeout *t = అధికప్రవాహం_తల();

		ఒకవేళ (t != ఖాళీ) {
			gap = (int64_t)t->dticks - (int64_t)curr_tick;
		}
	}

	తిరిగి gap;
}

స్థిర bool z_timeout_q_చొప్పించు(నిర్మాణం _timeout *గమ్యం, k_ticks_t dticks)
{
	uint64_t expiry = curr_tick + (uint64_t)dticks;
	int64_t old_gap = earliest_gap();

	ఒకవేళ (expiry < cursor_పునాది + పాత్ర_LISTS) {
		గుర్తులేని పూర్ణాంకం idx = (గుర్తులేని పూర్ణాంకం)(expiry & పాత్ర_ముసుగు);

		గమ్యం->dticks = idx;
		పాత్ర_బిట్లు |= పాత్ర_బిట్టు(idx);
		sys_dlist_append(&పాత్ర[idx], &గమ్యం->కణుపు);
	} లేకపోతే {
		/* Overflow: keep sorted ascending by absolute expiry. Insert
		 * after any equal-keyed entries so same-tick order stays FIFO.
		 */
		నిర్మాణం _timeout *t;

		గమ్యం->dticks = (k_ticks_t)expiry;
		SYS_DLIST_FOR_EACH_CONTAINER(&అధికప్రవాహం, t, కణుపు) {
			ఒకవేళ ((uint64_t)t->dticks > expiry) {
				sys_dlist_చొప్పించు(&t->కణుపు, &గమ్యం->కణుపు);
				తిరిగి (int64_t)dticks < old_gap;
			}
		}
		sys_dlist_append(&అధికప్రవాహం, &గమ్యం->కణుపు);
	}

	తిరిగి (int64_t)dticks < old_gap;
}

స్థిర k_ticks_t z_timeout_q_remainder(స్థిరరాశి నిర్మాణం _timeout *గమ్యం)
{
	ఒకవేళ ((uint64_t)గమ్యం->dticks < పాత్ర_LISTS) {
		/* Bucket entry: reconstruct its absolute expiry, the unique
		 * tick in the window whose low bits are the bucket index.
		 */
		uint64_t off = ((uint64_t)గమ్యం->dticks - cursor_పునాది) & పాత్ర_ముసుగు;

		తిరిగి (k_ticks_t)((cursor_పునాది + off) - curr_tick);
	}

	తిరిగి (k_ticks_t)((uint64_t)గమ్యం->dticks - curr_tick);
}

స్థిర bool z_timeout_q_remove(నిర్మాణం _timeout *గమ్యం)
{
	bool was_మొదటి = (z_timeout_q_remainder(గమ్యం) <= earliest_gap());

	ఒకవేళ ((uint64_t)గమ్యం->dticks < పాత్ర_LISTS) {
		గుర్తులేని పూర్ణాంకం idx = (గుర్తులేని పూర్ణాంకం)గమ్యం->dticks;

		sys_dlist_remove(&గమ్యం->కణుపు);
		ఒకవేళ (sys_dlist_is_ఖాళీలు(&పాత్ర[idx])) {
			పాత్ర_బిట్లు &= ~పాత్ర_బిట్టు(idx);
		}
	} లేకపోతే {
		/* Overflow stores absolute expiry (not a delta), so removal
		 * needs no successor fix-up.
		 */
		sys_dlist_remove(&గమ్యం->కణుపు);
	}

	తిరిగి was_మొదటి;
}

/* Ticks from curr_tick to the earliest pending timeout, or K_TICKS_FOREVER if
 * none. The announce-range cap is applied centrally by next_timeout() in
 * timeout.c.
 */
స్థిర k_ticks_t z_timeout_q_తదుపరి_expiry(శూన్య)
{
	int64_t gap = earliest_gap();

	తిరిగి (gap == INT64_MAX) ? K_TICKS_FOREVER : (k_ticks_t)gap;
}

స్థిర int32_t z_timeout_q_తదుపరి_gap(శూన్య)
{
	int64_t gap = earliest_gap();

	తిరిగి (gap == INT64_MAX) ? INT32_MAX : (int32_t)MIN(gap, (int64_t)INT32_MAX);
}

స్థిర శూన్య z_timeout_q_advance(int32_t dt)
{
	uint64_t new_curr = curr_tick + (uint64_t)dt;

	ఒకవేళ (new_curr < cursor_పునాది + పాత్ర_LISTS) {
		/* Still inside the current window: nothing to migrate. */
		తిరిగి;
	}

	/* Crossed the window. The loop only advances past the boundary when no
	 * bucket lies ahead (next_gap then came from overflow), so every bucket
	 * is empty here. Re-base the window onto curr_tick's new position and
	 * migrate the overflow entries that have come within range.
	 */
	cursor_పునాది = new_curr & ~(uint64_t)పాత్ర_ముసుగు;
	పాత్ర_బిట్లు = 0U;

	నిర్మాణం _timeout *t;

	అయితే ((t = అధికప్రవాహం_తల()) != ఖాళీ &&
	       (uint64_t)t->dticks < cursor_పునాది + పాత్ర_LISTS) {
		గుర్తులేని పూర్ణాంకం idx = (గుర్తులేని పూర్ణాంకం)((uint64_t)t->dticks & పాత్ర_ముసుగు);

		sys_dlist_remove(&t->కణుపు);
		t->dticks = idx;
		పాత్ర_బిట్లు |= పాత్ర_బిట్టు(idx);
		sys_dlist_append(&పాత్ర[idx], &t->కణుపు);
	}
}

స్థిర నిర్మాణం _timeout *z_timeout_q_తీయి_due(శూన్య)
{
	గుర్తులేని పూర్ణాంకం idx = (గుర్తులేని పూర్ణాంకం)(curr_tick & పాత్ర_ముసుగు);
	sys_dnode_t *n = sys_dlist_peek_తల(&పాత్ర[idx]);
	నిర్మాణం _timeout *t;

	ఒకవేళ (n == ఖాళీ) {
		తిరిగి ఖాళీ;
	}

	/* All entries in bucket[curr_tick & MASK] expire exactly at curr_tick. */
	t = CONTAINER_యొక్క(n, నిర్మాణం _timeout, కణుపు);
	sys_dlist_remove(&t->కణుపు);
	ఒకవేళ (sys_dlist_is_ఖాళీలు(&పాత్ర[idx])) {
		పాత్ర_బిట్లు &= ~పాత్ర_బిట్టు(idx);
	}

	తిరిగి t;
}

#అయితే_ముగింపు /* ZEPHYR_KERNEL_TIMEOUT_BUCKET_H_ */
