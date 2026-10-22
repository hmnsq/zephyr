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

#अन्तर्भाव <zephyr/kernel.h>
#अन्तर्भाव <zephyr/toolchain.h>
#अन्तर्भाव <kspinlock.h>
#अन्तर्भाव <ksched.h>
#अन्तर्भाव <scheduler.h>
#अन्तर्भाव <kthread.h>
#अन्तर्भाव <wait_q.h>
#अन्तर्भाव <errno.h>
#अन्तर्भाव <zephyr/init.h>
#अन्तर्भाव <zephyr/internal/syscall_handler.h>
#अन्तर्भाव <zephyr/tracing/tracing.h>
#अन्तर्भाव <zephyr/sys/check.h>
#अन्तर्भाव <zephyr/logging/log.h>
#अन्तर्भाव <zephyr/llext/symbol.h>
LOG_MODULE_DECLARE(os, CONFIG_KERNEL_LOG_LEVEL);

/*
 * We use a global spinlock rather than a per-object lock because the
 * priority-inheritance chain walk, the held-mutexes recalculation, the
 * K_FOREVER deadlock check, and the unlock handoff all read or write
 * fields (owner, mutex_pended_on, held_mutexes) belonging to mutexes
 * other than the one passed to the API entry point.
 */
स्थिर संरचना k_spinlock mutex_तालकम्;

#यदि_परिभाषितम् CONFIG_OBJ_CORE_MUTEX
K_OBJ_TYPE_DEFINE(obj_type_mutex, k_mutex, K_OBJ_TYPE_MUTEX_ID, रिक्त);
#यदि_अन्तः /* CONFIG_OBJ_CORE_MUTEX */

/*
 * Reset the mutex under mutex_lock so a concurrent k_mutex_lock() cannot
 * observe a half-initialized object. With refuse_if_owned set, a mutex that
 * has an owner is left untouched: resetting it would leave its held_node
 * linked in the owner's held_mutexes list and reset the wait queue under
 * pending waiters (threads only pend while the mutex has an owner).
 */
स्थिर पूर्णाङ्क mutex_init(संरचना k_mutex *mutex, bool refuse_if_owned)
{
	k_spinlock_कुञ्चिका_t कुञ्चिका = k_spin_तालकम्(&mutex_तालकम्);

	यदि (refuse_if_owned && (mutex->owner != रिक्त)) {
		k_spin_unlock(&mutex_तालकम्, कुञ्चिका);
		प्रत्यागम -EBUSY;
	}

	mutex->owner = रिक्त;
	mutex->तालकम्_गणनम् = 0U;

	z_waitq_init(&mutex->wait_q);

#यदि Z_MUTEX_PI_ENABLED
	mutex->held_ग्रन्थिः.अग्र = रिक्त;
#यदि_अन्तः

	k_spin_unlock(&mutex_तालकम्, कुञ्चिका);

	k_object_init(mutex);

#यदि_परिभाषितम् CONFIG_OBJ_CORE_MUTEX
	k_obj_core_init_and_link(K_OBJ_CORE(mutex), &obj_type_mutex);
#यदि_अन्तः /* CONFIG_OBJ_CORE_MUTEX */

	SYS_पत्तनम्_TRACING_OBJ_INIT(k_mutex, mutex, 0);

	प्रत्यागम 0;
}

पूर्णाङ्क z_impl_k_mutex_init(संरचना k_mutex *mutex)
{
	प्रत्यागम mutex_init(mutex, false);
}

#यदि_परिभाषितम् CONFIG_USERSPACE
स्थिर अन्तःस्थ पूर्णाङ्क z_vrfy_k_mutex_init(संरचना k_mutex *mutex)
{
	bool initialized;

	K_OOPS(K_SYSCALL_OBJ_INIT(mutex, K_OBJ_MUTEX));

	/* The owner field is only meaningful once the mutex was initialized */
	initialized = (k_object_find(mutex)->flags & K_OBJ_FLAG_INITIALIZED) != 0U;

	K_OOPS(K_SYSCALL_VERIFY_सन्देशः(mutex_init(mutex, initialized) == 0,
				    "mutex %p is locked", mutex));

	प्रत्यागम 0;
}
#अन्तर्भाव <zephyr/syscalls/k_mutex_init_mrsh.c>
#यदि_अन्तः /* CONFIG_USERSPACE */

#यदि Z_MUTEX_PI_ENABLED
/*
 * Upper bound on ownership-chain walk hops. This is a bound on walk cost
 * under mutex_lock, not a real limit on chain length -- equal-priority
 * threads can legally form longer chains.
 */
#परिभाषय MUTEX_CHAIN_WALK_MAX_HOPS 16

