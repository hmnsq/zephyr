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

#अन्तर्भाव <zephyr/init.h>
#अन्तर्भाव <zephyr/sys/dlist.h>

/* Per-node flag bits (struct _timeout's flags field): which tier the timeout
 * currently lives in, so removal can maintain the soon-list occupancy bitmap.
 */
#define _TIMEOUT_FLAG_TIMER_WHEEL_SOON  BIT(0)
#define _TIMEOUT_FLAG_TIMER_WHEEL_LATER BIT(1)

/* This backend supplies its own announce loop. */
#define _TIMEOUT_BACKEND_OWNS_ANNOUNCE 1

#define अंक_SOON_LISTS  32
#define अंक_LATER_LISTS 32

संरचना timeout_wheel {
	sys_dlist_t soon[अंक_SOON_LISTS];   /* timeouts expiring soon lists */
	sys_dlist_t later[अंक_LATER_LISTS]; /* timeouts expiring later lists */
	sys_dlist_t distant;                /* timeouts expiring far in future */
	sys_dlist_t soon_defer;             /* special soon list */
	uint32_t    soon_bits;              /* Set bit indicates list in use */
	uint32_t    soon_index;             /* Current soon list index */
	uint32_t    later_index;
	uint32_t    announcing;             /* 1: Announcing soon_index, else 0 */
	uint32_t    sifted;                 /* 1: Sifted, else 0 */
};

स्थिर संरचना timeout_wheel wheel;

स्थिर पूर्णाङ्क timeout_wheel_init(शून्य)
{
	अचिन्हित पूर्णाङ्क i;

	प्रति (i = 0; i < अंक_SOON_LISTS; i++) {
		sys_dlist_init(&wheel.soon[i]);
	}

	प्रति (i = 0; i < अंक_LATER_LISTS; i++) {
		sys_dlist_init(&wheel.later[i]);
	}

	sys_dlist_init(&wheel.distant);
	sys_dlist_init(&wheel.soon_defer);

	wheel.soon_bits = 0;
	wheel.soon_index = अंक_SOON_LISTS - 1;
	wheel.later_index = 0;
	wheel.announcing = 0;
	wheel.sifted = 1;

	प्रत्यागम 0;
}

/* Initialize the wheel before the timer driver (PRE_KERNEL_2) and before any
 * timeout can be added. Priority 0 keeps it ahead of other PRE_KERNEL_1 init.
 */
SYS_INIT(timeout_wheel_init, PRE_KERNEL_1, 0);

स्थिर अन्तःस्थ शून्य timeout_wheel_clear_flags(संरचना _timeout *to)
{
	to->flags &= ~(_TIMEOUT_FLAG_TIMER_WHEEL_SOON |
		       _TIMEOUT_FLAG_TIMER_WHEEL_LATER);
}

/* Return the last item in the specified list, or NULL if the list is empty. */
स्थिर संरचना _timeout *timeout_wheel_last_in_list(sys_dlist_t *list)
{
	sys_dnode_t *t = sys_dlist_peek_tail(list);

	प्रत्यागम (t == रिक्त) ? रिक्त : CONTAINER_OF(t, संरचना _timeout, गाँठ);
}

/* Return the first timeout expiring in the specified list, or NULL. */
स्थिर संरचना _timeout *timeout_wheel_first_in_list(sys_dlist_t *list)
{
	sys_dnode_t *t = sys_dlist_peek_head(list);

	प्रत्यागम (t == रिक्त) ? रिक्त : CONTAINER_OF(t, संरचना _timeout, गाँठ);
}

/*
 * Get the first timeout in the wheel from its 'soon' lists. NULL means that
 * the first timeout is the sift operation which always implicitly exists
 * within the final 'soon' list.
 */
स्थिर संरचना _timeout *timeout_wheel_first(शून्य)
{
	uint32_t bitmap;
	अचिन्हित पूर्णाङ्क index;
	sys_dnode_t *t;

	bitmap = wheel.soon_bits | BIT(अंक_SOON_LISTS - 1);
	यदि (wheel.sifted == 0) {
		bitmap &= ~(BIT(wheel.soon_index) - 1);
	}
	index = u32_गिनती_trailing_zeros(bitmap);

	t = sys_dlist_peek_head(&wheel.soon[index]);

	प्रत्यागम (t == रिक्त) ? रिक्त : CONTAINER_OF(t, संरचना _timeout, गाँठ);
}

