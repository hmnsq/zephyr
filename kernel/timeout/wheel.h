/*
 * Copyright (c) 2018 Intel Corporation
 * Copyright (c) 2026 Peter Mitsis
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#यदि_अपरिभाषितम् ZEPHYR_KERNEL_TIMEOUT_WHEEL_H_
#परिभाषय ZEPHYR_KERNEL_TIMEOUT_WHEEL_H_

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
#परिभाषय _TIMEOUT_FLAG_TIMER_WHEEL_SOON  BIT(0)
#परिभाषय _TIMEOUT_FLAG_TIMER_WHEEL_LATER BIT(1)

/* This backend supplies its own announce loop. */
#परिभाषय _TIMEOUT_BACKEND_OWNS_ANNOUNCE 1

#परिभाषय अङ्कः_SOON_LISTS  32
#परिभाषय अङ्कः_LATER_LISTS 32

संरचना timeout_wheel {
	sys_dlist_t soon[अङ्कः_SOON_LISTS];   /* timeouts expiring soon lists */
	sys_dlist_t later[अङ्कः_LATER_LISTS]; /* timeouts expiring later lists */
	sys_dlist_t distant;                /* timeouts expiring far in future */
	sys_dlist_t soon_defer;             /* special soon list */
	uint32_t    soon_द्वयङ्काः;              /* Set bit indicates list in use */
	uint32_t    soon_सूचकांक;             /* Current soon list index */
	uint32_t    later_सूचकांक;
	uint32_t    announcing;             /* 1: Announcing soon_index, else 0 */
	uint32_t    sifted;                 /* 1: Sifted, else 0 */
};

स्थिर संरचना timeout_wheel wheel;

स्थिर पूर्णाङ्क timeout_wheel_init(शून्य)
{
	अचिह्नित पूर्णाङ्क i;

	प्रति (i = 0; i < अङ्कः_SOON_LISTS; i++) {
		sys_dlist_init(&wheel.soon[i]);
	}

	प्रति (i = 0; i < अङ्कः_LATER_LISTS; i++) {
		sys_dlist_init(&wheel.later[i]);
	}

	sys_dlist_init(&wheel.distant);
	sys_dlist_init(&wheel.soon_defer);

	wheel.soon_द्वयङ्काः = 0;
	wheel.soon_सूचकांक = अङ्कः_SOON_LISTS - 1;
	wheel.later_सूचकांक = 0;
	wheel.announcing = 0;
	wheel.sifted = 1;

	प्रत्यागम 0;
}

/* Initialize the wheel before the timer driver (PRE_KERNEL_2) and before any
 * timeout can be added. Priority 0 keeps it ahead of other PRE_KERNEL_1 init.
 */
SYS_INIT(timeout_wheel_init, PRE_KERNEL_1, 0);

स्थिर अन्तःस्थ शून्य timeout_wheel_clear_flags(संरचना _timeout *गन्तव्यम्)
{
	गन्तव्यम्->flags &= ~(_TIMEOUT_FLAG_TIMER_WHEEL_SOON |
		       _TIMEOUT_FLAG_TIMER_WHEEL_LATER);
}

/* Return the last item in the specified list, or NULL if the list is empty. */
स्थिर संरचना _timeout *timeout_wheel_चरमः_अन्तर्_list(sys_dlist_t *list)
{
	sys_dnode_t *t = sys_dlist_peek_पुच्छ(list);

	प्रत्यागम (t == रिक्त) ? रिक्त : CONTAINER_सम्बन्धि(t, संरचना _timeout, ग्रन्थिः);
}

/* Return the first timeout expiring in the specified list, or NULL. */
स्थिर संरचना _timeout *timeout_wheel_first_अन्तर्_list(sys_dlist_t *list)
{
	sys_dnode_t *t = sys_dlist_peek_शीर्ष(list);

	प्रत्यागम (t == रिक्त) ? रिक्त : CONTAINER_सम्बन्धि(t, संरचना _timeout, ग्रन्थिः);
}

