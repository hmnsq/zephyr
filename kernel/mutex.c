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

#అన్తర్భావ <zephyr/kernel.h>
#అన్తర్భావ <zephyr/toolchain.h>
#అన్తర్భావ <kspinlock.h>
#అన్తర్భావ <ksched.h>
#అన్తర్భావ <scheduler.h>
#అన్తర్భావ <kthread.h>
#అన్తర్భావ <wait_q.h>
#అన్తర్భావ <errno.h>
#అన్తర్భావ <zephyr/init.h>
#అన్తర్భావ <zephyr/internal/syscall_handler.h>
#అన్తర్భావ <zephyr/tracing/tracing.h>
#అన్తర్భావ <zephyr/sys/check.h>
#అన్తర్భావ <zephyr/logging/log.h>
#అన్తర్భావ <zephyr/llext/symbol.h>
LOG_MODULE_DECLARE(os, CONFIG_KERNEL_LOG_LEVEL);

/*
 * We use a global spinlock rather than a per-object lock because the
 * priority-inheritance chain walk, the held-mutexes recalculation, the
 * K_FOREVER deadlock check, and the unlock handoff all read or write
 * fields (owner, mutex_pended_on, held_mutexes) belonging to mutexes
 * other than the one passed to the API entry point.
 */
స్థిర నిర్మాణం k_spinlock mutex_తాళం;

#నిర్వచితమైతే CONFIG_OBJ_CORE_MUTEX
K_OBJ_TYPE_DEFINE(obj_type_mutex, k_mutex, K_OBJ_TYPE_MUTEX_ID, ఖాళీ);
#అయితే_ముగింపు /* CONFIG_OBJ_CORE_MUTEX */

/*
 * Reset the mutex under mutex_lock so a concurrent k_mutex_lock() cannot
 * observe a half-initialized object. With refuse_if_owned set, a mutex that
 * has an owner is left untouched: resetting it would leave its held_node
 * linked in the owner's held_mutexes list and reset the wait queue under
 * pending waiters (threads only pend while the mutex has an owner).
 */
స్థిర పూర్ణాంకం mutex_init(నిర్మాణం k_mutex *mutex, bool refuse_if_owned)
{
	k_spinlock_కీలకం_t కీలకం = k_spin_తాళం(&mutex_తాళం);

	ఒకవేళ (refuse_if_owned && (mutex->owner != ఖాళీ)) {
		k_spin_unlock(&mutex_తాళం, కీలకం);
		తిరిగి -EBUSY;
	}

	mutex->owner = ఖాళీ;
	mutex->తాళం_లెక్క = 0U;

	z_waitq_init(&mutex->wait_q);

#ఒకవేళ Z_MUTEX_PI_ENABLED
	mutex->held_కణుపు.తదుపరి = ఖాళీ;
#అయితే_ముగింపు

	k_spin_unlock(&mutex_తాళం, కీలకం);

	k_object_init(mutex);

#నిర్వచితమైతే CONFIG_OBJ_CORE_MUTEX
	k_obj_core_init_and_link(K_OBJ_CORE(mutex), &obj_type_mutex);
#అయితే_ముగింపు /* CONFIG_OBJ_CORE_MUTEX */

	SYS_పోర్టు_TRACING_OBJ_INIT(k_mutex, mutex, 0);

	తిరిగి 0;
}

పూర్ణాంకం z_impl_k_mutex_init(నిర్మాణం k_mutex *mutex)
{
	తిరిగి mutex_init(mutex, false);
}

#నిర్వచితమైతే CONFIG_USERSPACE
స్థిర అంతఃవరుస పూర్ణాంకం z_vrfy_k_mutex_init(నిర్మాణం k_mutex *mutex)
{
	bool initialized;

	K_OOPS(K_SYSCALL_OBJ_INIT(mutex, K_OBJ_MUTEX));

	/* The owner field is only meaningful once the mutex was initialized */
	initialized = (k_object_కనుగొను(mutex)->flags & K_OBJ_FLAG_INITIALIZED) != 0U;

	K_OOPS(K_SYSCALL_VERIFY_సందేశం(mutex_init(mutex, initialized) == 0,
				    "mutex %p is locked", mutex));

	తిరిగి 0;
}
#అన్తర్భావ <zephyr/syscalls/k_mutex_init_mrsh.c>
#అయితే_ముగింపు /* CONFIG_USERSPACE */

#ఒకవేళ Z_MUTEX_PI_ENABLED
/*
 * Upper bound on ownership-chain walk hops. This is a bound on walk cost
 * under mutex_lock, not a real limit on chain length -- equal-priority
 * threads can legally form longer chains.
 */
#నిర్వచించు MUTEX_CHAIN_WALK_MAX_HOPS 16

