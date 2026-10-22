/*
 * Copyright (c) 1997-2016 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#અન્તર્ભાવ <zephyr/kernel.h>

#અન્તર્ભાવ <zephyr/init.h>
#અન્તર્ભાવ <zephyr/internal/syscall_handler.h>
#અન્તર્ભાવ <zephyr/sys/check.h>
#અન્તર્ભાવ <stdbool.h>
#અન્તર્ભાવ <zephyr/spinlock.h>
#અન્તર્ભાવ <ksched.h>
#અન્તર્ભાવ <wait_q.h>

સ્થિર સંરચના k_spinlock timer_lock;

#ifdef CONFIG_OBJ_CORE_TIMER
K_OBJ_TYPE_DEFINE(obj_type_timer, k_timer, K_OBJ_TYPE_TIMER_ID, ખાલી);
#endif /* CONFIG_OBJ_CORE_TIMER */

#જો defined(CONFIG_TIMER_OBSERVER)
સ્થિર અંતઃરેખ શૂન્ય z_timer_observer_on_init(સંરચના k_timer *timer)
{
	STRUCT_SECTION_FOREACH(k_timer_observer, obs) {
		જો (obs->on_init != ખાલી) {
			obs->on_init(timer);
		}
	}
}

સ્થિર અંતઃરેખ શૂન્ય z_timer_observer_on_start(સંરચના k_timer *timer, k_timeout_t duration,
					     k_timeout_t period)
{
	STRUCT_SECTION_FOREACH(k_timer_observer, obs) {
		જો (obs->on_start != ખાલી) {
			obs->on_start(timer, duration, period);
		}
	}
}

સ્થિર અંતઃરેખ શૂન્ય z_timer_observer_on_stop(સંરચના k_timer *timer)
{
	STRUCT_SECTION_FOREACH(k_timer_observer, obs) {
		જો (obs->on_stop != ખાલી) {
			obs->on_stop(timer);
		}
	}
}

સ્થિર અંતઃરેખ શૂન્ય z_timer_observer_on_expiry(સંરચના k_timer *timer)
{
	STRUCT_SECTION_FOREACH(k_timer_observer, obs) {
		જો (obs->on_expiry != ખાલી) {
			obs->on_expiry(timer);
		}
	}
}
#નહીંતર
#define z_timer_observer_on_init(timer) (શૂન્ય)0
#define z_timer_observer_on_start(timer, duration, period) (શૂન્ય)0
#define z_timer_observer_on_stop(timer) (શૂન્ય)0
#define z_timer_observer_on_expiry(timer) (શૂન્ય)0
#endif /* CONFIG_TIMER_OBSERVER */

/**
 * @brief Handle expiration of a kernel timer object.
 *
 * @param t  Timeout used by the timer.
 */
શૂન્ય z_timer_expiration_handler(સંરચના _timeout *t)
{
	સંરચના k_timer *timer = CONTAINER_OF(t, સંરચના k_timer, timeout);
	સંરચના k_thread *thread;
	k_spinlock_ચાવી_t ચાવી = k_spin_lock(&timer_lock);

	/* A same-CPU IRQ may have raced with our dispatch between
	 * sys_clock_announce() popping us off the queue and us taking
	 * timer.c::lock. Two cases:
	 *  - The timer was restarted (z_add_timeout re-queued the timeout):
	 *    it is active again, and the new schedule fires later.
	 *  - The timer was stopped (z_try_abort_timeout marked the
	 *    in-flight slot superseded): bail without firing expiry_fn.
	 */
	જો (!z_is_inactive_timeout(t) ||
	    z_timeout_inflight_superseded(t)) {
		k_spin_unlock(&timer_lock, ચાવી);
		પાછા;
	}

	/*
	 * if the timer is periodic, start it again; don't add _TICK_ALIGN
	 * since we're already aligned to a tick boundary
	 */
	જો (!K_TIMEOUT_EQ(timer->period, K_NO_WAIT) &&
	    !K_TIMEOUT_EQ(timer->period, K_FOREVER)) {
		k_timeout_t next = timer->period;

#ifdef CONFIG_TIMEOUT_64BIT
		/* Exploit the fact that uptime during a kernel
		 * timeout handler reflects the time of the scheduled
		 * event and not real time to get some inexpensive
		 * protection against late interrupts.  If we're
		 * delayed for any reason, we still end up calculating
		 * the next expiration as a regular stride from where
		 * we "should" have run.  Requires absolute timeouts.
		 */
		next = K_TIMEOUT_ABS_TICKS(k_uptime_ticks() + next.ticks);
#endif /* CONFIG_TIMEOUT_64BIT */
		z_add_timeout(&timer->timeout, z_timer_expiration_handler,
			      next);
	}

	/* update timer's status */
	timer->status += 1U;

	z_timer_observer_on_expiry(timer);

	/* invoke timer expiry function */
	જો (timer->expiry_fn != ખાલી) {
		k_timer_expiry_t expiry_fn = timer->expiry_fn;

		/* Unlock for user handler. */
		k_spin_unlock(&timer_lock, ચાવી);

		SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_timer, expiry, timer);

		expiry_fn(timer);

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_timer, expiry, timer);

		ચાવી = k_spin_lock(&timer_lock);
	}

	જો (!IS_ENABLED(CONFIG_MULTITHREADING)) {
		k_spin_unlock(&timer_lock, ચાવી);
		પાછા;
	}

	LOCK_SCHED_SPINLOCK {
		thread = z_waitq_head_locked(&timer->wait_q);

		જો (thread != ખાલી) {
			unpend_thread_no_timeout(thread);
			arch_thread_return_કિંમત_set(thread, 0);
			z_sched_ready_locked(thread);
		}
	}

	k_spin_unlock(&timer_lock, ચાવી);

}


