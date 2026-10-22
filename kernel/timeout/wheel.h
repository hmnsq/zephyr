/*
 * Copyright (c) 2018 Intel Corporation
 * Copyright (c) 2026 Peter Mitsis
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ ZEPHYR_KERNEL_TIMEOUT_WHEEL_H_
#ವ್ಯಾಖ್ಯಾನಿಸು ZEPHYR_KERNEL_TIMEOUT_WHEEL_H_

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

#ಅನ್ತರ್ಭಾವ <zephyr/init.h>
#ಅನ್ತರ್ಭಾವ <zephyr/sys/dlist.h>

/* Per-node flag bits (struct _timeout's flags field): which tier the timeout
 * currently lives in, so removal can maintain the soon-list occupancy bitmap.
 */
#ವ್ಯಾಖ್ಯಾನಿಸು _TIMEOUT_FLAG_TIMER_WHEEL_SOON  ಬಿಟ್(0)
#ವ್ಯಾಖ್ಯಾನಿಸು _TIMEOUT_FLAG_TIMER_WHEEL_LATER ಬಿಟ್(1)

/* This backend supplies its own announce loop. */
#ವ್ಯಾಖ್ಯಾನಿಸು _TIMEOUT_BACKEND_OWNS_ANNOUNCE 1

#ವ್ಯಾಖ್ಯಾನಿಸು ಅಂಕೆ_SOON_LISTS  32
#ವ್ಯಾಖ್ಯಾನಿಸು ಅಂಕೆ_LATER_LISTS 32

ರಚನೆ timeout_wheel {
	sys_dlist_t soon[ಅಂಕೆ_SOON_LISTS];   /* timeouts expiring soon lists */
	sys_dlist_t later[ಅಂಕೆ_LATER_LISTS]; /* timeouts expiring later lists */
	sys_dlist_t distant;                /* timeouts expiring far in future */
	sys_dlist_t soon_defer;             /* special soon list */
	uint32_t    soon_ಬಿಟ್ಟುಗಳು;              /* Set bit indicates list in use */
	uint32_t    soon_ಸೂಚ್ಯಂಕ;             /* Current soon list index */
	uint32_t    later_ಸೂಚ್ಯಂಕ;
	uint32_t    announcing;             /* 1: Announcing soon_index, else 0 */
	uint32_t    sifted;                 /* 1: Sifted, else 0 */
};

ಸ್ಥಿರ ರಚನೆ timeout_wheel wheel;

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ timeout_wheel_init(ಶೂನ್ಯ)
{
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ i;

	ಗಾಗಿ (i = 0; i < ಅಂಕೆ_SOON_LISTS; i++) {
		sys_dlist_init(&wheel.soon[i]);
	}

	ಗಾಗಿ (i = 0; i < ಅಂಕೆ_LATER_LISTS; i++) {
		sys_dlist_init(&wheel.later[i]);
	}

	sys_dlist_init(&wheel.distant);
	sys_dlist_init(&wheel.soon_defer);

	wheel.soon_ಬಿಟ್ಟುಗಳು = 0;
	wheel.soon_ಸೂಚ್ಯಂಕ = ಅಂಕೆ_SOON_LISTS - 1;
	wheel.later_ಸೂಚ್ಯಂಕ = 0;
	wheel.announcing = 0;
	wheel.sifted = 1;

	ಹಿಂತಿರುಗಿ 0;
}

/* Initialize the wheel before the timer driver (PRE_KERNEL_2) and before any
 * timeout can be added. Priority 0 keeps it ahead of other PRE_KERNEL_1 init.
 */
SYS_INIT(timeout_wheel_init, PRE_KERNEL_1, 0);

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ timeout_wheel_clear_flags(ರಚನೆ _timeout *ಗಮ್ಯ)
{
	ಗಮ್ಯ->flags &= ~(_TIMEOUT_FLAG_TIMER_WHEEL_SOON |
		       _TIMEOUT_FLAG_TIMER_WHEEL_LATER);
}

/* Return the last item in the specified list, or NULL if the list is empty. */
ಸ್ಥಿರ ರಚನೆ _timeout *timeout_wheel_ಕೊನೆಯ_ಒಳ_list(sys_dlist_t *list)
{
	sys_dnode_t *t = sys_dlist_peek_ಬಾಲ(list);

	ಹಿಂತಿರುಗಿ (t == ಖಾಲಿ) ? ಖಾಲಿ : CONTAINER_ದ(t, ರಚನೆ _timeout, ಗಂಟು);
}