/*
 * Get the first timeout in the wheel from its 'soon' lists. NULL means that
 * the first timeout is the sift operation which always implicitly exists
 * within the final 'soon' list.
 */
स्थिर संरचना _timeout *timeout_wheel_first(शून्य)
{
	uint32_t bitmap;
	अचिह्नित पूर्णाङ्क सूचकांक;
	sys_dnode_t *t;

	bitmap = wheel.soon_द्वयङ्काः | BIT(अङ्कः_SOON_LISTS - 1);
	यदि (wheel.sifted == 0) {
		bitmap &= ~(BIT(wheel.soon_सूचकांक) - 1);
	}
	सूचकांक = u32_गणनम्_trailing_zeros(bitmap);

	t = sys_dlist_peek_शीर्ष(&wheel.soon[सूचकांक]);

	प्रत्यागम (t == रिक्त) ? रिक्त : CONTAINER_सम्बन्धि(t, संरचना _timeout, ग्रन्थिः);
}

/* Return the timeout following @a t expiring "distantly". */
स्थिर संरचना _timeout *timeout_wheel_अग्र_distant(संरचना _timeout *t)
{
	sys_dnode_t *n = sys_dlist_peek_अग्र(&wheel.distant, &t->ग्रन्थिः);

	प्रत्यागम (n == रिक्त) ? रिक्त : CONTAINER_सम्बन्धि(n, संरचना _timeout, ग्रन्थिः);
}

/*
 * Prepend a timeout to the appropriate "soon" list. Only called during sifting
 * after the timeout had been removed from a 'later' list, where its 'dticks'
 * field was a composite value. Prepending preserves time ordering.
 */
स्थिर शून्य timeout_wheel_prepend_soon(संरचना _timeout *गन्तव्यम्)
{
	अचिह्नित पूर्णाङ्क सूचकांक = गन्तव्यम्->dticks & (अङ्कः_SOON_LISTS - 1);

	wheel.soon_द्वयङ्काः |= BIT(सूचकांक);
	sys_dlist_prepend(&wheel.soon[सूचकांक], &गन्तव्यम्->ग्रन्थिः);
	गन्तव्यम्->flags |= _TIMEOUT_FLAG_TIMER_WHEEL_SOON;
	गन्तव्यम्->dticks = सूचकांक;
}

/*
 * Append a timeout to the appropriate "soon" list, recording the list index
 * in the timeout's 'dticks' field.
 */
स्थिर शून्य timeout_wheel_append_soon(संरचना _timeout *गन्तव्यम्)
{
	अचिह्नित पूर्णाङ्क सूचकांक;

	सूचकांक = (गन्तव्यम्->dticks + wheel.soon_सूचकांक) & (अङ्कः_SOON_LISTS - 1);
	गन्तव्यम्->dticks = सूचकांक;

	wheel.soon_द्वयङ्काः |= BIT(सूचकांक);
	sys_dlist_append(&wheel.soon[सूचकांक], &गन्तव्यम्->ग्रन्थिः);
	गन्तव्यम्->flags |= _TIMEOUT_FLAG_TIMER_WHEEL_SOON;
}

/*
 * Append a timeout to the soon deferred list. Special case for timeouts that
 * are exactly NUM_SOON_LISTS into the future while the system is announcing:
 * they must not get mixed with the currently-expiring rotation. They move into
 * the appropriate 'soon' list at the end of the announce.
 */
स्थिर शून्य timeout_wheel_append_soon_defer(संरचना _timeout *गन्तव्यम्)
{
	अचिह्नित पूर्णाङ्क सूचकांक;

	सूचकांक = (गन्तव्यम्->dticks + wheel.soon_सूचकांक) & (अङ्कः_SOON_LISTS - 1);
	गन्तव्यम्->dticks = सूचकांक + अङ्कः_SOON_LISTS;

	sys_dlist_append(&wheel.soon_defer, &गन्तव्यम्->ग्रन्थिः);
	गन्तव्यम्->flags |= _TIMEOUT_FLAG_TIMER_WHEEL_SOON;
}

