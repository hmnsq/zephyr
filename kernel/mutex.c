/*
 * Copyright (c) 2016 Wind River Systems, Inc.
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief mutex kernel services
 *
 * This module contains routines for handling mutex locking and unlocking.
 *
 * Mutexes implement a priority inheritance algorithm that boosts the priority
 * level of the owning thread to match the priority level of the highest
 * priority thread waiting on any mutex it holds.
 *
 * When a thread pends on a contended mutex, the priority boost propagates
 * through the ownership chain: if the mutex owner is itself blocked on
 * another mutex, that owner is also boosted, and so on until the end of
 * the chain, a deadlock cycle is detected, or the walk's hop limit is
 * reached (see Known limitations below).
 *
 * Each thread maintains a list of all mutexes it currently holds
 * (held_mutexes). On unlock, the thread's priority is recalculated by
 * scanning the wait queues of all remaining held mutexes, ensuring the
 * correct priority is maintained when multiple mutexes are involved.
 *
 * Known limitations:
 * - Bounded chain walk: the ownership-chain walk stops after
 *   MUTEX_CHAIN_WALK_MAX_HOPS hops. Owners beyond that hop count keep their
 *   pre-boost priority. This is a deliberate bound on the work done under
 *   mutex_lock, not a claim that chains longer than the limit cannot
 *   legally form.
 * - Deferred priority drop on timeout: when a waiter times out, the owner's
 *   priority is adjusted in the waiter's context after it resumes, not at ISR
 *   level. The owner may run at a slightly higher priority than it should for
 *   a short time, but all threads will eventually run correctly.
 *   Fixing this requires acquiring mutex_lock from the timer handler, which
 *   is not safe: on single-core, mutex_lock disables interrupts and the ISR
 *   would spin forever if a thread holds it; on SMP, it reverses the lock
 *   ordering in k_mutex_unlock (mutex_lock -> _sched_spinlock) and deadlocks.
 * - Chain priority-down not propagated on timeout: when a timeout occurs in a
 *   multi-hop ownership chain, only the immediate owner's priority is adjusted;
 *   deeper owners remain over-boosted until they release their mutexes. The
 *   impact and self-correcting nature are the same as above.
 *   Propagating the drop through the full chain would require a chain walk of
 *   unbounded length in the timer handler under the scheduler lock, adding
 *   unbounded ISR latency.
 */

#ಅನ್ತರ್ಭಾವ <zephyr/kernel.h>
#ಅನ್ತರ್ಭಾವ <zephyr/toolchain.h>
#ಅನ್ತರ್ಭಾವ <kspinlock.h>
#ಅನ್ತರ್ಭಾವ <ksched.h>
#ಅನ್ತರ್ಭಾವ <scheduler.h>
#ಅನ್ತರ್ಭಾವ <kthread.h>
#ಅನ್ತರ್ಭಾವ <wait_q.h>
#ಅನ್ತರ್ಭಾವ <errno.h>
#ಅನ್ತರ್ಭಾವ <zephyr/init.h>
#ಅನ್ತರ್ಭಾವ <zephyr/internal/syscall_handler.h>
#ಅನ್ತರ್ಭಾವ <zephyr/tracing/tracing.h>
#ಅನ್ತರ್ಭಾವ <zephyr/sys/check.h>
#ಅನ್ತರ್ಭಾವ <zephyr/logging/log.h>
#ಅನ್ತರ್ಭಾವ <zephyr/llext/symbol.h>
LOG_MODULE_DECLARE(os, CONFIG_KERNEL_LOG_LEVEL);

/*
 * We use a global spinlock rather than a per-object lock because the
 * priority-inheritance chain walk, the held-mutexes recalculation, the
 * K_FOREVER deadlock check, and the unlock handoff all read or write
 * fields (owner, mutex_pended_on, held_mutexes) belonging to mutexes
 * other than the one passed to the API entry point.
 */
