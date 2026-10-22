/*
 * Copyright (c) 2018 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#அந்தர்பாவ <zephyr/kernel.h>

#அந்தர்பாவ <zephyr/timing/timing.h>
#அந்தர்பாவ <ksched.h>
#அந்தர்பாவ <zephyr/spinlock.h>
#அந்தர்பாவ <zephyr/sys/check.h>

/* Need one of these for this to work */
#எனில் !defined(CONFIG_USE_SWITCH) && !defined(CONFIG_INSTRUMENT_THREAD_SWITCHING)
#error "No data backend configured for CONFIG_SCHED_THREAD_USAGE"
#endif /* !CONFIG_USE_SWITCH && !CONFIG_INSTRUMENT_THREAD_SWITCHING */

நிலையான கட்டமைப்பு k_spinlock usage_lock;

நிலையான uint32_t usage_now(வெறுமை)
{
	uint32_t now;

#ifdef CONFIG_THREAD_RUNTIME_STATS_USE_TIMING_FUNCTIONS
	now = (uint32_t)timing_counter_get();
#இல்லையெனில்
	now = k_cycle_get_32();
#endif /* CONFIG_THREAD_RUNTIME_STATS_USE_TIMING_FUNCTIONS */

	/* Edge case: we use a zero as a null ("stop() already called") */
	திரும்பு (now == 0) ? 1 : now;
}

#ifdef CONFIG_SCHED_THREAD_USAGE_ALL
நிலையான வெறுமை sched_cpu_update_usage(கட்டமைப்பு _cpu *cpu, uint32_t cycles)
{
	எனில் (!cpu->usage->track_usage) {
		திரும்பு;
	}

	எனில் (cpu->current != cpu->idle_thread) {
		cpu->usage->total += cycles;

#ifdef CONFIG_SCHED_THREAD_USAGE_ANALYSIS
		cpu->usage->current += cycles;

		எனில் (cpu->usage->longest < cpu->usage->current) {
			cpu->usage->longest = cpu->usage->current;
		}
	} இல்லையெனில் {
		cpu->usage->current = 0;
		cpu->usage->இலக்கம்_windows++;
#endif /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */
	}
}
#இல்லையெனில்
#define sched_cpu_update_usage(cpu, cycles)   செய் { } போது (0)
#endif /* CONFIG_SCHED_THREAD_USAGE_ALL */

நிலையான வெறுமை sched_thread_update_usage(கட்டமைப்பு k_thread *thread, uint32_t cycles)
{
	thread->base.usage.total += cycles;

#ifdef CONFIG_SCHED_THREAD_USAGE_ANALYSIS
	thread->base.usage.current += cycles;

	எனில் (thread->base.usage.longest < thread->base.usage.current) {
		thread->base.usage.longest = thread->base.usage.current;
	}
#endif /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */
}

வெறுமை z_sched_usage_start(கட்டமைப்பு k_thread *thread)
{
#ifdef CONFIG_SCHED_THREAD_USAGE_ANALYSIS
	k_spinlock_சாவி_t  சாவி;

	சாவி = k_spin_lock(&usage_lock);

	_current_cpu->usage0 = usage_now();   /* Always update */

	எனில் (thread->base.usage.track_usage) {
		thread->base.usage.இலக்கம்_windows++;
		thread->base.usage.current = 0;
	}

	k_spin_unlock(&usage_lock, சாவி);
#இல்லையெனில்
	/* One write through a volatile pointer doesn't require
	 * synchronization as long as _usage() treats it as volatile
	 * (we can't race with _stop() by design).
	 */

	_current_cpu->usage0 = usage_now();
#endif /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */
}

வெறுமை z_sched_usage_stop(வெறுமை)
{
	கட்டமைப்பு _cpu *cpu = _current_cpu;

	/* Only this CPU writes usage0, and callers keep local IRQs masked. */
	எனில் (cpu->usage0 == 0U) {
		திரும்பு;
	}

	k_spinlock_சாவி_t k = k_spin_lock(&usage_lock);

	uint32_t u0 = cpu->usage0;

	எனில் (u0 != 0) {
		uint32_t cycles = usage_now() - u0;

		எனில் (cpu->current->base.usage.track_usage) {
			sched_thread_update_usage(cpu->current, cycles);
		}

		sched_cpu_update_usage(cpu, cycles);
	}

	cpu->usage0 = 0;
	k_spin_unlock(&usage_lock, k);
}

