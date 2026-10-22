/*
 * Copyright (c) 2018,2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#అన్తర్భావ <kernel_internal.h>
#అన్తర్భావ <zephyr/spinlock.h>
#అన్తర్భావ <zephyr/llext/symbol.h>

#నిర్వచించు SPIN_CPU_ID_ముసుగు (పరిమాణం(శూన్య *) - 1)

/* Outermost spinlock held on each CPU; set-if-NULL on lock,
 * clear-if-matches on unlock.
 */
స్థిర నిర్మాణం k_spinlock *z_held_spinlock[CONFIG_MP_MAX_అంకె_CPUS];

/* Total spinlock hold count per CPU, incremented on every lock and
 * decremented on every unlock regardless of nesting depth.
 */
స్థిర పూర్ణాంకం z_held_spinlock_లెక్క[CONFIG_MP_MAX_అంకె_CPUS];

/* Sentinel: set in swap_data by the ztest harness before force-aborting a
 * thread that holds a lock after an expected panic/assert.
 */
స్థిరరాశి uint8_t z_spinlock_abort_sentinel;

bool z_spin_తాళం_valid(నిర్మాణం k_spinlock *l)
{
	uintptr_t thread_cpu = l->thread_cpu;

	ఒకవేళ (thread_cpu != 0U) {
		ఒకవేళ ((thread_cpu & SPIN_CPU_ID_ముసుగు) == _current_cpu->id) {
			తిరిగి false;
		}
	}
	తిరిగి true;
}
EXPORT_చిహ్నం(z_spin_తాళం_valid);

/* z_spin_unlock_valid() and z_spin_lock_set_owner() below use _raw_current
 * rather than _current: they run at points where z_current_thread_set() may
 * just have changed the current thread, which an architecture caching it in a
 * register (CONFIG_ARCH_HAS_CUSTOM_CURRENT_IMPL) would not report.  Both are
 * called from k_spin_lock() and k_spin_unlock() with the lock held, hence with
 * interrupts locked as _raw_current requires.
 */
bool z_spin_unlock_valid(నిర్మాణం k_spinlock *l)
{
	uint8_t cpu_id = _current_cpu->id;
	uintptr_t tcpu = l->thread_cpu;

	l->thread_cpu = 0;

	ఒకవేళ (z_held_spinlock[cpu_id] == l) {
		z_held_spinlock[cpu_id] = ఖాళీ;
	}

	ఒకవేళ (arch_is_లోపల_isr() &&
	    (_raw_current->పునాది.thread_state & _THREAD_DUMMY) != 0) {
		/* Edge case where an ISR aborted _current */
		z_held_spinlock_లెక్క[cpu_id]--;
		__ASSERT(z_held_spinlock_లెక్క[cpu_id] >= 0,
			 "spinlock unlock with no matching lock");
		తిరిగి true;
	}
	ఒకవేళ (tcpu != (cpu_id | (uintptr_t)_raw_current)) {
		తిరిగి false;
	}
	z_held_spinlock_లెక్క[cpu_id]--;
	__ASSERT(z_held_spinlock_లెక్క[cpu_id] >= 0,
		 "spinlock unlock with no matching lock");
	తిరిగి true;
}
EXPORT_చిహ్నం(z_spin_unlock_valid);

శూన్య z_spin_తాళం_సమితి_owner(నిర్మాణం k_spinlock *l)
{
	uint8_t cpu_id = _current_cpu->id;

	l->thread_cpu = cpu_id | (uintptr_t)_raw_current;

	ఒకవేళ (z_held_spinlock[cpu_id] == ఖాళీ) {
		z_held_spinlock[cpu_id] = l;
	}
	z_held_spinlock_లెక్క[cpu_id]++;
}
EXPORT_చిహ్నం(z_spin_తాళం_సమితి_owner);

/* Called from the switch paths after z_current_thread_set() to transfer
 * ownership of an inherited lock.  Does NOT update the tracking arrays.
 *
 * @a thread is passed in rather than read from _current: the caller has just
 * made it current, and _current is not required to report it until the switch
 * completes (see z_current_thread_set()).
 */