स्थिर int32_t new_prio_for_inheritance(int32_t target, int32_t सीमा)
{
	पूर्णाङ्क new_prio = z_is_prio_higher(target, सीमा) ? target : सीमा;

	new_prio = z_get_new_prio_with_ceiling(new_prio);

	प्रत्यागम new_prio;
}

स्थिर bool adjust_owner_prio(संरचना k_mutex *mutex, int32_t new_prio)
{
	यदि (mutex->owner->आधारः.prio != new_prio) {

		LOG_DBG("%p (ready (y/n): %c) prio changed to %d (was %d)",
			mutex->owner, z_is_thread_ready(mutex->owner) ?
			'y' : 'n',
			new_prio, mutex->owner->आधारः.prio);

		प्रत्यागम z_thread_prio_समुच्चयः(mutex->owner, new_prio);
	}
	प्रत्यागम false;
}

/*
 * Scan all mutexes held by a thread and return the highest priority
 * among all their waiters, floored at floor_prio.
 */
स्थिर int32_t held_mutexes_highest_waiter_prio(संरचना k_thread *thread,
						 int32_t floor_prio)
{
	sys_snode_t *ग्रन्थिः;
	int32_t prio = floor_prio;

	तालकम्_SCHED_SPINLOCK {
		SYS_SLIST_FOR_EACH_ग्रन्थिः(&thread->held_mutexes, ग्रन्थिः) {
			संरचना k_mutex *m =
				CONTAINER_सम्बन्धि(ग्रन्थिः, संरचना k_mutex, held_ग्रन्थिः);
			संरचना k_thread *waiter = z_waitq_शीर्ष_locked(&m->wait_q);

			यदि (waiter != रिक्त) {
				int32_t wprio = z_get_new_prio_with_ceiling(waiter->आधारः.prio);

				यदि (z_is_prio_higher(wprio, prio)) {
					prio = wprio;
				}
			}
		}
	}
	प्रत्यागम prio;
}
#यदि_अन्तः

