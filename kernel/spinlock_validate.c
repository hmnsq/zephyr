/*
 * Copyright (c) 2018,2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ଅନ୍ତର୍ଭାଵ <kernel_internal.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/spinlock.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/llext/symbol.h>

#define SPIN_CPU_ID_MASK (ଆକାର(ଶୂନ୍ୟ *) - 1)

/* Outermost spinlock held on each CPU; set-if-NULL on lock,
 * clear-if-matches on unlock.
 */
ସ୍ଥିର ସଂରଚନା k_spinlock *z_held_spinlock[CONFIG_MP_MAX_ଅଙ୍କ_CPUS];

/* Total spinlock hold count per CPU, incremented on every lock and
 * decremented on every unlock regardless of nesting depth.
 */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା z_held_spinlock_ଗଣନା[CONFIG_MP_MAX_ଅଙ୍କ_CPUS];

/* Sentinel: set in swap_data by the ztest harness before force-aborting a
 * thread that holds a lock after an expected panic/assert.
 */
ଅଚଳ uint8_t z_spinlock_abort_sentinel;

bool z_spin_lock_valid(ସଂରଚନା k_spinlock *l)
{
	uintptr_t thread_cpu = l->thread_cpu;

	ଯଦି (thread_cpu != 0U) {
		ଯଦି ((thread_cpu & SPIN_CPU_ID_MASK) == _current_cpu->id) {
			ଫେରନ୍ତୁ false;
		}
	}
	ଫେରନ୍ତୁ true;
}
EXPORT_ପ୍ରତୀକ(z_spin_lock_valid);

/* z_spin_unlock_valid() and z_spin_lock_set_owner() below use _raw_current
 * rather than _current: they run at points where z_current_thread_set() may
 * just have changed the current thread, which an architecture caching it in a
 * register (CONFIG_ARCH_HAS_CUSTOM_CURRENT_IMPL) would not report.  Both are
 * called from k_spin_lock() and k_spin_unlock() with the lock held, hence with
 * interrupts locked as _raw_current requires.
 */
bool z_spin_unlock_valid(ସଂରଚନା k_spinlock *l)
{
	uint8_t cpu_id = _current_cpu->id;
	uintptr_t tcpu = l->thread_cpu;

	l->thread_cpu = 0;

	ଯଦି (z_held_spinlock[cpu_id] == l) {
		z_held_spinlock[cpu_id] = ଖାଲି;
	}

	ଯଦି (arch_is_in_isr() &&
	    (_raw_current->base.thread_state & _THREAD_DUMMY) != 0) {
		/* Edge case where an ISR aborted _current */
		z_held_spinlock_ଗଣନା[cpu_id]--;
		__ASSERT(z_held_spinlock_ଗଣନା[cpu_id] >= 0,
			 "spinlock unlock with no matching lock");
		ଫେରନ୍ତୁ true;
	}
	ଯଦି (tcpu != (cpu_id | (uintptr_t)_raw_current)) {
		ଫେରନ୍ତୁ false;
	}
	z_held_spinlock_ଗଣନା[cpu_id]--;
	__ASSERT(z_held_spinlock_ଗଣନା[cpu_id] >= 0,
		 "spinlock unlock with no matching lock");
	ଫେରନ୍ତୁ true;
}
EXPORT_ପ୍ରତୀକ(z_spin_unlock_valid);

ଶୂନ୍ୟ z_spin_lock_set_owner(ସଂରଚନା k_spinlock *l)
{
	uint8_t cpu_id = _current_cpu->id;

	l->thread_cpu = cpu_id | (uintptr_t)_raw_current;

	ଯଦି (z_held_spinlock[cpu_id] == ଖାଲି) {
		z_held_spinlock[cpu_id] = l;
	}
	z_held_spinlock_ଗଣନା[cpu_id]++;
}
EXPORT_ପ୍ରତୀକ(z_spin_lock_set_owner);

/* Called from the switch paths after z_current_thread_set() to transfer
 * ownership of an inherited lock.  Does NOT update the tracking arrays.
 *
 * @a thread is passed in rather than read from _current: the caller has just
 * made it current, and _current is not required to report it until the switch
 * completes (see z_current_thread_set()).
 */
ଶୂନ୍ୟ z_spin_lock_transfer_owner(ସଂରଚନା k_spinlock *l, ସଂରଚନା k_thread *thread)
{
	l->thread_cpu = _current_cpu->id | (uintptr_t)thread;
}
EXPORT_ପ୍ରତୀକ(z_spin_lock_transfer_owner);

/* Reset per-CPU spinlock tracking after abnormal thread termination.
 * All locks the dying thread held are abandoned (it will never resume to
 * release them), so only the locks still being released by the cleanup
 * path need to be counted.  lock_held=true if exactly one such lock
 * remains and will be released after this call.
 */
ଶୂନ୍ୟ z_spin_validate_reset(bool lock_held)
{
	uint8_t cpu_id = _current_cpu->id;

	z_held_spinlock[cpu_id] = ଖାଲି;
	z_held_spinlock_ଗଣନା[cpu_id] = lock_held ? 1 : 0;
}

/* Internal: caller must ensure IRQs are disabled. */
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ସଂରଚନା k_spinlock *z_spin_get_held_lock_locked(uint8_t cpu_id)
{
	ଫେରନ୍ତୁ z_held_spinlock[cpu_id];
}

#ifdef CONFIG_TEST
ସଂରଚନା k_spinlock *z_spin_get_held_lock(ଶୂନ୍ୟ)
{
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଚାବି = arch_irq_lock();
	ସଂରଚନା k_spinlock *l = z_spin_get_held_lock_locked(_current_cpu->id);

	arch_irq_unlock(ଚାବି);
	ଫେରନ୍ତୁ l;
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
ଶୂନ୍ୟ z_assert_can_swap(unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଚାବି, ସଂରଚନା k_spinlock *swap_lock)
{
	uint8_t cpu_id = _current_cpu->id;
	ସଂରଚନା k_spinlock *held = z_spin_get_held_lock_locked(cpu_id);
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଗଣନା_2 = z_held_spinlock_ଗଣନା[cpu_id];
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା extra = (swap_lock != ଖାଲି) ? ଗଣନା_2 - 1 : ଗଣନା_2;

	ARG_UNUSED(held);
	ARG_UNUSED(extra);

	/* ztest expected-fault: thread aborted with a lock held.*/
	ଯଦି ((_current->base.thread_state & _THREAD_DEAD) &&
	    _current->base.swap_data == (ଶୂନ୍ୟ *)&z_spinlock_abort_sentinel) {
		z_spin_validate_reset(swap_lock != ଖାଲି);
	} ଅନ୍ୟଥା {
		__ASSERT(extra >= 0,
			 "swap_lock %p is not tracked in the spinlock hold count!", swap_lock);

		__ASSERT(held == ଖାଲି || held == swap_lock,
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
	__ASSERT(arch_irq_unlocked(ଚାବି) ||
		 _current->base.thread_state & (_THREAD_DUMMY | _THREAD_DEAD),
		 "Context switching with irq_lock held!");
#endif
}

#ifdef CONFIG_KERNEL_COHERENCE
bool z_spin_lock_mem_coherent(ସଂରଚନା k_spinlock *l)
{
	ଫେରନ୍ତୁ sys_cache_is_mem_coherent((ଶୂନ୍ୟ *)l);
}
EXPORT_ପ୍ରତୀକ(z_spin_lock_mem_coherent);
#endif /* CONFIG_KERNEL_COHERENCE */
