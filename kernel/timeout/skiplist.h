/*
 * Copyright (c) 2026 Dhruv Menon <dhruvmenon1104@gmail.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#جيڪڏهن_وصف_نه_ٿيل ZEPHYR_KERNEL_TIMEOUT_SKIPLIST_H_
#وصف_ڪريو ZEPHYR_KERNEL_TIMEOUT_SKIPLIST_H_

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

#وصف_ڪريو SKIPLIST_سطحون CONFIG_TIMEOUT_SKIPLIST_MAX_LEVEL

/* Sentinel: never expires, participates in every level, never fired. */
ساكن بنية _timeout sl_سر = {
	.اوچائي = SKIPLIST_سطحون,
	.forward = { عدم },
};

/* Highest height of any currently queued node; 0 when the list is empty. */
ساكن uint8_t sl_چوٽي;

/*
 * Scratch predecessor vector shared by insert and remove. Both run with
 * timeout_lock held and neither keeps it across an unlock, so one array is
 * enough. Any future caller must hold that lock too.
 */
ساكن بنية _timeout *sl_update[SKIPLIST_سطحون];

/* Marsaglia xorshift32 state; period 2^32-1, must stay non-zero. */
ساكن uint32_t sl_rng = 2463534242U;

ساكن uint8_t skiplist_random_اوچائي(فراغ)
{
	uint32_t r = sl_rng;

	r ^= r << 13;
	r ^= r >> 17;
	r ^= r << 5;
	sl_rng = r;

	/* p = 1/2: height is 1 + the run of low 1-bits, capped at SKIPLIST_LEVELS. */
	أرجع 1 + MIN(u32_ڳڻپ_trailing_zeros(~r), SKIPLIST_سطحون - 1);
}

ساكن مضمن بنية _timeout *skiplist_پهريون(فراغ)
{
	أرجع sl_سر.forward[0];
}

ساكن فراغ skiplist_drop_چوٽي(فراغ)
{
	طالما (sl_چوٽي > 0 && sl_سر.forward[sl_چوٽي - 1] == عدم) {
		sl_چوٽي--;
	}
}

ساكن فراغ skiplist_update_init(بنية _timeout **update)
{
	uint8_t i;

	لكل (i = 0; i < SKIPLIST_سطحون; i++) {
		update[i] = &sl_سر;
	}
}

/* Splice @to out at every level it occupies. */
ساكن فراغ skiplist_unlink(بنية _timeout *منزل, بنية _timeout **update)
{
	uint8_t i;
	uint8_t اوچائي = منزل->اوچائي;

	لكل (i = 0; i < اوچائي; i++) {
		__ASSERT_NO_MSG(update[i]->forward[i] == منزل);
		update[i]->forward[i] = منزل->forward[i];
	}

	منزل->اوچائي = 0;
	skiplist_drop_چوٽي();
}

/* Equal keys are skipped so a later insert lands after them (same-tick FIFO). */
ساكن فراغ skiplist_predecessors_after(int64_t abs_ticks, بنية _timeout **update)
{
	بنية _timeout *x = &sl_سر;
	صحيح i;

	لكل (i = (صحيح)sl_چوٽي - 1; i >= 0; i--) {
		طالما (x->forward[i] != عدم && x->forward[i]->abs_ticks <= abs_ticks) {
			x = x->forward[i];
		}
		update[i] = x;
	}
}

/* Walk equal-key nodes so identity, not just expiry, selects the splice points. */
ساكن فراغ skiplist_predecessors_جو(بنية _timeout *منزل, بنية _timeout **update)
{
	بنية _timeout *x = &sl_سر;
	صحيح i;

	لكل (i = (صحيح)sl_چوٽي - 1; i >= 0; i--) {
		طالما (x->forward[i] != عدم && x->forward[i]->abs_ticks < منزل->abs_ticks) {
			x = x->forward[i];
		}
		update[i] = x;
	}

	لكل (i = 0; i < منزل->اوچائي; i++) {
		طالما (update[i]->forward[i] != عدم &&
		       update[i]->forward[i] != منزل &&
		       update[i]->forward[i]->abs_ticks == منزل->abs_ticks) {
			update[i] = update[i]->forward[i];
		}
	}
}

ساكن مضمن bool z_timeout_q_وجهو(بنية _timeout *منزل, k_ticks_t dticks)
{
	uint8_t i;
	uint8_t اوچائي;

	منزل->abs_ticks = (int64_t)curr_tick + dticks;
	اوچائي = skiplist_random_اوچائي();
	منزل->اوچائي = اوچائي;

	skiplist_update_init(sl_update);
	skiplist_predecessors_after(منزل->abs_ticks, sl_update);

	إذا (اوچائي > sl_چوٽي) {
		sl_چوٽي = اوچائي;
	}

	لكل (i = 0; i < اوچائي; i++) {
		منزل->forward[i] = sl_update[i]->forward[i];
		sl_update[i]->forward[i] = منزل;
	}

	أرجع skiplist_پهريون() == منزل;
}

ساكن مضمن bool z_timeout_q_remove(بنية _timeout *منزل)
{
	bool was_پهريون = (skiplist_پهريون() == منزل);

	__ASSERT_NO_MSG(منزل->اوچائي > 0);

	skiplist_update_init(sl_update);
	skiplist_predecessors_جو(منزل, sl_update);

	skiplist_unlink(منزل, sl_update);

	أرجع was_پهريون;
}

ساكن مضمن k_ticks_t z_timeout_q_remainder(ثابت بنية _timeout *منزل)
{
	أرجع (k_ticks_t)(منزل->abs_ticks - (int64_t)curr_tick);
}

ساكن مضمن k_ticks_t z_timeout_q_ايندڙ_expiry(فراغ)
{
	بنية _timeout *t = skiplist_پهريون();
	int64_t gap;

	إذا (t == عدم) {
		أرجع K_TICKS_FOREVER;
	}

	gap = t->abs_ticks - (int64_t)curr_tick;
	أرجع (gap < 0) ? 0 : (k_ticks_t)gap;
}

ساكن مضمن int32_t z_timeout_q_ايندڙ_gap(فراغ)
{
	بنية _timeout *t = skiplist_پهريون();
	int64_t gap;

	إذا (t == عدم) {
		أرجع INT32_MAX;
	}

	/* Overdue must be 0: announce casts dt to uint32_t. */
	gap = t->abs_ticks - (int64_t)curr_tick;
	إذا (gap <= 0) {
		أرجع 0;
	}

	أرجع (int32_t)MIN(gap, (int64_t)INT32_MAX);
}

ساكن مضمن فراغ z_timeout_q_advance(int32_t dt)
{
	ARG_اڻ_استعمال_ٿيل(dt);
}

/* Earliest node, so it is the head's successor at every level it occupies. */
ساكن مضمن بنية _timeout *z_timeout_q_ڪڍو_due(فراغ)
{
	بنية _timeout *t = skiplist_پهريون();
	uint8_t i;

	إذا ((t == عدم) || (t->abs_ticks > (int64_t)curr_tick)) {
		أرجع عدم;
	}

	لكل (i = 0; i < t->اوچائي; i++) {
		__ASSERT_NO_MSG(sl_سر.forward[i] == t);
		sl_سر.forward[i] = t->forward[i];
	}

	t->اوچائي = 0;
	skiplist_drop_چوٽي();

	أرجع t;
}

#جيڪڏهن_پورو /* ZEPHYR_KERNEL_TIMEOUT_SKIPLIST_H_ */
