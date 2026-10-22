/*
 * Copyright (c) 2026 Dhruv Menon <dhruvmenon1104@gmail.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#अगर_अघोषित ZEPHYR_KERNEL_TIMEOUT_SKIPLIST_H_
#घोषणा ZEPHYR_KERNEL_TIMEOUT_SKIPLIST_H_

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

#घोषणा SKIPLIST_LEVELS CONFIG_TIMEOUT_SKIPLIST_MAX_LEVEL

/* Sentinel: never expires, participates in every level, never fired. */
अटल काष्ठा _timeout sl_शीर्ष = {
	.height = SKIPLIST_LEVELS,
	.forward = { शून्य },
};

/* Highest height of any currently queued node; 0 when the list is empty. */
अटल uint8_t sl_शीर्ष_2;

/*
 * Scratch predecessor vector shared by insert and remove. Both run with
 * timeout_lock held and neither keeps it across an unlock, so one array is
 * enough. Any future caller must hold that lock too.
 */
अटल काष्ठा _timeout *sl_update[SKIPLIST_LEVELS];

/* Marsaglia xorshift32 state; period 2^32-1, must stay non-zero. */
अटल uint32_t sl_rng = 2463534242U;

अटल uint8_t skiplist_random_height(व्योम)
{
	uint32_t r = sl_rng;

	r ^= r << 13;
	r ^= r >> 17;
	r ^= r << 5;
	sl_rng = r;

	/* p = 1/2: height is 1 + the run of low 1-bits, capped at SKIPLIST_LEVELS. */
	वापस 1 + MIN(u32_गिनती_trailing_zeros(~r), SKIPLIST_LEVELS - 1);
}

अटल अंतरभूत काष्ठा _timeout *skiplist_first(व्योम)
{
	वापस sl_शीर्ष.forward[0];
}

अटल व्योम skiplist_drop_शीर्ष(व्योम)
{
	जबतक (sl_शीर्ष_2 > 0 && sl_शीर्ष.forward[sl_शीर्ष_2 - 1] == शून्य) {
		sl_शीर्ष_2--;
	}
}

अटल व्योम skiplist_update_init(काष्ठा _timeout **update)
{
	uint8_t i;

	क्रम (i = 0; i < SKIPLIST_LEVELS; i++) {
		update[i] = &sl_शीर्ष;
	}
}

/* Splice @to out at every level it occupies. */
अटल व्योम skiplist_unlink(काष्ठा _timeout *to, काष्ठा _timeout **update)
{
	uint8_t i;
	uint8_t height = to->height;

	क्रम (i = 0; i < height; i++) {
		__ASSERT_NO_MSG(update[i]->forward[i] == to);
		update[i]->forward[i] = to->forward[i];
	}

	to->height = 0;
	skiplist_drop_शीर्ष();
}

/* Equal keys are skipped so a later insert lands after them (same-tick FIFO). */
अटल व्योम skiplist_predecessors_after(int64_t abs_ticks, काष्ठा _timeout **update)
{
	काष्ठा _timeout *x = &sl_शीर्ष;
	पूर्णांक i;

	क्रम (i = (पूर्णांक)sl_शीर्ष_2 - 1; i >= 0; i--) {
		जबतक (x->forward[i] != शून्य && x->forward[i]->abs_ticks <= abs_ticks) {
			x = x->forward[i];
		}
		update[i] = x;
	}
}

/* Walk equal-key nodes so identity, not just expiry, selects the splice points. */
अटल व्योम skiplist_predecessors_of(काष्ठा _timeout *to, काष्ठा _timeout **update)
{
	काष्ठा _timeout *x = &sl_शीर्ष;
	पूर्णांक i;

	क्रम (i = (पूर्णांक)sl_शीर्ष_2 - 1; i >= 0; i--) {
		जबतक (x->forward[i] != शून्य && x->forward[i]->abs_ticks < to->abs_ticks) {
			x = x->forward[i];
		}
		update[i] = x;
	}

	क्रम (i = 0; i < to->height; i++) {
		जबतक (update[i]->forward[i] != शून्य &&
		       update[i]->forward[i] != to &&
		       update[i]->forward[i]->abs_ticks == to->abs_ticks) {
			update[i] = update[i]->forward[i];
		}
	}
}

अटल अंतरभूत bool z_timeout_q_जोड़ो(काष्ठा _timeout *to, k_ticks_t dticks)
{
	uint8_t i;
	uint8_t height;

	to->abs_ticks = (int64_t)curr_tick + dticks;
	height = skiplist_random_height();
	to->height = height;

	skiplist_update_init(sl_update);
	skiplist_predecessors_after(to->abs_ticks, sl_update);

	अगर (height > sl_शीर्ष_2) {
		sl_शीर्ष_2 = height;
	}

	क्रम (i = 0; i < height; i++) {
		to->forward[i] = sl_update[i]->forward[i];
		sl_update[i]->forward[i] = to;
	}

	वापस skiplist_first() == to;
}

अटल अंतरभूत bool z_timeout_q_remove(काष्ठा _timeout *to)
{
	bool was_first = (skiplist_first() == to);

	__ASSERT_NO_MSG(to->height > 0);

	skiplist_update_init(sl_update);
	skiplist_predecessors_of(to, sl_update);

	skiplist_unlink(to, sl_update);

	वापस was_first;
}

अटल अंतरभूत k_ticks_t z_timeout_q_remainder(स्थिर काष्ठा _timeout *to)
{
	वापस (k_ticks_t)(to->abs_ticks - (int64_t)curr_tick);
}

अटल अंतरभूत k_ticks_t z_timeout_q_अग्र_expiry(व्योम)
{
	काष्ठा _timeout *t = skiplist_first();
	int64_t gap;

	अगर (t == शून्य) {
		वापस K_TICKS_FOREVER;
	}

	gap = t->abs_ticks - (int64_t)curr_tick;
	वापस (gap < 0) ? 0 : (k_ticks_t)gap;
}

अटल अंतरभूत int32_t z_timeout_q_अग्र_gap(व्योम)
{
	काष्ठा _timeout *t = skiplist_first();
	int64_t gap;

	अगर (t == शून्य) {
		वापस INT32_MAX;
	}

	/* Overdue must be 0: announce casts dt to uint32_t. */
	gap = t->abs_ticks - (int64_t)curr_tick;
	अगर (gap <= 0) {
		वापस 0;
	}

	वापस (int32_t)MIN(gap, (int64_t)INT32_MAX);
}

अटल अंतरभूत व्योम z_timeout_q_advance(int32_t dt)
{
	ARG_UNUSED(dt);
}

/* Earliest node, so it is the head's successor at every level it occupies. */
अटल अंतरभूत काष्ठा _timeout *z_timeout_q_निकालो_due(व्योम)
{
	काष्ठा _timeout *t = skiplist_first();
	uint8_t i;

	अगर ((t == शून्य) || (t->abs_ticks > (int64_t)curr_tick)) {
		वापस शून्य;
	}

	क्रम (i = 0; i < t->height; i++) {
		__ASSERT_NO_MSG(sl_शीर्ष.forward[i] == t);
		sl_शीर्ष.forward[i] = t->forward[i];
	}

	t->height = 0;
	skiplist_drop_शीर्ष();

	वापस t;
}

#पूर्ण_अगर /* ZEPHYR_KERNEL_TIMEOUT_SKIPLIST_H_ */