ಸ್ಥಿರ ರಚನೆ k_spinlock mutex_ಬೀಗ;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_OBJ_CORE_MUTEX
K_OBJ_TYPE_DEFINE(obj_type_mutex, k_mutex, K_OBJ_TYPE_MUTEX_ID, ಖಾಲಿ);
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_OBJ_CORE_MUTEX */

/*
 * Reset the mutex under mutex_lock so a concurrent k_mutex_lock() cannot
 * observe a half-initialized object. With refuse_if_owned set, a mutex that
 * has an owner is left untouched: resetting it would leave its held_node
 * linked in the owner's held_mutexes list and reset the wait queue under
 * pending waiters (threads only pend while the mutex has an owner).
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ mutex_init(ರಚನೆ k_mutex *mutex, bool refuse_if_owned)
{
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_ಬೀಗ(&mutex_ಬೀಗ);

	ಆದರೆ (refuse_if_owned && (mutex->owner != ಖಾಲಿ)) {
		k_spin_unlock(&mutex_ಬೀಗ, ಕೀಲಿ);
		ಹಿಂತಿರುಗಿ -EBUSY;
	}

	mutex->owner = ಖಾಲಿ;
	mutex->ಬೀಗ_ಎಣಿಕೆ = 0U;

	z_waitq_init(&mutex->wait_q);

#ಆದರೆ Z_MUTEX_PI_ENABLED
	mutex->held_ಗಂಟು.ಮುಂದಿನ = ಖಾಲಿ;
#ಆದರೆ_ಅಂತ್ಯ

	k_spin_unlock(&mutex_ಬೀಗ, ಕೀಲಿ);

	k_object_init(mutex);

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_OBJ_CORE_MUTEX
	k_obj_core_init_and_link(K_OBJ_CORE(mutex), &obj_type_mutex);
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_OBJ_CORE_MUTEX */

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_INIT(k_mutex, mutex, 0);

	ಹಿಂತಿರುಗಿ 0;
}

