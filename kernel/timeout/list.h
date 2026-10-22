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

#അന്തര്ഭാവ <zephyr/sys/dlist.h>

സ്ഥിതം sys_dlist_t timeout_list = SYS_DLIST_STATIC_INIT(&timeout_list);

സ്ഥിതം വരിയിൽ ഘടന _timeout *z_timeout_q_first(ശൂന്യം)
{
	sys_dnode_t *t = sys_dlist_peek_head(&timeout_list);

	തിരികെ (t == ശൂന്യ) ? ശൂന്യ : CONTAINER_OF(t, ഘടന _timeout, കണ്ണി);
}

സ്ഥിതം വരിയിൽ ഘടന _timeout *z_timeout_q_next(ഘടന _timeout *t)
{
	sys_dnode_t *n = sys_dlist_peek_next(&timeout_list, &t->കണ്ണി);

	തിരികെ (n == ശൂന്യ) ? ശൂന്യ : CONTAINER_OF(n, ഘടന _timeout, കണ്ണി);
}

/* Insert @to so that it expires @dticks ticks after curr_tick. Returns true
 * if @to is now the earliest pending timeout.
 */
സ്ഥിതം വരിയിൽ bool z_timeout_q_തിരുകുക(ഘടന _timeout *to, k_ticks_t dticks)
{
	ഘടന _timeout *t;

	to->dticks = dticks;
	വേണ്ടി (t = z_timeout_q_first(); t != ശൂന്യ; t = z_timeout_q_next(t)) {
		എങ്കിൽ (t->dticks > to->dticks) {
			t->dticks -= to->dticks;
			sys_dlist_തിരുകുക(&t->കണ്ണി, &to->കണ്ണി);
			തിരികെ z_timeout_q_first() == to;
		}
		to->dticks -= t->dticks;
	}

	sys_dlist_append(&timeout_list, &to->കണ്ണി);
	തിരികെ z_timeout_q_first() == to;
}

/* Remove an arbitrary timeout from the queue, absorbing its delta into the
 * successor so the remaining deltas stay valid. Returns true if @to was the
 * earliest pending timeout. Used only by the abort path, never from announce.
 */
സ്ഥിതം വരിയിൽ bool z_timeout_q_remove(ഘടന _timeout *to)
{
	bool was_first = (z_timeout_q_first() == to);
	ഘടന _timeout *n = z_timeout_q_next(to);

	എങ്കിൽ (n != ശൂന്യ) {
		n->dticks += to->dticks;
	}
	sys_dlist_remove(&to->കണ്ണി);

	തിരികെ was_first;
}

/* Ticks from curr_tick until @to expires. Caller holds timeout_lock and has
 * verified that @to is active.
 */
സ്ഥിതം വരിയിൽ k_ticks_t z_timeout_q_remainder(സ്ഥിരം ഘടന _timeout *to)
{
	k_ticks_t ticks = 0;

	വേണ്ടി (ഘടന _timeout *t = z_timeout_q_first(); t != ശൂന്യ;
	     t = z_timeout_q_next(t)) {
		ticks += t->dticks;
		എങ്കിൽ (to == t) {
			നിർത്തുക;
		}
	}

	തിരികെ ticks;
}

/* Ticks from (curr_tick + ticks_elapsed) until the next expiry, clamped to
 * int32 / SYS_CLOCK_MAX_WAIT. Empty queue yields SYS_CLOCK_MAX_WAIT.
 */
/* Ticks from curr_tick to the earliest pending timeout, or K_TICKS_FOREVER if
 * the queue is empty. The announce-range cap is applied centrally by
 * next_timeout() in timeout.c.
 */
സ്ഥിതം വരിയിൽ k_ticks_t z_timeout_q_next_expiry(ശൂന്യം)
{
	ഘടന _timeout *to = z_timeout_q_first();

	തിരികെ (to == ശൂന്യ) ? K_TICKS_FOREVER : to->dticks;
}

/* --- announce-loop helpers (see sys_clock_announce_locked() in timeout.c) --- */

/* Ticks from curr_tick until the next event the announce loop must act on
 * (here: the earliest expiry). A value larger than any announce window when
 * the queue is empty.
 */
സ്ഥിതം വരിയിൽ int32_t z_timeout_q_next_gap(ശൂന്യം)
{
	ഘടന _timeout *t = z_timeout_q_first();

	തിരികെ (t == ശൂന്യ) ? INT32_MAX : (int32_t)MIN((int64_t)t->dticks, INT32_MAX);
}

/* Advance backend state by @dt ticks. For the delta list this shifts the head
 * closer to expiry; a head whose gap is fully consumed reads dticks == 0 and
 * is then drained by z_timeout_q_pop_due(). The caller advances curr_tick.
 */
സ്ഥിതം വരിയിൽ ശൂന്യം z_timeout_q_advance(int32_t dt)
{
	ഘടന _timeout *t = z_timeout_q_first();

	എങ്കിൽ (t != ശൂന്യ) {
		t->dticks -= dt;
	}
}

/* Pop and return one timeout that is due at the current curr_tick, or NULL if
 * none is due right now. Strict head-only removal with no delta absorption:
 * z_timeout_q_advance() has already brought the head's dticks to 0, and
 * curr_tick was advanced to match, so the successor's stored delta stays
 * valid without propagation.
 */
സ്ഥിതം വരിയിൽ ഘടന _timeout *z_timeout_q_എടുക്കുക_due(ശൂന്യം)
{
	ഘടന _timeout *t = z_timeout_q_first();

	എങ്കിൽ ((t == ശൂന്യ) || (t->dticks != 0)) {
		തിരികെ ശൂന്യ;
	}

	sys_dlist_remove(&t->കണ്ണി);

	തിരികെ t;
}

#endif /* ZEPHYR_KERNEL_TIMEOUT_LIST_H_ */
