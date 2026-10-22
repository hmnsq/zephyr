/*
 * Copyright (c) 2018 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#અન્તર્ભાવ <zephyr/kernel.h>

#અન્તર્ભાવ <zephyr/timing/timing.h>
#અન્તર્ભાવ <ksched.h>
#અન્તર્ભાવ <zephyr/spinlock.h>
#અન્તર્ભાવ <zephyr/sys/check.h>

/* Need one of these for this to work */
#જો !defined(CONFIG_USE_SWITCH) && !defined(CONFIG_INSTRUMENT_THREAD_SWITCHING)
#error "No data backend configured for CONFIG_SCHED_THREAD_USAGE"
#endif /* !CONFIG_USE_SWITCH && !CONFIG_INSTRUMENT_THREAD_SWITCHING */

સ્થિર સંરચના k_spinlock usage_lock;

સ્થિર uint32_t usage_now(શૂન્ય)
{
	uint32_t now;

#ifdef CONFIG_THREAD_RUNTIME_STATS_USE_TIMING_FUNCTIONS
	now = (uint32_t)timing_counter_get();
#નહીંતર
	now = k_cycle_get_32();
#endif /* CONFIG_THREAD_RUNTIME_STATS_USE_TIMING_FUNCTIONS */

	/* Edge case: we use a zero as a null ("stop() already called") */
	પાછા (now == 0) ? 1 : now;
}

#ifdef CONFIG_SCHED_THREAD_USAGE_ALL
સ્થિર શૂન્ય sched_cpu_update_usage(સંરચના _cpu *cpu, uint32_t cycles)
{
	જો (!cpu->usage->track_usage) {
		પાછા;
	}

	જો (cpu->current != cpu->idle_thread) {
		cpu->usage->total += cycles;

#ifdef CONFIG_SCHED_THREAD_USAGE_ANALYSIS
		cpu->usage->current += cycles;

		જો (cpu->usage->longest < cpu->usage->current) {
			cpu->usage->longest = cpu->usage->current;
		}
	} નહીંતર {
		cpu->usage->current = 0;
		cpu->usage->અંક_windows++;
#endif /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */
	}
}
#નહીંતર
#define sched_cpu_update_usage(cpu, cycles)   કરો { } જ્યારે (0)
#endif /* CONFIG_SCHED_THREAD_USAGE_ALL */

સ્થિર શૂન્ય sched_thread_update_usage(સંરચના k_thread *thread, uint32_t cycles)
{
	thread->base.usage.total += cycles;

#ifdef CONFIG_SCHED_THREAD_USAGE_ANALYSIS
	thread->base.usage.current += cycles;

	જો (thread->base.usage.longest < thread->base.usage.current) {
		thread->base.usage.longest = thread->base.usage.current;
	}
#endif /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */
}

શૂન્ય z_sched_usage_start(સંરચના k_thread *thread)
{
#ifdef CONFIG_SCHED_THREAD_USAGE_ANALYSIS
	k_spinlock_ચાવી_t  ચાવી;

	ચાવી = k_spin_lock(&usage_lock);

	_current_cpu->usage0 = usage_now();   /* Always update */

	જો (thread->base.usage.track_usage) {
		thread->base.usage.અંક_windows++;
		thread->base.usage.current = 0;
	}

	k_spin_unlock(&usage_lock, ચાવી);
#નહીંતર
	/* One write through a volatile pointer doesn't require
	 * synchronization as long as _usage() treats it as volatile
	 * (we can't race with _stop() by design).
	 */

	_current_cpu->usage0 = usage_now();
#endif /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */
}

શૂન્ય z_sched_usage_stop(શૂન્ય)
{
	સંરચના _cpu *cpu = _current_cpu;

	/* Only this CPU writes usage0, and callers keep local IRQs masked. */
	જો (cpu->usage0 == 0U) {
		પાછા;
	}

	k_spinlock_ચાવી_t k = k_spin_lock(&usage_lock);

	uint32_t u0 = cpu->usage0;

	જો (u0 != 0) {
		uint32_t cycles = usage_now() - u0;

		જો (cpu->current->base.usage.track_usage) {
			sched_thread_update_usage(cpu->current, cycles);
		}

		sched_cpu_update_usage(cpu, cycles);
	}

	cpu->usage0 = 0;
	k_spin_unlock(&usage_lock, k);
}

