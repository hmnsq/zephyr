/**
 * Copyright (c) 2024 Intel Corporation
 * SPDX-License-Identifier: Apache-2.0
 */

#شامل <zephyr/kernel.h>
#شامل <zephyr/sys/math_extras.h>
#شامل <kswap.h>
#شامل <ksched.h>
#شامل <ipi.h>

#اگر defined(CONFIG_IPI_OPTIMIZE_IDLE) && defined(CONFIG_PM)
#شامل <zephyr/pm/pm.h>
#endif

#ifdef CONFIG_SCHED_IPI_SUPPORTED
ایستا ساختار k_spinlock ipi_lock;
#endif

#ifdef CONFIG_TRACE_SCHED_IPI
بیرونی تهی z_trace_sched_ipi(تهی);
#endif

#اگر defined(CONFIG_IPI_OPTIMIZE_IDLE)
ایستا bool sched_ipi_idle_cpu_active(uint32_t cpu)
{
#اگر defined(CONFIG_PM)
	بازگشت pm_state_next_get((uint8_t)cpu)->state == PM_STATE_ACTIVE;
#وگرنه
	ARG_UNUSED(cpu);
	بازگشت true;
#endif
}

ایستا درونخطی صحیح sched_ipi_thread_reservation(ثابت ساختار k_thread *thread)
{
	uint32_t reservations = _kernel.sched_ipi_reserved;

	وقتی (reservations != 0U) {
		unsigned صحیح cpu = (unsigned صحیح)u32_شمار_trailing_zeros(reservations);

		reservations &= ~BIT(cpu);
		اگر (_kernel.sched_ipi_target[cpu] == thread) {
			بازگشت (صحیح)cpu;
		}
	}

	بازگشت -1;
}

ایستا تهی sched_ipi_idle_reserve(uint32_t cpu, ساختار k_thread *thread)
{
	uint32_t bit = BIT(cpu);

	__ASSERT_NO_MSG((_kernel.sched_ipi_reserved & bit) == 0U);
	__ASSERT_NO_MSG(_kernel.sched_ipi_target[cpu] == پوچ);

	_kernel.sched_ipi_target[cpu] = thread;
	_kernel.sched_ipi_reserved |= bit;
}

ایستا تهی sched_ipi_idle_unreserve(uint32_t cpu)
{
	uint32_t bit = BIT(cpu);

	_kernel.sched_ipi_target[cpu] = پوچ;
	_kernel.sched_ipi_reserved &= ~bit;
}

تهی ipi_idle_thread_unreserve(ساختار k_thread *thread)
{
	صحیح cpu = sched_ipi_thread_reservation(thread);

	/* Keep pending_ipi intact: its CPU bit may cover another request.
	 * An already-dispatched scheduling IPI is harmless.
	 */
	اگر (cpu >= 0) {
		sched_ipi_idle_unreserve((uint32_t)cpu);
	}
}

bool ipi_idle_thread_rebind(ساختار k_thread *old_thread,
			    ساختار k_thread *new_thread)
{
	صحیح old_cpu;

	__ASSERT_NO_MSG(old_thread != new_thread);

	old_cpu = sched_ipi_thread_reservation(old_thread);
	اگر (old_cpu < 0) {
		بازگشت false;
	}

	uint32_t cpu = (uint32_t)old_cpu;
	bool eligible = sched_ipi_idle_cpu_active(cpu);

#اگر defined(CONFIG_SCHED_CPU_MASK)
	eligible = eligible && ((new_thread->base.cpu_mask & BIT(cpu)) != 0U);
#endif

	اگر (!eligible) {
		sched_ipi_idle_unreserve(cpu);
		بازگشت false;
	}

	/* Reuse the outstanding IPI while transferring its coverage. */
	_kernel.sched_ipi_target[cpu] = new_thread;
	بازگشت true;
}

ساختار k_thread *ipi_idle_reserved_take(تهی)
{
	uint32_t cpu = _current_cpu->id;
	ساختار k_thread *thread = _kernel.sched_ipi_target[cpu];

	__ASSERT_NO_MSG((thread != پوچ) ==
			((_kernel.sched_ipi_reserved & BIT(cpu)) != 0U));

	اگر (thread != پوچ) {
		sched_ipi_idle_unreserve(cpu);
	}

	بازگشت thread;
}
#endif