/*
 * Append a timeout to the appropriate "later" list. The 'dticks' field is set
 * to identify both the eventual 'soon' list and the 'later' list it goes to.
 */
स्थिर शून्य timeout_wheel_append_later(संरचना _timeout *गन्तव्यम्)
{
	uint32_t position;
	uint32_t soon;
	uint32_t later;

	position = गन्तव्यम्->dticks + wheel.soon_सूचकांक - अङ्कः_SOON_LISTS;

	soon = position & (अङ्कः_SOON_LISTS - 1);
	later = ((position / अङ्कः_SOON_LISTS) + wheel.later_सूचकांक -
		 wheel.sifted) & (अङ्कः_LATER_LISTS - 1);

	गन्तव्यम्->dticks = (later * अङ्कः_SOON_LISTS) + soon;
	गन्तव्यम्->flags |= _TIMEOUT_FLAG_TIMER_WHEEL_LATER;

	sys_dlist_append(&wheel.later[later], &गन्तव्यम्->ग्रन्थिः);
}

/*
 * Insert a timeout into the "distant" list, an expiry-ordered delta list. For
 * good performance the distant list should stay short.
 */
स्थिर शून्य timeout_wheel_निवेशय_distant(संरचना _timeout *गन्तव्यम्, uint32_t cutoff)
{
	संरचना _timeout *t;

	गन्तव्यम्->dticks -= cutoff;

	प्रति (t = timeout_wheel_first_अन्तर्_list(&wheel.distant);
	     t != रिक्त; t = timeout_wheel_अग्र_distant(t)) {
		यदि (t->dticks > गन्तव्यम्->dticks) {
			t->dticks -= गन्तव्यम्->dticks;
			sys_dlist_निवेशय(&t->ग्रन्थिः, &गन्तव्यम्->ग्रन्थिः);
			भङ्ग;
		}
		गन्तव्यम्->dticks -= t->dticks;
	}

	यदि (t == रिक्त) {
		sys_dlist_append(&wheel.distant, &गन्तव्यम्->ग्रन्थिः);
	}
}

/*
 * Add a timeout to the wheel (with to->dticks already set to its tick delta
 * from curr_tick). It is never placed into the 'soon_index' list.
 */
स्थिर शून्य timeout_wheel_सङ्कलय_timeout(संरचना _timeout *गन्तव्यम्)
{
	__ASSERT_NO_MSG(गन्तव्यम्->dticks != 0);

	यदि (गन्तव्यम्->dticks <= (अङ्कः_SOON_LISTS - wheel.announcing)) {
		timeout_wheel_append_soon(गन्तव्यम्);
		प्रत्यागम;
	}

	यदि (गन्तव्यम्->dticks == अङ्कः_SOON_LISTS) {
		timeout_wheel_append_soon_defer(गन्तव्यम्);
		प्रत्यागम;
	}

	uint32_t cutoff;

	cutoff = ((अङ्कः_LATER_LISTS + wheel.sifted + 1) *
		  अङ्कः_SOON_LISTS) - wheel.soon_सूचकांक - 1;
	यदि (गन्तव्यम्->dticks <= cutoff) {
		timeout_wheel_append_later(गन्तव्यम्);
		प्रत्यागम;
	}

	timeout_wheel_निवेशय_distant(गन्तव्यम्, cutoff);
}

/* Remove the specified timeout. */
स्थिर शून्य timeout_wheel_remove_timeout(संरचना _timeout *गन्तव्यम्)
{
	sys_dlist_remove(&गन्तव्यम्->ग्रन्थिः);

	यदि (गन्तव्यम्->flags & _TIMEOUT_FLAG_TIMER_WHEEL_SOON) {
		अचिह्नित पूर्णाङ्क सूचकांक = गन्तव्यम्->dticks;

		/* soon_defer entries carry index + NUM_SOON_LISTS and own no
		 * soon_bits bit; only true soon-list entries do.
		 */
		यदि (सूचकांक < अङ्कः_SOON_LISTS &&
		    sys_dlist_is_रिक्तानि(&wheel.soon[सूचकांक])) {
			wheel.soon_द्वयङ्काः &= ~BIT(सूचकांक);
		}
	}

	timeout_wheel_clear_flags(गन्तव्यम्);
}