#ifdef CONFIG_SCHED_THREAD_USAGE_ALL
શૂન્ય z_sched_cpu_usage(uint8_t cpu_id, સંરચના k_thread_runtime_stats *stats)
{
	k_spinlock_ચાવી_t  ચાવી;
	સંરચના _cpu *cpu;
	uint32_t pending = 0U;
	bool pending_is_idle = false;

	ચાવી = k_spin_lock(&usage_lock);
	cpu = &_kernel.cpus[cpu_id];

	જો (cpu == _current_cpu) {
		uint32_t  now = usage_now();
		uint32_t cycles = now - cpu->usage0;

		/*
		 * Getting stats for the current CPU. Update both its
		 * current thread stats and the CPU stats as the CPU's
		 * [usage0] field will also get updated. This keeps all
		 * that information up-to-date.
		 */

		જો (cpu->current->base.usage.track_usage) {
			sched_thread_update_usage(cpu->current, cycles);
		}

		sched_cpu_update_usage(cpu, cycles);

		cpu->usage0 = now;
	} નહીંતર જો (cpu->usage->track_usage) {
		/*
		 * Getting stats for another CPU. Its counters only advance when
		 * it context switches or reports on itself, so the time it has
		 * spent in its current thread since then is not in them yet.
		 * Fold that in for the returned view only: mutating another
		 * CPU's accounting here would double count it once that CPU
		 * updates itself.
		 *
		 * Without this, a CPU that ran and then went idle reports all of
		 * the elapsed time as execution time, i.e. a 100% load.
		 */
		pending = usage_now() - cpu->usage0;
		pending_is_idle = (cpu->current == cpu->idle_thread);
	}

	stats->total_cycles     = cpu->usage->total +
				  (pending_is_idle ? 0U : pending);
#ifdef CONFIG_SCHED_THREAD_USAGE_ANALYSIS
	stats->current_cycles   = cpu->usage->current;
	stats->peak_cycles      = cpu->usage->longest;

	જો (cpu->usage->અંક_windows == 0) {
		stats->average_cycles = 0;
	} નહીંતર {
		stats->average_cycles = stats->total_cycles /
					cpu->usage->અંક_windows;
	}
#endif /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */

	stats->idle_cycles =
		_kernel.cpus[cpu_id].idle_thread->base.usage.total +
		(pending_is_idle ? pending : 0U);

	stats->execution_cycles = stats->total_cycles + stats->idle_cycles;

	k_spin_unlock(&usage_lock, ચાવી);
}
#endif /* CONFIG_SCHED_THREAD_USAGE_ALL */

શૂન્ય z_sched_thread_usage(સંરચના k_thread *thread,
			  સંરચના k_thread_runtime_stats *stats)
{
	સંરચના _cpu *cpu;
	k_spinlock_ચાવી_t  ચાવી;

	ચાવી = k_spin_lock(&usage_lock);
	cpu = _current_cpu;


	જો (thread == cpu->current) {
		uint32_t now = usage_now();
		uint32_t cycles = now - cpu->usage0;

		/*
		 * Getting stats for the current thread. Update both the
		 * current thread stats and its CPU stats as the CPU's
		 * [usage0] field will also get updated. This keeps all
		 * that information up-to-date.
		 */

		જો (thread->base.usage.track_usage) {
			sched_thread_update_usage(thread, cycles);
		}

		sched_cpu_update_usage(cpu, cycles);

		cpu->usage0 = now;
	}

	stats->execution_cycles = thread->base.usage.total;
	stats->total_cycles     = thread->base.usage.total;

	/* Copy-out the thread's usage stats */

#ifdef CONFIG_SCHED_THREAD_USAGE_ANALYSIS
	stats->current_cycles = thread->base.usage.current;
	stats->peak_cycles    = thread->base.usage.longest;

	જો (thread->base.usage.અંક_windows == 0) {
		stats->average_cycles = 0;
	} નહીંતર {
		stats->average_cycles = stats->total_cycles /
					thread->base.usage.અંક_windows;
	}
#endif /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */

#ifdef CONFIG_SCHED_THREAD_USAGE_ALL
	stats->idle_cycles = 0;
#endif /* CONFIG_SCHED_THREAD_USAGE_ALL */

	k_spin_unlock(&usage_lock, ચાવી);
}

