/*
 * Copyright (c) 2026 BayLibre SAS
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#اگر_غیر_تعریف_شدہ ZEPHYR_KERNEL_TIMEOUT_بالٹی_H
#تعریف_کرو ZEPHYR_KERNEL_TIMEOUT_بالٹی_H

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

#تعریف_کرو بالٹی_LISTS CONFIG_TIMEOUT_بالٹی_LISTS
#تعریف_کرو بالٹی_نقاب  (بالٹی_LISTS - 1)

BUILD_ASSERT((بالٹی_LISTS & بالٹی_نقاب) == 0,
	     "CONFIG_TIMEOUT_BUCKET_LISTS must be a power of two");
BUILD_ASSERT(بالٹی_LISTS <= 64, "bucket occupancy bitmap is at most 64 bits");

/* Occupancy bitmap, one bit per bucket. Stay 32-bit for the common (and
 * smallest, cheapest-to-scan on 32-bit targets) window and only widen to
 * 64-bit when more than 32 buckets are configured.
 */
#اگر بالٹی_LISTS <= 32
قسمتعریف uint32_t بالٹی_bitmap_t;
#تعریف_کرو بالٹی_بٹ_ctz(x) u32_گنتی_trailing_zeros(x)
#ورنہ
قسمتعریف uint64_t بالٹی_bitmap_t;
#تعریف_کرو بالٹی_بٹ_ctz(x) u64_گنتی_trailing_zeros(x)
#اگر_ختم
#تعریف_کرو بالٹی_ثنائی_ہندسہ(n) ((بالٹی_bitmap_t)1 << (n))

ساکن sys_dlist_t بالٹی[بالٹی_LISTS];
ساکن sys_dlist_t حد_سے_تجاوز;        /* sorted ascending by absolute expiry */
ساکن بالٹی_bitmap_t بالٹی_بٹ; /* bit p set => bucket[p] is non-empty */
ساکن uint64_t cursor_بنیاد;        /* window start, aligned to BUCKET_LISTS */

ساکن صحیح timeout_بالٹی_init(خالی)
{
	برائے (غیرعلامتی صحیح i = 0; i < بالٹی_LISTS; i++) {
		sys_dlist_init(&بالٹی[i]);
	}
	sys_dlist_init(&حد_سے_تجاوز);
	بالٹی_بٹ = 0U;
	cursor_بنیاد = 0U;

	واپس 0;
}

/* Runs before the timer driver (PRE_KERNEL_2) and before any timeout is added.
 * Priority 0 keeps it ahead of other PRE_KERNEL_1 initialisation.
 */
SYS_INIT(timeout_بالٹی_init, PRE_KERNEL_1, 0);

ساکن ساخت _timeout *حد_سے_تجاوز_سر(خالی)
{
	sys_dnode_t *n = sys_dlist_peek_سر(&حد_سے_تجاوز);

	واپس (n == تہی) ? تہی : CONTAINER_کا(n, ساخت _timeout, گرہ);
}

/* Ticks from curr_tick to the earliest pending expiry, or INT64_MAX if none. */
ساکن int64_t earliest_gap(خالی)
{
	int64_t gap = INT64_MAX;
	غیرعلامتی صحیح cur_off = (غیرعلامتی صحیح)(curr_tick & بالٹی_نقاب);

	/* Nearest non-empty bucket strictly ahead of curr_tick in the window.
	 * cursor_base is aligned, so bucket index == window offset.
	 */
	اگر (cur_off < (بالٹی_LISTS - 1)) {
		بالٹی_bitmap_t ahead = بالٹی_بٹ & (~(بالٹی_bitmap_t)0 << (cur_off + 1));

		اگر (ahead != 0U) {
			gap = (int64_t)بالٹی_بٹ_ctz(ahead) - cur_off;
		}
	}

	اگر (gap == INT64_MAX) {
		ساخت _timeout *t = حد_سے_تجاوز_سر();

		اگر (t != تہی) {
			gap = (int64_t)t->dticks - (int64_t)curr_tick;
		}
	}

	واپس gap;
}

ساکن bool z_timeout_q_داخل_کرو(ساخت _timeout *مقصد, k_ticks_t dticks)
{
	uint64_t expiry = curr_tick + (uint64_t)dticks;
	int64_t old_gap = earliest_gap();

	اگر (expiry < cursor_بنیاد + بالٹی_LISTS) {
		غیرعلامتی صحیح idx = (غیرعلامتی صحیح)(expiry & بالٹی_نقاب);

		مقصد->dticks = idx;
		بالٹی_بٹ |= بالٹی_ثنائی_ہندسہ(idx);
		sys_dlist_append(&بالٹی[idx], &مقصد->گرہ);
	} ورنہ {
		/* Overflow: keep sorted ascending by absolute expiry. Insert
		 * after any equal-keyed entries so same-tick order stays FIFO.
		 */
		ساخت _timeout *t;

		مقصد->dticks = (k_ticks_t)expiry;
		SYS_DLIST_FOR_EACH_CONTAINER(&حد_سے_تجاوز, t, گرہ) {
			اگر ((uint64_t)t->dticks > expiry) {
				sys_dlist_داخل_کرو(&t->گرہ, &مقصد->گرہ);
				واپس (int64_t)dticks < old_gap;
			}
		}
		sys_dlist_append(&حد_سے_تجاوز, &مقصد->گرہ);
	}

	واپس (int64_t)dticks < old_gap;
}

