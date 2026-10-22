/*
 * Copyright (c) 2018 Intel Corporation
 * Copyright (c) 2026 Peter Mitsis
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ ZEPHYR_KERNEL_TIMEOUT_WHEEL_H_
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ZEPHYR_KERNEL_TIMEOUT_WHEEL_H_

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

#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/init.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/sys/dlist.h>

/* Per-node flag bits (struct _timeout's flags field): which tier the timeout
 * currently lives in, so removal can maintain the soon-list occupancy bitmap.
 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ _TIMEOUT_FLAG_TIMER_WHEEL_SOON  ਦੋਆਧਾਰੀ_ਅੰਕ(0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ _TIMEOUT_FLAG_TIMER_WHEEL_LATER ਦੋਆਧਾਰੀ_ਅੰਕ(1)

/* This backend supplies its own announce loop. */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ _TIMEOUT_BACKEND_OWNS_ANNOUNCE 1

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਅੰਕ_SOON_LISTS  32
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਅੰਕ_LATER_LISTS 32

ਬਣਤਰ timeout_wheel {
	sys_dlist_t soon[ਅੰਕ_SOON_LISTS];   /* timeouts expiring soon lists */
	sys_dlist_t later[ਅੰਕ_LATER_LISTS]; /* timeouts expiring later lists */
	sys_dlist_t distant;                /* timeouts expiring far in future */
	sys_dlist_t soon_defer;             /* special soon list */
	uint32_t    soon_ਬਿੱਟ;              /* Set bit indicates list in use */
	uint32_t    soon_ਸੂਚਕਾਂਕ;             /* Current soon list index */
	uint32_t    later_ਸੂਚਕਾਂਕ;
	uint32_t    announcing;             /* 1: Announcing soon_index, else 0 */
	uint32_t    sifted;                 /* 1: Sifted, else 0 */
};

ਸਥਿਰ ਬਣਤਰ timeout_wheel wheel;

ਸਥਿਰ ਪੂਰਨ timeout_wheel_init(ਖਾਲੀ)
{
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ i;

	ਲਈ (i = 0; i < ਅੰਕ_SOON_LISTS; i++) {
		sys_dlist_init(&wheel.soon[i]);
	}

	ਲਈ (i = 0; i < ਅੰਕ_LATER_LISTS; i++) {
		sys_dlist_init(&wheel.later[i]);
	}

	sys_dlist_init(&wheel.distant);
	sys_dlist_init(&wheel.soon_defer);

	wheel.soon_ਬਿੱਟ = 0;
	wheel.soon_ਸੂਚਕਾਂਕ = ਅੰਕ_SOON_LISTS - 1;
	wheel.later_ਸੂਚਕਾਂਕ = 0;
	wheel.announcing = 0;
	wheel.sifted = 1;

	ਵਾਪਸ 0;
}

/* Initialize the wheel before the timer driver (PRE_KERNEL_2) and before any
 * timeout can be added. Priority 0 keeps it ahead of other PRE_KERNEL_1 init.
 */
SYS_INIT(timeout_wheel_init, PRE_KERNEL_1, 0);

ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ timeout_wheel_clear_flags(ਬਣਤਰ _timeout *ਮੰਜ਼ਿਲ)
{
	ਮੰਜ਼ਿਲ->flags &= ~(_TIMEOUT_FLAG_TIMER_WHEEL_SOON |
		       _TIMEOUT_FLAG_TIMER_WHEEL_LATER);
}

/* Return the last item in the specified list, or NULL if the list is empty. */
ਸਥਿਰ ਬਣਤਰ _timeout *timeout_wheel_ਆਖਰੀ_ਵਿੱਚ_list(sys_dlist_t *list)
{
	sys_dnode_t *t = sys_dlist_peek_ਪੂਛ(list);

	ਵਾਪਸ (t == ਨਿਰਾ) ? ਨਿਰਾ : CONTAINER_ਦਾ(t, ਬਣਤਰ _timeout, ਗੰਢ);
}

/* Return the first timeout expiring in the specified list, or NULL. */
ਸਥਿਰ ਬਣਤਰ _timeout *timeout_wheel_ਪਹਿਲਾ_ਵਿੱਚ_list(sys_dlist_t *list)
{
	sys_dnode_t *t = sys_dlist_peek_ਸਿਰ(list);

	ਵਾਪਸ (t == ਨਿਰਾ) ? ਨਿਰਾ : CONTAINER_ਦਾ(t, ਬਣਤਰ _timeout, ਗੰਢ);
}

