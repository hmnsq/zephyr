/*
 * Copyright (c) 2018 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#അന്തര്ഭാവ <zephyr/kernel.h>

#അന്തര്ഭാവ <zephyr/timing/timing.h>
#അന്തര്ഭാവ <ksched.h>
#അന്തര്ഭാവ <zephyr/spinlock.h>
#അന്തര്ഭാവ <zephyr/sys/check.h>

/* Need one of these for this to work */
#എങ്കിൽ !defined(CONFIG_USE_SWITCH) && !defined(CONFIG_INSTRUMENT_THREAD_SWITCHING)
#error "No data backend configured for CONFIG_SCHED_THREAD_USAGE"
#endif /* !CONFIG_USE_SWITCH && !CONFIG_INSTRUMENT_THREAD_SWITCHING */

സ്ഥിതം ഘടന k_spinlock usage_lock;

സ്ഥിതം uint32_t usage_now(ശൂന്യം)
{
	uint32_t now;

#ifdef CONFIG_THREAD_RUNTIME_STATS_USE_TIMING_FUNCTIONS
	now = (uint32_t)timing_counter_get();
#അല്ലെങ്കിൽ
	now = k_cycle_get_32();
#endif /* CONFIG_THREAD_RUNTIME_STATS_USE_TIMING_FUNCTIONS */

	/* Edge case: we use a zero as a null ("stop() already called") */
	തിരികെ (now == 0) ? 1 : now;
}

#ifdef CONFIG_SCHED_THREAD_USAGE_ALL
സ്ഥിതം ശൂന്യം sched_cpu_update_usage(ഘടന _cpu *cpu, uint32_t cycles)
{
	എങ്കിൽ (!cpu->usage->track_usage) {
		തിരികെ;
	}

	എങ്കിൽ (cpu->current != cpu->idle_thread) {
		cpu->usage->total += cycles;

#ifdef CONFIG_SCHED_THREAD_USAGE_ANALYSIS
		cpu->usage->current += cycles;

		എങ്കിൽ (cpu->usage->longest < cpu->usage->current) {
			cpu->usage->longest = cpu->usage->current;
		}
	} അല്ലെങ്കിൽ {
		cpu->usage->current = 0;
		cpu->usage->അക്കം_windows++;
#endif /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */
	}
}
#അല്ലെങ്കിൽ
#define sched_cpu_update_usage(cpu, cycles)   ചെയ്യുക { } ആകുമ്പോൾ (0)
#endif /* CONFIG_SCHED_THREAD_USAGE_ALL */

സ്ഥിതം ശൂന്യം sched_thread_update_usage(ഘടന k_thread *thread, uint32_t cycles)
{
	thread->base.usage.total += cycles;

#ifdef CONFIG_SCHED_THREAD_USAGE_ANALYSIS
	thread->base.usage.current += cycles;

	എങ്കിൽ (thread->base.usage.longest < thread->base.usage.current) {
		thread->base.usage.longest = thread->base.usage.current;
	}
#endif /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */
}

ശൂന്യം z_sched_usage_start(ഘടന k_thread *thread)
{
#ifdef CONFIG_SCHED_THREAD_USAGE_ANALYSIS
	k_spinlock_താക്കോൽ_t  താക്കോൽ;

	താക്കോൽ = k_spin_lock(&usage_lock);

	_current_cpu->usage0 = usage_now();   /* Always update */

	എങ്കിൽ (thread->base.usage.track_usage) {
		thread->base.usage.അക്കം_windows++;
		thread->base.usage.current = 0;
	}

	k_spin_unlock(&usage_lock, താക്കോൽ);
#അല്ലെങ്കിൽ
	/* One write through a volatile pointer doesn't require
	 * synchronization as long as _usage() treats it as volatile
	 * (we can't race with _stop() by design).
	 */

	_current_cpu->usage0 = usage_now();
#endif /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */
}

ശൂന്യം z_sched_usage_stop(ശൂന്യം)
{
	ഘടന _cpu *cpu = _current_cpu;

	/* Only this CPU writes usage0, and callers keep local IRQs masked. */
	എങ്കിൽ (cpu->usage0 == 0U) {
		തിരികെ;
	}

	k_spinlock_താക്കോൽ_t k = k_spin_lock(&usage_lock);

	uint32_t u0 = cpu->usage0;

	എങ്കിൽ (u0 != 0) {
		uint32_t cycles = usage_now() - u0;

		എങ്കിൽ (cpu->current->base.usage.track_usage) {
			sched_thread_update_usage(cpu->current, cycles);
		}

		sched_cpu_update_usage(cpu, cycles);
	}

	cpu->usage0 = 0;
	k_spin_unlock(&usage_lock, k);
}