/* Return the first timeout expiring in the specified list, or NULL. */
ಸ್ಥಿರ ರಚನೆ _timeout *timeout_wheel_ಮೊದಲ_ಒಳ_list(sys_dlist_t *list)
{
	sys_dnode_t *t = sys_dlist_peek_ತಲೆ(list);

	ಹಿಂತಿರುಗಿ (t == ಖಾಲಿ) ? ಖಾಲಿ : CONTAINER_ದ(t, ರಚನೆ _timeout, ಗಂಟು);
}

/*
 * Get the first timeout in the wheel from its 'soon' lists. NULL means that
 * the first timeout is the sift operation which always implicitly exists
 * within the final 'soon' list.
 */
ಸ್ಥಿರ ರಚನೆ _timeout *timeout_wheel_ಮೊದಲ(ಶೂನ್ಯ)
{
	uint32_t bitmap;
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಸೂಚ್ಯಂಕ;
	sys_dnode_t *t;

	bitmap = wheel.soon_ಬಿಟ್ಟುಗಳು | ಬಿಟ್(ಅಂಕೆ_SOON_LISTS - 1);
	ಆದರೆ (wheel.sifted == 0) {
		bitmap &= ~(ಬಿಟ್(wheel.soon_ಸೂಚ್ಯಂಕ) - 1);
	}
	ಸೂಚ್ಯಂಕ = u32_ಎಣಿಕೆ_trailing_zeros(bitmap);

	t = sys_dlist_peek_ತಲೆ(&wheel.soon[ಸೂಚ್ಯಂಕ]);

	ಹಿಂತಿರುಗಿ (t == ಖಾಲಿ) ? ಖಾಲಿ : CONTAINER_ದ(t, ರಚನೆ _timeout, ಗಂಟು);
}

/* Return the timeout following @a t expiring "distantly". */
ಸ್ಥಿರ ರಚನೆ _timeout *timeout_wheel_ಮುಂದಿನ_distant(ರಚನೆ _timeout *t)
{
	sys_dnode_t *n = sys_dlist_peek_ಮುಂದಿನ(&wheel.distant, &t->ಗಂಟು);

	ಹಿಂತಿರುಗಿ (n == ಖಾಲಿ) ? ಖಾಲಿ : CONTAINER_ದ(n, ರಚನೆ _timeout, ಗಂಟು);
}

/*
 * Prepend a timeout to the appropriate "soon" list. Only called during sifting
 * after the timeout had been removed from a 'later' list, where its 'dticks'
 * field was a composite value. Prepending preserves time ordering.
 */
ಸ್ಥಿರ ಶೂನ್ಯ timeout_wheel_prepend_soon(ರಚನೆ _timeout *ಗಮ್ಯ)
{
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಸೂಚ್ಯಂಕ = ಗಮ್ಯ->dticks & (ಅಂಕೆ_SOON_LISTS - 1);

	wheel.soon_ಬಿಟ್ಟುಗಳು |= ಬಿಟ್(ಸೂಚ್ಯಂಕ);
	sys_dlist_prepend(&wheel.soon[ಸೂಚ್ಯಂಕ], &ಗಮ್ಯ->ಗಂಟು);
	ಗಮ್ಯ->flags |= _TIMEOUT_FLAG_TIMER_WHEEL_SOON;
	ಗಮ್ಯ->dticks = ಸೂಚ್ಯಂಕ;
}

/*
 * Append a timeout to the appropriate "soon" list, recording the list index
 * in the timeout's 'dticks' field.
 */
ಸ್ಥಿರ ಶೂನ್ಯ timeout_wheel_append_soon(ರಚನೆ _timeout *ಗಮ್ಯ)
{
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಸೂಚ್ಯಂಕ;

	ಸೂಚ್ಯಂಕ = (ಗಮ್ಯ->dticks + wheel.soon_ಸೂಚ್ಯಂಕ) & (ಅಂಕೆ_SOON_LISTS - 1);
	ಗಮ್ಯ->dticks = ಸೂಚ್ಯಂಕ;

	wheel.soon_ಬಿಟ್ಟುಗಳು |= ಬಿಟ್(ಸೂಚ್ಯಂಕ);
	sys_dlist_append(&wheel.soon[ಸೂಚ್ಯಂಕ], &ಗಮ್ಯ->ಗಂಟು);
	ಗಮ್ಯ->flags |= _TIMEOUT_FLAG_TIMER_WHEEL_SOON;
}