/*
 * Get the first timeout in the wheel from its 'soon' lists. NULL means that
 * the first timeout is the sift operation which always implicitly exists
 * within the final 'soon' list.
 */
ਸਥਿਰ ਬਣਤਰ _timeout *timeout_wheel_ਪਹਿਲਾ(ਖਾਲੀ)
{
	uint32_t bitmap;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਸੂਚਕਾਂਕ;
	sys_dnode_t *t;

	bitmap = wheel.soon_ਬਿੱਟ | ਦੋਆਧਾਰੀ_ਅੰਕ(ਅੰਕ_SOON_LISTS - 1);
	ਜੇ (wheel.sifted == 0) {
		bitmap &= ~(ਦੋਆਧਾਰੀ_ਅੰਕ(wheel.soon_ਸੂਚਕਾਂਕ) - 1);
	}
	ਸੂਚਕਾਂਕ = u32_ਗਿਣਤੀ_trailing_zeros(bitmap);

	t = sys_dlist_peek_ਸਿਰ(&wheel.soon[ਸੂਚਕਾਂਕ]);

	ਵਾਪਸ (t == ਨਿਰਾ) ? ਨਿਰਾ : CONTAINER_ਦਾ(t, ਬਣਤਰ _timeout, ਗੰਢ);
}

/* Return the timeout following @a t expiring "distantly". */
ਸਥਿਰ ਬਣਤਰ _timeout *timeout_wheel_ਅਗਲਾ_distant(ਬਣਤਰ _timeout *t)
{
	sys_dnode_t *n = sys_dlist_peek_ਅਗਲਾ(&wheel.distant, &t->ਗੰਢ);

	ਵਾਪਸ (n == ਨਿਰਾ) ? ਨਿਰਾ : CONTAINER_ਦਾ(n, ਬਣਤਰ _timeout, ਗੰਢ);
}

/*
 * Prepend a timeout to the appropriate "soon" list. Only called during sifting
 * after the timeout had been removed from a 'later' list, where its 'dticks'
 * field was a composite value. Prepending preserves time ordering.
 */
ਸਥਿਰ ਖਾਲੀ timeout_wheel_prepend_soon(ਬਣਤਰ _timeout *ਮੰਜ਼ਿਲ)
{
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਸੂਚਕਾਂਕ = ਮੰਜ਼ਿਲ->dticks & (ਅੰਕ_SOON_LISTS - 1);

	wheel.soon_ਬਿੱਟ |= ਦੋਆਧਾਰੀ_ਅੰਕ(ਸੂਚਕਾਂਕ);
	sys_dlist_prepend(&wheel.soon[ਸੂਚਕਾਂਕ], &ਮੰਜ਼ਿਲ->ਗੰਢ);
	ਮੰਜ਼ਿਲ->flags |= _TIMEOUT_FLAG_TIMER_WHEEL_SOON;
	ਮੰਜ਼ਿਲ->dticks = ਸੂਚਕਾਂਕ;
}

/*
 * Append a timeout to the appropriate "soon" list, recording the list index
 * in the timeout's 'dticks' field.
 */
ਸਥਿਰ ਖਾਲੀ timeout_wheel_append_soon(ਬਣਤਰ _timeout *ਮੰਜ਼ਿਲ)
{
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਸੂਚਕਾਂਕ;

	ਸੂਚਕਾਂਕ = (ਮੰਜ਼ਿਲ->dticks + wheel.soon_ਸੂਚਕਾਂਕ) & (ਅੰਕ_SOON_LISTS - 1);
	ਮੰਜ਼ਿਲ->dticks = ਸੂਚਕਾਂਕ;

	wheel.soon_ਬਿੱਟ |= ਦੋਆਧਾਰੀ_ਅੰਕ(ਸੂਚਕਾਂਕ);
	sys_dlist_append(&wheel.soon[ਸੂਚਕਾਂਕ], &ਮੰਜ਼ਿਲ->ਗੰਢ);
	ਮੰਜ਼ਿਲ->flags |= _TIMEOUT_FLAG_TIMER_WHEEL_SOON;
}

