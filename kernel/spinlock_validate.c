/*
 * Copyright (c) 2018,2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#شامل <kernel_internal.h>
#شامل <zephyr/spinlock.h>
#شامل <zephyr/llext/symbol.h>

#تعریف_کرو SPIN_CPU_ID_نقاب (حجم(خالی *) - 1)

/* Outermost spinlock held on each CPU; set-if-NULL on lock,
 * clear-if-matches on unlock.
 */
ساکن ساخت k_spinlock *z_held_spinlock[CONFIG_MP_MAX_عدد_CPUS];

/* Total spinlock hold count per CPU, incremented on every lock and
 * decremented on every unlock regardless of nesting depth.
 */
ساکن صحیح z_held_spinlock_گنتی[CONFIG_MP_MAX_عدد_CPUS];

/* Sentinel: set in swap_data by the ztest harness before force-aborting a
 * thread that holds a lock after an expected panic/assert.
 */
ثابت uint8_t z_spinlock_abort_sentinel;

bool z_spin_تالا_valid(ساخت k_spinlock *l)
{
	uintptr_t thread_cpu = l->thread_cpu;

	اگر (thread_cpu != 0U) {
		اگر ((thread_cpu & SPIN_CPU_ID_نقاب) == _current_cpu->id) {
			واپس false;
		}
	}
	واپس true;
}
EXPORT_علامت(z_spin_تالا_valid);

/* z_spin_unlock_valid() and z_spin_lock_set_owner() below use _raw_current
 * rather than _current: they run at points where z_current_thread_set() may
 * just have changed the current thread, which an architecture caching it in a
 * register (CONFIG_ARCH_HAS_CUSTOM_CURRENT_IMPL) would not report.  Both are
 * called from k_spin_lock() and k_spin_unlock() with the lock held, hence with
 * interrupts locked as _raw_current requires.
 */
bool z_spin_unlock_valid(ساخت k_spinlock *l)
{
	uint8_t cpu_id = _current_cpu->id;
	uintptr_t tcpu = l->thread_cpu;

	l->thread_cpu = 0;

	اگر (z_held_spinlock[cpu_id] == l) {
		z_held_spinlock[cpu_id] = تہی;
	}

	اگر (arch_is_اندر_isr() &&
	    (_raw_current->بنیاد.thread_state & _THREAD_DUMMY) != 0) {
		/* Edge case where an ISR aborted _current */
		z_held_spinlock_گنتی[cpu_id]--;
		__ASSERT(z_held_spinlock_گنتی[cpu_id] >= 0,
			 "spinlock unlock with no matching lock");
		واپس true;
	}
	اگر (tcpu != (cpu_id | (uintptr_t)_raw_current)) {
		واپس false;
	}
	z_held_spinlock_گنتی[cpu_id]--;
	__ASSERT(z_held_spinlock_گنتی[cpu_id] >= 0,
		 "spinlock unlock with no matching lock");
	واپس true;
}
EXPORT_علامت(z_spin_unlock_valid);

خالی z_spin_تالا_سیٹ_owner(ساخت k_spinlock *l)
{
	uint8_t cpu_id = _current_cpu->id;

	l->thread_cpu = cpu_id | (uintptr_t)_raw_current;

	اگر (z_held_spinlock[cpu_id] == تہی) {
		z_held_spinlock[cpu_id] = l;
	}
	z_held_spinlock_گنتی[cpu_id]++;
}
EXPORT_علامت(z_spin_تالا_سیٹ_owner);

/* Called from the switch paths after z_current_thread_set() to transfer
 * ownership of an inherited lock.  Does NOT update the tracking arrays.
 *
 * @a thread is passed in rather than read from _current: the caller has just
 * made it current, and _current is not required to report it until the switch
 * completes (see z_current_thread_set()).
 */
