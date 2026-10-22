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

#அந்தர்பாவ <zephyr/sys/dlist.h>

நிலையான sys_dlist_t timeout_list = SYS_DLIST_STATIC_INIT(&timeout_list);

நிலையான உள்வரி கட்டமைப்பு _timeout *z_timeout_q_first(வெறுமை)
{
	sys_dnode_t *t = sys_dlist_peek_head(&timeout_list);

	திரும்பு (t == வெற்று) ? வெற்று : CONTAINER_OF(t, கட்டமைப்பு _timeout, முனை);
}

நிலையான உள்வரி கட்டமைப்பு _timeout *z_timeout_q_next(கட்டமைப்பு _timeout *t)
{
	sys_dnode_t *n = sys_dlist_peek_next(&timeout_list, &t->முனை);

	திரும்பு (n == வெற்று) ? வெற்று : CONTAINER_OF(n, கட்டமைப்பு _timeout, முனை);
}

/* Insert @to so that it expires @dticks ticks after curr_tick. Returns true
 * if @to is now the earliest pending timeout.
 */
நிலையான உள்வரி bool z_timeout_q_செருகு(கட்டமைப்பு _timeout *to, k_ticks_t dticks)
{
	கட்டமைப்பு _timeout *t;

	to->dticks = dticks;
	க்கு (t = z_timeout_q_first(); t != வெற்று; t = z_timeout_q_next(t)) {
		எனில் (t->dticks > to->dticks) {
			t->dticks -= to->dticks;
			sys_dlist_செருகு(&t->முனை, &to->முனை);
			திரும்பு z_timeout_q_first() == to;
		}
		to->dticks -= t->dticks;
	}

	sys_dlist_append(&timeout_list, &to->முனை);
	திரும்பு z_timeout_q_first() == to;
}

/* Remove an arbitrary timeout from the queue, absorbing its delta into the
 * successor so the remaining deltas stay valid. Returns true if @to was the
 * earliest pending timeout. Used only by the abort path, never from announce.
 */
நிலையான உள்வரி bool z_timeout_q_remove(கட்டமைப்பு _timeout *to)
{
	bool was_first = (z_timeout_q_first() == to);
	கட்டமைப்பு _timeout *n = z_timeout_q_next(to);

	எனில் (n != வெற்று) {
		n->dticks += to->dticks;
	}
	sys_dlist_remove(&to->முனை);

	திரும்பு was_first;
}

/* Ticks from curr_tick until @to expires. Caller holds timeout_lock and has
 * verified that @to is active.
 */
நிலையான உள்வரி k_ticks_t z_timeout_q_remainder(மாறிலி கட்டமைப்பு _timeout *to)
{
	k_ticks_t ticks = 0;

	க்கு (கட்டமைப்பு _timeout *t = z_timeout_q_first(); t != வெற்று;
	     t = z_timeout_q_next(t)) {
		ticks += t->dticks;
		எனில் (to == t) {
			நிறுத்து;
		}
	}

	திரும்பு ticks;
}

/* Ticks from (curr_tick + ticks_elapsed) until the next expiry, clamped to
 * int32 / SYS_CLOCK_MAX_WAIT. Empty queue yields SYS_CLOCK_MAX_WAIT.
 */
/* Ticks from curr_tick to the earliest pending timeout, or K_TICKS_FOREVER if
 * the queue is empty. The announce-range cap is applied centrally by
 * next_timeout() in timeout.c.
 */
நிலையான உள்வரி k_ticks_t z_timeout_q_next_expiry(வெறுமை)
{
	கட்டமைப்பு _timeout *to = z_timeout_q_first();

	திரும்பு (to == வெற்று) ? K_TICKS_FOREVER : to->dticks;
}

/* --- announce-loop helpers (see sys_clock_announce_locked() in timeout.c) --- */

/* Ticks from curr_tick until the next event the announce loop must act on
 * (here: the earliest expiry). A value larger than any announce window when
 * the queue is empty.
 */
நிலையான உள்வரி int32_t z_timeout_q_next_gap(வெறுமை)
{
	கட்டமைப்பு _timeout *t = z_timeout_q_first();

	திரும்பு (t == வெற்று) ? INT32_MAX : (int32_t)MIN((int64_t)t->dticks, INT32_MAX);
}

/* Advance backend state by @dt ticks. For the delta list this shifts the head
 * closer to expiry; a head whose gap is fully consumed reads dticks == 0 and
 * is then drained by z_timeout_q_pop_due(). The caller advances curr_tick.
 */
நிலையான உள்வரி வெறுமை z_timeout_q_advance(int32_t dt)
{
	கட்டமைப்பு _timeout *t = z_timeout_q_first();

	எனில் (t != வெற்று) {
		t->dticks -= dt;
	}
}

/* Pop and return one timeout that is due at the current curr_tick, or NULL if
 * none is due right now. Strict head-only removal with no delta absorption:
 * z_timeout_q_advance() has already brought the head's dticks to 0, and
 * curr_tick was advanced to match, so the successor's stored delta stays
 * valid without propagation.
 */
நிலையான உள்வரி கட்டமைப்பு _timeout *z_timeout_q_எடு_due(வெறுமை)
{
	கட்டமைப்பு _timeout *t = z_timeout_q_first();

	எனில் ((t == வெற்று) || (t->dticks != 0)) {
		திரும்பு வெற்று;
	}

	sys_dlist_remove(&t->முனை);

	திரும்பு t;
}

#endif /* ZEPHYR_KERNEL_TIMEOUT_LIST_H_ */
