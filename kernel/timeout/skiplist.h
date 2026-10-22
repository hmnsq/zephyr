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
ایستا ساختار _timeout sl_head = {
	.height = SKIPLIST_LEVELS,
	.forward = { پوچ },
};

/* Highest height of any currently queued node; 0 when the list is empty. */
ایستا uint8_t sl_بالاترین;

/*
 * Scratch predecessor vector shared by insert and remove. Both run with
 * timeout_lock held and neither keeps it across an unlock, so one array is
 * enough. Any future caller must hold that lock too.
 */
ایستا ساختار _timeout *sl_update[SKIPLIST_LEVELS];

/* Marsaglia xorshift32 state; period 2^32-1, must stay non-zero. */
ایستا uint32_t sl_rng = 2463534242U;

ایستا uint8_t skiplist_random_height(تهی)
{
	uint32_t r = sl_rng;

	r ^= r << 13;
	r ^= r >> 17;
	r ^= r << 5;
	sl_rng = r;

	/* p = 1/2: height is 1 + the run of low 1-bits, capped at SKIPLIST_LEVELS. */
	بازگشت 1 + MIN(u32_شمار_trailing_zeros(~r), SKIPLIST_LEVELS - 1);
}

ایستا درونخطی ساختار _timeout *skiplist_first(تهی)
{
	بازگشت sl_head.forward[0];
}

ایستا تهی skiplist_drop_بالاترین(تهی)
{
	وقتی (sl_بالاترین > 0 && sl_head.forward[sl_بالاترین - 1] == پوچ) {
		sl_بالاترین--;
	}
}

ایستا تهی skiplist_update_init(ساختار _timeout **update)
{
	uint8_t i;

	برای (i = 0; i < SKIPLIST_LEVELS; i++) {
		update[i] = &sl_head;
	}
}

/* Splice @to out at every level it occupies. */
ایستا تهی skiplist_unlink(ساختار _timeout *to, ساختار _timeout **update)
{
	uint8_t i;
	uint8_t height = to->height;

	برای (i = 0; i < height; i++) {
		__ASSERT_NO_MSG(update[i]->forward[i] == to);
		update[i]->forward[i] = to->forward[i];
	}

	to->height = 0;
	skiplist_drop_بالاترین();
}

/* Equal keys are skipped so a later insert lands after them (same-tick FIFO). */
ایستا تهی skiplist_predecessors_after(int64_t abs_ticks, ساختار _timeout **update)
{
	ساختار _timeout *x = &sl_head;
	صحیح i;

	برای (i = (صحیح)sl_بالاترین - 1; i >= 0; i--) {
		وقتی (x->forward[i] != پوچ && x->forward[i]->abs_ticks <= abs_ticks) {
			x = x->forward[i];
		}
		update[i] = x;
	}
}

/* Walk equal-key nodes so identity, not just expiry, selects the splice points. */
ایستا تهی skiplist_predecessors_of(ساختار _timeout *to, ساختار _timeout **update)
{
	ساختار _timeout *x = &sl_head;
	صحیح i;

	برای (i = (صحیح)sl_بالاترین - 1; i >= 0; i--) {
		وقتی (x->forward[i] != پوچ && x->forward[i]->abs_ticks < to->abs_ticks) {
			x = x->forward[i];
		}
		update[i] = x;
	}

	برای (i = 0; i < to->height; i++) {
		وقتی (update[i]->forward[i] != پوچ &&
		       update[i]->forward[i] != to &&
		       update[i]->forward[i]->abs_ticks == to->abs_ticks) {
			update[i] = update[i]->forward[i];
		}
	}
}

ایستا درونخطی bool z_timeout_q_درج(ساختار _timeout *to, k_ticks_t dticks)
{
	uint8_t i;
	uint8_t height;

	to->abs_ticks = (int64_t)curr_tick + dticks;
	height = skiplist_random_height();
	to->height = height;

	skiplist_update_init(sl_update);
	skiplist_predecessors_after(to->abs_ticks, sl_update);

	اگر (height > sl_بالاترین) {
		sl_بالاترین = height;
	}

	برای (i = 0; i < height; i++) {
		to->forward[i] = sl_update[i]->forward[i];
		sl_update[i]->forward[i] = to;
	}

	بازگشت skiplist_first() == to;
}

ایستا درونخطی bool z_timeout_q_remove(ساختار _timeout *to)
{
	bool was_first = (skiplist_first() == to);

	__ASSERT_NO_MSG(to->height > 0);

	skiplist_update_init(sl_update);
	skiplist_predecessors_of(to, sl_update);

	skiplist_unlink(to, sl_update);

	بازگشت was_first;
}

ایستا درونخطی k_ticks_t z_timeout_q_remainder(ثابت ساختار _timeout *to)
{
	بازگشت (k_ticks_t)(to->abs_ticks - (int64_t)curr_tick);
}

ایستا درونخطی k_ticks_t z_timeout_q_next_expiry(تهی)
{
	ساختار _timeout *t = skiplist_first();
	int64_t gap;

	اگر (t == پوچ) {
		بازگشت K_TICKS_FOREVER;
	}

	gap = t->abs_ticks - (int64_t)curr_tick;
	بازگشت (gap < 0) ? 0 : (k_ticks_t)gap;
}

ایستا درونخطی int32_t z_timeout_q_next_gap(تهی)
{
	ساختار _timeout *t = skiplist_first();
	int64_t gap;

	اگر (t == پوچ) {
		بازگشت INT32_MAX;
	}

	/* Overdue must be 0: announce casts dt to uint32_t. */
	gap = t->abs_ticks - (int64_t)curr_tick;
	اگر (gap <= 0) {
		بازگشت 0;
	}

	بازگشت (int32_t)MIN(gap, (int64_t)INT32_MAX);
}

ایستا درونخطی تهی z_timeout_q_advance(int32_t dt)
{
	ARG_UNUSED(dt);
}

/* Earliest node, so it is the head's successor at every level it occupies. */
ایستا درونخطی ساختار _timeout *z_timeout_q_برداشت_due(تهی)
{
	ساختار _timeout *t = skiplist_first();
	uint8_t i;

	اگر ((t == پوچ) || (t->abs_ticks > (int64_t)curr_tick)) {
		بازگشت پوچ;
	}

	برای (i = 0; i < t->height; i++) {
		__ASSERT_NO_MSG(sl_head.forward[i] == t);
		sl_head.forward[i] = t->forward[i];
	}

	t->height = 0;
	skiplist_drop_بالاترین();

	بازگشت t;
}

#endif /* ZEPHYR_KERNEL_TIMEOUT_SKIPLIST_H_ */
