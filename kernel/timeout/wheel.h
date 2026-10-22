/*
 * Copyright (c) 2018 Intel Corporation
 * Copyright (c) 2026 Peter Mitsis
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#నిర్వచితం_కాకపోతే ZEPHYR_KERNEL_TIMEOUT_WHEEL_H_
#నిర్వచించు ZEPHYR_KERNEL_TIMEOUT_WHEEL_H_

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

#అన్తర్భావ <zephyr/init.h>
#అన్తర్భావ <zephyr/sys/dlist.h>

/* Per-node flag bits (struct _timeout's flags field): which tier the timeout
 * currently lives in, so removal can maintain the soon-list occupancy bitmap.
 */
#నిర్వచించు _TIMEOUT_FLAG_TIMER_WHEEL_SOON  బిట్టు(0)
#నిర్వచించు _TIMEOUT_FLAG_TIMER_WHEEL_LATER బిట్టు(1)

/* This backend supplies its own announce loop. */
#నిర్వచించు _TIMEOUT_BACKEND_OWNS_ANNOUNCE 1

#నిర్వచించు అంకె_SOON_LISTS  32
#నిర్వచించు అంకె_LATER_LISTS 32

నిర్మాణం timeout_wheel {
	sys_dlist_t soon[అంకె_SOON_LISTS];   /* timeouts expiring soon lists */
	sys_dlist_t later[అంకె_LATER_LISTS]; /* timeouts expiring later lists */
	sys_dlist_t distant;                /* timeouts expiring far in future */
	sys_dlist_t soon_defer;             /* special soon list */
	uint32_t    soon_బిట్లు;              /* Set bit indicates list in use */
	uint32_t    soon_సూచీ;             /* Current soon list index */
	uint32_t    later_సూచీ;
	uint32_t    announcing;             /* 1: Announcing soon_index, else 0 */
	uint32_t    sifted;                 /* 1: Sifted, else 0 */
};

స్థిర నిర్మాణం timeout_wheel wheel;

స్థిర పూర్ణాంకం timeout_wheel_init(శూన్య)
{
	గుర్తులేని పూర్ణాంకం i;

	కొరకు (i = 0; i < అంకె_SOON_LISTS; i++) {
		sys_dlist_init(&wheel.soon[i]);
	}

	కొరకు (i = 0; i < అంకె_LATER_LISTS; i++) {
		sys_dlist_init(&wheel.later[i]);
	}

	sys_dlist_init(&wheel.distant);
	sys_dlist_init(&wheel.soon_defer);

	wheel.soon_బిట్లు = 0;
	wheel.soon_సూచీ = అంకె_SOON_LISTS - 1;
	wheel.later_సూచీ = 0;
	wheel.announcing = 0;
	wheel.sifted = 1;

	తిరిగి 0;
}

/* Initialize the wheel before the timer driver (PRE_KERNEL_2) and before any
 * timeout can be added. Priority 0 keeps it ahead of other PRE_KERNEL_1 init.
 */
SYS_INIT(timeout_wheel_init, PRE_KERNEL_1, 0);

స్థిర అంతఃవరుస శూన్య timeout_wheel_clear_flags(నిర్మాణం _timeout *గమ్యం)
{
	గమ్యం->flags &= ~(_TIMEOUT_FLAG_TIMER_WHEEL_SOON |
		       _TIMEOUT_FLAG_TIMER_WHEEL_LATER);
}

/* Return the last item in the specified list, or NULL if the list is empty. */
స్థిర నిర్మాణం _timeout *timeout_wheel_చివరి_లోపల_list(sys_dlist_t *list)
{
	sys_dnode_t *t = sys_dlist_peek_తోక(list);

	తిరిగి (t == ఖాళీ) ? ఖాళీ : CONTAINER_యొక్క(t, నిర్మాణం _timeout, కణుపు);
}

