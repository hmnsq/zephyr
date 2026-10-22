/*
 * Copyright (c) 2018,2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#समावेश <kernel_internal.h>
#समावेश <zephyr/spinlock.h>
#समावेश <zephyr/llext/symbol.h>

#घोषणा SPIN_CPU_ID_MASK (माप(व्योम *) - 1)

/* Outermost spinlock held on each CPU; set-if-NULL on lock,
 * clear-if-matches on unlock.
 */
अटल काष्ठा k_spinlock *z_held_spinlock[CONFIG_MP_MAX_अंक_CPUS];

/* Total spinlock hold count per CPU, incremented on every lock and
 * decremented on every unlock regardless of nesting depth.
 */
अटल पूर्णांक z_held_spinlock_गिनती[CONFIG_MP_MAX_अंक_CPUS];

/* Sentinel: set in swap_data by the ztest harness before force-aborting a
 * thread that holds a lock after an expected panic/assert.
 */
स्थिर uint8_t z_spinlock_abort_sentinel;

bool z_spin_lock_valid(काष्ठा k_spinlock *l)
{
	uintptr_t thread_cpu = l->thread_cpu;

	अगर (thread_cpu != 0U) {
		अगर ((thread_cpu & SPIN_CPU_ID_MASK) == _current_cpu->id) {
			वापस false;
		}
	}
	वापस true;
}
EXPORT_प्रतीक(z_spin_lock_valid);

/* z_spin_unlock_valid() and z_spin_lock_set_owner() below use _raw_current
 * rather than _current: they run at points where z_current_thread_set() may
 * just have changed the current thread, which an architecture caching it in a
 * register (CONFIG_ARCH_HAS_CUSTOM_CURRENT_IMPL) would not report.  Both are
 * called from k_spin_lock() and k_spin_unlock() with the lock held, hence with
 * interrupts locked as _raw_current requires.
 */
bool z_spin_unlock_valid(काष्ठा k_spinlock *l)
{
	uint8_t cpu_id = _current_cpu->id;
	uintptr_t tcpu = l->thread_cpu;

	l->thread_cpu = 0;

	अगर (z_held_spinlock[cpu_id] == l) {
		z_held_spinlock[cpu_id] = शून्य;
	}

	अगर (arch_is_in_isr() &&
	    (_raw_current->base.thread_state & _THREAD_DUMMY) != 0) {
		/* Edge case where an ISR aborted _current */
		z_held_spinlock_गिनती[cpu_id]--;
		__ASSERT(z_held_spinlock_गिनती[cpu_id] >= 0,
			 "spinlock unlock with no matching lock");
		वापस true;
	}
	अगर (tcpu != (cpu_id | (uintptr_t)_raw_current)) {
		वापस false;
	}
	z_held_spinlock_गिनती[cpu_id]--;
	__ASSERT(z_held_spinlock_गिनती[cpu_id] >= 0,
		 "spinlock unlock with no matching lock");
	वापस true;
}
EXPORT_प्रतीक(z_spin_unlock_valid);

व्योम z_spin_lock_set_owner(काष्ठा k_spinlock *l)
{
	uint8_t cpu_id = _current_cpu->id;

	l->thread_cpu = cpu_id | (uintptr_t)_raw_current;

	अगर (z_held_spinlock[cpu_id] == शून्य) {
		z_held_spinlock[cpu_id] = l;
	}
	z_held_spinlock_गिनती[cpu_id]++;
}
EXPORT_प्रतीक(z_spin_lock_set_owner);

/* Called from the switch paths after z_current_thread_set() to transfer
 * ownership of an inherited lock.  Does NOT update the tracking arrays.
 *
 * @a thread is passed in rather than read from _current: the caller has just
 * made it current, and _current is not required to report it until the switch
 * completes (see z_current_thread_set()).
 */
व्योम z_spin_lock_transfer_owner(काष्ठा k_spinlock *l, काष्ठा k_thread *thread)
{
	l->thread_cpu = _current_cpu->id | (uintptr_t)thread;
}
EXPORT_प्रतीक(z_spin_lock_transfer_owner);

/* Reset per-CPU spinlock tracking after abnormal thread termination.
 * All locks the dying thread held are abandoned (it will never resume to
 * release them), so only the locks still being released by the cleanup
 * path need to be counted.  lock_held=true if exactly one such lock
 * remains and will be released after this call.
 */
व्योम z_spin_validate_reset(bool lock_held)
{
	uint8_t cpu_id = _current_cpu->id;

	z_held_spinlock[cpu_id] = शून्य;
	z_held_spinlock_गिनती[cpu_id] = lock_held ? 1 : 0;
}

/* Internal: caller must ensure IRQs are disabled. */
अटल अंतरभूत काष्ठा k_spinlock *z_spin_get_held_lock_locked(uint8_t cpu_id)
{
	वापस z_held_spinlock[cpu_id];
}

#अगर_घोषित CONFIG_TEST
काष्ठा k_spinlock *z_spin_get_held_lock(व्योम)
{
	अचिन्हित पूर्णांक कुंजी = arch_irq_lock();
	काष्ठा k_spinlock *l = z_spin_get_held_lock_locked(_current_cpu->id);

	arch_irq_unlock(कुंजी);
	वापस l;
}
#पूर्ण_अगर /* CONFIG_TEST */

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
व्योम z_assert_can_swap(अचिन्हित पूर्णांक कुंजी, काष्ठा k_spinlock *swap_lock)
{
	uint8_t cpu_id = _current_cpu->id;
	काष्ठा k_spinlock *held = z_spin_get_held_lock_locked(cpu_id);
	पूर्णांक गिनती = z_held_spinlock_गिनती[cpu_id];
	पूर्णांक extra = (swap_lock != शून्य) ? गिनती - 1 : गिनती;

	ARG_UNUSED(held);
	ARG_UNUSED(extra);

	/* ztest expected-fault: thread aborted with a lock held.*/
	अगर ((_current->base.thread_state & _THREAD_DEAD) &&
	    _current->base.swap_तथ्य == (व्योम *)&z_spinlock_abort_sentinel) {
		z_spin_validate_reset(swap_lock != शून्य);
	} अन्यथा {
		__ASSERT(extra >= 0,
			 "swap_lock %p is not tracked in the spinlock hold count!", swap_lock);

		__ASSERT(held == शून्य || held == swap_lock,
			 "Context switching while holding spinlock %p!", held);

		/* Catches lock(A); lock(B); unlock(A); z_swap(): slot cleared when A
		 * is released but count stays at 1 because B is still held.
		 */
		__ASSERT(extra == 0,
			 "Context switching while holding %d extra spinlock(s)!", extra);
	}

#अगर_अघोषित CONFIG_ARM64
	/* Dummy threads start with IRQs masked and dead threads are being
	 * torn down; in both cases the IRQ state self-heals on the next
	 * switch-in, so they are exempt.  Disabled on ARM64 where FP/SIMD
	 * usage in exception context may leave IRQs masked without holding
	 * a lock (see #94285).
	 */
	__ASSERT(arch_irq_unlocked(कुंजी) ||
		 _current->base.thread_state & (_THREAD_DUMMY | _THREAD_DEAD),
		 "Context switching with irq_lock held!");
#पूर्ण_अगर
}

#अगर_घोषित CONFIG_KERNEL_COHERENCE
bool z_spin_lock_mem_coherent(काष्ठा k_spinlock *l)
{
	वापस sys_cache_is_mem_coherent((व्योम *)l);
}
EXPORT_प्रतीक(z_spin_lock_mem_coherent);
#पूर्ण_अगर /* CONFIG_KERNEL_COHERENCE */
