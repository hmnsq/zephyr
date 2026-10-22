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

#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/kernel.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/toolchain.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <kspinlock.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <ksched.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <scheduler.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <kthread.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <wait_q.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <errno.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/init.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/internal/syscall_handler.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/tracing/tracing.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/sys/check.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/logging/log.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/llext/symbol.h>
LOG_MODULE_DECLARE(os, CONFIG_KERNEL_LOG_LEVEL);

/*
 * We use a global spinlock rather than a per-object lock because the
 * priority-inheritance chain walk, the held-mutexes recalculation, the
 * K_FOREVER deadlock check, and the unlock handoff all read or write
 * fields (owner, mutex_pended_on, held_mutexes) belonging to mutexes
 * other than the one passed to the API entry point.
 */
ਸਥਿਰ ਬਣਤਰ k_spinlock mutex_ਤਾਲਾ;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_OBJ_CORE_MUTEX
K_OBJ_TYPE_DEFINE(obj_type_mutex, k_mutex, K_OBJ_TYPE_MUTEX_ID, ਨਿਰਾ);
#ਜੇ_ਅੰਤ /* CONFIG_OBJ_CORE_MUTEX */

/*
 * Reset the mutex under mutex_lock so a concurrent k_mutex_lock() cannot
 * observe a half-initialized object. With refuse_if_owned set, a mutex that
 * has an owner is left untouched: resetting it would leave its held_node
 * linked in the owner's held_mutexes list and reset the wait queue under
 * pending waiters (threads only pend while the mutex has an owner).
 */
ਸਥਿਰ ਪੂਰਨ mutex_init(ਬਣਤਰ k_mutex *mutex, bool refuse_if_owned)
{
	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&mutex_ਤਾਲਾ);

	ਜੇ (refuse_if_owned && (mutex->owner != ਨਿਰਾ)) {
		k_spin_unlock(&mutex_ਤਾਲਾ, ਕੁੰਜੀ);
		ਵਾਪਸ -EBUSY;
	}

	mutex->owner = ਨਿਰਾ;
	mutex->ਤਾਲਾ_ਗਿਣਤੀ = 0U;

	z_waitq_init(&mutex->wait_q);

#ਜੇ Z_MUTEX_PI_ENABLED
	mutex->held_ਗੰਢ.ਅਗਲਾ = ਨਿਰਾ;
#ਜੇ_ਅੰਤ

	k_spin_unlock(&mutex_ਤਾਲਾ, ਕੁੰਜੀ);

	k_object_init(mutex);

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_OBJ_CORE_MUTEX
	k_obj_core_init_and_link(K_OBJ_CORE(mutex), &obj_type_mutex);
#ਜੇ_ਅੰਤ /* CONFIG_OBJ_CORE_MUTEX */

	SYS_ਪੋਰਟ_TRACING_OBJ_INIT(k_mutex, mutex, 0);

	ਵਾਪਸ 0;
}

