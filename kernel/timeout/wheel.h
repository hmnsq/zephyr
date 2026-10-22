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

#অন্তর্ভাব <zephyr/init.h>
#অন্তর্ভাব <zephyr/sys/dlist.h>

/* Per-node flag bits (struct _timeout's flags field): which tier the timeout
 * currently lives in, so removal can maintain the soon-list occupancy bitmap.
 */
#define _TIMEOUT_FLAG_TIMER_WHEEL_SOON  BIT(0)
#define _TIMEOUT_FLAG_TIMER_WHEEL_LATER BIT(1)

/* This backend supplies its own announce loop. */
#define _TIMEOUT_BACKEND_OWNS_ANNOUNCE 1

#define অঙ্ক_SOON_LISTS  32
#define অঙ্ক_LATER_LISTS 32

কাঠামো timeout_wheel {
	sys_dlist_t soon[অঙ্ক_SOON_LISTS];   /* timeouts expiring soon lists */
	sys_dlist_t later[অঙ্ক_LATER_LISTS]; /* timeouts expiring later lists */
	sys_dlist_t distant;                /* timeouts expiring far in future */
	sys_dlist_t soon_defer;             /* special soon list */
	uint32_t    soon_bits;              /* Set bit indicates list in use */
	uint32_t    soon_সূচক;             /* Current soon list index */
	uint32_t    later_সূচক;
	uint32_t    announcing;             /* 1: Announcing soon_index, else 0 */
	uint32_t    sifted;                 /* 1: Sifted, else 0 */
};

স্থির কাঠামো timeout_wheel wheel;

স্থির পূর্ণসংখ্যা timeout_wheel_init(শূন্য)
{
	unsigned পূর্ণসংখ্যা i;

	জন্য (i = 0; i < অঙ্ক_SOON_LISTS; i++) {
		sys_dlist_init(&wheel.soon[i]);
	}

	জন্য (i = 0; i < অঙ্ক_LATER_LISTS; i++) {
		sys_dlist_init(&wheel.later[i]);
	}

	sys_dlist_init(&wheel.distant);
	sys_dlist_init(&wheel.soon_defer);

	wheel.soon_bits = 0;
	wheel.soon_সূচক = অঙ্ক_SOON_LISTS - 1;
	wheel.later_সূচক = 0;
	wheel.announcing = 0;
	wheel.sifted = 1;

	ফেরত 0;
}

/* Initialize the wheel before the timer driver (PRE_KERNEL_2) and before any
 * timeout can be added. Priority 0 keeps it ahead of other PRE_KERNEL_1 init.
 */
SYS_INIT(timeout_wheel_init, PRE_KERNEL_1, 0);

স্থির অন্তঃরেখ শূন্য timeout_wheel_clear_flags(কাঠামো _timeout *to)
{
	to->flags &= ~(_TIMEOUT_FLAG_TIMER_WHEEL_SOON |
		       _TIMEOUT_FLAG_TIMER_WHEEL_LATER);
}

/* Return the last item in the specified list, or NULL if the list is empty. */
স্থির কাঠামো _timeout *timeout_wheel_last_in_list(sys_dlist_t *list)
{
	sys_dnode_t *t = sys_dlist_peek_লেজ(list);

	ফেরত (t == ফাঁকা) ? ফাঁকা : CONTAINER_OF(t, কাঠামো _timeout, গ্রন্থি);
}

/* Return the first timeout expiring in the specified list, or NULL. */
স্থির কাঠামো _timeout *timeout_wheel_first_in_list(sys_dlist_t *list)
{
	sys_dnode_t *t = sys_dlist_peek_শীর্ষ(list);

	ফেরত (t == ফাঁকা) ? ফাঁকা : CONTAINER_OF(t, কাঠামো _timeout, গ্রন্থি);
}

/*
 * Get the first timeout in the wheel from its 'soon' lists. NULL means that
 * the first timeout is the sift operation which always implicitly exists
 * within the final 'soon' list.
 */
স্থির কাঠামো _timeout *timeout_wheel_first(শূন্য)
{
	uint32_t bitmap;
	unsigned পূর্ণসংখ্যা সূচক;
	sys_dnode_t *t;

	bitmap = wheel.soon_bits | BIT(অঙ্ক_SOON_LISTS - 1);
	যদি (wheel.sifted == 0) {
		bitmap &= ~(BIT(wheel.soon_সূচক) - 1);
	}
	সূচক = u32_গুনতি_trailing_zeros(bitmap);

	t = sys_dlist_peek_শীর্ষ(&wheel.soon[সূচক]);

	ফেরত (t == ফাঁকা) ? ফাঁকা : CONTAINER_OF(t, কাঠামো _timeout, গ্রন্থি);
}

