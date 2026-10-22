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
ସ୍ଥିର ସଂରଚନା _timeout sl_head = {
	.height = SKIPLIST_LEVELS,
	.forward = { ଖାଲି },
};

/* Highest height of any currently queued node; 0 when the list is empty. */
ସ୍ଥିର uint8_t sl_ଶୀର୍ଷ;

/*
 * Scratch predecessor vector shared by insert and remove. Both run with
 * timeout_lock held and neither keeps it across an unlock, so one array is
 * enough. Any future caller must hold that lock too.
 */
ସ୍ଥିର ସଂରଚନା _timeout *sl_update[SKIPLIST_LEVELS];

/* Marsaglia xorshift32 state; period 2^32-1, must stay non-zero. */
ସ୍ଥିର uint32_t sl_rng = 2463534242U;

ସ୍ଥିର uint8_t skiplist_random_height(ଶୂନ୍ୟ)
{
	uint32_t r = sl_rng;

	r ^= r << 13;
	r ^= r >> 17;
	r ^= r << 5;
	sl_rng = r;

	/* p = 1/2: height is 1 + the run of low 1-bits, capped at SKIPLIST_LEVELS. */
	ଫେରନ୍ତୁ 1 + MIN(u32_ଗଣନା_trailing_zeros(~r), SKIPLIST_LEVELS - 1);
}

ସ୍ଥିର ଧାଡ଼ିଭିତରେ ସଂରଚନା _timeout *skiplist_first(ଶୂନ୍ୟ)
{
	ଫେରନ୍ତୁ sl_head.forward[0];
}

ସ୍ଥିର ଶୂନ୍ୟ skiplist_drop_ଶୀର୍ଷ(ଶୂନ୍ୟ)
{
	ଯେତେବେଳେ (sl_ଶୀର୍ଷ > 0 && sl_head.forward[sl_ଶୀର୍ଷ - 1] == ଖାଲି) {
		sl_ଶୀର୍ଷ--;
	}
}

ସ୍ଥିର ଶୂନ୍ୟ skiplist_update_init(ସଂରଚନା _timeout **update)
{
	uint8_t i;

	ପାଇଁ (i = 0; i < SKIPLIST_LEVELS; i++) {
		update[i] = &sl_head;
	}
}

/* Splice @to out at every level it occupies. */
ସ୍ଥିର ଶୂନ୍ୟ skiplist_unlink(ସଂରଚନା _timeout *to, ସଂରଚନା _timeout **update)
{
	uint8_t i;
	uint8_t height = to->height;

	ପାଇଁ (i = 0; i < height; i++) {
		__ASSERT_NO_MSG(update[i]->forward[i] == to);
		update[i]->forward[i] = to->forward[i];
	}

	to->height = 0;
	skiplist_drop_ଶୀର୍ଷ();
}

/* Equal keys are skipped so a later insert lands after them (same-tick FIFO). */
ସ୍ଥିର ଶୂନ୍ୟ skiplist_predecessors_after(int64_t abs_ticks, ସଂରଚନା _timeout **update)
{
	ସଂରଚନା _timeout *x = &sl_head;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

	ପାଇଁ (i = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)sl_ଶୀର୍ଷ - 1; i >= 0; i--) {
		ଯେତେବେଳେ (x->forward[i] != ଖାଲି && x->forward[i]->abs_ticks <= abs_ticks) {
			x = x->forward[i];
		}
		update[i] = x;
	}
}

/* Walk equal-key nodes so identity, not just expiry, selects the splice points. */
ସ୍ଥିର ଶୂନ୍ୟ skiplist_predecessors_of(ସଂରଚନା _timeout *to, ସଂରଚନା _timeout **update)
{
	ସଂରଚନା _timeout *x = &sl_head;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

	ପାଇଁ (i = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)sl_ଶୀର୍ଷ - 1; i >= 0; i--) {
		ଯେତେବେଳେ (x->forward[i] != ଖାଲି && x->forward[i]->abs_ticks < to->abs_ticks) {
			x = x->forward[i];
		}
		update[i] = x;
	}

	ପାଇଁ (i = 0; i < to->height; i++) {
		ଯେତେବେଳେ (update[i]->forward[i] != ଖାଲି &&
		       update[i]->forward[i] != to &&
		       update[i]->forward[i]->abs_ticks == to->abs_ticks) {
			update[i] = update[i]->forward[i];
		}
	}
}

