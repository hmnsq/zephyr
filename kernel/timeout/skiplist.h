/*
 * Copyright (c) 2026 Dhruv Menon <dhruvmenon1104@gmail.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#यदि_अपरिभाषितम् ZEPHYR_KERNEL_TIMEOUT_SKIPLIST_H_
#परिभाषय ZEPHYR_KERNEL_TIMEOUT_SKIPLIST_H_

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

#परिभाषय SKIPLIST_स्तराः CONFIG_TIMEOUT_SKIPLIST_MAX_LEVEL

/* Sentinel: never expires, participates in every level, never fired. */
स्थिर संरचना _timeout sl_शीर्ष = {
	.height = SKIPLIST_स्तराः,
	.forward = { रिक्त },
};

/* Highest height of any currently queued node; 0 when the list is empty. */
स्थिर uint8_t sl_शिखरम्;

/*
 * Scratch predecessor vector shared by insert and remove. Both run with
 * timeout_lock held and neither keeps it across an unlock, so one array is
 * enough. Any future caller must hold that lock too.
 */
स्थिर संरचना _timeout *sl_update[SKIPLIST_स्तराः];

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
	प्रत्यागम 1 + MIN(u32_गणनम्_trailing_zeros(~r), SKIPLIST_स्तराः - 1);
}

स्थिर अन्तःस्थ संरचना _timeout *skiplist_first(शून्य)
{
	प्रत्यागम sl_शीर्ष.forward[0];
}

स्थिर शून्य skiplist_drop_शिखरम्(शून्य)
{
	यावत् (sl_शिखरम् > 0 && sl_शीर्ष.forward[sl_शिखरम् - 1] == रिक्त) {
		sl_शिखरम्--;
	}
}

स्थिर शून्य skiplist_update_init(संरचना _timeout **update)
{
	uint8_t i;

	प्रति (i = 0; i < SKIPLIST_स्तराः; i++) {
		update[i] = &sl_शीर्ष;
	}
}

/* Splice @to out at every level it occupies. */
स्थिर शून्य skiplist_unlink(संरचना _timeout *गन्तव्यम्, संरचना _timeout **update)
{
	uint8_t i;
	uint8_t height = गन्तव्यम्->height;

	प्रति (i = 0; i < height; i++) {
		__ASSERT_NO_MSG(update[i]->forward[i] == गन्तव्यम्);
		update[i]->forward[i] = गन्तव्यम्->forward[i];
	}

	गन्तव्यम्->height = 0;
	skiplist_drop_शिखरम्();
}

/* Equal keys are skipped so a later insert lands after them (same-tick FIFO). */
स्थिर शून्य skiplist_predecessors_after(int64_t abs_ticks, संरचना _timeout **update)
{
	संरचना _timeout *x = &sl_शीर्ष;
	पूर्णाङ्क i;

	प्रति (i = (पूर्णाङ्क)sl_शिखरम् - 1; i >= 0; i--) {
		यावत् (x->forward[i] != रिक्त && x->forward[i]->abs_ticks <= abs_ticks) {
			x = x->forward[i];
		}
		update[i] = x;
	}
}

/* Walk equal-key nodes so identity, not just expiry, selects the splice points. */
स्थिर शून्य skiplist_predecessors_सम्बन्धि(संरचना _timeout *गन्तव्यम्, संरचना _timeout **update)
{
	संरचना _timeout *x = &sl_शीर्ष;
	पूर्णाङ्क i;

	प्रति (i = (पूर्णाङ्क)sl_शिखरम् - 1; i >= 0; i--) {
		यावत् (x->forward[i] != रिक्त && x->forward[i]->abs_ticks < गन्तव्यम्->abs_ticks) {
			x = x->forward[i];
		}
		update[i] = x;
	}

	प्रति (i = 0; i < गन्तव्यम्->height; i++) {
		यावत् (update[i]->forward[i] != रिक्त &&
		       update[i]->forward[i] != गन्तव्यम् &&
		       update[i]->forward[i]->abs_ticks == गन्तव्यम्->abs_ticks) {
			update[i] = update[i]->forward[i];
		}
	}
}