/* Return the timeout following @a t expiring "distantly". */
স্থির কাঠামো _timeout *timeout_wheel_পরবর্তী_distant(কাঠামো _timeout *t)
{
	sys_dnode_t *n = sys_dlist_peek_পরবর্তী(&wheel.distant, &t->গ্রন্থি);

	ফেরত (n == ফাঁকা) ? ফাঁকা : CONTAINER_OF(n, কাঠামো _timeout, গ্রন্থি);
}

/*
 * Prepend a timeout to the appropriate "soon" list. Only called during sifting
 * after the timeout had been removed from a 'later' list, where its 'dticks'
 * field was a composite value. Prepending preserves time ordering.
 */
স্থির শূন্য timeout_wheel_prepend_soon(কাঠামো _timeout *to)
{
	unsigned পূর্ণসংখ্যা সূচক = to->dticks & (অঙ্ক_SOON_LISTS - 1);

	wheel.soon_bits |= BIT(সূচক);
	sys_dlist_prepend(&wheel.soon[সূচক], &to->গ্রন্থি);
	to->flags |= _TIMEOUT_FLAG_TIMER_WHEEL_SOON;
	to->dticks = সূচক;
}

/*
 * Append a timeout to the appropriate "soon" list, recording the list index
 * in the timeout's 'dticks' field.
 */
স্থির শূন্য timeout_wheel_append_soon(কাঠামো _timeout *to)
{
	unsigned পূর্ণসংখ্যা সূচক;

	সূচক = (to->dticks + wheel.soon_সূচক) & (অঙ্ক_SOON_LISTS - 1);
	to->dticks = সূচক;

	wheel.soon_bits |= BIT(সূচক);
	sys_dlist_append(&wheel.soon[সূচক], &to->গ্রন্থি);
	to->flags |= _TIMEOUT_FLAG_TIMER_WHEEL_SOON;
}

/*
 * Append a timeout to the soon deferred list. Special case for timeouts that
 * are exactly NUM_SOON_LISTS into the future while the system is announcing:
 * they must not get mixed with the currently-expiring rotation. They move into
 * the appropriate 'soon' list at the end of the announce.
 */
স্থির শূন্য timeout_wheel_append_soon_defer(কাঠামো _timeout *to)
{
	unsigned পূর্ণসংখ্যা সূচক;

	সূচক = (to->dticks + wheel.soon_সূচক) & (অঙ্ক_SOON_LISTS - 1);
	to->dticks = সূচক + অঙ্ক_SOON_LISTS;

	sys_dlist_append(&wheel.soon_defer, &to->গ্রন্থি);
	to->flags |= _TIMEOUT_FLAG_TIMER_WHEEL_SOON;
}

/*
 * Append a timeout to the appropriate "later" list. The 'dticks' field is set
 * to identify both the eventual 'soon' list and the 'later' list it goes to.
 */
স্থির শূন্য timeout_wheel_append_later(কাঠামো _timeout *to)
{
	uint32_t position;
	uint32_t soon;
	uint32_t later;

	position = to->dticks + wheel.soon_সূচক - অঙ্ক_SOON_LISTS;

	soon = position & (অঙ্ক_SOON_LISTS - 1);
	later = ((position / অঙ্ক_SOON_LISTS) + wheel.later_সূচক -
		 wheel.sifted) & (অঙ্ক_LATER_LISTS - 1);

	to->dticks = (later * অঙ্ক_SOON_LISTS) + soon;
	to->flags |= _TIMEOUT_FLAG_TIMER_WHEEL_LATER;

	sys_dlist_append(&wheel.later[later], &to->গ্রন্থি);
}

/*
 * Insert a timeout into the "distant" list, an expiry-ordered delta list. For
 * good performance the distant list should stay short.
 */
