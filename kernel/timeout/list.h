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

#અન્તર્ભાવ <zephyr/sys/dlist.h>

સ્થિર sys_dlist_t timeout_list = SYS_DLIST_STATIC_INIT(&timeout_list);

સ્થિર અંતઃરેખ સંરચના _timeout *z_timeout_q_first(શૂન્ય)
{
	sys_dnode_t *t = sys_dlist_peek_head(&timeout_list);

	પાછા (t == ખાલી) ? ખાલી : CONTAINER_OF(t, સંરચના _timeout, ગાંઠ);
}

સ્થિર અંતઃરેખ સંરચના _timeout *z_timeout_q_next(સંરચના _timeout *t)
{
	sys_dnode_t *n = sys_dlist_peek_next(&timeout_list, &t->ગાંઠ);

	પાછા (n == ખાલી) ? ખાલી : CONTAINER_OF(n, સંરચના _timeout, ગાંઠ);
}

/* Insert @to so that it expires @dticks ticks after curr_tick. Returns true
 * if @to is now the earliest pending timeout.
 */
સ્થિર અંતઃરેખ bool z_timeout_q_ઉમેરો(સંરચના _timeout *to, k_ticks_t dticks)
{
	સંરચના _timeout *t;

	to->dticks = dticks;
	માટે (t = z_timeout_q_first(); t != ખાલી; t = z_timeout_q_next(t)) {
		જો (t->dticks > to->dticks) {
			t->dticks -= to->dticks;
			sys_dlist_ઉમેરો(&t->ગાંઠ, &to->ગાંઠ);
			પાછા z_timeout_q_first() == to;
		}
		to->dticks -= t->dticks;
	}

	sys_dlist_append(&timeout_list, &to->ગાંઠ);
	પાછા z_timeout_q_first() == to;
}

/* Remove an arbitrary timeout from the queue, absorbing its delta into the
 * successor so the remaining deltas stay valid. Returns true if @to was the
 * earliest pending timeout. Used only by the abort path, never from announce.
 */
સ્થિર અંતઃરેખ bool z_timeout_q_remove(સંરચના _timeout *to)
{
	bool was_first = (z_timeout_q_first() == to);
	સંરચના _timeout *n = z_timeout_q_next(to);

	જો (n != ખાલી) {
		n->dticks += to->dticks;
	}
	sys_dlist_remove(&to->ગાંઠ);

	પાછા was_first;
}

/* Ticks from curr_tick until @to expires. Caller holds timeout_lock and has
 * verified that @to is active.
 */
સ્થિર અંતઃરેખ k_ticks_t z_timeout_q_remainder(અચલ સંરચના _timeout *to)
{
	k_ticks_t ticks = 0;

	માટે (સંરચના _timeout *t = z_timeout_q_first(); t != ખાલી;
	     t = z_timeout_q_next(t)) {
		ticks += t->dticks;
		જો (to == t) {
			તોડો;
		}
	}

	પાછા ticks;
}

/* Ticks from (curr_tick + ticks_elapsed) until the next expiry, clamped to
 * int32 / SYS_CLOCK_MAX_WAIT. Empty queue yields SYS_CLOCK_MAX_WAIT.
 */
/* Ticks from curr_tick to the earliest pending timeout, or K_TICKS_FOREVER if
 * the queue is empty. The announce-range cap is applied centrally by
 * next_timeout() in timeout.c.
 */
સ્થિર અંતઃરેખ k_ticks_t z_timeout_q_next_expiry(શૂન્ય)
{
	સંરચના _timeout *to = z_timeout_q_first();

	પાછા (to == ખાલી) ? K_TICKS_FOREVER : to->dticks;
}

/* --- announce-loop helpers (see sys_clock_announce_locked() in timeout.c) --- */

/* Ticks from curr_tick until the next event the announce loop must act on
 * (here: the earliest expiry). A value larger than any announce window when
 * the queue is empty.
 */
સ્થિર અંતઃરેખ int32_t z_timeout_q_next_gap(શૂન્ય)
{
	સંરચના _timeout *t = z_timeout_q_first();

	પાછા (t == ખાલી) ? INT32_MAX : (int32_t)MIN((int64_t)t->dticks, INT32_MAX);
}

/* Advance backend state by @dt ticks. For the delta list this shifts the head
 * closer to expiry; a head whose gap is fully consumed reads dticks == 0 and
 * is then drained by z_timeout_q_pop_due(). The caller advances curr_tick.
 */
સ્થિર અંતઃરેખ શૂન્ય z_timeout_q_advance(int32_t dt)
{
	સંરચના _timeout *t = z_timeout_q_first();

	જો (t != ખાલી) {
		t->dticks -= dt;
	}
}

/* Pop and return one timeout that is due at the current curr_tick, or NULL if
 * none is due right now. Strict head-only removal with no delta absorption:
 * z_timeout_q_advance() has already brought the head's dticks to 0, and
 * curr_tick was advanced to match, so the successor's stored delta stays
 * valid without propagation.
 */
સ્થિર અંતઃરેખ સંરચના _timeout *z_timeout_q_કાઢો_due(શૂન્ય)
{
	સંરચના _timeout *t = z_timeout_q_first();

	જો ((t == ખાલી) || (t->dticks != 0)) {
		પાછા ખાલી;
	}

	sys_dlist_remove(&t->ગાંઠ);

	પાછા t;
}

#endif /* ZEPHYR_KERNEL_TIMEOUT_LIST_H_ */
