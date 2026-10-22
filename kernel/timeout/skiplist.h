/*
 * Copyright (c) 2026 Dhruv Menon <dhruvmenon1104@gmail.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_KERNEL_TIMEOUT_SKIPLIST_H_
#define ZEPHYR_KERNEL_TIMEOUT_SKIPLIST_H_

/**
 * @file
 * @brief Skip-list timeout backend (implementation).
 *
 * Pending timeouts are kept in a Pugh skip list keyed on absolute expiry
 * tick (struct _timeout's abs_ticks). Expected insertion and arbitrary
 * removal are O(log n); the earliest timeout is always the level-0 successor
 * of the sentinel. A timeout that is not queued has height == 0.
 *
 * Same-tick firing order is FIFO: a new node is inserted after every node
 * that already has the same abs_ticks.
 *
 * Node height is drawn from a geometric(1/2) distribution using a private
 * xorshift32, so the announce/add/abort paths never call the entropy driver.
 * There is no capacity limit (unlike the min-heap backend).
 */

#define SKIPLIST_LEVELS CONFIG_TIMEOUT_SKIPLIST_MAX_LEVEL

/* Sentinel: never expires, participates in every level, never fired. */
স্থির সংরচনা _timeout sl_head = {
	.height = SKIPLIST_LEVELS,
	.forward = { রিক্ত },
};

/* Highest height of any currently queued node; 0 when the list is empty. */
স্থির uint8_t sl_শীর্ষ;

/*
 * Scratch predecessor vector shared by insert and remove. Both run with
 * timeout_lock held and neither keeps it across an unlock, so one array is
 * enough. Any future caller must hold that lock too.
 */
স্থির সংরচনা _timeout *sl_update[SKIPLIST_LEVELS];

/* Marsaglia xorshift32 state; period 2^32-1, must stay non-zero. */
স্থির uint32_t sl_rng = 2463534242U;

স্থির uint8_t skiplist_random_height(শূন্য)
{
	uint32_t r = sl_rng;

	r ^= r << 13;
	r ^= r >> 17;
	r ^= r << 5;
	sl_rng = r;

	/* p = 1/2: height is 1 + the run of low 1-bits, capped at SKIPLIST_LEVELS. */
	প্রত্যাগম 1 + MIN(u32_গুনতি_trailing_zeros(~r), SKIPLIST_LEVELS - 1);
}

স্থির অন্তঃস্থ সংরচনা _timeout *skiplist_first(শূন্য)
{
	প্রত্যাগম sl_head.forward[0];
}

স্থির শূন্য skiplist_drop_শীর্ষ(শূন্য)
{
	যাবত্ (sl_শীর্ষ > 0 && sl_head.forward[sl_শীর্ষ - 1] == রিক্ত) {
		sl_শীর্ষ--;
	}
}

স্থির শূন্য skiplist_update_init(সংরচনা _timeout **update)
{
	uint8_t i;

	প্রতি (i = 0; i < SKIPLIST_LEVELS; i++) {
		update[i] = &sl_head;
	}
}

/* Splice @to out at every level it occupies. */
স্থির শূন্য skiplist_unlink(সংরচনা _timeout *to, সংরচনা _timeout **update)
{
	uint8_t i;
	uint8_t height = to->height;

	প্রতি (i = 0; i < height; i++) {
		__ASSERT_NO_MSG(update[i]->forward[i] == to);
		update[i]->forward[i] = to->forward[i];
	}

	to->height = 0;
	skiplist_drop_শীর্ষ();
}

/* Equal keys are skipped so a later insert lands after them (same-tick FIFO). */
স্থির শূন্য skiplist_predecessors_after(int64_t abs_ticks, সংরচনা _timeout **update)
{
	সংরচনা _timeout *x = &sl_head;
	পূর্ণাঙ্ক i;

	প্রতি (i = (পূর্ণাঙ্ক)sl_শীর্ষ - 1; i >= 0; i--) {
		যাবত্ (x->forward[i] != রিক্ত && x->forward[i]->abs_ticks <= abs_ticks) {
			x = x->forward[i];
		}
		update[i] = x;
	}
}

