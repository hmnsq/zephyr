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

#அந்தர்பாவ <zephyr/init.h>
#அந்தர்பாவ <zephyr/sys/dlist.h>

/* Per-node flag bits (struct _timeout's flags field): which tier the timeout
 * currently lives in, so removal can maintain the soon-list occupancy bitmap.
 */
#define _TIMEOUT_FLAG_TIMER_WHEEL_SOON  BIT(0)
#define _TIMEOUT_FLAG_TIMER_WHEEL_LATER BIT(1)

/* This backend supplies its own announce loop. */
#define _TIMEOUT_BACKEND_OWNS_ANNOUNCE 1

#define இலக்கம்_SOON_LISTS  32
#define இலக்கம்_LATER_LISTS 32

கட்டமைப்பு timeout_wheel {
	sys_dlist_t soon[இலக்கம்_SOON_LISTS];   /* timeouts expiring soon lists */
	sys_dlist_t later[இலக்கம்_LATER_LISTS]; /* timeouts expiring later lists */
	sys_dlist_t distant;                /* timeouts expiring far in future */
	sys_dlist_t soon_defer;             /* special soon list */
	uint32_t    soon_bits;              /* Set bit indicates list in use */
	uint32_t    soon_index;             /* Current soon list index */
	uint32_t    later_index;
	uint32_t    announcing;             /* 1: Announcing soon_index, else 0 */
	uint32_t    sifted;                 /* 1: Sifted, else 0 */
};

நிலையான கட்டமைப்பு timeout_wheel wheel;

நிலையான முழு timeout_wheel_init(வெறுமை)
{
	unsigned முழு i;

	க்கு (i = 0; i < இலக்கம்_SOON_LISTS; i++) {
		sys_dlist_init(&wheel.soon[i]);
	}

	க்கு (i = 0; i < இலக்கம்_LATER_LISTS; i++) {
		sys_dlist_init(&wheel.later[i]);
	}

	sys_dlist_init(&wheel.distant);
	sys_dlist_init(&wheel.soon_defer);

	wheel.soon_bits = 0;
	wheel.soon_index = இலக்கம்_SOON_LISTS - 1;
	wheel.later_index = 0;
	wheel.announcing = 0;
	wheel.sifted = 1;

	திரும்பு 0;
}

/* Initialize the wheel before the timer driver (PRE_KERNEL_2) and before any
 * timeout can be added. Priority 0 keeps it ahead of other PRE_KERNEL_1 init.
 */
SYS_INIT(timeout_wheel_init, PRE_KERNEL_1, 0);

நிலையான உள்வரி வெறுமை timeout_wheel_clear_flags(கட்டமைப்பு _timeout *to)
{
	to->flags &= ~(_TIMEOUT_FLAG_TIMER_WHEEL_SOON |
		       _TIMEOUT_FLAG_TIMER_WHEEL_LATER);
}

/* Return the last item in the specified list, or NULL if the list is empty. */
நிலையான கட்டமைப்பு _timeout *timeout_wheel_last_in_list(sys_dlist_t *list)
{
	sys_dnode_t *t = sys_dlist_peek_tail(list);

	திரும்பு (t == வெற்று) ? வெற்று : CONTAINER_OF(t, கட்டமைப்பு _timeout, முனை);
}

/* Return the first timeout expiring in the specified list, or NULL. */
நிலையான கட்டமைப்பு _timeout *timeout_wheel_first_in_list(sys_dlist_t *list)
{
	sys_dnode_t *t = sys_dlist_peek_head(list);

	திரும்பு (t == வெற்று) ? வெற்று : CONTAINER_OF(t, கட்டமைப்பு _timeout, முனை);
}

/*
 * Get the first timeout in the wheel from its 'soon' lists. NULL means that
 * the first timeout is the sift operation which always implicitly exists
 * within the final 'soon' list.
 */
நிலையான கட்டமைப்பு _timeout *timeout_wheel_first(வெறுமை)
{
	uint32_t bitmap;
	unsigned முழு index;
	sys_dnode_t *t;

	bitmap = wheel.soon_bits | BIT(இலக்கம்_SOON_LISTS - 1);
	எனில் (wheel.sifted == 0) {
		bitmap &= ~(BIT(wheel.soon_index) - 1);
	}
	index = u32_மொத்தம்_trailing_zeros(bitmap);

	t = sys_dlist_peek_head(&wheel.soon[index]);

	திரும்பு (t == வெற்று) ? வெற்று : CONTAINER_OF(t, கட்டமைப்பு _timeout, முனை);
}