પૂર્ણાંક z_timer_cleanup(સંરચના k_timer *timer, __maybe_unused bool locked)
{
	/* Not callable from an ISR: this is the one timer path that can
	 * spin waiting for an in-flight handler, and an ISR spinning here
	 * could starve the very CPU the handler needs to make progress.
	 */
	__ASSERT(!arch_is_in_isr(), "");

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_timer, cleanup, timer);

	પૂર્ણાંક ret = 0;
	k_spinlock_ચાવી_t ચાવી;

	/* Refuse if anyone is still pending on the timer's wait queue
	 * (e.g. via k_timer_status_sync()): freeing the storage would
	 * leave dangling pended_on pointers.
	 */
retry:
	ચાવી = k_spin_lock(&timer_lock);

	CHECKIF(locked && (z_waitq_head_locked(&timer->wait_q) != ખાલી)) {
		ret = -EAGAIN;
		જાઓ out;
	}

	CHECKIF(!locked && (z_waitq_head(&timer->wait_q) != ખાલી)) {
		ret = -EAGAIN;
		જાઓ out;
	}

	/* Cancel the timeout AND wait for any in-flight expiration
	 * handler on another CPU to complete before returning. Unlike
	 * k_timer_stop(), we do not call any user stop_fn here: the
	 * caller is about to free the storage and there is no further
	 * consumer of the timer.
	 *
	 * This is the one timer path that waits on the handler (it must,
	 * before the storage is freed). The wait can only stall if the
	 * handler itself blocks on this CPU making progress -- e.g. an
	 * expiry_fn that aborts a thread running here. Freeing a timer
	 * whose handler does that is a caller bug; ordinary stop/start do
	 * not wait and so cannot stall.
	 */
	જો (z_try_abort_timeout(&timer->timeout) == -EAGAIN) {
		k_spin_unlock(&timer_lock, ચાવી);
		જાઓ retry;
	}

out:
	k_spin_unlock(&timer_lock, ચાવી);

#ifdef CONFIG_OBJ_CORE_TIMER
	જો (ret == 0) {
		k_obj_core_unlink(K_OBJ_CORE(timer));
	}
#endif /* CONFIG_OBJ_CORE_TIMER */

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_timer, cleanup, timer, ret);

	પાછા ret;
}

પૂર્ણાંક k_timer_cleanup(સંરચના k_timer *timer)
{
	પાછા z_timer_cleanup(timer, false);
}


શૂન્ય k_timer_init(સંરચના k_timer *timer,
			 k_timer_expiry_t expiry_fn,
			 k_timer_stop_t stop_fn)
{
	timer->expiry_fn = expiry_fn;
	timer->stop_fn = stop_fn;
	timer->status = 0U;

	જો (IS_ENABLED(CONFIG_MULTITHREADING)) {
		z_waitq_init(&timer->wait_q);
	}

	z_init_timeout(&timer->timeout);

	SYS_PORT_TRACING_OBJ_INIT(k_timer, timer);

	timer->user_data = ખાલી;

	k_object_init(timer);

#ifdef CONFIG_OBJ_CORE_TIMER
	k_obj_core_init_and_link(K_OBJ_CORE(timer), &obj_type_timer);
#endif /* CONFIG_OBJ_CORE_TIMER */

	z_timer_observer_on_init(timer);
}