/* Return the timeout following @a t expiring "distantly". */
स्थिर संरचना _timeout *timeout_wheel_next_distant(संरचना _timeout *t)
{
	sys_dnode_t *n = sys_dlist_peek_next(&wheel.distant, &t->गाँठ);

	प्रत्यागम (n == रिक्त) ? रिक्त : CONTAINER_OF(n, संरचना _timeout, गाँठ);
}

/*
 * Prepend a timeout to the appropriate "soon" list. Only called during sifting
 * after the timeout had been removed from a 'later' list, where its 'dticks'
 * field was a composite value. Prepending preserves time ordering.
 */
स्थिर शून्य timeout_wheel_prepend_soon(संरचना _timeout *to)
{
	अचिन्हित पूर्णाङ्क index = to->dticks & (अंक_SOON_LISTS - 1);

	wheel.soon_bits |= BIT(index);
	sys_dlist_prepend(&wheel.soon[index], &to->गाँठ);
	to->flags |= _TIMEOUT_FLAG_TIMER_WHEEL_SOON;
	to->dticks = index;
}

/*
 * Append a timeout to the appropriate "soon" list, recording the list index
 * in the timeout's 'dticks' field.
 */
स्थिर शून्य timeout_wheel_append_soon(संरचना _timeout *to)
{
	अचिन्हित पूर्णाङ्क index;

	index = (to->dticks + wheel.soon_index) & (अंक_SOON_LISTS - 1);
	to->dticks = index;

	wheel.soon_bits |= BIT(index);
	sys_dlist_append(&wheel.soon[index], &to->गाँठ);
	to->flags |= _TIMEOUT_FLAG_TIMER_WHEEL_SOON;
}

/*
 * Append a timeout to the soon deferred list. Special case for timeouts that
 * are exactly NUM_SOON_LISTS into the future while the system is announcing:
 * they must not get mixed with the currently-expiring rotation. They move into
 * the appropriate 'soon' list at the end of the announce.
 */
स्थिर शून्य timeout_wheel_append_soon_defer(संरचना _timeout *to)
{
	अचिन्हित पूर्णाङ्क index;

	index = (to->dticks + wheel.soon_index) & (अंक_SOON_LISTS - 1);
	to->dticks = index + अंक_SOON_LISTS;

	sys_dlist_append(&wheel.soon_defer, &to->गाँठ);
	to->flags |= _TIMEOUT_FLAG_TIMER_WHEEL_SOON;
}

/*
 * Append a timeout to the appropriate "later" list. The 'dticks' field is set
 * to identify both the eventual 'soon' list and the 'later' list it goes to.
 */
स्थिर शून्य timeout_wheel_append_later(संरचना _timeout *to)
{
	uint32_t position;
	uint32_t soon;
	uint32_t later;

	position = to->dticks + wheel.soon_index - अंक_SOON_LISTS;

	soon = position & (अंक_SOON_LISTS - 1);
	later = ((position / अंक_SOON_LISTS) + wheel.later_index -
		 wheel.sifted) & (अंक_LATER_LISTS - 1);

	to->dticks = (later * अंक_SOON_LISTS) + soon;
	to->flags |= _TIMEOUT_FLAG_TIMER_WHEEL_LATER;

	sys_dlist_append(&wheel.later[later], &to->गाँठ);
}

/*
 * Insert a timeout into the "distant" list, an expiry-ordered delta list. For
 * good performance the distant list should stay short.
 */