/* Return the timeout following @a t expiring "distantly". */
நிலையான கட்டமைப்பு _timeout *timeout_wheel_next_distant(கட்டமைப்பு _timeout *t)
{
	sys_dnode_t *n = sys_dlist_peek_next(&wheel.distant, &t->முனை);

	திரும்பு (n == வெற்று) ? வெற்று : CONTAINER_OF(n, கட்டமைப்பு _timeout, முனை);
}

/*
 * Prepend a timeout to the appropriate "soon" list. Only called during sifting
 * after the timeout had been removed from a 'later' list, where its 'dticks'
 * field was a composite value. Prepending preserves time ordering.
 */
நிலையான வெறுமை timeout_wheel_prepend_soon(கட்டமைப்பு _timeout *to)
{
	unsigned முழு index = to->dticks & (இலக்கம்_SOON_LISTS - 1);

	wheel.soon_bits |= BIT(index);
	sys_dlist_prepend(&wheel.soon[index], &to->முனை);
	to->flags |= _TIMEOUT_FLAG_TIMER_WHEEL_SOON;
	to->dticks = index;
}

/*
 * Append a timeout to the appropriate "soon" list, recording the list index
 * in the timeout's 'dticks' field.
 */
நிலையான வெறுமை timeout_wheel_append_soon(கட்டமைப்பு _timeout *to)
{
	unsigned முழு index;

	index = (to->dticks + wheel.soon_index) & (இலக்கம்_SOON_LISTS - 1);
	to->dticks = index;

	wheel.soon_bits |= BIT(index);
	sys_dlist_append(&wheel.soon[index], &to->முனை);
	to->flags |= _TIMEOUT_FLAG_TIMER_WHEEL_SOON;
}

/*
 * Append a timeout to the soon deferred list. Special case for timeouts that
 * are exactly NUM_SOON_LISTS into the future while the system is announcing:
 * they must not get mixed with the currently-expiring rotation. They move into
 * the appropriate 'soon' list at the end of the announce.
 */
நிலையான வெறுமை timeout_wheel_append_soon_defer(கட்டமைப்பு _timeout *to)
{
	unsigned முழு index;

	index = (to->dticks + wheel.soon_index) & (இலக்கம்_SOON_LISTS - 1);
	to->dticks = index + இலக்கம்_SOON_LISTS;

	sys_dlist_append(&wheel.soon_defer, &to->முனை);
	to->flags |= _TIMEOUT_FLAG_TIMER_WHEEL_SOON;
}

/*
 * Append a timeout to the appropriate "later" list. The 'dticks' field is set
 * to identify both the eventual 'soon' list and the 'later' list it goes to.
 */
நிலையான வெறுமை timeout_wheel_append_later(கட்டமைப்பு _timeout *to)
{
	uint32_t position;
	uint32_t soon;
	uint32_t later;

	position = to->dticks + wheel.soon_index - இலக்கம்_SOON_LISTS;

	soon = position & (இலக்கம்_SOON_LISTS - 1);
	later = ((position / இலக்கம்_SOON_LISTS) + wheel.later_index -
		 wheel.sifted) & (இலக்கம்_LATER_LISTS - 1);

	to->dticks = (later * இலக்கம்_SOON_LISTS) + soon;
	to->flags |= _TIMEOUT_FLAG_TIMER_WHEEL_LATER;

	sys_dlist_append(&wheel.later[later], &to->முனை);
}

/*
 * Insert a timeout into the "distant" list, an expiry-ordered delta list. For
 * good performance the distant list should stay short.
 */
