/*
 * Copyright (c) 2026 Dhruv Menon <dhruvmenon1104@gmail.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ ZEPHYR_KERNEL_TIMEOUT_SKIPLIST_H_
#ವ್ಯಾಖ್ಯಾನಿಸು ZEPHYR_KERNEL_TIMEOUT_SKIPLIST_H_

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

#ವ್ಯಾಖ್ಯಾನಿಸು SKIPLIST_ಮಟ್ಟಗಳು CONFIG_TIMEOUT_SKIPLIST_MAX_LEVEL

/* Sentinel: never expires, participates in every level, never fired. */
ಸ್ಥಿರ ರಚನೆ _timeout sl_ತಲೆ = {
	.ಎತ್ತರ = SKIPLIST_ಮಟ್ಟಗಳು,
	.forward = { ಖಾಲಿ },
};

/* Highest height of any currently queued node; 0 when the list is empty. */
ಸ್ಥಿರ uint8_t sl_ತುದಿ;

/*
 * Scratch predecessor vector shared by insert and remove. Both run with
 * timeout_lock held and neither keeps it across an unlock, so one array is
 * enough. Any future caller must hold that lock too.
 */
ಸ್ಥಿರ ರಚನೆ _timeout *sl_update[SKIPLIST_ಮಟ್ಟಗಳು];

/* Marsaglia xorshift32 state; period 2^32-1, must stay non-zero. */
ಸ್ಥಿರ uint32_t sl_rng = 2463534242U;

