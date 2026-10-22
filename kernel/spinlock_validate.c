/*
 * Copyright (c) 2018,2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ಅನ್ತರ್ಭಾವ <kernel_internal.h>
#ಅನ್ತರ್ಭಾವ <zephyr/spinlock.h>
#ಅನ್ತರ್ಭಾವ <zephyr/llext/symbol.h>

#ವ್ಯಾಖ್ಯಾನಿಸು SPIN_CPU_ID_ಮುಖವಾಡ (ಗಾತ್ರ(ಶೂನ್ಯ *) - 1)

/* Outermost spinlock held on each CPU; set-if-NULL on lock,
 * clear-if-matches on unlock.
 */
ಸ್ಥಿರ ರಚನೆ k_spinlock *z_held_spinlock[CONFIG_MP_MAX_ಅಂಕೆ_CPUS];

/* Total spinlock hold count per CPU, incremented on every lock and
 * decremented on every unlock regardless of nesting depth.
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ z_held_spinlock_ಎಣಿಕೆ[CONFIG_MP_MAX_ಅಂಕೆ_CPUS];

/* Sentinel: set in swap_data by the ztest harness before force-aborting a
 * thread that holds a lock after an expected panic/assert.
 */
ಸ್ಥಿರಾಂಕ uint8_t z_spinlock_abort_sentinel;

bool z_spin_ಬೀಗ_valid(ರಚನೆ k_spinlock *l)
{
	uintptr_t thread_cpu = l->thread_cpu;

	ಆದರೆ (thread_cpu != 0U) {
		ಆದರೆ ((thread_cpu & SPIN_CPU_ID_ಮುಖವಾಡ) == _current_cpu->id) {
			ಹಿಂತಿರುಗಿ false;
		}
	}
	ಹಿಂತಿರುಗಿ true;
}
EXPORT_ಚಿಹ್ನೆ(z_spin_ಬೀಗ_valid);

/* z_spin_unlock_valid() and z_spin_lock_set_owner() below use _raw_current
 * rather than _current: they run at points where z_current_thread_set() may
 * just have changed the current thread, which an architecture caching it in a
 * register (CONFIG_ARCH_HAS_CUSTOM_CURRENT_IMPL) would not report.  Both are
 * called from k_spin_lock() and k_spin_unlock() with the lock held, hence with
 * interrupts locked as _raw_current requires.
 */
bool z_spin_unlock_valid(ರಚನೆ k_spinlock *l)
{
	uint8_t cpu_id = _current_cpu->id;
	uintptr_t tcpu = l->thread_cpu;

	l->thread_cpu = 0;

	ಆದರೆ (z_held_spinlock[cpu_id] == l) {
		z_held_spinlock[cpu_id] = ಖಾಲಿ;
	}

	ಆದರೆ (arch_is_ಒಳ_isr() &&
	    (_raw_current->ತಳ.thread_state & _THREAD_DUMMY) != 0) {
		/* Edge case where an ISR aborted _current */
		z_held_spinlock_ಎಣಿಕೆ[cpu_id]--;
		__ASSERT(z_held_spinlock_ಎಣಿಕೆ[cpu_id] >= 0,
			 "spinlock unlock with no matching lock");
		ಹಿಂತಿರುಗಿ true;
	}
	ಆದರೆ (tcpu != (cpu_id | (uintptr_t)_raw_current)) {
		ಹಿಂತಿರುಗಿ false;
	}
	z_held_spinlock_ಎಣಿಕೆ[cpu_id]--;
	__ASSERT(z_held_spinlock_ಎಣಿಕೆ[cpu_id] >= 0,
		 "spinlock unlock with no matching lock");
	ಹಿಂತಿರುಗಿ true;
}
EXPORT_ಚಿಹ್ನೆ(z_spin_unlock_valid);

ಶೂನ್ಯ z_spin_ಬೀಗ_ಗಣ_owner(ರಚನೆ k_spinlock *l)
{
	uint8_t cpu_id = _current_cpu->id;

	l->thread_cpu = cpu_id | (uintptr_t)_raw_current;

	ಆದರೆ (z_held_spinlock[cpu_id] == ಖಾಲಿ) {
		z_held_spinlock[cpu_id] = l;
	}
	z_held_spinlock_ಎಣಿಕೆ[cpu_id]++;
}
EXPORT_ಚಿಹ್ನೆ(z_spin_ಬೀಗ_ಗಣ_owner);

/* Called from the switch paths after z_current_thread_set() to transfer
 * ownership of an inherited lock.  Does NOT update the tracking arrays.
 *
 * @a thread is passed in rather than read from _current: the caller has just
 * made it current, and _current is not required to report it until the switch
 * completes (see z_current_thread_set()).
 */