/*
 * Append a timeout to the soon deferred list. Special case for timeouts that
 * are exactly NUM_SOON_LISTS into the future while the system is announcing:
 * they must not get mixed with the currently-expiring rotation. They move into
 * the appropriate 'soon' list at the end of the announce.
 */
ਸਥਿਰ ਖਾਲੀ timeout_wheel_append_soon_defer(ਬਣਤਰ _timeout *ਮੰਜ਼ਿਲ)
{
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਸੂਚਕਾਂਕ;

	ਸੂਚਕਾਂਕ = (ਮੰਜ਼ਿਲ->dticks + wheel.soon_ਸੂਚਕਾਂਕ) & (ਅੰਕ_SOON_LISTS - 1);
	ਮੰਜ਼ਿਲ->dticks = ਸੂਚਕਾਂਕ + ਅੰਕ_SOON_LISTS;

	sys_dlist_append(&wheel.soon_defer, &ਮੰਜ਼ਿਲ->ਗੰਢ);
	ਮੰਜ਼ਿਲ->flags |= _TIMEOUT_FLAG_TIMER_WHEEL_SOON;
}

/*
 * Append a timeout to the appropriate "later" list. The 'dticks' field is set
 * to identify both the eventual 'soon' list and the 'later' list it goes to.
 */
ਸਥਿਰ ਖਾਲੀ timeout_wheel_append_later(ਬਣਤਰ _timeout *ਮੰਜ਼ਿਲ)
{
	uint32_t position;
	uint32_t soon;
	uint32_t later;

	position = ਮੰਜ਼ਿਲ->dticks + wheel.soon_ਸੂਚਕਾਂਕ - ਅੰਕ_SOON_LISTS;

	soon = position & (ਅੰਕ_SOON_LISTS - 1);
	later = ((position / ਅੰਕ_SOON_LISTS) + wheel.later_ਸੂਚਕਾਂਕ -
		 wheel.sifted) & (ਅੰਕ_LATER_LISTS - 1);

	ਮੰਜ਼ਿਲ->dticks = (later * ਅੰਕ_SOON_LISTS) + soon;
	ਮੰਜ਼ਿਲ->flags |= _TIMEOUT_FLAG_TIMER_WHEEL_LATER;

	sys_dlist_append(&wheel.later[later], &ਮੰਜ਼ਿਲ->ਗੰਢ);
}

/*
 * Insert a timeout into the "distant" list, an expiry-ordered delta list. For
 * good performance the distant list should stay short.
 */
ਸਥਿਰ ਖਾਲੀ timeout_wheel_ਪਾਓ_distant(ਬਣਤਰ _timeout *ਮੰਜ਼ਿਲ, uint32_t cutoff)
{
	ਬਣਤਰ _timeout *t;

	ਮੰਜ਼ਿਲ->dticks -= cutoff;

	ਲਈ (t = timeout_wheel_ਪਹਿਲਾ_ਵਿੱਚ_list(&wheel.distant);
	     t != ਨਿਰਾ; t = timeout_wheel_ਅਗਲਾ_distant(t)) {
		ਜੇ (t->dticks > ਮੰਜ਼ਿਲ->dticks) {
			t->dticks -= ਮੰਜ਼ਿਲ->dticks;
			sys_dlist_ਪਾਓ(&t->ਗੰਢ, &ਮੰਜ਼ਿਲ->ਗੰਢ);
			ਰੋਕੋ;
		}
		ਮੰਜ਼ਿਲ->dticks -= t->dticks;
	}

	ਜੇ (t == ਨਿਰਾ) {
		sys_dlist_append(&wheel.distant, &ਮੰਜ਼ਿਲ->ਗੰਢ);
	}
}

/*
 * Add a timeout to the wheel (with to->dticks already set to its tick delta
 * from curr_tick). It is never placed into the 'soon_index' list.
 */