ساکن k_ticks_t z_timeout_q_remainder(ثابت ساخت _timeout *مقصد)
{
	اگر ((uint64_t)مقصد->dticks < بالٹی_LISTS) {
		/* Bucket entry: reconstruct its absolute expiry, the unique
		 * tick in the window whose low bits are the bucket index.
		 */
		uint64_t off = ((uint64_t)مقصد->dticks - cursor_بنیاد) & بالٹی_نقاب;

		واپس (k_ticks_t)((cursor_بنیاد + off) - curr_tick);
	}

	واپس (k_ticks_t)((uint64_t)مقصد->dticks - curr_tick);
}

ساکن bool z_timeout_q_remove(ساخت _timeout *مقصد)
{
	bool was_پہلا = (z_timeout_q_remainder(مقصد) <= earliest_gap());

	اگر ((uint64_t)مقصد->dticks < بالٹی_LISTS) {
		غیرعلامتی صحیح idx = (غیرعلامتی صحیح)مقصد->dticks;

		sys_dlist_remove(&مقصد->گرہ);
		اگر (sys_dlist_is_خالی_خانے(&بالٹی[idx])) {
			بالٹی_بٹ &= ~بالٹی_ثنائی_ہندسہ(idx);
		}
	} ورنہ {
		/* Overflow stores absolute expiry (not a delta), so removal
		 * needs no successor fix-up.
		 */
		sys_dlist_remove(&مقصد->گرہ);
	}

	واپس was_پہلا;
}

/* Ticks from curr_tick to the earliest pending timeout, or K_TICKS_FOREVER if
 * none. The announce-range cap is applied centrally by next_timeout() in
 * timeout.c.
 */
ساکن k_ticks_t z_timeout_q_اگلا_expiry(خالی)
{
	int64_t gap = earliest_gap();

	واپس (gap == INT64_MAX) ? K_TICKS_FOREVER : (k_ticks_t)gap;
}

ساکن int32_t z_timeout_q_اگلا_gap(خالی)
{
	int64_t gap = earliest_gap();

	واپس (gap == INT64_MAX) ? INT32_MAX : (int32_t)MIN(gap, (int64_t)INT32_MAX);
}

ساکن خالی z_timeout_q_advance(int32_t dt)
{
	uint64_t new_curr = curr_tick + (uint64_t)dt;

	اگر (new_curr < cursor_بنیاد + بالٹی_LISTS) {
		/* Still inside the current window: nothing to migrate. */
		واپس;
	}

	/* Crossed the window. The loop only advances past the boundary when no
	 * bucket lies ahead (next_gap then came from overflow), so every bucket
	 * is empty here. Re-base the window onto curr_tick's new position and
	 * migrate the overflow entries that have come within range.
	 */
	cursor_بنیاد = new_curr & ~(uint64_t)بالٹی_نقاب;
	بالٹی_بٹ = 0U;

	ساخت _timeout *t;

	جب ((t = حد_سے_تجاوز_سر()) != تہی &&
	       (uint64_t)t->dticks < cursor_بنیاد + بالٹی_LISTS) {
		غیرعلامتی صحیح idx = (غیرعلامتی صحیح)((uint64_t)t->dticks & بالٹی_نقاب);

		sys_dlist_remove(&t->گرہ);
		t->dticks = idx;
		بالٹی_بٹ |= بالٹی_ثنائی_ہندسہ(idx);
		sys_dlist_append(&بالٹی[idx], &t->گرہ);
	}
}

ساکن ساخت _timeout *z_timeout_q_نکالو_due(خالی)
{
	غیرعلامتی صحیح idx = (غیرعلامتی صحیح)(curr_tick & بالٹی_نقاب);
	sys_dnode_t *n = sys_dlist_peek_سر(&بالٹی[idx]);
	ساخت _timeout *t;

	اگر (n == تہی) {
		واپس تہی;
	}

	/* All entries in bucket[curr_tick & MASK] expire exactly at curr_tick. */
	t = CONTAINER_کا(n, ساخت _timeout, گرہ);
	sys_dlist_remove(&t->گرہ);
	اگر (sys_dlist_is_خالی_خانے(&بالٹی[idx])) {
		بالٹی_بٹ &= ~بالٹی_ثنائی_ہندسہ(idx);
	}

	واپس t;
}

#اگر_ختم /* ZEPHYR_KERNEL_TIMEOUT_BUCKET_H_ */