/* Walk equal-key nodes so identity, not just expiry, selects the splice points. */
স্থির শূন্য skiplist_predecessors_of(সংরচনা _timeout *to, সংরচনা _timeout **update)
{
	সংরচনা _timeout *x = &sl_head;
	পূর্ণাঙ্ক i;

	প্রতি (i = (পূর্ণাঙ্ক)sl_শীর্ষ - 1; i >= 0; i--) {
		যাবত্ (x->forward[i] != রিক্ত && x->forward[i]->abs_ticks < to->abs_ticks) {
			x = x->forward[i];
		}
		update[i] = x;
	}

	প্রতি (i = 0; i < to->height; i++) {
		যাবত্ (update[i]->forward[i] != রিক্ত &&
		       update[i]->forward[i] != to &&
		       update[i]->forward[i]->abs_ticks == to->abs_ticks) {
			update[i] = update[i]->forward[i];
		}
	}
}

স্থির অন্তঃস্থ bool z_timeout_q_ঢোকাও(সংরচনা _timeout *to, k_ticks_t dticks)
{
	uint8_t i;
	uint8_t height;

	to->abs_ticks = (int64_t)curr_tick + dticks;
	height = skiplist_random_height();
	to->height = height;

	skiplist_update_init(sl_update);
	skiplist_predecessors_after(to->abs_ticks, sl_update);

	যদি (height > sl_শীর্ষ) {
		sl_শীর্ষ = height;
	}

	প্রতি (i = 0; i < height; i++) {
		to->forward[i] = sl_update[i]->forward[i];
		sl_update[i]->forward[i] = to;
	}

	প্রত্যাগম skiplist_first() == to;
}

স্থির অন্তঃস্থ bool z_timeout_q_remove(সংরচনা _timeout *to)
{
	bool was_first = (skiplist_first() == to);

	__ASSERT_NO_MSG(to->height > 0);

	skiplist_update_init(sl_update);
	skiplist_predecessors_of(to, sl_update);

	skiplist_unlink(to, sl_update);

	প্রত্যাগম was_first;
}

স্থির অন্তঃস্থ k_ticks_t z_timeout_q_remainder(ধ্রুব সংরচনা _timeout *to)
{
	প্রত্যাগম (k_ticks_t)(to->abs_ticks - (int64_t)curr_tick);
}

স্থির অন্তঃস্থ k_ticks_t z_timeout_q_next_expiry(শূন্য)
{
	সংরচনা _timeout *t = skiplist_first();
	int64_t gap;

	যদি (t == রিক্ত) {
		প্রত্যাগম K_TICKS_FOREVER;
	}

	gap = t->abs_ticks - (int64_t)curr_tick;
	প্রত্যাগম (gap < 0) ? 0 : (k_ticks_t)gap;
}

স্থির অন্তঃস্থ int32_t z_timeout_q_next_gap(শূন্য)
{
	সংরচনা _timeout *t = skiplist_first();
	int64_t gap;

	যদি (t == রিক্ত) {
		প্রত্যাগম INT32_MAX;
	}

	/* Overdue must be 0: announce casts dt to uint32_t. */
	gap = t->abs_ticks - (int64_t)curr_tick;
	যদি (gap <= 0) {
		প্রত্যাগম 0;
	}

	প্রত্যাগম (int32_t)MIN(gap, (int64_t)INT32_MAX);
}

স্থির অন্তঃস্থ শূন্য z_timeout_q_advance(int32_t dt)
{
	ARG_UNUSED(dt);
}

/* Earliest node, so it is the head's successor at every level it occupies. */
স্থির অন্তঃস্থ সংরচনা _timeout *z_timeout_q_বের_করো_due(শূন্য)
{
	সংরচনা _timeout *t = skiplist_first();
	uint8_t i;

	যদি ((t == রিক্ত) || (t->abs_ticks > (int64_t)curr_tick)) {
		প্রত্যাগম রিক্ত;
	}

	প্রতি (i = 0; i < t->height; i++) {
		__ASSERT_NO_MSG(sl_head.forward[i] == t);
		sl_head.forward[i] = t->forward[i];
	}

	t->height = 0;
	skiplist_drop_শীর্ষ();

	প্রত্যাগম t;
}

#endif /* ZEPHYR_KERNEL_TIMEOUT_SKIPLIST_H_ */
