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

#অন্তর্ভাৱ <zephyr/sys/dlist.h>

স্থির sys_dlist_t timeout_list = SYS_DLIST_STATIC_INIT(&timeout_list);

স্থির অন্তঃস্থ সংরচনা _timeout *z_timeout_q_first(শূন্য)
{
	sys_dnode_t *t = sys_dlist_peek_head(&timeout_list);

	ঘূৰাই (t == রিক্ত) ? রিক্ত : CONTAINER_OF(t, সংরচনা _timeout, গ্ৰন্থি);
}

স্থির অন্তঃস্থ সংরচনা _timeout *z_timeout_q_next(সংরচনা _timeout *t)
{
	sys_dnode_t *n = sys_dlist_peek_next(&timeout_list, &t->গ্ৰন্থি);

	ঘূৰাই (n == রিক্ত) ? রিক্ত : CONTAINER_OF(n, সংরচনা _timeout, গ্ৰন্থি);
}

/* Insert @to so that it expires @dticks ticks after curr_tick. Returns true
 * if @to is now the earliest pending timeout.
 */
স্থির অন্তঃস্থ bool z_timeout_q_ঢোকাও(সংরচনা _timeout *to, k_ticks_t dticks)
{
	সংরচনা _timeout *t;

	to->dticks = dticks;
	বাবে (t = z_timeout_q_first(); t != রিক্ত; t = z_timeout_q_next(t)) {
		যদি (t->dticks > to->dticks) {
			t->dticks -= to->dticks;
			sys_dlist_ঢোকাও(&t->গ্ৰন্থি, &to->গ্ৰন্থি);
			ঘূৰাই z_timeout_q_first() == to;
		}
		to->dticks -= t->dticks;
	}

	sys_dlist_append(&timeout_list, &to->গ্ৰন্থি);
	ঘূৰাই z_timeout_q_first() == to;
}

/* Remove an arbitrary timeout from the queue, absorbing its delta into the
 * successor so the remaining deltas stay valid. Returns true if @to was the
 * earliest pending timeout. Used only by the abort path, never from announce.
 */
স্থির অন্তঃস্থ bool z_timeout_q_remove(সংরচনা _timeout *to)
{
	bool was_first = (z_timeout_q_first() == to);
	সংরচনা _timeout *n = z_timeout_q_next(to);

	যদি (n != রিক্ত) {
		n->dticks += to->dticks;
	}
	sys_dlist_remove(&to->গ্ৰন্থি);

	ঘূৰাই was_first;
}

/* Ticks from curr_tick until @to expires. Caller holds timeout_lock and has
 * verified that @to is active.
 */
স্থির অন্তঃস্থ k_ticks_t z_timeout_q_remainder(ধ্রুৱ সংরচনা _timeout *to)
{
	k_ticks_t ticks = 0;

	বাবে (সংরচনা _timeout *t = z_timeout_q_first(); t != রিক্ত;
	     t = z_timeout_q_next(t)) {
		ticks += t->dticks;
		যদি (to == t) {
			ভঙ্গ;
		}
	}

	ঘূৰাই ticks;
}

/* Ticks from (curr_tick + ticks_elapsed) until the next expiry, clamped to
 * int32 / SYS_CLOCK_MAX_WAIT. Empty queue yields SYS_CLOCK_MAX_WAIT.
 */
/* Ticks from curr_tick to the earliest pending timeout, or K_TICKS_FOREVER if
 * the queue is empty. The announce-range cap is applied centrally by
 * next_timeout() in timeout.c.
 */
স্থির অন্তঃস্থ k_ticks_t z_timeout_q_next_expiry(শূন্য)
{
	সংরচনা _timeout *to = z_timeout_q_first();

	ঘূৰাই (to == রিক্ত) ? K_TICKS_FOREVER : to->dticks;
}

/* --- announce-loop helpers (see sys_clock_announce_locked() in timeout.c) --- */

/* Ticks from curr_tick until the next event the announce loop must act on
 * (here: the earliest expiry). A value larger than any announce window when
 * the queue is empty.
 */
স্থির অন্তঃস্থ int32_t z_timeout_q_next_gap(শূন্য)
{
	সংরচনা _timeout *t = z_timeout_q_first();

	ঘূৰাই (t == রিক্ত) ? INT32_MAX : (int32_t)MIN((int64_t)t->dticks, INT32_MAX);
}

/* Advance backend state by @dt ticks. For the delta list this shifts the head
 * closer to expiry; a head whose gap is fully consumed reads dticks == 0 and
 * is then drained by z_timeout_q_pop_due(). The caller advances curr_tick.
 */
স্থির অন্তঃস্থ শূন্য z_timeout_q_advance(int32_t dt)
{
	সংরচনা _timeout *t = z_timeout_q_first();

	যদি (t != রিক্ত) {
		t->dticks -= dt;
	}
}

/* Pop and return one timeout that is due at the current curr_tick, or NULL if
 * none is due right now. Strict head-only removal with no delta absorption:
 * z_timeout_q_advance() has already brought the head's dticks to 0, and
 * curr_tick was advanced to match, so the successor's stored delta stays
 * valid without propagation.
 */
স্থির অন্তঃস্থ সংরচনা _timeout *z_timeout_q_বেৰ_কৰো_due(শূন্য)
{
	সংরচনা _timeout *t = z_timeout_q_first();

	যদি ((t == রিক্ত) || (t->dticks != 0)) {
		ঘূৰাই রিক্ত;
	}

	sys_dlist_remove(&t->গ্ৰন্থি);

	ঘূৰাই t;
}

#endif /* ZEPHYR_KERNEL_TIMEOUT_LIST_H_ */
