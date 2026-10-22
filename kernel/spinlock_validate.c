/*
 * Copyright (c) 2018,2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#अन्तर्भाव <kernel_internal.h>
#अन्तर्भाव <zephyr/spinlock.h>
#अन्तर्भाव <zephyr/llext/symbol.h>

#परिभाषय SPIN_CPU_ID_आवरणम् (परिमाण(शून्य *) - 1)

/* Outermost spinlock held on each CPU; set-if-NULL on lock,
 * clear-if-matches on unlock.
 */
स्थिर संरचना k_spinlock *z_held_spinlock[CONFIG_MP_MAX_अङ्कः_CPUS];

/* Total spinlock hold count per CPU, incremented on every lock and
 * decremented on every unlock regardless of nesting depth.
 */
स्थिर पूर्णाङ्क z_held_spinlock_गणनम्[CONFIG_MP_MAX_अङ्कः_CPUS];

/* Sentinel: set in swap_data by the ztest harness before force-aborting a
 * thread that holds a lock after an expected panic/assert.
 */
ध्रुव uint8_t z_spinlock_abort_sentinel;

bool z_spin_तालकम्_valid(संरचना k_spinlock *l)
{
	uintptr_t thread_cpu = l->thread_cpu;

	यदि (thread_cpu != 0U) {
		यदि ((thread_cpu & SPIN_CPU_ID_आवरणम्) == _current_cpu->id) {
			प्रत्यागम false;
		}
	}
	प्रत्यागम true;
}
EXPORT_चिह्नम्(z_spin_तालकम्_valid);

/* z_spin_unlock_valid() and z_spin_lock_set_owner() below use _raw_current
 * rather than _current: they run at points where z_current_thread_set() may
 * just have changed the current thread, which an architecture caching it in a
 * register (CONFIG_ARCH_HAS_CUSTOM_CURRENT_IMPL) would not report.  Both are
 * called from k_spin_lock() and k_spin_unlock() with the lock held, hence with
 * interrupts locked as _raw_current requires.
 */
bool z_spin_unlock_valid(संरचना k_spinlock *l)
{
	uint8_t cpu_id = _current_cpu->id;
	uintptr_t tcpu = l->thread_cpu;

	l->thread_cpu = 0;

	यदि (z_held_spinlock[cpu_id] == l) {
		z_held_spinlock[cpu_id] = रिक्त;
	}

	यदि (arch_is_अन्तर्_isr() &&
	    (_raw_current->आधारः.thread_state & _THREAD_DUMMY) != 0) {
		/* Edge case where an ISR aborted _current */
		z_held_spinlock_गणनम्[cpu_id]--;
		__ASSERT(z_held_spinlock_गणनम्[cpu_id] >= 0,
			 "spinlock unlock with no matching lock");
		प्रत्यागम true;
	}
	यदि (tcpu != (cpu_id | (uintptr_t)_raw_current)) {
		प्रत्यागम false;
	}
	z_held_spinlock_गणनम्[cpu_id]--;
	__ASSERT(z_held_spinlock_गणनम्[cpu_id] >= 0,
		 "spinlock unlock with no matching lock");
	प्रत्यागम true;
}
EXPORT_चिह्नम्(z_spin_unlock_valid);

शून्य z_spin_तालकम्_समुच्चयः_owner(संरचना k_spinlock *l)
{
	uint8_t cpu_id = _current_cpu->id;

	l->thread_cpu = cpu_id | (uintptr_t)_raw_current;

	यदि (z_held_spinlock[cpu_id] == रिक्त) {
		z_held_spinlock[cpu_id] = l;
	}
	z_held_spinlock_गणनम्[cpu_id]++;
}
EXPORT_चिह्नम्(z_spin_तालकम्_समुच्चयः_owner);

/* Called from the switch paths after z_current_thread_set() to transfer
 * ownership of an inherited lock.  Does NOT update the tracking arrays.
 *
 * @a thread is passed in rather than read from _current: the caller has just
 * made it current, and _current is not required to report it until the switch
 * completes (see z_current_thread_set()).
 */
शून्य z_spin_तालकम्_transfer_owner(संरचना k_spinlock *l, संरचना k_thread *thread)
{
	l->thread_cpu = _current_cpu->id | (uintptr_t)thread;
}
EXPORT_चिह्नम्(z_spin_तालकम्_transfer_owner);

