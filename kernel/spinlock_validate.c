/*
 * Copyright (c) 2018,2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ਅਨ੍ਤਰ੍ਭਾਵ <kernel_internal.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/spinlock.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/llext/symbol.h>

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SPIN_CPU_ID_ਨਕਾਬ (ਆਕਾਰ(ਖਾਲੀ *) - 1)

/* Outermost spinlock held on each CPU; set-if-NULL on lock,
 * clear-if-matches on unlock.
 */
ਸਥਿਰ ਬਣਤਰ k_spinlock *z_held_spinlock[CONFIG_MP_MAX_ਅੰਕ_CPUS];

/* Total spinlock hold count per CPU, incremented on every lock and
 * decremented on every unlock regardless of nesting depth.
 */
ਸਥਿਰ ਪੂਰਨ z_held_spinlock_ਗਿਣਤੀ[CONFIG_MP_MAX_ਅੰਕ_CPUS];

/* Sentinel: set in swap_data by the ztest harness before force-aborting a
 * thread that holds a lock after an expected panic/assert.
 */
ਅਚਲ uint8_t z_spinlock_abort_sentinel;

bool z_spin_ਤਾਲਾ_valid(ਬਣਤਰ k_spinlock *l)
{
	uintptr_t thread_cpu = l->thread_cpu;

	ਜੇ (thread_cpu != 0U) {
		ਜੇ ((thread_cpu & SPIN_CPU_ID_ਨਕਾਬ) == _current_cpu->id) {
			ਵਾਪਸ false;
		}
	}
	ਵਾਪਸ true;
}
EXPORT_ਚਿੰਨ੍ਹ(z_spin_ਤਾਲਾ_valid);

/* z_spin_unlock_valid() and z_spin_lock_set_owner() below use _raw_current
 * rather than _current: they run at points where z_current_thread_set() may
 * just have changed the current thread, which an architecture caching it in a
 * register (CONFIG_ARCH_HAS_CUSTOM_CURRENT_IMPL) would not report.  Both are
 * called from k_spin_lock() and k_spin_unlock() with the lock held, hence with
 * interrupts locked as _raw_current requires.
 */
bool z_spin_unlock_valid(ਬਣਤਰ k_spinlock *l)
{
	uint8_t cpu_id = _current_cpu->id;
	uintptr_t tcpu = l->thread_cpu;

	l->thread_cpu = 0;

	ਜੇ (z_held_spinlock[cpu_id] == l) {
		z_held_spinlock[cpu_id] = ਨਿਰਾ;
	}

	ਜੇ (arch_is_ਵਿੱਚ_isr() &&
	    (_raw_current->ਅਧਾਰ.thread_state & _THREAD_DUMMY) != 0) {
		/* Edge case where an ISR aborted _current */
		z_held_spinlock_ਗਿਣਤੀ[cpu_id]--;
		__ASSERT(z_held_spinlock_ਗਿਣਤੀ[cpu_id] >= 0,
			 "spinlock unlock with no matching lock");
		ਵਾਪਸ true;
	}
	ਜੇ (tcpu != (cpu_id | (uintptr_t)_raw_current)) {
		ਵਾਪਸ false;
	}
	z_held_spinlock_ਗਿਣਤੀ[cpu_id]--;
	__ASSERT(z_held_spinlock_ਗਿਣਤੀ[cpu_id] >= 0,
		 "spinlock unlock with no matching lock");
	ਵਾਪਸ true;
}
EXPORT_ਚਿੰਨ੍ਹ(z_spin_unlock_valid);

ਖਾਲੀ z_spin_ਤਾਲਾ_ਸੈੱਟ_owner(ਬਣਤਰ k_spinlock *l)
{
	uint8_t cpu_id = _current_cpu->id;

	l->thread_cpu = cpu_id | (uintptr_t)_raw_current;

	ਜੇ (z_held_spinlock[cpu_id] == ਨਿਰਾ) {
		z_held_spinlock[cpu_id] = l;
	}
	z_held_spinlock_ਗਿਣਤੀ[cpu_id]++;
}
EXPORT_ਚਿੰਨ੍ਹ(z_spin_ਤਾਲਾ_ਸੈੱਟ_owner);

/* Called from the switch paths after z_current_thread_set() to transfer
 * ownership of an inherited lock.  Does NOT update the tracking arrays.
 *
 * @a thread is passed in rather than read from _current: the caller has just
 * made it current, and _current is not required to report it until the switch
 * completes (see z_current_thread_set()).
 */
ਖਾਲੀ z_spin_ਤਾਲਾ_transfer_owner(ਬਣਤਰ k_spinlock *l, ਬਣਤਰ k_thread *thread)
{
	l->thread_cpu = _current_cpu->id | (uintptr_t)thread;
}
EXPORT_ਚਿੰਨ੍ਹ(z_spin_ਤਾਲਾ_transfer_owner);