/* Calculate the number of ticks until the specified timeout expires. */
स्थिर k_ticks_t timeout_wheel_remainder(ध्रुव संरचना _timeout *गन्तव्यम्)
{
	यदि (गन्तव्यम्->flags & _TIMEOUT_FLAG_TIMER_WHEEL_SOON) {
		uint32_t now = wheel.soon_सूचकांक;
		uint32_t idx = गन्तव्यम्->dticks;

		यदि (idx == now) {
			प्रत्यागम (1U - wheel.announcing) * अङ्कः_SOON_LISTS;
		} अन्यथा यदि (idx >= अङ्कः_SOON_LISTS) {
			/* Special case if querying item in 'soon_defer' list */
			प्रत्यागम अङ्कः_SOON_LISTS;
		}

		प्रत्यागम (idx - now) & (अङ्कः_SOON_LISTS - 1U);
	}

	यदि (गन्तव्यम्->flags & _TIMEOUT_FLAG_TIMER_WHEEL_LATER) {
		uint32_t later_slot = गन्तव्यम्->dticks / अङ्कः_SOON_LISTS;
		uint32_t soon_सूचकांक = गन्तव्यम्->dticks & (अङ्कः_SOON_LISTS - 1U);

		/* later slots ahead until the bucket is sifted */
		uint32_t slots_ahead = ((later_slot - wheel.later_सूचकांक) &
					(अङ्कः_LATER_LISTS - 1U)) + wheel.sifted;

		प्रत्यागम (k_ticks_t)((अङ्कः_SOON_LISTS - wheel.soon_सूचकांक) +
				   (slots_ahead * अङ्कः_SOON_LISTS) + soon_सूचकांक);
	}

	संरचना _timeout *t;
	k_ticks_t remaining;

	remaining  = ((अङ्कः_LATER_LISTS + wheel.sifted + 1) *
		      अङ्कः_SOON_LISTS) - wheel.soon_सूचकांक - 1;

	प्रति (t = timeout_wheel_first_अन्तर्_list(&wheel.distant);
	     t != रिक्त; t = timeout_wheel_अग्र_distant(t)) {
		remaining += t->dticks;
		यदि (t == गन्तव्यम्) {
			भङ्ग;
		}
	}

	प्रत्यागम remaining;
}

/*
 * Sift from a 'later' list down into the appropriate 'soon' lists. Timeouts
 * are prepended to the 'soon' list to preserve time ordering.
 */
स्थिर शून्य timeout_wheel_शोधय_अधस्तात्_गन्तव्यम्_soon(शून्य)
{
	sys_dlist_t *list;
	संरचना _timeout *t;

	list = &wheel.later[wheel.later_सूचकांक];
	यावत् ((t = timeout_wheel_चरमः_अन्तर्_list(list)) != रिक्त) {
		sys_dlist_remove(&t->ग्रन्थिः);
		timeout_wheel_clear_flags(t);
		timeout_wheel_prepend_soon(t);
	}
}

/*
 * Sift from the 'distant' list down into the appropriate 'later' list. Two
 * steps: prepare (massage dticks/flags of the leading distant timeouts) then
 * move them en masse into the 'later' list.
 */
स्थिर शून्य timeout_wheel_शोधय_अधस्तात्_गन्तव्यम्_later(शून्य)
{
	संरचना _timeout *t;
	int64_t शोधय_ticks = अङ्कः_SOON_LISTS;
	संरचना _timeout *शीर्ष = timeout_wheel_first_अन्तर्_list(&wheel.distant);
	संरचना _timeout *पुच्छ = रिक्त;
	पूर्णाङ्क new_dticks = wheel.later_सूचकांक * अङ्कः_SOON_LISTS;

	प्रति (t = शीर्ष; t != रिक्त; t = timeout_wheel_अग्र_distant(t)) {
		यदि (t->dticks >= शोधय_ticks) {
			t->dticks -= शोधय_ticks;
			भङ्ग;
		}

		शोधय_ticks -= t->dticks;
		new_dticks += t->dticks;

		t->flags |= _TIMEOUT_FLAG_TIMER_WHEEL_LATER;
		t->dticks = new_dticks;
		पुच्छ = t;
	}

	यदि (पुच्छ != रिक्त) {
		sys_dlist_range_prepend(&wheel.later[wheel.later_सूचकांक],
					&शीर्ष->ग्रन्थिः, &पुच्छ->ग्रन्थिः);
	}
}