/* Return the first timeout expiring in the specified list, or NULL. */
స్థిర నిర్మాణం _timeout *timeout_wheel_మొదటి_లోపల_list(sys_dlist_t *list)
{
	sys_dnode_t *t = sys_dlist_peek_తల(list);

	తిరిగి (t == ఖాళీ) ? ఖాళీ : CONTAINER_యొక్క(t, నిర్మాణం _timeout, కణుపు);
}

/*
 * Get the first timeout in the wheel from its 'soon' lists. NULL means that
 * the first timeout is the sift operation which always implicitly exists
 * within the final 'soon' list.
 */
స్థిర నిర్మాణం _timeout *timeout_wheel_మొదటి(శూన్య)
{
	uint32_t bitmap;
	గుర్తులేని పూర్ణాంకం సూచీ;
	sys_dnode_t *t;

	bitmap = wheel.soon_బిట్లు | బిట్టు(అంకె_SOON_LISTS - 1);
	ఒకవేళ (wheel.sifted == 0) {
		bitmap &= ~(బిట్టు(wheel.soon_సూచీ) - 1);
	}
	సూచీ = u32_లెక్క_trailing_zeros(bitmap);

	t = sys_dlist_peek_తల(&wheel.soon[సూచీ]);

	తిరిగి (t == ఖాళీ) ? ఖాళీ : CONTAINER_యొక్క(t, నిర్మాణం _timeout, కణుపు);
}

/* Return the timeout following @a t expiring "distantly". */
స్థిర నిర్మాణం _timeout *timeout_wheel_తదుపరి_distant(నిర్మాణం _timeout *t)
{
	sys_dnode_t *n = sys_dlist_peek_తదుపరి(&wheel.distant, &t->కణుపు);

	తిరిగి (n == ఖాళీ) ? ఖాళీ : CONTAINER_యొక్క(n, నిర్మాణం _timeout, కణుపు);
}

/*
 * Prepend a timeout to the appropriate "soon" list. Only called during sifting
 * after the timeout had been removed from a 'later' list, where its 'dticks'
 * field was a composite value. Prepending preserves time ordering.
 */
స్థిర శూన్య timeout_wheel_prepend_soon(నిర్మాణం _timeout *గమ్యం)
{
	గుర్తులేని పూర్ణాంకం సూచీ = గమ్యం->dticks & (అంకె_SOON_LISTS - 1);

	wheel.soon_బిట్లు |= బిట్టు(సూచీ);
	sys_dlist_prepend(&wheel.soon[సూచీ], &గమ్యం->కణుపు);
	గమ్యం->flags |= _TIMEOUT_FLAG_TIMER_WHEEL_SOON;
	గమ్యం->dticks = సూచీ;
}

/*
 * Append a timeout to the appropriate "soon" list, recording the list index
 * in the timeout's 'dticks' field.
 */
స్థిర శూన్య timeout_wheel_append_soon(నిర్మాణం _timeout *గమ్యం)
{
	గుర్తులేని పూర్ణాంకం సూచీ;

	సూచీ = (గమ్యం->dticks + wheel.soon_సూచీ) & (అంకె_SOON_LISTS - 1);
	గమ్యం->dticks = సూచీ;

	wheel.soon_బిట్లు |= బిట్టు(సూచీ);
	sys_dlist_append(&wheel.soon[సూచీ], &గమ్యం->కణుపు);
	గమ్యం->flags |= _TIMEOUT_FLAG_TIMER_WHEEL_SOON;
}

/*
 * Append a timeout to the soon deferred list. Special case for timeouts that
 * are exactly NUM_SOON_LISTS into the future while the system is announcing:
 * they must not get mixed with the currently-expiring rotation. They move into
 * the appropriate 'soon' list at the end of the announce.
 */
స్థిర శూన్య timeout_wheel_append_soon_defer(నిర్మాణం _timeout *గమ్యం)
{
	గుర్తులేని పూర్ణాంకం సూచీ;

	సూచీ = (గమ్యం->dticks + wheel.soon_సూచీ) & (అంకె_SOON_LISTS - 1);
	గమ్యం->dticks = సూచీ + అంకె_SOON_LISTS;

	sys_dlist_append(&wheel.soon_defer, &గమ్యం->కణుపు);
	గమ్యం->flags |= _TIMEOUT_FLAG_TIMER_WHEEL_SOON;
}

