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

#ضمن <zephyr/kernel.h>
#ضمن <zephyr/toolchain.h>
#ضمن <kspinlock.h>
#ضمن <ksched.h>
#ضمن <scheduler.h>
#ضمن <kthread.h>
#ضمن <wait_q.h>
#ضمن <errno.h>
#ضمن <zephyr/init.h>
#ضمن <zephyr/internal/syscall_handler.h>
#ضمن <zephyr/tracing/tracing.h>
#ضمن <zephyr/sys/check.h>
#ضمن <zephyr/logging/log.h>
#ضمن <zephyr/llext/symbol.h>
LOG_MODULE_DECLARE(os, CONFIG_KERNEL_LOG_LEVEL);

/*
 * We use a global spinlock rather than a per-object lock because the
 * priority-inheritance chain walk, the held-mutexes recalculation, the
 * K_FOREVER deadlock check, and the unlock handoff all read or write
 * fields (owner, mutex_pended_on, held_mutexes) belonging to mutexes
 * other than the one passed to the API entry point.
 */
ساكن بنية k_spinlock mutex_قلف;

#که_تعریف_شوی CONFIG_OBJ_CORE_MUTEX
K_OBJ_TYPE_DEFINE(obj_type_mutex, k_mutex, K_OBJ_TYPE_MUTEX_ID, عدم);
#د_که_پای /* CONFIG_OBJ_CORE_MUTEX */

/*
 * Reset the mutex under mutex_lock so a concurrent k_mutex_lock() cannot
 * observe a half-initialized object. With refuse_if_owned set, a mutex that
 * has an owner is left untouched: resetting it would leave its held_node
 * linked in the owner's held_mutexes list and reset the wait queue under
 * pending waiters (threads only pend while the mutex has an owner).
 */
ساكن صحيح mutex_init(بنية k_mutex *mutex, bool refuse_if_owned)
{
	k_spinlock_کلی_t کلی = k_spin_قلف(&mutex_قلف);

	إذا (refuse_if_owned && (mutex->owner != عدم)) {
		k_spin_unlock(&mutex_قلف, کلی);
		أرجع -EBUSY;
	}

	mutex->owner = عدم;
	mutex->قلف_شمېر = 0U;

	z_waitq_init(&mutex->wait_q);

#إذا Z_MUTEX_PI_ENABLED
	mutex->held_غوټه.بل = عدم;
#د_که_پای

	k_spin_unlock(&mutex_قلف, کلی);

	k_object_init(mutex);

#که_تعریف_شوی CONFIG_OBJ_CORE_MUTEX
	k_obj_core_init_and_link(K_OBJ_CORE(mutex), &obj_type_mutex);
#د_که_پای /* CONFIG_OBJ_CORE_MUTEX */

	SYS_پورټ_TRACING_OBJ_INIT(k_mutex, mutex, 0);

	أرجع 0;
}

صحيح z_impl_k_mutex_init(بنية k_mutex *mutex)
{
	أرجع mutex_init(mutex, false);
}

#که_تعریف_شوی CONFIG_USERSPACE
ساكن مضمن صحيح z_vrfy_k_mutex_init(بنية k_mutex *mutex)
{
	bool initialized;

	K_OOPS(K_SYSCALL_OBJ_INIT(mutex, K_OBJ_MUTEX));

	/* The owner field is only meaningful once the mutex was initialized */
	initialized = (k_object_پیدا_کړه(mutex)->flags & K_OBJ_FLAG_INITIALIZED) != 0U;

	K_OOPS(K_SYSCALL_VERIFY_پیغام(mutex_init(mutex, initialized) == 0,
				    "mutex %p is locked", mutex));

	أرجع 0;
}
#ضمن <zephyr/syscalls/k_mutex_init_mrsh.c>
#د_که_پای /* CONFIG_USERSPACE */

#إذا Z_MUTEX_PI_ENABLED
/*
 * Upper bound on ownership-chain walk hops. This is a bound on walk cost
 * under mutex_lock, not a real limit on chain length -- equal-priority
 * threads can legally form longer chains.
 */
#تعریف_کړه MUTEX_CHAIN_WALK_MAX_HOPS 16