/* Reset per-CPU spinlock tracking after abnormal thread termination.
 * All locks the dying thread held are abandoned (it will never resume to
 * release them), so only the locks still being released by the cleanup
 * path need to be counted.  lock_held=true if exactly one such lock
 * remains and will be released after this call.
 */
ਖਾਲੀ z_spin_validate_reset(bool ਤਾਲਾ_held)
{
	uint8_t cpu_id = _current_cpu->id;

	z_held_spinlock[cpu_id] = ਨਿਰਾ;
	z_held_spinlock_ਗਿਣਤੀ[cpu_id] = ਤਾਲਾ_held ? 1 : 0;
}

/* Internal: caller must ensure IRQs are disabled. */
ਸਥਿਰ ਸਤਰਅੰਦਰ ਬਣਤਰ k_spinlock *z_spin_get_held_ਤਾਲਾ_locked(uint8_t cpu_id)
{
	ਵਾਪਸ z_held_spinlock[cpu_id];
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_TEST
ਬਣਤਰ k_spinlock *z_spin_get_held_ਤਾਲਾ(ਖਾਲੀ)
{
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਕੁੰਜੀ = arch_irq_ਤਾਲਾ();
	ਬਣਤਰ k_spinlock *l = z_spin_get_held_ਤਾਲਾ_locked(_current_cpu->id);

	arch_irq_unlock(ਕੁੰਜੀ);
	ਵਾਪਸ l;
}
#ਜੇ_ਅੰਤ /* CONFIG_TEST */

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
ਖਾਲੀ z_assert_can_ਅਦਲਾ_ਬਦਲੀ(ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਕੁੰਜੀ, ਬਣਤਰ k_spinlock *ਅਦਲਾ_ਬਦਲੀ_ਤਾਲਾ)
{
	uint8_t cpu_id = _current_cpu->id;
	ਬਣਤਰ k_spinlock *held = z_spin_get_held_ਤਾਲਾ_locked(cpu_id);
	ਪੂਰਨ ਗਿਣਤੀ_2 = z_held_spinlock_ਗਿਣਤੀ[cpu_id];
	ਪੂਰਨ extra = (ਅਦਲਾ_ਬਦਲੀ_ਤਾਲਾ != ਨਿਰਾ) ? ਗਿਣਤੀ_2 - 1 : ਗਿਣਤੀ_2;

	ARG_ਅਣਵਰਤਿਆ(held);
	ARG_ਅਣਵਰਤਿਆ(extra);

	/* ztest expected-fault: thread aborted with a lock held.*/
	ਜੇ ((_current->ਅਧਾਰ.thread_state & _THREAD_DEAD) &&
	    _current->ਅਧਾਰ.ਅਦਲਾ_ਬਦਲੀ_ਅੰਕੜੇ == (ਖਾਲੀ *)&z_spinlock_abort_sentinel) {
		z_spin_validate_reset(ਅਦਲਾ_ਬਦਲੀ_ਤਾਲਾ != ਨਿਰਾ);
	} ਨਹੀਂਤਾਂ {
		__ASSERT(extra >= 0,
			 "swap_lock %p is not tracked in the spinlock hold count!", ਅਦਲਾ_ਬਦਲੀ_ਤਾਲਾ);

		__ASSERT(held == ਨਿਰਾ || held == ਅਦਲਾ_ਬਦਲੀ_ਤਾਲਾ,
			 "Context switching while holding spinlock %p!", held);

		/* Catches lock(A); lock(B); unlock(A); z_swap(): slot cleared when A
		 * is released but count stays at 1 because B is still held.
		 */
		__ASSERT(extra == 0,
			 "Context switching while holding %d extra spinlock(s)!", extra);
	}

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ CONFIG_ARM64
	/* Dummy threads start with IRQs masked and dead threads are being
	 * torn down; in both cases the IRQ state self-heals on the next
	 * switch-in, so they are exempt.  Disabled on ARM64 where FP/SIMD
	 * usage in exception context may leave IRQs masked without holding
	 * a lock (see #94285).
	 */
	__ASSERT(arch_irq_unlocked(ਕੁੰਜੀ) ||
		 _current->ਅਧਾਰ.thread_state & (_THREAD_DUMMY | _THREAD_DEAD),
		 "Context switching with irq_lock held!");
#ਜੇ_ਅੰਤ
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_KERNEL_COHERENCE
bool z_spin_ਤਾਲਾ_mem_coherent(ਬਣਤਰ k_spinlock *l)
{
	ਵਾਪਸ sys_cache_is_mem_coherent((ਖਾਲੀ *)l);
}
EXPORT_ਚਿੰਨ੍ਹ(z_spin_ਤਾਲਾ_mem_coherent);
#ਜੇ_ਅੰਤ /* CONFIG_KERNEL_COHERENCE */