స్థిర int32_t new_prio_for_inheritance(int32_t లక్ష్యం, int32_t హద్దు)
{
	పూర్ణాంకం new_prio = z_is_prio_higher(లక్ష్యం, హద్దు) ? లక్ష్యం : హద్దు;

	new_prio = z_get_new_prio_with_ceiling(new_prio);

	తిరిగి new_prio;
}

స్థిర bool adjust_owner_prio(నిర్మాణం k_mutex *mutex, int32_t new_prio)
{
	ఒకవేళ (mutex->owner->పునాది.prio != new_prio) {

		LOG_DBG("%p (ready (y/n): %c) prio changed to %d (was %d)",
			mutex->owner, z_is_thread_ready(mutex->owner) ?
			'y' : 'n',
			new_prio, mutex->owner->పునాది.prio);

		తిరిగి z_thread_prio_సమితి(mutex->owner, new_prio);
	}
	తిరిగి false;
}

/*
 * Scan all mutexes held by a thread and return the highest priority
 * among all their waiters, floored at floor_prio.
 */
స్థిర int32_t held_mutexes_highest_waiter_prio(నిర్మాణం k_thread *thread,
						 int32_t floor_prio)
{
	sys_snode_t *కణుపు;
	int32_t prio = floor_prio;

	తాళం_SCHED_SPINLOCK {
		SYS_SLIST_FOR_EACH_కణుపు(&thread->held_mutexes, కణుపు) {
			నిర్మాణం k_mutex *m =
				CONTAINER_యొక్క(కణుపు, నిర్మాణం k_mutex, held_కణుపు);
			నిర్మాణం k_thread *waiter = z_waitq_తల_locked(&m->wait_q);

			ఒకవేళ (waiter != ఖాళీ) {
				int32_t wprio = z_get_new_prio_with_ceiling(waiter->పునాది.prio);

				ఒకవేళ (z_is_prio_higher(wprio, prio)) {
					prio = wprio;
				}
			}
		}
	}
	తిరిగి prio;
}
#అయితే_ముగింపు