ਪੂਰਨ z_impl_k_mutex_init(ਬਣਤਰ k_mutex *mutex)
{
	ਵਾਪਸ mutex_init(mutex, false);
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_USERSPACE
ਸਥਿਰ ਸਤਰਅੰਦਰ ਪੂਰਨ z_vrfy_k_mutex_init(ਬਣਤਰ k_mutex *mutex)
{
	bool initialized;

	K_OOPS(K_SYSCALL_OBJ_INIT(mutex, K_OBJ_MUTEX));

	/* The owner field is only meaningful once the mutex was initialized */
	initialized = (k_object_ਲੱਭੋ(mutex)->flags & K_OBJ_FLAG_INITIALIZED) != 0U;

	K_OOPS(K_SYSCALL_VERIFY_ਸੁਨੇਹਾ(mutex_init(mutex, initialized) == 0,
				    "mutex %p is locked", mutex));

	ਵਾਪਸ 0;
}
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/syscalls/k_mutex_init_mrsh.c>
#ਜੇ_ਅੰਤ /* CONFIG_USERSPACE */

#ਜੇ Z_MUTEX_PI_ENABLED
/*
 * Upper bound on ownership-chain walk hops. This is a bound on walk cost
 * under mutex_lock, not a real limit on chain length -- equal-priority
 * threads can legally form longer chains.
 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MUTEX_CHAIN_WALK_MAX_HOPS 16

ਸਥਿਰ int32_t new_prio_for_inheritance(int32_t ਨਿਸ਼ਾਨਾ, int32_t ਹੱਦਬੰਦੀ)
{
	ਪੂਰਨ new_prio = z_is_prio_higher(ਨਿਸ਼ਾਨਾ, ਹੱਦਬੰਦੀ) ? ਨਿਸ਼ਾਨਾ : ਹੱਦਬੰਦੀ;

	new_prio = z_get_new_prio_with_ceiling(new_prio);

	ਵਾਪਸ new_prio;
}

ਸਥਿਰ bool adjust_owner_prio(ਬਣਤਰ k_mutex *mutex, int32_t new_prio)
{
	ਜੇ (mutex->owner->ਅਧਾਰ.prio != new_prio) {

		LOG_DBG("%p (ready (y/n): %c) prio changed to %d (was %d)",
			mutex->owner, z_is_thread_ready(mutex->owner) ?
			'y' : 'n',
			new_prio, mutex->owner->ਅਧਾਰ.prio);

		ਵਾਪਸ z_thread_prio_ਸੈੱਟ(mutex->owner, new_prio);
	}
	ਵਾਪਸ false;
}

/*
 * Scan all mutexes held by a thread and return the highest priority
 * among all their waiters, floored at floor_prio.
 */
ਸਥਿਰ int32_t held_mutexes_highest_waiter_prio(ਬਣਤਰ k_thread *thread,
						 int32_t floor_prio)
{
	sys_snode_t *ਗੰਢ;
	int32_t prio = floor_prio;

	ਤਾਲਾ_SCHED_SPINLOCK {
		SYS_SLIST_FOR_EACH_ਗੰਢ(&thread->held_mutexes, ਗੰਢ) {
			ਬਣਤਰ k_mutex *m =
				CONTAINER_ਦਾ(ਗੰਢ, ਬਣਤਰ k_mutex, held_ਗੰਢ);
			ਬਣਤਰ k_thread *waiter = z_waitq_ਸਿਰ_locked(&m->wait_q);

			ਜੇ (waiter != ਨਿਰਾ) {
				int32_t wprio = z_get_new_prio_with_ceiling(waiter->ਅਧਾਰ.prio);

				ਜੇ (z_is_prio_higher(wprio, prio)) {
					prio = wprio;
				}
			}
		}
	}
	ਵਾਪਸ prio;
}
#ਜੇ_ਅੰਤ

ਪੂਰਨ z_impl_k_mutex_ਤਾਲਾ(ਬਣਤਰ k_mutex *mutex, k_timeout_t timeout)
{
	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ;
#ਜੇ Z_MUTEX_PI_ENABLED
	bool resched = false;
#ਜੇ_ਅੰਤ

	__ASSERT(!arch_is_ਵਿੱਚ_isr(), "mutexes cannot be used inside ISRs");

	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_ENTER(k_mutex, ਤਾਲਾ, mutex, timeout);

	ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&mutex_ਤਾਲਾ);

	ਜੇ (likely((mutex->ਤਾਲਾ_ਗਿਣਤੀ == 0U) || (mutex->owner == _current))) {

		mutex->ਤਾਲਾ_ਗਿਣਤੀ++;
		mutex->owner = _current;

#ਜੇ Z_MUTEX_PI_ENABLED
		LOG_DBG("%p took mutex %p, count: %d, orig prio: %d",
			_current, mutex, mutex->ਤਾਲਾ_ਗਿਣਤੀ,
			_current->orig_prio);

		/* Add to held list on first acquisition; recursive locks skip this. */
		ਜੇ (mutex->ਤਾਲਾ_ਗਿਣਤੀ == 1U) {
			/* Capture the thread's true pre-inheritance priority
			 * the first time it acquires any mutex.
			 */
			ਜੇ (sys_slist_is_ਖਾਲੀ_ਥਾਵਾਂ(&_current->held_mutexes)) {
				_current->orig_prio = _current->ਅਧਾਰ.prio;
			}
			sys_slist_append(&_current->held_mutexes, &mutex->held_ਗੰਢ);
		}
#ਨਹੀਂਤਾਂ
		LOG_DBG("%p took mutex %p, count: %d",
			_current, mutex, mutex->ਤਾਲਾ_ਗਿਣਤੀ);
#ਜੇ_ਅੰਤ

		k_spin_unlock(&mutex_ਤਾਲਾ, ਕੁੰਜੀ);

		SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_EXIT(k_mutex, ਤਾਲਾ, mutex, timeout, 0);

		ਵਾਪਸ 0;
	}

	ਜੇ (unlikely(K_TIMEOUT_EQ(timeout, K_NO_WAIT))) {
		k_spin_unlock(&mutex_ਤਾਲਾ, ਕੁੰਜੀ);

		SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_EXIT(k_mutex, ਤਾਲਾ, mutex, timeout, -EBUSY);

		ਵਾਪਸ -EBUSY;
	}

	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_BLOCKING(k_mutex, ਤਾਲਾ, mutex, timeout);

#ਜੇ Z_MUTEX_PI_ENABLED
	/* Record the mutex this thread is blocking on for priority inheritance. */
	_current->mutex_pended_on = mutex;

	/*
	 * Walk the ownership chain, boosting each owner's priority to match
	 * the highest-priority waiter. Detects deadlock cycles along the way.
	 */
	{
		int32_t boost_prio = new_prio_for_inheritance(
					_current->ਅਧਾਰ.prio,
					mutex->owner->ਅਧਾਰ.prio);
		ਬਣਤਰ k_mutex  *chain_mutex = mutex;
		ਬਣਤਰ k_thread *chain_owner = mutex->owner;
		ਪੂਰਨ hops = 0;
#ਜੇ defined(CONFIG_MUTEX_DEADLOCK_DETECT)
		bool all_forever = K_TIMEOUT_EQ(timeout, K_FOREVER);
#ਜੇ_ਅੰਤ

		LOG_DBG("chain boosting prio on mutex %p", mutex);

		ਜਦੋਂ (chain_owner != ਨਿਰਾ) {
#ਜੇ defined(CONFIG_MUTEX_DEADLOCK_DETECT)
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
				      z_is_inactive_timeout(&chain_owner->ਅਧਾਰ.timeout);

			ਜੇ (all_forever &&
			    chain_owner->mutex_pended_on != ਨਿਰਾ &&
			    z_is_thread_pending(chain_owner) &&
			    chain_owner->mutex_pended_on->owner == _current) {
				__ASSERT(false,
					"mutex deadlock: thread %p waiting on "
					"mutex %p (owner %p)",
					_current, mutex, mutex->owner);
				ਰੋਕੋ;
			}
#ਜੇ_ਅੰਤ /* CONFIG_MUTEX_DEADLOCK_DETECT */

			ਜੇ (z_is_prio_higher(boost_prio, chain_owner->ਅਧਾਰ.prio)) {
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
			ਜੇ (chain_owner->mutex_pended_on == ਨਿਰਾ ||
			    !z_is_thread_pending(chain_owner)) {
				ਰੋਕੋ;
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
			ਜੇ (chain_mutex == mutex ||
			    ++hops >= MUTEX_CHAIN_WALK_MAX_HOPS) {
				ਰੋਕੋ;
			}
			chain_owner = chain_mutex->owner;
		}
	}
#ਜੇ_ਅੰਤ

	ਪੂਰਨ got_mutex = z_pend_curr(&mutex_ਤਾਲਾ, ਕੁੰਜੀ, &mutex->wait_q, timeout);

	LOG_DBG("on mutex %p got_mutex value: %d", mutex, got_mutex);

	LOG_DBG("%p got mutex %p (y/n): %c", _current, mutex,
		got_mutex ? 'y' : 'n');

#ਜੇ Z_MUTEX_PI_ENABLED
	ਜੇ (got_mutex == 0) {
		/* Lock granted; clear the pending mutex pointer. */
		_current->mutex_pended_on = ਨਿਰਾ;
		SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_EXIT(k_mutex, ਤਾਲਾ, mutex, timeout, 0);
		ਵਾਪਸ 0;
	}

	/* timed out */

	LOG_DBG("%p timeout on mutex %p", _current, mutex);

	ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&mutex_ਤਾਲਾ);

	/*
	 * Clear mutex_pended_on under mutex_lock so no concurrent chain walk
	 * can observe the stale pointer and detect a false deadlock cycle.
	 */
	_current->mutex_pended_on = ਨਿਰਾ;

	/*
	 * Check if mutex was unlocked after this thread was unpended.
	 * If so, skip adjusting owner's priority down.
	 */
	ਜੇ (likely(mutex->owner != ਨਿਰਾ)) {
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

	ਜੇ (resched) {
		z_reschedule(&mutex_ਤਾਲਾ, ਕੁੰਜੀ);
	} ਨਹੀਂਤਾਂ {
		k_spin_unlock(&mutex_ਤਾਲਾ, ਕੁੰਜੀ);
	}

	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_EXIT(k_mutex, ਤਾਲਾ, mutex, timeout, -EAGAIN);

	ਵਾਪਸ -EAGAIN;
#ਨਹੀਂਤਾਂ
	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_EXIT(k_mutex, ਤਾਲਾ, mutex, timeout, got_mutex);

	ਵਾਪਸ got_mutex;
#ਜੇ_ਅੰਤ
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_USERSPACE
ਸਥਿਰ ਸਤਰਅੰਦਰ ਪੂਰਨ z_vrfy_k_mutex_ਤਾਲਾ(ਬਣਤਰ k_mutex *mutex,
				      k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(mutex, K_OBJ_MUTEX));
	ਵਾਪਸ z_impl_k_mutex_ਤਾਲਾ(mutex, timeout);
}
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/syscalls/k_mutex_lock_mrsh.c>
#ਜੇ_ਅੰਤ /* CONFIG_USERSPACE */

ਪੂਰਨ z_impl_k_mutex_unlock(ਬਣਤਰ k_mutex *mutex)
{
	ਬਣਤਰ k_thread *new_owner = ਨਿਰਾ;

	__ASSERT(!arch_is_ਵਿੱਚ_isr(), "mutexes cannot be used inside ISRs");

	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_ENTER(k_mutex, unlock, mutex);

	CHECKIF(mutex->owner == ਨਿਰਾ) {
		SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_EXIT(k_mutex, unlock, mutex, -EINVAL);

		ਵਾਪਸ -EINVAL;
	}
	/*
	 * The current thread does not own the mutex.
	 */
	CHECKIF(mutex->owner != _current) {
		SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_EXIT(k_mutex, unlock, mutex, -EPERM);

		ਵਾਪਸ -EPERM;
	}

	/*
	 * Attempt to unlock a mutex which is unlocked. mutex->lock_count
	 * cannot be zero if the current thread is equal to mutex->owner,
	 * therefore no underflow check is required. Use assert to catch
	 * undefined behavior.
	 */
	__ASSERT_NO_MSG(mutex->ਤਾਲਾ_ਗਿਣਤੀ > 0U);

	LOG_DBG("mutex %p lock_count: %d", mutex, mutex->ਤਾਲਾ_ਗਿਣਤੀ);

	/*
	 * If we are the owner and count is greater than 1, then decrement
	 * the count and return and keep current thread as the owner.
	 */
	ਜੇ (mutex->ਤਾਲਾ_ਗਿਣਤੀ > 1U) {
		mutex->ਤਾਲਾ_ਗਿਣਤੀ--;
		ਜਾਓ k_mutex_unlock_return;
	}

	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&mutex_ਤਾਲਾ);

#ਜੇ Z_MUTEX_PI_ENABLED
	/* Remove this mutex from the owner's list of held mutexes */
	sys_slist_ਲੱਭੋ_and_remove(&_current->held_mutexes, &mutex->held_ਗੰਢ);

	/*
	 * If the thread's priority was boosted, recalculate it by scanning
	 * the remaining held mutexes. The released mutex has already been
	 * removed from held_mutexes, so it is not included in the scan.
	 */
	ਜੇ (_current->ਅਧਾਰ.prio != _current->orig_prio) {
		int32_t new_prio = held_mutexes_highest_waiter_prio(
					_current, _current->orig_prio);

		adjust_owner_prio(mutex, new_prio);
	}
#ਜੇ_ਅੰਤ

	/* Pick the new owner (if any) and complete the wake atomically
	 * under the scheduler's spinlock, so a racing in-flight timeout
	 * handler cannot observe a half-initialized wake-up.
	 */
	ਤਾਲਾ_SCHED_SPINLOCK {
		new_owner = z_unpend_ਪਹਿਲਾ_thread_locked(&mutex->wait_q);
		mutex->owner = new_owner;

		LOG_DBG("new owner of mutex %p: %p (prio: %d)",
			mutex, new_owner, new_owner ? new_owner->ਅਧਾਰ.prio : -1000);

		ਜੇ (unlikely(new_owner != ਨਿਰਾ)) {
			/*
			 * new owner is already of higher or equal prio than first
			 * waiter since the wait queue is priority-based: no need to
			 * adjust its priority
			 */
#ਜੇ Z_MUTEX_PI_ENABLED
			ਜੇ (sys_slist_is_ਖਾਲੀ_ਥਾਵਾਂ(&new_owner->held_mutexes)) {
				new_owner->orig_prio = new_owner->ਅਧਾਰ.prio;
			}
			sys_slist_append(&new_owner->held_mutexes, &mutex->held_ਗੰਢ);
			new_owner->mutex_pended_on = ਨਿਰਾ;
#ਜੇ_ਅੰਤ
			arch_thread_return_ਮੁੱਲ_ਸੈੱਟ(new_owner, 0);
			z_sched_ready_locked(new_owner);
		} ਨਹੀਂਤਾਂ {
			mutex->ਤਾਲਾ_ਗਿਣਤੀ = 0U;
		}
	}

	ਜੇ (unlikely(new_owner != ਨਿਰਾ)) {
		z_reschedule(&mutex_ਤਾਲਾ, ਕੁੰਜੀ);
	} ਨਹੀਂਤਾਂ {
		k_spin_unlock(&mutex_ਤਾਲਾ, ਕੁੰਜੀ);
	}


k_mutex_unlock_return:
	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_EXIT(k_mutex, unlock, mutex, 0);

	ਵਾਪਸ 0;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_USERSPACE
ਸਥਿਰ ਸਤਰਅੰਦਰ ਪੂਰਨ z_vrfy_k_mutex_unlock(ਬਣਤਰ k_mutex *mutex)
{
	K_OOPS(K_SYSCALL_OBJ(mutex, K_OBJ_MUTEX));
	ਵਾਪਸ z_impl_k_mutex_unlock(mutex);
}
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/syscalls/k_mutex_unlock_mrsh.c>
#ਜੇ_ਅੰਤ /* CONFIG_USERSPACE */
