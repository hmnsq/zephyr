/*
 * Copyright (c) 2018 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ ZEPHYR_KERNEL_TIMEOUT_LIST_H_
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ZEPHYR_KERNEL_TIMEOUT_LIST_H_

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

#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/sys/dlist.h>

ਸਥਿਰ sys_dlist_t timeout_list = SYS_DLIST_STATIC_INIT(&timeout_list);

ਸਥਿਰ ਸਤਰਅੰਦਰ ਬਣਤਰ _timeout *z_timeout_q_ਪਹਿਲਾ(ਖਾਲੀ)
{
	sys_dnode_t *t = sys_dlist_peek_ਸਿਰ(&timeout_list);

	ਵਾਪਸ (t == ਨਿਰਾ) ? ਨਿਰਾ : CONTAINER_ਦਾ(t, ਬਣਤਰ _timeout, ਗੰਢ);
}

ਸਥਿਰ ਸਤਰਅੰਦਰ ਬਣਤਰ _timeout *z_timeout_q_ਅਗਲਾ(ਬਣਤਰ _timeout *t)
{
	sys_dnode_t *n = sys_dlist_peek_ਅਗਲਾ(&timeout_list, &t->ਗੰਢ);

	ਵਾਪਸ (n == ਨਿਰਾ) ? ਨਿਰਾ : CONTAINER_ਦਾ(n, ਬਣਤਰ _timeout, ਗੰਢ);
}

/* Insert @to so that it expires @dticks ticks after curr_tick. Returns true
 * if @to is now the earliest pending timeout.
 */
ਸਥਿਰ ਸਤਰਅੰਦਰ bool z_timeout_q_ਪਾਓ(ਬਣਤਰ _timeout *ਮੰਜ਼ਿਲ, k_ticks_t dticks)
{
	ਬਣਤਰ _timeout *t;

	ਮੰਜ਼ਿਲ->dticks = dticks;
	ਲਈ (t = z_timeout_q_ਪਹਿਲਾ(); t != ਨਿਰਾ; t = z_timeout_q_ਅਗਲਾ(t)) {
		ਜੇ (t->dticks > ਮੰਜ਼ਿਲ->dticks) {
			t->dticks -= ਮੰਜ਼ਿਲ->dticks;
			sys_dlist_ਪਾਓ(&t->ਗੰਢ, &ਮੰਜ਼ਿਲ->ਗੰਢ);
			ਵਾਪਸ z_timeout_q_ਪਹਿਲਾ() == ਮੰਜ਼ਿਲ;
		}
		ਮੰਜ਼ਿਲ->dticks -= t->dticks;
	}

	sys_dlist_append(&timeout_list, &ਮੰਜ਼ਿਲ->ਗੰਢ);
	ਵਾਪਸ z_timeout_q_ਪਹਿਲਾ() == ਮੰਜ਼ਿਲ;
}

/* Remove an arbitrary timeout from the queue, absorbing its delta into the
 * successor so the remaining deltas stay valid. Returns true if @to was the
 * earliest pending timeout. Used only by the abort path, never from announce.
 */
ਸਥਿਰ ਸਤਰਅੰਦਰ bool z_timeout_q_remove(ਬਣਤਰ _timeout *ਮੰਜ਼ਿਲ)
{
	bool was_ਪਹਿਲਾ = (z_timeout_q_ਪਹਿਲਾ() == ਮੰਜ਼ਿਲ);
	ਬਣਤਰ _timeout *n = z_timeout_q_ਅਗਲਾ(ਮੰਜ਼ਿਲ);

	ਜੇ (n != ਨਿਰਾ) {
		n->dticks += ਮੰਜ਼ਿਲ->dticks;
	}
	sys_dlist_remove(&ਮੰਜ਼ਿਲ->ਗੰਢ);

	ਵਾਪਸ was_ਪਹਿਲਾ;
}

