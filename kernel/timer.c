/*
 * Copyright (c) 1997-2016 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#समावेश <zephyr/kernel.h>

#समावेश <zephyr/init.h>
#समावेश <zephyr/internal/syscall_handler.h>
#समावेश <zephyr/sys/check.h>
#समावेश <stdbool.h>
#समावेश <zephyr/spinlock.h>
#समावेश <ksched.h>
#समावेश <wait_q.h>

अटल काष्ठा k_spinlock timer_lock;

#अगर_घोषित CONFIG_OBJ_CORE_TIMER
K_OBJ_TYPE_DEFINE(obj_type_timer, k_timer, K_OBJ_TYPE_TIMER_ID, शून्य);
#पूर्ण_अगर /* CONFIG_OBJ_CORE_TIMER */

#अगर defined(CONFIG_TIMER_OBSERVER)
अटल अंतरभूत व्योम z_timer_observer_on_init(काष्ठा k_timer *timer)
{
	STRUCT_SECTION_FOREACH(k_timer_observer, obs) {
		अगर (obs->on_init != शून्य) {
			obs->on_init(timer);
		}
	}
}

अटल अंतरभूत व्योम z_timer_observer_on_start(काष्ठा k_timer *timer, k_timeout_t duration,
					     k_timeout_t period)
{
	STRUCT_SECTION_FOREACH(k_timer_observer, obs) {
		अगर (obs->on_start != शून्य) {
			obs->on_start(timer, duration, period);
		}
	}
}

अटल अंतरभूत व्योम z_timer_observer_on_stop(काष्ठा k_timer *timer)
{
	STRUCT_SECTION_FOREACH(k_timer_observer, obs) {
		अगर (obs->on_stop != शून्य) {
			obs->on_stop(timer);
		}
	}
}

अटल अंतरभूत व्योम z_timer_observer_on_expiry(काष्ठा k_timer *timer)
{
	STRUCT_SECTION_FOREACH(k_timer_observer, obs) {
		अगर (obs->on_expiry != शून्य) {
			obs->on_expiry(timer);
		}
	}
}
#अन्यथा
#घोषणा z_timer_observer_on_init(timer) (व्योम)0
#घोषणा z_timer_observer_on_start(timer, duration, period) (व्योम)0
#घोषणा z_timer_observer_on_stop(timer) (व्योम)0
#घोषणा z_timer_observer_on_expiry(timer) (व्योम)0
#पूर्ण_अगर /* CONFIG_TIMER_OBSERVER */

/**
 * @brief Handle expiration of a kernel timer object.
 *
 * @param t  Timeout used by the timer.
 */
व्योम z_timer_expiration_handler(काष्ठा _timeout *t)
{
	काष्ठा k_timer *timer = CONTAINER_OF(t, काष्ठा k_timer, timeout);
	काष्ठा k_thread *thread;
	k_spinlock_कुंजी_t कुंजी = k_spin_lock(&timer_lock);

	/* A same-CPU IRQ may have raced with our dispatch between
	 * sys_clock_announce() popping us off the queue and us taking
	 * timer.c::lock. Two cases:
	 *  - The timer was restarted (z_add_timeout re-queued the timeout):
	 *    it is active again, and the new schedule fires later.
	 *  - The timer was stopped (z_try_abort_timeout marked the
	 *    in-flight slot superseded): bail without firing expiry_fn.
	 */
	अगर (!z_is_inactive_timeout(t) ||
	    z_timeout_inflight_superseded(t)) {
		k_spin_unlock(&timer_lock, कुंजी);
		वापस;
	}

	/*
	 * if the timer is periodic, start it again; don't add _TICK_ALIGN
	 * since we're already aligned to a tick boundary
	 */
	अगर (!K_TIMEOUT_EQ(timer->period, K_NO_WAIT) &&
	    !K_TIMEOUT_EQ(timer->period, K_FOREVER)) {
		k_timeout_t अग्र = timer->period;

#अगर_घोषित CONFIG_TIMEOUT_64BIT
		/* Exploit the fact that uptime during a kernel
		 * timeout handler reflects the time of the scheduled
		 * event and not real time to get some inexpensive
		 * protection against late interrupts.  If we're
		 * delayed for any reason, we still end up calculating
		 * the next expiration as a regular stride from where
		 * we "should" have run.  Requires absolute timeouts.
		 */
		अग्र = K_TIMEOUT_ABS_TICKS(k_uptime_ticks() + अग्र.ticks);
#पूर्ण_अगर /* CONFIG_TIMEOUT_64BIT */
		z_add_timeout(&timer->timeout, z_timer_expiration_handler,
			      अग्र);
	}

	/* update timer's status */
	timer->status += 1U;

	z_timer_observer_on_expiry(timer);

	/* invoke timer expiry function */
	अगर (timer->expiry_fn != शून्य) {
		k_timer_expiry_t expiry_fn = timer->expiry_fn;

		/* Unlock for user handler. */
		k_spin_unlock(&timer_lock, कुंजी);

		SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_timer, expiry, timer);

		expiry_fn(timer);

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_timer, expiry, timer);

		कुंजी = k_spin_lock(&timer_lock);
	}

	अगर (!IS_ENABLED(CONFIG_MULTITHREADING)) {
		k_spin_unlock(&timer_lock, कुंजी);
		वापस;
	}

	LOCK_SCHED_SPINLOCK {
		thread = z_waitq_शीर्ष_locked(&timer->wait_q);

		अगर (thread != शून्य) {
			unpend_thread_no_timeout(thread);
			arch_thread_return_मूल्य_set(thread, 0);
			z_sched_ready_locked(thread);
		}
	}

	k_spin_unlock(&timer_lock, कुंजी);

}


