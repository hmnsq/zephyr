/*
 * Copyright (c) 2018 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#अगर_अघोषित ZEPHYR_KERNEL_TIMEOUT_LIST_H_
#घोषणा ZEPHYR_KERNEL_TIMEOUT_LIST_H_

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

#समावेश <zephyr/sys/dlist.h>

अटल sys_dlist_t timeout_list = SYS_DLIST_STATIC_INIT(&timeout_list);

अटल अंतरभूत काष्ठा _timeout *z_timeout_q_first(व्योम)
{
	sys_dnode_t *t = sys_dlist_peek_शीर्ष(&timeout_list);

	वापस (t == शून्य) ? शून्य : CONTAINER_OF(t, काष्ठा _timeout, गाँठ);
}

अटल अंतरभूत काष्ठा _timeout *z_timeout_q_अग्र(काष्ठा _timeout *t)
{
	sys_dnode_t *n = sys_dlist_peek_अग्र(&timeout_list, &t->गाँठ);

	वापस (n == शून्य) ? शून्य : CONTAINER_OF(n, काष्ठा _timeout, गाँठ);
}

/* Insert @to so that it expires @dticks ticks after curr_tick. Returns true
 * if @to is now the earliest pending timeout.
 */
अटल अंतरभूत bool z_timeout_q_जोड़ो(काष्ठा _timeout *to, k_ticks_t dticks)
{
	काष्ठा _timeout *t;

	to->dticks = dticks;
	क्रम (t = z_timeout_q_first(); t != शून्य; t = z_timeout_q_अग्र(t)) {
		अगर (t->dticks > to->dticks) {
			t->dticks -= to->dticks;
			sys_dlist_जोड़ो(&t->गाँठ, &to->गाँठ);
			वापस z_timeout_q_first() == to;
		}
		to->dticks -= t->dticks;
	}

	sys_dlist_append(&timeout_list, &to->गाँठ);
	वापस z_timeout_q_first() == to;
}

/* Remove an arbitrary timeout from the queue, absorbing its delta into the
 * successor so the remaining deltas stay valid. Returns true if @to was the
 * earliest pending timeout. Used only by the abort path, never from announce.
 */
अटल अंतरभूत bool z_timeout_q_remove(काष्ठा _timeout *to)
{
	bool was_first = (z_timeout_q_first() == to);
	काष्ठा _timeout *n = z_timeout_q_अग्र(to);

	अगर (n != शून्य) {
		n->dticks += to->dticks;
	}
	sys_dlist_remove(&to->गाँठ);

	वापस was_first;
}

/* Ticks from curr_tick until @to expires. Caller holds timeout_lock and has
 * verified that @to is active.
 */
अटल अंतरभूत k_ticks_t z_timeout_q_remainder(स्थिर काष्ठा _timeout *to)
{
	k_ticks_t ticks = 0;

	क्रम (काष्ठा _timeout *t = z_timeout_q_first(); t != शून्य;
	     t = z_timeout_q_अग्र(t)) {
		ticks += t->dticks;
		अगर (to == t) {
			अवरोध;
		}
	}

	वापस ticks;
}

/* Ticks from (curr_tick + ticks_elapsed) until the next expiry, clamped to
 * int32 / SYS_CLOCK_MAX_WAIT. Empty queue yields SYS_CLOCK_MAX_WAIT.
 */
/* Ticks from curr_tick to the earliest pending timeout, or K_TICKS_FOREVER if
 * the queue is empty. The announce-range cap is applied centrally by
 * next_timeout() in timeout.c.
 */
अटल अंतरभूत k_ticks_t z_timeout_q_अग्र_expiry(व्योम)
{
	काष्ठा _timeout *to = z_timeout_q_first();

	वापस (to == शून्य) ? K_TICKS_FOREVER : to->dticks;
}

/* --- announce-loop helpers (see sys_clock_announce_locked() in timeout.c) --- */

/* Ticks from curr_tick until the next event the announce loop must act on
 * (here: the earliest expiry). A value larger than any announce window when
 * the queue is empty.
 */
अटल अंतरभूत int32_t z_timeout_q_अग्र_gap(व्योम)
{
	काष्ठा _timeout *t = z_timeout_q_first();

	वापस (t == शून्य) ? INT32_MAX : (int32_t)MIN((int64_t)t->dticks, INT32_MAX);
}

/* Advance backend state by @dt ticks. For the delta list this shifts the head
 * closer to expiry; a head whose gap is fully consumed reads dticks == 0 and
 * is then drained by z_timeout_q_pop_due(). The caller advances curr_tick.
 */
अटल अंतरभूत व्योम z_timeout_q_advance(int32_t dt)
{
	काष्ठा _timeout *t = z_timeout_q_first();

	अगर (t != शून्य) {
		t->dticks -= dt;
	}
}

/* Pop and return one timeout that is due at the current curr_tick, or NULL if
 * none is due right now. Strict head-only removal with no delta absorption:
 * z_timeout_q_advance() has already brought the head's dticks to 0, and
 * curr_tick was advanced to match, so the successor's stored delta stays
 * valid without propagation.
 */
अटल अंतरभूत काष्ठा _timeout *z_timeout_q_निकालो_due(व्योम)
{
	काष्ठा _timeout *t = z_timeout_q_first();

	अगर ((t == शून्य) || (t->dticks != 0)) {
		वापस शून्य;
	}

	sys_dlist_remove(&t->गाँठ);

	वापस t;
}

#पूर्ण_अगर /* ZEPHYR_KERNEL_TIMEOUT_LIST_H_ */