ਸਥਿਰ ਖਾਲੀ timeout_wheel_ਜੋੜੋ_timeout(ਬਣਤਰ _timeout *ਮੰਜ਼ਿਲ)
{
	__ASSERT_NO_MSG(ਮੰਜ਼ਿਲ->dticks != 0);

	ਜੇ (ਮੰਜ਼ਿਲ->dticks <= (ਅੰਕ_SOON_LISTS - wheel.announcing)) {
		timeout_wheel_append_soon(ਮੰਜ਼ਿਲ);
		ਵਾਪਸ;
	}

	ਜੇ (ਮੰਜ਼ਿਲ->dticks == ਅੰਕ_SOON_LISTS) {
		timeout_wheel_append_soon_defer(ਮੰਜ਼ਿਲ);
		ਵਾਪਸ;
	}

	uint32_t cutoff;

	cutoff = ((ਅੰਕ_LATER_LISTS + wheel.sifted + 1) *
		  ਅੰਕ_SOON_LISTS) - wheel.soon_ਸੂਚਕਾਂਕ - 1;
	ਜੇ (ਮੰਜ਼ਿਲ->dticks <= cutoff) {
		timeout_wheel_append_later(ਮੰਜ਼ਿਲ);
		ਵਾਪਸ;
	}

	timeout_wheel_ਪਾਓ_distant(ਮੰਜ਼ਿਲ, cutoff);
}

/* Remove the specified timeout. */
ਸਥਿਰ ਖਾਲੀ timeout_wheel_remove_timeout(ਬਣਤਰ _timeout *ਮੰਜ਼ਿਲ)
{
	sys_dlist_remove(&ਮੰਜ਼ਿਲ->ਗੰਢ);

	ਜੇ (ਮੰਜ਼ਿਲ->flags & _TIMEOUT_FLAG_TIMER_WHEEL_SOON) {
		ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਸੂਚਕਾਂਕ = ਮੰਜ਼ਿਲ->dticks;

		/* soon_defer entries carry index + NUM_SOON_LISTS and own no
		 * soon_bits bit; only true soon-list entries do.
		 */
		ਜੇ (ਸੂਚਕਾਂਕ < ਅੰਕ_SOON_LISTS &&
		    sys_dlist_is_ਖਾਲੀ_ਥਾਵਾਂ(&wheel.soon[ਸੂਚਕਾਂਕ])) {
			wheel.soon_ਬਿੱਟ &= ~ਦੋਆਧਾਰੀ_ਅੰਕ(ਸੂਚਕਾਂਕ);
		}
	}

	timeout_wheel_clear_flags(ਮੰਜ਼ਿਲ);
}

/* Calculate the number of ticks until the specified timeout expires. */
ਸਥਿਰ k_ticks_t timeout_wheel_remainder(ਅਚਲ ਬਣਤਰ _timeout *ਮੰਜ਼ਿਲ)
{
	ਜੇ (ਮੰਜ਼ਿਲ->flags & _TIMEOUT_FLAG_TIMER_WHEEL_SOON) {
		uint32_t now = wheel.soon_ਸੂਚਕਾਂਕ;
		uint32_t idx = ਮੰਜ਼ਿਲ->dticks;

		ਜੇ (idx == now) {
			ਵਾਪਸ (1U - wheel.announcing) * ਅੰਕ_SOON_LISTS;
		} ਨਹੀਂਤਾਂ ਜੇ (idx >= ਅੰਕ_SOON_LISTS) {
			/* Special case if querying item in 'soon_defer' list */
			ਵਾਪਸ ਅੰਕ_SOON_LISTS;
		}

		ਵਾਪਸ (idx - now) & (ਅੰਕ_SOON_LISTS - 1U);
	}

	ਜੇ (ਮੰਜ਼ਿਲ->flags & _TIMEOUT_FLAG_TIMER_WHEEL_LATER) {
		uint32_t later_ਥਾਂ = ਮੰਜ਼ਿਲ->dticks / ਅੰਕ_SOON_LISTS;
		uint32_t soon_ਸੂਚਕਾਂਕ = ਮੰਜ਼ਿਲ->dticks & (ਅੰਕ_SOON_LISTS - 1U);

		/* later slots ahead until the bucket is sifted */
		uint32_t slots_ahead = ((later_ਥਾਂ - wheel.later_ਸੂਚਕਾਂਕ) &
					(ਅੰਕ_LATER_LISTS - 1U)) + wheel.sifted;

		ਵਾਪਸ (k_ticks_t)((ਅੰਕ_SOON_LISTS - wheel.soon_ਸੂਚਕਾਂਕ) +
				   (slots_ahead * ਅੰਕ_SOON_LISTS) + soon_ਸੂਚਕਾਂਕ);
	}

	ਬਣਤਰ _timeout *t;
	k_ticks_t ਬਚਿਆ;

	ਬਚਿਆ  = ((ਅੰਕ_LATER_LISTS + wheel.sifted + 1) *
		      ਅੰਕ_SOON_LISTS) - wheel.soon_ਸੂਚਕਾਂਕ - 1;

	ਲਈ (t = timeout_wheel_ਪਹਿਲਾ_ਵਿੱਚ_list(&wheel.distant);
	     t != ਨਿਰਾ; t = timeout_wheel_ਅਗਲਾ_distant(t)) {
		ਬਚਿਆ += t->dticks;
		ਜੇ (t == ਮੰਜ਼ਿਲ) {
			ਰੋਕੋ;
		}
	}

	ਵਾਪਸ ਬਚਿਆ;
}