ସ୍ଥିର ଧାଡ଼ିଭିତରେ bool z_timeout_q_ଭର୍ତ୍ତି(ସଂରଚନା _timeout *to, k_ticks_t dticks)
{
	uint8_t i;
	uint8_t height;

	to->abs_ticks = (int64_t)curr_tick + dticks;
	height = skiplist_random_height();
	to->height = height;

	skiplist_update_init(sl_update);
	skiplist_predecessors_after(to->abs_ticks, sl_update);

	ଯଦି (height > sl_ଶୀର୍ଷ) {
		sl_ଶୀର୍ଷ = height;
	}

	ପାଇଁ (i = 0; i < height; i++) {
		to->forward[i] = sl_update[i]->forward[i];
		sl_update[i]->forward[i] = to;
	}

	ଫେରନ୍ତୁ skiplist_first() == to;
}

ସ୍ଥିର ଧାଡ଼ିଭିତରେ bool z_timeout_q_remove(ସଂରଚନା _timeout *to)
{
	bool was_first = (skiplist_first() == to);

	__ASSERT_NO_MSG(to->height > 0);

	skiplist_update_init(sl_update);
	skiplist_predecessors_of(to, sl_update);

	skiplist_unlink(to, sl_update);

	ଫେରନ୍ତୁ was_first;
}

ସ୍ଥିର ଧାଡ଼ିଭିତରେ k_ticks_t z_timeout_q_remainder(ଅଚଳ ସଂରଚନା _timeout *to)
{
	ଫେରନ୍ତୁ (k_ticks_t)(to->abs_ticks - (int64_t)curr_tick);
}

ସ୍ଥିର ଧାଡ଼ିଭିତରେ k_ticks_t z_timeout_q_next_expiry(ଶୂନ୍ୟ)
{
	ସଂରଚନା _timeout *t = skiplist_first();
	int64_t gap;

	ଯଦି (t == ଖାଲି) {
		ଫେରନ୍ତୁ K_TICKS_FOREVER;
	}

	gap = t->abs_ticks - (int64_t)curr_tick;
	ଫେରନ୍ତୁ (gap < 0) ? 0 : (k_ticks_t)gap;
}

ସ୍ଥିର ଧାଡ଼ିଭିତରେ int32_t z_timeout_q_next_gap(ଶୂନ୍ୟ)
{
	ସଂରଚନା _timeout *t = skiplist_first();
	int64_t gap;

	ଯଦି (t == ଖାଲି) {
		ଫେରନ୍ତୁ INT32_MAX;
	}

	/* Overdue must be 0: announce casts dt to uint32_t. */
	gap = t->abs_ticks - (int64_t)curr_tick;
	ଯଦି (gap <= 0) {
		ଫେରନ୍ତୁ 0;
	}

	ଫେରନ୍ତୁ (int32_t)MIN(gap, (int64_t)INT32_MAX);
}

ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ z_timeout_q_advance(int32_t dt)
{
	ARG_UNUSED(dt);
}

/* Earliest node, so it is the head's successor at every level it occupies. */
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ସଂରଚନା _timeout *z_timeout_q_କାଢ଼_due(ଶୂନ୍ୟ)
{
	ସଂରଚନା _timeout *t = skiplist_first();
	uint8_t i;

	ଯଦି ((t == ଖାଲି) || (t->abs_ticks > (int64_t)curr_tick)) {
		ଫେରନ୍ତୁ ଖାଲି;
	}

	ପାଇଁ (i = 0; i < t->height; i++) {
		__ASSERT_NO_MSG(sl_head.forward[i] == t);
		sl_head.forward[i] = t->forward[i];
	}

	t->height = 0;
	skiplist_drop_ଶୀର୍ଷ();

	ଫେରନ୍ତୁ t;
}

#endif /* ZEPHYR_KERNEL_TIMEOUT_SKIPLIST_H_ */