શૂન્ય z_impl_k_timer_start(સંરચના k_timer *timer, k_timeout_t duration,
			  k_timeout_t period)
{
	SYS_PORT_TRACING_OBJ_FUNC(k_timer, start, timer, duration, period);

	જો (K_TIMEOUT_EQ(duration, K_FOREVER)) {
		પાછા;
	}

	/* Hold the timer lock across abort + add to serialize against a
	 * concurrent k_timer_start on the same timer. An in-flight handler
	 * (z_try_abort_timeout() returning non-zero) is flagged superseded
	 * and will bail; the re-arm below also re-links the node, which
	 * makes a not-yet-committed handler bail too. Either way we do not
	 * wait for it.
	 */
	k_spinlock_ચાવી_t ચાવી = k_spin_lock(&timer_lock);

	(શૂન્ય)z_try_abort_timeout(&timer->timeout);

	timer->period = period;
	timer->status = 0U;

	z_add_timeout(&timer->timeout, z_timer_expiration_handler, duration);

	z_timer_observer_on_start(timer, duration, period);

	k_spin_unlock(&timer_lock, ચાવી);
}

#ifdef CONFIG_USERSPACE
સ્થિર અંતઃરેખ શૂન્ય z_vrfy_k_timer_start(સંરચના k_timer *timer,
					k_timeout_t duration,
					k_timeout_t period)
{
	K_OOPS(K_SYSCALL_OBJ(timer, K_OBJ_TIMER));
	z_impl_k_timer_start(timer, duration, period);
}
#અન્તર્ભાવ <zephyr/syscalls/k_timer_start_mrsh.c>
#endif /* CONFIG_USERSPACE */

શૂન્ય z_impl_k_timer_stop(સંરચના k_timer *timer)
{
	SYS_PORT_TRACING_OBJ_FUNC(k_timer, stop, timer);

	k_spinlock_ચાવી_t ચાવી = k_spin_lock(&timer_lock);

	જો (z_try_abort_timeout(&timer->timeout) != 0) {
		/* Not removed from the queue: either the timer was not
		 * active, or its handler is in flight. In the latter case
		 * z_try_abort_timeout() has flagged it superseded so the
		 * handler bails; we do not wait for it. Nothing to stop here.
		 */
		k_spin_unlock(&timer_lock, ચાવી);
		પાછા;
	}

	z_timer_observer_on_stop(timer);

	જો (timer->stop_fn != ખાલી) {
		SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_timer, stop_fn_expiry, timer);
		k_spin_unlock(&timer_lock, ચાવી);

		timer->stop_fn(timer);

		ચાવી = k_spin_lock(&timer_lock);

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_timer, stop_fn_expiry, timer);
	}

	જો (!IS_ENABLED(CONFIG_MULTITHREADING)) {
		k_spin_unlock(&timer_lock, ચાવી);
		પાછા;

	}

	સંરચના k_thread *pending_thread = z_unpend1_no_timeout(&timer->wait_q);

	જો (pending_thread != ખાલી) {
		z_ready_thread(pending_thread);
		z_reschedule(&timer_lock, ચાવી);
	} નહીંતર {
		k_spin_unlock(&timer_lock, ચાવી);
	}
}

#ifdef CONFIG_USERSPACE
સ્થિર અંતઃરેખ શૂન્ય z_vrfy_k_timer_stop(સંરચના k_timer *timer)
{
	K_OOPS(K_SYSCALL_OBJ(timer, K_OBJ_TIMER));
	z_impl_k_timer_stop(timer);
}
#અન્તર્ભાવ <zephyr/syscalls/k_timer_stop_mrsh.c>
#endif /* CONFIG_USERSPACE */

uint32_t z_impl_k_timer_status_get(સંરચના k_timer *timer)
{
	k_spinlock_ચાવી_t ચાવી = k_spin_lock(&timer_lock);
	uint32_t result = timer->status;

	timer->status = 0U;
	k_spin_unlock(&timer_lock, ચાવી);

	પાછા result;
}

