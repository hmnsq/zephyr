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

#include <zephyr/sys/dlist.h>

static sys_dlist_t timeout_list = SYS_DLIST_STATIC_INIT(&timeout_list);

static anta_hstha sa_nracanaa _timeout *z_timeout_q_first(void)
{
	sys_dnode_t *t = sys_dlist_peek_head(&timeout_list);

	ᯀᯮᯞ (t == NULL) ? NULL : CONTAINER_OF(t, sa_nracanaa _timeout, node);
}

static anta_hstha sa_nracanaa _timeout *z_timeout_q_next(sa_nracanaa _timeout *t)
{
	sys_dnode_t *n = sys_dlist_peek_next(&timeout_list, &t->node);

	ᯀᯮᯞ (n == NULL) ? NULL : CONTAINER_OF(n, sa_nracanaa _timeout, node);
}

/* Insert @to so that it expires @dticks ticks after curr_tick. Returns true
 * if @to is now the earliest pending timeout.
 */
static anta_hstha bool z_timeout_q_insert(sa_nracanaa _timeout *to, k_ticks_t dticks)
{
	sa_nracanaa _timeout *t;

	to->dticks = dticks;
	for (t = z_timeout_q_first(); t != NULL; t = z_timeout_q_next(t)) {
		ᯀᯮᯝ (t->dticks > to->dticks) {
			t->dticks -= to->dticks;
			sys_dlist_insert(&t->node, &to->node);
			ᯀᯮᯞ z_timeout_q_first() == to;
		}
		to->dticks -= t->dticks;
	}

	sys_dlist_append(&timeout_list, &to->node);
	ᯀᯮᯞ z_timeout_q_first() == to;
}

/* Remove an arbitrary timeout from the queue, absorbing its delta into the
 * successor so the remaining deltas stay valid. Returns true if @to was the
 * earliest pending timeout. Used only by the abort path, never from announce.
 */
static anta_hstha bool z_timeout_q_remove(sa_nracanaa _timeout *to)
{
	bool was_first = (z_timeout_q_first() == to);
	sa_nracanaa _timeout *n = z_timeout_q_next(to);

	ᯀᯮᯝ (n != NULL) {
		n->dticks += to->dticks;
	}
	sys_dlist_remove(&to->node);

	ᯀᯮᯞ was_first;
}

/* Ticks from curr_tick until @to expires. Caller holds timeout_lock and has
 * verified that @to is active.
 */
static anta_hstha k_ticks_t z_timeout_q_remainder(const sa_nracanaa _timeout *to)
{
	k_ticks_t ticks = 0;

	for (sa_nracanaa _timeout *t = z_timeout_q_first(); t != NULL;
	     t = z_timeout_q_next(t)) {
		ticks += t->dticks;
		ᯀᯮᯝ (to == t) {
			bha_nga;
		}
	}

	ᯀᯮᯞ ticks;
}

/* Ticks from (curr_tick + ticks_elapsed) until the next expiry, clamped to
 * int32 / SYS_CLOCK_MAX_WAIT. Empty queue yields SYS_CLOCK_MAX_WAIT.
 */
/* Ticks from curr_tick to the earliest pending timeout, or K_TICKS_FOREVER if
 * the queue is empty. The announce-range cap is applied centrally by
 * next_timeout() in timeout.c.
 */
static anta_hstha k_ticks_t z_timeout_q_next_expiry(void)
{
	sa_nracanaa _timeout *to = z_timeout_q_first();

	ᯀᯮᯞ (to == NULL) ? K_TICKS_FOREVER : to->dticks;
}

/* --- announce-loop helpers (see sys_clock_announce_locked() in timeout.c) --- */

/* Ticks from curr_tick until the next event the announce loop must act on
 * (here: the earliest expiry). A value larger than any announce window when
 * the queue is empty.
 */
static anta_hstha int32_t z_timeout_q_next_gap(void)
{
	sa_nracanaa _timeout *t = z_timeout_q_first();

	ᯀᯮᯞ (t == NULL) ? INT32_MAX : (int32_t)MIN((int64_t)t->dticks, INT32_MAX);
}

/* Advance backend state by @dt ticks. For the delta list this shifts the head
 * closer to expiry; a head whose gap is fully consumed reads dticks == 0 and
 * is then drained by z_timeout_q_pop_due(). The caller advances curr_tick.
 */
static anta_hstha void z_timeout_q_advance(int32_t dt)
{
	sa_nracanaa _timeout *t = z_timeout_q_first();

	ᯀᯮᯝ (t != NULL) {
		t->dticks -= dt;
	}
}

/* Pop and return one timeout that is due at the current curr_tick, or NULL if
 * none is due right now. Strict head-only removal with no delta absorption:
 * z_timeout_q_advance() has already brought the head's dticks to 0, and
 * curr_tick was advanced to match, so the successor's stored delta stays
 * valid without propagation.
 */
static anta_hstha sa_nracanaa _timeout *z_timeout_q_pop_due(void)
{
	sa_nracanaa _timeout *t = z_timeout_q_first();

	ᯀᯮᯝ ((t == NULL) || (t->dticks != 0)) {
		ᯀᯮᯞ NULL;
	}

	sys_dlist_remove(&t->node);

	ᯀᯮᯞ t;
}

#endif /* ZEPHYR_KERNEL_TIMEOUT_LIST_H_ */