நிலையான வெறுமை timeout_wheel_செருகு_distant(கட்டமைப்பு _timeout *to, uint32_t cutoff)
{
	கட்டமைப்பு _timeout *t;

	to->dticks -= cutoff;

	க்கு (t = timeout_wheel_first_in_list(&wheel.distant);
	     t != வெற்று; t = timeout_wheel_next_distant(t)) {
		எனில் (t->dticks > to->dticks) {
			t->dticks -= to->dticks;
			sys_dlist_செருகு(&t->முனை, &to->முனை);
			நிறுத்து;
		}
		to->dticks -= t->dticks;
	}

	எனில் (t == வெற்று) {
		sys_dlist_append(&wheel.distant, &to->முனை);
	}
}

/*
 * Add a timeout to the wheel (with to->dticks already set to its tick delta
 * from curr_tick). It is never placed into the 'soon_index' list.
 */
நிலையான வெறுமை timeout_wheel_add_timeout(கட்டமைப்பு _timeout *to)
{
	__ASSERT_NO_MSG(to->dticks != 0);

	எனில் (to->dticks <= (இலக்கம்_SOON_LISTS - wheel.announcing)) {
		timeout_wheel_append_soon(to);
		திரும்பு;
	}

	எனில் (to->dticks == இலக்கம்_SOON_LISTS) {
		timeout_wheel_append_soon_defer(to);
		திரும்பு;
	}

	uint32_t cutoff;

	cutoff = ((இலக்கம்_LATER_LISTS + wheel.sifted + 1) *
		  இலக்கம்_SOON_LISTS) - wheel.soon_index - 1;
	எனில் (to->dticks <= cutoff) {
		timeout_wheel_append_later(to);
		திரும்பு;
	}

	timeout_wheel_செருகு_distant(to, cutoff);
}

/* Remove the specified timeout. */
நிலையான வெறுமை timeout_wheel_remove_timeout(கட்டமைப்பு _timeout *to)
{
	sys_dlist_remove(&to->முனை);

	எனில் (to->flags & _TIMEOUT_FLAG_TIMER_WHEEL_SOON) {
		unsigned முழு index = to->dticks;

		/* soon_defer entries carry index + NUM_SOON_LISTS and own no
		 * soon_bits bit; only true soon-list entries do.
		 */
		எனில் (index < இலக்கம்_SOON_LISTS &&
		    sys_dlist_is_empty(&wheel.soon[index])) {
			wheel.soon_bits &= ~BIT(index);
		}
	}

	timeout_wheel_clear_flags(to);
}

/* Calculate the number of ticks until the specified timeout expires. */
நிலையான k_ticks_t timeout_wheel_remainder(மாறிலி கட்டமைப்பு _timeout *to)
{
	எனில் (to->flags & _TIMEOUT_FLAG_TIMER_WHEEL_SOON) {
		uint32_t now = wheel.soon_index;
		uint32_t idx = to->dticks;

		எனில் (idx == now) {
			திரும்பு (1U - wheel.announcing) * இலக்கம்_SOON_LISTS;
		} இல்லையெனில் எனில் (idx >= இலக்கம்_SOON_LISTS) {
			/* Special case if querying item in 'soon_defer' list */
			திரும்பு இலக்கம்_SOON_LISTS;
		}

		திரும்பு (idx - now) & (இலக்கம்_SOON_LISTS - 1U);
	}

	எனில் (to->flags & _TIMEOUT_FLAG_TIMER_WHEEL_LATER) {
		uint32_t later_slot = to->dticks / இலக்கம்_SOON_LISTS;
		uint32_t soon_index = to->dticks & (இலக்கம்_SOON_LISTS - 1U);

		/* later slots ahead until the bucket is sifted */
		uint32_t slots_ahead = ((later_slot - wheel.later_index) &
					(இலக்கம்_LATER_LISTS - 1U)) + wheel.sifted;

		திரும்பு (k_ticks_t)((இலக்கம்_SOON_LISTS - wheel.soon_index) +
				   (slots_ahead * இலக்கம்_SOON_LISTS) + soon_index);
	}

	கட்டமைப்பு _timeout *t;
	k_ticks_t remaining;

	remaining  = ((இலக்கம்_LATER_LISTS + wheel.sifted + 1) *
		      இலக்கம்_SOON_LISTS) - wheel.soon_index - 1;

	க்கு (t = timeout_wheel_first_in_list(&wheel.distant);
	     t != வெற்று; t = timeout_wheel_next_distant(t)) {
		remaining += t->dticks;
		எனில் (t == to) {
			நிறுத்து;
		}
	}

	திரும்பு remaining;
}