#ifdef CONFIG_USERSPACE
સ્થિર અંતઃરેખ uint32_t z_vrfy_k_timer_status_get(સંરચના k_timer *timer)
{
	K_OOPS(K_SYSCALL_OBJ(timer, K_OBJ_TIMER));
	પાછા z_impl_k_timer_status_get(timer);
}
#અન્તર્ભાવ <zephyr/syscalls/k_timer_status_get_mrsh.c>
#endif /* CONFIG_USERSPACE */

uint32_t z_impl_k_timer_status_sync(સંરચના k_timer *timer)
{
	__ASSERT(!arch_is_in_isr(), "");
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_timer, status_sync, timer);

	જો (!IS_ENABLED(CONFIG_MULTITHREADING)) {
		uint32_t result;

		કરો {
			unsigned પૂર્ણાંક ચાવી = irq_lock();

			જો (!z_is_inactive_timeout(&timer->timeout)) {
				result = *(અસ્થિર uint32_t *)&timer->status;
				timer->status = 0U;
				જો (result > 0) {
					irq_unlock(ચાવી);
					તોડો;
				} નહીંતર {
					k_cpu_atomic_idle(ચાવી);
				}
			} નહીંતર {
				result = timer->status;
				irq_unlock(ચાવી);
				તોડો;
			}
		} જ્યારે (true);

		પાછા result;
	}

	k_spinlock_ચાવી_t ચાવી = k_spin_lock(&timer_lock);
	uint32_t result = timer->status;

	જો (result == 0U) {
		જો (!z_is_inactive_timeout(&timer->timeout)) {
			SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_timer, status_sync, timer, K_FOREVER);

			/* wait for timer to expire or stop */
			(શૂન્ય)z_pend_curr(&timer_lock, ચાવી, &timer->wait_q, K_FOREVER);

			/* get updated timer status */
			ચાવી = k_spin_lock(&timer_lock);
			result = timer->status;
		} નહીંતર {
			/* timer is already stopped */
		}
	} નહીંતર {
		/* timer has already expired at least once */
	}

	timer->status = 0U;
	k_spin_unlock(&timer_lock, ચાવી);

	/**
	 * @note	New tracing hook
	 */
	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_timer, status_sync, timer, result);

	પાછા result;
}

#ifdef CONFIG_USERSPACE
સ્થિર અંતઃરેખ uint32_t z_vrfy_k_timer_status_sync(સંરચના k_timer *timer)
{
	K_OOPS(K_SYSCALL_OBJ(timer, K_OBJ_TIMER));
	પાછા z_impl_k_timer_status_sync(timer);
}
#અન્તર્ભાવ <zephyr/syscalls/k_timer_status_sync_mrsh.c>

સ્થિર અંતઃરેખ k_ticks_t z_vrfy_k_timer_remaining_ticks(
						અચલ સંરચના k_timer *timer)
{
	K_OOPS(K_SYSCALL_OBJ(timer, K_OBJ_TIMER));
	પાછા z_impl_k_timer_remaining_ticks(timer);
}
#અન્તર્ભાવ <zephyr/syscalls/k_timer_remaining_ticks_mrsh.c>

સ્થિર અંતઃરેખ k_ticks_t z_vrfy_k_timer_expires_ticks(
						અચલ સંરચના k_timer *timer)
{
	K_OOPS(K_SYSCALL_OBJ(timer, K_OBJ_TIMER));
	પાછા z_impl_k_timer_expires_ticks(timer);
}
#અન્તર્ભાવ <zephyr/syscalls/k_timer_expires_ticks_mrsh.c>

સ્થિર અંતઃરેખ શૂન્ય *z_vrfy_k_timer_user_data_get(અચલ સંરચના k_timer *timer)
{
	K_OOPS(K_SYSCALL_OBJ(timer, K_OBJ_TIMER));
	પાછા z_impl_k_timer_user_data_get(timer);
}
#અન્તર્ભાવ <zephyr/syscalls/k_timer_user_data_get_mrsh.c>

સ્થિર અંતઃરેખ શૂન્ય z_vrfy_k_timer_user_data_set(સંરચના k_timer *timer,
						શૂન્ય *user_data)
{
	K_OOPS(K_SYSCALL_OBJ(timer, K_OBJ_TIMER));
	z_impl_k_timer_user_data_set(timer, user_data);
}
#અન્તર્ભાવ <zephyr/syscalls/k_timer_user_data_set_mrsh.c>

#endif /* CONFIG_USERSPACE */