/*
 * Append a timeout to the soon deferred list. Special case for timeouts that
 * are exactly NUM_SOON_LISTS into the future while the system is announcing:
 * they must not get mixed with the currently-expiring rotation. They move into
 * the appropriate 'soon' list at the end of the announce.
 */
ಸ್ಥಿರ ಶೂನ್ಯ timeout_wheel_append_soon_defer(ರಚನೆ _timeout *ಗಮ್ಯ)
{
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಸೂಚ್ಯಂಕ;

	ಸೂಚ್ಯಂಕ = (ಗಮ್ಯ->dticks + wheel.soon_ಸೂಚ್ಯಂಕ) & (ಅಂಕೆ_SOON_LISTS - 1);
	ಗಮ್ಯ->dticks = ಸೂಚ್ಯಂಕ + ಅಂಕೆ_SOON_LISTS;

	sys_dlist_append(&wheel.soon_defer, &ಗಮ್ಯ->ಗಂಟು);
	ಗಮ್ಯ->flags |= _TIMEOUT_FLAG_TIMER_WHEEL_SOON;
}

/*
 * Append a timeout to the appropriate "later" list. The 'dticks' field is set
 * to identify both the eventual 'soon' list and the 'later' list it goes to.
 */
ಸ್ಥಿರ ಶೂನ್ಯ timeout_wheel_append_later(ರಚನೆ _timeout *ಗಮ್ಯ)
{
	uint32_t position;
	uint32_t soon;
	uint32_t later;

	position = ಗಮ್ಯ->dticks + wheel.soon_ಸೂಚ್ಯಂಕ - ಅಂಕೆ_SOON_LISTS;

	soon = position & (ಅಂಕೆ_SOON_LISTS - 1);
	later = ((position / ಅಂಕೆ_SOON_LISTS) + wheel.later_ಸೂಚ್ಯಂಕ -
		 wheel.sifted) & (ಅಂಕೆ_LATER_LISTS - 1);

	ಗಮ್ಯ->dticks = (later * ಅಂಕೆ_SOON_LISTS) + soon;
	ಗಮ್ಯ->flags |= _TIMEOUT_FLAG_TIMER_WHEEL_LATER;

	sys_dlist_append(&wheel.later[later], &ಗಮ್ಯ->ಗಂಟು);
}

/*
 * Insert a timeout into the "distant" list, an expiry-ordered delta list. For
 * good performance the distant list should stay short.
 */
ಸ್ಥಿರ ಶೂನ್ಯ timeout_wheel_ಸೇರಿಸು_distant(ರಚನೆ _timeout *ಗಮ್ಯ, uint32_t cutoff)
{
	ರಚನೆ _timeout *t;

	ಗಮ್ಯ->dticks -= cutoff;

	ಗಾಗಿ (t = timeout_wheel_ಮೊದಲ_ಒಳ_list(&wheel.distant);
	     t != ಖಾಲಿ; t = timeout_wheel_ಮುಂದಿನ_distant(t)) {
		ಆದರೆ (t->dticks > ಗಮ್ಯ->dticks) {
			t->dticks -= ಗಮ್ಯ->dticks;
			sys_dlist_ಸೇರಿಸು(&t->ಗಂಟು, &ಗಮ್ಯ->ಗಂಟು);
			ನಿಲ್ಲಿಸು;
		}
		ಗಮ್ಯ->dticks -= t->dticks;
	}

	ಆದರೆ (t == ಖಾಲಿ) {
		sys_dlist_append(&wheel.distant, &ಗಮ್ಯ->ಗಂಟು);
	}
}

/*
 * Add a timeout to the wheel (with to->dticks already set to its tick delta
 * from curr_tick). It is never placed into the 'soon_index' list.
 */