/*
 * Sift from a 'later' list down into the appropriate 'soon' lists. Timeouts
 * are prepended to the 'soon' list to preserve time ordering.
 */
ਸਥਿਰ ਖਾਲੀ timeout_wheel_ਛਾਣੋ_ਥੱਲੇ_ਮੰਜ਼ਿਲ_soon(ਖਾਲੀ)
{
	sys_dlist_t *list;
	ਬਣਤਰ _timeout *t;

	list = &wheel.later[wheel.later_ਸੂਚਕਾਂਕ];
	ਜਦੋਂ ((t = timeout_wheel_ਆਖਰੀ_ਵਿੱਚ_list(list)) != ਨਿਰਾ) {
		sys_dlist_remove(&t->ਗੰਢ);
		timeout_wheel_clear_flags(t);
		timeout_wheel_prepend_soon(t);
	}
}

/*
 * Sift from the 'distant' list down into the appropriate 'later' list. Two
 * steps: prepare (massage dticks/flags of the leading distant timeouts) then
 * move them en masse into the 'later' list.
 */
ਸਥਿਰ ਖਾਲੀ timeout_wheel_ਛਾਣੋ_ਥੱਲੇ_ਮੰਜ਼ਿਲ_later(ਖਾਲੀ)
{
	ਬਣਤਰ _timeout *t;
	int64_t ਛਾਣੋ_ticks = ਅੰਕ_SOON_LISTS;
	ਬਣਤਰ _timeout *ਸਿਰ = timeout_wheel_ਪਹਿਲਾ_ਵਿੱਚ_list(&wheel.distant);
	ਬਣਤਰ _timeout *ਪੂਛ = ਨਿਰਾ;
	ਪੂਰਨ new_dticks = wheel.later_ਸੂਚਕਾਂਕ * ਅੰਕ_SOON_LISTS;

	ਲਈ (t = ਸਿਰ; t != ਨਿਰਾ; t = timeout_wheel_ਅਗਲਾ_distant(t)) {
		ਜੇ (t->dticks >= ਛਾਣੋ_ticks) {
			t->dticks -= ਛਾਣੋ_ticks;
			ਰੋਕੋ;
		}

		ਛਾਣੋ_ticks -= t->dticks;
		new_dticks += t->dticks;

		t->flags |= _TIMEOUT_FLAG_TIMER_WHEEL_LATER;
		t->dticks = new_dticks;
		ਪੂਛ = t;
	}

	ਜੇ (ਪੂਛ != ਨਿਰਾ) {
		sys_dlist_range_prepend(&wheel.later[wheel.later_ਸੂਚਕਾਂਕ],
					&ਸਿਰ->ਗੰਢ, &ਪੂਛ->ਗੰਢ);
	}
}

/*
 * Sifting runs every NUM_SOON_LISTS ticks: first move the current 'later' list
 * into the 'soon' lists, freeing it to receive the next batch sifted from
 * 'distant'.
 */
ਸਥਿਰ ਖਾਲੀ timeout_wheel_ਛਾਣੋ_ਥੱਲੇ(ਖਾਲੀ)
{
	timeout_wheel_ਛਾਣੋ_ਥੱਲੇ_ਮੰਜ਼ਿਲ_soon();

	timeout_wheel_ਛਾਣੋ_ਥੱਲੇ_ਮੰਜ਼ਿਲ_later();

	/* Advance to the next 'later' list */
	wheel.later_ਸੂਚਕਾਂਕ++;
	wheel.later_ਸੂਚਕਾਂਕ &= (ਅੰਕ_LATER_LISTS - 1);
	wheel.sifted = 1;
}

