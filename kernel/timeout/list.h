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

#ଅନ୍ତର୍ଭାଵ <zephyr/sys/dlist.h>

ସ୍ଥିର sys_dlist_t timeout_list = SYS_DLIST_STATIC_INIT(&timeout_list);

ସ୍ଥିର ଧାଡ଼ିଭିତରେ ସଂରଚନା _timeout *z_timeout_q_first(ଶୂନ୍ୟ)
{
	sys_dnode_t *t = sys_dlist_peek_head(&timeout_list);

	ଫେରନ୍ତୁ (t == ଖାଲି) ? ଖାଲି : CONTAINER_OF(t, ସଂରଚନା _timeout, ଗଣ୍ଠି);
}

ସ୍ଥିର ଧାଡ଼ିଭିତରେ ସଂରଚନା _timeout *z_timeout_q_next(ସଂରଚନା _timeout *t)
{
	sys_dnode_t *n = sys_dlist_peek_next(&timeout_list, &t->ଗଣ୍ଠି);

	ଫେରନ୍ତୁ (n == ଖାଲି) ? ଖାଲି : CONTAINER_OF(n, ସଂରଚନା _timeout, ଗଣ୍ଠି);
}

/* Insert @to so that it expires @dticks ticks after curr_tick. Returns true
 * if @to is now the earliest pending timeout.
 */
ସ୍ଥିର ଧାଡ଼ିଭିତରେ bool z_timeout_q_ଭର୍ତ୍ତି(ସଂରଚନା _timeout *to, k_ticks_t dticks)
{
	ସଂରଚନା _timeout *t;

	to->dticks = dticks;
	ପାଇଁ (t = z_timeout_q_first(); t != ଖାଲି; t = z_timeout_q_next(t)) {
		ଯଦି (t->dticks > to->dticks) {
			t->dticks -= to->dticks;
			sys_dlist_ଭର୍ତ୍ତି(&t->ଗଣ୍ଠି, &to->ଗଣ୍ଠି);
			ଫେରନ୍ତୁ z_timeout_q_first() == to;
		}
		to->dticks -= t->dticks;
	}

	sys_dlist_append(&timeout_list, &to->ଗଣ୍ଠି);
	ଫେରନ୍ତୁ z_timeout_q_first() == to;
}

/* Remove an arbitrary timeout from the queue, absorbing its delta into the
 * successor so the remaining deltas stay valid. Returns true if @to was the
 * earliest pending timeout. Used only by the abort path, never from announce.
 */
ସ୍ଥିର ଧାଡ଼ିଭିତରେ bool z_timeout_q_remove(ସଂରଚନା _timeout *to)
{
	bool was_first = (z_timeout_q_first() == to);
	ସଂରଚନା _timeout *n = z_timeout_q_next(to);

	ଯଦି (n != ଖାଲି) {
		n->dticks += to->dticks;
	}
	sys_dlist_remove(&to->ଗଣ୍ଠି);

	ଫେରନ୍ତୁ was_first;
}

/* Ticks from curr_tick until @to expires. Caller holds timeout_lock and has
 * verified that @to is active.
 */
ସ୍ଥିର ଧାଡ଼ିଭିତରେ k_ticks_t z_timeout_q_remainder(ଅଚଳ ସଂରଚନା _timeout *to)
{
	k_ticks_t ticks = 0;

	ପାଇଁ (ସଂରଚନା _timeout *t = z_timeout_q_first(); t != ଖାଲି;
	     t = z_timeout_q_next(t)) {
		ticks += t->dticks;
		ଯଦି (to == t) {
			ଭାଙ୍ଗ;
		}
	}

	ଫେରନ୍ତୁ ticks;
}

/* Ticks from (curr_tick + ticks_elapsed) until the next expiry, clamped to
 * int32 / SYS_CLOCK_MAX_WAIT. Empty queue yields SYS_CLOCK_MAX_WAIT.
 */
/* Ticks from curr_tick to the earliest pending timeout, or K_TICKS_FOREVER if
 * the queue is empty. The announce-range cap is applied centrally by
 * next_timeout() in timeout.c.
 */
ସ୍ଥିର ଧାଡ଼ିଭିତରେ k_ticks_t z_timeout_q_next_expiry(ଶୂନ୍ୟ)
{
	ସଂରଚନା _timeout *to = z_timeout_q_first();

	ଫେରନ୍ତୁ (to == ଖାଲି) ? K_TICKS_FOREVER : to->dticks;
}

/* --- announce-loop helpers (see sys_clock_announce_locked() in timeout.c) --- */

/* Ticks from curr_tick until the next event the announce loop must act on
 * (here: the earliest expiry). A value larger than any announce window when
 * the queue is empty.
 */
ସ୍ଥିର ଧାଡ଼ିଭିତରେ int32_t z_timeout_q_next_gap(ଶୂନ୍ୟ)
{
	ସଂରଚନା _timeout *t = z_timeout_q_first();

	ଫେରନ୍ତୁ (t == ଖାଲି) ? INT32_MAX : (int32_t)MIN((int64_t)t->dticks, INT32_MAX);
}

/* Advance backend state by @dt ticks. For the delta list this shifts the head
 * closer to expiry; a head whose gap is fully consumed reads dticks == 0 and
 * is then drained by z_timeout_q_pop_due(). The caller advances curr_tick.
 */
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ z_timeout_q_advance(int32_t dt)
{
	ସଂରଚନା _timeout *t = z_timeout_q_first();

	ଯଦି (t != ଖାଲି) {
		t->dticks -= dt;
	}
}

/* Pop and return one timeout that is due at the current curr_tick, or NULL if
 * none is due right now. Strict head-only removal with no delta absorption:
 * z_timeout_q_advance() has already brought the head's dticks to 0, and
 * curr_tick was advanced to match, so the successor's stored delta stays
 * valid without propagation.
 */
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ସଂରଚନା _timeout *z_timeout_q_କାଢ଼_due(ଶୂନ୍ୟ)
{
	ସଂରଚନା _timeout *t = z_timeout_q_first();

	ଯଦି ((t == ଖାଲି) || (t->dticks != 0)) {
		ଫେରନ୍ତୁ ଖାଲି;
	}

	sys_dlist_remove(&t->ଗଣ୍ଠି);

	ଫେରନ୍ତୁ t;
}

#endif /* ZEPHYR_KERNEL_TIMEOUT_LIST_H_ */
