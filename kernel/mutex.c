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

#অন্তর্ভাব <zephyr/kernel.h>
#অন্তর্ভাব <zephyr/toolchain.h>
#অন্তর্ভাব <kspinlock.h>
#অন্তর্ভাব <ksched.h>
#অন্তর্ভাব <scheduler.h>
#অন্তর্ভাব <kthread.h>
#অন্তর্ভাব <wait_q.h>
#অন্তর্ভাব <errno.h>
#অন্তর্ভাব <zephyr/init.h>
#অন্তর্ভাব <zephyr/internal/syscall_handler.h>
#অন্তর্ভাব <zephyr/tracing/tracing.h>
#অন্তর্ভাব <zephyr/sys/check.h>
#অন্তর্ভাব <zephyr/logging/log.h>
#অন্তর্ভাব <zephyr/llext/symbol.h>
LOG_MODULE_DECLARE(os, CONFIG_KERNEL_LOG_LEVEL);

/*
 * We use a global spinlock rather than a per-object lock because the
 * priority-inheritance chain walk, the held-mutexes recalculation, the
 * K_FOREVER deadlock check, and the unlock handoff all read or write
 * fields (owner, mutex_pended_on, held_mutexes) belonging to mutexes
 * other than the one passed to the API entry point.
 */
স্থির কাঠামো k_spinlock mutex_lock;

#ifdef CONFIG_OBJ_CORE_MUTEX
K_OBJ_TYPE_DEFINE(obj_type_mutex, k_mutex, K_OBJ_TYPE_MUTEX_ID, ফাঁকা);
#endif /* CONFIG_OBJ_CORE_MUTEX */

/*
 * Reset the mutex under mutex_lock so a concurrent k_mutex_lock() cannot
 * observe a half-initialized object. With refuse_if_owned set, a mutex that
 * has an owner is left untouched: resetting it would leave its held_node
 * linked in the owner's held_mutexes list and reset the wait queue under
 * pending waiters (threads only pend while the mutex has an owner).
 */
স্থির পূর্ণসংখ্যা mutex_init(কাঠামো k_mutex *mutex, bool refuse_if_owned)
{
	k_spinlock_চাবি_t চাবি = k_spin_lock(&mutex_lock);

	যদি (refuse_if_owned && (mutex->owner != ফাঁকা)) {
		k_spin_unlock(&mutex_lock, চাবি);
		ফেরত -EBUSY;
	}

	mutex->owner = ফাঁকা;
	mutex->lock_গুনতি = 0U;

	z_waitq_init(&mutex->wait_q);

#যদি Z_MUTEX_PI_ENABLED
	mutex->held_গ্রন্থি.পরবর্তী = ফাঁকা;
#endif

	k_spin_unlock(&mutex_lock, চাবি);

	k_object_init(mutex);

#ifdef CONFIG_OBJ_CORE_MUTEX
	k_obj_core_init_and_link(K_OBJ_CORE(mutex), &obj_type_mutex);
#endif /* CONFIG_OBJ_CORE_MUTEX */

	SYS_PORT_TRACING_OBJ_INIT(k_mutex, mutex, 0);

	ফেরত 0;
}

পূর্ণসংখ্যা z_impl_k_mutex_init(কাঠামো k_mutex *mutex)
{
	ফেরত mutex_init(mutex, false);
}

#ifdef CONFIG_USERSPACE
স্থির অন্তঃরেখ পূর্ণসংখ্যা z_vrfy_k_mutex_init(কাঠামো k_mutex *mutex)
{
	bool initialized;

	K_OOPS(K_SYSCALL_OBJ_INIT(mutex, K_OBJ_MUTEX));

	/* The owner field is only meaningful once the mutex was initialized */
	initialized = (k_object_find(mutex)->flags & K_OBJ_FLAG_INITIALIZED) != 0U;

	K_OOPS(K_SYSCALL_VERIFY_বার্তা(mutex_init(mutex, initialized) == 0,
				    "mutex %p is locked", mutex));

	ফেরত 0;
}
#অন্তর্ভাব <zephyr/syscalls/k_mutex_init_mrsh.c>
#endif /* CONFIG_USERSPACE */

