/*
 * Copyright (c) 2026 Dhruv Menon <dhruvmenon1104@gmail.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ ZEPHYR_KERNEL_TIMEOUT_SKIPLIST_H_
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ZEPHYR_KERNEL_TIMEOUT_SKIPLIST_H_

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

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SKIPLIST_ਪੱਧਰ CONFIG_TIMEOUT_SKIPLIST_MAX_LEVEL

/* Sentinel: never expires, participates in every level, never fired. */
ਸਥਿਰ ਬਣਤਰ _timeout sl_ਸਿਰ = {
	.ਉਚਾਈ = SKIPLIST_ਪੱਧਰ,
	.forward = { ਨਿਰਾ },
};

/* Highest height of any currently queued node; 0 when the list is empty. */
ਸਥਿਰ uint8_t sl_ਸਿਖਰ;

/*
 * Scratch predecessor vector shared by insert and remove. Both run with
 * timeout_lock held and neither keeps it across an unlock, so one array is
 * enough. Any future caller must hold that lock too.
 */
ਸਥਿਰ ਬਣਤਰ _timeout *sl_update[SKIPLIST_ਪੱਧਰ];

/* Marsaglia xorshift32 state; period 2^32-1, must stay non-zero. */
ਸਥਿਰ uint32_t sl_rng = 2463534242U;

ਸਥਿਰ uint8_t skiplist_random_ਉਚਾਈ(ਖਾਲੀ)
{
	uint32_t r = sl_rng;

	r ^= r << 13;
	r ^= r >> 17;
	r ^= r << 5;
	sl_rng = r;

	/* p = 1/2: height is 1 + the run of low 1-bits, capped at SKIPLIST_LEVELS. */
	ਵਾਪਸ 1 + MIN(u32_ਗਿਣਤੀ_trailing_zeros(~r), SKIPLIST_ਪੱਧਰ - 1);
}

ਸਥਿਰ ਸਤਰਅੰਦਰ ਬਣਤਰ _timeout *skiplist_ਪਹਿਲਾ(ਖਾਲੀ)
{
	ਵਾਪਸ sl_ਸਿਰ.forward[0];
}

ਸਥਿਰ ਖਾਲੀ skiplist_drop_ਸਿਖਰ(ਖਾਲੀ)
{
	ਜਦੋਂ (sl_ਸਿਖਰ > 0 && sl_ਸਿਰ.forward[sl_ਸਿਖਰ - 1] == ਨਿਰਾ) {
		sl_ਸਿਖਰ--;
	}
}

ਸਥਿਰ ਖਾਲੀ skiplist_update_init(ਬਣਤਰ _timeout **update)
{
	uint8_t i;

	ਲਈ (i = 0; i < SKIPLIST_ਪੱਧਰ; i++) {
		update[i] = &sl_ਸਿਰ;
	}
}

/* Splice @to out at every level it occupies. */
ਸਥਿਰ ਖਾਲੀ skiplist_unlink(ਬਣਤਰ _timeout *ਮੰਜ਼ਿਲ, ਬਣਤਰ _timeout **update)
{
	uint8_t i;
	uint8_t ਉਚਾਈ = ਮੰਜ਼ਿਲ->ਉਚਾਈ;

	ਲਈ (i = 0; i < ਉਚਾਈ; i++) {
		__ASSERT_NO_MSG(update[i]->forward[i] == ਮੰਜ਼ਿਲ);
		update[i]->forward[i] = ਮੰਜ਼ਿਲ->forward[i];
	}

	ਮੰਜ਼ਿਲ->ਉਚਾਈ = 0;
	skiplist_drop_ਸਿਖਰ();
}

/* Equal keys are skipped so a later insert lands after them (same-tick FIFO). */
ਸਥਿਰ ਖਾਲੀ skiplist_predecessors_after(int64_t abs_ticks, ਬਣਤਰ _timeout **update)
{
	ਬਣਤਰ _timeout *x = &sl_ਸਿਰ;
	ਪੂਰਨ i;

	ਲਈ (i = (ਪੂਰਨ)sl_ਸਿਖਰ - 1; i >= 0; i--) {
		ਜਦੋਂ (x->forward[i] != ਨਿਰਾ && x->forward[i]->abs_ticks <= abs_ticks) {
			x = x->forward[i];
		}
		update[i] = x;
	}
}

/* Walk equal-key nodes so identity, not just expiry, selects the splice points. */
ਸਥਿਰ ਖਾਲੀ skiplist_predecessors_ਦਾ(ਬਣਤਰ _timeout *ਮੰਜ਼ਿਲ, ਬਣਤਰ _timeout **update)
{
	ਬਣਤਰ _timeout *x = &sl_ਸਿਰ;
	ਪੂਰਨ i;

	ਲਈ (i = (ਪੂਰਨ)sl_ਸਿਖਰ - 1; i >= 0; i--) {
		ਜਦੋਂ (x->forward[i] != ਨਿਰਾ && x->forward[i]->abs_ticks < ਮੰਜ਼ਿਲ->abs_ticks) {
			x = x->forward[i];
		}
		update[i] = x;
	}

	ਲਈ (i = 0; i < ਮੰਜ਼ਿਲ->ਉਚਾਈ; i++) {
		ਜਦੋਂ (update[i]->forward[i] != ਨਿਰਾ &&
		       update[i]->forward[i] != ਮੰਜ਼ਿਲ &&
		       update[i]->forward[i]->abs_ticks == ਮੰਜ਼ਿਲ->abs_ticks) {
			update[i] = update[i]->forward[i];
		}
	}
}

