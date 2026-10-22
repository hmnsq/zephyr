/*
 * Copyright (c) 2018 Intel Corporation
 * Copyright (c) 2026 Peter Mitsis
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#جے_بنا_تعریف ZEPHYR_KERNEL_TIMEOUT_WHEEL_H_
#تعریف_کرو ZEPHYR_KERNEL_TIMEOUT_WHEEL_H_

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

#ضمن <zephyr/init.h>
#ضمن <zephyr/sys/dlist.h>

/* Per-node flag bits (struct _timeout's flags field): which tier the timeout
 * currently lives in, so removal can maintain the soon-list occupancy bitmap.
 */
#تعریف_کرو _TIMEOUT_FLAG_TIMER_WHEEL_SOON  ثنائی_ہندسہ(0)
#تعریف_کرو _TIMEOUT_FLAG_TIMER_WHEEL_LATER ثنائی_ہندسہ(1)

/* This backend supplies its own announce loop. */
#تعریف_کرو _TIMEOUT_BACKEND_OWNS_ANNOUNCE 1

#تعریف_کرو عدد_SOON_LISTS  32
#تعریف_کرو عدد_LATER_LISTS 32

بنية timeout_wheel {
	sys_dlist_t soon[عدد_SOON_LISTS];   /* timeouts expiring soon lists */
	sys_dlist_t later[عدد_LATER_LISTS]; /* timeouts expiring later lists */
	sys_dlist_t distant;                /* timeouts expiring far in future */
	sys_dlist_t soon_defer;             /* special soon list */
	uint32_t    soon_بٹ;              /* Set bit indicates list in use */
	uint32_t    soon_اشاریہ;             /* Current soon list index */
	uint32_t    later_اشاریہ;
	uint32_t    announcing;             /* 1: Announcing soon_index, else 0 */
	uint32_t    sifted;                 /* 1: Sifted, else 0 */
};

ساكن بنية timeout_wheel wheel;

ساكن صحيح timeout_wheel_init(فراغ)
{
	بنانشانی صحيح i;

	لكل (i = 0; i < عدد_SOON_LISTS; i++) {
		sys_dlist_init(&wheel.soon[i]);
	}

	لكل (i = 0; i < عدد_LATER_LISTS; i++) {
		sys_dlist_init(&wheel.later[i]);
	}

	sys_dlist_init(&wheel.distant);
	sys_dlist_init(&wheel.soon_defer);

	wheel.soon_بٹ = 0;
	wheel.soon_اشاریہ = عدد_SOON_LISTS - 1;
	wheel.later_اشاریہ = 0;
	wheel.announcing = 0;
	wheel.sifted = 1;

	أرجع 0;
}

/* Initialize the wheel before the timer driver (PRE_KERNEL_2) and before any
 * timeout can be added. Priority 0 keeps it ahead of other PRE_KERNEL_1 init.
 */
SYS_INIT(timeout_wheel_init, PRE_KERNEL_1, 0);

ساكن مضمن فراغ timeout_wheel_clear_flags(بنية _timeout *منزل)
{
	منزل->flags &= ~(_TIMEOUT_FLAG_TIMER_WHEEL_SOON |
		       _TIMEOUT_FLAG_TIMER_WHEEL_LATER);
}

/* Return the last item in the specified list, or NULL if the list is empty. */
ساكن بنية _timeout *timeout_wheel_آخری_وچ_list(sys_dlist_t *list)
{
	sys_dnode_t *t = sys_dlist_peek_پوچھ(list);

	أرجع (t == عدم) ? عدم : CONTAINER_دا(t, بنية _timeout, گرہ);
}

/* Return the first timeout expiring in the specified list, or NULL. */
ساكن بنية _timeout *timeout_wheel_پہلا_وچ_list(sys_dlist_t *list)
{
	sys_dnode_t *t = sys_dlist_peek_سر(list);

	أرجع (t == عدم) ? عدم : CONTAINER_دا(t, بنية _timeout, گرہ);
}

/*
 * Get the first timeout in the wheel from its 'soon' lists. NULL means that
 * the first timeout is the sift operation which always implicitly exists
 * within the final 'soon' list.
 */
