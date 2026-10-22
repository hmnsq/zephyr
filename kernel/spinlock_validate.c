/*
 * Copyright (c) 2018,2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#અન્તર્ભાવ <kernel_internal.h>
#અન્તર્ભાવ <zephyr/spinlock.h>
#અન્તર્ભાવ <zephyr/llext/symbol.h>

#define SPIN_CPU_ID_MASK (કદ(શૂન્ય *) - 1)

/* Outermost spinlock held on each CPU; set-if-NULL on lock,
 * clear-if-matches on unlock.
 */
સ્થિર સંરચના k_spinlock *z_held_spinlock[CONFIG_MP_MAX_અંક_CPUS];

/* Total spinlock hold count per CPU, incremented on every lock and
 * decremented on every unlock regardless of nesting depth.
 */
સ્થિર પૂર્ણાંક z_held_spinlock_ગણતરી[CONFIG_MP_MAX_અંક_CPUS];

/* Sentinel: set in swap_data by the ztest harness before force-aborting a
 * thread that holds a lock after an expected panic/assert.
 */
અચલ uint8_t z_spinlock_abort_sentinel;

bool z_spin_lock_valid(સંરચના k_spinlock *l)
{
	uintptr_t thread_cpu = l->thread_cpu;

	જો (thread_cpu != 0U) {
		જો ((thread_cpu & SPIN_CPU_ID_MASK) == _current_cpu->id) {
			પાછા false;
		}
	}
	પાછા true;
}
EXPORT_પ્રતીક(z_spin_lock_valid);

/* z_spin_unlock_valid() and z_spin_lock_set_owner() below use _raw_current
 * rather than _current: they run at points where z_current_thread_set() may
 * just have changed the current thread, which an architecture caching it in a
 * register (CONFIG_ARCH_HAS_CUSTOM_CURRENT_IMPL) would not report.  Both are
 * called from k_spin_lock() and k_spin_unlock() with the lock held, hence with
 * interrupts locked as _raw_current requires.
 */
bool z_spin_unlock_valid(સંરચના k_spinlock *l)
{
	uint8_t cpu_id = _current_cpu->id;
	uintptr_t tcpu = l->thread_cpu;

	l->thread_cpu = 0;

	જો (z_held_spinlock[cpu_id] == l) {
		z_held_spinlock[cpu_id] = ખાલી;
	}

	જો (arch_is_in_isr() &&
	    (_raw_current->base.thread_state & _THREAD_DUMMY) != 0) {
		/* Edge case where an ISR aborted _current */
		z_held_spinlock_ગણતરી[cpu_id]--;
		__ASSERT(z_held_spinlock_ગણતરી[cpu_id] >= 0,
			 "spinlock unlock with no matching lock");
		પાછા true;
	}
	જો (tcpu != (cpu_id | (uintptr_t)_raw_current)) {
		પાછા false;
	}
	z_held_spinlock_ગણતરી[cpu_id]--;
	__ASSERT(z_held_spinlock_ગણતરી[cpu_id] >= 0,
		 "spinlock unlock with no matching lock");
	પાછા true;
}
EXPORT_પ્રતીક(z_spin_unlock_valid);

શૂન્ય z_spin_lock_set_owner(સંરચના k_spinlock *l)
{
	uint8_t cpu_id = _current_cpu->id;

	l->thread_cpu = cpu_id | (uintptr_t)_raw_current;

	જો (z_held_spinlock[cpu_id] == ખાલી) {
		z_held_spinlock[cpu_id] = l;
	}
	z_held_spinlock_ગણતરી[cpu_id]++;
}
EXPORT_પ્રતીક(z_spin_lock_set_owner);

/* Called from the switch paths after z_current_thread_set() to transfer
 * ownership of an inherited lock.  Does NOT update the tracking arrays.
 *
 * @a thread is passed in rather than read from _current: the caller has just
 * made it current, and _current is not required to report it until the switch
 * completes (see z_current_thread_set()).
 */