/* Ticks from curr_tick until @to expires. Caller holds timeout_lock and has
 * verified that @to is active.
 */
ਸਥਿਰ ਸਤਰਅੰਦਰ k_ticks_t z_timeout_q_remainder(ਅਚਲ ਬਣਤਰ _timeout *ਮੰਜ਼ਿਲ)
{
	k_ticks_t ticks = 0;

	ਲਈ (ਬਣਤਰ _timeout *t = z_timeout_q_ਪਹਿਲਾ(); t != ਨਿਰਾ;
	     t = z_timeout_q_ਅਗਲਾ(t)) {
		ticks += t->dticks;
		ਜੇ (ਮੰਜ਼ਿਲ == t) {
			ਰੋਕੋ;
		}
	}

	ਵਾਪਸ ticks;
}

/* Ticks from (curr_tick + ticks_elapsed) until the next expiry, clamped to
 * int32 / SYS_CLOCK_MAX_WAIT. Empty queue yields SYS_CLOCK_MAX_WAIT.
 */
/* Ticks from curr_tick to the earliest pending timeout, or K_TICKS_FOREVER if
 * the queue is empty. The announce-range cap is applied centrally by
 * next_timeout() in timeout.c.
 */
ਸਥਿਰ ਸਤਰਅੰਦਰ k_ticks_t z_timeout_q_ਅਗਲਾ_expiry(ਖਾਲੀ)
{
	ਬਣਤਰ _timeout *ਮੰਜ਼ਿਲ = z_timeout_q_ਪਹਿਲਾ();

	ਵਾਪਸ (ਮੰਜ਼ਿਲ == ਨਿਰਾ) ? K_TICKS_FOREVER : ਮੰਜ਼ਿਲ->dticks;
}

/* --- announce-loop helpers (see sys_clock_announce_locked() in timeout.c) --- */

/* Ticks from curr_tick until the next event the announce loop must act on
 * (here: the earliest expiry). A value larger than any announce window when
 * the queue is empty.
 */
ਸਥਿਰ ਸਤਰਅੰਦਰ int32_t z_timeout_q_ਅਗਲਾ_gap(ਖਾਲੀ)
{
	ਬਣਤਰ _timeout *t = z_timeout_q_ਪਹਿਲਾ();

	ਵਾਪਸ (t == ਨਿਰਾ) ? INT32_MAX : (int32_t)MIN((int64_t)t->dticks, INT32_MAX);
}

/* Advance backend state by @dt ticks. For the delta list this shifts the head
 * closer to expiry; a head whose gap is fully consumed reads dticks == 0 and
 * is then drained by z_timeout_q_pop_due(). The caller advances curr_tick.
 */
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ z_timeout_q_advance(int32_t dt)
{
	ਬਣਤਰ _timeout *t = z_timeout_q_ਪਹਿਲਾ();

	ਜੇ (t != ਨਿਰਾ) {
		t->dticks -= dt;
	}
}

/* Pop and return one timeout that is due at the current curr_tick, or NULL if
 * none is due right now. Strict head-only removal with no delta absorption:
 * z_timeout_q_advance() has already brought the head's dticks to 0, and
 * curr_tick was advanced to match, so the successor's stored delta stays
 * valid without propagation.
 */
ਸਥਿਰ ਸਤਰਅੰਦਰ ਬਣਤਰ _timeout *z_timeout_q_ਕੱਢੋ_due(ਖਾਲੀ)
{
	ਬਣਤਰ _timeout *t = z_timeout_q_ਪਹਿਲਾ();

	ਜੇ ((t == ਨਿਰਾ) || (t->dticks != 0)) {
		ਵਾਪਸ ਨਿਰਾ;
	}

	sys_dlist_remove(&t->ਗੰਢ);

	ਵਾਪਸ t;
}

#ਜੇ_ਅੰਤ /* ZEPHYR_KERNEL_TIMEOUT_LIST_H_ */