ساكن بنية _timeout *timeout_wheel_پہلا(فراغ)
{
	uint32_t bitmap;
	بنانشانی صحيح اشاریہ;
	sys_dnode_t *t;

	bitmap = wheel.soon_بٹ | ثنائی_ہندسہ(عدد_SOON_LISTS - 1);
	إذا (wheel.sifted == 0) {
		bitmap &= ~(ثنائی_ہندسہ(wheel.soon_اشاریہ) - 1);
	}
	اشاریہ = u32_گنتی_trailing_zeros(bitmap);

	t = sys_dlist_peek_سر(&wheel.soon[اشاریہ]);

	أرجع (t == عدم) ? عدم : CONTAINER_دا(t, بنية _timeout, گرہ);
}

/* Return the timeout following @a t expiring "distantly". */
ساكن بنية _timeout *timeout_wheel_اگلا_distant(بنية _timeout *t)
{
	sys_dnode_t *n = sys_dlist_peek_اگلا(&wheel.distant, &t->گرہ);

	أرجع (n == عدم) ? عدم : CONTAINER_دا(n, بنية _timeout, گرہ);
}

/*
 * Prepend a timeout to the appropriate "soon" list. Only called during sifting
 * after the timeout had been removed from a 'later' list, where its 'dticks'
 * field was a composite value. Prepending preserves time ordering.
 */
ساكن فراغ timeout_wheel_prepend_soon(بنية _timeout *منزل)
{
	بنانشانی صحيح اشاریہ = منزل->dticks & (عدد_SOON_LISTS - 1);

	wheel.soon_بٹ |= ثنائی_ہندسہ(اشاریہ);
	sys_dlist_prepend(&wheel.soon[اشاریہ], &منزل->گرہ);
	منزل->flags |= _TIMEOUT_FLAG_TIMER_WHEEL_SOON;
	منزل->dticks = اشاریہ;
}

/*
 * Append a timeout to the appropriate "soon" list, recording the list index
 * in the timeout's 'dticks' field.
 */
ساكن فراغ timeout_wheel_append_soon(بنية _timeout *منزل)
{
	بنانشانی صحيح اشاریہ;

	اشاریہ = (منزل->dticks + wheel.soon_اشاریہ) & (عدد_SOON_LISTS - 1);
	منزل->dticks = اشاریہ;

	wheel.soon_بٹ |= ثنائی_ہندسہ(اشاریہ);
	sys_dlist_append(&wheel.soon[اشاریہ], &منزل->گرہ);
	منزل->flags |= _TIMEOUT_FLAG_TIMER_WHEEL_SOON;
}

/*
 * Append a timeout to the soon deferred list. Special case for timeouts that
 * are exactly NUM_SOON_LISTS into the future while the system is announcing:
 * they must not get mixed with the currently-expiring rotation. They move into
 * the appropriate 'soon' list at the end of the announce.
 */
ساكن فراغ timeout_wheel_append_soon_defer(بنية _timeout *منزل)
{
	بنانشانی صحيح اشاریہ;

	اشاریہ = (منزل->dticks + wheel.soon_اشاریہ) & (عدد_SOON_LISTS - 1);
	منزل->dticks = اشاریہ + عدد_SOON_LISTS;

	sys_dlist_append(&wheel.soon_defer, &منزل->گرہ);
	منزل->flags |= _TIMEOUT_FLAG_TIMER_WHEEL_SOON;
}

/*
 * Append a timeout to the appropriate "later" list. The 'dticks' field is set
 * to identify both the eventual 'soon' list and the 'later' list it goes to.
 */
ساكن فراغ timeout_wheel_append_later(بنية _timeout *منزل)
{
	uint32_t position;
	uint32_t soon;
	uint32_t later;

	position = منزل->dticks + wheel.soon_اشاریہ - عدد_SOON_LISTS;

	soon = position & (عدد_SOON_LISTS - 1);
	later = ((position / عدد_SOON_LISTS) + wheel.later_اشاریہ -
		 wheel.sifted) & (عدد_LATER_LISTS - 1);

	منزل->dticks = (later * عدد_SOON_LISTS) + soon;
	منزل->flags |= _TIMEOUT_FLAG_TIMER_WHEEL_LATER;

	sys_dlist_append(&wheel.later[later], &منزل->گرہ);
}

/*
 * Insert a timeout into the "distant" list, an expiry-ordered delta list. For
 * good performance the distant list should stay short.
 */
