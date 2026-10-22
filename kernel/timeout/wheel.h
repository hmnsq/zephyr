/*
 * Copyright (c) 2018 Intel Corporation
 * Copyright (c) 2026 Peter Mitsis
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_KERNEL_TIMEOUT_WHEEL_H_
#define ZEPHYR_KERNEL_TIMEOUT_WHEEL_H_

/**
 * @file
 * @brief Hierarchical timer-wheel timeout backend (implementation).
 *
 * Timeouts are bucketed by how far in the future they expire:
 *   - "soon"    : one list per tick for the next NUM_SOON_LISTS ticks (O(1))
 *   - "later"   : one list per NUM_SOON_LISTS-tick band (O(1))
 *   - "distant" : a sorted delta list for everything beyond (O(n), but the
 *                 list is kept short)
 * Every NUM_SOON_LISTS ticks the announce path sifts the current "later" band
 * down into "soon" and refills it from "distant". Insertion into soon/later is
 * O(1); the win is large when many short-lived timeouts are pending.
 *
 * The wheel provides its own announce loop (z_timeout_q_announce, selected by
 * _TIMEOUT_BACKEND_OWNS_ANNOUNCE) rather than the generic
 * next_gap/advance/pop_due primitives: its per-tick advance is a bitmap scan
 * that jumps over empty ticks, and its sift is a time-driven event not tied to
 * any single timeout.
 *
 * Included only by kernel/timeout.c, after its shared state, so the wheel
 * state, the z_timeout_q_*() operations and the backend-owned announce loop
 * below are private to that translation unit and reach the front end's shared
 * state (curr_tick, announce_remaining, inflight_timeout) directly. The
 * per-node helpers live in kernel/include/timeout_q.h.
 */

#അന്തര്ഭാവ <zephyr/init.h>
#അന്തര്ഭാവ <zephyr/sys/dlist.h>

/* Per-node flag bits (struct _timeout's flags field): which tier the timeout
 * currently lives in, so removal can maintain the soon-list occupancy bitmap.
 */
#define _TIMEOUT_FLAG_TIMER_WHEEL_SOON  BIT(0)
#define _TIMEOUT_FLAG_TIMER_WHEEL_LATER BIT(1)

/* This backend supplies its own announce loop. */
#define _TIMEOUT_BACKEND_OWNS_ANNOUNCE 1

#define അക്കം_SOON_LISTS  32
#define അക്കം_LATER_LISTS 32

ഘടന timeout_wheel {
	sys_dlist_t soon[അക്കം_SOON_LISTS];   /* timeouts expiring soon lists */
	sys_dlist_t later[അക്കം_LATER_LISTS]; /* timeouts expiring later lists */
	sys_dlist_t distant;                /* timeouts expiring far in future */
	sys_dlist_t soon_defer;             /* special soon list */
	uint32_t    soon_bits;              /* Set bit indicates list in use */
	uint32_t    soon_index;             /* Current soon list index */
	uint32_t    later_index;
	uint32_t    announcing;             /* 1: Announcing soon_index, else 0 */
	uint32_t    sifted;                 /* 1: Sifted, else 0 */
};

സ്ഥിതം ഘടന timeout_wheel wheel;

സ്ഥിതം പൂർണ്ണസംഖ്യ timeout_wheel_init(ശൂന്യം)
{
	unsigned പൂർണ്ണസംഖ്യ i;

	വേണ്ടി (i = 0; i < അക്കം_SOON_LISTS; i++) {
		sys_dlist_init(&wheel.soon[i]);
	}

	വേണ്ടി (i = 0; i < അക്കം_LATER_LISTS; i++) {
		sys_dlist_init(&wheel.later[i]);
	}

	sys_dlist_init(&wheel.distant);
	sys_dlist_init(&wheel.soon_defer);

	wheel.soon_bits = 0;
	wheel.soon_index = അക്കം_SOON_LISTS - 1;
	wheel.later_index = 0;
	wheel.announcing = 0;
	wheel.sifted = 1;

	തിരികെ 0;
}