خالی z_spin_تالا_transfer_owner(ساخت k_spinlock *l, ساخت k_thread *thread)
{
	l->thread_cpu = _current_cpu->id | (uintptr_t)thread;
}
EXPORT_علامت(z_spin_تالا_transfer_owner);

/* Reset per-CPU spinlock tracking after abnormal thread termination.
 * All locks the dying thread held are abandoned (it will never resume to
 * release them), so only the locks still being released by the cleanup
 * path need to be counted.  lock_held=true if exactly one such lock
 * remains and will be released after this call.
 */
خالی z_spin_validate_reset(bool تالا_held)
{
	uint8_t cpu_id = _current_cpu->id;

	z_held_spinlock[cpu_id] = تہی;
	z_held_spinlock_گنتی[cpu_id] = تالا_held ? 1 : 0;
}

/* Internal: caller must ensure IRQs are disabled. */
ساکن اندرونی ساخت k_spinlock *z_spin_get_held_تالا_locked(uint8_t cpu_id)
{
	واپس z_held_spinlock[cpu_id];
}

#اگر_تعریف_شدہ CONFIG_TEST
ساخت k_spinlock *z_spin_get_held_تالا(خالی)
{
	غیرعلامتی صحیح کلید = arch_irq_تالا();
	ساخت k_spinlock *l = z_spin_get_held_تالا_locked(_current_cpu->id);

	arch_irq_unlock(کلید);
	واپس l;
}
#اگر_ختم /* CONFIG_TEST */

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
خالی z_assert_can_تبادلہ(غیرعلامتی صحیح کلید, ساخت k_spinlock *تبادلہ_تالا)
{
	uint8_t cpu_id = _current_cpu->id;
	ساخت k_spinlock *held = z_spin_get_held_تالا_locked(cpu_id);
	صحیح گنتی = z_held_spinlock_گنتی[cpu_id];
	صحیح extra = (تبادلہ_تالا != تہی) ? گنتی - 1 : گنتی;

	ARG_غیر_مستعمل(held);
	ARG_غیر_مستعمل(extra);

	/* ztest expected-fault: thread aborted with a lock held.*/
	اگر ((_current->بنیاد.thread_state & _THREAD_DEAD) &&
	    _current->بنیاد.تبادلہ_ڈیٹا == (خالی *)&z_spinlock_abort_sentinel) {
		z_spin_validate_reset(تبادلہ_تالا != تہی);
	} ورنہ {
		__ASSERT(extra >= 0,
			 "swap_lock %p is not tracked in the spinlock hold count!", تبادلہ_تالا);

		__ASSERT(held == تہی || held == تبادلہ_تالا,
			 "Context switching while holding spinlock %p!", held);

		/* Catches lock(A); lock(B); unlock(A); z_swap(): slot cleared when A
		 * is released but count stays at 1 because B is still held.
		 */
		__ASSERT(extra == 0,
			 "Context switching while holding %d extra spinlock(s)!", extra);
	}

#اگر_غیر_تعریف_شدہ CONFIG_ARM64
	/* Dummy threads start with IRQs masked and dead threads are being
	 * torn down; in both cases the IRQ state self-heals on the next
	 * switch-in, so they are exempt.  Disabled on ARM64 where FP/SIMD
	 * usage in exception context may leave IRQs masked without holding
	 * a lock (see #94285).
	 */
	__ASSERT(arch_irq_unlocked(کلید) ||
		 _current->بنیاد.thread_state & (_THREAD_DUMMY | _THREAD_DEAD),
		 "Context switching with irq_lock held!");
#اگر_ختم
}

#اگر_تعریف_شدہ CONFIG_KERNEL_COHERENCE
bool z_spin_تالا_mem_coherent(ساخت k_spinlock *l)
{
	واپس sys_cache_is_mem_coherent((خالی *)l);
}
EXPORT_علامت(z_spin_تالا_mem_coherent);
#اگر_ختم /* CONFIG_KERNEL_COHERENCE */