#ifdef CONFIG_SCHED_THREAD_USAGE_ANALYSIS
પૂર્ણાંક k_thread_runtime_stats_enable(k_tid_t  thread)
{
	k_spinlock_ચાવી_t  ચાવી;

	CHECKIF(thread == ખાલી) {
		પાછા -EINVAL;
	}

	ચાવી = k_spin_lock(&usage_lock);
	સંરચના _cpu *cpu = _current_cpu;

	જો (!thread->base.usage.track_usage) {
		thread->base.usage.track_usage = true;
		thread->base.usage.અંક_windows++;
		thread->base.usage.current = 0;

		જો (thread == cpu->current) {
			uint32_t now = usage_now();

			sched_cpu_update_usage(cpu, now - cpu->usage0);

			cpu->usage0 = now;
		}
	}

	k_spin_unlock(&usage_lock, ચાવી);

	પાછા 0;
}

પૂર્ણાંક k_thread_runtime_stats_disable(k_tid_t  thread)
{
	k_spinlock_ચાવી_t ચાવી;

	CHECKIF(thread == ખાલી) {
		પાછા -EINVAL;
	}

	ચાવી = k_spin_lock(&usage_lock);
	સંરચના _cpu *cpu = _current_cpu;

	જો (thread->base.usage.track_usage) {
		thread->base.usage.track_usage = false;

		જો (thread == cpu->current) {
			uint32_t now = usage_now();
			uint32_t cycles = now - cpu->usage0;

			sched_thread_update_usage(thread, cycles);
			sched_cpu_update_usage(cpu, cycles);

			cpu->usage0 = now;
		}
	}

	k_spin_unlock(&usage_lock, ચાવી);

	પાછા 0;
}

bool k_thread_runtime_stats_is_enabled(k_tid_t thread)
{
	પાછા thread->base.usage.track_usage;
}
#endif /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */

#ifdef CONFIG_SCHED_THREAD_USAGE_ALL
શૂન્ય k_sys_runtime_stats_enable(શૂન્ય)
{
	k_spinlock_ચાવી_t  ચાવી;

	ચાવી = k_spin_lock(&usage_lock);

	જો (_current_cpu->usage->track_usage) {

		/*
		 * Usage tracking is already enabled on the current CPU
		 * and thus on all other CPUs (if applicable). There is
		 * nothing left to do.
		 */

		k_spin_unlock(&usage_lock, ચાવી);
		પાછા;
	}

	/* Enable gathering of runtime stats on each CPU */

	unsigned પૂર્ણાંક અંક_cpus = arch_અંક_cpus();

	માટે (unsigned પૂર્ણાંક i = 0; i < અંક_cpus; i++) {
		_kernel.cpus[i].usage->track_usage = true;
#ifdef CONFIG_SCHED_THREAD_USAGE_ANALYSIS
		_kernel.cpus[i].usage->અંક_windows++;
		_kernel.cpus[i].usage->current = 0;
#endif /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */
	}

	k_spin_unlock(&usage_lock, ચાવી);
}

શૂન્ય k_sys_runtime_stats_disable(શૂન્ય)
{
	સંરચના _cpu *cpu;
	k_spinlock_ચાવી_t ચાવી;

	ચાવી = k_spin_lock(&usage_lock);

	જો (!_current_cpu->usage->track_usage) {

		/*
		 * Usage tracking is already disabled on the current CPU
		 * and thus on all other CPUs (if applicable). There is
		 * nothing left to do.
		 */

		k_spin_unlock(&usage_lock, ચાવી);
		પાછા;
	}

	uint32_t now = usage_now();

	unsigned પૂર્ણાંક અંક_cpus = arch_અંક_cpus();

	માટે (unsigned પૂર્ણાંક i = 0; i < અંક_cpus; i++) {
		cpu = &_kernel.cpus[i];
		જો (cpu->usage0 != 0) {
			sched_cpu_update_usage(cpu, now - cpu->usage0);
		}
		cpu->usage->track_usage = false;
	}

	k_spin_unlock(&usage_lock, ચાવી);
}
#endif /* CONFIG_SCHED_THREAD_USAGE_ALL */

#ifdef CONFIG_OBJ_CORE_STATS_THREAD
પૂર્ણાંક z_thread_stats_raw(સંરચના k_obj_core *obj_core, શૂન્ય *stats)
{
	k_spinlock_ચાવી_t  ચાવી;

	ચાવી = k_spin_lock(&usage_lock);
	memcpy(stats, obj_core->stats, કદ(સંરચના k_cycle_stats));
	k_spin_unlock(&usage_lock, ચાવી);

	પાછા 0;
}

