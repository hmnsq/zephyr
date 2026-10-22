/*
 * Copyright (c) 2018 Intel Corporation
 * Copyright (c) 2026 Peter Mitsis
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#جيڪڏهن_وصف_نه_ٿيل ZEPHYR_KERNEL_TIMEOUT_WHEEL_H_
#وصف_ڪريو ZEPHYR_KERNEL_TIMEOUT_WHEEL_H_

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
#وصف_ڪريو _TIMEOUT_FLAG_TIMER_WHEEL_SOON  ٻٽو_انگ(0)
#وصف_ڪريو _TIMEOUT_FLAG_TIMER_WHEEL_LATER ٻٽو_انگ(1)

/* This backend supplies its own announce loop. */
#وصف_ڪريو _TIMEOUT_BACKEND_OWNS_ANNOUNCE 1

#وصف_ڪريو انگ_SOON_LISTS  32
#وصف_ڪريو انگ_LATER_LISTS 32

بنية timeout_wheel {
	sys_dlist_t soon[انگ_SOON_LISTS];   /* timeouts expiring soon lists */
	sys_dlist_t later[انگ_LATER_LISTS]; /* timeouts expiring later lists */
	sys_dlist_t distant;                /* timeouts expiring far in future */
	sys_dlist_t soon_defer;             /* special soon list */
	uint32_t    soon_بٽ;              /* Set bit indicates list in use */
	uint32_t    soon_اشاريو;             /* Current soon list index */
	uint32_t    later_اشاريو;
	uint32_t    announcing;             /* 1: Announcing soon_index, else 0 */
	uint32_t    sifted;                 /* 1: Sifted, else 0 */
};

ساكن بنية timeout_wheel wheel;

