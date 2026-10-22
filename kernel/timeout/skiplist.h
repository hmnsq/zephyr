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
ಸ್ಥಿರ ಸಂರಚನಾ _timeout sl_head = {
	.height = SKIPLIST_LEVELS,
	.forward = { ರಿಕ್ತ },
};

/* Highest height of any currently queued node; 0 when the list is empty. */
ಸ್ಥಿರ uint8_t sl_ತುದಿ;

/*
 * Scratch predecessor vector shared by insert and remove. Both run with
 * timeout_lock held and neither keeps it across an unlock, so one array is
 * enough. Any future caller must hold that lock too.
 */
ಸ್ಥಿರ ಸಂರಚನಾ _timeout *sl_update[SKIPLIST_LEVELS];

/* Marsaglia xorshift32 state; period 2^32-1, must stay non-zero. */
ಸ್ಥಿರ uint32_t sl_rng = 2463534242U;

ಸ್ಥಿರ uint8_t skiplist_random_height(ಶೂನ್ಯ)
{
	uint32_t r = sl_rng;

	r ^= r << 13;
	r ^= r >> 17;
	r ^= r << 5;
	sl_rng = r;

	/* p = 1/2: height is 1 + the run of low 1-bits, capped at SKIPLIST_LEVELS. */
	ಪ್ರತ್ಯಾಗಮ 1 + MIN(u32_ಎಣಿಕೆ_trailing_zeros(~r), SKIPLIST_LEVELS - 1);
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಸಂರಚನಾ _timeout *skiplist_first(ಶೂನ್ಯ)
{
	ಪ್ರತ್ಯಾಗಮ sl_head.forward[0];
}

ಸ್ಥಿರ ಶೂನ್ಯ skiplist_drop_ತುದಿ(ಶೂನ್ಯ)
{
	ಯಾವತ್ (sl_ತುದಿ > 0 && sl_head.forward[sl_ತುದಿ - 1] == ರಿಕ್ತ) {
		sl_ತುದಿ--;
	}
}

ಸ್ಥಿರ ಶೂನ್ಯ skiplist_update_init(ಸಂರಚನಾ _timeout **update)
{
	uint8_t i;

	ಪ್ರತಿ (i = 0; i < SKIPLIST_LEVELS; i++) {
		update[i] = &sl_head;
	}
}

/* Splice @to out at every level it occupies. */
ಸ್ಥಿರ ಶೂನ್ಯ skiplist_unlink(ಸಂರಚನಾ _timeout *to, ಸಂರಚನಾ _timeout **update)
{
	uint8_t i;
	uint8_t height = to->height;

	ಪ್ರತಿ (i = 0; i < height; i++) {
		__ASSERT_NO_MSG(update[i]->forward[i] == to);
		update[i]->forward[i] = to->forward[i];
	}

	to->height = 0;
	skiplist_drop_ತುದಿ();
}

/* Equal keys are skipped so a later insert lands after them (same-tick FIFO). */
ಸ್ಥಿರ ಶೂನ್ಯ skiplist_predecessors_after(int64_t abs_ticks, ಸಂರಚನಾ _timeout **update)
{
	ಸಂರಚನಾ _timeout *x = &sl_head;
	ಪೂರ್ಣಾಙ್ಕ i;

	ಪ್ರತಿ (i = (ಪೂರ್ಣಾಙ್ಕ)sl_ತುದಿ - 1; i >= 0; i--) {
		ಯಾವತ್ (x->forward[i] != ರಿಕ್ತ && x->forward[i]->abs_ticks <= abs_ticks) {
			x = x->forward[i];
		}
		update[i] = x;
	}
}

/* Walk equal-key nodes so identity, not just expiry, selects the splice points. */
ಸ್ಥಿರ ಶೂನ್ಯ skiplist_predecessors_of(ಸಂರಚನಾ _timeout *to, ಸಂರಚನಾ _timeout **update)
{
	ಸಂರಚನಾ _timeout *x = &sl_head;
	ಪೂರ್ಣಾಙ್ಕ i;

	ಪ್ರತಿ (i = (ಪೂರ್ಣಾಙ್ಕ)sl_ತುದಿ - 1; i >= 0; i--) {
		ಯಾವತ್ (x->forward[i] != ರಿಕ್ತ && x->forward[i]->abs_ticks < to->abs_ticks) {
			x = x->forward[i];
		}
		update[i] = x;
	}

	ಪ್ರತಿ (i = 0; i < to->height; i++) {
		ಯಾವತ್ (update[i]->forward[i] != ರಿಕ್ತ &&
		       update[i]->forward[i] != to &&
		       update[i]->forward[i]->abs_ticks == to->abs_ticks) {
			update[i] = update[i]->forward[i];
		}
	}
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ bool z_timeout_q_ಸೇರಿಸು(ಸಂರಚನಾ _timeout *to, k_ticks_t dticks)
{
	uint8_t i;
	uint8_t height;

	to->abs_ticks = (int64_t)curr_tick + dticks;
	height = skiplist_random_height();
	to->height = height;

	skiplist_update_init(sl_update);
	skiplist_predecessors_after(to->abs_ticks, sl_update);

	ಯದಿ (height > sl_ತುದಿ) {
		sl_ತುದಿ = height;
	}

	ಪ್ರತಿ (i = 0; i < height; i++) {
		to->forward[i] = sl_update[i]->forward[i];
		sl_update[i]->forward[i] = to;
	}

	ಪ್ರತ್ಯಾಗಮ skiplist_first() == to;
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ bool z_timeout_q_remove(ಸಂರಚನಾ _timeout *to)
{
	bool was_first = (skiplist_first() == to);

	__ASSERT_NO_MSG(to->height > 0);

	skiplist_update_init(sl_update);
	skiplist_predecessors_of(to, sl_update);

	skiplist_unlink(to, sl_update);

	ಪ್ರತ್ಯಾಗಮ was_first;
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ k_ticks_t z_timeout_q_remainder(ಧ್ರುವ ಸಂರಚನಾ _timeout *to)
{
	ಪ್ರತ್ಯಾಗಮ (k_ticks_t)(to->abs_ticks - (int64_t)curr_tick);
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ k_ticks_t z_timeout_q_next_expiry(ಶೂನ್ಯ)
{
	ಸಂರಚನಾ _timeout *t = skiplist_first();
	int64_t gap;

	ಯದಿ (t == ರಿಕ್ತ) {
		ಪ್ರತ್ಯಾಗಮ K_TICKS_FOREVER;
	}

	gap = t->abs_ticks - (int64_t)curr_tick;
	ಪ್ರತ್ಯಾಗಮ (gap < 0) ? 0 : (k_ticks_t)gap;
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ int32_t z_timeout_q_next_gap(ಶೂನ್ಯ)
{
	ಸಂರಚನಾ _timeout *t = skiplist_first();
	int64_t gap;

	ಯದಿ (t == ರಿಕ್ತ) {
		ಪ್ರತ್ಯಾಗಮ INT32_MAX;
	}

	/* Overdue must be 0: announce casts dt to uint32_t. */
	gap = t->abs_ticks - (int64_t)curr_tick;
	ಯದಿ (gap <= 0) {
		ಪ್ರತ್ಯಾಗಮ 0;
	}

	ಪ್ರತ್ಯಾಗಮ (int32_t)MIN(gap, (int64_t)INT32_MAX);
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ z_timeout_q_advance(int32_t dt)
{
	ARG_UNUSED(dt);
}

/* Earliest node, so it is the head's successor at every level it occupies. */
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಸಂರಚನಾ _timeout *z_timeout_q_ತೆಗೆ_due(ಶೂನ್ಯ)
{
	ಸಂರಚನಾ _timeout *t = skiplist_first();
	uint8_t i;

	ಯದಿ ((t == ರಿಕ್ತ) || (t->abs_ticks > (int64_t)curr_tick)) {
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}

	ಪ್ರತಿ (i = 0; i < t->height; i++) {
		__ASSERT_NO_MSG(sl_head.forward[i] == t);
		sl_head.forward[i] = t->forward[i];
	}

	t->height = 0;
	skiplist_drop_ತುದಿ();

	ಪ್ರತ್ಯಾಗಮ t;
}

#endif /* ZEPHYR_KERNEL_TIMEOUT_SKIPLIST_H_ */