పూర్ణాంకం z_impl_k_mutex_తాళం(నిర్మాణం k_mutex *mutex, k_timeout_t timeout)
{
	k_spinlock_కీలకం_t కీలకం;
#ఒకవేళ Z_MUTEX_PI_ENABLED
	bool resched = false;
#అయితే_ముగింపు

	__ASSERT(!arch_is_లోపల_isr(), "mutexes cannot be used inside ISRs");

	SYS_పోర్టు_TRACING_OBJ_FUNC_ENTER(k_mutex, తాళం, mutex, timeout);

	కీలకం = k_spin_తాళం(&mutex_తాళం);

	ఒకవేళ (likely((mutex->తాళం_లెక్క == 0U) || (mutex->owner == _current))) {

		mutex->తాళం_లెక్క++;
		mutex->owner = _current;

#ఒకవేళ Z_MUTEX_PI_ENABLED
		LOG_DBG("%p took mutex %p, count: %d, orig prio: %d",
			_current, mutex, mutex->తాళం_లెక్క,
			_current->orig_prio);

		/* Add to held list on first acquisition; recursive locks skip this. */
		ఒకవేళ (mutex->తాళం_లెక్క == 1U) {
			/* Capture the thread's true pre-inheritance priority
			 * the first time it acquires any mutex.
			 */
			ఒకవేళ (sys_slist_is_ఖాళీలు(&_current->held_mutexes)) {
				_current->orig_prio = _current->పునాది.prio;
			}
			sys_slist_append(&_current->held_mutexes, &mutex->held_కణుపు);
		}
#లేకపోతే
		LOG_DBG("%p took mutex %p, count: %d",
			_current, mutex, mutex->తాళం_లెక్క);
#అయితే_ముగింపు

		k_spin_unlock(&mutex_తాళం, కీలకం);

		SYS_పోర్టు_TRACING_OBJ_FUNC_EXIT(k_mutex, తాళం, mutex, timeout, 0);

		తిరిగి 0;
	}

	ఒకవేళ (unlikely(K_TIMEOUT_EQ(timeout, K_NO_WAIT))) {
		k_spin_unlock(&mutex_తాళం, కీలకం);

		SYS_పోర్టు_TRACING_OBJ_FUNC_EXIT(k_mutex, తాళం, mutex, timeout, -EBUSY);

		తిరిగి -EBUSY;
	}

	SYS_పోర్టు_TRACING_OBJ_FUNC_BLOCKING(k_mutex, తాళం, mutex, timeout);

#ఒకవేళ Z_MUTEX_PI_ENABLED
	/* Record the mutex this thread is blocking on for priority inheritance. */
	_current->mutex_pended_on = mutex;

	/*
	 * Walk the ownership chain, boosting each owner's priority to match
	 * the highest-priority waiter. Detects deadlock cycles along the way.
	 */
	{
		int32_t boost_prio = new_prio_for_inheritance(
					_current->పునాది.prio,
					mutex->owner->పునాది.prio);
		నిర్మాణం k_mutex  *chain_mutex = mutex;
		నిర్మాణం k_thread *chain_owner = mutex->owner;
		పూర్ణాంకం hops = 0;
#ఒకవేళ defined(CONFIG_MUTEX_DEADLOCK_DETECT)
		bool all_forever = K_TIMEOUT_EQ(timeout, K_FOREVER);
#అయితే_ముగింపు

		LOG_DBG("chain boosting prio on mutex %p", mutex);

		అయితే (chain_owner != ఖాళీ) {
#ఒకవేళ defined(CONFIG_MUTEX_DEADLOCK_DETECT)
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
				      z_is_inactive_timeout(&chain_owner->పునాది.timeout);

			ఒకవేళ (all_forever &&
			    chain_owner->mutex_pended_on != ఖాళీ &&
			    z_is_thread_pending(chain_owner) &&
			    chain_owner->mutex_pended_on->owner == _current) {
				__ASSERT(false,
					"mutex deadlock: thread %p waiting on "
					"mutex %p (owner %p)",
					_current, mutex, mutex->owner);
				ఆపు;
			}
#అయితే_ముగింపు /* CONFIG_MUTEX_DEADLOCK_DETECT */

			ఒకవేళ (z_is_prio_higher(boost_prio, chain_owner->పునాది.prio)) {
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
			ఒకవేళ (chain_owner->mutex_pended_on == ఖాళీ ||
			    !z_is_thread_pending(chain_owner)) {
				ఆపు;
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
			ఒకవేళ (chain_mutex == mutex ||
			    ++hops >= MUTEX_CHAIN_WALK_MAX_HOPS) {
				ఆపు;
			}
			chain_owner = chain_mutex->owner;
		}
	}
#అయితే_ముగింపు

	పూర్ణాంకం got_mutex = z_pend_curr(&mutex_తాళం, కీలకం, &mutex->wait_q, timeout);

	LOG_DBG("on mutex %p got_mutex value: %d", mutex, got_mutex);

	LOG_DBG("%p got mutex %p (y/n): %c", _current, mutex,
		got_mutex ? 'y' : 'n');

#ఒకవేళ Z_MUTEX_PI_ENABLED
	ఒకవేళ (got_mutex == 0) {
		/* Lock granted; clear the pending mutex pointer. */
		_current->mutex_pended_on = ఖాళీ;
		SYS_పోర్టు_TRACING_OBJ_FUNC_EXIT(k_mutex, తాళం, mutex, timeout, 0);
		తిరిగి 0;
	}

	/* timed out */

	LOG_DBG("%p timeout on mutex %p", _current, mutex);

	కీలకం = k_spin_తాళం(&mutex_తాళం);

	/*
	 * Clear mutex_pended_on under mutex_lock so no concurrent chain walk
	 * can observe the stale pointer and detect a false deadlock cycle.
	 */
	_current->mutex_pended_on = ఖాళీ;

	/*
	 * Check if mutex was unlocked after this thread was unpended.
	 * If so, skip adjusting owner's priority down.
	 */
	ఒకవేళ (likely(mutex->owner != ఖాళీ)) {
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

	ఒకవేళ (resched) {
		z_reschedule(&mutex_తాళం, కీలకం);
	} లేకపోతే {
		k_spin_unlock(&mutex_తాళం, కీలకం);
	}

	SYS_పోర్టు_TRACING_OBJ_FUNC_EXIT(k_mutex, తాళం, mutex, timeout, -EAGAIN);

	తిరిగి -EAGAIN;
#లేకపోతే
	SYS_పోర్టు_TRACING_OBJ_FUNC_EXIT(k_mutex, తాళం, mutex, timeout, got_mutex);

	తిరిగి got_mutex;
#అయితే_ముగింపు
}

#నిర్వచితమైతే CONFIG_USERSPACE
స్థిర అంతఃవరుస పూర్ణాంకం z_vrfy_k_mutex_తాళం(నిర్మాణం k_mutex *mutex,
				      k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(mutex, K_OBJ_MUTEX));
	తిరిగి z_impl_k_mutex_తాళం(mutex, timeout);
}
#అన్తర్భావ <zephyr/syscalls/k_mutex_lock_mrsh.c>
#అయితే_ముగింపు /* CONFIG_USERSPACE */

పూర్ణాంకం z_impl_k_mutex_unlock(నిర్మాణం k_mutex *mutex)
{
	నిర్మాణం k_thread *new_owner = ఖాళీ;

	__ASSERT(!arch_is_లోపల_isr(), "mutexes cannot be used inside ISRs");

	SYS_పోర్టు_TRACING_OBJ_FUNC_ENTER(k_mutex, unlock, mutex);

	CHECKIF(mutex->owner == ఖాళీ) {
		SYS_పోర్టు_TRACING_OBJ_FUNC_EXIT(k_mutex, unlock, mutex, -EINVAL);

		తిరిగి -EINVAL;
	}
	/*
	 * The current thread does not own the mutex.
	 */
	CHECKIF(mutex->owner != _current) {
		SYS_పోర్టు_TRACING_OBJ_FUNC_EXIT(k_mutex, unlock, mutex, -EPERM);

		తిరిగి -EPERM;
	}

	/*
	 * Attempt to unlock a mutex which is unlocked. mutex->lock_count
	 * cannot be zero if the current thread is equal to mutex->owner,
	 * therefore no underflow check is required. Use assert to catch
	 * undefined behavior.
	 */
	__ASSERT_NO_MSG(mutex->తాళం_లెక్క > 0U);

	LOG_DBG("mutex %p lock_count: %d", mutex, mutex->తాళం_లెక్క);

	/*
	 * If we are the owner and count is greater than 1, then decrement
	 * the count and return and keep current thread as the owner.
	 */
	ఒకవేళ (mutex->తాళం_లెక్క > 1U) {
		mutex->తాళం_లెక్క--;
		వెళ్ళు k_mutex_unlock_return;
	}

	k_spinlock_కీలకం_t కీలకం = k_spin_తాళం(&mutex_తాళం);

#ఒకవేళ Z_MUTEX_PI_ENABLED
	/* Remove this mutex from the owner's list of held mutexes */
	sys_slist_కనుగొను_and_remove(&_current->held_mutexes, &mutex->held_కణుపు);

	/*
	 * If the thread's priority was boosted, recalculate it by scanning
	 * the remaining held mutexes. The released mutex has already been
	 * removed from held_mutexes, so it is not included in the scan.
	 */
	ఒకవేళ (_current->పునాది.prio != _current->orig_prio) {
		int32_t new_prio = held_mutexes_highest_waiter_prio(
					_current, _current->orig_prio);

		adjust_owner_prio(mutex, new_prio);
	}
#అయితే_ముగింపు

	/* Pick the new owner (if any) and complete the wake atomically
	 * under the scheduler's spinlock, so a racing in-flight timeout
	 * handler cannot observe a half-initialized wake-up.
	 */
	తాళం_SCHED_SPINLOCK {
		new_owner = z_unpend_మొదటి_thread_locked(&mutex->wait_q);
		mutex->owner = new_owner;

		LOG_DBG("new owner of mutex %p: %p (prio: %d)",
			mutex, new_owner, new_owner ? new_owner->పునాది.prio : -1000);

		ఒకవేళ (unlikely(new_owner != ఖాళీ)) {
			/*
			 * new owner is already of higher or equal prio than first
			 * waiter since the wait queue is priority-based: no need to
			 * adjust its priority
			 */
#ఒకవేళ Z_MUTEX_PI_ENABLED
			ఒకవేళ (sys_slist_is_ఖాళీలు(&new_owner->held_mutexes)) {
				new_owner->orig_prio = new_owner->పునాది.prio;
			}
			sys_slist_append(&new_owner->held_mutexes, &mutex->held_కణుపు);
			new_owner->mutex_pended_on = ఖాళీ;
#అయితే_ముగింపు
			arch_thread_return_విలువ_సమితి(new_owner, 0);
			z_sched_ready_locked(new_owner);
		} లేకపోతే {
			mutex->తాళం_లెక్క = 0U;
		}
	}

	ఒకవేళ (unlikely(new_owner != ఖాళీ)) {
		z_reschedule(&mutex_తాళం, కీలకం);
	} లేకపోతే {
		k_spin_unlock(&mutex_తాళం, కీలకం);
	}


k_mutex_unlock_return:
	SYS_పోర్టు_TRACING_OBJ_FUNC_EXIT(k_mutex, unlock, mutex, 0);

	తిరిగి 0;
}

#నిర్వచితమైతే CONFIG_USERSPACE
స్థిర అంతఃవరుస పూర్ణాంకం z_vrfy_k_mutex_unlock(నిర్మాణం k_mutex *mutex)
{
	K_OOPS(K_SYSCALL_OBJ(mutex, K_OBJ_MUTEX));
	తిరిగి z_impl_k_mutex_unlock(mutex);
}
#అన్తర్భావ <zephyr/syscalls/k_mutex_unlock_mrsh.c>
#అయితే_ముగింపు /* CONFIG_USERSPACE */