/* Initialize the wheel before the timer driver (PRE_KERNEL_2) and before any
 * timeout can be added. Priority 0 keeps it ahead of other PRE_KERNEL_1 init.
 */
SYS_INIT(timeout_wheel_init, PRE_KERNEL_1, 0);

സ്ഥിതം വരിയിൽ ശൂന്യം timeout_wheel_clear_flags(ഘടന _timeout *to)
{
	to->flags &= ~(_TIMEOUT_FLAG_TIMER_WHEEL_SOON |
		       _TIMEOUT_FLAG_TIMER_WHEEL_LATER);
}

/* Return the last item in the specified list, or NULL if the list is empty. */
സ്ഥിതം ഘടന _timeout *timeout_wheel_last_in_list(sys_dlist_t *list)
{
	sys_dnode_t *t = sys_dlist_peek_tail(list);

	തിരികെ (t == ശൂന്യ) ? ശൂന്യ : CONTAINER_OF(t, ഘടന _timeout, കണ്ണി);
}

/* Return the first timeout expiring in the specified list, or NULL. */
സ്ഥിതം ഘടന _timeout *timeout_wheel_first_in_list(sys_dlist_t *list)
{
	sys_dnode_t *t = sys_dlist_peek_head(list);

	തിരികെ (t == ശൂന്യ) ? ശൂന്യ : CONTAINER_OF(t, ഘടന _timeout, കണ്ണി);
}

/*
 * Get the first timeout in the wheel from its 'soon' lists. NULL means that
 * the first timeout is the sift operation which always implicitly exists
 * within the final 'soon' list.
 */
സ്ഥിതം ഘടന _timeout *timeout_wheel_first(ശൂന്യം)
{
	uint32_t bitmap;
	unsigned പൂർണ്ണസംഖ്യ index;
	sys_dnode_t *t;

	bitmap = wheel.soon_bits | BIT(അക്കം_SOON_LISTS - 1);
	എങ്കിൽ (wheel.sifted == 0) {
		bitmap &= ~(BIT(wheel.soon_index) - 1);
	}
	index = u32_എണ്ണം_trailing_zeros(bitmap);

	t = sys_dlist_peek_head(&wheel.soon[index]);

	തിരികെ (t == ശൂന്യ) ? ശൂന്യ : CONTAINER_OF(t, ഘടന _timeout, കണ്ണി);
}

/* Return the timeout following @a t expiring "distantly". */
സ്ഥിതം ഘടന _timeout *timeout_wheel_next_distant(ഘടന _timeout *t)
{
	sys_dnode_t *n = sys_dlist_peek_next(&wheel.distant, &t->കണ്ണി);

	തിരികെ (n == ശൂന്യ) ? ശൂന്യ : CONTAINER_OF(n, ഘടന _timeout, കണ്ണി);
}

/*
 * Prepend a timeout to the appropriate "soon" list. Only called during sifting
 * after the timeout had been removed from a 'later' list, where its 'dticks'
 * field was a composite value. Prepending preserves time ordering.
 */
സ്ഥിതം ശൂന്യം timeout_wheel_prepend_soon(ഘടന _timeout *to)
{
	unsigned പൂർണ്ണസംഖ്യ index = to->dticks & (അക്കം_SOON_LISTS - 1);

	wheel.soon_bits |= BIT(index);
	sys_dlist_prepend(&wheel.soon[index], &to->കണ്ണി);
	to->flags |= _TIMEOUT_FLAG_TIMER_WHEEL_SOON;
	to->dticks = index;
}

/*
 * Append a timeout to the appropriate "soon" list, recording the list index
 * in the timeout's 'dticks' field.
 */