#যদি Z_MUTEX_PI_ENABLED
/*
 * Upper bound on ownership-chain walk hops. This is a bound on walk cost
 * under mutex_lock, not a real limit on chain length -- equal-priority
 * threads can legally form longer chains.
 */
#define MUTEX_CHAIN_WALK_MAX_HOPS 16

স্থির int32_t new_prio_for_inheritance(int32_t target, int32_t limit)
{
	পূর্ণসংখ্যা new_prio = z_is_prio_higher(target, limit) ? target : limit;

	new_prio = z_get_new_prio_with_ceiling(new_prio);

	ফেরত new_prio;
}

স্থির bool adjust_owner_prio(কাঠামো k_mutex *mutex, int32_t new_prio)
{
	যদি (mutex->owner->base.prio != new_prio) {

		LOG_DBG("%p (ready (y/n): %c) prio changed to %d (was %d)",
			mutex->owner, z_is_thread_ready(mutex->owner) ?
			'y' : 'n',
			new_prio, mutex->owner->base.prio);

		ফেরত z_thread_prio_set(mutex->owner, new_prio);
	}
	ফেরত false;
}

/*
 * Scan all mutexes held by a thread and return the highest priority
 * among all their waiters, floored at floor_prio.
 */
স্থির int32_t held_mutexes_highest_waiter_prio(কাঠামো k_thread *thread,
						 int32_t floor_prio)
{
	sys_snode_t *গ্রন্থি;
	int32_t prio = floor_prio;

	LOCK_SCHED_SPINLOCK {
		SYS_SLIST_FOR_EACH_গ্রন্থি(&thread->held_mutexes, গ্রন্থি) {
			কাঠামো k_mutex *m =
				CONTAINER_OF(গ্রন্থি, কাঠামো k_mutex, held_গ্রন্থি);
			কাঠামো k_thread *waiter = z_waitq_শীর্ষ_locked(&m->wait_q);

			যদি (waiter != ফাঁকা) {
				int32_t wprio = z_get_new_prio_with_ceiling(waiter->base.prio);

				যদি (z_is_prio_higher(wprio, prio)) {
					prio = wprio;
				}
			}
		}
	}
	ফেরত prio;
}
#endif

পূর্ণসংখ্যা z_impl_k_mutex_lock(কাঠামো k_mutex *mutex, k_timeout_t timeout)
{
	k_spinlock_চাবি_t চাবি;
#যদি Z_MUTEX_PI_ENABLED
	bool resched = false;
#endif

	__ASSERT(!arch_is_in_isr(), "mutexes cannot be used inside ISRs");

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_mutex, lock, mutex, timeout);

	চাবি = k_spin_lock(&mutex_lock);

	যদি (likely((mutex->lock_গুনতি == 0U) || (mutex->owner == _current))) {

		mutex->lock_গুনতি++;
		mutex->owner = _current;

#যদি Z_MUTEX_PI_ENABLED
		LOG_DBG("%p took mutex %p, count: %d, orig prio: %d",
			_current, mutex, mutex->lock_গুনতি,
			_current->orig_prio);

		/* Add to held list on first acquisition; recursive locks skip this. */
		যদি (mutex->lock_গুনতি == 1U) {
			/* Capture the thread's true pre-inheritance priority
			 * the first time it acquires any mutex.
			 */
			যদি (sys_slist_is_empty(&_current->held_mutexes)) {
				_current->orig_prio = _current->base.prio;
			}
			sys_slist_append(&_current->held_mutexes, &mutex->held_গ্রন্থি);
		}
#অন্যথা
		LOG_DBG("%p took mutex %p, count: %d",
			_current, mutex, mutex->lock_গুনতি);
#endif

		k_spin_unlock(&mutex_lock, চাবি);

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mutex, lock, mutex, timeout, 0);

		ফেরত 0;
	}

	যদি (unlikely(K_TIMEOUT_EQ(timeout, K_NO_WAIT))) {
		k_spin_unlock(&mutex_lock, চাবি);

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mutex, lock, mutex, timeout, -EBUSY);

		ফেরত -EBUSY;
	}

	SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_mutex, lock, mutex, timeout);