પૂર્ણાંક z_thread_stats_query(સંરચના k_obj_core *obj_core, શૂન્ય *stats)
{
	સંરચના k_thread *thread;

	thread = CONTAINER_OF(obj_core, સંરચના k_thread, obj_core);

	z_sched_thread_usage(thread, stats);

	પાછા 0;
}

પૂર્ણાંક z_thread_stats_reset(સંરચના k_obj_core *obj_core)
{
	k_spinlock_ચાવી_t  ચાવી;
	સંરચના k_cycle_stats  *stats;
	સંરચના k_thread *thread;

	thread = CONTAINER_OF(obj_core, સંરચના k_thread, obj_core);
	ચાવી = k_spin_lock(&usage_lock);
	stats = obj_core->stats;

	stats->total = 0ULL;
#ifdef CONFIG_SCHED_THREAD_USAGE_ANALYSIS
	stats->current = 0ULL;
	stats->longest = 0ULL;
	stats->અંક_windows = (thread->base.usage.track_usage) ?  1U : 0U;
#endif /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */

	જો (thread != _current_cpu->current) {

		/*
		 * If the thread is not running, there is nothing else to do.
		 * If the thread is running on another core, then it is not
		 * safe to do anything else but unlock and return (and pretend
		 * that its stats were reset at the start of its execution
		 * window.
		 */

		k_spin_unlock(&usage_lock, ચાવી);

		પાછા 0;
	}

	/* Update the current CPU stats. */

	uint32_t now = usage_now();

	sched_cpu_update_usage(_current_cpu, now - _current_cpu->usage0);

	_current_cpu->usage0 = now;

	k_spin_unlock(&usage_lock, ચાવી);

	પાછા 0;
}

પૂર્ણાંક z_thread_stats_disable(સંરચના k_obj_core *obj_core)
{
#ifdef CONFIG_SCHED_THREAD_USAGE_ANALYSIS
	સંરચના k_thread *thread;

	thread = CONTAINER_OF(obj_core, સંરચના k_thread, obj_core);

	પાછા k_thread_runtime_stats_disable(thread);
#નહીંતર
	પાછા -ENOTSUP;
#endif /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */
}

પૂર્ણાંક z_thread_stats_enable(સંરચના k_obj_core *obj_core)
{
#ifdef CONFIG_SCHED_THREAD_USAGE_ANALYSIS
	સંરચના k_thread *thread;

	thread = CONTAINER_OF(obj_core, સંરચના k_thread, obj_core);

	પાછા k_thread_runtime_stats_enable(thread);
#નહીંતર
	પાછા -ENOTSUP;
#endif /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */
}
#endif /* CONFIG_OBJ_CORE_STATS_THREAD */

#ifdef CONFIG_OBJ_CORE_STATS_SYSTEM
પૂર્ણાંક z_cpu_stats_raw(સંરચના k_obj_core *obj_core, શૂન્ય *stats)
{
	k_spinlock_ચાવી_t  ચાવી;

	ચાવી = k_spin_lock(&usage_lock);
	memcpy(stats, obj_core->stats, કદ(સંરચના k_cycle_stats));
	k_spin_unlock(&usage_lock, ચાવી);

	પાછા 0;
}

પૂર્ણાંક z_cpu_stats_query(સંરચના k_obj_core *obj_core, શૂન્ય *stats)
{
	સંરચના _cpu  *cpu;

	cpu = CONTAINER_OF(obj_core, સંરચના _cpu, obj_core);

	z_sched_cpu_usage(cpu->id, stats);

	પાછા 0;
}
#endif /* CONFIG_OBJ_CORE_STATS_SYSTEM */

#ifdef CONFIG_OBJ_CORE_STATS_SYSTEM
પૂર્ણાંક z_kernel_stats_raw(સંરચના k_obj_core *obj_core, શૂન્ય *stats)
{
	k_spinlock_ચાવી_t  ચાવી;

	ચાવી = k_spin_lock(&usage_lock);
	memcpy(stats, obj_core->stats,
	       CONFIG_MP_MAX_અંક_CPUS * કદ(સંરચના k_cycle_stats));
	k_spin_unlock(&usage_lock, ચાવી);

	પાછા 0;
}

પૂર્ણાંક z_kernel_stats_query(સંરચના k_obj_core *obj_core, શૂન્ય *stats)
{
	ARG_UNUSED(obj_core);

	પાછા k_thread_runtime_stats_all_get(stats);
}
#endif /* CONFIG_OBJ_CORE_STATS_SYSTEM */