स्थिर शून्य timeout_wheel_जोड़ो_distant(संरचना _timeout *to, uint32_t cutoff)
{
	संरचना _timeout *t;

	to->dticks -= cutoff;

	प्रति (t = timeout_wheel_first_in_list(&wheel.distant);
	     t != रिक्त; t = timeout_wheel_next_distant(t)) {
		यदि (t->dticks > to->dticks) {
			t->dticks -= to->dticks;
			sys_dlist_जोड़ो(&t->गाँठ, &to->गाँठ);
			भङ्ग;
		}
		to->dticks -= t->dticks;
	}

	यदि (t == रिक्त) {
		sys_dlist_append(&wheel.distant, &to->गाँठ);
	}
}

/*
 * Add a timeout to the wheel (with to->dticks already set to its tick delta
 * from curr_tick). It is never placed into the 'soon_index' list.
 */
स्थिर शून्य timeout_wheel_add_timeout(संरचना _timeout *to)
{
	__ASSERT_NO_MSG(to->dticks != 0);

	यदि (to->dticks <= (अंक_SOON_LISTS - wheel.announcing)) {
		timeout_wheel_append_soon(to);
		प्रत्यागम;
	}

	यदि (to->dticks == अंक_SOON_LISTS) {
		timeout_wheel_append_soon_defer(to);
		प्रत्यागम;
	}

	uint32_t cutoff;

	cutoff = ((अंक_LATER_LISTS + wheel.sifted + 1) *
		  अंक_SOON_LISTS) - wheel.soon_index - 1;
	यदि (to->dticks <= cutoff) {
		timeout_wheel_append_later(to);
		प्रत्यागम;
	}

	timeout_wheel_जोड़ो_distant(to, cutoff);
}

/* Remove the specified timeout. */
स्थिर शून्य timeout_wheel_remove_timeout(संरचना _timeout *to)
{
	sys_dlist_remove(&to->गाँठ);

	यदि (to->flags & _TIMEOUT_FLAG_TIMER_WHEEL_SOON) {
		अचिन्हित पूर्णाङ्क index = to->dticks;

		/* soon_defer entries carry index + NUM_SOON_LISTS and own no
		 * soon_bits bit; only true soon-list entries do.
		 */
		यदि (index < अंक_SOON_LISTS &&
		    sys_dlist_is_empty(&wheel.soon[index])) {
			wheel.soon_bits &= ~BIT(index);
		}
	}

	timeout_wheel_clear_flags(to);
}

/* Calculate the number of ticks until the specified timeout expires. */
स्थिर k_ticks_t timeout_wheel_remainder(ध्रुव संरचना _timeout *to)
{
	यदि (to->flags & _TIMEOUT_FLAG_TIMER_WHEEL_SOON) {
		uint32_t now = wheel.soon_index;
		uint32_t idx = to->dticks;

		यदि (idx == now) {
			प्रत्यागम (1U - wheel.announcing) * अंक_SOON_LISTS;
		} अन्यथा यदि (idx >= अंक_SOON_LISTS) {
			/* Special case if querying item in 'soon_defer' list */
			प्रत्यागम अंक_SOON_LISTS;
		}

		प्रत्यागम (idx - now) & (अंक_SOON_LISTS - 1U);
	}

	यदि (to->flags & _TIMEOUT_FLAG_TIMER_WHEEL_LATER) {
		uint32_t later_slot = to->dticks / अंक_SOON_LISTS;
		uint32_t soon_index = to->dticks & (अंक_SOON_LISTS - 1U);

		/* later slots ahead until the bucket is sifted */
		uint32_t slots_ahead = ((later_slot - wheel.later_index) &
					(अंक_LATER_LISTS - 1U)) + wheel.sifted;

		प्रत्यागम (k_ticks_t)((अंक_SOON_LISTS - wheel.soon_index) +
				   (slots_ahead * अंक_SOON_LISTS) + soon_index);
	}

	संरचना _timeout *t;
	k_ticks_t remaining;

	remaining  = ((अंक_LATER_LISTS + wheel.sifted + 1) *
		      अंक_SOON_LISTS) - wheel.soon_index - 1;

	प्रति (t = timeout_wheel_first_in_list(&wheel.distant);
	     t != रिक्त; t = timeout_wheel_next_distant(t)) {
		remaining += t->dticks;
		यदि (t == to) {
			भङ्ग;
		}
	}

	प्रत्यागम remaining;
}