पूर्णांक z_timer_cleanup(काष्ठा k_timer *timer, __maybe_unused bool locked)
{
	/* Not callable from an ISR: this is the one timer path that can
	 * spin waiting for an in-flight handler, and an ISR spinning here
	 * could starve the very CPU the handler needs to make progress.
	 */
	__ASSERT(!arch_is_in_isr(), "");

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_timer, cleanup, timer);

	पूर्णांक ret = 0;
	k_spinlock_कुंजी_t कुंजी;

	/* Refuse if anyone is still pending on the timer's wait queue
	 * (e.g. via k_timer_status_sync()): freeing the storage would
	 * leave dangling pended_on pointers.
	 */
retry:
	कुंजी = k_spin_lock(&timer_lock);

	CHECKIF(locked && (z_waitq_शीर्ष_locked(&timer->wait_q) != शून्य)) {
		ret = -EAGAIN;
		जाओ out;
	}

	CHECKIF(!locked && (z_waitq_शीर्ष(&timer->wait_q) != शून्य)) {
		ret = -EAGAIN;
		जाओ out;
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
	अगर (z_try_abort_timeout(&timer->timeout) == -EAGAIN) {
		k_spin_unlock(&timer_lock, कुंजी);
		जाओ retry;
	}

out:
	k_spin_unlock(&timer_lock, कुंजी);

#अगर_घोषित CONFIG_OBJ_CORE_TIMER
	अगर (ret == 0) {
		k_obj_core_unlink(K_OBJ_CORE(timer));
	}
#पूर्ण_अगर /* CONFIG_OBJ_CORE_TIMER */

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_timer, cleanup, timer, ret);

	वापस ret;
}

पूर्णांक k_timer_cleanup(काष्ठा k_timer *timer)
{
	वापस z_timer_cleanup(timer, false);
}


व्योम k_timer_init(काष्ठा k_timer *timer,
			 k_timer_expiry_t expiry_fn,
			 k_timer_stop_t stop_fn)
{
	timer->expiry_fn = expiry_fn;
	timer->stop_fn = stop_fn;
	timer->status = 0U;

	अगर (IS_ENABLED(CONFIG_MULTITHREADING)) {
		z_waitq_init(&timer->wait_q);
	}

	z_init_timeout(&timer->timeout);

	SYS_PORT_TRACING_OBJ_INIT(k_timer, timer);

	timer->user_तथ्य = शून्य;

	k_object_init(timer);

#अगर_घोषित CONFIG_OBJ_CORE_TIMER
	k_obj_core_init_and_link(K_OBJ_CORE(timer), &obj_type_timer);
#पूर्ण_अगर /* CONFIG_OBJ_CORE_TIMER */

	z_timer_observer_on_init(timer);
}


