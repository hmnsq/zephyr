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
സ്ഥിതം ഘടന _timeout sl_head = {
	.height = SKIPLIST_LEVELS,
	.forward = { ശൂന്യ },
};

/* Highest height of any currently queued node; 0 when the list is empty. */
സ്ഥിതം uint8_t sl_മുകൾ;

/*
 * Scratch predecessor vector shared by insert and remove. Both run with
 * timeout_lock held and neither keeps it across an unlock, so one array is
 * enough. Any future caller must hold that lock too.
 */
സ്ഥിതം ഘടന _timeout *sl_update[SKIPLIST_LEVELS];

/* Marsaglia xorshift32 state; period 2^32-1, must stay non-zero. */
സ്ഥിതം uint32_t sl_rng = 2463534242U;

സ്ഥിതം uint8_t skiplist_random_height(ശൂന്യം)
{
	uint32_t r = sl_rng;

	r ^= r << 13;
	r ^= r >> 17;
	r ^= r << 5;
	sl_rng = r;

	/* p = 1/2: height is 1 + the run of low 1-bits, capped at SKIPLIST_LEVELS. */
	തിരികെ 1 + MIN(u32_എണ്ണം_trailing_zeros(~r), SKIPLIST_LEVELS - 1);
}

സ്ഥിതം വരിയിൽ ഘടന _timeout *skiplist_first(ശൂന്യം)
{
	തിരികെ sl_head.forward[0];
}

സ്ഥിതം ശൂന്യം skiplist_drop_മുകൾ(ശൂന്യം)
{
	ആകുമ്പോൾ (sl_മുകൾ > 0 && sl_head.forward[sl_മുകൾ - 1] == ശൂന്യ) {
		sl_മുകൾ--;
	}
}

സ്ഥിതം ശൂന്യം skiplist_update_init(ഘടന _timeout **update)
{
	uint8_t i;

	വേണ്ടി (i = 0; i < SKIPLIST_LEVELS; i++) {
		update[i] = &sl_head;
	}
}

/* Splice @to out at every level it occupies. */
സ്ഥിതം ശൂന്യം skiplist_unlink(ഘടന _timeout *to, ഘടന _timeout **update)
{
	uint8_t i;
	uint8_t height = to->height;

	വേണ്ടി (i = 0; i < height; i++) {
		__ASSERT_NO_MSG(update[i]->forward[i] == to);
		update[i]->forward[i] = to->forward[i];
	}

	to->height = 0;
	skiplist_drop_മുകൾ();
}

/* Equal keys are skipped so a later insert lands after them (same-tick FIFO). */
സ്ഥിതം ശൂന്യം skiplist_predecessors_after(int64_t abs_ticks, ഘടന _timeout **update)
{
	ഘടന _timeout *x = &sl_head;
	പൂർണ്ണസംഖ്യ i;

	വേണ്ടി (i = (പൂർണ്ണസംഖ്യ)sl_മുകൾ - 1; i >= 0; i--) {
		ആകുമ്പോൾ (x->forward[i] != ശൂന്യ && x->forward[i]->abs_ticks <= abs_ticks) {
			x = x->forward[i];
		}
		update[i] = x;
	}
}

/* Walk equal-key nodes so identity, not just expiry, selects the splice points. */
സ്ഥിതം ശൂന്യം skiplist_predecessors_of(ഘടന _timeout *to, ഘടന _timeout **update)
{
	ഘടന _timeout *x = &sl_head;
	പൂർണ്ണസംഖ്യ i;

	വേണ്ടി (i = (പൂർണ്ണസംഖ്യ)sl_മുകൾ - 1; i >= 0; i--) {
		ആകുമ്പോൾ (x->forward[i] != ശൂന്യ && x->forward[i]->abs_ticks < to->abs_ticks) {
			x = x->forward[i];
		}
		update[i] = x;
	}

	വേണ്ടി (i = 0; i < to->height; i++) {
		ആകുമ്പോൾ (update[i]->forward[i] != ശൂന്യ &&
		       update[i]->forward[i] != to &&
		       update[i]->forward[i]->abs_ticks == to->abs_ticks) {
			update[i] = update[i]->forward[i];
		}
	}
}

സ്ഥിതം വരിയിൽ bool z_timeout_q_തിരുകുക(ഘടന _timeout *to, k_ticks_t dticks)
{
	uint8_t i;
	uint8_t height;

	to->abs_ticks = (int64_t)curr_tick + dticks;
	height = skiplist_random_height();
	to->height = height;

	skiplist_update_init(sl_update);
	skiplist_predecessors_after(to->abs_ticks, sl_update);

	എങ്കിൽ (height > sl_മുകൾ) {
		sl_മുകൾ = height;
	}

	വേണ്ടി (i = 0; i < height; i++) {
		to->forward[i] = sl_update[i]->forward[i];
		sl_update[i]->forward[i] = to;
	}

	തിരികെ skiplist_first() == to;
}

സ്ഥിതം വരിയിൽ bool z_timeout_q_remove(ഘടന _timeout *to)
{
	bool was_first = (skiplist_first() == to);

	__ASSERT_NO_MSG(to->height > 0);

	skiplist_update_init(sl_update);
	skiplist_predecessors_of(to, sl_update);

	skiplist_unlink(to, sl_update);

	തിരികെ was_first;
}

സ്ഥിതം വരിയിൽ k_ticks_t z_timeout_q_remainder(സ്ഥിരം ഘടന _timeout *to)
{
	തിരികെ (k_ticks_t)(to->abs_ticks - (int64_t)curr_tick);
}

സ്ഥിതം വരിയിൽ k_ticks_t z_timeout_q_next_expiry(ശൂന്യം)
{
	ഘടന _timeout *t = skiplist_first();
	int64_t gap;

	എങ്കിൽ (t == ശൂന്യ) {
		തിരികെ K_TICKS_FOREVER;
	}

	gap = t->abs_ticks - (int64_t)curr_tick;
	തിരികെ (gap < 0) ? 0 : (k_ticks_t)gap;
}

സ്ഥിതം വരിയിൽ int32_t z_timeout_q_next_gap(ശൂന്യം)
{
	ഘടന _timeout *t = skiplist_first();
	int64_t gap;

	എങ്കിൽ (t == ശൂന്യ) {
		തിരികെ INT32_MAX;
	}

	/* Overdue must be 0: announce casts dt to uint32_t. */
	gap = t->abs_ticks - (int64_t)curr_tick;
	എങ്കിൽ (gap <= 0) {
		തിരികെ 0;
	}

	തിരികെ (int32_t)MIN(gap, (int64_t)INT32_MAX);
}

സ്ഥിതം വരിയിൽ ശൂന്യം z_timeout_q_advance(int32_t dt)
{
	ARG_UNUSED(dt);
}

/* Earliest node, so it is the head's successor at every level it occupies. */
സ്ഥിതം വരിയിൽ ഘടന _timeout *z_timeout_q_എടുക്കുക_due(ശൂന്യം)
{
	ഘടന _timeout *t = skiplist_first();
	uint8_t i;

	എങ്കിൽ ((t == ശൂന്യ) || (t->abs_ticks > (int64_t)curr_tick)) {
		തിരികെ ശൂന്യ;
	}

	വേണ്ടി (i = 0; i < t->height; i++) {
		__ASSERT_NO_MSG(sl_head.forward[i] == t);
		sl_head.forward[i] = t->forward[i];
	}

	t->height = 0;
	skiplist_drop_മുകൾ();

	തിരികെ t;
}

#endif /* ZEPHYR_KERNEL_TIMEOUT_SKIPLIST_H_ */
