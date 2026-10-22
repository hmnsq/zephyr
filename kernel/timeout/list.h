/*
 * Copyright (c) 2018 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#اگر_تعریف_نشده ZEPHYR_KERNEL_TIMEOUT_LIST_H_
#تعریف_کن ZEPHYR_KERNEL_TIMEOUT_LIST_H_

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

ساكن مضمن بنية _timeout *z_timeout_q_اولی(فراغ)
{
	sys_dnode_t *t = sys_dlist_peek_سر(&timeout_list);

	أرجع (t == عدم) ? عدم : CONTAINER_از(t, بنية _timeout, گره);
}

ساكن مضمن بنية _timeout *z_timeout_q_بعدی(بنية _timeout *t)
{
	sys_dnode_t *n = sys_dlist_peek_بعدی(&timeout_list, &t->گره);

	أرجع (n == عدم) ? عدم : CONTAINER_از(n, بنية _timeout, گره);
}

/* Insert @to so that it expires @dticks ticks after curr_tick. Returns true
 * if @to is now the earliest pending timeout.
 */
ساكن مضمن bool z_timeout_q_درج(بنية _timeout *مقصد, k_ticks_t dticks)
{
	بنية _timeout *t;

	مقصد->dticks = dticks;
	لكل (t = z_timeout_q_اولی(); t != عدم; t = z_timeout_q_بعدی(t)) {
		إذا (t->dticks > مقصد->dticks) {
			t->dticks -= مقصد->dticks;
			sys_dlist_درج(&t->گره, &مقصد->گره);
			أرجع z_timeout_q_اولی() == مقصد;
		}
		مقصد->dticks -= t->dticks;
	}

	sys_dlist_append(&timeout_list, &مقصد->گره);
	أرجع z_timeout_q_اولی() == مقصد;
}

/* Remove an arbitrary timeout from the queue, absorbing its delta into the
 * successor so the remaining deltas stay valid. Returns true if @to was the
 * earliest pending timeout. Used only by the abort path, never from announce.
 */
ساكن مضمن bool z_timeout_q_remove(بنية _timeout *مقصد)
{
	bool was_اولی = (z_timeout_q_اولی() == مقصد);
	بنية _timeout *n = z_timeout_q_بعدی(مقصد);

	إذا (n != عدم) {
		n->dticks += مقصد->dticks;
	}
	sys_dlist_remove(&مقصد->گره);

	أرجع was_اولی;
}

/* Ticks from curr_tick until @to expires. Caller holds timeout_lock and has
 * verified that @to is active.
 */
ساكن مضمن k_ticks_t z_timeout_q_remainder(ثابت بنية _timeout *مقصد)
{
	k_ticks_t ticks = 0;

	لكل (بنية _timeout *t = z_timeout_q_اولی(); t != عدم;
	     t = z_timeout_q_بعدی(t)) {
		ticks += t->dticks;
		إذا (مقصد == t) {
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
ساكن مضمن k_ticks_t z_timeout_q_بعدی_expiry(فراغ)
{
	بنية _timeout *مقصد = z_timeout_q_اولی();

	أرجع (مقصد == عدم) ? K_TICKS_FOREVER : مقصد->dticks;
}

/* --- announce-loop helpers (see sys_clock_announce_locked() in timeout.c) --- */

/* Ticks from curr_tick until the next event the announce loop must act on
 * (here: the earliest expiry). A value larger than any announce window when
 * the queue is empty.
 */
ساكن مضمن int32_t z_timeout_q_بعدی_gap(فراغ)
{
	بنية _timeout *t = z_timeout_q_اولی();

	أرجع (t == عدم) ? INT32_MAX : (int32_t)MIN((int64_t)t->dticks, INT32_MAX);
}

/* Advance backend state by @dt ticks. For the delta list this shifts the head
 * closer to expiry; a head whose gap is fully consumed reads dticks == 0 and
 * is then drained by z_timeout_q_pop_due(). The caller advances curr_tick.
 */
ساكن مضمن فراغ z_timeout_q_advance(int32_t dt)
{
	بنية _timeout *t = z_timeout_q_اولی();

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
ساكن مضمن بنية _timeout *z_timeout_q_برداشت_due(فراغ)
{
	بنية _timeout *t = z_timeout_q_اولی();

	إذا ((t == عدم) || (t->dticks != 0)) {
		أرجع عدم;
	}

	sys_dlist_remove(&t->گره);

	أرجع t;
}

#پایان_اگر /* ZEPHYR_KERNEL_TIMEOUT_LIST_H_ */