ساكن فراغ timeout_wheel_داخل_کرو_distant(بنية _timeout *منزل, uint32_t cutoff)
{
	بنية _timeout *t;

	منزل->dticks -= cutoff;

	لكل (t = timeout_wheel_پہلا_وچ_list(&wheel.distant);
	     t != عدم; t = timeout_wheel_اگلا_distant(t)) {
		إذا (t->dticks > منزل->dticks) {
			t->dticks -= منزل->dticks;
			sys_dlist_داخل_کرو(&t->گرہ, &منزل->گرہ);
			اكسر;
		}
		منزل->dticks -= t->dticks;
	}

	إذا (t == عدم) {
		sys_dlist_append(&wheel.distant, &منزل->گرہ);
	}
}

/*
 * Add a timeout to the wheel (with to->dticks already set to its tick delta
 * from curr_tick). It is never placed into the 'soon_index' list.
 */
ساكن فراغ timeout_wheel_جوڑو_timeout(بنية _timeout *منزل)
{
	__ASSERT_NO_MSG(منزل->dticks != 0);

	إذا (منزل->dticks <= (عدد_SOON_LISTS - wheel.announcing)) {
		timeout_wheel_append_soon(منزل);
		أرجع;
	}

	إذا (منزل->dticks == عدد_SOON_LISTS) {
		timeout_wheel_append_soon_defer(منزل);
		أرجع;
	}

	uint32_t cutoff;

	cutoff = ((عدد_LATER_LISTS + wheel.sifted + 1) *
		  عدد_SOON_LISTS) - wheel.soon_اشاریہ - 1;
	إذا (منزل->dticks <= cutoff) {
		timeout_wheel_append_later(منزل);
		أرجع;
	}

	timeout_wheel_داخل_کرو_distant(منزل, cutoff);
}

/* Remove the specified timeout. */
ساكن فراغ timeout_wheel_remove_timeout(بنية _timeout *منزل)
{
	sys_dlist_remove(&منزل->گرہ);

	إذا (منزل->flags & _TIMEOUT_FLAG_TIMER_WHEEL_SOON) {
		بنانشانی صحيح اشاریہ = منزل->dticks;

		/* soon_defer entries carry index + NUM_SOON_LISTS and own no
		 * soon_bits bit; only true soon-list entries do.
		 */
		إذا (اشاریہ < عدد_SOON_LISTS &&
		    sys_dlist_is_خالی_تھاواں(&wheel.soon[اشاریہ])) {
			wheel.soon_بٹ &= ~ثنائی_ہندسہ(اشاریہ);
		}
	}

	timeout_wheel_clear_flags(منزل);
}

/* Calculate the number of ticks until the specified timeout expires. */
ساكن k_ticks_t timeout_wheel_remainder(ثابت بنية _timeout *منزل)
{
	إذا (منزل->flags & _TIMEOUT_FLAG_TIMER_WHEEL_SOON) {
		uint32_t now = wheel.soon_اشاریہ;
		uint32_t idx = منزل->dticks;

		إذا (idx == now) {
			أرجع (1U - wheel.announcing) * عدد_SOON_LISTS;
		} وإلا إذا (idx >= عدد_SOON_LISTS) {
			/* Special case if querying item in 'soon_defer' list */
			أرجع عدد_SOON_LISTS;
		}

		أرجع (idx - now) & (عدد_SOON_LISTS - 1U);
	}

	إذا (منزل->flags & _TIMEOUT_FLAG_TIMER_WHEEL_LATER) {
		uint32_t later_تھاں = منزل->dticks / عدد_SOON_LISTS;
		uint32_t soon_اشاریہ = منزل->dticks & (عدد_SOON_LISTS - 1U);

		/* later slots ahead until the bucket is sifted */
		uint32_t slots_ahead = ((later_تھاں - wheel.later_اشاریہ) &
					(عدد_LATER_LISTS - 1U)) + wheel.sifted;

		أرجع (k_ticks_t)((عدد_SOON_LISTS - wheel.soon_اشاریہ) +
				   (slots_ahead * عدد_SOON_LISTS) + soon_اشاریہ);
	}

	بنية _timeout *t;
	k_ticks_t بچیا;

	بچیا  = ((عدد_LATER_LISTS + wheel.sifted + 1) *
		      عدد_SOON_LISTS) - wheel.soon_اشاریہ - 1;

	لكل (t = timeout_wheel_پہلا_وچ_list(&wheel.distant);
	     t != عدم; t = timeout_wheel_اگلا_distant(t)) {
		بچیا += t->dticks;
		إذا (t == منزل) {
			اكسر;
		}
	}

	أرجع بچیا;
}

