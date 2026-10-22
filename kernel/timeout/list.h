/*
 * Copyright (c) 2018 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_KERNEL_TIMEOUT_LIST_H_
#define ZEPHYR_KERNEL_TIMEOUT_LIST_H_

/**
 * @file
 * @brief Sorted delta-list timeout backend (implementation).
 *
 * This is the original (and default) timeout queue: a doubly linked list
 * sorted by expiry, where each node stores its expiry as a delta (dticks)
 * relative to its predecessor. Insertion is O(n), head removal is O(1).
 *
 * Included only by kernel/timeout.c, after its shared state, so the queue
 * instance and the z_timeout_q_*() operations below are private to that
 * translation unit. The per-node helpers live in kernel/include/timeout_q.h.
 */

#ಅನ್ತರ್ಭಾವ <zephyr/sys/dlist.h>

ಸ್ಥಿರ sys_dlist_t timeout_list = SYS_DLIST_STATIC_INIT(&timeout_list);

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಸಂರಚನಾ _timeout *z_timeout_q_first(ಶೂನ್ಯ)
{
	sys_dnode_t *t = sys_dlist_peek_head(&timeout_list);

	ಪ್ರತ್ಯಾಗಮ (t == ರಿಕ್ತ) ? ರಿಕ್ತ : CONTAINER_OF(t, ಸಂರಚನಾ _timeout, ಗಂಟು);
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಸಂರಚನಾ _timeout *z_timeout_q_next(ಸಂರಚನಾ _timeout *t)
{
	sys_dnode_t *n = sys_dlist_peek_next(&timeout_list, &t->ಗಂಟು);

	ಪ್ರತ್ಯಾಗಮ (n == ರಿಕ್ತ) ? ರಿಕ್ತ : CONTAINER_OF(n, ಸಂರಚನಾ _timeout, ಗಂಟು);
}

/* Insert @to so that it expires @dticks ticks after curr_tick. Returns true
 * if @to is now the earliest pending timeout.
 */
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ bool z_timeout_q_ಸೇರಿಸು(ಸಂರಚನಾ _timeout *to, k_ticks_t dticks)
{
	ಸಂರಚನಾ _timeout *t;

	to->dticks = dticks;
	ಪ್ರತಿ (t = z_timeout_q_first(); t != ರಿಕ್ತ; t = z_timeout_q_next(t)) {
		ಯದಿ (t->dticks > to->dticks) {
			t->dticks -= to->dticks;
			sys_dlist_ಸೇರಿಸು(&t->ಗಂಟು, &to->ಗಂಟು);
			ಪ್ರತ್ಯಾಗಮ z_timeout_q_first() == to;
		}
		to->dticks -= t->dticks;
	}

	sys_dlist_append(&timeout_list, &to->ಗಂಟು);
	ಪ್ರತ್ಯಾಗಮ z_timeout_q_first() == to;
}

/* Remove an arbitrary timeout from the queue, absorbing its delta into the
 * successor so the remaining deltas stay valid. Returns true if @to was the
 * earliest pending timeout. Used only by the abort path, never from announce.
 */
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ bool z_timeout_q_remove(ಸಂರಚನಾ _timeout *to)
{
	bool was_first = (z_timeout_q_first() == to);
	ಸಂರಚನಾ _timeout *n = z_timeout_q_next(to);

	ಯದಿ (n != ರಿಕ್ತ) {
		n->dticks += to->dticks;
	}
	sys_dlist_remove(&to->ಗಂಟು);

	ಪ್ರತ್ಯಾಗಮ was_first;
}

/* Ticks from curr_tick until @to expires. Caller holds timeout_lock and has
 * verified that @to is active.
 */
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ k_ticks_t z_timeout_q_remainder(ಧ್ರುವ ಸಂರಚನಾ _timeout *to)
{
	k_ticks_t ticks = 0;

	ಪ್ರತಿ (ಸಂರಚನಾ _timeout *t = z_timeout_q_first(); t != ರಿಕ್ತ;
	     t = z_timeout_q_next(t)) {
		ticks += t->dticks;
		ಯದಿ (to == t) {
			ಭಙ್ಗ;
		}
	}

	ಪ್ರತ್ಯಾಗಮ ticks;
}

/* Ticks from (curr_tick + ticks_elapsed) until the next expiry, clamped to
 * int32 / SYS_CLOCK_MAX_WAIT. Empty queue yields SYS_CLOCK_MAX_WAIT.
 */
/* Ticks from curr_tick to the earliest pending timeout, or K_TICKS_FOREVER if
 * the queue is empty. The announce-range cap is applied centrally by
 * next_timeout() in timeout.c.
 */
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ k_ticks_t z_timeout_q_next_expiry(ಶೂನ್ಯ)
{
	ಸಂರಚನಾ _timeout *to = z_timeout_q_first();

	ಪ್ರತ್ಯಾಗಮ (to == ರಿಕ್ತ) ? K_TICKS_FOREVER : to->dticks;
}

/* --- announce-loop helpers (see sys_clock_announce_locked() in timeout.c) --- */

/* Ticks from curr_tick until the next event the announce loop must act on
 * (here: the earliest expiry). A value larger than any announce window when
 * the queue is empty.
 */
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ int32_t z_timeout_q_next_gap(ಶೂನ್ಯ)
{
	ಸಂರಚನಾ _timeout *t = z_timeout_q_first();

	ಪ್ರತ್ಯಾಗಮ (t == ರಿಕ್ತ) ? INT32_MAX : (int32_t)MIN((int64_t)t->dticks, INT32_MAX);
}

/* Advance backend state by @dt ticks. For the delta list this shifts the head
 * closer to expiry; a head whose gap is fully consumed reads dticks == 0 and
 * is then drained by z_timeout_q_pop_due(). The caller advances curr_tick.
 */
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ z_timeout_q_advance(int32_t dt)
{
	ಸಂರಚನಾ _timeout *t = z_timeout_q_first();

	ಯದಿ (t != ರಿಕ್ತ) {
		t->dticks -= dt;
	}
}

/* Pop and return one timeout that is due at the current curr_tick, or NULL if
 * none is due right now. Strict head-only removal with no delta absorption:
 * z_timeout_q_advance() has already brought the head's dticks to 0, and
 * curr_tick was advanced to match, so the successor's stored delta stays
 * valid without propagation.
 */
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಸಂರಚನಾ _timeout *z_timeout_q_ತೆಗೆ_due(ಶೂನ್ಯ)
{
	ಸಂರಚನಾ _timeout *t = z_timeout_q_first();

	ಯದಿ ((t == ರಿಕ್ತ) || (t->dticks != 0)) {
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}

	sys_dlist_remove(&t->ಗಂಟು);

	ಪ್ರತ್ಯಾಗಮ t;
}

#endif /* ZEPHYR_KERNEL_TIMEOUT_LIST_H_ */