স্থির শূন্য timeout_wheel_ঢোকাও_distant(কাঠামো _timeout *to, uint32_t cutoff)
{
	কাঠামো _timeout *t;

	to->dticks -= cutoff;

	জন্য (t = timeout_wheel_first_in_list(&wheel.distant);
	     t != ফাঁকা; t = timeout_wheel_পরবর্তী_distant(t)) {
		যদি (t->dticks > to->dticks) {
			t->dticks -= to->dticks;
			sys_dlist_ঢোকাও(&t->গ্রন্থি, &to->গ্রন্থি);
			বিরতি;
		}
		to->dticks -= t->dticks;
	}

	যদি (t == ফাঁকা) {
		sys_dlist_append(&wheel.distant, &to->গ্রন্থি);
	}
}

/*
 * Add a timeout to the wheel (with to->dticks already set to its tick delta
 * from curr_tick). It is never placed into the 'soon_index' list.
 */
স্থির শূন্য timeout_wheel_add_timeout(কাঠামো _timeout *to)
{
	__ASSERT_NO_MSG(to->dticks != 0);

	যদি (to->dticks <= (অঙ্ক_SOON_LISTS - wheel.announcing)) {
		timeout_wheel_append_soon(to);
		ফেরত;
	}

	যদি (to->dticks == অঙ্ক_SOON_LISTS) {
		timeout_wheel_append_soon_defer(to);
		ফেরত;
	}

	uint32_t cutoff;

	cutoff = ((অঙ্ক_LATER_LISTS + wheel.sifted + 1) *
		  অঙ্ক_SOON_LISTS) - wheel.soon_সূচক - 1;
	যদি (to->dticks <= cutoff) {
		timeout_wheel_append_later(to);
		ফেরত;
	}

	timeout_wheel_ঢোকাও_distant(to, cutoff);
}

/* Remove the specified timeout. */
স্থির শূন্য timeout_wheel_remove_timeout(কাঠামো _timeout *to)
{
	sys_dlist_remove(&to->গ্রন্থি);

	যদি (to->flags & _TIMEOUT_FLAG_TIMER_WHEEL_SOON) {
		unsigned পূর্ণসংখ্যা সূচক = to->dticks;

		/* soon_defer entries carry index + NUM_SOON_LISTS and own no
		 * soon_bits bit; only true soon-list entries do.
		 */
		যদি (সূচক < অঙ্ক_SOON_LISTS &&
		    sys_dlist_is_empty(&wheel.soon[সূচক])) {
			wheel.soon_bits &= ~BIT(সূচক);
		}
	}

	timeout_wheel_clear_flags(to);
}

/* Calculate the number of ticks until the specified timeout expires. */
স্থির k_ticks_t timeout_wheel_remainder(ধ্রুবক কাঠামো _timeout *to)
{
	যদি (to->flags & _TIMEOUT_FLAG_TIMER_WHEEL_SOON) {
		uint32_t now = wheel.soon_সূচক;
		uint32_t idx = to->dticks;

		যদি (idx == now) {
			ফেরত (1U - wheel.announcing) * অঙ্ক_SOON_LISTS;
		} অন্যথা যদি (idx >= অঙ্ক_SOON_LISTS) {
			/* Special case if querying item in 'soon_defer' list */
			ফেরত অঙ্ক_SOON_LISTS;
		}

		ফেরত (idx - now) & (অঙ্ক_SOON_LISTS - 1U);
	}

	যদি (to->flags & _TIMEOUT_FLAG_TIMER_WHEEL_LATER) {
		uint32_t later_slot = to->dticks / অঙ্ক_SOON_LISTS;
		uint32_t soon_সূচক = to->dticks & (অঙ্ক_SOON_LISTS - 1U);

		/* later slots ahead until the bucket is sifted */
		uint32_t slots_ahead = ((later_slot - wheel.later_সূচক) &
					(অঙ্ক_LATER_LISTS - 1U)) + wheel.sifted;

		ফেরত (k_ticks_t)((অঙ্ক_SOON_LISTS - wheel.soon_সূচক) +
				   (slots_ahead * অঙ্ক_SOON_LISTS) + soon_সূচক);
	}

	কাঠামো _timeout *t;
	k_ticks_t remaining;

	remaining  = ((অঙ্ক_LATER_LISTS + wheel.sifted + 1) *
		      অঙ্ক_SOON_LISTS) - wheel.soon_সূচক - 1;

	জন্য (t = timeout_wheel_first_in_list(&wheel.distant);
	     t != ফাঁকা; t = timeout_wheel_পরবর্তী_distant(t)) {
		remaining += t->dticks;
		যদি (t == to) {
			বিরতি;
		}
	}

	ফেরত remaining;
}