/*
 * Sift from a 'later' list down into the appropriate 'soon' lists. Timeouts
 * are prepended to the 'soon' list to preserve time ordering.
 */
நிலையான வெறுமை timeout_wheel_sift_down_to_soon(வெறுமை)
{
	sys_dlist_t *list;
	கட்டமைப்பு _timeout *t;

	list = &wheel.later[wheel.later_index];
	போது ((t = timeout_wheel_last_in_list(list)) != வெற்று) {
		sys_dlist_remove(&t->முனை);
		timeout_wheel_clear_flags(t);
		timeout_wheel_prepend_soon(t);
	}
}

/*
 * Sift from the 'distant' list down into the appropriate 'later' list. Two
 * steps: prepare (massage dticks/flags of the leading distant timeouts) then
 * move them en masse into the 'later' list.
 */
நிலையான வெறுமை timeout_wheel_sift_down_to_later(வெறுமை)
{
	கட்டமைப்பு _timeout *t;
	int64_t sift_ticks = இலக்கம்_SOON_LISTS;
	கட்டமைப்பு _timeout *head = timeout_wheel_first_in_list(&wheel.distant);
	கட்டமைப்பு _timeout *tail = வெற்று;
	முழு new_dticks = wheel.later_index * இலக்கம்_SOON_LISTS;

	க்கு (t = head; t != வெற்று; t = timeout_wheel_next_distant(t)) {
		எனில் (t->dticks >= sift_ticks) {
			t->dticks -= sift_ticks;
			நிறுத்து;
		}

		sift_ticks -= t->dticks;
		new_dticks += t->dticks;

		t->flags |= _TIMEOUT_FLAG_TIMER_WHEEL_LATER;
		t->dticks = new_dticks;
		tail = t;
	}

	எனில் (tail != வெற்று) {
		sys_dlist_range_prepend(&wheel.later[wheel.later_index],
					&head->முனை, &tail->முனை);
	}
}

/*
 * Sifting runs every NUM_SOON_LISTS ticks: first move the current 'later' list
 * into the 'soon' lists, freeing it to receive the next batch sifted from
 * 'distant'.
 */
நிலையான வெறுமை timeout_wheel_sift_down(வெறுமை)
{
	timeout_wheel_sift_down_to_soon();

	timeout_wheel_sift_down_to_later();

	/* Advance to the next 'later' list */
	wheel.later_index++;
	wheel.later_index &= (இலக்கம்_LATER_LISTS - 1);
	wheel.sifted = 1;
}

/*
 * Announce all the timeouts expiring "now" (at soon_index). curr_tick has been
 * advanced. Handlers fire with the lock dropped; inflight_timeout tracks
 * the in-flight timeout so aborts race correctly.
 */
நிலையான k_spinlock_சாவி_t timeout_wheel_announce_now(k_spinlock_சாவி_t சாவி)
{
	கட்டமைப்பு _timeout *t;
	uint32_t index = wheel.soon_index;

	wheel.sifted = 0;
	wheel.announcing = 1;
	க்கு (t = timeout_wheel_first_in_list(&wheel.soon[index]);
	     t != வெற்று;
	     t = timeout_wheel_first_in_list(&wheel.soon[index])) {
		sys_dlist_remove(&t->முனை);
		timeout_wheel_clear_flags(t);

		inflight_timeout = t;
		k_spin_unlock(&timeout_lock, சாவி);
		t->fn(t);
		சாவி = k_spin_lock(&timeout_lock);
		inflight_timeout = வெற்று;
	}
	wheel.announcing = 0;

	/*
	 * The 'soon_index' slot is now drained, so 'soon deferred' timeouts
	 * (exactly NUM_SOON_LISTS into the future) can move into their slots.
	 */
	க்கு (t = timeout_wheel_first_in_list(&wheel.soon_defer);
	     t != வெற்று;
	     t = timeout_wheel_first_in_list(&wheel.soon_defer)) {
		sys_dlist_remove(&t->முனை);
		/* Correct the dticks to be the appropriate 'soon' list index */
		t->dticks &= (இலக்கம்_SOON_LISTS - 1);
		wheel.soon_bits |= BIT(t->dticks);
		sys_dlist_append(&wheel.soon[t->dticks], &t->முனை);
	}

	எனில் (index == (இலக்கம்_SOON_LISTS - 1)) {
		timeout_wheel_sift_down();
	}

	திரும்பு சாவி;
}

