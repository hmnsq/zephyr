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

#অন্তর্ভাব <zephyr/sys/dlist.h>

স্থির sys_dlist_t timeout_list = SYS_DLIST_STATIC_INIT(&timeout_list);

স্থির অন্তঃরেখ কাঠামো _timeout *z_timeout_q_first(শূন্য)
{
	sys_dnode_t *t = sys_dlist_peek_শীর্ষ(&timeout_list);

	ফেরত (t == ফাঁকা) ? ফাঁকা : CONTAINER_OF(t, কাঠামো _timeout, গ্রন্থি);
}

স্থির অন্তঃরেখ কাঠামো _timeout *z_timeout_q_পরবর্তী(কাঠামো _timeout *t)
{
	sys_dnode_t *n = sys_dlist_peek_পরবর্তী(&timeout_list, &t->গ্রন্থি);

	ফেরত (n == ফাঁকা) ? ফাঁকা : CONTAINER_OF(n, কাঠামো _timeout, গ্রন্থি);
}

/* Insert @to so that it expires @dticks ticks after curr_tick. Returns true
 * if @to is now the earliest pending timeout.
 */
স্থির অন্তঃরেখ bool z_timeout_q_ঢোকাও(কাঠামো _timeout *to, k_ticks_t dticks)
{
	কাঠামো _timeout *t;

	to->dticks = dticks;
	জন্য (t = z_timeout_q_first(); t != ফাঁকা; t = z_timeout_q_পরবর্তী(t)) {
		যদি (t->dticks > to->dticks) {
			t->dticks -= to->dticks;
			sys_dlist_ঢোকাও(&t->গ্রন্থি, &to->গ্রন্থি);
			ফেরত z_timeout_q_first() == to;
		}
		to->dticks -= t->dticks;
	}

	sys_dlist_append(&timeout_list, &to->গ্রন্থি);
	ফেরত z_timeout_q_first() == to;
}

/* Remove an arbitrary timeout from the queue, absorbing its delta into the
 * successor so the remaining deltas stay valid. Returns true if @to was the
 * earliest pending timeout. Used only by the abort path, never from announce.
 */
স্থির অন্তঃরেখ bool z_timeout_q_remove(কাঠামো _timeout *to)
{
	bool was_first = (z_timeout_q_first() == to);
	কাঠামো _timeout *n = z_timeout_q_পরবর্তী(to);

	যদি (n != ফাঁকা) {
		n->dticks += to->dticks;
	}
	sys_dlist_remove(&to->গ্রন্থি);

	ফেরত was_first;
}

/* Ticks from curr_tick until @to expires. Caller holds timeout_lock and has
 * verified that @to is active.
 */
স্থির অন্তঃরেখ k_ticks_t z_timeout_q_remainder(ধ্রুবক কাঠামো _timeout *to)
{
	k_ticks_t ticks = 0;

	জন্য (কাঠামো _timeout *t = z_timeout_q_first(); t != ফাঁকা;
	     t = z_timeout_q_পরবর্তী(t)) {
		ticks += t->dticks;
		যদি (to == t) {
			বিরতি;
		}
	}

	ফেরত ticks;
}

/* Ticks from (curr_tick + ticks_elapsed) until the next expiry, clamped to
 * int32 / SYS_CLOCK_MAX_WAIT. Empty queue yields SYS_CLOCK_MAX_WAIT.
 */
/* Ticks from curr_tick to the earliest pending timeout, or K_TICKS_FOREVER if
 * the queue is empty. The announce-range cap is applied centrally by
 * next_timeout() in timeout.c.
 */
স্থির অন্তঃরেখ k_ticks_t z_timeout_q_পরবর্তী_expiry(শূন্য)
{
	কাঠামো _timeout *to = z_timeout_q_first();

	ফেরত (to == ফাঁকা) ? K_TICKS_FOREVER : to->dticks;
}

/* --- announce-loop helpers (see sys_clock_announce_locked() in timeout.c) --- */

/* Ticks from curr_tick until the next event the announce loop must act on
 * (here: the earliest expiry). A value larger than any announce window when
 * the queue is empty.
 */
স্থির অন্তঃরেখ int32_t z_timeout_q_পরবর্তী_gap(শূন্য)
{
	কাঠামো _timeout *t = z_timeout_q_first();

	ফেরত (t == ফাঁকা) ? INT32_MAX : (int32_t)MIN((int64_t)t->dticks, INT32_MAX);
}

/* Advance backend state by @dt ticks. For the delta list this shifts the head
 * closer to expiry; a head whose gap is fully consumed reads dticks == 0 and
 * is then drained by z_timeout_q_pop_due(). The caller advances curr_tick.
 */
স্থির অন্তঃরেখ শূন্য z_timeout_q_advance(int32_t dt)
{
	কাঠামো _timeout *t = z_timeout_q_first();

	যদি (t != ফাঁকা) {
		t->dticks -= dt;
	}
}

/* Pop and return one timeout that is due at the current curr_tick, or NULL if
 * none is due right now. Strict head-only removal with no delta absorption:
 * z_timeout_q_advance() has already brought the head's dticks to 0, and
 * curr_tick was advanced to match, so the successor's stored delta stays
 * valid without propagation.
 */
স্থির অন্তঃরেখ কাঠামো _timeout *z_timeout_q_বের_করো_due(শূন্য)
{
	কাঠামো _timeout *t = z_timeout_q_first();

	যদি ((t == ফাঁকা) || (t->dticks != 0)) {
		ফেরত ফাঁকা;
	}

	sys_dlist_remove(&t->গ্রন্থি);

	ফেরত t;
}

#endif /* ZEPHYR_KERNEL_TIMEOUT_LIST_H_ */