/*
 * Sifting runs every NUM_SOON_LISTS ticks: first move the current 'later' list
 * into the 'soon' lists, freeing it to receive the next batch sifted from
 * 'distant'.
 */
स्थिर शून्य timeout_wheel_शोधय_अधस्तात्(शून्य)
{
	timeout_wheel_शोधय_अधस्तात्_गन्तव्यम्_soon();

	timeout_wheel_शोधय_अधस्तात्_गन्तव्यम्_later();

	/* Advance to the next 'later' list */
	wheel.later_सूचकांक++;
	wheel.later_सूचकांक &= (अङ्कः_LATER_LISTS - 1);
	wheel.sifted = 1;
}

/*
 * Announce all the timeouts expiring "now" (at soon_index). curr_tick has been
 * advanced. Handlers fire with the lock dropped; inflight_timeout tracks
 * the in-flight timeout so aborts race correctly.
 */
स्थिर k_spinlock_कुञ्चिका_t timeout_wheel_announce_now(k_spinlock_कुञ्चिका_t कुञ्चिका)
{
	संरचना _timeout *t;
	uint32_t सूचकांक = wheel.soon_सूचकांक;

	wheel.sifted = 0;
	wheel.announcing = 1;
	प्रति (t = timeout_wheel_first_अन्तर्_list(&wheel.soon[सूचकांक]);
	     t != रिक्त;
	     t = timeout_wheel_first_अन्तर्_list(&wheel.soon[सूचकांक])) {
		sys_dlist_remove(&t->ग्रन्थिः);
		timeout_wheel_clear_flags(t);

		inflight_timeout = t;
		k_spin_unlock(&timeout_तालकम्, कुञ्चिका);
		t->fn(t);
		कुञ्चिका = k_spin_तालकम्(&timeout_तालकम्);
		inflight_timeout = रिक्त;
	}
	wheel.announcing = 0;

	/*
	 * The 'soon_index' slot is now drained, so 'soon deferred' timeouts
	 * (exactly NUM_SOON_LISTS into the future) can move into their slots.
	 */
	प्रति (t = timeout_wheel_first_अन्तर्_list(&wheel.soon_defer);
	     t != रिक्त;
	     t = timeout_wheel_first_अन्तर्_list(&wheel.soon_defer)) {
		sys_dlist_remove(&t->ग्रन्थिः);
		/* Correct the dticks to be the appropriate 'soon' list index */
		t->dticks &= (अङ्कः_SOON_LISTS - 1);
		wheel.soon_द्वयङ्काः |= BIT(t->dticks);
		sys_dlist_append(&wheel.soon[t->dticks], &t->ग्रन्थिः);
	}

	यदि (सूचकांक == (अङ्कः_SOON_LISTS - 1)) {
		timeout_wheel_शोधय_अधस्तात्();
	}

	प्रत्यागम कुञ्चिका;
}

/*
 * Backend interface (see kernel/timeout.c).
 */

स्थिर bool z_timeout_q_निवेशय(संरचना _timeout *गन्तव्यम्, k_ticks_t dticks)
{
	गन्तव्यम्->dticks = dticks;
	timeout_wheel_सङ्कलय_timeout(गन्तव्यम्);

	प्रत्यागम गन्तव्यम् == timeout_wheel_first();
}

स्थिर bool z_timeout_q_remove(संरचना _timeout *गन्तव्यम्)
{
	bool was_first = (गन्तव्यम् == timeout_wheel_first());

	timeout_wheel_remove_timeout(गन्तव्यम्);

	प्रत्यागम was_first;
}