/*
 * Sift from a 'later' list down into the appropriate 'soon' lists. Timeouts
 * are prepended to the 'soon' list to preserve time ordering.
 */
স্থির শূন্য timeout_wheel_sift_down_to_soon(শূন্য)
{
	sys_dlist_t *list;
	কাঠামো _timeout *t;

	list = &wheel.later[wheel.later_সূচক];
	যখন ((t = timeout_wheel_last_in_list(list)) != ফাঁকা) {
		sys_dlist_remove(&t->গ্রন্থি);
		timeout_wheel_clear_flags(t);
		timeout_wheel_prepend_soon(t);
	}
}

/*
 * Sift from the 'distant' list down into the appropriate 'later' list. Two
 * steps: prepare (massage dticks/flags of the leading distant timeouts) then
 * move them en masse into the 'later' list.
 */
স্থির শূন্য timeout_wheel_sift_down_to_later(শূন্য)
{
	কাঠামো _timeout *t;
	int64_t sift_ticks = অঙ্ক_SOON_LISTS;
	কাঠামো _timeout *শীর্ষ = timeout_wheel_first_in_list(&wheel.distant);
	কাঠামো _timeout *লেজ = ফাঁকা;
	পূর্ণসংখ্যা new_dticks = wheel.later_সূচক * অঙ্ক_SOON_LISTS;

	জন্য (t = শীর্ষ; t != ফাঁকা; t = timeout_wheel_পরবর্তী_distant(t)) {
		যদি (t->dticks >= sift_ticks) {
			t->dticks -= sift_ticks;
			বিরতি;
		}

		sift_ticks -= t->dticks;
		new_dticks += t->dticks;

		t->flags |= _TIMEOUT_FLAG_TIMER_WHEEL_LATER;
		t->dticks = new_dticks;
		লেজ = t;
	}

	যদি (লেজ != ফাঁকা) {
		sys_dlist_range_prepend(&wheel.later[wheel.later_সূচক],
					&শীর্ষ->গ্রন্থি, &লেজ->গ্রন্থি);
	}
}

/*
 * Sifting runs every NUM_SOON_LISTS ticks: first move the current 'later' list
 * into the 'soon' lists, freeing it to receive the next batch sifted from
 * 'distant'.
 */
স্থির শূন্য timeout_wheel_sift_down(শূন্য)
{
	timeout_wheel_sift_down_to_soon();

	timeout_wheel_sift_down_to_later();

	/* Advance to the next 'later' list */
	wheel.later_সূচক++;
	wheel.later_সূচক &= (অঙ্ক_LATER_LISTS - 1);
	wheel.sifted = 1;
}

/*
 * Announce all the timeouts expiring "now" (at soon_index). curr_tick has been
 * advanced. Handlers fire with the lock dropped; inflight_timeout tracks
 * the in-flight timeout so aborts race correctly.
 */
স্থির k_spinlock_চাবি_t timeout_wheel_announce_now(k_spinlock_চাবি_t চাবি)
{
	কাঠামো _timeout *t;
	uint32_t সূচক = wheel.soon_সূচক;

	wheel.sifted = 0;
	wheel.announcing = 1;
	জন্য (t = timeout_wheel_first_in_list(&wheel.soon[সূচক]);
	     t != ফাঁকা;
	     t = timeout_wheel_first_in_list(&wheel.soon[সূচক])) {
		sys_dlist_remove(&t->গ্রন্থি);
		timeout_wheel_clear_flags(t);

		inflight_timeout = t;
		k_spin_unlock(&timeout_lock, চাবি);
		t->fn(t);
		চাবি = k_spin_lock(&timeout_lock);
		inflight_timeout = ফাঁকা;
	}
	wheel.announcing = 0;

	/*
	 * The 'soon_index' slot is now drained, so 'soon deferred' timeouts
	 * (exactly NUM_SOON_LISTS into the future) can move into their slots.
	 */
	জন্য (t = timeout_wheel_first_in_list(&wheel.soon_defer);
	     t != ফাঁকা;
	     t = timeout_wheel_first_in_list(&wheel.soon_defer)) {
		sys_dlist_remove(&t->গ্রন্থি);
		/* Correct the dticks to be the appropriate 'soon' list index */
		t->dticks &= (অঙ্ক_SOON_LISTS - 1);
		wheel.soon_bits |= BIT(t->dticks);
		sys_dlist_append(&wheel.soon[t->dticks], &t->গ্রন্থি);
	}

	যদি (সূচক == (অঙ্ক_SOON_LISTS - 1)) {
		timeout_wheel_sift_down();
	}

	ফেরত চাবি;
}