ಸ್ಥಿರ ಶೂನ್ಯ timeout_wheel_ಕೂಡಿಸು_timeout(ರಚನೆ _timeout *ಗಮ್ಯ)
{
	__ASSERT_NO_MSG(ಗಮ್ಯ->dticks != 0);

	ಆದರೆ (ಗಮ್ಯ->dticks <= (ಅಂಕೆ_SOON_LISTS - wheel.announcing)) {
		timeout_wheel_append_soon(ಗಮ್ಯ);
		ಹಿಂತಿರುಗಿ;
	}

	ಆದರೆ (ಗಮ್ಯ->dticks == ಅಂಕೆ_SOON_LISTS) {
		timeout_wheel_append_soon_defer(ಗಮ್ಯ);
		ಹಿಂತಿರುಗಿ;
	}

	uint32_t cutoff;

	cutoff = ((ಅಂಕೆ_LATER_LISTS + wheel.sifted + 1) *
		  ಅಂಕೆ_SOON_LISTS) - wheel.soon_ಸೂಚ್ಯಂಕ - 1;
	ಆದರೆ (ಗಮ್ಯ->dticks <= cutoff) {
		timeout_wheel_append_later(ಗಮ್ಯ);
		ಹಿಂತಿರುಗಿ;
	}

	timeout_wheel_ಸೇರಿಸು_distant(ಗಮ್ಯ, cutoff);
}

/* Remove the specified timeout. */
ಸ್ಥಿರ ಶೂನ್ಯ timeout_wheel_remove_timeout(ರಚನೆ _timeout *ಗಮ್ಯ)
{
	sys_dlist_remove(&ಗಮ್ಯ->ಗಂಟು);

	ಆದರೆ (ಗಮ್ಯ->flags & _TIMEOUT_FLAG_TIMER_WHEEL_SOON) {
		ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಸೂಚ್ಯಂಕ = ಗಮ್ಯ->dticks;

		/* soon_defer entries carry index + NUM_SOON_LISTS and own no
		 * soon_bits bit; only true soon-list entries do.
		 */
		ಆದರೆ (ಸೂಚ್ಯಂಕ < ಅಂಕೆ_SOON_LISTS &&
		    sys_dlist_is_ಖಾಲಿಗಳು(&wheel.soon[ಸೂಚ್ಯಂಕ])) {
			wheel.soon_ಬಿಟ್ಟುಗಳು &= ~ಬಿಟ್(ಸೂಚ್ಯಂಕ);
		}
	}

	timeout_wheel_clear_flags(ಗಮ್ಯ);
}