/*
 * Sift from a 'later' list down into the appropriate 'soon' lists. Timeouts
 * are prepended to the 'soon' list to preserve time ordering.
 */
ساكن فراغ timeout_wheel_چھانو_تھلے_منزل_soon(فراغ)
{
	sys_dlist_t *list;
	بنية _timeout *t;

	list = &wheel.later[wheel.later_اشاریہ];
	طالما ((t = timeout_wheel_آخری_وچ_list(list)) != عدم) {
		sys_dlist_remove(&t->گرہ);
		timeout_wheel_clear_flags(t);
		timeout_wheel_prepend_soon(t);
	}
}

/*
 * Sift from the 'distant' list down into the appropriate 'later' list. Two
 * steps: prepare (massage dticks/flags of the leading distant timeouts) then
 * move them en masse into the 'later' list.
 */
ساكن فراغ timeout_wheel_چھانو_تھلے_منزل_later(فراغ)
{
	بنية _timeout *t;
	int64_t چھانو_ticks = عدد_SOON_LISTS;
	بنية _timeout *سر = timeout_wheel_پہلا_وچ_list(&wheel.distant);
	بنية _timeout *پوچھ = عدم;
	صحيح new_dticks = wheel.later_اشاریہ * عدد_SOON_LISTS;

	لكل (t = سر; t != عدم; t = timeout_wheel_اگلا_distant(t)) {
		إذا (t->dticks >= چھانو_ticks) {
			t->dticks -= چھانو_ticks;
			اكسر;
		}

		چھانو_ticks -= t->dticks;
		new_dticks += t->dticks;

		t->flags |= _TIMEOUT_FLAG_TIMER_WHEEL_LATER;
		t->dticks = new_dticks;
		پوچھ = t;
	}

	إذا (پوچھ != عدم) {
		sys_dlist_range_prepend(&wheel.later[wheel.later_اشاریہ],
					&سر->گرہ, &پوچھ->گرہ);
	}
}

/*
 * Sifting runs every NUM_SOON_LISTS ticks: first move the current 'later' list
 * into the 'soon' lists, freeing it to receive the next batch sifted from
 * 'distant'.
 */
ساكن فراغ timeout_wheel_چھانو_تھلے(فراغ)
{
	timeout_wheel_چھانو_تھلے_منزل_soon();

	timeout_wheel_چھانو_تھلے_منزل_later();

	/* Advance to the next 'later' list */
	wheel.later_اشاریہ++;
	wheel.later_اشاریہ &= (عدد_LATER_LISTS - 1);
	wheel.sifted = 1;
}

/*
 * Announce all the timeouts expiring "now" (at soon_index). curr_tick has been
 * advanced. Handlers fire with the lock dropped; inflight_timeout tracks
 * the in-flight timeout so aborts race correctly.
 */
ساكن k_spinlock_کلید_t timeout_wheel_announce_now(k_spinlock_کلید_t کلید)
{
	بنية _timeout *t;
	uint32_t اشاریہ = wheel.soon_اشاریہ;

	wheel.sifted = 0;
	wheel.announcing = 1;
	لكل (t = timeout_wheel_پہلا_وچ_list(&wheel.soon[اشاریہ]);
	     t != عدم;
	     t = timeout_wheel_پہلا_وچ_list(&wheel.soon[اشاریہ])) {
		sys_dlist_remove(&t->گرہ);
		timeout_wheel_clear_flags(t);

		inflight_timeout = t;
		k_spin_unlock(&timeout_جندرا, کلید);
		t->fn(t);
		کلید = k_spin_جندرا(&timeout_جندرا);
		inflight_timeout = عدم;
	}
	wheel.announcing = 0;

	/*
	 * The 'soon_index' slot is now drained, so 'soon deferred' timeouts
	 * (exactly NUM_SOON_LISTS into the future) can move into their slots.
	 */
	لكل (t = timeout_wheel_پہلا_وچ_list(&wheel.soon_defer);
	     t != عدم;
	     t = timeout_wheel_پہلا_وچ_list(&wheel.soon_defer)) {
		sys_dlist_remove(&t->گرہ);
		/* Correct the dticks to be the appropriate 'soon' list index */
		t->dticks &= (عدد_SOON_LISTS - 1);
		wheel.soon_بٹ |= ثنائی_ہندسہ(t->dticks);
		sys_dlist_append(&wheel.soon[t->dticks], &t->گرہ);
	}

	إذا (اشاریہ == (عدد_SOON_LISTS - 1)) {
		timeout_wheel_چھانو_تھلے();
	}

	أرجع کلید;
}