#ifdef CONFIG_SCHED_THREAD_USAGE_ALL
ശൂന്യം z_sched_cpu_usage(uint8_t cpu_id, ഘടന k_thread_runtime_stats *stats)
{
	k_spinlock_താക്കോൽ_t  താക്കോൽ;
	ഘടന _cpu *cpu;
	uint32_t pending = 0U;
	bool pending_is_idle = false;

	താക്കോൽ = k_spin_lock(&usage_lock);
	cpu = &_kernel.cpus[cpu_id];

	എങ്കിൽ (cpu == _current_cpu) {
		uint32_t  now = usage_now();
		uint32_t cycles = now - cpu->usage0;

		/*
		 * Getting stats for the current CPU. Update both its
		 * current thread stats and the CPU stats as the CPU's
		 * [usage0] field will also get updated. This keeps all
		 * that information up-to-date.
		 */

		എങ്കിൽ (cpu->current->base.usage.track_usage) {
			sched_thread_update_usage(cpu->current, cycles);
		}

		sched_cpu_update_usage(cpu, cycles);

		cpu->usage0 = now;
	} അല്ലെങ്കിൽ എങ്കിൽ (cpu->usage->track_usage) {
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

	എങ്കിൽ (cpu->usage->അക്കം_windows == 0) {
		stats->average_cycles = 0;
	} അല്ലെങ്കിൽ {
		stats->average_cycles = stats->total_cycles /
					cpu->usage->അക്കം_windows;
	}
#endif /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */

	stats->idle_cycles =
		_kernel.cpus[cpu_id].idle_thread->base.usage.total +
		(pending_is_idle ? pending : 0U);

	stats->execution_cycles = stats->total_cycles + stats->idle_cycles;

	k_spin_unlock(&usage_lock, താക്കോൽ);
}
#endif /* CONFIG_SCHED_THREAD_USAGE_ALL */

ശൂന്യം z_sched_thread_usage(ഘടന k_thread *thread,
			  ഘടന k_thread_runtime_stats *stats)
{
	ഘടന _cpu *cpu;
	k_spinlock_താക്കോൽ_t  താക്കോൽ;

	താക്കോൽ = k_spin_lock(&usage_lock);
	cpu = _current_cpu;


	എങ്കിൽ (thread == cpu->current) {
		uint32_t now = usage_now();
		uint32_t cycles = now - cpu->usage0;

		/*
		 * Getting stats for the current thread. Update both the
		 * current thread stats and its CPU stats as the CPU's
		 * [usage0] field will also get updated. This keeps all
		 * that information up-to-date.
		 */

		എങ്കിൽ (thread->base.usage.track_usage) {
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

	എങ്കിൽ (thread->base.usage.അക്കം_windows == 0) {
		stats->average_cycles = 0;
	} അല്ലെങ്കിൽ {
		stats->average_cycles = stats->total_cycles /
					thread->base.usage.അക്കം_windows;
	}
#endif /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */

#ifdef CONFIG_SCHED_THREAD_USAGE_ALL
	stats->idle_cycles = 0;
#endif /* CONFIG_SCHED_THREAD_USAGE_ALL */

	k_spin_unlock(&usage_lock, താക്കോൽ);
}

#ifdef CONFIG_SCHED_THREAD_USAGE_ANALYSIS
പൂർണ്ണസംഖ്യ k_thread_runtime_stats_enable(k_tid_t  thread)
{
	k_spinlock_താക്കോൽ_t  താക്കോൽ;

	CHECKIF(thread == ശൂന്യ) {
		തിരികെ -EINVAL;
	}

	താക്കോൽ = k_spin_lock(&usage_lock);
	ഘടന _cpu *cpu = _current_cpu;

	എങ്കിൽ (!thread->base.usage.track_usage) {
		thread->base.usage.track_usage = true;
		thread->base.usage.അക്കം_windows++;
		thread->base.usage.current = 0;

		എങ്കിൽ (thread == cpu->current) {
			uint32_t now = usage_now();

			sched_cpu_update_usage(cpu, now - cpu->usage0);

			cpu->usage0 = now;
		}
	}

	k_spin_unlock(&usage_lock, താക്കോൽ);

	തിരികെ 0;
}

പൂർണ്ണസംഖ്യ k_thread_runtime_stats_disable(k_tid_t  thread)
{
	k_spinlock_താക്കോൽ_t താക്കോൽ;

	CHECKIF(thread == ശൂന്യ) {
		തിരികെ -EINVAL;
	}

	താക്കോൽ = k_spin_lock(&usage_lock);
	ഘടന _cpu *cpu = _current_cpu;

	എങ്കിൽ (thread->base.usage.track_usage) {
		thread->base.usage.track_usage = false;

		എങ്കിൽ (thread == cpu->current) {
			uint32_t now = usage_now();
			uint32_t cycles = now - cpu->usage0;

			sched_thread_update_usage(thread, cycles);
			sched_cpu_update_usage(cpu, cycles);

			cpu->usage0 = now;
		}
	}

	k_spin_unlock(&usage_lock, താക്കോൽ);

	തിരികെ 0;
}

bool k_thread_runtime_stats_is_enabled(k_tid_t thread)
{
	തിരികെ thread->base.usage.track_usage;
}
#endif /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */

#ifdef CONFIG_SCHED_THREAD_USAGE_ALL
ശൂന്യം k_sys_runtime_stats_enable(ശൂന്യം)
{
	k_spinlock_താക്കോൽ_t  താക്കോൽ;

	താക്കോൽ = k_spin_lock(&usage_lock);

	എങ്കിൽ (_current_cpu->usage->track_usage) {

		/*
		 * Usage tracking is already enabled on the current CPU
		 * and thus on all other CPUs (if applicable). There is
		 * nothing left to do.
		 */

		k_spin_unlock(&usage_lock, താക്കോൽ);
		തിരികെ;
	}

	/* Enable gathering of runtime stats on each CPU */

	unsigned പൂർണ്ണസംഖ്യ അക്കം_cpus = arch_അക്കം_cpus();

	വേണ്ടി (unsigned പൂർണ്ണസംഖ്യ i = 0; i < അക്കം_cpus; i++) {
		_kernel.cpus[i].usage->track_usage = true;
#ifdef CONFIG_SCHED_THREAD_USAGE_ANALYSIS
		_kernel.cpus[i].usage->അക്കം_windows++;
		_kernel.cpus[i].usage->current = 0;
#endif /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */
	}

	k_spin_unlock(&usage_lock, താക്കോൽ);
}

ശൂന്യം k_sys_runtime_stats_disable(ശൂന്യം)
{
	ഘടന _cpu *cpu;
	k_spinlock_താക്കോൽ_t താക്കോൽ;

	താക്കോൽ = k_spin_lock(&usage_lock);

	എങ്കിൽ (!_current_cpu->usage->track_usage) {

		/*
		 * Usage tracking is already disabled on the current CPU
		 * and thus on all other CPUs (if applicable). There is
		 * nothing left to do.
		 */

		k_spin_unlock(&usage_lock, താക്കോൽ);
		തിരികെ;
	}

	uint32_t now = usage_now();

	unsigned പൂർണ്ണസംഖ്യ അക്കം_cpus = arch_അക്കം_cpus();

	വേണ്ടി (unsigned പൂർണ്ണസംഖ്യ i = 0; i < അക്കം_cpus; i++) {
		cpu = &_kernel.cpus[i];
		എങ്കിൽ (cpu->usage0 != 0) {
			sched_cpu_update_usage(cpu, now - cpu->usage0);
		}
		cpu->usage->track_usage = false;
	}

	k_spin_unlock(&usage_lock, താക്കോൽ);
}
#endif /* CONFIG_SCHED_THREAD_USAGE_ALL */

#ifdef CONFIG_OBJ_CORE_STATS_THREAD
പൂർണ്ണസംഖ്യ z_thread_stats_raw(ഘടന k_obj_core *obj_core, ശൂന്യം *stats)
{
	k_spinlock_താക്കോൽ_t  താക്കോൽ;

	താക്കോൽ = k_spin_lock(&usage_lock);
	memcpy(stats, obj_core->stats, വലുപ്പം(ഘടന k_cycle_stats));
	k_spin_unlock(&usage_lock, താക്കോൽ);

	തിരികെ 0;
}

പൂർണ്ണസംഖ്യ z_thread_stats_query(ഘടന k_obj_core *obj_core, ശൂന്യം *stats)
{
	ഘടന k_thread *thread;

	thread = CONTAINER_OF(obj_core, ഘടന k_thread, obj_core);

	z_sched_thread_usage(thread, stats);

	തിരികെ 0;
}

പൂർണ്ണസംഖ്യ z_thread_stats_reset(ഘടന k_obj_core *obj_core)
{
	k_spinlock_താക്കോൽ_t  താക്കോൽ;
	ഘടന k_cycle_stats  *stats;
	ഘടന k_thread *thread;

	thread = CONTAINER_OF(obj_core, ഘടന k_thread, obj_core);
	താക്കോൽ = k_spin_lock(&usage_lock);
	stats = obj_core->stats;

	stats->total = 0ULL;
#ifdef CONFIG_SCHED_THREAD_USAGE_ANALYSIS
	stats->current = 0ULL;
	stats->longest = 0ULL;
	stats->അക്കം_windows = (thread->base.usage.track_usage) ?  1U : 0U;
#endif /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */

	എങ്കിൽ (thread != _current_cpu->current) {

		/*
		 * If the thread is not running, there is nothing else to do.
		 * If the thread is running on another core, then it is not
		 * safe to do anything else but unlock and return (and pretend
		 * that its stats were reset at the start of its execution
		 * window.
		 */

		k_spin_unlock(&usage_lock, താക്കോൽ);

		തിരികെ 0;
	}

	/* Update the current CPU stats. */

	uint32_t now = usage_now();

	sched_cpu_update_usage(_current_cpu, now - _current_cpu->usage0);

	_current_cpu->usage0 = now;

	k_spin_unlock(&usage_lock, താക്കോൽ);

	തിരികെ 0;
}

പൂർണ്ണസംഖ്യ z_thread_stats_disable(ഘടന k_obj_core *obj_core)
{
#ifdef CONFIG_SCHED_THREAD_USAGE_ANALYSIS
	ഘടന k_thread *thread;

	thread = CONTAINER_OF(obj_core, ഘടന k_thread, obj_core);

	തിരികെ k_thread_runtime_stats_disable(thread);
#അല്ലെങ്കിൽ
	തിരികെ -ENOTSUP;
#endif /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */
}

പൂർണ്ണസംഖ്യ z_thread_stats_enable(ഘടന k_obj_core *obj_core)
{
#ifdef CONFIG_SCHED_THREAD_USAGE_ANALYSIS
	ഘടന k_thread *thread;

	thread = CONTAINER_OF(obj_core, ഘടന k_thread, obj_core);

	തിരികെ k_thread_runtime_stats_enable(thread);
#അല്ലെങ്കിൽ
	തിരികെ -ENOTSUP;
#endif /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */
}
#endif /* CONFIG_OBJ_CORE_STATS_THREAD */

#ifdef CONFIG_OBJ_CORE_STATS_SYSTEM
പൂർണ്ണസംഖ്യ z_cpu_stats_raw(ഘടന k_obj_core *obj_core, ശൂന്യം *stats)
{
	k_spinlock_താക്കോൽ_t  താക്കോൽ;

	താക്കോൽ = k_spin_lock(&usage_lock);
	memcpy(stats, obj_core->stats, വലുപ്പം(ഘടന k_cycle_stats));
	k_spin_unlock(&usage_lock, താക്കോൽ);

	തിരികെ 0;
}

പൂർണ്ണസംഖ്യ z_cpu_stats_query(ഘടന k_obj_core *obj_core, ശൂന്യം *stats)
{
	ഘടന _cpu  *cpu;

	cpu = CONTAINER_OF(obj_core, ഘടന _cpu, obj_core);

	z_sched_cpu_usage(cpu->id, stats);

	തിരികെ 0;
}
#endif /* CONFIG_OBJ_CORE_STATS_SYSTEM */

#ifdef CONFIG_OBJ_CORE_STATS_SYSTEM
പൂർണ്ണസംഖ്യ z_kernel_stats_raw(ഘടന k_obj_core *obj_core, ശൂന്യം *stats)
{
	k_spinlock_താക്കോൽ_t  താക്കോൽ;

	താക്കോൽ = k_spin_lock(&usage_lock);
	memcpy(stats, obj_core->stats,
	       CONFIG_MP_MAX_അക്കം_CPUS * വലുപ്പം(ഘടന k_cycle_stats));
	k_spin_unlock(&usage_lock, താക്കോൽ);

	തിരികെ 0;
}

പൂർണ്ണസംഖ്യ z_kernel_stats_query(ഘടന k_obj_core *obj_core, ശൂന്യം *stats)
{
	ARG_UNUSED(obj_core);

	തിരികെ k_thread_runtime_stats_all_get(stats);
}
#endif /* CONFIG_OBJ_CORE_STATS_SYSTEM */
