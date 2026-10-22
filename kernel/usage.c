/*
 * Copyright (c) 2018 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ಅನ್ತರ್ಭಾವ <zephyr/kernel.h>

#ಅನ್ತರ್ಭಾವ <zephyr/timing/timing.h>
#ಅನ್ತರ್ಭಾವ <ksched.h>
#ಅನ್ತರ್ಭಾವ <zephyr/spinlock.h>
#ಅನ್ತರ್ಭಾವ <zephyr/sys/check.h>

/* Need one of these for this to work */
#ಯದಿ !defined(CONFIG_USE_SWITCH) && !defined(CONFIG_INSTRUMENT_THREAD_SWITCHING)
#error "No data backend configured for CONFIG_SCHED_THREAD_USAGE"
#endif /* !CONFIG_USE_SWITCH && !CONFIG_INSTRUMENT_THREAD_SWITCHING */

ಸ್ಥಿರ ಸಂರಚನಾ k_spinlock usage_lock;

ಸ್ಥಿರ uint32_t usage_now(ಶೂನ್ಯ)
{
	uint32_t now;

#ifdef CONFIG_THREAD_RUNTIME_STATS_USE_TIMING_FUNCTIONS
	now = (uint32_t)timing_counter_get();
#ಅನ್ಯಥಾ
	now = k_cycle_get_32();
#endif /* CONFIG_THREAD_RUNTIME_STATS_USE_TIMING_FUNCTIONS */

	/* Edge case: we use a zero as a null ("stop() already called") */
	ಪ್ರತ್ಯಾಗಮ (now == 0) ? 1 : now;
}

#ifdef CONFIG_SCHED_THREAD_USAGE_ALL
ಸ್ಥಿರ ಶೂನ್ಯ sched_cpu_update_usage(ಸಂರಚನಾ _cpu *cpu, uint32_t cycles)
{
	ಯದಿ (!cpu->usage->track_usage) {
		ಪ್ರತ್ಯಾಗಮ;
	}

	ಯದಿ (cpu->current != cpu->idle_thread) {
		cpu->usage->total += cycles;

#ifdef CONFIG_SCHED_THREAD_USAGE_ANALYSIS
		cpu->usage->current += cycles;

		ಯದಿ (cpu->usage->longest < cpu->usage->current) {
			cpu->usage->longest = cpu->usage->current;
		}
	} ಅನ್ಯಥಾ {
		cpu->usage->current = 0;
		cpu->usage->ಅಂಕೆ_windows++;
#endif /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */
	}
}
#ಅನ್ಯಥಾ
#define sched_cpu_update_usage(cpu, cycles)   ಕುರು { } ಯಾವತ್ (0)
#endif /* CONFIG_SCHED_THREAD_USAGE_ALL */

ಸ್ಥಿರ ಶೂನ್ಯ sched_thread_update_usage(ಸಂರಚನಾ k_thread *thread, uint32_t cycles)
{
	thread->base.usage.total += cycles;

#ifdef CONFIG_SCHED_THREAD_USAGE_ANALYSIS
	thread->base.usage.current += cycles;

	ಯದಿ (thread->base.usage.longest < thread->base.usage.current) {
		thread->base.usage.longest = thread->base.usage.current;
	}
#endif /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */
}

ಶೂನ್ಯ z_sched_usage_start(ಸಂರಚನಾ k_thread *thread)
{
#ifdef CONFIG_SCHED_THREAD_USAGE_ANALYSIS
	k_spinlock_ಕೀಲಿ_t  ಕೀಲಿ;

	ಕೀಲಿ = k_spin_lock(&usage_lock);

	_current_cpu->usage0 = usage_now();   /* Always update */

	ಯದಿ (thread->base.usage.track_usage) {
		thread->base.usage.ಅಂಕೆ_windows++;
		thread->base.usage.current = 0;
	}

	k_spin_unlock(&usage_lock, ಕೀಲಿ);
#ಅನ್ಯಥಾ
	/* One write through a volatile pointer doesn't require
	 * synchronization as long as _usage() treats it as volatile
	 * (we can't race with _stop() by design).
	 */

	_current_cpu->usage0 = usage_now();
#endif /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */
}

