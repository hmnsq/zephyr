/*
 * Copyright (c) 2018 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ ZEPHYR_KERNEL_TIMEOUT_LIST_H_
#ವ್ಯಾಖ್ಯಾನಿಸು ZEPHYR_KERNEL_TIMEOUT_LIST_H_

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

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ರಚನೆ _timeout *z_timeout_q_ಮೊದಲ(ಶೂನ್ಯ)
{
	sys_dnode_t *t = sys_dlist_peek_ತಲೆ(&timeout_list);

	ಹಿಂತಿರುಗಿ (t == ಖಾಲಿ) ? ಖಾಲಿ : CONTAINER_ದ(t, ರಚನೆ _timeout, ಗಂಟು);
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ರಚನೆ _timeout *z_timeout_q_ಮುಂದಿನ(ರಚನೆ _timeout *t)
{
	sys_dnode_t *n = sys_dlist_peek_ಮುಂದಿನ(&timeout_list, &t->ಗಂಟು);

	ಹಿಂತಿರುಗಿ (n == ಖಾಲಿ) ? ಖಾಲಿ : CONTAINER_ದ(n, ರಚನೆ _timeout, ಗಂಟು);
}

/* Insert @to so that it expires @dticks ticks after curr_tick. Returns true
 * if @to is now the earliest pending timeout.
 */
ಸ್ಥಿರ ಸಾಲೊಳಗೆ bool z_timeout_q_ಸೇರಿಸು(ರಚನೆ _timeout *ಗಮ್ಯ, k_ticks_t dticks)
{
	ರಚನೆ _timeout *t;

	ಗಮ್ಯ->dticks = dticks;
	ಗಾಗಿ (t = z_timeout_q_ಮೊದಲ(); t != ಖಾಲಿ; t = z_timeout_q_ಮುಂದಿನ(t)) {
		ಆದರೆ (t->dticks > ಗಮ್ಯ->dticks) {
			t->dticks -= ಗಮ್ಯ->dticks;
			sys_dlist_ಸೇರಿಸು(&t->ಗಂಟು, &ಗಮ್ಯ->ಗಂಟು);
			ಹಿಂತಿರುಗಿ z_timeout_q_ಮೊದಲ() == ಗಮ್ಯ;
		}
		ಗಮ್ಯ->dticks -= t->dticks;
	}

	sys_dlist_append(&timeout_list, &ಗಮ್ಯ->ಗಂಟು);
	ಹಿಂತಿರುಗಿ z_timeout_q_ಮೊದಲ() == ಗಮ್ಯ;
}

/* Remove an arbitrary timeout from the queue, absorbing its delta into the
 * successor so the remaining deltas stay valid. Returns true if @to was the
 * earliest pending timeout. Used only by the abort path, never from announce.
 */
ಸ್ಥಿರ ಸಾಲೊಳಗೆ bool z_timeout_q_remove(ರಚನೆ _timeout *ಗಮ್ಯ)
{
	bool was_ಮೊದಲ = (z_timeout_q_ಮೊದಲ() == ಗಮ್ಯ);
	ರಚನೆ _timeout *n = z_timeout_q_ಮುಂದಿನ(ಗಮ್ಯ);

	ಆದರೆ (n != ಖಾಲಿ) {
		n->dticks += ಗಮ್ಯ->dticks;
	}
	sys_dlist_remove(&ಗಮ್ಯ->ಗಂಟು);

	ಹಿಂತಿರುಗಿ was_ಮೊದಲ;
}

/* Ticks from curr_tick until @to expires. Caller holds timeout_lock and has
 * verified that @to is active.
 */
ಸ್ಥಿರ ಸಾಲೊಳಗೆ k_ticks_t z_timeout_q_remainder(ಸ್ಥಿರಾಂಕ ರಚನೆ _timeout *ಗಮ್ಯ)
{
	k_ticks_t ticks = 0;

	ಗಾಗಿ (ರಚನೆ _timeout *t = z_timeout_q_ಮೊದಲ(); t != ಖಾಲಿ;
	     t = z_timeout_q_ಮುಂದಿನ(t)) {
		ticks += t->dticks;
		ಆದರೆ (ಗಮ್ಯ == t) {
			ನಿಲ್ಲಿಸು;
		}
	}

	ಹಿಂತಿರುಗಿ ticks;
}

/* Ticks from (curr_tick + ticks_elapsed) until the next expiry, clamped to
 * int32 / SYS_CLOCK_MAX_WAIT. Empty queue yields SYS_CLOCK_MAX_WAIT.
 */
/* Ticks from curr_tick to the earliest pending timeout, or K_TICKS_FOREVER if
 * the queue is empty. The announce-range cap is applied centrally by
 * next_timeout() in timeout.c.
 */
ಸ್ಥಿರ ಸಾಲೊಳಗೆ k_ticks_t z_timeout_q_ಮುಂದಿನ_expiry(ಶೂನ್ಯ)
{
	ರಚನೆ _timeout *ಗಮ್ಯ = z_timeout_q_ಮೊದಲ();

	ಹಿಂತಿರುಗಿ (ಗಮ್ಯ == ಖಾಲಿ) ? K_TICKS_FOREVER : ಗಮ್ಯ->dticks;
}

/* --- announce-loop helpers (see sys_clock_announce_locked() in timeout.c) --- */

/* Ticks from curr_tick until the next event the announce loop must act on
 * (here: the earliest expiry). A value larger than any announce window when
 * the queue is empty.
 */
ಸ್ಥಿರ ಸಾಲೊಳಗೆ int32_t z_timeout_q_ಮುಂದಿನ_gap(ಶೂನ್ಯ)
{
	ರಚನೆ _timeout *t = z_timeout_q_ಮೊದಲ();

	ಹಿಂತಿರುಗಿ (t == ಖಾಲಿ) ? INT32_MAX : (int32_t)MIN((int64_t)t->dticks, INT32_MAX);
}

/* Advance backend state by @dt ticks. For the delta list this shifts the head
 * closer to expiry; a head whose gap is fully consumed reads dticks == 0 and
 * is then drained by z_timeout_q_pop_due(). The caller advances curr_tick.
 */
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ z_timeout_q_advance(int32_t dt)
{
	ರಚನೆ _timeout *t = z_timeout_q_ಮೊದಲ();

	ಆದರೆ (t != ಖಾಲಿ) {
		t->dticks -= dt;
	}
}

/* Pop and return one timeout that is due at the current curr_tick, or NULL if
 * none is due right now. Strict head-only removal with no delta absorption:
 * z_timeout_q_advance() has already brought the head's dticks to 0, and
 * curr_tick was advanced to match, so the successor's stored delta stays
 * valid without propagation.
 */
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ರಚನೆ _timeout *z_timeout_q_ತೆಗೆ_due(ಶೂನ್ಯ)
{
	ರಚನೆ _timeout *t = z_timeout_q_ಮೊದಲ();

	ಆದರೆ ((t == ಖಾಲಿ) || (t->dticks != 0)) {
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}

	sys_dlist_remove(&t->ಗಂಟು);

	ಹಿಂತಿರುಗಿ t;
}

#ಆದರೆ_ಅಂತ್ಯ /* ZEPHYR_KERNEL_TIMEOUT_LIST_H_ */
