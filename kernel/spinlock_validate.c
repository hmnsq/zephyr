/*
 * Copyright (c) 2018,2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ضمن <kernel_internal.h>
#ضمن <zephyr/spinlock.h>
#ضمن <zephyr/llext/symbol.h>

#تعریف_کرو SPIN_CPU_ID_نقاب (حجم(فراغ *) - 1)

/* Outermost spinlock held on each CPU; set-if-NULL on lock,
 * clear-if-matches on unlock.
 */
ساكن بنية k_spinlock *z_held_spinlock[CONFIG_MP_MAX_عدد_CPUS];

/* Total spinlock hold count per CPU, incremented on every lock and
 * decremented on every unlock regardless of nesting depth.
 */
ساكن صحيح z_held_spinlock_گنتی[CONFIG_MP_MAX_عدد_CPUS];

/* Sentinel: set in swap_data by the ztest harness before force-aborting a
 * thread that holds a lock after an expected panic/assert.
 */
ثابت uint8_t z_spinlock_abort_sentinel;

bool z_spin_جندرا_valid(بنية k_spinlock *l)
{
	uintptr_t thread_cpu = l->thread_cpu;

	إذا (thread_cpu != 0U) {
		إذا ((thread_cpu & SPIN_CPU_ID_نقاب) == _current_cpu->id) {
			أرجع false;
		}
	}
	أرجع true;
}
EXPORT_علامت(z_spin_جندرا_valid);

/* z_spin_unlock_valid() and z_spin_lock_set_owner() below use _raw_current
 * rather than _current: they run at points where z_current_thread_set() may
 * just have changed the current thread, which an architecture caching it in a
 * register (CONFIG_ARCH_HAS_CUSTOM_CURRENT_IMPL) would not report.  Both are
 * called from k_spin_lock() and k_spin_unlock() with the lock held, hence with
 * interrupts locked as _raw_current requires.
 */
bool z_spin_unlock_valid(بنية k_spinlock *l)
{
	uint8_t cpu_id = _current_cpu->id;
	uintptr_t tcpu = l->thread_cpu;

	l->thread_cpu = 0;

	إذا (z_held_spinlock[cpu_id] == l) {
		z_held_spinlock[cpu_id] = عدم;
	}

	إذا (arch_is_وچ_isr() &&
	    (_raw_current->ادھار.thread_state & _THREAD_DUMMY) != 0) {
		/* Edge case where an ISR aborted _current */
		z_held_spinlock_گنتی[cpu_id]--;
		__ASSERT(z_held_spinlock_گنتی[cpu_id] >= 0,
			 "spinlock unlock with no matching lock");
		أرجع true;
	}
	إذا (tcpu != (cpu_id | (uintptr_t)_raw_current)) {
		أرجع false;
	}
	z_held_spinlock_گنتی[cpu_id]--;
	__ASSERT(z_held_spinlock_گنتی[cpu_id] >= 0,
		 "spinlock unlock with no matching lock");
	أرجع true;
}
EXPORT_علامت(z_spin_unlock_valid);

فراغ z_spin_جندرا_سیٹ_owner(بنية k_spinlock *l)
{
	uint8_t cpu_id = _current_cpu->id;

	l->thread_cpu = cpu_id | (uintptr_t)_raw_current;

	إذا (z_held_spinlock[cpu_id] == عدم) {
		z_held_spinlock[cpu_id] = l;
	}
	z_held_spinlock_گنتی[cpu_id]++;
}
EXPORT_علامت(z_spin_جندرا_سیٹ_owner);

/* Called from the switch paths after z_current_thread_set() to transfer
 * ownership of an inherited lock.  Does NOT update the tracking arrays.
 *
 * @a thread is passed in rather than read from _current: the caller has just
 * made it current, and _current is not required to report it until the switch
 * completes (see z_current_thread_set()).
 */
