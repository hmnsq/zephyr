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

#अन्तर्भाव <zephyr/sys/dlist.h>

स्थिर sys_dlist_t timeout_list = SYS_DLIST_STATIC_INIT(&timeout_list);

स्थिर अन्तःस्थ संरचना _timeout *z_timeout_q_first(शून्य)
{
	sys_dnode_t *t = sys_dlist_peek_head(&timeout_list);

	फर्काऊ (t == रिक्त) ? रिक्त : CONTAINER_OF(t, संरचना _timeout, गाँठो);
}

स्थिर अन्तःस्थ संरचना _timeout *z_timeout_q_next(संरचना _timeout *t)
{
	sys_dnode_t *n = sys_dlist_peek_next(&timeout_list, &t->गाँठो);

	फर्काऊ (n == रिक्त) ? रिक्त : CONTAINER_OF(n, संरचना _timeout, गाँठो);
}

/* Insert @to so that it expires @dticks ticks after curr_tick. Returns true
 * if @to is now the earliest pending timeout.
 */
स्थिर अन्तःस्थ bool z_timeout_q_घुसाउ(संरचना _timeout *to, k_ticks_t dticks)
{
	संरचना _timeout *t;

	to->dticks = dticks;
	लागि (t = z_timeout_q_first(); t != रिक्त; t = z_timeout_q_next(t)) {
		यदि (t->dticks > to->dticks) {
			t->dticks -= to->dticks;
			sys_dlist_घुसाउ(&t->गाँठो, &to->गाँठो);
			फर्काऊ z_timeout_q_first() == to;
		}
		to->dticks -= t->dticks;
	}

	sys_dlist_append(&timeout_list, &to->गाँठो);
	फर्काऊ z_timeout_q_first() == to;
}

/* Remove an arbitrary timeout from the queue, absorbing its delta into the
 * successor so the remaining deltas stay valid. Returns true if @to was the
 * earliest pending timeout. Used only by the abort path, never from announce.
 */
स्थिर अन्तःस्थ bool z_timeout_q_remove(संरचना _timeout *to)
{
	bool was_first = (z_timeout_q_first() == to);
	संरचना _timeout *n = z_timeout_q_next(to);

	यदि (n != रिक्त) {
		n->dticks += to->dticks;
	}
	sys_dlist_remove(&to->गाँठो);

	फर्काऊ was_first;
}

/* Ticks from curr_tick until @to expires. Caller holds timeout_lock and has
 * verified that @to is active.
 */
स्थिर अन्तःस्थ k_ticks_t z_timeout_q_remainder(अचर संरचना _timeout *to)
{
	k_ticks_t ticks = 0;

	लागि (संरचना _timeout *t = z_timeout_q_first(); t != रिक्त;
	     t = z_timeout_q_next(t)) {
		ticks += t->dticks;
		यदि (to == t) {
			रोक;
		}
	}

	फर्काऊ ticks;
}

/* Ticks from (curr_tick + ticks_elapsed) until the next expiry, clamped to
 * int32 / SYS_CLOCK_MAX_WAIT. Empty queue yields SYS_CLOCK_MAX_WAIT.
 */
/* Ticks from curr_tick to the earliest pending timeout, or K_TICKS_FOREVER if
 * the queue is empty. The announce-range cap is applied centrally by
 * next_timeout() in timeout.c.
 */
स्थिर अन्तःस्थ k_ticks_t z_timeout_q_next_expiry(शून्य)
{
	संरचना _timeout *to = z_timeout_q_first();

	फर्काऊ (to == रिक्त) ? K_TICKS_FOREVER : to->dticks;
}

/* --- announce-loop helpers (see sys_clock_announce_locked() in timeout.c) --- */

/* Ticks from curr_tick until the next event the announce loop must act on
 * (here: the earliest expiry). A value larger than any announce window when
 * the queue is empty.
 */
स्थिर अन्तःस्थ int32_t z_timeout_q_next_gap(शून्य)
{
	संरचना _timeout *t = z_timeout_q_first();

	फर्काऊ (t == रिक्त) ? INT32_MAX : (int32_t)MIN((int64_t)t->dticks, INT32_MAX);
}

/* Advance backend state by @dt ticks. For the delta list this shifts the head
 * closer to expiry; a head whose gap is fully consumed reads dticks == 0 and
 * is then drained by z_timeout_q_pop_due(). The caller advances curr_tick.
 */
स्थिर अन्तःस्थ शून्य z_timeout_q_advance(int32_t dt)
{
	संरचना _timeout *t = z_timeout_q_first();

	यदि (t != रिक्त) {
		t->dticks -= dt;
	}
}

/* Pop and return one timeout that is due at the current curr_tick, or NULL if
 * none is due right now. Strict head-only removal with no delta absorption:
 * z_timeout_q_advance() has already brought the head's dticks to 0, and
 * curr_tick was advanced to match, so the successor's stored delta stays
 * valid without propagation.
 */
स्थिर अन्तःस्थ संरचना _timeout *z_timeout_q_झिक_due(शून्य)
{
	संरचना _timeout *t = z_timeout_q_first();

	यदि ((t == रिक्त) || (t->dticks != 0)) {
		फर्काऊ रिक्त;
	}

	sys_dlist_remove(&t->गाँठो);

	फर्काऊ t;
}

#endif /* ZEPHYR_KERNEL_TIMEOUT_LIST_H_ */