ಸ್ಥಿರ uint8_t skiplist_random_ಎತ್ತರ(ಶೂನ್ಯ)
{
	uint32_t r = sl_rng;

	r ^= r << 13;
	r ^= r >> 17;
	r ^= r << 5;
	sl_rng = r;

	/* p = 1/2: height is 1 + the run of low 1-bits, capped at SKIPLIST_LEVELS. */
	ಹಿಂತಿರುಗಿ 1 + MIN(u32_ಎಣಿಕೆ_trailing_zeros(~r), SKIPLIST_ಮಟ್ಟಗಳು - 1);
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ರಚನೆ _timeout *skiplist_ಮೊದಲ(ಶೂನ್ಯ)
{
	ಹಿಂತಿರುಗಿ sl_ತಲೆ.forward[0];
}

ಸ್ಥಿರ ಶೂನ್ಯ skiplist_drop_ತುದಿ(ಶೂನ್ಯ)
{
	ಆಗ (sl_ತುದಿ > 0 && sl_ತಲೆ.forward[sl_ತುದಿ - 1] == ಖಾಲಿ) {
		sl_ತುದಿ--;
	}
}

ಸ್ಥಿರ ಶೂನ್ಯ skiplist_update_init(ರಚನೆ _timeout **update)
{
	uint8_t i;

	ಗಾಗಿ (i = 0; i < SKIPLIST_ಮಟ್ಟಗಳು; i++) {
		update[i] = &sl_ತಲೆ;
	}
}

/* Splice @to out at every level it occupies. */
ಸ್ಥಿರ ಶೂನ್ಯ skiplist_unlink(ರಚನೆ _timeout *ಗಮ್ಯ, ರಚನೆ _timeout **update)
{
	uint8_t i;
	uint8_t ಎತ್ತರ = ಗಮ್ಯ->ಎತ್ತರ;

	ಗಾಗಿ (i = 0; i < ಎತ್ತರ; i++) {
		__ASSERT_NO_MSG(update[i]->forward[i] == ಗಮ್ಯ);
		update[i]->forward[i] = ಗಮ್ಯ->forward[i];
	}

	ಗಮ್ಯ->ಎತ್ತರ = 0;
	skiplist_drop_ತುದಿ();
}

/* Equal keys are skipped so a later insert lands after them (same-tick FIFO). */
ಸ್ಥಿರ ಶೂನ್ಯ skiplist_predecessors_after(int64_t abs_ticks, ರಚನೆ _timeout **update)
{
	ರಚನೆ _timeout *x = &sl_ತಲೆ;
	ಪೂರ್ಣಾಂಕ i;

	ಗಾಗಿ (i = (ಪೂರ್ಣಾಂಕ)sl_ತುದಿ - 1; i >= 0; i--) {
		ಆಗ (x->forward[i] != ಖಾಲಿ && x->forward[i]->abs_ticks <= abs_ticks) {
			x = x->forward[i];
		}
		update[i] = x;
	}
}

/* Walk equal-key nodes so identity, not just expiry, selects the splice points. */
ಸ್ಥಿರ ಶೂನ್ಯ skiplist_predecessors_ದ(ರಚನೆ _timeout *ಗಮ್ಯ, ರಚನೆ _timeout **update)
{
	ರಚನೆ _timeout *x = &sl_ತಲೆ;
	ಪೂರ್ಣಾಂಕ i;

	ಗಾಗಿ (i = (ಪೂರ್ಣಾಂಕ)sl_ತುದಿ - 1; i >= 0; i--) {
		ಆಗ (x->forward[i] != ಖಾಲಿ && x->forward[i]->abs_ticks < ಗಮ್ಯ->abs_ticks) {
			x = x->forward[i];
		}
		update[i] = x;
	}

	ಗಾಗಿ (i = 0; i < ಗಮ್ಯ->ಎತ್ತರ; i++) {
		ಆಗ (update[i]->forward[i] != ಖಾಲಿ &&
		       update[i]->forward[i] != ಗಮ್ಯ &&
		       update[i]->forward[i]->abs_ticks == ಗಮ್ಯ->abs_ticks) {
			update[i] = update[i]->forward[i];
		}
	}
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ bool z_timeout_q_ಸೇರಿಸು(ರಚನೆ _timeout *ಗಮ್ಯ, k_ticks_t dticks)
{
	uint8_t i;
	uint8_t ಎತ್ತರ;

	ಗಮ್ಯ->abs_ticks = (int64_t)curr_tick + dticks;
	ಎತ್ತರ = skiplist_random_ಎತ್ತರ();
	ಗಮ್ಯ->ಎತ್ತರ = ಎತ್ತರ;

	skiplist_update_init(sl_update);
	skiplist_predecessors_after(ಗಮ್ಯ->abs_ticks, sl_update);

	ಆದರೆ (ಎತ್ತರ > sl_ತುದಿ) {
		sl_ತುದಿ = ಎತ್ತರ;
	}

	ಗಾಗಿ (i = 0; i < ಎತ್ತರ; i++) {
		ಗಮ್ಯ->forward[i] = sl_update[i]->forward[i];
		sl_update[i]->forward[i] = ಗಮ್ಯ;
	}

	ಹಿಂತಿರುಗಿ skiplist_ಮೊದಲ() == ಗಮ್ಯ;
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ bool z_timeout_q_remove(ರಚನೆ _timeout *ಗಮ್ಯ)
{
	bool was_ಮೊದಲ = (skiplist_ಮೊದಲ() == ಗಮ್ಯ);

	__ASSERT_NO_MSG(ಗಮ್ಯ->ಎತ್ತರ > 0);

	skiplist_update_init(sl_update);
	skiplist_predecessors_ದ(ಗಮ್ಯ, sl_update);

	skiplist_unlink(ಗಮ್ಯ, sl_update);

	ಹಿಂತಿರುಗಿ was_ಮೊದಲ;
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ k_ticks_t z_timeout_q_remainder(ಸ್ಥಿರಾಂಕ ರಚನೆ _timeout *ಗಮ್ಯ)
{
	ಹಿಂತಿರುಗಿ (k_ticks_t)(ಗಮ್ಯ->abs_ticks - (int64_t)curr_tick);
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ k_ticks_t z_timeout_q_ಮುಂದಿನ_expiry(ಶೂನ್ಯ)
{
	ರಚನೆ _timeout *t = skiplist_ಮೊದಲ();
	int64_t gap;

	ಆದರೆ (t == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ K_TICKS_FOREVER;
	}

	gap = t->abs_ticks - (int64_t)curr_tick;
	ಹಿಂತಿರುಗಿ (gap < 0) ? 0 : (k_ticks_t)gap;
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ int32_t z_timeout_q_ಮುಂದಿನ_gap(ಶೂನ್ಯ)
{
	ರಚನೆ _timeout *t = skiplist_ಮೊದಲ();
	int64_t gap;

	ಆದರೆ (t == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ INT32_MAX;
	}

	/* Overdue must be 0: announce casts dt to uint32_t. */
	gap = t->abs_ticks - (int64_t)curr_tick;
	ಆದರೆ (gap <= 0) {
		ಹಿಂತಿರುಗಿ 0;
	}

	ಹಿಂತಿರುಗಿ (int32_t)MIN(gap, (int64_t)INT32_MAX);
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ z_timeout_q_advance(int32_t dt)
{
	ARG_ಬಳಸದ(dt);
}

/* Earliest node, so it is the head's successor at every level it occupies. */
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ರಚನೆ _timeout *z_timeout_q_ತೆಗೆ_due(ಶೂನ್ಯ)
{
	ರಚನೆ _timeout *t = skiplist_ಮೊದಲ();
	uint8_t i;

	ಆದರೆ ((t == ಖಾಲಿ) || (t->abs_ticks > (int64_t)curr_tick)) {
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}

	ಗಾಗಿ (i = 0; i < t->ಎತ್ತರ; i++) {
		__ASSERT_NO_MSG(sl_ತಲೆ.forward[i] == t);
		sl_ತಲೆ.forward[i] = t->forward[i];
	}

	t->ಎತ್ತರ = 0;
	skiplist_drop_ತುದಿ();

	ಹಿಂತಿರುಗಿ t;
}

#ಆದರೆ_ಅಂತ್ಯ /* ZEPHYR_KERNEL_TIMEOUT_SKIPLIST_H_ */