സ്ഥിതം ശൂന്യം timeout_wheel_append_soon(ഘടന _timeout *to)
{
	unsigned പൂർണ്ണസംഖ്യ index;

	index = (to->dticks + wheel.soon_index) & (അക്കം_SOON_LISTS - 1);
	to->dticks = index;

	wheel.soon_bits |= BIT(index);
	sys_dlist_append(&wheel.soon[index], &to->കണ്ണി);
	to->flags |= _TIMEOUT_FLAG_TIMER_WHEEL_SOON;
}

/*
 * Append a timeout to the soon deferred list. Special case for timeouts that
 * are exactly NUM_SOON_LISTS into the future while the system is announcing:
 * they must not get mixed with the currently-expiring rotation. They move into
 * the appropriate 'soon' list at the end of the announce.
 */
സ്ഥിതം ശൂന്യം timeout_wheel_append_soon_defer(ഘടന _timeout *to)
{
	unsigned പൂർണ്ണസംഖ്യ index;

	index = (to->dticks + wheel.soon_index) & (അക്കം_SOON_LISTS - 1);
	to->dticks = index + അക്കം_SOON_LISTS;

	sys_dlist_append(&wheel.soon_defer, &to->കണ്ണി);
	to->flags |= _TIMEOUT_FLAG_TIMER_WHEEL_SOON;
}

/*
 * Append a timeout to the appropriate "later" list. The 'dticks' field is set
 * to identify both the eventual 'soon' list and the 'later' list it goes to.
 */
സ്ഥിതം ശൂന്യം timeout_wheel_append_later(ഘടന _timeout *to)
{
	uint32_t position;
	uint32_t soon;
	uint32_t later;

	position = to->dticks + wheel.soon_index - അക്കം_SOON_LISTS;

	soon = position & (അക്കം_SOON_LISTS - 1);
	later = ((position / അക്കം_SOON_LISTS) + wheel.later_index -
		 wheel.sifted) & (അക്കം_LATER_LISTS - 1);

	to->dticks = (later * അക്കം_SOON_LISTS) + soon;
	to->flags |= _TIMEOUT_FLAG_TIMER_WHEEL_LATER;

	sys_dlist_append(&wheel.later[later], &to->കണ്ണി);
}

/*
 * Insert a timeout into the "distant" list, an expiry-ordered delta list. For
 * good performance the distant list should stay short.
 */
സ്ഥിതം ശൂന്യം timeout_wheel_തിരുകുക_distant(ഘടന _timeout *to, uint32_t cutoff)
{
	ഘടന _timeout *t;

	to->dticks -= cutoff;

	വേണ്ടി (t = timeout_wheel_first_in_list(&wheel.distant);
	     t != ശൂന്യ; t = timeout_wheel_next_distant(t)) {
		എങ്കിൽ (t->dticks > to->dticks) {
			t->dticks -= to->dticks;
			sys_dlist_തിരുകുക(&t->കണ്ണി, &to->കണ്ണി);
			നിർത്തുക;
		}
		to->dticks -= t->dticks;
	}

	എങ്കിൽ (t == ശൂന്യ) {
		sys_dlist_append(&wheel.distant, &to->കണ്ണി);
	}
}

/*
 * Add a timeout to the wheel (with to->dticks already set to its tick delta
 * from curr_tick). It is never placed into the 'soon_index' list.
 */
സ്ഥിതം ശൂന്യം timeout_wheel_add_timeout(ഘടന _timeout *to)
{
	__ASSERT_NO_MSG(to->dticks != 0);

	എങ്കിൽ (to->dticks <= (അക്കം_SOON_LISTS - wheel.announcing)) {
		timeout_wheel_append_soon(to);
		തിരികെ;
	}

	എങ്കിൽ (to->dticks == അക്കം_SOON_LISTS) {
		timeout_wheel_append_soon_defer(to);
		തിരികെ;
	}

	uint32_t cutoff;

	cutoff = ((അക്കം_LATER_LISTS + wheel.sifted + 1) *
		  അക്കം_SOON_LISTS) - wheel.soon_index - 1;
	എങ്കിൽ (to->dticks <= cutoff) {
		timeout_wheel_append_later(to);
		തിരികെ;
	}

	timeout_wheel_തിരുകുക_distant(to, cutoff);
}