/*
 * Sift from a 'later' list down into the appropriate 'soon' lists. Timeouts
 * are prepended to the 'soon' list to preserve time ordering.
 */
स्थिर शून्य timeout_wheel_sift_down_to_soon(शून्य)
{
	sys_dlist_t *list;
	संरचना _timeout *t;

	list = &wheel.later[wheel.later_index];
	यावत् ((t = timeout_wheel_last_in_list(list)) != रिक्त) {
		sys_dlist_remove(&t->गाँठ);
		timeout_wheel_clear_flags(t);
		timeout_wheel_prepend_soon(t);
	}
}

/*
 * Sift from the 'distant' list down into the appropriate 'later' list. Two
 * steps: prepare (massage dticks/flags of the leading distant timeouts) then
 * move them en masse into the 'later' list.
 */
स्थिर शून्य timeout_wheel_sift_down_to_later(शून्य)
{
	संरचना _timeout *t;
	int64_t sift_ticks = अंक_SOON_LISTS;
	संरचना _timeout *head = timeout_wheel_first_in_list(&wheel.distant);
	संरचना _timeout *tail = रिक्त;
	पूर्णाङ्क new_dticks = wheel.later_index * अंक_SOON_LISTS;

	प्रति (t = head; t != रिक्त; t = timeout_wheel_next_distant(t)) {
		यदि (t->dticks >= sift_ticks) {
			t->dticks -= sift_ticks;
			भङ्ग;
		}

		sift_ticks -= t->dticks;
		new_dticks += t->dticks;

		t->flags |= _TIMEOUT_FLAG_TIMER_WHEEL_LATER;
		t->dticks = new_dticks;
		tail = t;
	}

	यदि (tail != रिक्त) {
		sys_dlist_range_prepend(&wheel.later[wheel.later_index],
					&head->गाँठ, &tail->गाँठ);
	}
}

/*
 * Sifting runs every NUM_SOON_LISTS ticks: first move the current 'later' list
 * into the 'soon' lists, freeing it to receive the next batch sifted from
 * 'distant'.
 */
स्थिर शून्य timeout_wheel_sift_down(शून्य)
{
	timeout_wheel_sift_down_to_soon();

	timeout_wheel_sift_down_to_later();

	/* Advance to the next 'later' list */
	wheel.later_index++;
	wheel.later_index &= (अंक_LATER_LISTS - 1);
	wheel.sifted = 1;
}

/*
 * Announce all the timeouts expiring "now" (at soon_index). curr_tick has been
 * advanced. Handlers fire with the lock dropped; inflight_timeout tracks
 * the in-flight timeout so aborts race correctly.
 */
स्थिर k_spinlock_कुंजी_t timeout_wheel_announce_now(k_spinlock_कुंजी_t कुंजी)
{
	संरचना _timeout *t;
	uint32_t index = wheel.soon_index;

	wheel.sifted = 0;
	wheel.announcing = 1;
	प्रति (t = timeout_wheel_first_in_list(&wheel.soon[index]);
	     t != रिक्त;
	     t = timeout_wheel_first_in_list(&wheel.soon[index])) {
		sys_dlist_remove(&t->गाँठ);
		timeout_wheel_clear_flags(t);

		inflight_timeout = t;
		k_spin_unlock(&timeout_lock, कुंजी);
		t->fn(t);
		कुंजी = k_spin_lock(&timeout_lock);
		inflight_timeout = रिक्त;
	}
	wheel.announcing = 0;

	/*
	 * The 'soon_index' slot is now drained, so 'soon deferred' timeouts
	 * (exactly NUM_SOON_LISTS into the future) can move into their slots.
	 */
	प्रति (t = timeout_wheel_first_in_list(&wheel.soon_defer);
	     t != रिक्त;
	     t = timeout_wheel_first_in_list(&wheel.soon_defer)) {
		sys_dlist_remove(&t->गाँठ);
		/* Correct the dticks to be the appropriate 'soon' list index */
		t->dticks &= (अंक_SOON_LISTS - 1);
		wheel.soon_bits |= BIT(t->dticks);
		sys_dlist_append(&wheel.soon[t->dticks], &t->गाँठ);
	}

	यदि (index == (अंक_SOON_LISTS - 1)) {
		timeout_wheel_sift_down();
	}

	प्रत्यागम कुंजी;
}