#ifdef CONFIG_SCHED_THREAD_USAGE_ALL
வெறுமை z_sched_cpu_usage(uint8_t cpu_id, கட்டமைப்பு k_thread_runtime_stats *stats)
{
	k_spinlock_சாவி_t  சாவி;
	கட்டமைப்பு _cpu *cpu;
	uint32_t pending = 0U;
	bool pending_is_idle = false;

	சாவி = k_spin_lock(&usage_lock);
	cpu = &_kernel.cpus[cpu_id];

	எனில் (cpu == _current_cpu) {
		uint32_t  now = usage_now();
		uint32_t cycles = now - cpu->usage0;

		/*
		 * Getting stats for the current CPU. Update both its
		 * current thread stats and the CPU stats as the CPU's
		 * [usage0] field will also get updated. This keeps all
		 * that information up-to-date.
		 */

		எனில் (cpu->current->base.usage.track_usage) {
			sched_thread_update_usage(cpu->current, cycles);
		}

		sched_cpu_update_usage(cpu, cycles);

		cpu->usage0 = now;
	} இல்லையெனில் எனில் (cpu->usage->track_usage) {
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

	எனில் (cpu->usage->இலக்கம்_windows == 0) {
		stats->average_cycles = 0;
	} இல்லையெனில் {
		stats->average_cycles = stats->total_cycles /
					cpu->usage->இலக்கம்_windows;
	}
#endif /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */

	stats->idle_cycles =
		_kernel.cpus[cpu_id].idle_thread->base.usage.total +
		(pending_is_idle ? pending : 0U);

	stats->execution_cycles = stats->total_cycles + stats->idle_cycles;

	k_spin_unlock(&usage_lock, சாவி);
}
#endif /* CONFIG_SCHED_THREAD_USAGE_ALL */

வெறுமை z_sched_thread_usage(கட்டமைப்பு k_thread *thread,
			  கட்டமைப்பு k_thread_runtime_stats *stats)
{
	கட்டமைப்பு _cpu *cpu;
	k_spinlock_சாவி_t  சாவி;

	சாவி = k_spin_lock(&usage_lock);
	cpu = _current_cpu;


	எனில் (thread == cpu->current) {
		uint32_t now = usage_now();
		uint32_t cycles = now - cpu->usage0;

		/*
		 * Getting stats for the current thread. Update both the
		 * current thread stats and its CPU stats as the CPU's
		 * [usage0] field will also get updated. This keeps all
		 * that information up-to-date.
		 */

		எனில் (thread->base.usage.track_usage) {
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

	எனில் (thread->base.usage.இலக்கம்_windows == 0) {
		stats->average_cycles = 0;
	} இல்லையெனில் {
		stats->average_cycles = stats->total_cycles /
					thread->base.usage.இலக்கம்_windows;
	}
#endif /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */

#ifdef CONFIG_SCHED_THREAD_USAGE_ALL
	stats->idle_cycles = 0;
#endif /* CONFIG_SCHED_THREAD_USAGE_ALL */

	k_spin_unlock(&usage_lock, சாவி);
}

#ifdef CONFIG_SCHED_THREAD_USAGE_ANALYSIS
முழு k_thread_runtime_stats_enable(k_tid_t  thread)
{
	k_spinlock_சாவி_t  சாவி;

	CHECKIF(thread == வெற்று) {
		திரும்பு -EINVAL;
	}

	சாவி = k_spin_lock(&usage_lock);
	கட்டமைப்பு _cpu *cpu = _current_cpu;

	எனில் (!thread->base.usage.track_usage) {
		thread->base.usage.track_usage = true;
		thread->base.usage.இலக்கம்_windows++;
		thread->base.usage.current = 0;

		எனில் (thread == cpu->current) {
			uint32_t now = usage_now();

			sched_cpu_update_usage(cpu, now - cpu->usage0);

			cpu->usage0 = now;
		}
	}

	k_spin_unlock(&usage_lock, சாவி);

	திரும்பு 0;
}

முழு k_thread_runtime_stats_disable(k_tid_t  thread)
{
	k_spinlock_சாவி_t சாவி;

	CHECKIF(thread == வெற்று) {
		திரும்பு -EINVAL;
	}

	சாவி = k_spin_lock(&usage_lock);
	கட்டமைப்பு _cpu *cpu = _current_cpu;

	எனில் (thread->base.usage.track_usage) {
		thread->base.usage.track_usage = false;

		எனில் (thread == cpu->current) {
			uint32_t now = usage_now();
			uint32_t cycles = now - cpu->usage0;

			sched_thread_update_usage(thread, cycles);
			sched_cpu_update_usage(cpu, cycles);

			cpu->usage0 = now;
		}
	}

	k_spin_unlock(&usage_lock, சாவி);

	திரும்பு 0;
}

bool k_thread_runtime_stats_is_enabled(k_tid_t thread)
{
	திரும்பு thread->base.usage.track_usage;
}
#endif /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */

#ifdef CONFIG_SCHED_THREAD_USAGE_ALL
வெறுமை k_sys_runtime_stats_enable(வெறுமை)
{
	k_spinlock_சாவி_t  சாவி;

	சாவி = k_spin_lock(&usage_lock);

	எனில் (_current_cpu->usage->track_usage) {

		/*
		 * Usage tracking is already enabled on the current CPU
		 * and thus on all other CPUs (if applicable). There is
		 * nothing left to do.
		 */

		k_spin_unlock(&usage_lock, சாவி);
		திரும்பு;
	}

	/* Enable gathering of runtime stats on each CPU */

	unsigned முழு இலக்கம்_cpus = arch_இலக்கம்_cpus();

	க்கு (unsigned முழு i = 0; i < இலக்கம்_cpus; i++) {
		_kernel.cpus[i].usage->track_usage = true;
#ifdef CONFIG_SCHED_THREAD_USAGE_ANALYSIS
		_kernel.cpus[i].usage->இலக்கம்_windows++;
		_kernel.cpus[i].usage->current = 0;
#endif /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */
	}

	k_spin_unlock(&usage_lock, சாவி);
}

வெறுமை k_sys_runtime_stats_disable(வெறுமை)
{
	கட்டமைப்பு _cpu *cpu;
	k_spinlock_சாவி_t சாவி;

	சாவி = k_spin_lock(&usage_lock);

	எனில் (!_current_cpu->usage->track_usage) {

		/*
		 * Usage tracking is already disabled on the current CPU
		 * and thus on all other CPUs (if applicable). There is
		 * nothing left to do.
		 */

		k_spin_unlock(&usage_lock, சாவி);
		திரும்பு;
	}

	uint32_t now = usage_now();

	unsigned முழு இலக்கம்_cpus = arch_இலக்கம்_cpus();

	க்கு (unsigned முழு i = 0; i < இலக்கம்_cpus; i++) {
		cpu = &_kernel.cpus[i];
		எனில் (cpu->usage0 != 0) {
			sched_cpu_update_usage(cpu, now - cpu->usage0);
		}
		cpu->usage->track_usage = false;
	}

	k_spin_unlock(&usage_lock, சாவி);
}
#endif /* CONFIG_SCHED_THREAD_USAGE_ALL */

#ifdef CONFIG_OBJ_CORE_STATS_THREAD
முழு z_thread_stats_raw(கட்டமைப்பு k_obj_core *obj_core, வெறுமை *stats)
{
	k_spinlock_சாவி_t  சாவி;

	சாவி = k_spin_lock(&usage_lock);
	memcpy(stats, obj_core->stats, அளவு(கட்டமைப்பு k_cycle_stats));
	k_spin_unlock(&usage_lock, சாவி);

	திரும்பு 0;
}

முழு z_thread_stats_query(கட்டமைப்பு k_obj_core *obj_core, வெறுமை *stats)
{
	கட்டமைப்பு k_thread *thread;

	thread = CONTAINER_OF(obj_core, கட்டமைப்பு k_thread, obj_core);

	z_sched_thread_usage(thread, stats);

	திரும்பு 0;
}

முழு z_thread_stats_reset(கட்டமைப்பு k_obj_core *obj_core)
{
	k_spinlock_சாவி_t  சாவி;
	கட்டமைப்பு k_cycle_stats  *stats;
	கட்டமைப்பு k_thread *thread;

	thread = CONTAINER_OF(obj_core, கட்டமைப்பு k_thread, obj_core);
	சாவி = k_spin_lock(&usage_lock);
	stats = obj_core->stats;

	stats->total = 0ULL;
#ifdef CONFIG_SCHED_THREAD_USAGE_ANALYSIS
	stats->current = 0ULL;
	stats->longest = 0ULL;
	stats->இலக்கம்_windows = (thread->base.usage.track_usage) ?  1U : 0U;
#endif /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */

	எனில் (thread != _current_cpu->current) {

		/*
		 * If the thread is not running, there is nothing else to do.
		 * If the thread is running on another core, then it is not
		 * safe to do anything else but unlock and return (and pretend
		 * that its stats were reset at the start of its execution
		 * window.
		 */

		k_spin_unlock(&usage_lock, சாவி);

		திரும்பு 0;
	}

	/* Update the current CPU stats. */

	uint32_t now = usage_now();

	sched_cpu_update_usage(_current_cpu, now - _current_cpu->usage0);

	_current_cpu->usage0 = now;

	k_spin_unlock(&usage_lock, சாவி);

	திரும்பு 0;
}

முழு z_thread_stats_disable(கட்டமைப்பு k_obj_core *obj_core)
{
#ifdef CONFIG_SCHED_THREAD_USAGE_ANALYSIS
	கட்டமைப்பு k_thread *thread;

	thread = CONTAINER_OF(obj_core, கட்டமைப்பு k_thread, obj_core);

	திரும்பு k_thread_runtime_stats_disable(thread);
#இல்லையெனில்
	திரும்பு -ENOTSUP;
#endif /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */
}

முழு z_thread_stats_enable(கட்டமைப்பு k_obj_core *obj_core)
{
#ifdef CONFIG_SCHED_THREAD_USAGE_ANALYSIS
	கட்டமைப்பு k_thread *thread;

	thread = CONTAINER_OF(obj_core, கட்டமைப்பு k_thread, obj_core);

	திரும்பு k_thread_runtime_stats_enable(thread);
#இல்லையெனில்
	திரும்பு -ENOTSUP;
#endif /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */
}
#endif /* CONFIG_OBJ_CORE_STATS_THREAD */

#ifdef CONFIG_OBJ_CORE_STATS_SYSTEM
முழு z_cpu_stats_raw(கட்டமைப்பு k_obj_core *obj_core, வெறுமை *stats)
{
	k_spinlock_சாவி_t  சாவி;

	சாவி = k_spin_lock(&usage_lock);
	memcpy(stats, obj_core->stats, அளவு(கட்டமைப்பு k_cycle_stats));
	k_spin_unlock(&usage_lock, சாவி);

	திரும்பு 0;
}

முழு z_cpu_stats_query(கட்டமைப்பு k_obj_core *obj_core, வெறுமை *stats)
{
	கட்டமைப்பு _cpu  *cpu;

	cpu = CONTAINER_OF(obj_core, கட்டமைப்பு _cpu, obj_core);

	z_sched_cpu_usage(cpu->id, stats);

	திரும்பு 0;
}
#endif /* CONFIG_OBJ_CORE_STATS_SYSTEM */

#ifdef CONFIG_OBJ_CORE_STATS_SYSTEM
முழு z_kernel_stats_raw(கட்டமைப்பு k_obj_core *obj_core, வெறுமை *stats)
{
	k_spinlock_சாவி_t  சாவி;

	சாவி = k_spin_lock(&usage_lock);
	memcpy(stats, obj_core->stats,
	       CONFIG_MP_MAX_இலக்கம்_CPUS * அளவு(கட்டமைப்பு k_cycle_stats));
	k_spin_unlock(&usage_lock, சாவி);

	திரும்பு 0;
}

முழு z_kernel_stats_query(கட்டமைப்பு k_obj_core *obj_core, வெறுமை *stats)
{
	ARG_UNUSED(obj_core);

	திரும்பு k_thread_runtime_stats_all_get(stats);
}
#endif /* CONFIG_OBJ_CORE_STATS_SYSTEM */