व्योम z_impl_k_timer_start(काष्ठा k_timer *timer, k_timeout_t duration,
			  k_timeout_t period)
{
	SYS_PORT_TRACING_OBJ_FUNC(k_timer, start, timer, duration, period);

	अगर (K_TIMEOUT_EQ(duration, K_FOREVER)) {
		वापस;
	}

	/* Hold the timer lock across abort + add to serialize against a
	 * concurrent k_timer_start on the same timer. An in-flight handler
	 * (z_try_abort_timeout() returning non-zero) is flagged superseded
	 * and will bail; the re-arm below also re-links the node, which
	 * makes a not-yet-committed handler bail too. Either way we do not
	 * wait for it.
	 */
	k_spinlock_कुंजी_t कुंजी = k_spin_lock(&timer_lock);

	(व्योम)z_try_abort_timeout(&timer->timeout);

	timer->period = period;
	timer->status = 0U;

	z_add_timeout(&timer->timeout, z_timer_expiration_handler, duration);

	z_timer_observer_on_start(timer, duration, period);

	k_spin_unlock(&timer_lock, कुंजी);
}

#अगर_घोषित CONFIG_USERSPACE
अटल अंतरभूत व्योम z_vrfy_k_timer_start(काष्ठा k_timer *timer,
					k_timeout_t duration,
					k_timeout_t period)
{
	K_OOPS(K_SYSCALL_OBJ(timer, K_OBJ_TIMER));
	z_impl_k_timer_start(timer, duration, period);
}
#समावेश <zephyr/syscalls/k_timer_start_mrsh.c>
#पूर्ण_अगर /* CONFIG_USERSPACE */

व्योम z_impl_k_timer_stop(काष्ठा k_timer *timer)
{
	SYS_PORT_TRACING_OBJ_FUNC(k_timer, stop, timer);

	k_spinlock_कुंजी_t कुंजी = k_spin_lock(&timer_lock);

	अगर (z_try_abort_timeout(&timer->timeout) != 0) {
		/* Not removed from the queue: either the timer was not
		 * active, or its handler is in flight. In the latter case
		 * z_try_abort_timeout() has flagged it superseded so the
		 * handler bails; we do not wait for it. Nothing to stop here.
		 */
		k_spin_unlock(&timer_lock, कुंजी);
		वापस;
	}

	z_timer_observer_on_stop(timer);

	अगर (timer->stop_fn != शून्य) {
		SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_timer, stop_fn_expiry, timer);
		k_spin_unlock(&timer_lock, कुंजी);

		timer->stop_fn(timer);

		कुंजी = k_spin_lock(&timer_lock);

		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_timer, stop_fn_expiry, timer);
	}

	अगर (!IS_ENABLED(CONFIG_MULTITHREADING)) {
		k_spin_unlock(&timer_lock, कुंजी);
		वापस;

	}

	काष्ठा k_thread *pending_thread = z_unpend1_no_timeout(&timer->wait_q);

	अगर (pending_thread != शून्य) {
		z_ready_thread(pending_thread);
		z_reschedule(&timer_lock, कुंजी);
	} अन्यथा {
		k_spin_unlock(&timer_lock, कुंजी);
	}
}

#अगर_घोषित CONFIG_USERSPACE
अटल अंतरभूत व्योम z_vrfy_k_timer_stop(काष्ठा k_timer *timer)
{
	K_OOPS(K_SYSCALL_OBJ(timer, K_OBJ_TIMER));
	z_impl_k_timer_stop(timer);
}
#समावेश <zephyr/syscalls/k_timer_stop_mrsh.c>
#पूर्ण_अगर /* CONFIG_USERSPACE */

uint32_t z_impl_k_timer_status_get(काष्ठा k_timer *timer)
{
	k_spinlock_कुंजी_t कुंजी = k_spin_lock(&timer_lock);
	uint32_t परिणाम = timer->status;

	timer->status = 0U;
	k_spin_unlock(&timer_lock, कुंजी);

	वापस परिणाम;
}

#अगर_घोषित CONFIG_USERSPACE
अटल अंतरभूत uint32_t z_vrfy_k_timer_status_get(काष्ठा k_timer *timer)
{
	K_OOPS(K_SYSCALL_OBJ(timer, K_OBJ_TIMER));
	वापस z_impl_k_timer_status_get(timer);
}
#समावेश <zephyr/syscalls/k_timer_status_get_mrsh.c>
#पूर्ण_अगर /* CONFIG_USERSPACE */

