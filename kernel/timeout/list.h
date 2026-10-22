/*
 * Copyright (c) 2018 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#నిర్వచితం_కాకపోతే ZEPHYR_KERNEL_TIMEOUT_LIST_H_
#నిర్వచించు ZEPHYR_KERNEL_TIMEOUT_LIST_H_

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

#అన్తర్భావ <zephyr/sys/dlist.h>

స్థిర sys_dlist_t timeout_list = SYS_DLIST_STATIC_INIT(&timeout_list);

స్థిర అంతఃవరుస నిర్మాణం _timeout *z_timeout_q_మొదటి(శూన్య)
{
	sys_dnode_t *t = sys_dlist_peek_తల(&timeout_list);

	తిరిగి (t == ఖాళీ) ? ఖాళీ : CONTAINER_యొక్క(t, నిర్మాణం _timeout, కణుపు);
}

స్థిర అంతఃవరుస నిర్మాణం _timeout *z_timeout_q_తదుపరి(నిర్మాణం _timeout *t)
{
	sys_dnode_t *n = sys_dlist_peek_తదుపరి(&timeout_list, &t->కణుపు);

	తిరిగి (n == ఖాళీ) ? ఖాళీ : CONTAINER_యొక్క(n, నిర్మాణం _timeout, కణుపు);
}

/* Insert @to so that it expires @dticks ticks after curr_tick. Returns true
 * if @to is now the earliest pending timeout.
 */
స్థిర అంతఃవరుస bool z_timeout_q_చొప్పించు(నిర్మాణం _timeout *గమ్యం, k_ticks_t dticks)
{
	నిర్మాణం _timeout *t;

	గమ్యం->dticks = dticks;
	కొరకు (t = z_timeout_q_మొదటి(); t != ఖాళీ; t = z_timeout_q_తదుపరి(t)) {
		ఒకవేళ (t->dticks > గమ్యం->dticks) {
			t->dticks -= గమ్యం->dticks;
			sys_dlist_చొప్పించు(&t->కణుపు, &గమ్యం->కణుపు);
			తిరిగి z_timeout_q_మొదటి() == గమ్యం;
		}
		గమ్యం->dticks -= t->dticks;
	}

	sys_dlist_append(&timeout_list, &గమ్యం->కణుపు);
	తిరిగి z_timeout_q_మొదటి() == గమ్యం;
}

/* Remove an arbitrary timeout from the queue, absorbing its delta into the
 * successor so the remaining deltas stay valid. Returns true if @to was the
 * earliest pending timeout. Used only by the abort path, never from announce.
 */
స్థిర అంతఃవరుస bool z_timeout_q_remove(నిర్మాణం _timeout *గమ్యం)
{
	bool was_మొదటి = (z_timeout_q_మొదటి() == గమ్యం);
	నిర్మాణం _timeout *n = z_timeout_q_తదుపరి(గమ్యం);

	ఒకవేళ (n != ఖాళీ) {
		n->dticks += గమ్యం->dticks;
	}
	sys_dlist_remove(&గమ్యం->కణుపు);

	తిరిగి was_మొదటి;
}

/* Ticks from curr_tick until @to expires. Caller holds timeout_lock and has
 * verified that @to is active.
 */
స్థిర అంతఃవరుస k_ticks_t z_timeout_q_remainder(స్థిరరాశి నిర్మాణం _timeout *గమ్యం)
{
	k_ticks_t ticks = 0;

	కొరకు (నిర్మాణం _timeout *t = z_timeout_q_మొదటి(); t != ఖాళీ;
	     t = z_timeout_q_తదుపరి(t)) {
		ticks += t->dticks;
		ఒకవేళ (గమ్యం == t) {
			ఆపు;
		}
	}

	తిరిగి ticks;
}

/* Ticks from (curr_tick + ticks_elapsed) until the next expiry, clamped to
 * int32 / SYS_CLOCK_MAX_WAIT. Empty queue yields SYS_CLOCK_MAX_WAIT.
 */
/* Ticks from curr_tick to the earliest pending timeout, or K_TICKS_FOREVER if
 * the queue is empty. The announce-range cap is applied centrally by
 * next_timeout() in timeout.c.
 */
స్థిర అంతఃవరుస k_ticks_t z_timeout_q_తదుపరి_expiry(శూన్య)
{
	నిర్మాణం _timeout *గమ్యం = z_timeout_q_మొదటి();

	తిరిగి (గమ్యం == ఖాళీ) ? K_TICKS_FOREVER : గమ్యం->dticks;
}

/* --- announce-loop helpers (see sys_clock_announce_locked() in timeout.c) --- */

/* Ticks from curr_tick until the next event the announce loop must act on
 * (here: the earliest expiry). A value larger than any announce window when
 * the queue is empty.
 */
స్థిర అంతఃవరుస int32_t z_timeout_q_తదుపరి_gap(శూన్య)
{
	నిర్మాణం _timeout *t = z_timeout_q_మొదటి();

	తిరిగి (t == ఖాళీ) ? INT32_MAX : (int32_t)MIN((int64_t)t->dticks, INT32_MAX);
}

/* Advance backend state by @dt ticks. For the delta list this shifts the head
 * closer to expiry; a head whose gap is fully consumed reads dticks == 0 and
 * is then drained by z_timeout_q_pop_due(). The caller advances curr_tick.
 */
స్థిర అంతఃవరుస శూన్య z_timeout_q_advance(int32_t dt)
{
	నిర్మాణం _timeout *t = z_timeout_q_మొదటి();

	ఒకవేళ (t != ఖాళీ) {
		t->dticks -= dt;
	}
}

/* Pop and return one timeout that is due at the current curr_tick, or NULL if
 * none is due right now. Strict head-only removal with no delta absorption:
 * z_timeout_q_advance() has already brought the head's dticks to 0, and
 * curr_tick was advanced to match, so the successor's stored delta stays
 * valid without propagation.
 */
స్థిర అంతఃవరుస నిర్మాణం _timeout *z_timeout_q_తీయి_due(శూన్య)
{
	నిర్మాణం _timeout *t = z_timeout_q_మొదటి();

	ఒకవేళ ((t == ఖాళీ) || (t->dticks != 0)) {
		తిరిగి ఖాళీ;
	}

	sys_dlist_remove(&t->కణుపు);

	తిరిగి t;
}

#అయితే_ముగింపు /* ZEPHYR_KERNEL_TIMEOUT_LIST_H_ */