శూన్య z_spin_తాళం_transfer_owner(నిర్మాణం k_spinlock *l, నిర్మాణం k_thread *thread)
{
	l->thread_cpu = _current_cpu->id | (uintptr_t)thread;
}
EXPORT_చిహ్నం(z_spin_తాళం_transfer_owner);

/* Reset per-CPU spinlock tracking after abnormal thread termination.
 * All locks the dying thread held are abandoned (it will never resume to
 * release them), so only the locks still being released by the cleanup
 * path need to be counted.  lock_held=true if exactly one such lock
 * remains and will be released after this call.
 */
శూన్య z_spin_validate_reset(bool తాళం_held)
{
	uint8_t cpu_id = _current_cpu->id;

	z_held_spinlock[cpu_id] = ఖాళీ;
	z_held_spinlock_లెక్క[cpu_id] = తాళం_held ? 1 : 0;
}

/* Internal: caller must ensure IRQs are disabled. */
స్థిర అంతఃవరుస నిర్మాణం k_spinlock *z_spin_get_held_తాళం_locked(uint8_t cpu_id)
{
	తిరిగి z_held_spinlock[cpu_id];
}

#నిర్వచితమైతే CONFIG_TEST
నిర్మాణం k_spinlock *z_spin_get_held_తాళం(శూన్య)
{
	గుర్తులేని పూర్ణాంకం కీలకం = arch_irq_తాళం();
	నిర్మాణం k_spinlock *l = z_spin_get_held_తాళం_locked(_current_cpu->id);

	arch_irq_unlock(కీలకం);
	తిరిగి l;
}
#అయితే_ముగింపు /* CONFIG_TEST */

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
శూన్య z_assert_can_మార్పిడి(గుర్తులేని పూర్ణాంకం కీలకం, నిర్మాణం k_spinlock *మార్పిడి_తాళం)
{
	uint8_t cpu_id = _current_cpu->id;
	నిర్మాణం k_spinlock *held = z_spin_get_held_తాళం_locked(cpu_id);
	పూర్ణాంకం లెక్క = z_held_spinlock_లెక్క[cpu_id];
	పూర్ణాంకం extra = (మార్పిడి_తాళం != ఖాళీ) ? లెక్క - 1 : లెక్క;

	ARG_వాడని(held);
	ARG_వాడని(extra);

	/* ztest expected-fault: thread aborted with a lock held.*/
	ఒకవేళ ((_current->పునాది.thread_state & _THREAD_DEAD) &&
	    _current->పునాది.మార్పిడి_దత్తాంశం == (శూన్య *)&z_spinlock_abort_sentinel) {
		z_spin_validate_reset(మార్పిడి_తాళం != ఖాళీ);
	} లేకపోతే {
		__ASSERT(extra >= 0,
			 "swap_lock %p is not tracked in the spinlock hold count!", మార్పిడి_తాళం);

		__ASSERT(held == ఖాళీ || held == మార్పిడి_తాళం,
			 "Context switching while holding spinlock %p!", held);

		/* Catches lock(A); lock(B); unlock(A); z_swap(): slot cleared when A
		 * is released but count stays at 1 because B is still held.
		 */
		__ASSERT(extra == 0,
			 "Context switching while holding %d extra spinlock(s)!", extra);
	}

#నిర్వచితం_కాకపోతే CONFIG_ARM64
	/* Dummy threads start with IRQs masked and dead threads are being
	 * torn down; in both cases the IRQ state self-heals on the next
	 * switch-in, so they are exempt.  Disabled on ARM64 where FP/SIMD
	 * usage in exception context may leave IRQs masked without holding
	 * a lock (see #94285).
	 */
	__ASSERT(arch_irq_unlocked(కీలకం) ||
		 _current->పునాది.thread_state & (_THREAD_DUMMY | _THREAD_DEAD),
		 "Context switching with irq_lock held!");
#అయితే_ముగింపు
}

#నిర్వచితమైతే CONFIG_KERNEL_COHERENCE
bool z_spin_తాళం_mem_coherent(నిర్మాణం k_spinlock *l)
{
	తిరిగి sys_cache_is_mem_coherent((శూన్య *)l);
}
EXPORT_చిహ్నం(z_spin_తాళం_mem_coherent);
#అయితే_ముగింపు /* CONFIG_KERNEL_COHERENCE */