/*
 * Append a timeout to the appropriate "later" list. The 'dticks' field is set
 * to identify both the eventual 'soon' list and the 'later' list it goes to.
 */
స్థిర శూన్య timeout_wheel_append_later(నిర్మాణం _timeout *గమ్యం)
{
	uint32_t position;
	uint32_t soon;
	uint32_t later;

	position = గమ్యం->dticks + wheel.soon_సూచీ - అంకె_SOON_LISTS;

	soon = position & (అంకె_SOON_LISTS - 1);
	later = ((position / అంకె_SOON_LISTS) + wheel.later_సూచీ -
		 wheel.sifted) & (అంకె_LATER_LISTS - 1);

	గమ్యం->dticks = (later * అంకె_SOON_LISTS) + soon;
	గమ్యం->flags |= _TIMEOUT_FLAG_TIMER_WHEEL_LATER;

	sys_dlist_append(&wheel.later[later], &గమ్యం->కణుపు);
}

/*
 * Insert a timeout into the "distant" list, an expiry-ordered delta list. For
 * good performance the distant list should stay short.
 */
స్థిర శూన్య timeout_wheel_చొప్పించు_distant(నిర్మాణం _timeout *గమ్యం, uint32_t cutoff)
{
	నిర్మాణం _timeout *t;

	గమ్యం->dticks -= cutoff;

	కొరకు (t = timeout_wheel_మొదటి_లోపల_list(&wheel.distant);
	     t != ఖాళీ; t = timeout_wheel_తదుపరి_distant(t)) {
		ఒకవేళ (t->dticks > గమ్యం->dticks) {
			t->dticks -= గమ్యం->dticks;
			sys_dlist_చొప్పించు(&t->కణుపు, &గమ్యం->కణుపు);
			ఆపు;
		}
		గమ్యం->dticks -= t->dticks;
	}

	ఒకవేళ (t == ఖాళీ) {
		sys_dlist_append(&wheel.distant, &గమ్యం->కణుపు);
	}
}

/*
 * Add a timeout to the wheel (with to->dticks already set to its tick delta
 * from curr_tick). It is never placed into the 'soon_index' list.
 */
స్థిర శూన్య timeout_wheel_కూడు_timeout(నిర్మాణం _timeout *గమ్యం)
{
	__ASSERT_NO_MSG(గమ్యం->dticks != 0);

	ఒకవేళ (గమ్యం->dticks <= (అంకె_SOON_LISTS - wheel.announcing)) {
		timeout_wheel_append_soon(గమ్యం);
		తిరిగి;
	}

	ఒకవేళ (గమ్యం->dticks == అంకె_SOON_LISTS) {
		timeout_wheel_append_soon_defer(గమ్యం);
		తిరిగి;
	}

	uint32_t cutoff;

	cutoff = ((అంకె_LATER_LISTS + wheel.sifted + 1) *
		  అంకె_SOON_LISTS) - wheel.soon_సూచీ - 1;
	ఒకవేళ (గమ్యం->dticks <= cutoff) {
		timeout_wheel_append_later(గమ్యం);
		తిరిగి;
	}

	timeout_wheel_చొప్పించు_distant(గమ్యం, cutoff);
}

/* Remove the specified timeout. */
స్థిర శూన్య timeout_wheel_remove_timeout(నిర్మాణం _timeout *గమ్యం)
{
	sys_dlist_remove(&గమ్యం->కణుపు);

	ఒకవేళ (గమ్యం->flags & _TIMEOUT_FLAG_TIMER_WHEEL_SOON) {
		గుర్తులేని పూర్ణాంకం సూచీ = గమ్యం->dticks;

		/* soon_defer entries carry index + NUM_SOON_LISTS and own no
		 * soon_bits bit; only true soon-list entries do.
		 */
		ఒకవేళ (సూచీ < అంకె_SOON_LISTS &&
		    sys_dlist_is_ఖాళీలు(&wheel.soon[సూచీ])) {
			wheel.soon_బిట్లు &= ~బిట్టు(సూచీ);
		}
	}

	timeout_wheel_clear_flags(గమ్యం);
}