/*
 * Backend interface (see kernel/timeout.c).
 */

நிலையான bool z_timeout_q_செருகு(கட்டமைப்பு _timeout *to, k_ticks_t dticks)
{
	to->dticks = dticks;
	timeout_wheel_add_timeout(to);

	திரும்பு to == timeout_wheel_first();
}

நிலையான bool z_timeout_q_remove(கட்டமைப்பு _timeout *to)
{
	bool was_first = (to == timeout_wheel_first());

	timeout_wheel_remove_timeout(to);

	திரும்பு was_first;
}

நிலையான k_ticks_t z_timeout_q_remainder(மாறிலி கட்டமைப்பு _timeout *to)
{
	திரும்பு timeout_wheel_remainder(to);
}

/*
 * Ticks from curr_tick to the next 'soon' timeout, between 1 and
 * NUM_SOON_LISTS (the 'soon_index' list is excluded as it is timing out).
 * Never K_TICKS_FOREVER: a sift is always pending, which is why the wheel
 * wakes a tickless-idle CPU at least every NUM_SOON_LISTS ticks. The
 * announce-range cap is applied centrally by next_timeout() in timeout.c.
 */
நிலையான k_ticks_t z_timeout_q_next_expiry(வெறுமை)
{
	கட்டமைப்பு _timeout *to = timeout_wheel_first();
	int64_t dticks = (to != வெற்று) ? to->dticks : (இலக்கம்_SOON_LISTS - 1);

	dticks = ((இலக்கம்_SOON_LISTS - 1 - wheel.soon_index + dticks) &
		  (இலக்கம்_SOON_LISTS - 1)) + 1;

	திரும்பு (k_ticks_t)dticks;
}

/* Backend-owned announce loop (see _TIMEOUT_BACKEND_OWNS_ANNOUNCE). Drains all
 * timeouts due within announce_remaining ticks, firing each handler with the
 * subsystem lock dropped; advances curr_tick and announce_remaining. Returns
 * the (possibly re-acquired) spinlock key.
 */
நிலையான k_spinlock_சாவி_t z_timeout_q_announce(k_spinlock_சாவி_t சாவி)
{
	uint32_t mask;
	முழு      next;    /* Index of next 'soon' list to expire */
	முழு      இலக்கம்_ticks_to_advance;
	முழு      correction;  /* NUM_SOON_LISTS or 0 */
	uint32_t bits;

	போது (announce_remaining > 0) {

		mask = 0xFFFFFFFF << ((wheel.soon_index + 1) & (இலக்கம்_SOON_LISTS - 1));
		bits = wheel.soon_bits | BIT(இலக்கம்_SOON_LISTS - 1);
		next = u32_மொத்தம்_trailing_zeros(mask & bits);
		correction = ((wheel.soon_index + 1) / இலக்கம்_SOON_LISTS) * இலக்கம்_SOON_LISTS;
		இலக்கம்_ticks_to_advance = next + correction - wheel.soon_index;

		எனில் (இலக்கம்_ticks_to_advance > announce_remaining) {
			curr_tick += announce_remaining;
			wheel.sifted = 0;
			wheel.soon_index += announce_remaining;
			wheel.soon_index &= (இலக்கம்_SOON_LISTS - 1);

			announce_remaining = 0;
			நிறுத்து;
		}

		/*
		 * 1. Advance to the next tick to process.
		 * 2. Clear its bit in the 'soon_bits'.
		 */
		wheel.soon_index += இலக்கம்_ticks_to_advance;
		wheel.soon_index &= (இலக்கம்_SOON_LISTS - 1);
		wheel.soon_bits &= ~BIT(wheel.soon_index);

		curr_tick += இலக்கம்_ticks_to_advance;
		சாவி = timeout_wheel_announce_now(சாவி);
		announce_remaining -= இலக்கம்_ticks_to_advance;
	}

	திரும்பு சாவி;
}

#endif /* ZEPHYR_KERNEL_TIMEOUT_WHEEL_H_ */