ಶೂನ್ಯ z_sched_usage_stop(ಶೂನ್ಯ)
{
	ಸಂರಚನಾ _cpu *cpu = _current_cpu;

	/* Only this CPU writes usage0, and callers keep local IRQs masked. */
	ಯದಿ (cpu->usage0 == 0U) {
		ಪ್ರತ್ಯಾಗಮ;
	}

	k_spinlock_ಕೀಲಿ_t k = k_spin_lock(&usage_lock);

	uint32_t u0 = cpu->usage0;

	ಯದಿ (u0 != 0) {
		uint32_t cycles = usage_now() - u0;

		ಯದಿ (cpu->current->base.usage.track_usage) {
			sched_thread_update_usage(cpu->current, cycles);
		}

		sched_cpu_update_usage(cpu, cycles);
	}

	cpu->usage0 = 0;
	k_spin_unlock(&usage_lock, k);
}

#ifdef CONFIG_SCHED_THREAD_USAGE_ALL
ಶೂನ್ಯ z_sched_cpu_usage(uint8_t cpu_id, ಸಂರಚನಾ k_thread_runtime_stats *stats)
{
	k_spinlock_ಕೀಲಿ_t  ಕೀಲಿ;
	ಸಂರಚನಾ _cpu *cpu;
	uint32_t pending = 0U;
	bool pending_is_idle = false;

	ಕೀಲಿ = k_spin_lock(&usage_lock);
	cpu = &_kernel.cpus[cpu_id];

	ಯದಿ (cpu == _current_cpu) {
		uint32_t  now = usage_now();
		uint32_t cycles = now - cpu->usage0;

		/*
		 * Getting stats for the current CPU. Update both its
		 * current thread stats and the CPU stats as the CPU's
		 * [usage0] field will also get updated. This keeps all
		 * that information up-to-date.
		 */

		ಯದಿ (cpu->current->base.usage.track_usage) {
			sched_thread_update_usage(cpu->current, cycles);
		}

		sched_cpu_update_usage(cpu, cycles);

		cpu->usage0 = now;
	} ಅನ್ಯಥಾ ಯದಿ (cpu->usage->track_usage) {
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

	ಯದಿ (cpu->usage->ಅಂಕೆ_windows == 0) {
		stats->average_cycles = 0;
	} ಅನ್ಯಥಾ {
		stats->average_cycles = stats->total_cycles /
					cpu->usage->ಅಂಕೆ_windows;
	}
#endif /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */

	stats->idle_cycles =
		_kernel.cpus[cpu_id].idle_thread->base.usage.total +
		(pending_is_idle ? pending : 0U);

	stats->execution_cycles = stats->total_cycles + stats->idle_cycles;

	k_spin_unlock(&usage_lock, ಕೀಲಿ);
}
#endif /* CONFIG_SCHED_THREAD_USAGE_ALL */

ಶೂನ್ಯ z_sched_thread_usage(ಸಂರಚನಾ k_thread *thread,
			  ಸಂರಚನಾ k_thread_runtime_stats *stats)
{
	ಸಂರಚನಾ _cpu *cpu;
	k_spinlock_ಕೀಲಿ_t  ಕೀಲಿ;

	ಕೀಲಿ = k_spin_lock(&usage_lock);
	cpu = _current_cpu;


	ಯದಿ (thread == cpu->current) {
		uint32_t now = usage_now();
		uint32_t cycles = now - cpu->usage0;

		/*
		 * Getting stats for the current thread. Update both the
		 * current thread stats and its CPU stats as the CPU's
		 * [usage0] field will also get updated. This keeps all
		 * that information up-to-date.
		 */

		ಯದಿ (thread->base.usage.track_usage) {
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

	ಯದಿ (thread->base.usage.ಅಂಕೆ_windows == 0) {
		stats->average_cycles = 0;
	} ಅನ್ಯಥಾ {
		stats->average_cycles = stats->total_cycles /
					thread->base.usage.ಅಂಕೆ_windows;
	}
#endif /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */

#ifdef CONFIG_SCHED_THREAD_USAGE_ALL
	stats->idle_cycles = 0;
#endif /* CONFIG_SCHED_THREAD_USAGE_ALL */

	k_spin_unlock(&usage_lock, ಕೀಲಿ);
}

#ifdef CONFIG_SCHED_THREAD_USAGE_ANALYSIS
ಪೂರ್ಣಾಙ್ಕ k_thread_runtime_stats_enable(k_tid_t  thread)
{
	k_spinlock_ಕೀಲಿ_t  ಕೀಲಿ;

	CHECKIF(thread == ರಿಕ್ತ) {
		ಪ್ರತ್ಯಾಗಮ -EINVAL;
	}

	ಕೀಲಿ = k_spin_lock(&usage_lock);
	ಸಂರಚನಾ _cpu *cpu = _current_cpu;

	ಯದಿ (!thread->base.usage.track_usage) {
		thread->base.usage.track_usage = true;
		thread->base.usage.ಅಂಕೆ_windows++;
		thread->base.usage.current = 0;

		ಯದಿ (thread == cpu->current) {
			uint32_t now = usage_now();

			sched_cpu_update_usage(cpu, now - cpu->usage0);

			cpu->usage0 = now;
		}
	}

	k_spin_unlock(&usage_lock, ಕೀಲಿ);

	ಪ್ರತ್ಯಾಗಮ 0;
}

ಪೂರ್ಣಾಙ್ಕ k_thread_runtime_stats_disable(k_tid_t  thread)
{
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ;

	CHECKIF(thread == ರಿಕ್ತ) {
		ಪ್ರತ್ಯಾಗಮ -EINVAL;
	}

	ಕೀಲಿ = k_spin_lock(&usage_lock);
	ಸಂರಚನಾ _cpu *cpu = _current_cpu;

	ಯದಿ (thread->base.usage.track_usage) {
		thread->base.usage.track_usage = false;

		ಯದಿ (thread == cpu->current) {
			uint32_t now = usage_now();
			uint32_t cycles = now - cpu->usage0;

			sched_thread_update_usage(thread, cycles);
			sched_cpu_update_usage(cpu, cycles);

			cpu->usage0 = now;
		}
	}

	k_spin_unlock(&usage_lock, ಕೀಲಿ);

	ಪ್ರತ್ಯಾಗಮ 0;
}

bool k_thread_runtime_stats_is_enabled(k_tid_t thread)
{
	ಪ್ರತ್ಯಾಗಮ thread->base.usage.track_usage;
}
#endif /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */

#ifdef CONFIG_SCHED_THREAD_USAGE_ALL
ಶೂನ್ಯ k_sys_runtime_stats_enable(ಶೂನ್ಯ)
{
	k_spinlock_ಕೀಲಿ_t  ಕೀಲಿ;

	ಕೀಲಿ = k_spin_lock(&usage_lock);

	ಯದಿ (_current_cpu->usage->track_usage) {

		/*
		 * Usage tracking is already enabled on the current CPU
		 * and thus on all other CPUs (if applicable). There is
		 * nothing left to do.
		 */

		k_spin_unlock(&usage_lock, ಕೀಲಿ);
		ಪ್ರತ್ಯಾಗಮ;
	}

	/* Enable gathering of runtime stats on each CPU */

	unsigned ಪೂರ್ಣಾಙ್ಕ ಅಂಕೆ_cpus = arch_ಅಂಕೆ_cpus();

	ಪ್ರತಿ (unsigned ಪೂರ್ಣಾಙ್ಕ i = 0; i < ಅಂಕೆ_cpus; i++) {
		_kernel.cpus[i].usage->track_usage = true;
#ifdef CONFIG_SCHED_THREAD_USAGE_ANALYSIS
		_kernel.cpus[i].usage->ಅಂಕೆ_windows++;
		_kernel.cpus[i].usage->current = 0;
#endif /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */
	}

	k_spin_unlock(&usage_lock, ಕೀಲಿ);
}

ಶೂನ್ಯ k_sys_runtime_stats_disable(ಶೂನ್ಯ)
{
	ಸಂರಚನಾ _cpu *cpu;
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ;

	ಕೀಲಿ = k_spin_lock(&usage_lock);

	ಯದಿ (!_current_cpu->usage->track_usage) {

		/*
		 * Usage tracking is already disabled on the current CPU
		 * and thus on all other CPUs (if applicable). There is
		 * nothing left to do.
		 */

		k_spin_unlock(&usage_lock, ಕೀಲಿ);
		ಪ್ರತ್ಯಾಗಮ;
	}

	uint32_t now = usage_now();

	unsigned ಪೂರ್ಣಾಙ್ಕ ಅಂಕೆ_cpus = arch_ಅಂಕೆ_cpus();

	ಪ್ರತಿ (unsigned ಪೂರ್ಣಾಙ್ಕ i = 0; i < ಅಂಕೆ_cpus; i++) {
		cpu = &_kernel.cpus[i];
		ಯದಿ (cpu->usage0 != 0) {
			sched_cpu_update_usage(cpu, now - cpu->usage0);
		}
		cpu->usage->track_usage = false;
	}

	k_spin_unlock(&usage_lock, ಕೀಲಿ);
}
#endif /* CONFIG_SCHED_THREAD_USAGE_ALL */

#ifdef CONFIG_OBJ_CORE_STATS_THREAD
ಪೂರ್ಣಾಙ್ಕ z_thread_stats_raw(ಸಂರಚನಾ k_obj_core *obj_core, ಶೂನ್ಯ *stats)
{
	k_spinlock_ಕೀಲಿ_t  ಕೀಲಿ;

	ಕೀಲಿ = k_spin_lock(&usage_lock);
	memcpy(stats, obj_core->stats, ಪರಿಮಾಣ(ಸಂರಚನಾ k_cycle_stats));
	k_spin_unlock(&usage_lock, ಕೀಲಿ);

	ಪ್ರತ್ಯಾಗಮ 0;
}

ಪೂರ್ಣಾಙ್ಕ z_thread_stats_query(ಸಂರಚನಾ k_obj_core *obj_core, ಶೂನ್ಯ *stats)
{
	ಸಂರಚನಾ k_thread *thread;

	thread = CONTAINER_OF(obj_core, ಸಂರಚನಾ k_thread, obj_core);

	z_sched_thread_usage(thread, stats);

	ಪ್ರತ್ಯಾಗಮ 0;
}

ಪೂರ್ಣಾಙ್ಕ z_thread_stats_reset(ಸಂರಚನಾ k_obj_core *obj_core)
{
	k_spinlock_ಕೀಲಿ_t  ಕೀಲಿ;
	ಸಂರಚನಾ k_cycle_stats  *stats;
	ಸಂರಚನಾ k_thread *thread;

	thread = CONTAINER_OF(obj_core, ಸಂರಚನಾ k_thread, obj_core);
	ಕೀಲಿ = k_spin_lock(&usage_lock);
	stats = obj_core->stats;

	stats->total = 0ULL;
#ifdef CONFIG_SCHED_THREAD_USAGE_ANALYSIS
	stats->current = 0ULL;
	stats->longest = 0ULL;
	stats->ಅಂಕೆ_windows = (thread->base.usage.track_usage) ?  1U : 0U;
#endif /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */

	ಯದಿ (thread != _current_cpu->current) {

		/*
		 * If the thread is not running, there is nothing else to do.
		 * If the thread is running on another core, then it is not
		 * safe to do anything else but unlock and return (and pretend
		 * that its stats were reset at the start of its execution
		 * window.
		 */

		k_spin_unlock(&usage_lock, ಕೀಲಿ);

		ಪ್ರತ್ಯಾಗಮ 0;
	}

	/* Update the current CPU stats. */

	uint32_t now = usage_now();

	sched_cpu_update_usage(_current_cpu, now - _current_cpu->usage0);

	_current_cpu->usage0 = now;

	k_spin_unlock(&usage_lock, ಕೀಲಿ);

	ಪ್ರತ್ಯಾಗಮ 0;
}

ಪೂರ್ಣಾಙ್ಕ z_thread_stats_disable(ಸಂರಚನಾ k_obj_core *obj_core)
{
#ifdef CONFIG_SCHED_THREAD_USAGE_ANALYSIS
	ಸಂರಚನಾ k_thread *thread;

	thread = CONTAINER_OF(obj_core, ಸಂರಚನಾ k_thread, obj_core);

	ಪ್ರತ್ಯಾಗಮ k_thread_runtime_stats_disable(thread);
#ಅನ್ಯಥಾ
	ಪ್ರತ್ಯಾಗಮ -ENOTSUP;
#endif /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */
}

ಪೂರ್ಣಾಙ್ಕ z_thread_stats_enable(ಸಂರಚನಾ k_obj_core *obj_core)
{
#ifdef CONFIG_SCHED_THREAD_USAGE_ANALYSIS
	ಸಂರಚನಾ k_thread *thread;

	thread = CONTAINER_OF(obj_core, ಸಂರಚನಾ k_thread, obj_core);

	ಪ್ರತ್ಯಾಗಮ k_thread_runtime_stats_enable(thread);
#ಅನ್ಯಥಾ
	ಪ್ರತ್ಯಾಗಮ -ENOTSUP;
#endif /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */
}
#endif /* CONFIG_OBJ_CORE_STATS_THREAD */

#ifdef CONFIG_OBJ_CORE_STATS_SYSTEM
ಪೂರ್ಣಾಙ್ಕ z_cpu_stats_raw(ಸಂರಚನಾ k_obj_core *obj_core, ಶೂನ್ಯ *stats)
{
	k_spinlock_ಕೀಲಿ_t  ಕೀಲಿ;

	ಕೀಲಿ = k_spin_lock(&usage_lock);
	memcpy(stats, obj_core->stats, ಪರಿಮಾಣ(ಸಂರಚನಾ k_cycle_stats));
	k_spin_unlock(&usage_lock, ಕೀಲಿ);

	ಪ್ರತ್ಯಾಗಮ 0;
}

ಪೂರ್ಣಾಙ್ಕ z_cpu_stats_query(ಸಂರಚನಾ k_obj_core *obj_core, ಶೂನ್ಯ *stats)
{
	ಸಂರಚನಾ _cpu  *cpu;

	cpu = CONTAINER_OF(obj_core, ಸಂರಚನಾ _cpu, obj_core);

	z_sched_cpu_usage(cpu->id, stats);

	ಪ್ರತ್ಯಾಗಮ 0;
}
#endif /* CONFIG_OBJ_CORE_STATS_SYSTEM */

#ifdef CONFIG_OBJ_CORE_STATS_SYSTEM
ಪೂರ್ಣಾಙ್ಕ z_kernel_stats_raw(ಸಂರಚನಾ k_obj_core *obj_core, ಶೂನ್ಯ *stats)
{
	k_spinlock_ಕೀಲಿ_t  ಕೀಲಿ;

	ಕೀಲಿ = k_spin_lock(&usage_lock);
	memcpy(stats, obj_core->stats,
	       CONFIG_MP_MAX_ಅಂಕೆ_CPUS * ಪರಿಮಾಣ(ಸಂರಚನಾ k_cycle_stats));
	k_spin_unlock(&usage_lock, ಕೀಲಿ);

	ಪ್ರತ್ಯಾಗಮ 0;
}

ಪೂರ್ಣಾಙ್ಕ z_kernel_stats_query(ಸಂರಚನಾ k_obj_core *obj_core, ಶೂನ್ಯ *stats)
{
	ARG_UNUSED(obj_core);

	ಪ್ರತ್ಯಾಗಮ k_thread_runtime_stats_all_get(stats);
}
#endif /* CONFIG_OBJ_CORE_STATS_SYSTEM */
