/*
 * Copyright (c) 2018,2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#அந்தர்பாவ <kernel_internal.h>
#அந்தர்பாவ <zephyr/spinlock.h>
#அந்தர்பாவ <zephyr/llext/symbol.h>

#define SPIN_CPU_ID_MASK (அளவு(வெறுமை *) - 1)

/* Outermost spinlock held on each CPU; set-if-NULL on lock,
 * clear-if-matches on unlock.
 */
நிலையான கட்டமைப்பு k_spinlock *z_held_spinlock[CONFIG_MP_MAX_இலக்கம்_CPUS];

/* Total spinlock hold count per CPU, incremented on every lock and
 * decremented on every unlock regardless of nesting depth.
 */
நிலையான முழு z_held_spinlock_மொத்தம்[CONFIG_MP_MAX_இலக்கம்_CPUS];

/* Sentinel: set in swap_data by the ztest harness before force-aborting a
 * thread that holds a lock after an expected panic/assert.
 */
மாறிலி uint8_t z_spinlock_abort_sentinel;

bool z_spin_lock_valid(கட்டமைப்பு k_spinlock *l)
{
	uintptr_t thread_cpu = l->thread_cpu;

	எனில் (thread_cpu != 0U) {
		எனில் ((thread_cpu & SPIN_CPU_ID_MASK) == _current_cpu->id) {
			திரும்பு false;
		}
	}
	திரும்பு true;
}
EXPORT_குறியீடு(z_spin_lock_valid);

/* z_spin_unlock_valid() and z_spin_lock_set_owner() below use _raw_current
 * rather than _current: they run at points where z_current_thread_set() may
 * just have changed the current thread, which an architecture caching it in a
 * register (CONFIG_ARCH_HAS_CUSTOM_CURRENT_IMPL) would not report.  Both are
 * called from k_spin_lock() and k_spin_unlock() with the lock held, hence with
 * interrupts locked as _raw_current requires.
 */
bool z_spin_unlock_valid(கட்டமைப்பு k_spinlock *l)
{
	uint8_t cpu_id = _current_cpu->id;
	uintptr_t tcpu = l->thread_cpu;

	l->thread_cpu = 0;

	எனில் (z_held_spinlock[cpu_id] == l) {
		z_held_spinlock[cpu_id] = வெற்று;
	}

	எனில் (arch_is_in_isr() &&
	    (_raw_current->base.thread_state & _THREAD_DUMMY) != 0) {
		/* Edge case where an ISR aborted _current */
		z_held_spinlock_மொத்தம்[cpu_id]--;
		__ASSERT(z_held_spinlock_மொத்தம்[cpu_id] >= 0,
			 "spinlock unlock with no matching lock");
		திரும்பு true;
	}
	எனில் (tcpu != (cpu_id | (uintptr_t)_raw_current)) {
		திரும்பு false;
	}
	z_held_spinlock_மொத்தம்[cpu_id]--;
	__ASSERT(z_held_spinlock_மொத்தம்[cpu_id] >= 0,
		 "spinlock unlock with no matching lock");
	திரும்பு true;
}
EXPORT_குறியீடு(z_spin_unlock_valid);

வெறுமை z_spin_lock_set_owner(கட்டமைப்பு k_spinlock *l)
{
	uint8_t cpu_id = _current_cpu->id;

	l->thread_cpu = cpu_id | (uintptr_t)_raw_current;

	எனில் (z_held_spinlock[cpu_id] == வெற்று) {
		z_held_spinlock[cpu_id] = l;
	}
	z_held_spinlock_மொத்தம்[cpu_id]++;
}
EXPORT_குறியீடு(z_spin_lock_set_owner);

/* Called from the switch paths after z_current_thread_set() to transfer
 * ownership of an inherited lock.  Does NOT update the tracking arrays.
 *
 * @a thread is passed in rather than read from _current: the caller has just
 * made it current, and _current is not required to report it until the switch
 * completes (see z_current_thread_set()).
 */
வெறுமை z_spin_lock_transfer_owner(கட்டமைப்பு k_spinlock *l, கட்டமைப்பு k_thread *thread)
{
	l->thread_cpu = _current_cpu->id | (uintptr_t)thread;
}
EXPORT_குறியீடு(z_spin_lock_transfer_owner);

/* Reset per-CPU spinlock tracking after abnormal thread termination.
 * All locks the dying thread held are abandoned (it will never resume to
 * release them), so only the locks still being released by the cleanup
 * path need to be counted.  lock_held=true if exactly one such lock
 * remains and will be released after this call.
 */
வெறுமை z_spin_validate_reset(bool lock_held)
{
	uint8_t cpu_id = _current_cpu->id;

	z_held_spinlock[cpu_id] = வெற்று;
	z_held_spinlock_மொத்தம்[cpu_id] = lock_held ? 1 : 0;
}

/* Internal: caller must ensure IRQs are disabled. */
நிலையான உள்வரி கட்டமைப்பு k_spinlock *z_spin_get_held_lock_locked(uint8_t cpu_id)
{
	திரும்பு z_held_spinlock[cpu_id];
}

#ifdef CONFIG_TEST
கட்டமைப்பு k_spinlock *z_spin_get_held_lock(வெறுமை)
{
	unsigned முழு சாவி = arch_irq_lock();
	கட்டமைப்பு k_spinlock *l = z_spin_get_held_lock_locked(_current_cpu->id);

	arch_irq_unlock(சாவி);
	திரும்பு l;
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
வெறுமை z_assert_can_swap(unsigned முழு சாவி, கட்டமைப்பு k_spinlock *swap_lock)
{
	uint8_t cpu_id = _current_cpu->id;
	கட்டமைப்பு k_spinlock *held = z_spin_get_held_lock_locked(cpu_id);
	முழு மொத்தம் = z_held_spinlock_மொத்தம்[cpu_id];
	முழு extra = (swap_lock != வெற்று) ? மொத்தம் - 1 : மொத்தம்;

	ARG_UNUSED(held);
	ARG_UNUSED(extra);

	/* ztest expected-fault: thread aborted with a lock held.*/
	எனில் ((_current->base.thread_state & _THREAD_DEAD) &&
	    _current->base.swap_data == (வெறுமை *)&z_spinlock_abort_sentinel) {
		z_spin_validate_reset(swap_lock != வெற்று);
	} இல்லையெனில் {
		__ASSERT(extra >= 0,
			 "swap_lock %p is not tracked in the spinlock hold count!", swap_lock);

		__ASSERT(held == வெற்று || held == swap_lock,
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
	__ASSERT(arch_irq_unlocked(சாவி) ||
		 _current->base.thread_state & (_THREAD_DUMMY | _THREAD_DEAD),
		 "Context switching with irq_lock held!");
#endif
}

#ifdef CONFIG_KERNEL_COHERENCE
bool z_spin_lock_mem_coherent(கட்டமைப்பு k_spinlock *l)
{
	திரும்பு sys_cache_is_mem_coherent((வெறுமை *)l);
}
EXPORT_குறியீடு(z_spin_lock_mem_coherent);
#endif /* CONFIG_KERNEL_COHERENCE */