ਸਥਿਰ ਸਤਰਅੰਦਰ bool z_timeout_q_ਪਾਓ(ਬਣਤਰ _timeout *ਮੰਜ਼ਿਲ, k_ticks_t dticks)
{
	uint8_t i;
	uint8_t ਉਚਾਈ;

	ਮੰਜ਼ਿਲ->abs_ticks = (int64_t)curr_tick + dticks;
	ਉਚਾਈ = skiplist_random_ਉਚਾਈ();
	ਮੰਜ਼ਿਲ->ਉਚਾਈ = ਉਚਾਈ;

	skiplist_update_init(sl_update);
	skiplist_predecessors_after(ਮੰਜ਼ਿਲ->abs_ticks, sl_update);

	ਜੇ (ਉਚਾਈ > sl_ਸਿਖਰ) {
		sl_ਸਿਖਰ = ਉਚਾਈ;
	}

	ਲਈ (i = 0; i < ਉਚਾਈ; i++) {
		ਮੰਜ਼ਿਲ->forward[i] = sl_update[i]->forward[i];
		sl_update[i]->forward[i] = ਮੰਜ਼ਿਲ;
	}

	ਵਾਪਸ skiplist_ਪਹਿਲਾ() == ਮੰਜ਼ਿਲ;
}

ਸਥਿਰ ਸਤਰਅੰਦਰ bool z_timeout_q_remove(ਬਣਤਰ _timeout *ਮੰਜ਼ਿਲ)
{
	bool was_ਪਹਿਲਾ = (skiplist_ਪਹਿਲਾ() == ਮੰਜ਼ਿਲ);

	__ASSERT_NO_MSG(ਮੰਜ਼ਿਲ->ਉਚਾਈ > 0);

	skiplist_update_init(sl_update);
	skiplist_predecessors_ਦਾ(ਮੰਜ਼ਿਲ, sl_update);

	skiplist_unlink(ਮੰਜ਼ਿਲ, sl_update);

	ਵਾਪਸ was_ਪਹਿਲਾ;
}

ਸਥਿਰ ਸਤਰਅੰਦਰ k_ticks_t z_timeout_q_remainder(ਅਚਲ ਬਣਤਰ _timeout *ਮੰਜ਼ਿਲ)
{
	ਵਾਪਸ (k_ticks_t)(ਮੰਜ਼ਿਲ->abs_ticks - (int64_t)curr_tick);
}

ਸਥਿਰ ਸਤਰਅੰਦਰ k_ticks_t z_timeout_q_ਅਗਲਾ_expiry(ਖਾਲੀ)
{
	ਬਣਤਰ _timeout *t = skiplist_ਪਹਿਲਾ();
	int64_t gap;

	ਜੇ (t == ਨਿਰਾ) {
		ਵਾਪਸ K_TICKS_FOREVER;
	}

	gap = t->abs_ticks - (int64_t)curr_tick;
	ਵਾਪਸ (gap < 0) ? 0 : (k_ticks_t)gap;
}

ਸਥਿਰ ਸਤਰਅੰਦਰ int32_t z_timeout_q_ਅਗਲਾ_gap(ਖਾਲੀ)
{
	ਬਣਤਰ _timeout *t = skiplist_ਪਹਿਲਾ();
	int64_t gap;

	ਜੇ (t == ਨਿਰਾ) {
		ਵਾਪਸ INT32_MAX;
	}

	/* Overdue must be 0: announce casts dt to uint32_t. */
	gap = t->abs_ticks - (int64_t)curr_tick;
	ਜੇ (gap <= 0) {
		ਵਾਪਸ 0;
	}

	ਵਾਪਸ (int32_t)MIN(gap, (int64_t)INT32_MAX);
}

ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ z_timeout_q_advance(int32_t dt)
{
	ARG_ਅਣਵਰਤਿਆ(dt);
}

/* Earliest node, so it is the head's successor at every level it occupies. */
ਸਥਿਰ ਸਤਰਅੰਦਰ ਬਣਤਰ _timeout *z_timeout_q_ਕੱਢੋ_due(ਖਾਲੀ)
{
	ਬਣਤਰ _timeout *t = skiplist_ਪਹਿਲਾ();
	uint8_t i;

	ਜੇ ((t == ਨਿਰਾ) || (t->abs_ticks > (int64_t)curr_tick)) {
		ਵਾਪਸ ਨਿਰਾ;
	}

	ਲਈ (i = 0; i < t->ਉਚਾਈ; i++) {
		__ASSERT_NO_MSG(sl_ਸਿਰ.forward[i] == t);
		sl_ਸਿਰ.forward[i] = t->forward[i];
	}

	t->ਉਚਾਈ = 0;
	skiplist_drop_ਸਿਖਰ();

	ਵਾਪਸ t;
}

#ਜੇ_ਅੰਤ /* ZEPHYR_KERNEL_TIMEOUT_SKIPLIST_H_ */