ಶೂನ್ಯ z_spin_ಬೀಗ_transfer_owner(ರಚನೆ k_spinlock *l, ರಚನೆ k_thread *thread)
{
	l->thread_cpu = _current_cpu->id | (uintptr_t)thread;
}
EXPORT_ಚಿಹ್ನೆ(z_spin_ಬೀಗ_transfer_owner);

/* Reset per-CPU spinlock tracking after abnormal thread termination.
 * All locks the dying thread held are abandoned (it will never resume to
 * release them), so only the locks still being released by the cleanup
 * path need to be counted.  lock_held=true if exactly one such lock
 * remains and will be released after this call.
 */
ಶೂನ್ಯ z_spin_validate_reset(bool ಬೀಗ_held)
{
	uint8_t cpu_id = _current_cpu->id;

	z_held_spinlock[cpu_id] = ಖಾಲಿ;
	z_held_spinlock_ಎಣಿಕೆ[cpu_id] = ಬೀಗ_held ? 1 : 0;
}

/* Internal: caller must ensure IRQs are disabled. */
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ರಚನೆ k_spinlock *z_spin_get_held_ಬೀಗ_locked(uint8_t cpu_id)
{
	ಹಿಂತಿರುಗಿ z_held_spinlock[cpu_id];
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_TEST
ರಚನೆ k_spinlock *z_spin_get_held_ಬೀಗ(ಶೂನ್ಯ)
{
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಕೀಲಿ = arch_irq_ಬೀಗ();
	ರಚನೆ k_spinlock *l = z_spin_get_held_ಬೀಗ_locked(_current_cpu->id);

	arch_irq_unlock(ಕೀಲಿ);
	ಹಿಂತಿರುಗಿ l;
}
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_TEST */

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
ಶೂನ್ಯ z_assert_can_ಅದಲುಬದಲು(ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಕೀಲಿ, ರಚನೆ k_spinlock *ಅದಲುಬದಲು_ಬೀಗ)
{
	uint8_t cpu_id = _current_cpu->id;
	ರಚನೆ k_spinlock *held = z_spin_get_held_ಬೀಗ_locked(cpu_id);
	ಪೂರ್ಣಾಂಕ ಎಣಿಕೆ_2 = z_held_spinlock_ಎಣಿಕೆ[cpu_id];
	ಪೂರ್ಣಾಂಕ extra = (ಅದಲುಬದಲು_ಬೀಗ != ಖಾಲಿ) ? ಎಣಿಕೆ_2 - 1 : ಎಣಿಕೆ_2;

	ARG_ಬಳಸದ(held);
	ARG_ಬಳಸದ(extra);

	/* ztest expected-fault: thread aborted with a lock held.*/
	ಆದರೆ ((_current->ತಳ.thread_state & _THREAD_DEAD) &&
	    _current->ತಳ.ಅದಲುಬದಲು_ದತ್ತಾಂಶ == (ಶೂನ್ಯ *)&z_spinlock_abort_sentinel) {
		z_spin_validate_reset(ಅದಲುಬದಲು_ಬೀಗ != ಖಾಲಿ);
	} ಇಲ್ಲದಿದ್ದರೆ {
		__ASSERT(extra >= 0,
			 "swap_lock %p is not tracked in the spinlock hold count!", ಅದಲುಬದಲು_ಬೀಗ);

		__ASSERT(held == ಖಾಲಿ || held == ಅದಲುಬದಲು_ಬೀಗ,
			 "Context switching while holding spinlock %p!", held);

		/* Catches lock(A); lock(B); unlock(A); z_swap(): slot cleared when A
		 * is released but count stays at 1 because B is still held.
		 */
		__ASSERT(extra == 0,
			 "Context switching while holding %d extra spinlock(s)!", extra);
	}

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ CONFIG_ARM64
	/* Dummy threads start with IRQs masked and dead threads are being
	 * torn down; in both cases the IRQ state self-heals on the next
	 * switch-in, so they are exempt.  Disabled on ARM64 where FP/SIMD
	 * usage in exception context may leave IRQs masked without holding
	 * a lock (see #94285).
	 */
	__ASSERT(arch_irq_unlocked(ಕೀಲಿ) ||
		 _current->ತಳ.thread_state & (_THREAD_DUMMY | _THREAD_DEAD),
		 "Context switching with irq_lock held!");
#ಆದರೆ_ಅಂತ್ಯ
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_KERNEL_COHERENCE
bool z_spin_ಬೀಗ_mem_coherent(ರಚನೆ k_spinlock *l)
{
	ಹಿಂತಿರುಗಿ sys_cache_is_mem_coherent((ಶೂನ್ಯ *)l);
}
EXPORT_ಚಿಹ್ನೆ(z_spin_ಬೀಗ_mem_coherent);
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_KERNEL_COHERENCE */