/* Reset per-CPU spinlock tracking after abnormal thread termination.
 * All locks the dying thread held are abandoned (it will never resume to
 * release them), so only the locks still being released by the cleanup
 * path need to be counted.  lock_held=true if exactly one such lock
 * remains and will be released after this call.
 */
शून्य z_spin_validate_reset(bool तालकम्_held)
{
	uint8_t cpu_id = _current_cpu->id;

	z_held_spinlock[cpu_id] = रिक्त;
	z_held_spinlock_गणनम्[cpu_id] = तालकम्_held ? 1 : 0;
}

/* Internal: caller must ensure IRQs are disabled. */
स्थिर अन्तःस्थ संरचना k_spinlock *z_spin_get_held_तालकम्_locked(uint8_t cpu_id)
{
	प्रत्यागम z_held_spinlock[cpu_id];
}

#यदि_परिभाषितम् CONFIG_TEST
संरचना k_spinlock *z_spin_get_held_तालकम्(शून्य)
{
	अचिह्नित पूर्णाङ्क कुञ्चिका = arch_irq_तालकम्();
	संरचना k_spinlock *l = z_spin_get_held_तालकम्_locked(_current_cpu->id);

	arch_irq_unlock(कुञ्चिका);
	प्रत्यागम l;
}
#यदि_अन्तः /* CONFIG_TEST */

/*
 * Verify that a context switch is safe to perform.  Called from
 * do_swap() and z_swap_irqlock() before the scheduler takes over.
 *
 * swap_lock is the spinlock being released as part of this swap (e.g.
 * _sched_spinlock), or NULL for the irq-lock-only path.  It may legitimately
 * be held at the point of the call; any other held spinlock is a bug.
 *
 * key must show that interrupts were unlocked before the swap was entered.
 * If not, we are context switching out of a nested irq_lock() critical
 * section — i.e. breaking the lock of someone higher up the call stack,
 * which is forbidden.
 */
शून्य z_assert_can_swap(अचिह्नित पूर्णाङ्क कुञ्चिका, संरचना k_spinlock *swap_तालकम्)
{
	uint8_t cpu_id = _current_cpu->id;
	संरचना k_spinlock *held = z_spin_get_held_तालकम्_locked(cpu_id);
	पूर्णाङ्क गणनम् = z_held_spinlock_गणनम्[cpu_id];
	पूर्णाङ्क extra = (swap_तालकम् != रिक्त) ? गणनम् - 1 : गणनम्;

	ARG_अप्रयुक्तम्(held);
	ARG_अप्रयुक्तम्(extra);

	/* ztest expected-fault: thread aborted with a lock held.*/
	यदि ((_current->आधारः.thread_state & _THREAD_DEAD) &&
	    _current->आधारः.swap_दत्त == (शून्य *)&z_spinlock_abort_sentinel) {
		z_spin_validate_reset(swap_तालकम् != रिक्त);
	} अन्यथा {
		__ASSERT(extra >= 0,
			 "swap_lock %p is not tracked in the spinlock hold count!", swap_तालकम्);

		__ASSERT(held == रिक्त || held == swap_तालकम्,
			 "Context switching while holding spinlock %p!", held);

		/* Catches lock(A); lock(B); unlock(A); z_swap(): slot cleared when A
		 * is released but count stays at 1 because B is still held.
		 */
		__ASSERT(extra == 0,
			 "Context switching while holding %d extra spinlock(s)!", extra);
	}

#यदि_अपरिभाषितम् CONFIG_ARM64
	/* Dummy threads start with IRQs masked and dead threads are being
	 * torn down; in both cases the IRQ state self-heals on the next
	 * switch-in, so they are exempt.  Disabled on ARM64 where FP/SIMD
	 * usage in exception context may leave IRQs masked without holding
	 * a lock (see #94285).
	 */
	__ASSERT(arch_irq_unlocked(कुञ्चिका) ||
		 _current->आधारः.thread_state & (_THREAD_DUMMY | _THREAD_DEAD),
		 "Context switching with irq_lock held!");
#यदि_अन्तः
}

#यदि_परिभाषितम् CONFIG_KERNEL_COHERENCE
bool z_spin_तालकम्_mem_coherent(संरचना k_spinlock *l)
{
	प्रत्यागम sys_cache_is_mem_coherent((शून्य *)l);
}
EXPORT_चिह्नम्(z_spin_तालकम्_mem_coherent);
#यदि_अन्तः /* CONFIG_KERNEL_COHERENCE */
