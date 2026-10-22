/*
 * Copyright (c) 2026 BayLibre SAS
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#جے_بنا_تعریف ZEPHYR_KERNEL_TIMEOUT_بالٹی_H
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

#ضمن <zephyr/init.h>
#ضمن <zephyr/sys/dlist.h>

#تعریف_کرو بالٹی_LISTS CONFIG_TIMEOUT_بالٹی_LISTS
#تعریف_کرو بالٹی_نقاب  (بالٹی_LISTS - 1)

BUILD_ASSERT((بالٹی_LISTS & بالٹی_نقاب) == 0,
	     "CONFIG_TIMEOUT_BUCKET_LISTS must be a power of two");
BUILD_ASSERT(بالٹی_LISTS <= 64, "bucket occupancy bitmap is at most 64 bits");

/* Occupancy bitmap, one bit per bucket. Stay 32-bit for the common (and
 * smallest, cheapest-to-scan on 32-bit targets) window and only widen to
 * 64-bit when more than 32 buckets are configured.
 */
#إذا بالٹی_LISTS <= 32
تعريفنوع uint32_t بالٹی_bitmap_t;
#تعریف_کرو بالٹی_بٹ_ctz(x) u32_گنتی_trailing_zeros(x)
#وإلا
تعريفنوع uint64_t بالٹی_bitmap_t;
#تعریف_کرو بالٹی_بٹ_ctz(x) u64_گنتی_trailing_zeros(x)
#جے_انت
#تعریف_کرو بالٹی_ثنائی_ہندسہ(n) ((بالٹی_bitmap_t)1 << (n))

ساكن sys_dlist_t بالٹی[بالٹی_LISTS];
ساكن sys_dlist_t حد_توں_ودھ;        /* sorted ascending by absolute expiry */
ساكن بالٹی_bitmap_t بالٹی_بٹ; /* bit p set => bucket[p] is non-empty */
ساكن uint64_t cursor_ادھار;        /* window start, aligned to BUCKET_LISTS */

ساكن صحيح timeout_بالٹی_init(فراغ)
{
	لكل (بنانشانی صحيح i = 0; i < بالٹی_LISTS; i++) {
		sys_dlist_init(&بالٹی[i]);
	}
	sys_dlist_init(&حد_توں_ودھ);
	بالٹی_بٹ = 0U;
	cursor_ادھار = 0U;

	أرجع 0;
}

/* Runs before the timer driver (PRE_KERNEL_2) and before any timeout is added.
 * Priority 0 keeps it ahead of other PRE_KERNEL_1 initialisation.
 */
SYS_INIT(timeout_بالٹی_init, PRE_KERNEL_1, 0);

ساكن بنية _timeout *حد_توں_ودھ_سر(فراغ)
{
	sys_dnode_t *n = sys_dlist_peek_سر(&حد_توں_ودھ);

	أرجع (n == عدم) ? عدم : CONTAINER_دا(n, بنية _timeout, گرہ);
}

/* Ticks from curr_tick to the earliest pending expiry, or INT64_MAX if none. */
ساكن int64_t earliest_gap(فراغ)
{
	int64_t gap = INT64_MAX;
	بنانشانی صحيح cur_off = (بنانشانی صحيح)(curr_tick & بالٹی_نقاب);

	/* Nearest non-empty bucket strictly ahead of curr_tick in the window.
	 * cursor_base is aligned, so bucket index == window offset.
	 */
	إذا (cur_off < (بالٹی_LISTS - 1)) {
		بالٹی_bitmap_t ahead = بالٹی_بٹ & (~(بالٹی_bitmap_t)0 << (cur_off + 1));

		إذا (ahead != 0U) {
			gap = (int64_t)بالٹی_بٹ_ctz(ahead) - cur_off;
		}
	}

	إذا (gap == INT64_MAX) {
		بنية _timeout *t = حد_توں_ودھ_سر();

		إذا (t != عدم) {
			gap = (int64_t)t->dticks - (int64_t)curr_tick;
		}
	}

	أرجع gap;
}

ساكن bool z_timeout_q_داخل_کرو(بنية _timeout *منزل, k_ticks_t dticks)
{
	uint64_t expiry = curr_tick + (uint64_t)dticks;
	int64_t old_gap = earliest_gap();

	إذا (expiry < cursor_ادھار + بالٹی_LISTS) {
		بنانشانی صحيح idx = (بنانشانی صحيح)(expiry & بالٹی_نقاب);

		منزل->dticks = idx;
		بالٹی_بٹ |= بالٹی_ثنائی_ہندسہ(idx);
		sys_dlist_append(&بالٹی[idx], &منزل->گرہ);
	} وإلا {
		/* Overflow: keep sorted ascending by absolute expiry. Insert
		 * after any equal-keyed entries so same-tick order stays FIFO.
		 */
		بنية _timeout *t;

		منزل->dticks = (k_ticks_t)expiry;
		SYS_DLIST_FOR_EACH_CONTAINER(&حد_توں_ودھ, t, گرہ) {
			إذا ((uint64_t)t->dticks > expiry) {
				sys_dlist_داخل_کرو(&t->گرہ, &منزل->گرہ);
				أرجع (int64_t)dticks < old_gap;
			}
		}
		sys_dlist_append(&حد_توں_ودھ, &منزل->گرہ);
	}

	أرجع (int64_t)dticks < old_gap;
}