ಪೂರ್ಣಾಂಕ z_impl_k_mutex_init(ರಚನೆ k_mutex *mutex)
{
	ಹಿಂತಿರುಗಿ mutex_init(mutex, false);
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_USERSPACE
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಪೂರ್ಣಾಂಕ z_vrfy_k_mutex_init(ರಚನೆ k_mutex *mutex)
{
	bool initialized;

	K_OOPS(K_SYSCALL_OBJ_INIT(mutex, K_OBJ_MUTEX));

	/* The owner field is only meaningful once the mutex was initialized */
	initialized = (k_object_ಕಂಡುಹಿಡಿ(mutex)->flags & K_OBJ_FLAG_INITIALIZED) != 0U;

	K_OOPS(K_SYSCALL_VERIFY_ಸಂದೇಶ(mutex_init(mutex, initialized) == 0,
				    "mutex %p is locked", mutex));

	ಹಿಂತಿರುಗಿ 0;
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_mutex_init_mrsh.c>
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_USERSPACE */

#ಆದರೆ Z_MUTEX_PI_ENABLED
/*
 * Upper bound on ownership-chain walk hops. This is a bound on walk cost
 * under mutex_lock, not a real limit on chain length -- equal-priority
 * threads can legally form longer chains.
 */
#ವ್ಯಾಖ್ಯಾನಿಸು MUTEX_CHAIN_WALK_MAX_HOPS 16

ಸ್ಥಿರ int32_t new_prio_for_inheritance(int32_t ಗುರಿ, int32_t ಎಲ್ಲೆ)
{
	ಪೂರ್ಣಾಂಕ new_prio = z_is_prio_higher(ಗುರಿ, ಎಲ್ಲೆ) ? ಗುರಿ : ಎಲ್ಲೆ;

	new_prio = z_get_new_prio_with_ceiling(new_prio);

	ಹಿಂತಿರುಗಿ new_prio;
}

ಸ್ಥಿರ bool adjust_owner_prio(ರಚನೆ k_mutex *mutex, int32_t new_prio)
{
	ಆದರೆ (mutex->owner->ತಳ.prio != new_prio) {

		LOG_DBG("%p (ready (y/n): %c) prio changed to %d (was %d)",
			mutex->owner, z_is_thread_ready(mutex->owner) ?
			'y' : 'n',
			new_prio, mutex->owner->ತಳ.prio);

		ಹಿಂತಿರುಗಿ z_thread_prio_ಗಣ(mutex->owner, new_prio);
	}
	ಹಿಂತಿರುಗಿ false;
}

/*
 * Scan all mutexes held by a thread and return the highest priority
 * among all their waiters, floored at floor_prio.
 */
ಸ್ಥಿರ int32_t held_mutexes_highest_waiter_prio(ರಚನೆ k_thread *thread,
						 int32_t floor_prio)
{
	sys_snode_t *ಗಂಟು;
	int32_t prio = floor_prio;

	ಬೀಗ_SCHED_SPINLOCK {
		SYS_SLIST_FOR_EACH_ಗಂಟು(&thread->held_mutexes, ಗಂಟು) {
			ರಚನೆ k_mutex *m =
				CONTAINER_ದ(ಗಂಟು, ರಚನೆ k_mutex, held_ಗಂಟು);
			ರಚನೆ k_thread *waiter = z_waitq_ತಲೆ_locked(&m->wait_q);

			ಆದರೆ (waiter != ಖಾಲಿ) {
				int32_t wprio = z_get_new_prio_with_ceiling(waiter->ತಳ.prio);

				ಆದರೆ (z_is_prio_higher(wprio, prio)) {
					prio = wprio;
				}
			}
		}
	}
	ಹಿಂತಿರುಗಿ prio;
}
#ಆದರೆ_ಅಂತ್ಯ

ಪೂರ್ಣಾಂಕ z_impl_k_mutex_ಬೀಗ(ರಚನೆ k_mutex *mutex, k_timeout_t timeout)
{
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ;
#ಆದರೆ Z_MUTEX_PI_ENABLED
	bool resched = false;
#ಆದರೆ_ಅಂತ್ಯ

	__ASSERT(!arch_is_ಒಳ_isr(), "mutexes cannot be used inside ISRs");

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_ENTER(k_mutex, ಬೀಗ, mutex, timeout);

	ಕೀಲಿ = k_spin_ಬೀಗ(&mutex_ಬೀಗ);

	ಆದರೆ (likely((mutex->ಬೀಗ_ಎಣಿಕೆ == 0U) || (mutex->owner == _current))) {

		mutex->ಬೀಗ_ಎಣಿಕೆ++;
		mutex->owner = _current;

#ಆದರೆ Z_MUTEX_PI_ENABLED
		LOG_DBG("%p took mutex %p, count: %d, orig prio: %d",
			_current, mutex, mutex->ಬೀಗ_ಎಣಿಕೆ,
			_current->orig_prio);

		/* Add to held list on first acquisition; recursive locks skip this. */
		ಆದರೆ (mutex->ಬೀಗ_ಎಣಿಕೆ == 1U) {
			/* Capture the thread's true pre-inheritance priority
			 * the first time it acquires any mutex.
			 */
			ಆದರೆ (sys_slist_is_ಖಾಲಿಗಳು(&_current->held_mutexes)) {
				_current->orig_prio = _current->ತಳ.prio;
			}
			sys_slist_append(&_current->held_mutexes, &mutex->held_ಗಂಟು);
		}
#ಇಲ್ಲದಿದ್ದರೆ
		LOG_DBG("%p took mutex %p, count: %d",
			_current, mutex, mutex->ಬೀಗ_ಎಣಿಕೆ);
#ಆದರೆ_ಅಂತ್ಯ

		k_spin_unlock(&mutex_ಬೀಗ, ಕೀಲಿ);

		SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_EXIT(k_mutex, ಬೀಗ, mutex, timeout, 0);

		ಹಿಂತಿರುಗಿ 0;
	}

	ಆದರೆ (unlikely(K_TIMEOUT_EQ(timeout, K_NO_WAIT))) {
		k_spin_unlock(&mutex_ಬೀಗ, ಕೀಲಿ);

		SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_EXIT(k_mutex, ಬೀಗ, mutex, timeout, -EBUSY);

		ಹಿಂತಿರುಗಿ -EBUSY;
	}

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_BLOCKING(k_mutex, ಬೀಗ, mutex, timeout);

#ಆದರೆ Z_MUTEX_PI_ENABLED
	/* Record the mutex this thread is blocking on for priority inheritance. */
	_current->mutex_pended_on = mutex;

	/*
	 * Walk the ownership chain, boosting each owner's priority to match
	 * the highest-priority waiter. Detects deadlock cycles along the way.
	 */
	{
		int32_t boost_prio = new_prio_for_inheritance(
					_current->ತಳ.prio,
					mutex->owner->ತಳ.prio);
		ರಚನೆ k_mutex  *chain_mutex = mutex;
		ರಚನೆ k_thread *chain_owner = mutex->owner;
		ಪೂರ್ಣಾಂಕ hops = 0;
#ಆದರೆ defined(CONFIG_MUTEX_DEADLOCK_DETECT)
		bool all_forever = K_TIMEOUT_EQ(timeout, K_FOREVER);
#ಆದರೆ_ಅಂತ್ಯ

		LOG_DBG("chain boosting prio on mutex %p", mutex);

		ಆಗ (chain_owner != ಖಾಲಿ) {
#ಆದರೆ defined(CONFIG_MUTEX_DEADLOCK_DETECT)
			/*
			 * Deadlock detection: only when _current and every
			 * chain member visited so far wait forever. all_forever
			 * is updated on every hop, so a finite timeout anywhere
			 * in the chain -- not just at the hop that closes the
			 * cycle -- correctly rules out a false positive.
			 * z_is_thread_pending() guards against a stale
			 * mutex_pended_on in a thread that timed out but has
			 * not yet cleared the field.
			 */
			all_forever = all_forever &&
				      z_is_inactive_timeout(&chain_owner->ತಳ.timeout);

			ಆದರೆ (all_forever &&
			    chain_owner->mutex_pended_on != ಖಾಲಿ &&
			    z_is_thread_pending(chain_owner) &&
			    chain_owner->mutex_pended_on->owner == _current) {
				__ASSERT(false,
					"mutex deadlock: thread %p waiting on "
					"mutex %p (owner %p)",
					_current, mutex, mutex->owner);
				ನಿಲ್ಲಿಸು;
			}
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_MUTEX_DEADLOCK_DETECT */

			ಆದರೆ (z_is_prio_higher(boost_prio, chain_owner->ತಳ.prio)) {
				resched = adjust_owner_prio(chain_mutex, boost_prio)
					  || resched;
			}

			/*
			 * Stop the chain walk if the owner is not actually
			 * pending — its mutex_pended_on may be stale (e.g. it
			 * just timed out but has not yet cleared the field).
			 * Always boost the current owner first, then decide
			 * whether to continue.
			 */
			ಆದರೆ (chain_owner->mutex_pended_on == ಖಾಲಿ ||
			    !z_is_thread_pending(chain_owner)) {
				ನಿಲ್ಲಿಸು;
			}

			chain_mutex = chain_owner->mutex_pended_on;
			/*
			 * Cap the walk to prevent spinning on a cycle that
			 * does not pass through the starting mutex.
			 * MUTEX_CHAIN_WALK_MAX_HOPS bounds walk cost under
			 * mutex_lock; it is not a real limit on chain length.
			 * The chain_mutex == mutex check is a fast path for
			 * the common 2-hop cycle through the starting mutex.
			 */
			ಆದರೆ (chain_mutex == mutex ||
			    ++hops >= MUTEX_CHAIN_WALK_MAX_HOPS) {
				ನಿಲ್ಲಿಸು;
			}
			chain_owner = chain_mutex->owner;
		}
	}
#ಆದರೆ_ಅಂತ್ಯ

	ಪೂರ್ಣಾಂಕ got_mutex = z_pend_curr(&mutex_ಬೀಗ, ಕೀಲಿ, &mutex->wait_q, timeout);

	LOG_DBG("on mutex %p got_mutex value: %d", mutex, got_mutex);

	LOG_DBG("%p got mutex %p (y/n): %c", _current, mutex,
		got_mutex ? 'y' : 'n');

#ಆದರೆ Z_MUTEX_PI_ENABLED
	ಆದರೆ (got_mutex == 0) {
		/* Lock granted; clear the pending mutex pointer. */
		_current->mutex_pended_on = ಖಾಲಿ;
		SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_EXIT(k_mutex, ಬೀಗ, mutex, timeout, 0);
		ಹಿಂತಿರುಗಿ 0;
	}

	/* timed out */

	LOG_DBG("%p timeout on mutex %p", _current, mutex);

	ಕೀಲಿ = k_spin_ಬೀಗ(&mutex_ಬೀಗ);

	/*
	 * Clear mutex_pended_on under mutex_lock so no concurrent chain walk
	 * can observe the stale pointer and detect a false deadlock cycle.
	 */
	_current->mutex_pended_on = ಖಾಲಿ;

	/*
	 * Check if mutex was unlocked after this thread was unpended.
	 * If so, skip adjusting owner's priority down.
	 */
	ಆದರೆ (likely(mutex->owner != ಖಾಲಿ)) {
		/*
		 * Recalculate the owner's priority across all mutexes it still
		 * holds; another held mutex may still justify a partial boost.
		 */
		int32_t new_prio = held_mutexes_highest_waiter_prio(
						mutex->owner,
						mutex->owner->orig_prio);

		LOG_DBG("adjusting prio down on mutex %p", mutex);

		resched = adjust_owner_prio(mutex, new_prio) || resched;
	}

	ಆದರೆ (resched) {
		z_reschedule(&mutex_ಬೀಗ, ಕೀಲಿ);
	} ಇಲ್ಲದಿದ್ದರೆ {
		k_spin_unlock(&mutex_ಬೀಗ, ಕೀಲಿ);
	}

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_EXIT(k_mutex, ಬೀಗ, mutex, timeout, -EAGAIN);

	ಹಿಂತಿರುಗಿ -EAGAIN;
#ಇಲ್ಲದಿದ್ದರೆ
	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_EXIT(k_mutex, ಬೀಗ, mutex, timeout, got_mutex);

	ಹಿಂತಿರುಗಿ got_mutex;
#ಆದರೆ_ಅಂತ್ಯ
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_USERSPACE
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಪೂರ್ಣಾಂಕ z_vrfy_k_mutex_ಬೀಗ(ರಚನೆ k_mutex *mutex,
				      k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(mutex, K_OBJ_MUTEX));
	ಹಿಂತಿರುಗಿ z_impl_k_mutex_ಬೀಗ(mutex, timeout);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_mutex_lock_mrsh.c>
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_USERSPACE */

ಪೂರ್ಣಾಂಕ z_impl_k_mutex_unlock(ರಚನೆ k_mutex *mutex)
{
	ರಚನೆ k_thread *new_owner = ಖಾಲಿ;

	__ASSERT(!arch_is_ಒಳ_isr(), "mutexes cannot be used inside ISRs");

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_ENTER(k_mutex, unlock, mutex);

	CHECKIF(mutex->owner == ಖಾಲಿ) {
		SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_EXIT(k_mutex, unlock, mutex, -EINVAL);

		ಹಿಂತಿರುಗಿ -EINVAL;
	}
	/*
	 * The current thread does not own the mutex.
	 */
	CHECKIF(mutex->owner != _current) {
		SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_EXIT(k_mutex, unlock, mutex, -EPERM);

		ಹಿಂತಿರುಗಿ -EPERM;
	}

	/*
	 * Attempt to unlock a mutex which is unlocked. mutex->lock_count
	 * cannot be zero if the current thread is equal to mutex->owner,
	 * therefore no underflow check is required. Use assert to catch
	 * undefined behavior.
	 */
	__ASSERT_NO_MSG(mutex->ಬೀಗ_ಎಣಿಕೆ > 0U);

	LOG_DBG("mutex %p lock_count: %d", mutex, mutex->ಬೀಗ_ಎಣಿಕೆ);

	/*
	 * If we are the owner and count is greater than 1, then decrement
	 * the count and return and keep current thread as the owner.
	 */
	ಆದರೆ (mutex->ಬೀಗ_ಎಣಿಕೆ > 1U) {
		mutex->ಬೀಗ_ಎಣಿಕೆ--;
		ಹೋಗು k_mutex_unlock_return;
	}

	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_ಬೀಗ(&mutex_ಬೀಗ);

#ಆದರೆ Z_MUTEX_PI_ENABLED
	/* Remove this mutex from the owner's list of held mutexes */
	sys_slist_ಕಂಡುಹಿಡಿ_and_remove(&_current->held_mutexes, &mutex->held_ಗಂಟು);

	/*
	 * If the thread's priority was boosted, recalculate it by scanning
	 * the remaining held mutexes. The released mutex has already been
	 * removed from held_mutexes, so it is not included in the scan.
	 */
	ಆದರೆ (_current->ತಳ.prio != _current->orig_prio) {
		int32_t new_prio = held_mutexes_highest_waiter_prio(
					_current, _current->orig_prio);

		adjust_owner_prio(mutex, new_prio);
	}
#ಆದರೆ_ಅಂತ್ಯ

	/* Pick the new owner (if any) and complete the wake atomically
	 * under the scheduler's spinlock, so a racing in-flight timeout
	 * handler cannot observe a half-initialized wake-up.
	 */
	ಬೀಗ_SCHED_SPINLOCK {
		new_owner = z_unpend_ಮೊದಲ_thread_locked(&mutex->wait_q);
		mutex->owner = new_owner;

		LOG_DBG("new owner of mutex %p: %p (prio: %d)",
			mutex, new_owner, new_owner ? new_owner->ತಳ.prio : -1000);

		ಆದರೆ (unlikely(new_owner != ಖಾಲಿ)) {
			/*
			 * new owner is already of higher or equal prio than first
			 * waiter since the wait queue is priority-based: no need to
			 * adjust its priority
			 */
#ಆದರೆ Z_MUTEX_PI_ENABLED
			ಆದರೆ (sys_slist_is_ಖಾಲಿಗಳು(&new_owner->held_mutexes)) {
				new_owner->orig_prio = new_owner->ತಳ.prio;
			}
			sys_slist_append(&new_owner->held_mutexes, &mutex->held_ಗಂಟು);
			new_owner->mutex_pended_on = ಖಾಲಿ;
#ಆದರೆ_ಅಂತ್ಯ
			arch_thread_return_ಮೌಲ್ಯ_ಗಣ(new_owner, 0);
			z_sched_ready_locked(new_owner);
		} ಇಲ್ಲದಿದ್ದರೆ {
			mutex->ಬೀಗ_ಎಣಿಕೆ = 0U;
		}
	}

	ಆದರೆ (unlikely(new_owner != ಖಾಲಿ)) {
		z_reschedule(&mutex_ಬೀಗ, ಕೀಲಿ);
	} ಇಲ್ಲದಿದ್ದರೆ {
		k_spin_unlock(&mutex_ಬೀಗ, ಕೀಲಿ);
	}


k_mutex_unlock_return:
	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_EXIT(k_mutex, unlock, mutex, 0);

	ಹಿಂತಿರುಗಿ 0;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_USERSPACE
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಪೂರ್ಣಾಂಕ z_vrfy_k_mutex_unlock(ರಚನೆ k_mutex *mutex)
{
	K_OOPS(K_SYSCALL_OBJ(mutex, K_OBJ_MUTEX));
	ಹಿಂತಿರುಗಿ z_impl_k_mutex_unlock(mutex);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_mutex_unlock_mrsh.c>
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_USERSPACE */
