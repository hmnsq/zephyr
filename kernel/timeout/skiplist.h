/*
 * Copyright (c) 2026 Dhruv Menon <dhruvmenon1104@gmail.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#నిర్వచితం_కాకపోతే ZEPHYR_KERNEL_TIMEOUT_SKIPLIST_H_
#నిర్వచించు ZEPHYR_KERNEL_TIMEOUT_SKIPLIST_H_

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

#నిర్వచించు SKIPLIST_స్థాయిలు CONFIG_TIMEOUT_SKIPLIST_MAX_LEVEL

/* Sentinel: never expires, participates in every level, never fired. */
స్థిర నిర్మాణం _timeout sl_తల = {
	.ఎత్తు = SKIPLIST_స్థాయిలు,
	.forward = { ఖాళీ },
};

/* Highest height of any currently queued node; 0 when the list is empty. */
స్థిర uint8_t sl_శిఖరం;

/*
 * Scratch predecessor vector shared by insert and remove. Both run with
 * timeout_lock held and neither keeps it across an unlock, so one array is
 * enough. Any future caller must hold that lock too.
 */
స్థిర నిర్మాణం _timeout *sl_update[SKIPLIST_స్థాయిలు];

/* Marsaglia xorshift32 state; period 2^32-1, must stay non-zero. */
స్థిర uint32_t sl_rng = 2463534242U;

స్థిర uint8_t skiplist_random_ఎత్తు(శూన్య)
{
	uint32_t r = sl_rng;

	r ^= r << 13;
	r ^= r >> 17;
	r ^= r << 5;
	sl_rng = r;

	/* p = 1/2: height is 1 + the run of low 1-bits, capped at SKIPLIST_LEVELS. */
	తిరిగి 1 + MIN(u32_లెక్క_trailing_zeros(~r), SKIPLIST_స్థాయిలు - 1);
}

స్థిర అంతఃవరుస నిర్మాణం _timeout *skiplist_మొదటి(శూన్య)
{
	తిరిగి sl_తల.forward[0];
}

స్థిర శూన్య skiplist_drop_శిఖరం(శూన్య)
{
	అయితే (sl_శిఖరం > 0 && sl_తల.forward[sl_శిఖరం - 1] == ఖాళీ) {
		sl_శిఖరం--;
	}
}

స్థిర శూన్య skiplist_update_init(నిర్మాణం _timeout **update)
{
	uint8_t i;

	కొరకు (i = 0; i < SKIPLIST_స్థాయిలు; i++) {
		update[i] = &sl_తల;
	}
}

/* Splice @to out at every level it occupies. */
స్థిర శూన్య skiplist_unlink(నిర్మాణం _timeout *గమ్యం, నిర్మాణం _timeout **update)
{
	uint8_t i;
	uint8_t ఎత్తు = గమ్యం->ఎత్తు;

	కొరకు (i = 0; i < ఎత్తు; i++) {
		__ASSERT_NO_MSG(update[i]->forward[i] == గమ్యం);
		update[i]->forward[i] = గమ్యం->forward[i];
	}

	గమ్యం->ఎత్తు = 0;
	skiplist_drop_శిఖరం();
}

/* Equal keys are skipped so a later insert lands after them (same-tick FIFO). */
స్థిర శూన్య skiplist_predecessors_after(int64_t abs_ticks, నిర్మాణం _timeout **update)
{
	నిర్మాణం _timeout *x = &sl_తల;
	పూర్ణాంకం i;

	కొరకు (i = (పూర్ణాంకం)sl_శిఖరం - 1; i >= 0; i--) {
		అయితే (x->forward[i] != ఖాళీ && x->forward[i]->abs_ticks <= abs_ticks) {
			x = x->forward[i];
		}
		update[i] = x;
	}
}

/* Walk equal-key nodes so identity, not just expiry, selects the splice points. */
స్థిర శూన్య skiplist_predecessors_యొక్క(నిర్మాణం _timeout *గమ్యం, నిర్మాణం _timeout **update)
{
	నిర్మాణం _timeout *x = &sl_తల;
	పూర్ణాంకం i;

	కొరకు (i = (పూర్ణాంకం)sl_శిఖరం - 1; i >= 0; i--) {
		అయితే (x->forward[i] != ఖాళీ && x->forward[i]->abs_ticks < గమ్యం->abs_ticks) {
			x = x->forward[i];
		}
		update[i] = x;
	}

	కొరకు (i = 0; i < గమ్యం->ఎత్తు; i++) {
		అయితే (update[i]->forward[i] != ఖాళీ &&
		       update[i]->forward[i] != గమ్యం &&
		       update[i]->forward[i]->abs_ticks == గమ్యం->abs_ticks) {
			update[i] = update[i]->forward[i];
		}
	}
}