ساكن int32_t new_prio_for_inheritance(int32_t موخه, int32_t حد)
{
	صحيح new_prio = z_is_prio_higher(موخه, حد) ? موخه : حد;

	new_prio = z_get_new_prio_with_ceiling(new_prio);

	أرجع new_prio;
}

ساكن bool adjust_owner_prio(بنية k_mutex *mutex, int32_t new_prio)
{
	إذا (mutex->owner->بنسټ.prio != new_prio) {

		LOG_DBG("%p (ready (y/n): %c) prio changed to %d (was %d)",
			mutex->owner, z_is_thread_ready(mutex->owner) ?
			'y' : 'n',
			new_prio, mutex->owner->بنسټ.prio);

		أرجع z_thread_prio_سېټ(mutex->owner, new_prio);
	}
	أرجع false;
}

/*
 * Scan all mutexes held by a thread and return the highest priority
 * among all their waiters, floored at floor_prio.
 */
ساكن int32_t held_mutexes_highest_waiter_prio(بنية k_thread *thread,
						 int32_t floor_prio)
{
	sys_snode_t *غوټه;
	int32_t prio = floor_prio;

	قلف_SCHED_SPINLOCK {
		SYS_SLIST_FOR_EACH_غوټه(&thread->held_mutexes, غوټه) {
			بنية k_mutex *m =
				CONTAINER_د(غوټه, بنية k_mutex, held_غوټه);
			بنية k_thread *waiter = z_waitq_سر_locked(&m->wait_q);

			إذا (waiter != عدم) {
				int32_t wprio = z_get_new_prio_with_ceiling(waiter->بنسټ.prio);

				إذا (z_is_prio_higher(wprio, prio)) {
					prio = wprio;
				}
			}
		}
	}
	أرجع prio;
}
#د_که_پای

