/*
 * Copyright (c) 2018 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#اگر_غیر_تعریف_شدہ ZEPHYR_KERNEL_TIMEOUT_LIST_H_
#تعریف_کرو ZEPHYR_KERNEL_TIMEOUT_LIST_H_

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

ساکن sys_dlist_t timeout_list = SYS_DLIST_STATIC_INIT(&timeout_list);

ساکن اندرونی ساخت _timeout *z_timeout_q_پہلا(خالی)
{
	sys_dnode_t *t = sys_dlist_peek_سر(&timeout_list);

	واپس (t == تہی) ? تہی : CONTAINER_کا(t, ساخت _timeout, گرہ);
}

ساکن اندرونی ساخت _timeout *z_timeout_q_اگلا(ساخت _timeout *t)
{
	sys_dnode_t *n = sys_dlist_peek_اگلا(&timeout_list, &t->گرہ);

	واپس (n == تہی) ? تہی : CONTAINER_کا(n, ساخت _timeout, گرہ);
}

/* Insert @to so that it expires @dticks ticks after curr_tick. Returns true
 * if @to is now the earliest pending timeout.
 */
ساکن اندرونی bool z_timeout_q_داخل_کرو(ساخت _timeout *مقصد, k_ticks_t dticks)
{
	ساخت _timeout *t;

	مقصد->dticks = dticks;
	برائے (t = z_timeout_q_پہلا(); t != تہی; t = z_timeout_q_اگلا(t)) {
		اگر (t->dticks > مقصد->dticks) {
			t->dticks -= مقصد->dticks;
			sys_dlist_داخل_کرو(&t->گرہ, &مقصد->گرہ);
			واپس z_timeout_q_پہلا() == مقصد;
		}
		مقصد->dticks -= t->dticks;
	}

	sys_dlist_append(&timeout_list, &مقصد->گرہ);
	واپس z_timeout_q_پہلا() == مقصد;
}

/* Remove an arbitrary timeout from the queue, absorbing its delta into the
 * successor so the remaining deltas stay valid. Returns true if @to was the
 * earliest pending timeout. Used only by the abort path, never from announce.
 */
ساکن اندرونی bool z_timeout_q_remove(ساخت _timeout *مقصد)
{
	bool was_پہلا = (z_timeout_q_پہلا() == مقصد);
	ساخت _timeout *n = z_timeout_q_اگلا(مقصد);

	اگر (n != تہی) {
		n->dticks += مقصد->dticks;
	}
	sys_dlist_remove(&مقصد->گرہ);

	واپس was_پہلا;
}

/* Ticks from curr_tick until @to expires. Caller holds timeout_lock and has
 * verified that @to is active.
 */
ساکن اندرونی k_ticks_t z_timeout_q_remainder(ثابت ساخت _timeout *مقصد)
{
	k_ticks_t ticks = 0;

	برائے (ساخت _timeout *t = z_timeout_q_پہلا(); t != تہی;
	     t = z_timeout_q_اگلا(t)) {
		ticks += t->dticks;
		اگر (مقصد == t) {
			توڑو;
		}
	}

	واپس ticks;
}

/* Ticks from (curr_tick + ticks_elapsed) until the next expiry, clamped to
 * int32 / SYS_CLOCK_MAX_WAIT. Empty queue yields SYS_CLOCK_MAX_WAIT.
 */
/* Ticks from curr_tick to the earliest pending timeout, or K_TICKS_FOREVER if
 * the queue is empty. The announce-range cap is applied centrally by
 * next_timeout() in timeout.c.
 */
ساکن اندرونی k_ticks_t z_timeout_q_اگلا_expiry(خالی)
{
	ساخت _timeout *مقصد = z_timeout_q_پہلا();

	واپس (مقصد == تہی) ? K_TICKS_FOREVER : مقصد->dticks;
}

/* --- announce-loop helpers (see sys_clock_announce_locked() in timeout.c) --- */

/* Ticks from curr_tick until the next event the announce loop must act on
 * (here: the earliest expiry). A value larger than any announce window when
 * the queue is empty.
 */
ساکن اندرونی int32_t z_timeout_q_اگلا_gap(خالی)
{
	ساخت _timeout *t = z_timeout_q_پہلا();

	واپس (t == تہی) ? INT32_MAX : (int32_t)MIN((int64_t)t->dticks, INT32_MAX);
}

/* Advance backend state by @dt ticks. For the delta list this shifts the head
 * closer to expiry; a head whose gap is fully consumed reads dticks == 0 and
 * is then drained by z_timeout_q_pop_due(). The caller advances curr_tick.
 */
ساکن اندرونی خالی z_timeout_q_advance(int32_t dt)
{
	ساخت _timeout *t = z_timeout_q_پہلا();

	اگر (t != تہی) {
		t->dticks -= dt;
	}
}

/* Pop and return one timeout that is due at the current curr_tick, or NULL if
 * none is due right now. Strict head-only removal with no delta absorption:
 * z_timeout_q_advance() has already brought the head's dticks to 0, and
 * curr_tick was advanced to match, so the successor's stored delta stays
 * valid without propagation.
 */
ساکن اندرونی ساخت _timeout *z_timeout_q_نکالو_due(خالی)
{
	ساخت _timeout *t = z_timeout_q_پہلا();

	اگر ((t == تہی) || (t->dticks != 0)) {
		واپس تہی;
	}

	sys_dlist_remove(&t->گرہ);

	واپس t;
}

#اگر_ختم /* ZEPHYR_KERNEL_TIMEOUT_LIST_H_ */
