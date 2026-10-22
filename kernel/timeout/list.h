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

#ضمن <zephyr/sys/dlist.h>

ساكن sys_dlist_t timeout_list = SYS_DLIST_STATIC_INIT(&timeout_list);

ساكن مضمن بنية _timeout *z_timeout_q_first(فراغ)
{
	sys_dnode_t *t = sys_dlist_peek_head(&timeout_list);

	أرجع (t == عدم) ? عدم : CONTAINER_OF(t, بنية _timeout, نود);
}

ساكن مضمن بنية _timeout *z_timeout_q_next(بنية _timeout *t)
{
	sys_dnode_t *n = sys_dlist_peek_next(&timeout_list, &t->نود);

	أرجع (n == عدم) ? عدم : CONTAINER_OF(n, بنية _timeout, نود);
}

/* Insert @to so that it expires @dticks ticks after curr_tick. Returns true
 * if @to is now the earliest pending timeout.
 */
ساكن مضمن bool z_timeout_q_سيسيڤ(بنية _timeout *to, k_ticks_t dticks)
{
	بنية _timeout *t;

	to->dticks = dticks;
	لكل (t = z_timeout_q_first(); t != عدم; t = z_timeout_q_next(t)) {
		إذا (t->dticks > to->dticks) {
			t->dticks -= to->dticks;
			sys_dlist_سيسيڤ(&t->نود, &to->نود);
			أرجع z_timeout_q_first() == to;
		}
		to->dticks -= t->dticks;
	}

	sys_dlist_append(&timeout_list, &to->نود);
	أرجع z_timeout_q_first() == to;
}

/* Remove an arbitrary timeout from the queue, absorbing its delta into the
 * successor so the remaining deltas stay valid. Returns true if @to was the
 * earliest pending timeout. Used only by the abort path, never from announce.
 */
ساكن مضمن bool z_timeout_q_remove(بنية _timeout *to)
{
	bool was_first = (z_timeout_q_first() == to);
	بنية _timeout *n = z_timeout_q_next(to);

	إذا (n != عدم) {
		n->dticks += to->dticks;
	}
	sys_dlist_remove(&to->نود);

	أرجع was_first;
}

/* Ticks from curr_tick until @to expires. Caller holds timeout_lock and has
 * verified that @to is active.
 */
ساكن مضمن k_ticks_t z_timeout_q_remainder(ثابت بنية _timeout *to)
{
	k_ticks_t ticks = 0;

	لكل (بنية _timeout *t = z_timeout_q_first(); t != عدم;
	     t = z_timeout_q_next(t)) {
		ticks += t->dticks;
		إذا (to == t) {
			اكسر;
		}
	}

	أرجع ticks;
}

/* Ticks from (curr_tick + ticks_elapsed) until the next expiry, clamped to
 * int32 / SYS_CLOCK_MAX_WAIT. Empty queue yields SYS_CLOCK_MAX_WAIT.
 */
/* Ticks from curr_tick to the earliest pending timeout, or K_TICKS_FOREVER if
 * the queue is empty. The announce-range cap is applied centrally by
 * next_timeout() in timeout.c.
 */
ساكن مضمن k_ticks_t z_timeout_q_next_expiry(فراغ)
{
	بنية _timeout *to = z_timeout_q_first();

	أرجع (to == عدم) ? K_TICKS_FOREVER : to->dticks;
}

/* --- announce-loop helpers (see sys_clock_announce_locked() in timeout.c) --- */

/* Ticks from curr_tick until the next event the announce loop must act on
 * (here: the earliest expiry). A value larger than any announce window when
 * the queue is empty.
 */
ساكن مضمن int32_t z_timeout_q_next_gap(فراغ)
{
	بنية _timeout *t = z_timeout_q_first();

	أرجع (t == عدم) ? INT32_MAX : (int32_t)MIN((int64_t)t->dticks, INT32_MAX);
}

/* Advance backend state by @dt ticks. For the delta list this shifts the head
 * closer to expiry; a head whose gap is fully consumed reads dticks == 0 and
 * is then drained by z_timeout_q_pop_due(). The caller advances curr_tick.
 */
ساكن مضمن فراغ z_timeout_q_advance(int32_t dt)
{
	بنية _timeout *t = z_timeout_q_first();

	إذا (t != عدم) {
		t->dticks -= dt;
	}
}

/* Pop and return one timeout that is due at the current curr_tick, or NULL if
 * none is due right now. Strict head-only removal with no delta absorption:
 * z_timeout_q_advance() has already brought the head's dticks to 0, and
 * curr_tick was advanced to match, so the successor's stored delta stays
 * valid without propagation.
 */
ساكن مضمن بنية _timeout *z_timeout_q_کلوار_due(فراغ)
{
	بنية _timeout *t = z_timeout_q_first();

	إذا ((t == عدم) || (t->dticks != 0)) {
		أرجع عدم;
	}

	sys_dlist_remove(&t->نود);

	أرجع t;
}

#endif /* ZEPHYR_KERNEL_TIMEOUT_LIST_H_ */