स्थिर k_ticks_t z_timeout_q_remainder(ध्रुव संरचना _timeout *गन्तव्यम्)
{
	प्रत्यागम timeout_wheel_remainder(गन्तव्यम्);
}

/*
 * Ticks from curr_tick to the next 'soon' timeout, between 1 and
 * NUM_SOON_LISTS (the 'soon_index' list is excluded as it is timing out).
 * Never K_TICKS_FOREVER: a sift is always pending, which is why the wheel
 * wakes a tickless-idle CPU at least every NUM_SOON_LISTS ticks. The
 * announce-range cap is applied centrally by next_timeout() in timeout.c.
 */
स्थिर k_ticks_t z_timeout_q_अग्र_expiry(शून्य)
{
	संरचना _timeout *गन्तव्यम् = timeout_wheel_first();
	int64_t dticks = (गन्तव्यम् != रिक्त) ? गन्तव्यम्->dticks : (अङ्कः_SOON_LISTS - 1);

	dticks = ((अङ्कः_SOON_LISTS - 1 - wheel.soon_सूचकांक + dticks) &
		  (अङ्कः_SOON_LISTS - 1)) + 1;

	प्रत्यागम (k_ticks_t)dticks;
}

/* Backend-owned announce loop (see _TIMEOUT_BACKEND_OWNS_ANNOUNCE). Drains all
 * timeouts due within announce_remaining ticks, firing each handler with the
 * subsystem lock dropped; advances curr_tick and announce_remaining. Returns
 * the (possibly re-acquired) spinlock key.
 */
स्थिर k_spinlock_कुञ्चिका_t z_timeout_q_announce(k_spinlock_कुञ्चिका_t कुञ्चिका)
{
	uint32_t आवरणम्;
	पूर्णाङ्क      अग्र;    /* Index of next 'soon' list to expire */
	पूर्णाङ्क      अङ्कः_ticks_गन्तव्यम्_advance;
	पूर्णाङ्क      correction;  /* NUM_SOON_LISTS or 0 */
	uint32_t द्वयङ्काः;

	यावत् (announce_remaining > 0) {

		आवरणम् = 0xFFFFFFFF << ((wheel.soon_सूचकांक + 1) & (अङ्कः_SOON_LISTS - 1));
		द्वयङ्काः = wheel.soon_द्वयङ्काः | BIT(अङ्कः_SOON_LISTS - 1);
		अग्र = u32_गणनम्_trailing_zeros(आवरणम् & द्वयङ्काः);
		correction = ((wheel.soon_सूचकांक + 1) / अङ्कः_SOON_LISTS) * अङ्कः_SOON_LISTS;
		अङ्कः_ticks_गन्तव्यम्_advance = अग्र + correction - wheel.soon_सूचकांक;

		यदि (अङ्कः_ticks_गन्तव्यम्_advance > announce_remaining) {
			curr_tick += announce_remaining;
			wheel.sifted = 0;
			wheel.soon_सूचकांक += announce_remaining;
			wheel.soon_सूचकांक &= (अङ्कः_SOON_LISTS - 1);

			announce_remaining = 0;
			भङ्ग;
		}

		/*
		 * 1. Advance to the next tick to process.
		 * 2. Clear its bit in the 'soon_bits'.
		 */
		wheel.soon_सूचकांक += अङ्कः_ticks_गन्तव्यम्_advance;
		wheel.soon_सूचकांक &= (अङ्कः_SOON_LISTS - 1);
		wheel.soon_द्वयङ्काः &= ~BIT(wheel.soon_सूचकांक);

		curr_tick += अङ्कः_ticks_गन्तव्यम्_advance;
		कुञ्चिका = timeout_wheel_announce_now(कुञ्चिका);
		announce_remaining -= अङ्कः_ticks_गन्तव्यम्_advance;
	}

	प्रत्यागम कुञ्चिका;
}

#यदि_अन्तः /* ZEPHYR_KERNEL_TIMEOUT_WHEEL_H_ */