શૂન્ય z_spin_lock_transfer_owner(સંરચના k_spinlock *l, સંરચના k_thread *thread)
{
	l->thread_cpu = _current_cpu->id | (uintptr_t)thread;
}
EXPORT_પ્રતીક(z_spin_lock_transfer_owner);

/* Reset per-CPU spinlock tracking after abnormal thread termination.
 * All locks the dying thread held are abandoned (it will never resume to
 * release them), so only the locks still being released by the cleanup
 * path need to be counted.  lock_held=true if exactly one such lock
 * remains and will be released after this call.
 */
શૂન્ય z_spin_validate_reset(bool lock_held)
{
	uint8_t cpu_id = _current_cpu->id;

	z_held_spinlock[cpu_id] = ખાલી;
	z_held_spinlock_ગણતરી[cpu_id] = lock_held ? 1 : 0;
}

/* Internal: caller must ensure IRQs are disabled. */
સ્થિર અંતઃરેખ સંરચના k_spinlock *z_spin_get_held_lock_locked(uint8_t cpu_id)
{
	પાછા z_held_spinlock[cpu_id];
}

#ifdef CONFIG_TEST
સંરચના k_spinlock *z_spin_get_held_lock(શૂન્ય)
{
	unsigned પૂર્ણાંક ચાવી = arch_irq_lock();
	સંરચના k_spinlock *l = z_spin_get_held_lock_locked(_current_cpu->id);

	arch_irq_unlock(ચાવી);
	પાછા l;
}
#endif /* CONFIG_TEST */

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
શૂન્ય z_assert_can_swap(unsigned પૂર્ણાંક ચાવી, સંરચના k_spinlock *swap_lock)
{
	uint8_t cpu_id = _current_cpu->id;
	સંરચના k_spinlock *held = z_spin_get_held_lock_locked(cpu_id);
	પૂર્ણાંક ગણતરી = z_held_spinlock_ગણતરી[cpu_id];
	પૂર્ણાંક extra = (swap_lock != ખાલી) ? ગણતરી - 1 : ગણતરી;

	ARG_UNUSED(held);
	ARG_UNUSED(extra);

	/* ztest expected-fault: thread aborted with a lock held.*/
	જો ((_current->base.thread_state & _THREAD_DEAD) &&
	    _current->base.swap_data == (શૂન્ય *)&z_spinlock_abort_sentinel) {
		z_spin_validate_reset(swap_lock != ખાલી);
	} નહીંતર {
		__ASSERT(extra >= 0,
			 "swap_lock %p is not tracked in the spinlock hold count!", swap_lock);

		__ASSERT(held == ખાલી || held == swap_lock,
			 "Context switching while holding spinlock %p!", held);

		/* Catches lock(A); lock(B); unlock(A); z_swap(): slot cleared when A
		 * is released but count stays at 1 because B is still held.
		 */
		__ASSERT(extra == 0,
			 "Context switching while holding %d extra spinlock(s)!", extra);
	}

#ifndef CONFIG_ARM64
	/* Dummy threads start with IRQs masked and dead threads are being
	 * torn down; in both cases the IRQ state self-heals on the next
	 * switch-in, so they are exempt.  Disabled on ARM64 where FP/SIMD
	 * usage in exception context may leave IRQs masked without holding
	 * a lock (see #94285).
	 */
	__ASSERT(arch_irq_unlocked(ચાવી) ||
		 _current->base.thread_state & (_THREAD_DUMMY | _THREAD_DEAD),
		 "Context switching with irq_lock held!");
#endif
}

#ifdef CONFIG_KERNEL_COHERENCE
bool z_spin_lock_mem_coherent(સંરચના k_spinlock *l)
{
	પાછા sys_cache_is_mem_coherent((શૂન્ય *)l);
}
EXPORT_પ્રતીક(z_spin_lock_mem_coherent);
#endif /* CONFIG_KERNEL_COHERENCE */
