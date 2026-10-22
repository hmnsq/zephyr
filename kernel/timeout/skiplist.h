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
நிலையான கட்டமைப்பு _timeout sl_head = {
	.height = SKIPLIST_LEVELS,
	.forward = { வெற்று },
};

/* Highest height of any currently queued node; 0 when the list is empty. */
நிலையான uint8_t sl_உச்சி;

/*
 * Scratch predecessor vector shared by insert and remove. Both run with
 * timeout_lock held and neither keeps it across an unlock, so one array is
 * enough. Any future caller must hold that lock too.
 */
நிலையான கட்டமைப்பு _timeout *sl_update[SKIPLIST_LEVELS];

/* Marsaglia xorshift32 state; period 2^32-1, must stay non-zero. */
நிலையான uint32_t sl_rng = 2463534242U;

நிலையான uint8_t skiplist_random_height(வெறுமை)
{
	uint32_t r = sl_rng;

	r ^= r << 13;
	r ^= r >> 17;
	r ^= r << 5;
	sl_rng = r;

	/* p = 1/2: height is 1 + the run of low 1-bits, capped at SKIPLIST_LEVELS. */
	திரும்பு 1 + MIN(u32_மொத்தம்_trailing_zeros(~r), SKIPLIST_LEVELS - 1);
}

நிலையான உள்வரி கட்டமைப்பு _timeout *skiplist_first(வெறுமை)
{
	திரும்பு sl_head.forward[0];
}

நிலையான வெறுமை skiplist_drop_உச்சி(வெறுமை)
{
	போது (sl_உச்சி > 0 && sl_head.forward[sl_உச்சி - 1] == வெற்று) {
		sl_உச்சி--;
	}
}

நிலையான வெறுமை skiplist_update_init(கட்டமைப்பு _timeout **update)
{
	uint8_t i;

	க்கு (i = 0; i < SKIPLIST_LEVELS; i++) {
		update[i] = &sl_head;
	}
}

/* Splice @to out at every level it occupies. */
நிலையான வெறுமை skiplist_unlink(கட்டமைப்பு _timeout *to, கட்டமைப்பு _timeout **update)
{
	uint8_t i;
	uint8_t height = to->height;

	க்கு (i = 0; i < height; i++) {
		__ASSERT_NO_MSG(update[i]->forward[i] == to);
		update[i]->forward[i] = to->forward[i];
	}

	to->height = 0;
	skiplist_drop_உச்சி();
}

/* Equal keys are skipped so a later insert lands after them (same-tick FIFO). */
நிலையான வெறுமை skiplist_predecessors_after(int64_t abs_ticks, கட்டமைப்பு _timeout **update)
{
	கட்டமைப்பு _timeout *x = &sl_head;
	முழு i;

	க்கு (i = (முழு)sl_உச்சி - 1; i >= 0; i--) {
		போது (x->forward[i] != வெற்று && x->forward[i]->abs_ticks <= abs_ticks) {
			x = x->forward[i];
		}
		update[i] = x;
	}
}

/* Walk equal-key nodes so identity, not just expiry, selects the splice points. */
நிலையான வெறுமை skiplist_predecessors_of(கட்டமைப்பு _timeout *to, கட்டமைப்பு _timeout **update)
{
	கட்டமைப்பு _timeout *x = &sl_head;
	முழு i;

	க்கு (i = (முழு)sl_உச்சி - 1; i >= 0; i--) {
		போது (x->forward[i] != வெற்று && x->forward[i]->abs_ticks < to->abs_ticks) {
			x = x->forward[i];
		}
		update[i] = x;
	}

	க்கு (i = 0; i < to->height; i++) {
		போது (update[i]->forward[i] != வெற்று &&
		       update[i]->forward[i] != to &&
		       update[i]->forward[i]->abs_ticks == to->abs_ticks) {
			update[i] = update[i]->forward[i];
		}
	}
}

நிலையான உள்வரி bool z_timeout_q_செருகு(கட்டமைப்பு _timeout *to, k_ticks_t dticks)
{
	uint8_t i;
	uint8_t height;

	to->abs_ticks = (int64_t)curr_tick + dticks;
	height = skiplist_random_height();
	to->height = height;

	skiplist_update_init(sl_update);
	skiplist_predecessors_after(to->abs_ticks, sl_update);

	எனில் (height > sl_உச்சி) {
		sl_உச்சி = height;
	}

	க்கு (i = 0; i < height; i++) {
		to->forward[i] = sl_update[i]->forward[i];
		sl_update[i]->forward[i] = to;
	}

	திரும்பு skiplist_first() == to;
}

நிலையான உள்வரி bool z_timeout_q_remove(கட்டமைப்பு _timeout *to)
{
	bool was_first = (skiplist_first() == to);

	__ASSERT_NO_MSG(to->height > 0);

	skiplist_update_init(sl_update);
	skiplist_predecessors_of(to, sl_update);

	skiplist_unlink(to, sl_update);

	திரும்பு was_first;
}

நிலையான உள்வரி k_ticks_t z_timeout_q_remainder(மாறிலி கட்டமைப்பு _timeout *to)
{
	திரும்பு (k_ticks_t)(to->abs_ticks - (int64_t)curr_tick);
}

நிலையான உள்வரி k_ticks_t z_timeout_q_next_expiry(வெறுமை)
{
	கட்டமைப்பு _timeout *t = skiplist_first();
	int64_t gap;

	எனில் (t == வெற்று) {
		திரும்பு K_TICKS_FOREVER;
	}

	gap = t->abs_ticks - (int64_t)curr_tick;
	திரும்பு (gap < 0) ? 0 : (k_ticks_t)gap;
}

நிலையான உள்வரி int32_t z_timeout_q_next_gap(வெறுமை)
{
	கட்டமைப்பு _timeout *t = skiplist_first();
	int64_t gap;

	எனில் (t == வெற்று) {
		திரும்பு INT32_MAX;
	}

	/* Overdue must be 0: announce casts dt to uint32_t. */
	gap = t->abs_ticks - (int64_t)curr_tick;
	எனில் (gap <= 0) {
		திரும்பு 0;
	}

	திரும்பு (int32_t)MIN(gap, (int64_t)INT32_MAX);
}

நிலையான உள்வரி வெறுமை z_timeout_q_advance(int32_t dt)
{
	ARG_UNUSED(dt);
}

/* Earliest node, so it is the head's successor at every level it occupies. */
நிலையான உள்வரி கட்டமைப்பு _timeout *z_timeout_q_எடு_due(வெறுமை)
{
	கட்டமைப்பு _timeout *t = skiplist_first();
	uint8_t i;

	எனில் ((t == வெற்று) || (t->abs_ticks > (int64_t)curr_tick)) {
		திரும்பு வெற்று;
	}

	க்கு (i = 0; i < t->height; i++) {
		__ASSERT_NO_MSG(sl_head.forward[i] == t);
		sl_head.forward[i] = t->forward[i];
	}

	t->height = 0;
	skiplist_drop_உச்சி();

	திரும்பு t;
}

#endif /* ZEPHYR_KERNEL_TIMEOUT_SKIPLIST_H_ */
