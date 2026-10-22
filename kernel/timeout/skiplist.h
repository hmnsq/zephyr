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
स्थिर संरचना _timeout sl_head = {
	.height = SKIPLIST_LEVELS,
	.forward = { रिक्त },
};

/* Highest height of any currently queued node; 0 when the list is empty. */
स्थिर uint8_t sl_शीर्ष;

/*
 * Scratch predecessor vector shared by insert and remove. Both run with
 * timeout_lock held and neither keeps it across an unlock, so one array is
 * enough. Any future caller must hold that lock too.
 */
स्थिर संरचना _timeout *sl_update[SKIPLIST_LEVELS];

/* Marsaglia xorshift32 state; period 2^32-1, must stay non-zero. */
स्थिर uint32_t sl_rng = 2463534242U;

स्थिर uint8_t skiplist_random_height(शून्य)
{
	uint32_t r = sl_rng;

	r ^= r << 13;
	r ^= r >> 17;
	r ^= r << 5;
	sl_rng = r;

	/* p = 1/2: height is 1 + the run of low 1-bits, capped at SKIPLIST_LEVELS. */
	परत 1 + MIN(u32_मोजणी_trailing_zeros(~r), SKIPLIST_LEVELS - 1);
}

स्थिर ओळीत संरचना _timeout *skiplist_first(शून्य)
{
	परत sl_head.forward[0];
}

स्थिर शून्य skiplist_drop_शीर्ष(शून्य)
{
	पर्यंत (sl_शीर्ष > 0 && sl_head.forward[sl_शीर्ष - 1] == रिक्त) {
		sl_शीर्ष--;
	}
}

स्थिर शून्य skiplist_update_init(संरचना _timeout **update)
{
	uint8_t i;

	साठी (i = 0; i < SKIPLIST_LEVELS; i++) {
		update[i] = &sl_head;
	}
}

/* Splice @to out at every level it occupies. */
स्थिर शून्य skiplist_unlink(संरचना _timeout *to, संरचना _timeout **update)
{
	uint8_t i;
	uint8_t height = to->height;

	साठी (i = 0; i < height; i++) {
		__ASSERT_NO_MSG(update[i]->forward[i] == to);
		update[i]->forward[i] = to->forward[i];
	}

	to->height = 0;
	skiplist_drop_शीर्ष();
}

/* Equal keys are skipped so a later insert lands after them (same-tick FIFO). */
स्थिर शून्य skiplist_predecessors_after(int64_t abs_ticks, संरचना _timeout **update)
{
	संरचना _timeout *x = &sl_head;
	पूर्णांक i;

	साठी (i = (पूर्णांक)sl_शीर्ष - 1; i >= 0; i--) {
		पर्यंत (x->forward[i] != रिक्त && x->forward[i]->abs_ticks <= abs_ticks) {
			x = x->forward[i];
		}
		update[i] = x;
	}
}

/* Walk equal-key nodes so identity, not just expiry, selects the splice points. */
स्थिर शून्य skiplist_predecessors_of(संरचना _timeout *to, संरचना _timeout **update)
{
	संरचना _timeout *x = &sl_head;
	पूर्णांक i;

	साठी (i = (पूर्णांक)sl_शीर्ष - 1; i >= 0; i--) {
		पर्यंत (x->forward[i] != रिक्त && x->forward[i]->abs_ticks < to->abs_ticks) {
			x = x->forward[i];
		}
		update[i] = x;
	}

	साठी (i = 0; i < to->height; i++) {
		पर्यंत (update[i]->forward[i] != रिक्त &&
		       update[i]->forward[i] != to &&
		       update[i]->forward[i]->abs_ticks == to->abs_ticks) {
			update[i] = update[i]->forward[i];
		}
	}
}

स्थिर ओळीत bool z_timeout_q_घाला(संरचना _timeout *to, k_ticks_t dticks)
{
	uint8_t i;
	uint8_t height;

	to->abs_ticks = (int64_t)curr_tick + dticks;
	height = skiplist_random_height();
	to->height = height;

	skiplist_update_init(sl_update);
	skiplist_predecessors_after(to->abs_ticks, sl_update);

	जर (height > sl_शीर्ष) {
		sl_शीर्ष = height;
	}

	साठी (i = 0; i < height; i++) {
		to->forward[i] = sl_update[i]->forward[i];
		sl_update[i]->forward[i] = to;
	}

	परत skiplist_first() == to;
}

स्थिर ओळीत bool z_timeout_q_remove(संरचना _timeout *to)
{
	bool was_first = (skiplist_first() == to);

	__ASSERT_NO_MSG(to->height > 0);

	skiplist_update_init(sl_update);
	skiplist_predecessors_of(to, sl_update);

	skiplist_unlink(to, sl_update);

	परत was_first;
}

स्थिर ओळीत k_ticks_t z_timeout_q_remainder(स्थिरांक संरचना _timeout *to)
{
	परत (k_ticks_t)(to->abs_ticks - (int64_t)curr_tick);
}

स्थिर ओळीत k_ticks_t z_timeout_q_next_expiry(शून्य)
{
	संरचना _timeout *t = skiplist_first();
	int64_t gap;

	जर (t == रिक्त) {
		परत K_TICKS_FOREVER;
	}

	gap = t->abs_ticks - (int64_t)curr_tick;
	परत (gap < 0) ? 0 : (k_ticks_t)gap;
}

स्थिर ओळीत int32_t z_timeout_q_next_gap(शून्य)
{
	संरचना _timeout *t = skiplist_first();
	int64_t gap;

	जर (t == रिक्त) {
		परत INT32_MAX;
	}

	/* Overdue must be 0: announce casts dt to uint32_t. */
	gap = t->abs_ticks - (int64_t)curr_tick;
	जर (gap <= 0) {
		परत 0;
	}

	परत (int32_t)MIN(gap, (int64_t)INT32_MAX);
}

स्थिर ओळीत शून्य z_timeout_q_advance(int32_t dt)
{
	ARG_UNUSED(dt);
}

/* Earliest node, so it is the head's successor at every level it occupies. */
स्थिर ओळीत संरचना _timeout *z_timeout_q_काढा_due(शून्य)
{
	संरचना _timeout *t = skiplist_first();
	uint8_t i;

	जर ((t == रिक्त) || (t->abs_ticks > (int64_t)curr_tick)) {
		परत रिक्त;
	}

	साठी (i = 0; i < t->height; i++) {
		__ASSERT_NO_MSG(sl_head.forward[i] == t);
		sl_head.forward[i] = t->forward[i];
	}

	t->height = 0;
	skiplist_drop_शीर्ष();

	परत t;
}

#endif /* ZEPHYR_KERNEL_TIMEOUT_SKIPLIST_H_ */