#যদি Z_MUTEX_PI_ENABLED
	/* Record the mutex this thread is blocking on for priority inheritance. */
	_current->mutex_pended_on = mutex;

	/*
	 * Walk the ownership chain, boosting each owner's priority to match
	 * the highest-priority waiter. Detects deadlock cycles along the way.
	 */
	{
		int32_t boost_prio = new_prio_for_inheritance(
					_current->base.prio,
					mutex->owner->base.prio);
		কাঠামো k_mutex  *chain_mutex = mutex;
		কাঠামো k_thread *chain_owner = mutex->owner;
		পূর্ণসংখ্যা hops = 0;
#যদি defined(CONFIG_MUTEX_DEADLOCK_DETECT)
		bool all_forever = K_TIMEOUT_EQ(timeout, K_FOREVER);
#endif

		LOG_DBG("chain boosting prio on mutex %p", mutex);

		যখন (chain_owner != ফাঁকা) {
#যদি defined(CONFIG_MUTEX_DEADLOCK_DETECT)
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
				      z_is_inactive_timeout(&chain_owner->base.timeout);

			যদি (all_forever &&
			    chain_owner->mutex_pended_on != ফাঁকা &&
			    z_is_thread_pending(chain_owner) &&
			    chain_owner->mutex_pended_on->owner == _current) {
				__ASSERT(false,
					"mutex deadlock: thread %p waiting on "
					"mutex %p (owner %p)",
					_current, mutex, mutex->owner);
				বিরতি;
			}
#endif /* CONFIG_MUTEX_DEADLOCK_DETECT */

			যদি (z_is_prio_higher(boost_prio, chain_owner->base.prio)) {
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
			যদি (chain_owner->mutex_pended_on == ফাঁকা ||
			    !z_is_thread_pending(chain_owner)) {
				বিরতি;
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
			যদি (chain_mutex == mutex ||
			    ++hops >= MUTEX_CHAIN_WALK_MAX_HOPS) {
				বিরতি;
			}
			chain_owner = chain_mutex->owner;
		}
	}
#endif

	পূর্ণসংখ্যা got_mutex = z_pend_curr(&mutex_lock, চাবি, &mutex->wait_q, timeout);

	LOG_DBG("on mutex %p got_mutex value: %d", mutex, got_mutex);

	LOG_DBG("%p got mutex %p (y/n): %c", _current, mutex,
		got_mutex ? 'y' : 'n');

#যদি Z_MUTEX_PI_ENABLED
	যদি (got_mutex == 0) {
		/* Lock granted; clear the pending mutex pointer. */
		_current->mutex_pended_on = ফাঁকা;
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mutex, lock, mutex, timeout, 0);
		ফেরত 0;
	}

	/* timed out */

	LOG_DBG("%p timeout on mutex %p", _current, mutex);

	চাবি = k_spin_lock(&mutex_lock);

	/*
	 * Clear mutex_pended_on under mutex_lock so no concurrent chain walk
	 * can observe the stale pointer and detect a false deadlock cycle.
	 */
	_current->mutex_pended_on = ফাঁকা;

	/*
	 * Check if mutex was unlocked after this thread was unpended.
	 * If so, skip adjusting owner's priority down.
	 */
	যদি (likely(mutex->owner != ফাঁকা)) {
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

	যদি (resched) {
		z_reschedule(&mutex_lock, চাবি);
	} অন্যথা {
		k_spin_unlock(&mutex_lock, চাবি);
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mutex, lock, mutex, timeout, -EAGAIN);

	ফেরত -EAGAIN;
#অন্যথা
	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mutex, lock, mutex, timeout, got_mutex);

	ফেরত got_mutex;
#endif
}

#ifdef CONFIG_USERSPACE
স্থির অন্তঃরেখ পূর্ণসংখ্যা z_vrfy_k_mutex_lock(কাঠামো k_mutex *mutex,
				      k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(mutex, K_OBJ_MUTEX));
	ফেরত z_impl_k_mutex_lock(mutex, timeout);
}
#অন্তর্ভাব <zephyr/syscalls/k_mutex_lock_mrsh.c>
#endif /* CONFIG_USERSPACE */