uint32_t z_impl_k_timer_status_sync(काष्ठा k_timer *timer)
{
	__ASSERT(!arch_is_in_isr(), "");
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_timer, status_sync, timer);

	अगर (!IS_ENABLED(CONFIG_MULTITHREADING)) {
		uint32_t परिणाम;

		करो {
			अचिन्हित पूर्णांक कुंजी = irq_lock();

			अगर (!z_is_inactive_timeout(&timer->timeout)) {
				परिणाम = *(अस्थिर uint32_t *)&timer->status;
				timer->status = 0U;
				अगर (परिणाम > 0) {
					irq_unlock(कुंजी);
					अवरोध;
				} अन्यथा {
					k_cpu_atomic_idle(कुंजी);
				}
			} अन्यथा {
				परिणाम = timer->status;
				irq_unlock(कुंजी);
				अवरोध;
			}
		} जबतक (true);

		वापस परिणाम;
	}

	k_spinlock_कुंजी_t कुंजी = k_spin_lock(&timer_lock);
	uint32_t परिणाम = timer->status;

	अगर (परिणाम == 0U) {
		अगर (!z_is_inactive_timeout(&timer->timeout)) {
			SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_timer, status_sync, timer, K_FOREVER);

			/* wait for timer to expire or stop */
			(व्योम)z_pend_curr(&timer_lock, कुंजी, &timer->wait_q, K_FOREVER);

			/* get updated timer status */
			कुंजी = k_spin_lock(&timer_lock);
			परिणाम = timer->status;
		} अन्यथा {
			/* timer is already stopped */
		}
	} अन्यथा {
		/* timer has already expired at least once */
	}

	timer->status = 0U;
	k_spin_unlock(&timer_lock, कुंजी);

	/**
	 * @note	New tracing hook
	 */
	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_timer, status_sync, timer, परिणाम);

	वापस परिणाम;
}

#अगर_घोषित CONFIG_USERSPACE
अटल अंतरभूत uint32_t z_vrfy_k_timer_status_sync(काष्ठा k_timer *timer)
{
	K_OOPS(K_SYSCALL_OBJ(timer, K_OBJ_TIMER));
	वापस z_impl_k_timer_status_sync(timer);
}
#समावेश <zephyr/syscalls/k_timer_status_sync_mrsh.c>

अटल अंतरभूत k_ticks_t z_vrfy_k_timer_remaining_ticks(
						स्थिर काष्ठा k_timer *timer)
{
	K_OOPS(K_SYSCALL_OBJ(timer, K_OBJ_TIMER));
	वापस z_impl_k_timer_remaining_ticks(timer);
}
#समावेश <zephyr/syscalls/k_timer_remaining_ticks_mrsh.c>

अटल अंतरभूत k_ticks_t z_vrfy_k_timer_expires_ticks(
						स्थिर काष्ठा k_timer *timer)
{
	K_OOPS(K_SYSCALL_OBJ(timer, K_OBJ_TIMER));
	वापस z_impl_k_timer_expires_ticks(timer);
}
#समावेश <zephyr/syscalls/k_timer_expires_ticks_mrsh.c>

अटल अंतरभूत व्योम *z_vrfy_k_timer_user_तथ्य_get(स्थिर काष्ठा k_timer *timer)
{
	K_OOPS(K_SYSCALL_OBJ(timer, K_OBJ_TIMER));
	वापस z_impl_k_timer_user_तथ्य_get(timer);
}
#समावेश <zephyr/syscalls/k_timer_user_data_get_mrsh.c>

अटल अंतरभूत व्योम z_vrfy_k_timer_user_तथ्य_set(काष्ठा k_timer *timer,
						व्योम *user_तथ्य)
{
	K_OOPS(K_SYSCALL_OBJ(timer, K_OBJ_TIMER));
	z_impl_k_timer_user_तथ्य_set(timer, user_तथ्य);
}
#समावेश <zephyr/syscalls/k_timer_user_data_set_mrsh.c>

#पूर्ण_अगर /* CONFIG_USERSPACE */