/* Calculate the number of ticks until the specified timeout expires. */
స్థిర k_ticks_t timeout_wheel_remainder(స్థిరరాశి నిర్మాణం _timeout *గమ్యం)
{
	ఒకవేళ (గమ్యం->flags & _TIMEOUT_FLAG_TIMER_WHEEL_SOON) {
		uint32_t now = wheel.soon_సూచీ;
		uint32_t idx = గమ్యం->dticks;

		ఒకవేళ (idx == now) {
			తిరిగి (1U - wheel.announcing) * అంకె_SOON_LISTS;
		} లేకపోతే ఒకవేళ (idx >= అంకె_SOON_LISTS) {
			/* Special case if querying item in 'soon_defer' list */
			తిరిగి అంకె_SOON_LISTS;
		}

		తిరిగి (idx - now) & (అంకె_SOON_LISTS - 1U);
	}

	ఒకవేళ (గమ్యం->flags & _TIMEOUT_FLAG_TIMER_WHEEL_LATER) {
		uint32_t later_స్థానం = గమ్యం->dticks / అంకె_SOON_LISTS;
		uint32_t soon_సూచీ = గమ్యం->dticks & (అంకె_SOON_LISTS - 1U);

		/* later slots ahead until the bucket is sifted */
		uint32_t slots_ahead = ((later_స్థానం - wheel.later_సూచీ) &
					(అంకె_LATER_LISTS - 1U)) + wheel.sifted;

		తిరిగి (k_ticks_t)((అంకె_SOON_LISTS - wheel.soon_సూచీ) +
				   (slots_ahead * అంకె_SOON_LISTS) + soon_సూచీ);
	}

	నిర్మాణం _timeout *t;
	k_ticks_t మిగిలిన;

	మిగిలిన  = ((అంకె_LATER_LISTS + wheel.sifted + 1) *
		      అంకె_SOON_LISTS) - wheel.soon_సూచీ - 1;

	కొరకు (t = timeout_wheel_మొదటి_లోపల_list(&wheel.distant);
	     t != ఖాళీ; t = timeout_wheel_తదుపరి_distant(t)) {
		మిగిలిన += t->dticks;
		ఒకవేళ (t == గమ్యం) {
			ఆపు;
		}
	}

	తిరిగి మిగిలిన;
}

/*
 * Sift from a 'later' list down into the appropriate 'soon' lists. Timeouts
 * are prepended to the 'soon' list to preserve time ordering.
 */
స్థిర శూన్య timeout_wheel_జల్లించు_కిందికి_గమ్యం_soon(శూన్య)
{
	sys_dlist_t *list;
	నిర్మాణం _timeout *t;

	list = &wheel.later[wheel.later_సూచీ];
	అయితే ((t = timeout_wheel_చివరి_లోపల_list(list)) != ఖాళీ) {
		sys_dlist_remove(&t->కణుపు);
		timeout_wheel_clear_flags(t);
		timeout_wheel_prepend_soon(t);
	}
}

/*
 * Sift from the 'distant' list down into the appropriate 'later' list. Two
 * steps: prepare (massage dticks/flags of the leading distant timeouts) then
 * move them en masse into the 'later' list.
 */