/*
 * Announce all the timeouts expiring "now" (at soon_index). curr_tick has been
 * advanced. Handlers fire with the lock dropped; inflight_timeout tracks
 * the in-flight timeout so aborts race correctly.
 */
ਸਥਿਰ k_spinlock_ਕੁੰਜੀ_t timeout_wheel_announce_now(k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ)
{
	ਬਣਤਰ _timeout *t;
	uint32_t ਸੂਚਕਾਂਕ = wheel.soon_ਸੂਚਕਾਂਕ;

	wheel.sifted = 0;
	wheel.announcing = 1;
	ਲਈ (t = timeout_wheel_ਪਹਿਲਾ_ਵਿੱਚ_list(&wheel.soon[ਸੂਚਕਾਂਕ]);
	     t != ਨਿਰਾ;
	     t = timeout_wheel_ਪਹਿਲਾ_ਵਿੱਚ_list(&wheel.soon[ਸੂਚਕਾਂਕ])) {
		sys_dlist_remove(&t->ਗੰਢ);
		timeout_wheel_clear_flags(t);

		inflight_timeout = t;
		k_spin_unlock(&timeout_ਤਾਲਾ, ਕੁੰਜੀ);
		t->fn(t);
		ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&timeout_ਤਾਲਾ);
		inflight_timeout = ਨਿਰਾ;
	}
	wheel.announcing = 0;

	/*
	 * The 'soon_index' slot is now drained, so 'soon deferred' timeouts
	 * (exactly NUM_SOON_LISTS into the future) can move into their slots.
	 */
	ਲਈ (t = timeout_wheel_ਪਹਿਲਾ_ਵਿੱਚ_list(&wheel.soon_defer);
	     t != ਨਿਰਾ;
	     t = timeout_wheel_ਪਹਿਲਾ_ਵਿੱਚ_list(&wheel.soon_defer)) {
		sys_dlist_remove(&t->ਗੰਢ);
		/* Correct the dticks to be the appropriate 'soon' list index */
		t->dticks &= (ਅੰਕ_SOON_LISTS - 1);
		wheel.soon_ਬਿੱਟ |= ਦੋਆਧਾਰੀ_ਅੰਕ(t->dticks);
		sys_dlist_append(&wheel.soon[t->dticks], &t->ਗੰਢ);
	}

	ਜੇ (ਸੂਚਕਾਂਕ == (ਅੰਕ_SOON_LISTS - 1)) {
		timeout_wheel_ਛਾਣੋ_ਥੱਲੇ();
	}

	ਵਾਪਸ ਕੁੰਜੀ;
}

/*
 * Backend interface (see kernel/timeout.c).
 */

ਸਥਿਰ bool z_timeout_q_ਪਾਓ(ਬਣਤਰ _timeout *ਮੰਜ਼ਿਲ, k_ticks_t dticks)
{
	ਮੰਜ਼ਿਲ->dticks = dticks;
	timeout_wheel_ਜੋੜੋ_timeout(ਮੰਜ਼ਿਲ);

	ਵਾਪਸ ਮੰਜ਼ਿਲ == timeout_wheel_ਪਹਿਲਾ();
}

ਸਥਿਰ bool z_timeout_q_remove(ਬਣਤਰ _timeout *ਮੰਜ਼ਿਲ)
{
	bool was_ਪਹਿਲਾ = (ਮੰਜ਼ਿਲ == timeout_wheel_ਪਹਿਲਾ());

	timeout_wheel_remove_timeout(ਮੰਜ਼ਿਲ);

	ਵਾਪਸ was_ਪਹਿਲਾ;
}

ਸਥਿਰ k_ticks_t z_timeout_q_remainder(ਅਚਲ ਬਣਤਰ _timeout *ਮੰਜ਼ਿਲ)
{
	ਵਾਪਸ timeout_wheel_remainder(ਮੰਜ਼ਿਲ);
}

/*
 * Ticks from curr_tick to the next 'soon' timeout, between 1 and
 * NUM_SOON_LISTS (the 'soon_index' list is excluded as it is timing out).
 * Never K_TICKS_FOREVER: a sift is always pending, which is why the wheel
 * wakes a tickless-idle CPU at least every NUM_SOON_LISTS ticks. The
 * announce-range cap is applied centrally by next_timeout() in timeout.c.
 */