स्थिर अन्तःस्थ bool z_timeout_q_निवेशय(संरचना _timeout *गन्तव्यम्, k_ticks_t dticks)
{
	uint8_t i;
	uint8_t height;

	गन्तव्यम्->abs_ticks = (int64_t)curr_tick + dticks;
	height = skiplist_random_height();
	गन्तव्यम्->height = height;

	skiplist_update_init(sl_update);
	skiplist_predecessors_after(गन्तव्यम्->abs_ticks, sl_update);

	यदि (height > sl_शिखरम्) {
		sl_शिखरम् = height;
	}

	प्रति (i = 0; i < height; i++) {
		गन्तव्यम्->forward[i] = sl_update[i]->forward[i];
		sl_update[i]->forward[i] = गन्तव्यम्;
	}

	प्रत्यागम skiplist_first() == गन्तव्यम्;
}

स्थिर अन्तःस्थ bool z_timeout_q_remove(संरचना _timeout *गन्तव्यम्)
{
	bool was_first = (skiplist_first() == गन्तव्यम्);

	__ASSERT_NO_MSG(गन्तव्यम्->height > 0);

	skiplist_update_init(sl_update);
	skiplist_predecessors_सम्बन्धि(गन्तव्यम्, sl_update);

	skiplist_unlink(गन्तव्यम्, sl_update);

	प्रत्यागम was_first;
}

स्थिर अन्तःस्थ k_ticks_t z_timeout_q_remainder(ध्रुव संरचना _timeout *गन्तव्यम्)
{
	प्रत्यागम (k_ticks_t)(गन्तव्यम्->abs_ticks - (int64_t)curr_tick);
}

स्थिर अन्तःस्थ k_ticks_t z_timeout_q_अग्र_expiry(शून्य)
{
	संरचना _timeout *t = skiplist_first();
	int64_t gap;

	यदि (t == रिक्त) {
		प्रत्यागम K_TICKS_FOREVER;
	}

	gap = t->abs_ticks - (int64_t)curr_tick;
	प्रत्यागम (gap < 0) ? 0 : (k_ticks_t)gap;
}

स्थिर अन्तःस्थ int32_t z_timeout_q_अग्र_gap(शून्य)
{
	संरचना _timeout *t = skiplist_first();
	int64_t gap;

	यदि (t == रिक्त) {
		प्रत्यागम INT32_MAX;
	}

	/* Overdue must be 0: announce casts dt to uint32_t. */
	gap = t->abs_ticks - (int64_t)curr_tick;
	यदि (gap <= 0) {
		प्रत्यागम 0;
	}

	प्रत्यागम (int32_t)MIN(gap, (int64_t)INT32_MAX);
}

स्थिर अन्तःस्थ शून्य z_timeout_q_advance(int32_t dt)
{
	ARG_अप्रयुक्तम्(dt);
}

/* Earliest node, so it is the head's successor at every level it occupies. */
स्थिर अन्तःस्थ संरचना _timeout *z_timeout_q_निष्कासय_due(शून्य)
{
	संरचना _timeout *t = skiplist_first();
	uint8_t i;

	यदि ((t == रिक्त) || (t->abs_ticks > (int64_t)curr_tick)) {
		प्रत्यागम रिक्त;
	}

	प्रति (i = 0; i < t->height; i++) {
		__ASSERT_NO_MSG(sl_शीर्ष.forward[i] == t);
		sl_शीर्ष.forward[i] = t->forward[i];
	}

	t->height = 0;
	skiplist_drop_शिखरम्();

	प्रत्यागम t;
}

#यदि_अन्तः /* ZEPHYR_KERNEL_TIMEOUT_SKIPLIST_H_ */