ساكن صحيح timeout_wheel_init(فراغ)
{
	بنانشان صحيح i;

	لكل (i = 0; i < انگ_SOON_LISTS; i++) {
		sys_dlist_init(&wheel.soon[i]);
	}

	لكل (i = 0; i < انگ_LATER_LISTS; i++) {
		sys_dlist_init(&wheel.later[i]);
	}

	sys_dlist_init(&wheel.distant);
	sys_dlist_init(&wheel.soon_defer);

	wheel.soon_بٽ = 0;
	wheel.soon_اشاريو = انگ_SOON_LISTS - 1;
	wheel.later_اشاريو = 0;
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
ساكن بنية _timeout *timeout_wheel_آخري_اندريون_list(sys_dlist_t *list)
{
	sys_dnode_t *t = sys_dlist_peek_پڇ(list);

	أرجع (t == عدم) ? عدم : CONTAINER_جو(t, بنية _timeout, ڳنڍ);
}

/* Return the first timeout expiring in the specified list, or NULL. */
ساكن بنية _timeout *timeout_wheel_پهريون_اندريون_list(sys_dlist_t *list)
{
	sys_dnode_t *t = sys_dlist_peek_سر(list);

	أرجع (t == عدم) ? عدم : CONTAINER_جو(t, بنية _timeout, ڳنڍ);
}

/*
 * Get the first timeout in the wheel from its 'soon' lists. NULL means that
 * the first timeout is the sift operation which always implicitly exists
 * within the final 'soon' list.
 */
ساكن بنية _timeout *timeout_wheel_پهريون(فراغ)
{
	uint32_t bitmap;
	بنانشان صحيح اشاريو;
	sys_dnode_t *t;

	bitmap = wheel.soon_بٽ | ٻٽو_انگ(انگ_SOON_LISTS - 1);
	إذا (wheel.sifted == 0) {
		bitmap &= ~(ٻٽو_انگ(wheel.soon_اشاريو) - 1);
	}
	اشاريو = u32_ڳڻپ_trailing_zeros(bitmap);

	t = sys_dlist_peek_سر(&wheel.soon[اشاريو]);

	أرجع (t == عدم) ? عدم : CONTAINER_جو(t, بنية _timeout, ڳنڍ);
}

/* Return the timeout following @a t expiring "distantly". */
ساكن بنية _timeout *timeout_wheel_ايندڙ_distant(بنية _timeout *t)
{
	sys_dnode_t *n = sys_dlist_peek_ايندڙ(&wheel.distant, &t->ڳنڍ);

	أرجع (n == عدم) ? عدم : CONTAINER_جو(n, بنية _timeout, ڳنڍ);
}

/*
 * Prepend a timeout to the appropriate "soon" list. Only called during sifting
 * after the timeout had been removed from a 'later' list, where its 'dticks'
 * field was a composite value. Prepending preserves time ordering.
 */
ساكن فراغ timeout_wheel_prepend_soon(بنية _timeout *منزل)
{
	بنانشان صحيح اشاريو = منزل->dticks & (انگ_SOON_LISTS - 1);

	wheel.soon_بٽ |= ٻٽو_انگ(اشاريو);
	sys_dlist_prepend(&wheel.soon[اشاريو], &منزل->ڳنڍ);
	منزل->flags |= _TIMEOUT_FLAG_TIMER_WHEEL_SOON;
	منزل->dticks = اشاريو;
}

/*
 * Append a timeout to the appropriate "soon" list, recording the list index
 * in the timeout's 'dticks' field.
 */
ساكن فراغ timeout_wheel_append_soon(بنية _timeout *منزل)
{
	بنانشان صحيح اشاريو;

	اشاريو = (منزل->dticks + wheel.soon_اشاريو) & (انگ_SOON_LISTS - 1);
	منزل->dticks = اشاريو;

	wheel.soon_بٽ |= ٻٽو_انگ(اشاريو);
	sys_dlist_append(&wheel.soon[اشاريو], &منزل->ڳنڍ);
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
	بنانشان صحيح اشاريو;

	اشاريو = (منزل->dticks + wheel.soon_اشاريو) & (انگ_SOON_LISTS - 1);
	منزل->dticks = اشاريو + انگ_SOON_LISTS;

	sys_dlist_append(&wheel.soon_defer, &منزل->ڳنڍ);
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

	position = منزل->dticks + wheel.soon_اشاريو - انگ_SOON_LISTS;

	soon = position & (انگ_SOON_LISTS - 1);
	later = ((position / انگ_SOON_LISTS) + wheel.later_اشاريو -
		 wheel.sifted) & (انگ_LATER_LISTS - 1);

	منزل->dticks = (later * انگ_SOON_LISTS) + soon;
	منزل->flags |= _TIMEOUT_FLAG_TIMER_WHEEL_LATER;

	sys_dlist_append(&wheel.later[later], &منزل->ڳنڍ);
}

/*
 * Insert a timeout into the "distant" list, an expiry-ordered delta list. For
 * good performance the distant list should stay short.
 */
ساكن فراغ timeout_wheel_وجهو_distant(بنية _timeout *منزل, uint32_t cutoff)
{
	بنية _timeout *t;

	منزل->dticks -= cutoff;

	لكل (t = timeout_wheel_پهريون_اندريون_list(&wheel.distant);
	     t != عدم; t = timeout_wheel_ايندڙ_distant(t)) {
		إذا (t->dticks > منزل->dticks) {
			t->dticks -= منزل->dticks;
			sys_dlist_وجهو(&t->ڳنڍ, &منزل->ڳنڍ);
			اكسر;
		}
		منزل->dticks -= t->dticks;
	}

	إذا (t == عدم) {
		sys_dlist_append(&wheel.distant, &منزل->ڳنڍ);
	}
}

/*
 * Add a timeout to the wheel (with to->dticks already set to its tick delta
 * from curr_tick). It is never placed into the 'soon_index' list.
 */
ساكن فراغ timeout_wheel_جوڙيو_timeout(بنية _timeout *منزل)
{
	__ASSERT_NO_MSG(منزل->dticks != 0);

	إذا (منزل->dticks <= (انگ_SOON_LISTS - wheel.announcing)) {
		timeout_wheel_append_soon(منزل);
		أرجع;
	}

	إذا (منزل->dticks == انگ_SOON_LISTS) {
		timeout_wheel_append_soon_defer(منزل);
		أرجع;
	}

	uint32_t cutoff;

	cutoff = ((انگ_LATER_LISTS + wheel.sifted + 1) *
		  انگ_SOON_LISTS) - wheel.soon_اشاريو - 1;
	إذا (منزل->dticks <= cutoff) {
		timeout_wheel_append_later(منزل);
		أرجع;
	}

	timeout_wheel_وجهو_distant(منزل, cutoff);
}

/* Remove the specified timeout. */
ساكن فراغ timeout_wheel_remove_timeout(بنية _timeout *منزل)
{
	sys_dlist_remove(&منزل->ڳنڍ);

	إذا (منزل->flags & _TIMEOUT_FLAG_TIMER_WHEEL_SOON) {
		بنانشان صحيح اشاريو = منزل->dticks;

		/* soon_defer entries carry index + NUM_SOON_LISTS and own no
		 * soon_bits bit; only true soon-list entries do.
		 */
		إذا (اشاريو < انگ_SOON_LISTS &&
		    sys_dlist_is_خالي_جايون(&wheel.soon[اشاريو])) {
			wheel.soon_بٽ &= ~ٻٽو_انگ(اشاريو);
		}
	}

	timeout_wheel_clear_flags(منزل);
}

/* Calculate the number of ticks until the specified timeout expires. */
ساكن k_ticks_t timeout_wheel_remainder(ثابت بنية _timeout *منزل)
{
	إذا (منزل->flags & _TIMEOUT_FLAG_TIMER_WHEEL_SOON) {
		uint32_t now = wheel.soon_اشاريو;
		uint32_t idx = منزل->dticks;

		إذا (idx == now) {
			أرجع (1U - wheel.announcing) * انگ_SOON_LISTS;
		} وإلا إذا (idx >= انگ_SOON_LISTS) {
			/* Special case if querying item in 'soon_defer' list */
			أرجع انگ_SOON_LISTS;
		}

		أرجع (idx - now) & (انگ_SOON_LISTS - 1U);
	}

	إذا (منزل->flags & _TIMEOUT_FLAG_TIMER_WHEEL_LATER) {
		uint32_t later_جڳهه = منزل->dticks / انگ_SOON_LISTS;
		uint32_t soon_اشاريو = منزل->dticks & (انگ_SOON_LISTS - 1U);

		/* later slots ahead until the bucket is sifted */
		uint32_t slots_ahead = ((later_جڳهه - wheel.later_اشاريو) &
					(انگ_LATER_LISTS - 1U)) + wheel.sifted;

		أرجع (k_ticks_t)((انگ_SOON_LISTS - wheel.soon_اشاريو) +
				   (slots_ahead * انگ_SOON_LISTS) + soon_اشاريو);
	}

	بنية _timeout *t;
	k_ticks_t بچيل;

	بچيل  = ((انگ_LATER_LISTS + wheel.sifted + 1) *
		      انگ_SOON_LISTS) - wheel.soon_اشاريو - 1;

	لكل (t = timeout_wheel_پهريون_اندريون_list(&wheel.distant);
	     t != عدم; t = timeout_wheel_ايندڙ_distant(t)) {
		بچيل += t->dticks;
		إذا (t == منزل) {
			اكسر;
		}
	}

	أرجع بچيل;
}

/*
 * Sift from a 'later' list down into the appropriate 'soon' lists. Timeouts
 * are prepended to the 'soon' list to preserve time ordering.
 */
ساكن فراغ timeout_wheel_ڇاڻيو_هيٺ_ڏانهن_منزل_soon(فراغ)
{
	sys_dlist_t *list;
	بنية _timeout *t;

	list = &wheel.later[wheel.later_اشاريو];
	طالما ((t = timeout_wheel_آخري_اندريون_list(list)) != عدم) {
		sys_dlist_remove(&t->ڳنڍ);
		timeout_wheel_clear_flags(t);
		timeout_wheel_prepend_soon(t);
	}
}

/*
 * Sift from the 'distant' list down into the appropriate 'later' list. Two
 * steps: prepare (massage dticks/flags of the leading distant timeouts) then
 * move them en masse into the 'later' list.
 */
ساكن فراغ timeout_wheel_ڇاڻيو_هيٺ_ڏانهن_منزل_later(فراغ)
{
	بنية _timeout *t;
	int64_t ڇاڻيو_ticks = انگ_SOON_LISTS;
	بنية _timeout *سر = timeout_wheel_پهريون_اندريون_list(&wheel.distant);
	بنية _timeout *پڇ = عدم;
	صحيح new_dticks = wheel.later_اشاريو * انگ_SOON_LISTS;

	لكل (t = سر; t != عدم; t = timeout_wheel_ايندڙ_distant(t)) {
		إذا (t->dticks >= ڇاڻيو_ticks) {
			t->dticks -= ڇاڻيو_ticks;
			اكسر;
		}

		ڇاڻيو_ticks -= t->dticks;
		new_dticks += t->dticks;

		t->flags |= _TIMEOUT_FLAG_TIMER_WHEEL_LATER;
		t->dticks = new_dticks;
		پڇ = t;
	}

	إذا (پڇ != عدم) {
		sys_dlist_range_prepend(&wheel.later[wheel.later_اشاريو],
					&سر->ڳنڍ, &پڇ->ڳنڍ);
	}
}

/*
 * Sifting runs every NUM_SOON_LISTS ticks: first move the current 'later' list
 * into the 'soon' lists, freeing it to receive the next batch sifted from
 * 'distant'.
 */
ساكن فراغ timeout_wheel_ڇاڻيو_هيٺ_ڏانهن(فراغ)
{
	timeout_wheel_ڇاڻيو_هيٺ_ڏانهن_منزل_soon();

	timeout_wheel_ڇاڻيو_هيٺ_ڏانهن_منزل_later();

	/* Advance to the next 'later' list */
	wheel.later_اشاريو++;
	wheel.later_اشاريو &= (انگ_LATER_LISTS - 1);
	wheel.sifted = 1;
}

/*
 * Announce all the timeouts expiring "now" (at soon_index). curr_tick has been
 * advanced. Handlers fire with the lock dropped; inflight_timeout tracks
 * the in-flight timeout so aborts race correctly.
 */
ساكن k_spinlock_ڪنجي_t timeout_wheel_announce_now(k_spinlock_ڪنجي_t ڪنجي)
{
	بنية _timeout *t;
	uint32_t اشاريو = wheel.soon_اشاريو;

	wheel.sifted = 0;
	wheel.announcing = 1;
	لكل (t = timeout_wheel_پهريون_اندريون_list(&wheel.soon[اشاريو]);
	     t != عدم;
	     t = timeout_wheel_پهريون_اندريون_list(&wheel.soon[اشاريو])) {
		sys_dlist_remove(&t->ڳنڍ);
		timeout_wheel_clear_flags(t);

		inflight_timeout = t;
		k_spin_unlock(&timeout_تالو, ڪنجي);
		t->fn(t);
		ڪنجي = k_spin_تالو(&timeout_تالو);
		inflight_timeout = عدم;
	}
	wheel.announcing = 0;

	/*
	 * The 'soon_index' slot is now drained, so 'soon deferred' timeouts
	 * (exactly NUM_SOON_LISTS into the future) can move into their slots.
	 */
	لكل (t = timeout_wheel_پهريون_اندريون_list(&wheel.soon_defer);
	     t != عدم;
	     t = timeout_wheel_پهريون_اندريون_list(&wheel.soon_defer)) {
		sys_dlist_remove(&t->ڳنڍ);
		/* Correct the dticks to be the appropriate 'soon' list index */
		t->dticks &= (انگ_SOON_LISTS - 1);
		wheel.soon_بٽ |= ٻٽو_انگ(t->dticks);
		sys_dlist_append(&wheel.soon[t->dticks], &t->ڳنڍ);
	}

	إذا (اشاريو == (انگ_SOON_LISTS - 1)) {
		timeout_wheel_ڇاڻيو_هيٺ_ڏانهن();
	}

	أرجع ڪنجي;
}

/*
 * Backend interface (see kernel/timeout.c).
 */

ساكن bool z_timeout_q_وجهو(بنية _timeout *منزل, k_ticks_t dticks)
{
	منزل->dticks = dticks;
	timeout_wheel_جوڙيو_timeout(منزل);

	أرجع منزل == timeout_wheel_پهريون();
}

ساكن bool z_timeout_q_remove(بنية _timeout *منزل)
{
	bool was_پهريون = (منزل == timeout_wheel_پهريون());

	timeout_wheel_remove_timeout(منزل);

	أرجع was_پهريون;
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
ساكن k_ticks_t z_timeout_q_ايندڙ_expiry(فراغ)
{
	بنية _timeout *منزل = timeout_wheel_پهريون();
	int64_t dticks = (منزل != عدم) ? منزل->dticks : (انگ_SOON_LISTS - 1);

	dticks = ((انگ_SOON_LISTS - 1 - wheel.soon_اشاريو + dticks) &
		  (انگ_SOON_LISTS - 1)) + 1;

	أرجع (k_ticks_t)dticks;
}

/* Backend-owned announce loop (see _TIMEOUT_BACKEND_OWNS_ANNOUNCE). Drains all
 * timeouts due within announce_remaining ticks, firing each handler with the
 * subsystem lock dropped; advances curr_tick and announce_remaining. Returns
 * the (possibly re-acquired) spinlock key.
 */
ساكن k_spinlock_ڪنجي_t z_timeout_q_announce(k_spinlock_ڪنجي_t ڪنجي)
{
	uint32_t نقاب;
	صحيح      ايندڙ;    /* Index of next 'soon' list to expire */
	صحيح      انگ_ticks_منزل_advance;
	صحيح      correction;  /* NUM_SOON_LISTS or 0 */
	uint32_t بٽ;

	طالما (announce_بچيل > 0) {

		نقاب = 0xFFFFFFFF << ((wheel.soon_اشاريو + 1) & (انگ_SOON_LISTS - 1));
		بٽ = wheel.soon_بٽ | ٻٽو_انگ(انگ_SOON_LISTS - 1);
		ايندڙ = u32_ڳڻپ_trailing_zeros(نقاب & بٽ);
		correction = ((wheel.soon_اشاريو + 1) / انگ_SOON_LISTS) * انگ_SOON_LISTS;
		انگ_ticks_منزل_advance = ايندڙ + correction - wheel.soon_اشاريو;

		إذا (انگ_ticks_منزل_advance > announce_بچيل) {
			curr_tick += announce_بچيل;
			wheel.sifted = 0;
			wheel.soon_اشاريو += announce_بچيل;
			wheel.soon_اشاريو &= (انگ_SOON_LISTS - 1);

			announce_بچيل = 0;
			اكسر;
		}

		/*
		 * 1. Advance to the next tick to process.
		 * 2. Clear its bit in the 'soon_bits'.
		 */
		wheel.soon_اشاريو += انگ_ticks_منزل_advance;
		wheel.soon_اشاريو &= (انگ_SOON_LISTS - 1);
		wheel.soon_بٽ &= ~ٻٽو_انگ(wheel.soon_اشاريو);

		curr_tick += انگ_ticks_منزل_advance;
		ڪنجي = timeout_wheel_announce_now(ڪنجي);
		announce_بچيل -= انگ_ticks_منزل_advance;
	}

	أرجع ڪنجي;
}

#جيڪڏهن_پورو /* ZEPHYR_KERNEL_TIMEOUT_WHEEL_H_ */
