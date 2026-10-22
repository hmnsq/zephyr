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
સ્થિર સંરચના _timeout sl_head = {
	.height = SKIPLIST_LEVELS,
	.forward = { ખાલી },
};

/* Highest height of any currently queued node; 0 when the list is empty. */
સ્થિર uint8_t sl_ટોચ;

/*
 * Scratch predecessor vector shared by insert and remove. Both run with
 * timeout_lock held and neither keeps it across an unlock, so one array is
 * enough. Any future caller must hold that lock too.
 */
સ્થિર સંરચના _timeout *sl_update[SKIPLIST_LEVELS];

/* Marsaglia xorshift32 state; period 2^32-1, must stay non-zero. */
સ્થિર uint32_t sl_rng = 2463534242U;

સ્થિર uint8_t skiplist_random_height(શૂન્ય)
{
	uint32_t r = sl_rng;

	r ^= r << 13;
	r ^= r >> 17;
	r ^= r << 5;
	sl_rng = r;

	/* p = 1/2: height is 1 + the run of low 1-bits, capped at SKIPLIST_LEVELS. */
	પાછા 1 + MIN(u32_ગણતરી_trailing_zeros(~r), SKIPLIST_LEVELS - 1);
}

સ્થિર અંતઃરેખ સંરચના _timeout *skiplist_first(શૂન્ય)
{
	પાછા sl_head.forward[0];
}

સ્થિર શૂન્ય skiplist_drop_ટોચ(શૂન્ય)
{
	જ્યારે (sl_ટોચ > 0 && sl_head.forward[sl_ટોચ - 1] == ખાલી) {
		sl_ટોચ--;
	}
}

સ્થિર શૂન્ય skiplist_update_init(સંરચના _timeout **update)
{
	uint8_t i;

	માટે (i = 0; i < SKIPLIST_LEVELS; i++) {
		update[i] = &sl_head;
	}
}

/* Splice @to out at every level it occupies. */
સ્થિર શૂન્ય skiplist_unlink(સંરચના _timeout *to, સંરચના _timeout **update)
{
	uint8_t i;
	uint8_t height = to->height;

	માટે (i = 0; i < height; i++) {
		__ASSERT_NO_MSG(update[i]->forward[i] == to);
		update[i]->forward[i] = to->forward[i];
	}

	to->height = 0;
	skiplist_drop_ટોચ();
}

/* Equal keys are skipped so a later insert lands after them (same-tick FIFO). */
સ્થિર શૂન્ય skiplist_predecessors_after(int64_t abs_ticks, સંરચના _timeout **update)
{
	સંરચના _timeout *x = &sl_head;
	પૂર્ણાંક i;

	માટે (i = (પૂર્ણાંક)sl_ટોચ - 1; i >= 0; i--) {
		જ્યારે (x->forward[i] != ખાલી && x->forward[i]->abs_ticks <= abs_ticks) {
			x = x->forward[i];
		}
		update[i] = x;
	}
}

/* Walk equal-key nodes so identity, not just expiry, selects the splice points. */
સ્થિર શૂન્ય skiplist_predecessors_of(સંરચના _timeout *to, સંરચના _timeout **update)
{
	સંરચના _timeout *x = &sl_head;
	પૂર્ણાંક i;

	માટે (i = (પૂર્ણાંક)sl_ટોચ - 1; i >= 0; i--) {
		જ્યારે (x->forward[i] != ખાલી && x->forward[i]->abs_ticks < to->abs_ticks) {
			x = x->forward[i];
		}
		update[i] = x;
	}

	માટે (i = 0; i < to->height; i++) {
		જ્યારે (update[i]->forward[i] != ખાલી &&
		       update[i]->forward[i] != to &&
		       update[i]->forward[i]->abs_ticks == to->abs_ticks) {
			update[i] = update[i]->forward[i];
		}
	}
}

સ્થિર અંતઃરેખ bool z_timeout_q_ઉમેરો(સંરચના _timeout *to, k_ticks_t dticks)
{
	uint8_t i;
	uint8_t height;

	to->abs_ticks = (int64_t)curr_tick + dticks;
	height = skiplist_random_height();
	to->height = height;

	skiplist_update_init(sl_update);
	skiplist_predecessors_after(to->abs_ticks, sl_update);

	જો (height > sl_ટોચ) {
		sl_ટોચ = height;
	}

	માટે (i = 0; i < height; i++) {
		to->forward[i] = sl_update[i]->forward[i];
		sl_update[i]->forward[i] = to;
	}

	પાછા skiplist_first() == to;
}

સ્થિર અંતઃરેખ bool z_timeout_q_remove(સંરચના _timeout *to)
{
	bool was_first = (skiplist_first() == to);

	__ASSERT_NO_MSG(to->height > 0);

	skiplist_update_init(sl_update);
	skiplist_predecessors_of(to, sl_update);

	skiplist_unlink(to, sl_update);

	પાછા was_first;
}

સ્થિર અંતઃરેખ k_ticks_t z_timeout_q_remainder(અચલ સંરચના _timeout *to)
{
	પાછા (k_ticks_t)(to->abs_ticks - (int64_t)curr_tick);
}

સ્થિર અંતઃરેખ k_ticks_t z_timeout_q_next_expiry(શૂન્ય)
{
	સંરચના _timeout *t = skiplist_first();
	int64_t gap;

	જો (t == ખાલી) {
		પાછા K_TICKS_FOREVER;
	}

	gap = t->abs_ticks - (int64_t)curr_tick;
	પાછા (gap < 0) ? 0 : (k_ticks_t)gap;
}

સ્થિર અંતઃરેખ int32_t z_timeout_q_next_gap(શૂન્ય)
{
	સંરચના _timeout *t = skiplist_first();
	int64_t gap;

	જો (t == ખાલી) {
		પાછા INT32_MAX;
	}

	/* Overdue must be 0: announce casts dt to uint32_t. */
	gap = t->abs_ticks - (int64_t)curr_tick;
	જો (gap <= 0) {
		પાછા 0;
	}

	પાછા (int32_t)MIN(gap, (int64_t)INT32_MAX);
}

સ્થિર અંતઃરેખ શૂન્ય z_timeout_q_advance(int32_t dt)
{
	ARG_UNUSED(dt);
}

/* Earliest node, so it is the head's successor at every level it occupies. */
સ્થિર અંતઃરેખ સંરચના _timeout *z_timeout_q_કાઢો_due(શૂન્ય)
{
	સંરચના _timeout *t = skiplist_first();
	uint8_t i;

	જો ((t == ખાલી) || (t->abs_ticks > (int64_t)curr_tick)) {
		પાછા ખાલી;
	}

	માટે (i = 0; i < t->height; i++) {
		__ASSERT_NO_MSG(sl_head.forward[i] == t);
		sl_head.forward[i] = t->forward[i];
	}

	t->height = 0;
	skiplist_drop_ટોચ();

	પાછા t;
}

#endif /* ZEPHYR_KERNEL_TIMEOUT_SKIPLIST_H_ */