/* Remove the specified timeout. */
സ്ഥിതം ശൂന്യം timeout_wheel_remove_timeout(ഘടന _timeout *to)
{
	sys_dlist_remove(&to->കണ്ണി);

	എങ്കിൽ (to->flags & _TIMEOUT_FLAG_TIMER_WHEEL_SOON) {
		unsigned പൂർണ്ണസംഖ്യ index = to->dticks;

		/* soon_defer entries carry index + NUM_SOON_LISTS and own no
		 * soon_bits bit; only true soon-list entries do.
		 */
		എങ്കിൽ (index < അക്കം_SOON_LISTS &&
		    sys_dlist_is_empty(&wheel.soon[index])) {
			wheel.soon_bits &= ~BIT(index);
		}
	}

	timeout_wheel_clear_flags(to);
}

/* Calculate the number of ticks until the specified timeout expires. */
സ്ഥിതം k_ticks_t timeout_wheel_remainder(സ്ഥിരം ഘടന _timeout *to)
{
	എങ്കിൽ (to->flags & _TIMEOUT_FLAG_TIMER_WHEEL_SOON) {
		uint32_t now = wheel.soon_index;
		uint32_t idx = to->dticks;

		എങ്കിൽ (idx == now) {
			തിരികെ (1U - wheel.announcing) * അക്കം_SOON_LISTS;
		} അല്ലെങ്കിൽ എങ്കിൽ (idx >= അക്കം_SOON_LISTS) {
			/* Special case if querying item in 'soon_defer' list */
			തിരികെ അക്കം_SOON_LISTS;
		}

		തിരികെ (idx - now) & (അക്കം_SOON_LISTS - 1U);
	}

	എങ്കിൽ (to->flags & _TIMEOUT_FLAG_TIMER_WHEEL_LATER) {
		uint32_t later_slot = to->dticks / അക്കം_SOON_LISTS;
		uint32_t soon_index = to->dticks & (അക്കം_SOON_LISTS - 1U);

		/* later slots ahead until the bucket is sifted */
		uint32_t slots_ahead = ((later_slot - wheel.later_index) &
					(അക്കം_LATER_LISTS - 1U)) + wheel.sifted;

		തിരികെ (k_ticks_t)((അക്കം_SOON_LISTS - wheel.soon_index) +
				   (slots_ahead * അക്കം_SOON_LISTS) + soon_index);
	}

	ഘടന _timeout *t;
	k_ticks_t remaining;

	remaining  = ((അക്കം_LATER_LISTS + wheel.sifted + 1) *
		      അക്കം_SOON_LISTS) - wheel.soon_index - 1;

	വേണ്ടി (t = timeout_wheel_first_in_list(&wheel.distant);
	     t != ശൂന്യ; t = timeout_wheel_next_distant(t)) {
		remaining += t->dticks;
		എങ്കിൽ (t == to) {
			നിർത്തുക;
		}
	}

	തിരികെ remaining;
}

/*
 * Sift from a 'later' list down into the appropriate 'soon' lists. Timeouts
 * are prepended to the 'soon' list to preserve time ordering.
 */
സ്ഥിതം ശൂന്യം timeout_wheel_sift_down_to_soon(ശൂന്യം)
{
	sys_dlist_t *list;
	ഘടന _timeout *t;

	list = &wheel.later[wheel.later_index];
	ആകുമ്പോൾ ((t = timeout_wheel_last_in_list(list)) != ശൂന്യ) {
		sys_dlist_remove(&t->കണ്ണി);
		timeout_wheel_clear_flags(t);
		timeout_wheel_prepend_soon(t);
	}
}

/*
 * Sift from the 'distant' list down into the appropriate 'later' list. Two
 * steps: prepare (massage dticks/flags of the leading distant timeouts) then
 * move them en masse into the 'later' list.
 */