صحيح z_impl_k_mutex_قلف(بنية k_mutex *mutex, k_timeout_t timeout)
{
	k_spinlock_کلی_t کلی;
#إذا Z_MUTEX_PI_ENABLED
	bool resched = false;
#د_که_پای

	__ASSERT(!arch_is_دننه_isr(), "mutexes cannot be used inside ISRs");

	SYS_پورټ_TRACING_OBJ_FUNC_ENTER(k_mutex, قلف, mutex, timeout);

	کلی = k_spin_قلف(&mutex_قلف);

	إذا (likely((mutex->قلف_شمېر == 0U) || (mutex->owner == _current))) {

		mutex->قلف_شمېر++;
		mutex->owner = _current;

#إذا Z_MUTEX_PI_ENABLED
		LOG_DBG("%p took mutex %p, count: %d, orig prio: %d",
			_current, mutex, mutex->قلف_شمېر,
			_current->orig_prio);

		/* Add to held list on first acquisition; recursive locks skip this. */
		إذا (mutex->قلف_شمېر == 1U) {
			/* Capture the thread's true pre-inheritance priority
			 * the first time it acquires any mutex.
			 */
			إذا (sys_slist_is_تشه(&_current->held_mutexes)) {
				_current->orig_prio = _current->بنسټ.prio;
			}
			sys_slist_append(&_current->held_mutexes, &mutex->held_غوټه);
		}
#وإلا
		LOG_DBG("%p took mutex %p, count: %d",
			_current, mutex, mutex->قلف_شمېر);
#د_که_پای

		k_spin_unlock(&mutex_قلف, کلی);

		SYS_پورټ_TRACING_OBJ_FUNC_EXIT(k_mutex, قلف, mutex, timeout, 0);

		أرجع 0;
	}

	إذا (unlikely(K_TIMEOUT_EQ(timeout, K_NO_WAIT))) {
		k_spin_unlock(&mutex_قلف, کلی);

		SYS_پورټ_TRACING_OBJ_FUNC_EXIT(k_mutex, قلف, mutex, timeout, -EBUSY);

		أرجع -EBUSY;
	}

	SYS_پورټ_TRACING_OBJ_FUNC_BLOCKING(k_mutex, قلف, mutex, timeout);

#إذا Z_MUTEX_PI_ENABLED
	/* Record the mutex this thread is blocking on for priority inheritance. */
	_current->mutex_pended_on = mutex;

	/*
	 * Walk the ownership chain, boosting each owner's priority to match
	 * the highest-priority waiter. Detects deadlock cycles along the way.
	 */
	{
		int32_t boost_prio = new_prio_for_inheritance(
					_current->بنسټ.prio,
					mutex->owner->بنسټ.prio);
		بنية k_mutex  *chain_mutex = mutex;
		بنية k_thread *chain_owner = mutex->owner;
		صحيح hops = 0;
#إذا defined(CONFIG_MUTEX_DEADLOCK_DETECT)
		bool all_forever = K_TIMEOUT_EQ(timeout, K_FOREVER);
#د_که_پای

		LOG_DBG("chain boosting prio on mutex %p", mutex);

		طالما (chain_owner != عدم) {
#إذا defined(CONFIG_MUTEX_DEADLOCK_DETECT)
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
				      z_is_inactive_timeout(&chain_owner->بنسټ.timeout);

			إذا (all_forever &&
			    chain_owner->mutex_pended_on != عدم &&
			    z_is_thread_pending(chain_owner) &&
			    chain_owner->mutex_pended_on->owner == _current) {
				__ASSERT(false,
					"mutex deadlock: thread %p waiting on "
					"mutex %p (owner %p)",
					_current, mutex, mutex->owner);
				اكسر;
			}
#د_که_پای /* CONFIG_MUTEX_DEADLOCK_DETECT */

			إذا (z_is_prio_higher(boost_prio, chain_owner->بنسټ.prio)) {
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
			إذا (chain_owner->mutex_pended_on == عدم ||
			    !z_is_thread_pending(chain_owner)) {
				اكسر;
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
			إذا (chain_mutex == mutex ||
			    ++hops >= MUTEX_CHAIN_WALK_MAX_HOPS) {
				اكسر;
			}
			chain_owner = chain_mutex->owner;
		}
	}
#د_که_پای

	صحيح got_mutex = z_pend_curr(&mutex_قلف, کلی, &mutex->wait_q, timeout);

	LOG_DBG("on mutex %p got_mutex value: %d", mutex, got_mutex);

	LOG_DBG("%p got mutex %p (y/n): %c", _current, mutex,
		got_mutex ? 'y' : 'n');

#إذا Z_MUTEX_PI_ENABLED
	إذا (got_mutex == 0) {
		/* Lock granted; clear the pending mutex pointer. */
		_current->mutex_pended_on = عدم;
		SYS_پورټ_TRACING_OBJ_FUNC_EXIT(k_mutex, قلف, mutex, timeout, 0);
		أرجع 0;
	}

	/* timed out */

	LOG_DBG("%p timeout on mutex %p", _current, mutex);

	کلی = k_spin_قلف(&mutex_قلف);

	/*
	 * Clear mutex_pended_on under mutex_lock so no concurrent chain walk
	 * can observe the stale pointer and detect a false deadlock cycle.
	 */
	_current->mutex_pended_on = عدم;

	/*
	 * Check if mutex was unlocked after this thread was unpended.
	 * If so, skip adjusting owner's priority down.
	 */
	إذا (likely(mutex->owner != عدم)) {
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

	إذا (resched) {
		z_reschedule(&mutex_قلف, کلی);
	} وإلا {
		k_spin_unlock(&mutex_قلف, کلی);
	}

	SYS_پورټ_TRACING_OBJ_FUNC_EXIT(k_mutex, قلف, mutex, timeout, -EAGAIN);

	أرجع -EAGAIN;
#وإلا
	SYS_پورټ_TRACING_OBJ_FUNC_EXIT(k_mutex, قلف, mutex, timeout, got_mutex);

	أرجع got_mutex;
#د_که_پای
}

#که_تعریف_شوی CONFIG_USERSPACE
ساكن مضمن صحيح z_vrfy_k_mutex_قلف(بنية k_mutex *mutex,
				      k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(mutex, K_OBJ_MUTEX));
	أرجع z_impl_k_mutex_قلف(mutex, timeout);
}
#ضمن <zephyr/syscalls/k_mutex_lock_mrsh.c>
#د_که_پای /* CONFIG_USERSPACE */

صحيح z_impl_k_mutex_unlock(بنية k_mutex *mutex)
{
	بنية k_thread *new_owner = عدم;

	__ASSERT(!arch_is_دننه_isr(), "mutexes cannot be used inside ISRs");

	SYS_پورټ_TRACING_OBJ_FUNC_ENTER(k_mutex, unlock, mutex);

	CHECKIF(mutex->owner == عدم) {
		SYS_پورټ_TRACING_OBJ_FUNC_EXIT(k_mutex, unlock, mutex, -EINVAL);

		أرجع -EINVAL;
	}
	/*
	 * The current thread does not own the mutex.
	 */
	CHECKIF(mutex->owner != _current) {
		SYS_پورټ_TRACING_OBJ_FUNC_EXIT(k_mutex, unlock, mutex, -EPERM);

		أرجع -EPERM;
	}

	/*
	 * Attempt to unlock a mutex which is unlocked. mutex->lock_count
	 * cannot be zero if the current thread is equal to mutex->owner,
	 * therefore no underflow check is required. Use assert to catch
	 * undefined behavior.
	 */
	__ASSERT_NO_MSG(mutex->قلف_شمېر > 0U);

	LOG_DBG("mutex %p lock_count: %d", mutex, mutex->قلف_شمېر);

	/*
	 * If we are the owner and count is greater than 1, then decrement
	 * the count and return and keep current thread as the owner.
	 */
	إذا (mutex->قلف_شمېر > 1U) {
		mutex->قلف_شمېر--;
		اذهب k_mutex_unlock_return;
	}

	k_spinlock_کلی_t کلی = k_spin_قلف(&mutex_قلف);

#إذا Z_MUTEX_PI_ENABLED
	/* Remove this mutex from the owner's list of held mutexes */
	sys_slist_پیدا_کړه_and_remove(&_current->held_mutexes, &mutex->held_غوټه);

	/*
	 * If the thread's priority was boosted, recalculate it by scanning
	 * the remaining held mutexes. The released mutex has already been
	 * removed from held_mutexes, so it is not included in the scan.
	 */
	إذا (_current->بنسټ.prio != _current->orig_prio) {
		int32_t new_prio = held_mutexes_highest_waiter_prio(
					_current, _current->orig_prio);

		adjust_owner_prio(mutex, new_prio);
	}
#د_که_پای

	/* Pick the new owner (if any) and complete the wake atomically
	 * under the scheduler's spinlock, so a racing in-flight timeout
	 * handler cannot observe a half-initialized wake-up.
	 */
	قلف_SCHED_SPINLOCK {
		new_owner = z_unpend_لومړی_thread_locked(&mutex->wait_q);
		mutex->owner = new_owner;

		LOG_DBG("new owner of mutex %p: %p (prio: %d)",
			mutex, new_owner, new_owner ? new_owner->بنسټ.prio : -1000);

		إذا (unlikely(new_owner != عدم)) {
			/*
			 * new owner is already of higher or equal prio than first
			 * waiter since the wait queue is priority-based: no need to
			 * adjust its priority
			 */
#إذا Z_MUTEX_PI_ENABLED
			إذا (sys_slist_is_تشه(&new_owner->held_mutexes)) {
				new_owner->orig_prio = new_owner->بنسټ.prio;
			}
			sys_slist_append(&new_owner->held_mutexes, &mutex->held_غوټه);
			new_owner->mutex_pended_on = عدم;
#د_که_پای
			arch_thread_return_ارزښت_سېټ(new_owner, 0);
			z_sched_ready_locked(new_owner);
		} وإلا {
			mutex->قلف_شمېر = 0U;
		}
	}

	إذا (unlikely(new_owner != عدم)) {
		z_reschedule(&mutex_قلف, کلی);
	} وإلا {
		k_spin_unlock(&mutex_قلف, کلی);
	}


k_mutex_unlock_return:
	SYS_پورټ_TRACING_OBJ_FUNC_EXIT(k_mutex, unlock, mutex, 0);

	أرجع 0;
}

#که_تعریف_شوی CONFIG_USERSPACE
ساكن مضمن صحيح z_vrfy_k_mutex_unlock(بنية k_mutex *mutex)
{
	K_OOPS(K_SYSCALL_OBJ(mutex, K_OBJ_MUTEX));
	أرجع z_impl_k_mutex_unlock(mutex);
}
#ضمن <zephyr/syscalls/k_mutex_unlock_mrsh.c>
#د_که_پای /* CONFIG_USERSPACE */
