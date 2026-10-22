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

#شامل <zephyr/sys/dlist.h>

ایستا sys_dlist_t timeout_list = SYS_DLIST_STATIC_INIT(&timeout_list);

ایستا درونخطی ساختار _timeout *z_timeout_q_first(تهی)
{
	sys_dnode_t *t = sys_dlist_peek_head(&timeout_list);

	بازگشت (t == پوچ) ? پوچ : CONTAINER_OF(t, ساختار _timeout, گره);
}

ایستا درونخطی ساختار _timeout *z_timeout_q_next(ساختار _timeout *t)
{
	sys_dnode_t *n = sys_dlist_peek_next(&timeout_list, &t->گره);

	بازگشت (n == پوچ) ? پوچ : CONTAINER_OF(n, ساختار _timeout, گره);
}

/* Insert @to so that it expires @dticks ticks after curr_tick. Returns true
 * if @to is now the earliest pending timeout.
 */
ایستا درونخطی bool z_timeout_q_درج(ساختار _timeout *to, k_ticks_t dticks)
{
	ساختار _timeout *t;

	to->dticks = dticks;
	برای (t = z_timeout_q_first(); t != پوچ; t = z_timeout_q_next(t)) {
		اگر (t->dticks > to->dticks) {
			t->dticks -= to->dticks;
			sys_dlist_درج(&t->گره, &to->گره);
			بازگشت z_timeout_q_first() == to;
		}
		to->dticks -= t->dticks;
	}

	sys_dlist_append(&timeout_list, &to->گره);
	بازگشت z_timeout_q_first() == to;
}

/* Remove an arbitrary timeout from the queue, absorbing its delta into the
 * successor so the remaining deltas stay valid. Returns true if @to was the
 * earliest pending timeout. Used only by the abort path, never from announce.
 */
ایستا درونخطی bool z_timeout_q_remove(ساختار _timeout *to)
{
	bool was_first = (z_timeout_q_first() == to);
	ساختار _timeout *n = z_timeout_q_next(to);

	اگر (n != پوچ) {
		n->dticks += to->dticks;
	}
	sys_dlist_remove(&to->گره);

	بازگشت was_first;
}

/* Ticks from curr_tick until @to expires. Caller holds timeout_lock and has
 * verified that @to is active.
 */
ایستا درونخطی k_ticks_t z_timeout_q_remainder(ثابت ساختار _timeout *to)
{
	k_ticks_t ticks = 0;

	برای (ساختار _timeout *t = z_timeout_q_first(); t != پوچ;
	     t = z_timeout_q_next(t)) {
		ticks += t->dticks;
		اگر (to == t) {
			بشکن;
		}
	}

	بازگشت ticks;
}

/* Ticks from (curr_tick + ticks_elapsed) until the next expiry, clamped to
 * int32 / SYS_CLOCK_MAX_WAIT. Empty queue yields SYS_CLOCK_MAX_WAIT.
 */
/* Ticks from curr_tick to the earliest pending timeout, or K_TICKS_FOREVER if
 * the queue is empty. The announce-range cap is applied centrally by
 * next_timeout() in timeout.c.
 */
ایستا درونخطی k_ticks_t z_timeout_q_next_expiry(تهی)
{
	ساختار _timeout *to = z_timeout_q_first();

	بازگشت (to == پوچ) ? K_TICKS_FOREVER : to->dticks;
}

/* --- announce-loop helpers (see sys_clock_announce_locked() in timeout.c) --- */

/* Ticks from curr_tick until the next event the announce loop must act on
 * (here: the earliest expiry). A value larger than any announce window when
 * the queue is empty.
 */
ایستا درونخطی int32_t z_timeout_q_next_gap(تهی)
{
	ساختار _timeout *t = z_timeout_q_first();

	بازگشت (t == پوچ) ? INT32_MAX : (int32_t)MIN((int64_t)t->dticks, INT32_MAX);
}

/* Advance backend state by @dt ticks. For the delta list this shifts the head
 * closer to expiry; a head whose gap is fully consumed reads dticks == 0 and
 * is then drained by z_timeout_q_pop_due(). The caller advances curr_tick.
 */
ایستا درونخطی تهی z_timeout_q_advance(int32_t dt)
{
	ساختار _timeout *t = z_timeout_q_first();

	اگر (t != پوچ) {
		t->dticks -= dt;
	}
}

/* Pop and return one timeout that is due at the current curr_tick, or NULL if
 * none is due right now. Strict head-only removal with no delta absorption:
 * z_timeout_q_advance() has already brought the head's dticks to 0, and
 * curr_tick was advanced to match, so the successor's stored delta stays
 * valid without propagation.
 */
ایستا درونخطی ساختار _timeout *z_timeout_q_برداشت_due(تهی)
{
	ساختار _timeout *t = z_timeout_q_first();

	اگر ((t == پوچ) || (t->dticks != 0)) {
		بازگشت پوچ;
	}

	sys_dlist_remove(&t->گره);

	بازگشت t;
}

#endif /* ZEPHYR_KERNEL_TIMEOUT_LIST_H_ */