/*
 * Backend interface (see kernel/timeout.c).
 */

ساكن bool z_timeout_q_داخل_کرو(بنية _timeout *منزل, k_ticks_t dticks)
{
	منزل->dticks = dticks;
	timeout_wheel_جوڑو_timeout(منزل);

	أرجع منزل == timeout_wheel_پہلا();
}

ساكن bool z_timeout_q_remove(بنية _timeout *منزل)
{
	bool was_پہلا = (منزل == timeout_wheel_پہلا());

	timeout_wheel_remove_timeout(منزل);

	أرجع was_پہلا;
}

ساكن k_ticks_t z_timeout_q_remainder(ثابت بنية _timeout *منزل)
{
	أرجع timeout_wheel_remainder(منزل);
}

/*
 * Ticks from curr_tick to the next 'soon' timeout, between 1 and
 * NUM_SOON_LISTS (the 'soon_index' list is excluded as it is timing out).
 * Never K_TICKS_FOREVER: a sift is always pending, which is why the wheel
 * wakes a tickless-idle CPU at least every NUM_SOON_LISTS ticks. The
 * announce-range cap is applied centrally by next_timeout() in timeout.c.
 */
ساكن k_ticks_t z_timeout_q_اگلا_expiry(فراغ)
{
	بنية _timeout *منزل = timeout_wheel_پہلا();
	int64_t dticks = (منزل != عدم) ? منزل->dticks : (عدد_SOON_LISTS - 1);

	dticks = ((عدد_SOON_LISTS - 1 - wheel.soon_اشاریہ + dticks) &
		  (عدد_SOON_LISTS - 1)) + 1;

	أرجع (k_ticks_t)dticks;
}

/* Backend-owned announce loop (see _TIMEOUT_BACKEND_OWNS_ANNOUNCE). Drains all
 * timeouts due within announce_remaining ticks, firing each handler with the
 * subsystem lock dropped; advances curr_tick and announce_remaining. Returns
 * the (possibly re-acquired) spinlock key.
 */
ساكن k_spinlock_کلید_t z_timeout_q_announce(k_spinlock_کلید_t کلید)
{
	uint32_t نقاب;
	صحيح      اگلا;    /* Index of next 'soon' list to expire */
	صحيح      عدد_ticks_منزل_advance;
	صحيح      correction;  /* NUM_SOON_LISTS or 0 */
	uint32_t بٹ;

	طالما (announce_بچیا > 0) {

		نقاب = 0xFFFFFFFF << ((wheel.soon_اشاریہ + 1) & (عدد_SOON_LISTS - 1));
		بٹ = wheel.soon_بٹ | ثنائی_ہندسہ(عدد_SOON_LISTS - 1);
		اگلا = u32_گنتی_trailing_zeros(نقاب & بٹ);
		correction = ((wheel.soon_اشاریہ + 1) / عدد_SOON_LISTS) * عدد_SOON_LISTS;
		عدد_ticks_منزل_advance = اگلا + correction - wheel.soon_اشاریہ;

		إذا (عدد_ticks_منزل_advance > announce_بچیا) {
			curr_tick += announce_بچیا;
			wheel.sifted = 0;
			wheel.soon_اشاریہ += announce_بچیا;
			wheel.soon_اشاریہ &= (عدد_SOON_LISTS - 1);

			announce_بچیا = 0;
			اكسر;
		}

		/*
		 * 1. Advance to the next tick to process.
		 * 2. Clear its bit in the 'soon_bits'.
		 */
		wheel.soon_اشاریہ += عدد_ticks_منزل_advance;
		wheel.soon_اشاریہ &= (عدد_SOON_LISTS - 1);
		wheel.soon_بٹ &= ~ثنائی_ہندسہ(wheel.soon_اشاریہ);

		curr_tick += عدد_ticks_منزل_advance;
		کلید = timeout_wheel_announce_now(کلید);
		announce_بچیا -= عدد_ticks_منزل_advance;
	}

	أرجع کلید;
}

#جے_انت /* ZEPHYR_KERNEL_TIMEOUT_WHEEL_H_ */