/*
 * Backend interface (see kernel/timeout.c).
 */

स्थिर bool z_timeout_q_जोड़ो(संरचना _timeout *to, k_ticks_t dticks)
{
	to->dticks = dticks;
	timeout_wheel_add_timeout(to);

	प्रत्यागम to == timeout_wheel_first();
}

स्थिर bool z_timeout_q_remove(संरचना _timeout *to)
{
	bool was_first = (to == timeout_wheel_first());

	timeout_wheel_remove_timeout(to);

	प्रत्यागम was_first;
}

स्थिर k_ticks_t z_timeout_q_remainder(ध्रुव संरचना _timeout *to)
{
	प्रत्यागम timeout_wheel_remainder(to);
}

/*
 * Ticks from curr_tick to the next 'soon' timeout, between 1 and
 * NUM_SOON_LISTS (the 'soon_index' list is excluded as it is timing out).
 * Never K_TICKS_FOREVER: a sift is always pending, which is why the wheel
 * wakes a tickless-idle CPU at least every NUM_SOON_LISTS ticks. The
 * announce-range cap is applied centrally by next_timeout() in timeout.c.
 */
स्थिर k_ticks_t z_timeout_q_next_expiry(शून्य)
{
	संरचना _timeout *to = timeout_wheel_first();
	int64_t dticks = (to != रिक्त) ? to->dticks : (अंक_SOON_LISTS - 1);

	dticks = ((अंक_SOON_LISTS - 1 - wheel.soon_index + dticks) &
		  (अंक_SOON_LISTS - 1)) + 1;

	प्रत्यागम (k_ticks_t)dticks;
}

/* Backend-owned announce loop (see _TIMEOUT_BACKEND_OWNS_ANNOUNCE). Drains all
 * timeouts due within announce_remaining ticks, firing each handler with the
 * subsystem lock dropped; advances curr_tick and announce_remaining. Returns
 * the (possibly re-acquired) spinlock key.
 */
स्थिर k_spinlock_कुंजी_t z_timeout_q_announce(k_spinlock_कुंजी_t कुंजी)
{
	uint32_t mask;
	पूर्णाङ्क      next;    /* Index of next 'soon' list to expire */
	पूर्णाङ्क      अंक_ticks_to_advance;
	पूर्णाङ्क      correction;  /* NUM_SOON_LISTS or 0 */
	uint32_t bits;

	यावत् (announce_remaining > 0) {

		mask = 0xFFFFFFFF << ((wheel.soon_index + 1) & (अंक_SOON_LISTS - 1));
		bits = wheel.soon_bits | BIT(अंक_SOON_LISTS - 1);
		next = u32_गिनती_trailing_zeros(mask & bits);
		correction = ((wheel.soon_index + 1) / अंक_SOON_LISTS) * अंक_SOON_LISTS;
		अंक_ticks_to_advance = next + correction - wheel.soon_index;

		यदि (अंक_ticks_to_advance > announce_remaining) {
			curr_tick += announce_remaining;
			wheel.sifted = 0;
			wheel.soon_index += announce_remaining;
			wheel.soon_index &= (अंक_SOON_LISTS - 1);

			announce_remaining = 0;
			भङ्ग;
		}

		/*
		 * 1. Advance to the next tick to process.
		 * 2. Clear its bit in the 'soon_bits'.
		 */
		wheel.soon_index += अंक_ticks_to_advance;
		wheel.soon_index &= (अंक_SOON_LISTS - 1);
		wheel.soon_bits &= ~BIT(wheel.soon_index);

		curr_tick += अंक_ticks_to_advance;
		कुंजी = timeout_wheel_announce_now(कुंजी);
		announce_remaining -= अंक_ticks_to_advance;
	}

	प्रत्यागम कुंजी;
}

#endif /* ZEPHYR_KERNEL_TIMEOUT_WHEEL_H_ */