స్థిర శూన్య timeout_wheel_జల్లించు_కిందికి_గమ్యం_later(శూన్య)
{
	నిర్మాణం _timeout *t;
	int64_t జల్లించు_ticks = అంకె_SOON_LISTS;
	నిర్మాణం _timeout *తల = timeout_wheel_మొదటి_లోపల_list(&wheel.distant);
	నిర్మాణం _timeout *తోక = ఖాళీ;
	పూర్ణాంకం new_dticks = wheel.later_సూచీ * అంకె_SOON_LISTS;

	కొరకు (t = తల; t != ఖాళీ; t = timeout_wheel_తదుపరి_distant(t)) {
		ఒకవేళ (t->dticks >= జల్లించు_ticks) {
			t->dticks -= జల్లించు_ticks;
			ఆపు;
		}

		జల్లించు_ticks -= t->dticks;
		new_dticks += t->dticks;

		t->flags |= _TIMEOUT_FLAG_TIMER_WHEEL_LATER;
		t->dticks = new_dticks;
		తోక = t;
	}

	ఒకవేళ (తోక != ఖాళీ) {
		sys_dlist_range_prepend(&wheel.later[wheel.later_సూచీ],
					&తల->కణుపు, &తోక->కణుపు);
	}
}

/*
 * Sifting runs every NUM_SOON_LISTS ticks: first move the current 'later' list
 * into the 'soon' lists, freeing it to receive the next batch sifted from
 * 'distant'.
 */
స్థిర శూన్య timeout_wheel_జల్లించు_కిందికి(శూన్య)
{
	timeout_wheel_జల్లించు_కిందికి_గమ్యం_soon();

	timeout_wheel_జల్లించు_కిందికి_గమ్యం_later();

	/* Advance to the next 'later' list */
	wheel.later_సూచీ++;
	wheel.later_సూచీ &= (అంకె_LATER_LISTS - 1);
	wheel.sifted = 1;
}

/*
 * Announce all the timeouts expiring "now" (at soon_index). curr_tick has been
 * advanced. Handlers fire with the lock dropped; inflight_timeout tracks
 * the in-flight timeout so aborts race correctly.
 */
స్థిర k_spinlock_కీలకం_t timeout_wheel_announce_now(k_spinlock_కీలకం_t కీలకం)
{
	నిర్మాణం _timeout *t;
	uint32_t సూచీ = wheel.soon_సూచీ;

	wheel.sifted = 0;
	wheel.announcing = 1;
	కొరకు (t = timeout_wheel_మొదటి_లోపల_list(&wheel.soon[సూచీ]);
	     t != ఖాళీ;
	     t = timeout_wheel_మొదటి_లోపల_list(&wheel.soon[సూచీ])) {
		sys_dlist_remove(&t->కణుపు);
		timeout_wheel_clear_flags(t);

		inflight_timeout = t;
		k_spin_unlock(&timeout_తాళం, కీలకం);
		t->fn(t);
		కీలకం = k_spin_తాళం(&timeout_తాళం);
		inflight_timeout = ఖాళీ;
	}
	wheel.announcing = 0;

	/*
	 * The 'soon_index' slot is now drained, so 'soon deferred' timeouts
	 * (exactly NUM_SOON_LISTS into the future) can move into their slots.
	 */
	కొరకు (t = timeout_wheel_మొదటి_లోపల_list(&wheel.soon_defer);
	     t != ఖాళీ;
	     t = timeout_wheel_మొదటి_లోపల_list(&wheel.soon_defer)) {
		sys_dlist_remove(&t->కణుపు);
		/* Correct the dticks to be the appropriate 'soon' list index */
		t->dticks &= (అంకె_SOON_LISTS - 1);
		wheel.soon_బిట్లు |= బిట్టు(t->dticks);
		sys_dlist_append(&wheel.soon[t->dticks], &t->కణుపు);
	}

	ఒకవేళ (సూచీ == (అంకె_SOON_LISTS - 1)) {
		timeout_wheel_జల్లించు_కిందికి();
	}

	తిరిగి కీలకం;
}

/*
 * Backend interface (see kernel/timeout.c).
 */

స్థిర bool z_timeout_q_చొప్పించు(నిర్మాణం _timeout *గమ్యం, k_ticks_t dticks)
{
	గమ్యం->dticks = dticks;
	timeout_wheel_కూడు_timeout(గమ్యం);

	తిరిగి గమ్యం == timeout_wheel_మొదటి();
}