/* Calculate the number of ticks until the specified timeout expires. */
ಸ್ಥಿರ k_ticks_t timeout_wheel_remainder(ಸ್ಥಿರಾಂಕ ರಚನೆ _timeout *ಗಮ್ಯ)
{
	ಆದರೆ (ಗಮ್ಯ->flags & _TIMEOUT_FLAG_TIMER_WHEEL_SOON) {
		uint32_t now = wheel.soon_ಸೂಚ್ಯಂಕ;
		uint32_t idx = ಗಮ್ಯ->dticks;

		ಆದರೆ (idx == now) {
			ಹಿಂತಿರುಗಿ (1U - wheel.announcing) * ಅಂಕೆ_SOON_LISTS;
		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (idx >= ಅಂಕೆ_SOON_LISTS) {
			/* Special case if querying item in 'soon_defer' list */
			ಹಿಂತಿರುಗಿ ಅಂಕೆ_SOON_LISTS;
		}

		ಹಿಂತಿರುಗಿ (idx - now) & (ಅಂಕೆ_SOON_LISTS - 1U);
	}

	ಆದರೆ (ಗಮ್ಯ->flags & _TIMEOUT_FLAG_TIMER_WHEEL_LATER) {
		uint32_t later_ಸ್ಥಾನ = ಗಮ್ಯ->dticks / ಅಂಕೆ_SOON_LISTS;
		uint32_t soon_ಸೂಚ್ಯಂಕ = ಗಮ್ಯ->dticks & (ಅಂಕೆ_SOON_LISTS - 1U);

		/* later slots ahead until the bucket is sifted */
		uint32_t slots_ahead = ((later_ಸ್ಥಾನ - wheel.later_ಸೂಚ್ಯಂಕ) &
					(ಅಂಕೆ_LATER_LISTS - 1U)) + wheel.sifted;

		ಹಿಂತಿರುಗಿ (k_ticks_t)((ಅಂಕೆ_SOON_LISTS - wheel.soon_ಸೂಚ್ಯಂಕ) +
				   (slots_ahead * ಅಂಕೆ_SOON_LISTS) + soon_ಸೂಚ್ಯಂಕ);
	}

	ರಚನೆ _timeout *t;
	k_ticks_t ಉಳಿದ;

	ಉಳಿದ  = ((ಅಂಕೆ_LATER_LISTS + wheel.sifted + 1) *
		      ಅಂಕೆ_SOON_LISTS) - wheel.soon_ಸೂಚ್ಯಂಕ - 1;

	ಗಾಗಿ (t = timeout_wheel_ಮೊದಲ_ಒಳ_list(&wheel.distant);
	     t != ಖಾಲಿ; t = timeout_wheel_ಮುಂದಿನ_distant(t)) {
		ಉಳಿದ += t->dticks;
		ಆದರೆ (t == ಗಮ್ಯ) {
			ನಿಲ್ಲಿಸು;
		}
	}

	ಹಿಂತಿರುಗಿ ಉಳಿದ;
}

/*
 * Sift from a 'later' list down into the appropriate 'soon' lists. Timeouts
 * are prepended to the 'soon' list to preserve time ordering.
 */
ಸ್ಥಿರ ಶೂನ್ಯ timeout_wheel_ಜರಡಿ_ಕೆಳಕ್ಕೆ_ಗಮ್ಯ_soon(ಶೂನ್ಯ)
{
	sys_dlist_t *list;
	ರಚನೆ _timeout *t;

	list = &wheel.later[wheel.later_ಸೂಚ್ಯಂಕ];
	ಆಗ ((t = timeout_wheel_ಕೊನೆಯ_ಒಳ_list(list)) != ಖಾಲಿ) {
		sys_dlist_remove(&t->ಗಂಟು);
		timeout_wheel_clear_flags(t);
		timeout_wheel_prepend_soon(t);
	}
}

/*
 * Sift from the 'distant' list down into the appropriate 'later' list. Two
 * steps: prepare (massage dticks/flags of the leading distant timeouts) then
 * move them en masse into the 'later' list.
 */
ಸ್ಥಿರ ಶೂನ್ಯ timeout_wheel_ಜರಡಿ_ಕೆಳಕ್ಕೆ_ಗಮ್ಯ_later(ಶೂನ್ಯ)
{
	ರಚನೆ _timeout *t;
	int64_t ಜರಡಿ_ticks = ಅಂಕೆ_SOON_LISTS;
	ರಚನೆ _timeout *ತಲೆ = timeout_wheel_ಮೊದಲ_ಒಳ_list(&wheel.distant);
	ರಚನೆ _timeout *ಬಾಲ = ಖಾಲಿ;
	ಪೂರ್ಣಾಂಕ new_dticks = wheel.later_ಸೂಚ್ಯಂಕ * ಅಂಕೆ_SOON_LISTS;

	ಗಾಗಿ (t = ತಲೆ; t != ಖಾಲಿ; t = timeout_wheel_ಮುಂದಿನ_distant(t)) {
		ಆದರೆ (t->dticks >= ಜರಡಿ_ticks) {
			t->dticks -= ಜರಡಿ_ticks;
			ನಿಲ್ಲಿಸು;
		}

		ಜರಡಿ_ticks -= t->dticks;
		new_dticks += t->dticks;

		t->flags |= _TIMEOUT_FLAG_TIMER_WHEEL_LATER;
		t->dticks = new_dticks;
		ಬಾಲ = t;
	}

	ಆದರೆ (ಬಾಲ != ಖಾಲಿ) {
		sys_dlist_range_prepend(&wheel.later[wheel.later_ಸೂಚ್ಯಂಕ],
					&ತಲೆ->ಗಂಟು, &ಬಾಲ->ಗಂಟು);
	}
}

/*
 * Sifting runs every NUM_SOON_LISTS ticks: first move the current 'later' list
 * into the 'soon' lists, freeing it to receive the next batch sifted from
 * 'distant'.
 */
ಸ್ಥಿರ ಶೂನ್ಯ timeout_wheel_ಜರಡಿ_ಕೆಳಕ್ಕೆ(ಶೂನ್ಯ)
{
	timeout_wheel_ಜರಡಿ_ಕೆಳಕ್ಕೆ_ಗಮ್ಯ_soon();

	timeout_wheel_ಜರಡಿ_ಕೆಳಕ್ಕೆ_ಗಮ್ಯ_later();

	/* Advance to the next 'later' list */
	wheel.later_ಸೂಚ್ಯಂಕ++;
	wheel.later_ಸೂಚ್ಯಂಕ &= (ಅಂಕೆ_LATER_LISTS - 1);
	wheel.sifted = 1;
}

/*
 * Announce all the timeouts expiring "now" (at soon_index). curr_tick has been
 * advanced. Handlers fire with the lock dropped; inflight_timeout tracks
 * the in-flight timeout so aborts race correctly.
 */
ಸ್ಥಿರ k_spinlock_ಕೀಲಿ_t timeout_wheel_announce_now(k_spinlock_ಕೀಲಿ_t ಕೀಲಿ)
{
	ರಚನೆ _timeout *t;
	uint32_t ಸೂಚ್ಯಂಕ = wheel.soon_ಸೂಚ್ಯಂಕ;

	wheel.sifted = 0;
	wheel.announcing = 1;
	ಗಾಗಿ (t = timeout_wheel_ಮೊದಲ_ಒಳ_list(&wheel.soon[ಸೂಚ್ಯಂಕ]);
	     t != ಖಾಲಿ;
	     t = timeout_wheel_ಮೊದಲ_ಒಳ_list(&wheel.soon[ಸೂಚ್ಯಂಕ])) {
		sys_dlist_remove(&t->ಗಂಟು);
		timeout_wheel_clear_flags(t);

		inflight_timeout = t;
		k_spin_unlock(&timeout_ಬೀಗ, ಕೀಲಿ);
		t->fn(t);
		ಕೀಲಿ = k_spin_ಬೀಗ(&timeout_ಬೀಗ);
		inflight_timeout = ಖಾಲಿ;
	}
	wheel.announcing = 0;

	/*
	 * The 'soon_index' slot is now drained, so 'soon deferred' timeouts
	 * (exactly NUM_SOON_LISTS into the future) can move into their slots.
	 */
	ಗಾಗಿ (t = timeout_wheel_ಮೊದಲ_ಒಳ_list(&wheel.soon_defer);
	     t != ಖಾಲಿ;
	     t = timeout_wheel_ಮೊದಲ_ಒಳ_list(&wheel.soon_defer)) {
		sys_dlist_remove(&t->ಗಂಟು);
		/* Correct the dticks to be the appropriate 'soon' list index */
		t->dticks &= (ಅಂಕೆ_SOON_LISTS - 1);
		wheel.soon_ಬಿಟ್ಟುಗಳು |= ಬಿಟ್(t->dticks);
		sys_dlist_append(&wheel.soon[t->dticks], &t->ಗಂಟು);
	}

	ಆದರೆ (ಸೂಚ್ಯಂಕ == (ಅಂಕೆ_SOON_LISTS - 1)) {
		timeout_wheel_ಜರಡಿ_ಕೆಳಕ್ಕೆ();
	}

	ಹಿಂತಿರುಗಿ ಕೀಲಿ;
}

/*
 * Backend interface (see kernel/timeout.c).
 */

ಸ್ಥಿರ bool z_timeout_q_ಸೇರಿಸು(ರಚನೆ _timeout *ಗಮ್ಯ, k_ticks_t dticks)
{
	ಗಮ್ಯ->dticks = dticks;
	timeout_wheel_ಕೂಡಿಸು_timeout(ಗಮ್ಯ);

	ಹಿಂತಿರುಗಿ ಗಮ್ಯ == timeout_wheel_ಮೊದಲ();
}

ಸ್ಥಿರ bool z_timeout_q_remove(ರಚನೆ _timeout *ಗಮ್ಯ)
{
	bool was_ಮೊದಲ = (ಗಮ್ಯ == timeout_wheel_ಮೊದಲ());

	timeout_wheel_remove_timeout(ಗಮ್ಯ);

	ಹಿಂತಿರುಗಿ was_ಮೊದಲ;
}

ಸ್ಥಿರ k_ticks_t z_timeout_q_remainder(ಸ್ಥಿರಾಂಕ ರಚನೆ _timeout *ಗಮ್ಯ)
{
	ಹಿಂತಿರುಗಿ timeout_wheel_remainder(ಗಮ್ಯ);
}

/*
 * Ticks from curr_tick to the next 'soon' timeout, between 1 and
 * NUM_SOON_LISTS (the 'soon_index' list is excluded as it is timing out).
 * Never K_TICKS_FOREVER: a sift is always pending, which is why the wheel
 * wakes a tickless-idle CPU at least every NUM_SOON_LISTS ticks. The
 * announce-range cap is applied centrally by next_timeout() in timeout.c.
 */
ಸ್ಥಿರ k_ticks_t z_timeout_q_ಮುಂದಿನ_expiry(ಶೂನ್ಯ)
{
	ರಚನೆ _timeout *ಗಮ್ಯ = timeout_wheel_ಮೊದಲ();
	int64_t dticks = (ಗಮ್ಯ != ಖಾಲಿ) ? ಗಮ್ಯ->dticks : (ಅಂಕೆ_SOON_LISTS - 1);

	dticks = ((ಅಂಕೆ_SOON_LISTS - 1 - wheel.soon_ಸೂಚ್ಯಂಕ + dticks) &
		  (ಅಂಕೆ_SOON_LISTS - 1)) + 1;

	ಹಿಂತಿರುಗಿ (k_ticks_t)dticks;
}

/* Backend-owned announce loop (see _TIMEOUT_BACKEND_OWNS_ANNOUNCE). Drains all
 * timeouts due within announce_remaining ticks, firing each handler with the
 * subsystem lock dropped; advances curr_tick and announce_remaining. Returns
 * the (possibly re-acquired) spinlock key.
 */
ಸ್ಥಿರ k_spinlock_ಕೀಲಿ_t z_timeout_q_announce(k_spinlock_ಕೀಲಿ_t ಕೀಲಿ)
{
	uint32_t ಮುಖವಾಡ;
	ಪೂರ್ಣಾಂಕ      ಮುಂದಿನ;    /* Index of next 'soon' list to expire */
	ಪೂರ್ಣಾಂಕ      ಅಂಕೆ_ticks_ಗಮ್ಯ_advance;
	ಪೂರ್ಣಾಂಕ      correction;  /* NUM_SOON_LISTS or 0 */
	uint32_t ಬಿಟ್ಟುಗಳು;

	ಆಗ (announce_ಉಳಿದ > 0) {

		ಮುಖವಾಡ = 0xFFFFFFFF << ((wheel.soon_ಸೂಚ್ಯಂಕ + 1) & (ಅಂಕೆ_SOON_LISTS - 1));
		ಬಿಟ್ಟುಗಳು = wheel.soon_ಬಿಟ್ಟುಗಳು | ಬಿಟ್(ಅಂಕೆ_SOON_LISTS - 1);
		ಮುಂದಿನ = u32_ಎಣಿಕೆ_trailing_zeros(ಮುಖವಾಡ & ಬಿಟ್ಟುಗಳು);
		correction = ((wheel.soon_ಸೂಚ್ಯಂಕ + 1) / ಅಂಕೆ_SOON_LISTS) * ಅಂಕೆ_SOON_LISTS;
		ಅಂಕೆ_ticks_ಗಮ್ಯ_advance = ಮುಂದಿನ + correction - wheel.soon_ಸೂಚ್ಯಂಕ;

		ಆದರೆ (ಅಂಕೆ_ticks_ಗಮ್ಯ_advance > announce_ಉಳಿದ) {
			curr_tick += announce_ಉಳಿದ;
			wheel.sifted = 0;
			wheel.soon_ಸೂಚ್ಯಂಕ += announce_ಉಳಿದ;
			wheel.soon_ಸೂಚ್ಯಂಕ &= (ಅಂಕೆ_SOON_LISTS - 1);

			announce_ಉಳಿದ = 0;
			ನಿಲ್ಲಿಸು;
		}

		/*
		 * 1. Advance to the next tick to process.
		 * 2. Clear its bit in the 'soon_bits'.
		 */
		wheel.soon_ಸೂಚ್ಯಂಕ += ಅಂಕೆ_ticks_ಗಮ್ಯ_advance;
		wheel.soon_ಸೂಚ್ಯಂಕ &= (ಅಂಕೆ_SOON_LISTS - 1);
		wheel.soon_ಬಿಟ್ಟುಗಳು &= ~ಬಿಟ್(wheel.soon_ಸೂಚ್ಯಂಕ);

		curr_tick += ಅಂಕೆ_ticks_ಗಮ್ಯ_advance;
		ಕೀಲಿ = timeout_wheel_announce_now(ಕೀಲಿ);
		announce_ಉಳಿದ -= ಅಂಕೆ_ticks_ಗಮ್ಯ_advance;
	}

	ಹಿಂತಿರುಗಿ ಕೀಲಿ;
}

#ಆದರೆ_ಅಂತ್ಯ /* ZEPHYR_KERNEL_TIMEOUT_WHEEL_H_ */
