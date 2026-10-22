/*
 * Copyright (c) 2018,2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#অন্তর্ভাব <kernel_internal.h>
#অন্তর্ভাব <zephyr/spinlock.h>
#অন্তর্ভাব <zephyr/llext/symbol.h>

#define SPIN_CPU_ID_MASK (পরিমাণ(শূন্য *) - 1)

/* Outermost spinlock held on each CPU; set-if-NULL on lock,
 * clear-if-matches on unlock.
 */
স্থির সংরচনা k_spinlock *z_held_spinlock[CONFIG_MP_MAX_অঙ্ক_CPUS];

/* Total spinlock hold count per CPU, incremented on every lock and
 * decremented on every unlock regardless of nesting depth.
 */
স্থির পূর্ণাঙ্ক z_held_spinlock_গুনতি[CONFIG_MP_MAX_অঙ্ক_CPUS];

/* Sentinel: set in swap_data by the ztest harness before force-aborting a
 * thread that holds a lock after an expected panic/assert.
 */
ধ্রুব uint8_t z_spinlock_abort_sentinel;

bool z_spin_lock_valid(সংরচনা k_spinlock *l)
{
	uintptr_t thread_cpu = l->thread_cpu;

	যদি (thread_cpu != 0U) {
		যদি ((thread_cpu & SPIN_CPU_ID_MASK) == _current_cpu->id) {
			প্রত্যাগম false;
		}
	}
	প্রত্যাগম true;
}
EXPORT_প্রতীক(z_spin_lock_valid);

/* z_spin_unlock_valid() and z_spin_lock_set_owner() below use _raw_current
 * rather than _current: they run at points where z_current_thread_set() may
 * just have changed the current thread, which an architecture caching it in a
 * register (CONFIG_ARCH_HAS_CUSTOM_CURRENT_IMPL) would not report.  Both are
 * called from k_spin_lock() and k_spin_unlock() with the lock held, hence with
 * interrupts locked as _raw_current requires.
 */
bool z_spin_unlock_valid(সংরচনা k_spinlock *l)
{
	uint8_t cpu_id = _current_cpu->id;
	uintptr_t tcpu = l->thread_cpu;

	l->thread_cpu = 0;

	যদি (z_held_spinlock[cpu_id] == l) {
		z_held_spinlock[cpu_id] = রিক্ত;
	}

	যদি (arch_is_in_isr() &&
	    (_raw_current->base.thread_state & _THREAD_DUMMY) != 0) {
		/* Edge case where an ISR aborted _current */
		z_held_spinlock_গুনতি[cpu_id]--;
		__ASSERT(z_held_spinlock_গুনতি[cpu_id] >= 0,
			 "spinlock unlock with no matching lock");
		প্রত্যাগম true;
	}
	যদি (tcpu != (cpu_id | (uintptr_t)_raw_current)) {
		প্রত্যাগম false;
	}
	z_held_spinlock_গুনতি[cpu_id]--;
	__ASSERT(z_held_spinlock_গুনতি[cpu_id] >= 0,
		 "spinlock unlock with no matching lock");
	প্রত্যাগম true;
}
EXPORT_প্রতীক(z_spin_unlock_valid);

শূন্য z_spin_lock_set_owner(সংরচনা k_spinlock *l)
{
	uint8_t cpu_id = _current_cpu->id;

	l->thread_cpu = cpu_id | (uintptr_t)_raw_current;

	যদি (z_held_spinlock[cpu_id] == রিক্ত) {
		z_held_spinlock[cpu_id] = l;
	}
	z_held_spinlock_গুনতি[cpu_id]++;
}
EXPORT_প্রতীক(z_spin_lock_set_owner);

/* Called from the switch paths after z_current_thread_set() to transfer
 * ownership of an inherited lock.  Does NOT update the tracking arrays.
 *
 * @a thread is passed in rather than read from _current: the caller has just
 * made it current, and _current is not required to report it until the switch
 * completes (see z_current_thread_set()).
 */
শূন্য z_spin_lock_transfer_owner(সংরচনা k_spinlock *l, সংরচনা k_thread *thread)
{
	l->thread_cpu = _current_cpu->id | (uintptr_t)thread;
}
EXPORT_প্রতীক(z_spin_lock_transfer_owner);

/* Reset per-CPU spinlock tracking after abnormal thread termination.
 * All locks the dying thread held are abandoned (it will never resume to
 * release them), so only the locks still being released by the cleanup
 * path need to be counted.  lock_held=true if exactly one such lock
 * remains and will be released after this call.
 */
শূন্য z_spin_validate_reset(bool lock_held)
{
	uint8_t cpu_id = _current_cpu->id;

	z_held_spinlock[cpu_id] = রিক্ত;
	z_held_spinlock_গুনতি[cpu_id] = lock_held ? 1 : 0;
}

/* Internal: caller must ensure IRQs are disabled. */
স্থির অন্তঃস্থ সংরচনা k_spinlock *z_spin_get_held_lock_locked(uint8_t cpu_id)
{
	প্রত্যাগম z_held_spinlock[cpu_id];
}

#ifdef CONFIG_TEST
সংরচনা k_spinlock *z_spin_get_held_lock(শূন্য)
{
	unsigned পূর্ণাঙ্ক চাবি = arch_irq_lock();
	সংরচনা k_spinlock *l = z_spin_get_held_lock_locked(_current_cpu->id);

	arch_irq_unlock(চাবি);
	প্রত্যাগম l;
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
শূন্য z_assert_can_swap(unsigned পূর্ণাঙ্ক চাবি, সংরচনা k_spinlock *swap_lock)
{
	uint8_t cpu_id = _current_cpu->id;
	সংরচনা k_spinlock *held = z_spin_get_held_lock_locked(cpu_id);
	পূর্ণাঙ্ক গুনতি = z_held_spinlock_গুনতি[cpu_id];
	পূর্ণাঙ্ক extra = (swap_lock != রিক্ত) ? গুনতি - 1 : গুনতি;

	ARG_UNUSED(held);
	ARG_UNUSED(extra);

	/* ztest expected-fault: thread aborted with a lock held.*/
	যদি ((_current->base.thread_state & _THREAD_DEAD) &&
	    _current->base.swap_data == (শূন্য *)&z_spinlock_abort_sentinel) {
		z_spin_validate_reset(swap_lock != রিক্ত);
	} অন্যথা {
		__ASSERT(extra >= 0,
			 "swap_lock %p is not tracked in the spinlock hold count!", swap_lock);

		__ASSERT(held == রিক্ত || held == swap_lock,
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
	__ASSERT(arch_irq_unlocked(চাবি) ||
		 _current->base.thread_state & (_THREAD_DUMMY | _THREAD_DEAD),
		 "Context switching with irq_lock held!");
#endif
}

#ifdef CONFIG_KERNEL_COHERENCE
bool z_spin_lock_mem_coherent(সংরচনা k_spinlock *l)
{
	প্রত্যাগম sys_cache_is_mem_coherent((শূন্য *)l);
}
EXPORT_প্রতীক(z_spin_lock_mem_coherent);
#endif /* CONFIG_KERNEL_COHERENCE */