تهی flag_ipi(uint32_t ipi_mask)
{
#اگر defined(CONFIG_SCHED_IPI_SUPPORTED)
	اگر (arch_رقم_cpus() > 1) {
		atomic_or(&_kernel.pending_ipi, (atomic_val_t)ipi_mask);
	}
#endif /* CONFIG_SCHED_IPI_SUPPORTED */
}

/* Create a bitmask of CPUs that need an IPI. Note: sched_spinlock is held. */
atomic_val_t ipi_mask_create(ساختار k_thread *thread)
{
	اگر (!IS_ENABLED(CONFIG_IPI_OPTIMIZE)) {
		بازگشت (CONFIG_MP_MAX_رقم_CPUS > 1) ? IPI_ALL_CPUS_MASK : 0;
	}

	uint32_t  ipi_mask = 0;
	uint32_t  رقم_cpus = (uint32_t)arch_رقم_cpus();
	uint32_t  id = _current_cpu->id;
	ساختار k_thread *cpu_thread;

#اگر defined(CONFIG_IPI_OPTIMIZE_IDLE)
	/* An outstanding idle CPU IPI already covers this thread. */
	اگر (sched_ipi_thread_reservation(thread) >= 0) {
		بازگشت 0;
	}

	/* Fast path: the thread's previous CPU is idle and not already reserved.
	 */
	uint8_t last = thread->base.cpu;
	ساختار k_thread *last_thread = _kernel.cpus[last].current;

	اگر (last != id &&
	    last_thread != پوچ &&
	    z_is_idle_thread_object(last_thread) &&
	    sched_ipi_idle_cpu_active(last) &&
#اگر defined(CONFIG_SCHED_CPU_MASK)
	    (thread->base.cpu_mask & BIT(last)) != 0 &&
#endif
	    (_kernel.sched_ipi_reserved & BIT(last)) == 0U) {
		sched_ipi_idle_reserve(last, thread);
		بازگشت IPI_CPU_MASK(last);
	}

#endif

	برای (uint32_t i = 0; i < رقم_cpus; i++) {
		اگر (id == i) {
			ادامه;
		}

		/*
		 * An IPI absolutely does not need to be sent if ...
		 * 1. the CPU is not active, or
		 * 2. <thread> can not execute on the target CPU
		 * ... and might not need to be sent if ...
		 * 3. the target CPU's active thread is not preemptible, or
		 * 4. the target CPU's active thread has a higher priority
		 *    (Items 3 & 4 may be overridden by a metaIRQ thread)
		 */

#اگر defined(CONFIG_SCHED_CPU_MASK)
		اگر ((thread->base.cpu_mask & BIT(i)) == 0) {
			ادامه;
		}
#endif

		cpu_thread = _kernel.cpus[i].current;

		اگر (cpu_thread == پوچ) {
			ادامه;
		}

#اگر defined(CONFIG_IPI_OPTIMIZE_IDLE)
		اگر ((_kernel.sched_ipi_reserved & BIT(i)) != 0U) {
			ادامه;
		}
		اگر (i != (uint32_t)thread->base.cpu &&
		    z_is_idle_thread_object(cpu_thread) &&
		    sched_ipi_idle_cpu_active(i)) {
			sched_ipi_idle_reserve(i, thread);
			بازگشت IPI_CPU_MASK(i);
		}
#endif

		اگر ((z_sched_prio_cmp(cpu_thread, thread) < 0 &&
		     thread_is_preemptible(cpu_thread)) ||
		    thread_is_metairq(thread)) {
			ipi_mask |= BIT(i);
		}
	}

	بازگشت (atomic_val_t)ipi_mask;
}

تهی signal_pending_ipi(تهی)
{
	/* Drain only the bits for other CPUs. Clearing our own bit here
	 * would silently drop an IPI destined for us, because
	 * arch_sched_directed_ipi() skips the calling CPU. Our own bit
	 * is left set so the next interrupt entry on this CPU will see
	 * it and run the scheduler.
	 *
	 * When rescheduling, callers must ensure that signal_pending_ipi()
	 * is invoked while the scheduler lock is still held. Holding the
	 * lock ensures the scheduling decision and IPI dispatch are atomic:
	 * either a concurrent flag_ipi() lands before the lock is acquired
	 * (and the CPU sees the new thread), or it lands after the lock is
	 * released (and the other CPU dispatches the IPI).
	 */

#اگر defined(CONFIG_SCHED_IPI_SUPPORTED)
	اگر (arch_رقم_cpus() > 1) {
		uint32_t self_bit = BIT(_current_cpu->id);
		uint32_t cpu_bitmap;

		cpu_bitmap = (uint32_t)atomic_and(&_kernel.pending_ipi,
						  (atomic_val_t)self_bit);
		cpu_bitmap &= ~self_bit;
		اگر (cpu_bitmap != 0) {
#ifdef CONFIG_ARCH_HAS_DIRECTED_IPIS
			arch_sched_directed_ipi(cpu_bitmap);
#وگرنه
			arch_sched_broadcast_ipi();
#endif
		}
	}
#endif /* CONFIG_SCHED_IPI_SUPPORTED */
}