സ്ഥിതം ശൂന്യം timeout_wheel_sift_down_to_later(ശൂന്യം)
{
	ഘടന _timeout *t;
	int64_t sift_ticks = അക്കം_SOON_LISTS;
	ഘടന _timeout *head = timeout_wheel_first_in_list(&wheel.distant);
	ഘടന _timeout *tail = ശൂന്യ;
	പൂർണ്ണസംഖ്യ new_dticks = wheel.later_index * അക്കം_SOON_LISTS;

	വേണ്ടി (t = head; t != ശൂന്യ; t = timeout_wheel_next_distant(t)) {
		എങ്കിൽ (t->dticks >= sift_ticks) {
			t->dticks -= sift_ticks;
			നിർത്തുക;
		}

		sift_ticks -= t->dticks;
		new_dticks += t->dticks;

		t->flags |= _TIMEOUT_FLAG_TIMER_WHEEL_LATER;
		t->dticks = new_dticks;
		tail = t;
	}

	എങ്കിൽ (tail != ശൂന്യ) {
		sys_dlist_range_prepend(&wheel.later[wheel.later_index],
					&head->കണ്ണി, &tail->കണ്ണി);
	}
}

/*
 * Sifting runs every NUM_SOON_LISTS ticks: first move the current 'later' list
 * into the 'soon' lists, freeing it to receive the next batch sifted from
 * 'distant'.
 */
സ്ഥിതം ശൂന്യം timeout_wheel_sift_down(ശൂന്യം)
{
	timeout_wheel_sift_down_to_soon();

	timeout_wheel_sift_down_to_later();

	/* Advance to the next 'later' list */
	wheel.later_index++;
	wheel.later_index &= (അക്കം_LATER_LISTS - 1);
	wheel.sifted = 1;
}

/*
 * Announce all the timeouts expiring "now" (at soon_index). curr_tick has been
 * advanced. Handlers fire with the lock dropped; inflight_timeout tracks
 * the in-flight timeout so aborts race correctly.
 */
സ്ഥിതം k_spinlock_താക്കോൽ_t timeout_wheel_announce_now(k_spinlock_താക്കോൽ_t താക്കോൽ)
{
	ഘടന _timeout *t;
	uint32_t index = wheel.soon_index;

	wheel.sifted = 0;
	wheel.announcing = 1;
	വേണ്ടി (t = timeout_wheel_first_in_list(&wheel.soon[index]);
	     t != ശൂന്യ;
	     t = timeout_wheel_first_in_list(&wheel.soon[index])) {
		sys_dlist_remove(&t->കണ്ണി);
		timeout_wheel_clear_flags(t);

		inflight_timeout = t;
		k_spin_unlock(&timeout_lock, താക്കോൽ);
		t->fn(t);
		താക്കോൽ = k_spin_lock(&timeout_lock);
		inflight_timeout = ശൂന്യ;
	}
	wheel.announcing = 0;

	/*
	 * The 'soon_index' slot is now drained, so 'soon deferred' timeouts
	 * (exactly NUM_SOON_LISTS into the future) can move into their slots.
	 */
	വേണ്ടി (t = timeout_wheel_first_in_list(&wheel.soon_defer);
	     t != ശൂന്യ;
	     t = timeout_wheel_first_in_list(&wheel.soon_defer)) {
		sys_dlist_remove(&t->കണ്ണി);
		/* Correct the dticks to be the appropriate 'soon' list index */
		t->dticks &= (അക്കം_SOON_LISTS - 1);
		wheel.soon_bits |= BIT(t->dticks);
		sys_dlist_append(&wheel.soon[t->dticks], &t->കണ്ണി);
	}

	എങ്കിൽ (index == (അക്കം_SOON_LISTS - 1)) {
		timeout_wheel_sift_down();
	}

	തിരികെ താക്കോൽ;
}

/*
 * Backend interface (see kernel/timeout.c).
 */