స్థిర bool z_timeout_q_remove(నిర్మాణం _timeout *గమ్యం)
{
	bool was_మొదటి = (గమ్యం == timeout_wheel_మొదటి());

	timeout_wheel_remove_timeout(గమ్యం);

	తిరిగి was_మొదటి;
}

స్థిర k_ticks_t z_timeout_q_remainder(స్థిరరాశి నిర్మాణం _timeout *గమ్యం)
{
	తిరిగి timeout_wheel_remainder(గమ్యం);
}

/*
 * Ticks from curr_tick to the next 'soon' timeout, between 1 and
 * NUM_SOON_LISTS (the 'soon_index' list is excluded as it is timing out).
 * Never K_TICKS_FOREVER: a sift is always pending, which is why the wheel
 * wakes a tickless-idle CPU at least every NUM_SOON_LISTS ticks. The
 * announce-range cap is applied centrally by next_timeout() in timeout.c.
 */
స్థిర k_ticks_t z_timeout_q_తదుపరి_expiry(శూన్య)
{
	నిర్మాణం _timeout *గమ్యం = timeout_wheel_మొదటి();
	int64_t dticks = (గమ్యం != ఖాళీ) ? గమ్యం->dticks : (అంకె_SOON_LISTS - 1);

	dticks = ((అంకె_SOON_LISTS - 1 - wheel.soon_సూచీ + dticks) &
		  (అంకె_SOON_LISTS - 1)) + 1;

	తిరిగి (k_ticks_t)dticks;
}

/* Backend-owned announce loop (see _TIMEOUT_BACKEND_OWNS_ANNOUNCE). Drains all
 * timeouts due within announce_remaining ticks, firing each handler with the
 * subsystem lock dropped; advances curr_tick and announce_remaining. Returns
 * the (possibly re-acquired) spinlock key.
 */
స్థిర k_spinlock_కీలకం_t z_timeout_q_announce(k_spinlock_కీలకం_t కీలకం)
{
	uint32_t ముసుగు;
	పూర్ణాంకం      తదుపరి;    /* Index of next 'soon' list to expire */
	పూర్ణాంకం      అంకె_ticks_గమ్యం_advance;
	పూర్ణాంకం      correction;  /* NUM_SOON_LISTS or 0 */
	uint32_t బిట్లు;

	అయితే (announce_మిగిలిన > 0) {

		ముసుగు = 0xFFFFFFFF << ((wheel.soon_సూచీ + 1) & (అంకె_SOON_LISTS - 1));
		బిట్లు = wheel.soon_బిట్లు | బిట్టు(అంకె_SOON_LISTS - 1);
		తదుపరి = u32_లెక్క_trailing_zeros(ముసుగు & బిట్లు);
		correction = ((wheel.soon_సూచీ + 1) / అంకె_SOON_LISTS) * అంకె_SOON_LISTS;
		అంకె_ticks_గమ్యం_advance = తదుపరి + correction - wheel.soon_సూచీ;

		ఒకవేళ (అంకె_ticks_గమ్యం_advance > announce_మిగిలిన) {
			curr_tick += announce_మిగిలిన;
			wheel.sifted = 0;
			wheel.soon_సూచీ += announce_మిగిలిన;
			wheel.soon_సూచీ &= (అంకె_SOON_LISTS - 1);

			announce_మిగిలిన = 0;
			ఆపు;
		}

		/*
		 * 1. Advance to the next tick to process.
		 * 2. Clear its bit in the 'soon_bits'.
		 */
		wheel.soon_సూచీ += అంకె_ticks_గమ్యం_advance;
		wheel.soon_సూచీ &= (అంకె_SOON_LISTS - 1);
		wheel.soon_బిట్లు &= ~బిట్టు(wheel.soon_సూచీ);

		curr_tick += అంకె_ticks_గమ్యం_advance;
		కీలకం = timeout_wheel_announce_now(కీలకం);
		announce_మిగిలిన -= అంకె_ticks_గమ్యం_advance;
	}

	తిరిగి కీలకం;
}

#అయితే_ముగింపు /* ZEPHYR_KERNEL_TIMEOUT_WHEEL_H_ */