#ifdef CONFIG_SCHED_IPI_SUPPORTED
ایستا ساختار k_ipi_work *first_ipi_work(sys_dlist_t *list, unsigned صحیح cpu_id)
{
	sys_dnode_t *work = sys_dlist_peek_head(list);

	بازگشت (work == پوچ) ? پوچ
			      : CONTAINER_OF(work, ساختار k_ipi_work, گره[cpu_id]);
}

صحیح k_ipi_work_add(ساختار k_ipi_work *work, uint32_t cpu_bitmask,
		   k_ipi_func_t func)
{
	__ASSERT(work != پوچ, "");
	__ASSERT(func != پوچ, "");

	k_spinlock_کلید_t کلید = k_spin_lock(&ipi_lock);

	/* Verify the IPI work item is not currently in use */

	اگر (k_event_wait_all(&work->event, work->bitmask,
			     false, K_NO_WAIT) != work->bitmask) {
		k_spin_unlock(&ipi_lock, کلید);
		بازگشت -EBUSY;
	}

	/*
	 * Add the IPI work item to the list(s)--but not for the current
	 * CPU as the architecture may not support sending an IPI to itself.
	 */

	unsigned صحیح cpu_id = _current_cpu->id;

	cpu_bitmask &= (IPI_ALL_CPUS_MASK & ~BIT(cpu_id));

	k_event_clear(&work->event, IPI_ALL_CPUS_MASK);
	work->func = func;
	work->bitmask = cpu_bitmask;

	برای (unsigned صحیح id = 0; id < arch_رقم_cpus(); id++) {
		اگر ((cpu_bitmask & BIT(id)) != 0) {
			sys_dlist_append(&_kernel.cpus[id].ipi_workq, &work->گره[id]);
		}
	}

	flag_ipi(cpu_bitmask);

	k_spin_unlock(&ipi_lock, کلید);

	بازگشت 0;
}

صحیح k_ipi_work_wait(ساختار k_ipi_work *work, k_timeout_t timeout)
{
	uint32_t rv = k_event_wait_all(&work->event, work->bitmask,
				       false, timeout);

	بازگشت (rv == 0) ? -EAGAIN : 0;
}

تهی k_ipi_work_signal(تهی)
{
	signal_pending_ipi();
}

ایستا تهی ipi_work_process(sys_dlist_t *list, unsigned صحیح cpu_id)
{
	k_spinlock_کلید_t کلید = k_spin_lock(&ipi_lock);

	برای (ساختار k_ipi_work *work = first_ipi_work(list, cpu_id);
	     work != پوچ; work = first_ipi_work(list, cpu_id)) {
		sys_dlist_remove(&work->گره[cpu_id]);
		k_spin_unlock(&ipi_lock, کلید);

		work->func(work);

		کلید = k_spin_lock(&ipi_lock);
		k_event_post(&work->event, BIT(cpu_id));
	}

	k_spin_unlock(&ipi_lock, کلید);
}
#endif /* CONFIG_SCHED_IPI_SUPPORTED */

تهی z_sched_ipi(تهی)
{
	/* NOTE: When adding code to this, make sure this is called
	 * at appropriate location when !CONFIG_SCHED_IPI_SUPPORTED.
	 */
#ifdef CONFIG_TRACE_SCHED_IPI
	z_trace_sched_ipi();
#endif /* CONFIG_TRACE_SCHED_IPI */

#ifdef CONFIG_TIMESLICING
	z_time_slice();
#endif /* CONFIG_TIMESLICING */

#ifdef CONFIG_ARCH_IPI_LAZY_COPROCESSORS_SAVE
	arch_ipi_lazy_coprocessors_save();
#endif

#ifdef CONFIG_SCHED_IPI_SUPPORTED
	unsigned صحیح cpu_id = _current_cpu->id;

	ipi_work_process(&_kernel.cpus[cpu_id].ipi_workq, cpu_id);
#endif
}