ساكن k_ticks_t z_timeout_q_remainder(ثابت بنية _timeout *منزل)
{
	إذا ((uint64_t)منزل->dticks < بالٹی_LISTS) {
		/* Bucket entry: reconstruct its absolute expiry, the unique
		 * tick in the window whose low bits are the bucket index.
		 */
		uint64_t off = ((uint64_t)منزل->dticks - cursor_ادھار) & بالٹی_نقاب;

		أرجع (k_ticks_t)((cursor_ادھار + off) - curr_tick);
	}

	أرجع (k_ticks_t)((uint64_t)منزل->dticks - curr_tick);
}

ساكن bool z_timeout_q_remove(بنية _timeout *منزل)
{
	bool was_پہلا = (z_timeout_q_remainder(منزل) <= earliest_gap());

	إذا ((uint64_t)منزل->dticks < بالٹی_LISTS) {
		بنانشانی صحيح idx = (بنانشانی صحيح)منزل->dticks;

		sys_dlist_remove(&منزل->گرہ);
		إذا (sys_dlist_is_خالی_تھاواں(&بالٹی[idx])) {
			بالٹی_بٹ &= ~بالٹی_ثنائی_ہندسہ(idx);
		}
	} وإلا {
		/* Overflow stores absolute expiry (not a delta), so removal
		 * needs no successor fix-up.
		 */
		sys_dlist_remove(&منزل->گرہ);
	}

	أرجع was_پہلا;
}

/* Ticks from curr_tick to the earliest pending timeout, or K_TICKS_FOREVER if
 * none. The announce-range cap is applied centrally by next_timeout() in
 * timeout.c.
 */
ساكن k_ticks_t z_timeout_q_اگلا_expiry(فراغ)
{
	int64_t gap = earliest_gap();

	أرجع (gap == INT64_MAX) ? K_TICKS_FOREVER : (k_ticks_t)gap;
}

ساكن int32_t z_timeout_q_اگلا_gap(فراغ)
{
	int64_t gap = earliest_gap();

	أرجع (gap == INT64_MAX) ? INT32_MAX : (int32_t)MIN(gap, (int64_t)INT32_MAX);
}

ساكن فراغ z_timeout_q_advance(int32_t dt)
{
	uint64_t new_curr = curr_tick + (uint64_t)dt;

	إذا (new_curr < cursor_ادھار + بالٹی_LISTS) {
		/* Still inside the current window: nothing to migrate. */
		أرجع;
	}

	/* Crossed the window. The loop only advances past the boundary when no
	 * bucket lies ahead (next_gap then came from overflow), so every bucket
	 * is empty here. Re-base the window onto curr_tick's new position and
	 * migrate the overflow entries that have come within range.
	 */
	cursor_ادھار = new_curr & ~(uint64_t)بالٹی_نقاب;
	بالٹی_بٹ = 0U;

	بنية _timeout *t;

	طالما ((t = حد_توں_ودھ_سر()) != عدم &&
	       (uint64_t)t->dticks < cursor_ادھار + بالٹی_LISTS) {
		بنانشانی صحيح idx = (بنانشانی صحيح)((uint64_t)t->dticks & بالٹی_نقاب);

		sys_dlist_remove(&t->گرہ);
		t->dticks = idx;
		بالٹی_بٹ |= بالٹی_ثنائی_ہندسہ(idx);
		sys_dlist_append(&بالٹی[idx], &t->گرہ);
	}
}

ساكن بنية _timeout *z_timeout_q_نکالو_due(فراغ)
{
	بنانشانی صحيح idx = (بنانشانی صحيح)(curr_tick & بالٹی_نقاب);
	sys_dnode_t *n = sys_dlist_peek_سر(&بالٹی[idx]);
	بنية _timeout *t;

	إذا (n == عدم) {
		أرجع عدم;
	}

	/* All entries in bucket[curr_tick & MASK] expire exactly at curr_tick. */
	t = CONTAINER_دا(n, بنية _timeout, گرہ);
	sys_dlist_remove(&t->گرہ);
	إذا (sys_dlist_is_خالی_تھاواں(&بالٹی[idx])) {
		بالٹی_بٹ &= ~بالٹی_ثنائی_ہندسہ(idx);
	}

	أرجع t;
}

#جے_انت /* ZEPHYR_KERNEL_TIMEOUT_BUCKET_H_ */
