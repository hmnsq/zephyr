/*
 * Copyright (c) 2026 Dhruv Menon <dhruvmenon1104@gmail.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#اگر_غیر_تعریف_شدہ ZEPHYR_KERNEL_TIMEOUT_SKIPLIST_H_
#تعریف_کرو ZEPHYR_KERNEL_TIMEOUT_SKIPLIST_H_

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

#تعریف_کرو SKIPLIST_سطحیں CONFIG_TIMEOUT_SKIPLIST_MAX_LEVEL

/* Sentinel: never expires, participates in every level, never fired. */
ساکن ساخت _timeout sl_سر = {
	.اونچائی = SKIPLIST_سطحیں,
	.forward = { تہی },
};

/* Highest height of any currently queued node; 0 when the list is empty. */
ساکن uint8_t sl_چوٹی;

/*
 * Scratch predecessor vector shared by insert and remove. Both run with
 * timeout_lock held and neither keeps it across an unlock, so one array is
 * enough. Any future caller must hold that lock too.
 */
ساکن ساخت _timeout *sl_update[SKIPLIST_سطحیں];

/* Marsaglia xorshift32 state; period 2^32-1, must stay non-zero. */
ساکن uint32_t sl_rng = 2463534242U;

ساکن uint8_t skiplist_random_اونچائی(خالی)
{
	uint32_t r = sl_rng;

	r ^= r << 13;
	r ^= r >> 17;
	r ^= r << 5;
	sl_rng = r;

	/* p = 1/2: height is 1 + the run of low 1-bits, capped at SKIPLIST_LEVELS. */
	واپس 1 + MIN(u32_گنتی_trailing_zeros(~r), SKIPLIST_سطحیں - 1);
}

ساکن اندرونی ساخت _timeout *skiplist_پہلا(خالی)
{
	واپس sl_سر.forward[0];
}

ساکن خالی skiplist_drop_چوٹی(خالی)
{
	جب (sl_چوٹی > 0 && sl_سر.forward[sl_چوٹی - 1] == تہی) {
		sl_چوٹی--;
	}
}

ساکن خالی skiplist_update_init(ساخت _timeout **update)
{
	uint8_t i;

	برائے (i = 0; i < SKIPLIST_سطحیں; i++) {
		update[i] = &sl_سر;
	}
}

/* Splice @to out at every level it occupies. */
ساکن خالی skiplist_unlink(ساخت _timeout *مقصد, ساخت _timeout **update)
{
	uint8_t i;
	uint8_t اونچائی = مقصد->اونچائی;

	برائے (i = 0; i < اونچائی; i++) {
		__ASSERT_NO_MSG(update[i]->forward[i] == مقصد);
		update[i]->forward[i] = مقصد->forward[i];
	}

	مقصد->اونچائی = 0;
	skiplist_drop_چوٹی();
}

/* Equal keys are skipped so a later insert lands after them (same-tick FIFO). */
ساکن خالی skiplist_predecessors_after(int64_t abs_ticks, ساخت _timeout **update)
{
	ساخت _timeout *x = &sl_سر;
	صحیح i;

	برائے (i = (صحیح)sl_چوٹی - 1; i >= 0; i--) {
		جب (x->forward[i] != تہی && x->forward[i]->abs_ticks <= abs_ticks) {
			x = x->forward[i];
		}
		update[i] = x;
	}
}

/* Walk equal-key nodes so identity, not just expiry, selects the splice points. */
ساکن خالی skiplist_predecessors_کا(ساخت _timeout *مقصد, ساخت _timeout **update)
{
	ساخت _timeout *x = &sl_سر;
	صحیح i;

	برائے (i = (صحیح)sl_چوٹی - 1; i >= 0; i--) {
		جب (x->forward[i] != تہی && x->forward[i]->abs_ticks < مقصد->abs_ticks) {
			x = x->forward[i];
		}
		update[i] = x;
	}

	برائے (i = 0; i < مقصد->اونچائی; i++) {
		جب (update[i]->forward[i] != تہی &&
		       update[i]->forward[i] != مقصد &&
		       update[i]->forward[i]->abs_ticks == مقصد->abs_ticks) {
			update[i] = update[i]->forward[i];
		}
	}
}

ساکن اندرونی bool z_timeout_q_داخل_کرو(ساخت _timeout *مقصد, k_ticks_t dticks)
{
	uint8_t i;
	uint8_t اونچائی;

	مقصد->abs_ticks = (int64_t)curr_tick + dticks;
	اونچائی = skiplist_random_اونچائی();
	مقصد->اونچائی = اونچائی;

	skiplist_update_init(sl_update);
	skiplist_predecessors_after(مقصد->abs_ticks, sl_update);

	اگر (اونچائی > sl_چوٹی) {
		sl_چوٹی = اونچائی;
	}

	برائے (i = 0; i < اونچائی; i++) {
		مقصد->forward[i] = sl_update[i]->forward[i];
		sl_update[i]->forward[i] = مقصد;
	}

	واپس skiplist_پہلا() == مقصد;
}

ساکن اندرونی bool z_timeout_q_remove(ساخت _timeout *مقصد)
{
	bool was_پہلا = (skiplist_پہلا() == مقصد);

	__ASSERT_NO_MSG(مقصد->اونچائی > 0);

	skiplist_update_init(sl_update);
	skiplist_predecessors_کا(مقصد, sl_update);

	skiplist_unlink(مقصد, sl_update);

	واپس was_پہلا;
}

ساکن اندرونی k_ticks_t z_timeout_q_remainder(ثابت ساخت _timeout *مقصد)
{
	واپس (k_ticks_t)(مقصد->abs_ticks - (int64_t)curr_tick);
}

ساکن اندرونی k_ticks_t z_timeout_q_اگلا_expiry(خالی)
{
	ساخت _timeout *t = skiplist_پہلا();
	int64_t gap;

	اگر (t == تہی) {
		واپس K_TICKS_FOREVER;
	}

	gap = t->abs_ticks - (int64_t)curr_tick;
	واپس (gap < 0) ? 0 : (k_ticks_t)gap;
}

ساکن اندرونی int32_t z_timeout_q_اگلا_gap(خالی)
{
	ساخت _timeout *t = skiplist_پہلا();
	int64_t gap;

	اگر (t == تہی) {
		واپس INT32_MAX;
	}

	/* Overdue must be 0: announce casts dt to uint32_t. */
	gap = t->abs_ticks - (int64_t)curr_tick;
	اگر (gap <= 0) {
		واپس 0;
	}

	واپس (int32_t)MIN(gap, (int64_t)INT32_MAX);
}

ساکن اندرونی خالی z_timeout_q_advance(int32_t dt)
{
	ARG_غیر_مستعمل(dt);
}

/* Earliest node, so it is the head's successor at every level it occupies. */
ساکن اندرونی ساخت _timeout *z_timeout_q_نکالو_due(خالی)
{
	ساخت _timeout *t = skiplist_پہلا();
	uint8_t i;

	اگر ((t == تہی) || (t->abs_ticks > (int64_t)curr_tick)) {
		واپس تہی;
	}

	برائے (i = 0; i < t->اونچائی; i++) {
		__ASSERT_NO_MSG(sl_سر.forward[i] == t);
		sl_سر.forward[i] = t->forward[i];
	}

	t->اونچائی = 0;
	skiplist_drop_چوٹی();

	واپس t;
}

#اگر_ختم /* ZEPHYR_KERNEL_TIMEOUT_SKIPLIST_H_ */