فراغ z_spin_جندرا_transfer_owner(بنية k_spinlock *l, بنية k_thread *thread)
{
	l->thread_cpu = _current_cpu->id | (uintptr_t)thread;
}
EXPORT_علامت(z_spin_جندرا_transfer_owner);

/* Reset per-CPU spinlock tracking after abnormal thread termination.
 * All locks the dying thread held are abandoned (it will never resume to
 * release them), so only the locks still being released by the cleanup
 * path need to be counted.  lock_held=true if exactly one such lock
 * remains and will be released after this call.
 */
فراغ z_spin_validate_reset(bool جندرا_held)
{
	uint8_t cpu_id = _current_cpu->id;

	z_held_spinlock[cpu_id] = عدم;
	z_held_spinlock_گنتی[cpu_id] = جندرا_held ? 1 : 0;
}

/* Internal: caller must ensure IRQs are disabled. */
ساكن مضمن بنية k_spinlock *z_spin_get_held_جندرا_locked(uint8_t cpu_id)
{
	أرجع z_held_spinlock[cpu_id];
}

#جے_تعریف_شدہ CONFIG_TEST
بنية k_spinlock *z_spin_get_held_جندرا(فراغ)
{
	بنانشانی صحيح کلید = arch_irq_جندرا();
	بنية k_spinlock *l = z_spin_get_held_جندرا_locked(_current_cpu->id);

	arch_irq_unlock(کلید);
	أرجع l;
}
#جے_انت /* CONFIG_TEST */

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
فراغ z_assert_can_وٹاندرا(بنانشانی صحيح کلید, بنية k_spinlock *وٹاندرا_جندرا)
{
	uint8_t cpu_id = _current_cpu->id;
	بنية k_spinlock *held = z_spin_get_held_جندرا_locked(cpu_id);
	صحيح گنتی = z_held_spinlock_گنتی[cpu_id];
	صحيح extra = (وٹاندرا_جندرا != عدم) ? گنتی - 1 : گنتی;

	ARG_بن_ورتیا(held);
	ARG_بن_ورتیا(extra);

	/* ztest expected-fault: thread aborted with a lock held.*/
	إذا ((_current->ادھار.thread_state & _THREAD_DEAD) &&
	    _current->ادھار.وٹاندرا_انکڑے == (فراغ *)&z_spinlock_abort_sentinel) {
		z_spin_validate_reset(وٹاندرا_جندرا != عدم);
	} وإلا {
		__ASSERT(extra >= 0,
			 "swap_lock %p is not tracked in the spinlock hold count!", وٹاندرا_جندرا);

		__ASSERT(held == عدم || held == وٹاندرا_جندرا,
			 "Context switching while holding spinlock %p!", held);

		/* Catches lock(A); lock(B); unlock(A); z_swap(): slot cleared when A
		 * is released but count stays at 1 because B is still held.
		 */
		__ASSERT(extra == 0,
			 "Context switching while holding %d extra spinlock(s)!", extra);
	}

#جے_بنا_تعریف CONFIG_ARM64
	/* Dummy threads start with IRQs masked and dead threads are being
	 * torn down; in both cases the IRQ state self-heals on the next
	 * switch-in, so they are exempt.  Disabled on ARM64 where FP/SIMD
	 * usage in exception context may leave IRQs masked without holding
	 * a lock (see #94285).
	 */
	__ASSERT(arch_irq_unlocked(کلید) ||
		 _current->ادھار.thread_state & (_THREAD_DUMMY | _THREAD_DEAD),
		 "Context switching with irq_lock held!");
#جے_انت
}

#جے_تعریف_شدہ CONFIG_KERNEL_COHERENCE
bool z_spin_جندرا_mem_coherent(بنية k_spinlock *l)
{
	أرجع sys_cache_is_mem_coherent((فراغ *)l);
}
EXPORT_علامت(z_spin_جندرا_mem_coherent);
#جے_انت /* CONFIG_KERNEL_COHERENCE */