സ്ഥിതം bool z_timeout_q_തിരുകുക(ഘടന _timeout *to, k_ticks_t dticks)
{
	to->dticks = dticks;
	timeout_wheel_add_timeout(to);

	തിരികെ to == timeout_wheel_first();
}

സ്ഥിതം bool z_timeout_q_remove(ഘടന _timeout *to)
{
	bool was_first = (to == timeout_wheel_first());

	timeout_wheel_remove_timeout(to);

	തിരികെ was_first;
}

സ്ഥിതം k_ticks_t z_timeout_q_remainder(സ്ഥിരം ഘടന _timeout *to)
{
	തിരികെ timeout_wheel_remainder(to);
}

/*
 * Ticks from curr_tick to the next 'soon' timeout, between 1 and
 * NUM_SOON_LISTS (the 'soon_index' list is excluded as it is timing out).
 * Never K_TICKS_FOREVER: a sift is always pending, which is why the wheel
 * wakes a tickless-idle CPU at least every NUM_SOON_LISTS ticks. The
 * announce-range cap is applied centrally by next_timeout() in timeout.c.
 */
സ്ഥിതം k_ticks_t z_timeout_q_next_expiry(ശൂന്യം)
{
	ഘടന _timeout *to = timeout_wheel_first();
	int64_t dticks = (to != ശൂന്യ) ? to->dticks : (അക്കം_SOON_LISTS - 1);

	dticks = ((അക്കം_SOON_LISTS - 1 - wheel.soon_index + dticks) &
		  (അക്കം_SOON_LISTS - 1)) + 1;

	തിരികെ (k_ticks_t)dticks;
}

/* Backend-owned announce loop (see _TIMEOUT_BACKEND_OWNS_ANNOUNCE). Drains all
 * timeouts due within announce_remaining ticks, firing each handler with the
 * subsystem lock dropped; advances curr_tick and announce_remaining. Returns
 * the (possibly re-acquired) spinlock key.
 */
സ്ഥിതം k_spinlock_താക്കോൽ_t z_timeout_q_announce(k_spinlock_താക്കോൽ_t താക്കോൽ)
{
	uint32_t mask;
	പൂർണ്ണസംഖ്യ      next;    /* Index of next 'soon' list to expire */
	പൂർണ്ണസംഖ്യ      അക്കം_ticks_to_advance;
	പൂർണ്ണസംഖ്യ      correction;  /* NUM_SOON_LISTS or 0 */
	uint32_t bits;

	ആകുമ്പോൾ (announce_remaining > 0) {

		mask = 0xFFFFFFFF << ((wheel.soon_index + 1) & (അക്കം_SOON_LISTS - 1));
		bits = wheel.soon_bits | BIT(അക്കം_SOON_LISTS - 1);
		next = u32_എണ്ണം_trailing_zeros(mask & bits);
		correction = ((wheel.soon_index + 1) / അക്കം_SOON_LISTS) * അക്കം_SOON_LISTS;
		അക്കം_ticks_to_advance = next + correction - wheel.soon_index;

		എങ്കിൽ (അക്കം_ticks_to_advance > announce_remaining) {
			curr_tick += announce_remaining;
			wheel.sifted = 0;
			wheel.soon_index += announce_remaining;
			wheel.soon_index &= (അക്കം_SOON_LISTS - 1);

			announce_remaining = 0;
			നിർത്തുക;
		}

		/*
		 * 1. Advance to the next tick to process.
		 * 2. Clear its bit in the 'soon_bits'.
		 */
		wheel.soon_index += അക്കം_ticks_to_advance;
		wheel.soon_index &= (അക്കം_SOON_LISTS - 1);
		wheel.soon_bits &= ~BIT(wheel.soon_index);

		curr_tick += അക്കം_ticks_to_advance;
		താക്കോൽ = timeout_wheel_announce_now(താക്കോൽ);
		announce_remaining -= അക്കം_ticks_to_advance;
	}

	തിരികെ താക്കോൽ;
}

#endif /* ZEPHYR_KERNEL_TIMEOUT_WHEEL_H_ */