స్థిర అంతఃవరుస bool z_timeout_q_చొప్పించు(నిర్మాణం _timeout *గమ్యం, k_ticks_t dticks)
{
	uint8_t i;
	uint8_t ఎత్తు;

	గమ్యం->abs_ticks = (int64_t)curr_tick + dticks;
	ఎత్తు = skiplist_random_ఎత్తు();
	గమ్యం->ఎత్తు = ఎత్తు;

	skiplist_update_init(sl_update);
	skiplist_predecessors_after(గమ్యం->abs_ticks, sl_update);

	ఒకవేళ (ఎత్తు > sl_శిఖరం) {
		sl_శిఖరం = ఎత్తు;
	}

	కొరకు (i = 0; i < ఎత్తు; i++) {
		గమ్యం->forward[i] = sl_update[i]->forward[i];
		sl_update[i]->forward[i] = గమ్యం;
	}

	తిరిగి skiplist_మొదటి() == గమ్యం;
}

స్థిర అంతఃవరుస bool z_timeout_q_remove(నిర్మాణం _timeout *గమ్యం)
{
	bool was_మొదటి = (skiplist_మొదటి() == గమ్యం);

	__ASSERT_NO_MSG(గమ్యం->ఎత్తు > 0);

	skiplist_update_init(sl_update);
	skiplist_predecessors_యొక్క(గమ్యం, sl_update);

	skiplist_unlink(గమ్యం, sl_update);

	తిరిగి was_మొదటి;
}

స్థిర అంతఃవరుస k_ticks_t z_timeout_q_remainder(స్థిరరాశి నిర్మాణం _timeout *గమ్యం)
{
	తిరిగి (k_ticks_t)(గమ్యం->abs_ticks - (int64_t)curr_tick);
}

స్థిర అంతఃవరుస k_ticks_t z_timeout_q_తదుపరి_expiry(శూన్య)
{
	నిర్మాణం _timeout *t = skiplist_మొదటి();
	int64_t gap;

	ఒకవేళ (t == ఖాళీ) {
		తిరిగి K_TICKS_FOREVER;
	}

	gap = t->abs_ticks - (int64_t)curr_tick;
	తిరిగి (gap < 0) ? 0 : (k_ticks_t)gap;
}

స్థిర అంతఃవరుస int32_t z_timeout_q_తదుపరి_gap(శూన్య)
{
	నిర్మాణం _timeout *t = skiplist_మొదటి();
	int64_t gap;

	ఒకవేళ (t == ఖాళీ) {
		తిరిగి INT32_MAX;
	}

	/* Overdue must be 0: announce casts dt to uint32_t. */
	gap = t->abs_ticks - (int64_t)curr_tick;
	ఒకవేళ (gap <= 0) {
		తిరిగి 0;
	}

	తిరిగి (int32_t)MIN(gap, (int64_t)INT32_MAX);
}

స్థిర అంతఃవరుస శూన్య z_timeout_q_advance(int32_t dt)
{
	ARG_వాడని(dt);
}

/* Earliest node, so it is the head's successor at every level it occupies. */
స్థిర అంతఃవరుస నిర్మాణం _timeout *z_timeout_q_తీయి_due(శూన్య)
{
	నిర్మాణం _timeout *t = skiplist_మొదటి();
	uint8_t i;

	ఒకవేళ ((t == ఖాళీ) || (t->abs_ticks > (int64_t)curr_tick)) {
		తిరిగి ఖాళీ;
	}

	కొరకు (i = 0; i < t->ఎత్తు; i++) {
		__ASSERT_NO_MSG(sl_తల.forward[i] == t);
		sl_తల.forward[i] = t->forward[i];
	}

	t->ఎత్తు = 0;
	skiplist_drop_శిఖరం();

	తిరిగి t;
}

#అయితే_ముగింపు /* ZEPHYR_KERNEL_TIMEOUT_SKIPLIST_H_ */