পূর্ণসংখ্যা z_impl_k_mutex_unlock(কাঠামো k_mutex *mutex)
{
	কাঠামো k_thread *new_owner = ফাঁকা;

	__ASSERT(!arch_is_in_isr(), "mutexes cannot be used inside ISRs");

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_mutex, unlock, mutex);

	CHECKIF(mutex->owner == ফাঁকা) {
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mutex, unlock, mutex, -EINVAL);

		ফেরত -EINVAL;
	}
	/*
	 * The current thread does not own the mutex.
	 */
	CHECKIF(mutex->owner != _current) {
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mutex, unlock, mutex, -EPERM);

		ফেরত -EPERM;
	}

	/*
	 * Attempt to unlock a mutex which is unlocked. mutex->lock_count
	 * cannot be zero if the current thread is equal to mutex->owner,
	 * therefore no underflow check is required. Use assert to catch
	 * undefined behavior.
	 */
	__ASSERT_NO_MSG(mutex->lock_গুনতি > 0U);

	LOG_DBG("mutex %p lock_count: %d", mutex, mutex->lock_গুনতি);

	/*
	 * If we are the owner and count is greater than 1, then decrement
	 * the count and return and keep current thread as the owner.
	 */
	যদি (mutex->lock_গুনতি > 1U) {
		mutex->lock_গুনতি--;
		যাও k_mutex_unlock_return;
	}

	k_spinlock_চাবি_t চাবি = k_spin_lock(&mutex_lock);

#যদি Z_MUTEX_PI_ENABLED
	/* Remove this mutex from the owner's list of held mutexes */
	sys_slist_find_and_remove(&_current->held_mutexes, &mutex->held_গ্রন্থি);

	/*
	 * If the thread's priority was boosted, recalculate it by scanning
	 * the remaining held mutexes. The released mutex has already been
	 * removed from held_mutexes, so it is not included in the scan.
	 */
	যদি (_current->base.prio != _current->orig_prio) {
		int32_t new_prio = held_mutexes_highest_waiter_prio(
					_current, _current->orig_prio);

		adjust_owner_prio(mutex, new_prio);
	}
#endif

	/* Pick the new owner (if any) and complete the wake atomically
	 * under the scheduler's spinlock, so a racing in-flight timeout
	 * handler cannot observe a half-initialized wake-up.
	 */
	LOCK_SCHED_SPINLOCK {
		new_owner = z_unpend_first_thread_locked(&mutex->wait_q);
		mutex->owner = new_owner;

		LOG_DBG("new owner of mutex %p: %p (prio: %d)",
			mutex, new_owner, new_owner ? new_owner->base.prio : -1000);

		যদি (unlikely(new_owner != ফাঁকা)) {
			/*
			 * new owner is already of higher or equal prio than first
			 * waiter since the wait queue is priority-based: no need to
			 * adjust its priority
			 */
#যদি Z_MUTEX_PI_ENABLED
			যদি (sys_slist_is_empty(&new_owner->held_mutexes)) {
				new_owner->orig_prio = new_owner->base.prio;
			}
			sys_slist_append(&new_owner->held_mutexes, &mutex->held_গ্রন্থি);
			new_owner->mutex_pended_on = ফাঁকা;
#endif
			arch_thread_return_মান_set(new_owner, 0);
			z_sched_ready_locked(new_owner);
		} অন্যথা {
			mutex->lock_গুনতি = 0U;
		}
	}

	যদি (unlikely(new_owner != ফাঁকা)) {
		z_reschedule(&mutex_lock, চাবি);
	} অন্যথা {
		k_spin_unlock(&mutex_lock, চাবি);
	}


k_mutex_unlock_return:
	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_mutex, unlock, mutex, 0);

	ফেরত 0;
}

#ifdef CONFIG_USERSPACE
স্থির অন্তঃরেখ পূর্ণসংখ্যা z_vrfy_k_mutex_unlock(কাঠামো k_mutex *mutex)
{
	K_OOPS(K_SYSCALL_OBJ(mutex, K_OBJ_MUTEX));
	ফেরত z_impl_k_mutex_unlock(mutex);
}
#অন্তর্ভাব <zephyr/syscalls/k_mutex_unlock_mrsh.c>
#endif /* CONFIG_USERSPACE */