पूर्णाङ्क z_impl_k_mutex_तालकम्(संरचना k_mutex *mutex, k_timeout_t timeout)
{
	k_spinlock_कुञ्चिका_t कुञ्चिका;
#यदि Z_MUTEX_PI_ENABLED
	bool resched = false;
#यदि_अन्तः

	__ASSERT(!arch_is_अन्तर्_isr(), "mutexes cannot be used inside ISRs");

	SYS_पत्तनम्_TRACING_OBJ_FUNC_ENTER(k_mutex, तालकम्, mutex, timeout);

	कुञ्चिका = k_spin_तालकम्(&mutex_तालकम्);

	यदि (likely((mutex->तालकम्_गणनम् == 0U) || (mutex->owner == _current))) {

		mutex->तालकम्_गणनम्++;
		mutex->owner = _current;

#यदि Z_MUTEX_PI_ENABLED
		LOG_DBG("%p took mutex %p, count: %d, orig prio: %d",
			_current, mutex, mutex->तालकम्_गणनम्,
			_current->orig_prio);

		/* Add to held list on first acquisition; recursive locks skip this. */
		यदि (mutex->तालकम्_गणनम् == 1U) {
			/* Capture the thread's true pre-inheritance priority
			 * the first time it acquires any mutex.
			 */
			यदि (sys_slist_is_रिक्तानि(&_current->held_mutexes)) {
				_current->orig_prio = _current->आधारः.prio;
			}
			sys_slist_append(&_current->held_mutexes, &mutex->held_ग्रन्थिः);
		}
#अन्यथा
		LOG_DBG("%p took mutex %p, count: %d",
			_current, mutex, mutex->तालकम्_गणनम्);
#यदि_अन्तः

		k_spin_unlock(&mutex_तालकम्, कुञ्चिका);

		SYS_पत्तनम्_TRACING_OBJ_FUNC_EXIT(k_mutex, तालकम्, mutex, timeout, 0);

		प्रत्यागम 0;
	}

	यदि (unlikely(K_TIMEOUT_EQ(timeout, K_NO_WAIT))) {
		k_spin_unlock(&mutex_तालकम्, कुञ्चिका);

		SYS_पत्तनम्_TRACING_OBJ_FUNC_EXIT(k_mutex, तालकम्, mutex, timeout, -EBUSY);

		प्रत्यागम -EBUSY;
	}

	SYS_पत्तनम्_TRACING_OBJ_FUNC_BLOCKING(k_mutex, तालकम्, mutex, timeout);

#यदि Z_MUTEX_PI_ENABLED
	/* Record the mutex this thread is blocking on for priority inheritance. */
	_current->mutex_pended_on = mutex;

	/*
	 * Walk the ownership chain, boosting each owner's priority to match
	 * the highest-priority waiter. Detects deadlock cycles along the way.
	 */
	{
		int32_t boost_prio = new_prio_for_inheritance(
					_current->आधारः.prio,
					mutex->owner->आधारः.prio);
		संरचना k_mutex  *chain_mutex = mutex;
		संरचना k_thread *chain_owner = mutex->owner;
		पूर्णाङ्क hops = 0;
#यदि defined(CONFIG_MUTEX_DEADLOCK_DETECT)
		bool all_forever = K_TIMEOUT_EQ(timeout, K_FOREVER);
#यदि_अन्तः

		LOG_DBG("chain boosting prio on mutex %p", mutex);

		यावत् (chain_owner != रिक्त) {
#यदि defined(CONFIG_MUTEX_DEADLOCK_DETECT)
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
				      z_is_inactive_timeout(&chain_owner->आधारः.timeout);

			यदि (all_forever &&
			    chain_owner->mutex_pended_on != रिक्त &&
			    z_is_thread_pending(chain_owner) &&
			    chain_owner->mutex_pended_on->owner == _current) {
				__ASSERT(false,
					"mutex deadlock: thread %p waiting on "
					"mutex %p (owner %p)",
					_current, mutex, mutex->owner);
				भङ्ग;
			}
#यदि_अन्तः /* CONFIG_MUTEX_DEADLOCK_DETECT */

			यदि (z_is_prio_higher(boost_prio, chain_owner->आधारः.prio)) {
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
			यदि (chain_owner->mutex_pended_on == रिक्त ||
			    !z_is_thread_pending(chain_owner)) {
				भङ्ग;
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
			यदि (chain_mutex == mutex ||
			    ++hops >= MUTEX_CHAIN_WALK_MAX_HOPS) {
				भङ्ग;
			}
			chain_owner = chain_mutex->owner;
		}
	}
#यदि_अन्तः

	पूर्णाङ्क got_mutex = z_pend_curr(&mutex_तालकम्, कुञ्चिका, &mutex->wait_q, timeout);

	LOG_DBG("on mutex %p got_mutex value: %d", mutex, got_mutex);

	LOG_DBG("%p got mutex %p (y/n): %c", _current, mutex,
		got_mutex ? 'y' : 'n');

#यदि Z_MUTEX_PI_ENABLED
	यदि (got_mutex == 0) {
		/* Lock granted; clear the pending mutex pointer. */
		_current->mutex_pended_on = रिक्त;
		SYS_पत्तनम्_TRACING_OBJ_FUNC_EXIT(k_mutex, तालकम्, mutex, timeout, 0);
		प्रत्यागम 0;
	}

	/* timed out */

	LOG_DBG("%p timeout on mutex %p", _current, mutex);

	कुञ्चिका = k_spin_तालकम्(&mutex_तालकम्);

	/*
	 * Clear mutex_pended_on under mutex_lock so no concurrent chain walk
	 * can observe the stale pointer and detect a false deadlock cycle.
	 */
	_current->mutex_pended_on = रिक्त;

	/*
	 * Check if mutex was unlocked after this thread was unpended.
	 * If so, skip adjusting owner's priority down.
	 */
	यदि (likely(mutex->owner != रिक्त)) {
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

	यदि (resched) {
		z_reschedule(&mutex_तालकम्, कुञ्चिका);
	} अन्यथा {
		k_spin_unlock(&mutex_तालकम्, कुञ्चिका);
	}

	SYS_पत्तनम्_TRACING_OBJ_FUNC_EXIT(k_mutex, तालकम्, mutex, timeout, -EAGAIN);

	प्रत्यागम -EAGAIN;
#अन्यथा
	SYS_पत्तनम्_TRACING_OBJ_FUNC_EXIT(k_mutex, तालकम्, mutex, timeout, got_mutex);

	प्रत्यागम got_mutex;
#यदि_अन्तः
}

#यदि_परिभाषितम् CONFIG_USERSPACE
स्थिर अन्तःस्थ पूर्णाङ्क z_vrfy_k_mutex_तालकम्(संरचना k_mutex *mutex,
				      k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(mutex, K_OBJ_MUTEX));
	प्रत्यागम z_impl_k_mutex_तालकम्(mutex, timeout);
}
#अन्तर्भाव <zephyr/syscalls/k_mutex_lock_mrsh.c>
#यदि_अन्तः /* CONFIG_USERSPACE */

पूर्णाङ्क z_impl_k_mutex_unlock(संरचना k_mutex *mutex)
{
	संरचना k_thread *new_owner = रिक्त;

	__ASSERT(!arch_is_अन्तर्_isr(), "mutexes cannot be used inside ISRs");

	SYS_पत्तनम्_TRACING_OBJ_FUNC_ENTER(k_mutex, unlock, mutex);

	CHECKIF(mutex->owner == रिक्त) {
		SYS_पत्तनम्_TRACING_OBJ_FUNC_EXIT(k_mutex, unlock, mutex, -EINVAL);

		प्रत्यागम -EINVAL;
	}
	/*
	 * The current thread does not own the mutex.
	 */
	CHECKIF(mutex->owner != _current) {
		SYS_पत्तनम्_TRACING_OBJ_FUNC_EXIT(k_mutex, unlock, mutex, -EPERM);

		प्रत्यागम -EPERM;
	}

	/*
	 * Attempt to unlock a mutex which is unlocked. mutex->lock_count
	 * cannot be zero if the current thread is equal to mutex->owner,
	 * therefore no underflow check is required. Use assert to catch
	 * undefined behavior.
	 */
	__ASSERT_NO_MSG(mutex->तालकम्_गणनम् > 0U);

	LOG_DBG("mutex %p lock_count: %d", mutex, mutex->तालकम्_गणनम्);

	/*
	 * If we are the owner and count is greater than 1, then decrement
	 * the count and return and keep current thread as the owner.
	 */
	यदि (mutex->तालकम्_गणनम् > 1U) {
		mutex->तालकम्_गणनम्--;
		गच्छ k_mutex_unlock_return;
	}

	k_spinlock_कुञ्चिका_t कुञ्चिका = k_spin_तालकम्(&mutex_तालकम्);

#यदि Z_MUTEX_PI_ENABLED
	/* Remove this mutex from the owner's list of held mutexes */
	sys_slist_find_and_remove(&_current->held_mutexes, &mutex->held_ग्रन्थिः);

	/*
	 * If the thread's priority was boosted, recalculate it by scanning
	 * the remaining held mutexes. The released mutex has already been
	 * removed from held_mutexes, so it is not included in the scan.
	 */
	यदि (_current->आधारः.prio != _current->orig_prio) {
		int32_t new_prio = held_mutexes_highest_waiter_prio(
					_current, _current->orig_prio);

		adjust_owner_prio(mutex, new_prio);
	}
#यदि_अन्तः

	/* Pick the new owner (if any) and complete the wake atomically
	 * under the scheduler's spinlock, so a racing in-flight timeout
	 * handler cannot observe a half-initialized wake-up.
	 */
	तालकम्_SCHED_SPINLOCK {
		new_owner = z_unpend_first_thread_locked(&mutex->wait_q);
		mutex->owner = new_owner;

		LOG_DBG("new owner of mutex %p: %p (prio: %d)",
			mutex, new_owner, new_owner ? new_owner->आधारः.prio : -1000);

		यदि (unlikely(new_owner != रिक्त)) {
			/*
			 * new owner is already of higher or equal prio than first
			 * waiter since the wait queue is priority-based: no need to
			 * adjust its priority
			 */
#यदि Z_MUTEX_PI_ENABLED
			यदि (sys_slist_is_रिक्तानि(&new_owner->held_mutexes)) {
				new_owner->orig_prio = new_owner->आधारः.prio;
			}
			sys_slist_append(&new_owner->held_mutexes, &mutex->held_ग्रन्थिः);
			new_owner->mutex_pended_on = रिक्त;
#यदि_अन्तः
			arch_thread_return_मानम्_समुच्चयः(new_owner, 0);
			z_sched_ready_locked(new_owner);
		} अन्यथा {
			mutex->तालकम्_गणनम् = 0U;
		}
	}

	यदि (unlikely(new_owner != रिक्त)) {
		z_reschedule(&mutex_तालकम्, कुञ्चिका);
	} अन्यथा {
		k_spin_unlock(&mutex_तालकम्, कुञ्चिका);
	}


k_mutex_unlock_return:
	SYS_पत्तनम्_TRACING_OBJ_FUNC_EXIT(k_mutex, unlock, mutex, 0);

	प्रत्यागम 0;
}

#यदि_परिभाषितम् CONFIG_USERSPACE
स्थिर अन्तःस्थ पूर्णाङ्क z_vrfy_k_mutex_unlock(संरचना k_mutex *mutex)
{
	K_OOPS(K_SYSCALL_OBJ(mutex, K_OBJ_MUTEX));
	प्रत्यागम z_impl_k_mutex_unlock(mutex);
}
#अन्तर्भाव <zephyr/syscalls/k_mutex_unlock_mrsh.c>
#यदि_अन्तः /* CONFIG_USERSPACE */