/*
 * Backend interface (see kernel/timeout.c).
 */

স্থির bool z_timeout_q_ঢোকাও(কাঠামো _timeout *to, k_ticks_t dticks)
{
	to->dticks = dticks;
	timeout_wheel_add_timeout(to);

	ফেরত to == timeout_wheel_first();
}

স্থির bool z_timeout_q_remove(কাঠামো _timeout *to)
{
	bool was_first = (to == timeout_wheel_first());

	timeout_wheel_remove_timeout(to);

	ফেরত was_first;
}

স্থির k_ticks_t z_timeout_q_remainder(ধ্রুবক কাঠামো _timeout *to)
{
	ফেরত timeout_wheel_remainder(to);
}

/*
 * Ticks from curr_tick to the next 'soon' timeout, between 1 and
 * NUM_SOON_LISTS (the 'soon_index' list is excluded as it is timing out).
 * Never K_TICKS_FOREVER: a sift is always pending, which is why the wheel
 * wakes a tickless-idle CPU at least every NUM_SOON_LISTS ticks. The
 * announce-range cap is applied centrally by next_timeout() in timeout.c.
 */
স্থির k_ticks_t z_timeout_q_পরবর্তী_expiry(শূন্য)
{
	কাঠামো _timeout *to = timeout_wheel_first();
	int64_t dticks = (to != ফাঁকা) ? to->dticks : (অঙ্ক_SOON_LISTS - 1);

	dticks = ((অঙ্ক_SOON_LISTS - 1 - wheel.soon_সূচক + dticks) &
		  (অঙ্ক_SOON_LISTS - 1)) + 1;

	ফেরত (k_ticks_t)dticks;
}

/* Backend-owned announce loop (see _TIMEOUT_BACKEND_OWNS_ANNOUNCE). Drains all
 * timeouts due within announce_remaining ticks, firing each handler with the
 * subsystem lock dropped; advances curr_tick and announce_remaining. Returns
 * the (possibly re-acquired) spinlock key.
 */
স্থির k_spinlock_চাবি_t z_timeout_q_announce(k_spinlock_চাবি_t চাবি)
{
	uint32_t mask;
	পূর্ণসংখ্যা      পরবর্তী;    /* Index of next 'soon' list to expire */
	পূর্ণসংখ্যা      অঙ্ক_ticks_to_advance;
	পূর্ণসংখ্যা      correction;  /* NUM_SOON_LISTS or 0 */
	uint32_t bits;

	যখন (announce_remaining > 0) {

		mask = 0xFFFFFFFF << ((wheel.soon_সূচক + 1) & (অঙ্ক_SOON_LISTS - 1));
		bits = wheel.soon_bits | BIT(অঙ্ক_SOON_LISTS - 1);
		পরবর্তী = u32_গুনতি_trailing_zeros(mask & bits);
		correction = ((wheel.soon_সূচক + 1) / অঙ্ক_SOON_LISTS) * অঙ্ক_SOON_LISTS;
		অঙ্ক_ticks_to_advance = পরবর্তী + correction - wheel.soon_সূচক;

		যদি (অঙ্ক_ticks_to_advance > announce_remaining) {
			curr_tick += announce_remaining;
			wheel.sifted = 0;
			wheel.soon_সূচক += announce_remaining;
			wheel.soon_সূচক &= (অঙ্ক_SOON_LISTS - 1);

			announce_remaining = 0;
			বিরতি;
		}

		/*
		 * 1. Advance to the next tick to process.
		 * 2. Clear its bit in the 'soon_bits'.
		 */
		wheel.soon_সূচক += অঙ্ক_ticks_to_advance;
		wheel.soon_সূচক &= (অঙ্ক_SOON_LISTS - 1);
		wheel.soon_bits &= ~BIT(wheel.soon_সূচক);

		curr_tick += অঙ্ক_ticks_to_advance;
		চাবি = timeout_wheel_announce_now(চাবি);
		announce_remaining -= অঙ্ক_ticks_to_advance;
	}

	ফেরত চাবি;
}

#endif /* ZEPHYR_KERNEL_TIMEOUT_WHEEL_H_ */