ਸਥਿਰ k_ticks_t z_timeout_q_ਅਗਲਾ_expiry(ਖਾਲੀ)
{
	ਬਣਤਰ _timeout *ਮੰਜ਼ਿਲ = timeout_wheel_ਪਹਿਲਾ();
	int64_t dticks = (ਮੰਜ਼ਿਲ != ਨਿਰਾ) ? ਮੰਜ਼ਿਲ->dticks : (ਅੰਕ_SOON_LISTS - 1);

	dticks = ((ਅੰਕ_SOON_LISTS - 1 - wheel.soon_ਸੂਚਕਾਂਕ + dticks) &
		  (ਅੰਕ_SOON_LISTS - 1)) + 1;

	ਵਾਪਸ (k_ticks_t)dticks;
}

/* Backend-owned announce loop (see _TIMEOUT_BACKEND_OWNS_ANNOUNCE). Drains all
 * timeouts due within announce_remaining ticks, firing each handler with the
 * subsystem lock dropped; advances curr_tick and announce_remaining. Returns
 * the (possibly re-acquired) spinlock key.
 */
ਸਥਿਰ k_spinlock_ਕੁੰਜੀ_t z_timeout_q_announce(k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ)
{
	uint32_t ਨਕਾਬ;
	ਪੂਰਨ      ਅਗਲਾ;    /* Index of next 'soon' list to expire */
	ਪੂਰਨ      ਅੰਕ_ticks_ਮੰਜ਼ਿਲ_advance;
	ਪੂਰਨ      correction;  /* NUM_SOON_LISTS or 0 */
	uint32_t ਬਿੱਟ;

	ਜਦੋਂ (announce_ਬਚਿਆ > 0) {

		ਨਕਾਬ = 0xFFFFFFFF << ((wheel.soon_ਸੂਚਕਾਂਕ + 1) & (ਅੰਕ_SOON_LISTS - 1));
		ਬਿੱਟ = wheel.soon_ਬਿੱਟ | ਦੋਆਧਾਰੀ_ਅੰਕ(ਅੰਕ_SOON_LISTS - 1);
		ਅਗਲਾ = u32_ਗਿਣਤੀ_trailing_zeros(ਨਕਾਬ & ਬਿੱਟ);
		correction = ((wheel.soon_ਸੂਚਕਾਂਕ + 1) / ਅੰਕ_SOON_LISTS) * ਅੰਕ_SOON_LISTS;
		ਅੰਕ_ticks_ਮੰਜ਼ਿਲ_advance = ਅਗਲਾ + correction - wheel.soon_ਸੂਚਕਾਂਕ;

		ਜੇ (ਅੰਕ_ticks_ਮੰਜ਼ਿਲ_advance > announce_ਬਚਿਆ) {
			curr_tick += announce_ਬਚਿਆ;
			wheel.sifted = 0;
			wheel.soon_ਸੂਚਕਾਂਕ += announce_ਬਚਿਆ;
			wheel.soon_ਸੂਚਕਾਂਕ &= (ਅੰਕ_SOON_LISTS - 1);

			announce_ਬਚਿਆ = 0;
			ਰੋਕੋ;
		}

		/*
		 * 1. Advance to the next tick to process.
		 * 2. Clear its bit in the 'soon_bits'.
		 */
		wheel.soon_ਸੂਚਕਾਂਕ += ਅੰਕ_ticks_ਮੰਜ਼ਿਲ_advance;
		wheel.soon_ਸੂਚਕਾਂਕ &= (ਅੰਕ_SOON_LISTS - 1);
		wheel.soon_ਬਿੱਟ &= ~ਦੋਆਧਾਰੀ_ਅੰਕ(wheel.soon_ਸੂਚਕਾਂਕ);

		curr_tick += ਅੰਕ_ticks_ਮੰਜ਼ਿਲ_advance;
		ਕੁੰਜੀ = timeout_wheel_announce_now(ਕੁੰਜੀ);
		announce_ਬਚਿਆ -= ਅੰਕ_ticks_ਮੰਜ਼ਿਲ_advance;
	}

	ਵਾਪਸ ਕੁੰਜੀ;
}

#ਜੇ_ਅੰਤ /* ZEPHYR_KERNEL_TIMEOUT_WHEEL_H_ */
