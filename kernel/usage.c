/*
 * Copyright (c) 2018 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#অন্তর্ভাৱ <zephyr/kernel.h>

#অন্তর্ভাৱ <zephyr/timing/timing.h>
#অন্তর্ভাৱ <ksched.h>
#অন্তর্ভাৱ <zephyr/spinlock.h>
#অন্তর্ভাৱ <zephyr/sys/check.h>

/* Need one of these for this to work */
#যদি !defined(CONFIG_USE_SWITCH) && !defined(CONFIG_INSTRUMENT_THREAD_SWITCHING)
#error "No data backend configured for CONFIG_SCHED_THREAD_USAGE"
#endif /* !CONFIG_USE_SWITCH && !CONFIG_INSTRUMENT_THREAD_SWITCHING */

স্থির সংরচনা k_spinlock usage_lock;

স্থির uint32_t usage_now(শূন্য)
{
	uint32_t now;

#ifdef CONFIG_THREAD_RUNTIME_STATS_USE_TIMING_FUNCTIONS
	now = (uint32_t)timing_counter_get();
#অন্যথা
	now = k_cycle_get_32();
#endif /* CONFIG_THREAD_RUNTIME_STATS_USE_TIMING_FUNCTIONS */

	/* Edge case: we use a zero as a null ("stop() already called") */
	ঘূৰাই (now == 0) ? 1 : now;
}

#ifdef CONFIG_SCHED_THREAD_USAGE_ALL
স্থির শূন্য sched_cpu_update_usage(সংরচনা _cpu *cpu, uint32_t cycles)
{
	যদি (!cpu->usage->track_usage) {
		ঘূৰাই;
	}

	যদি (cpu->current != cpu->idle_thread) {
		cpu->usage->total += cycles;

#ifdef CONFIG_SCHED_THREAD_USAGE_ANALYSIS
		cpu->usage->current += cycles;

		যদি (cpu->usage->longest < cpu->usage->current) {
			cpu->usage->longest = cpu->usage->current;
		}
	} অন্যথা {
		cpu->usage->current = 0;
		cpu->usage->অঙ্ক_windows++;
#endif /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */
	}
}
#অন্যথা
#define sched_cpu_update_usage(cpu, cycles)   কৰা { } যেতিয়ালৈ (0)
#endif /* CONFIG_SCHED_THREAD_USAGE_ALL */

স্থির শূন্য sched_thread_update_usage(সংরচনা k_thread *thread, uint32_t cycles)
{
	thread->base.usage.total += cycles;

#ifdef CONFIG_SCHED_THREAD_USAGE_ANALYSIS
	thread->base.usage.current += cycles;

	যদি (thread->base.usage.longest < thread->base.usage.current) {
		thread->base.usage.longest = thread->base.usage.current;
	}
#endif /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */
}

শূন্য z_sched_usage_start(সংরচনা k_thread *thread)
{
#ifdef CONFIG_SCHED_THREAD_USAGE_ANALYSIS
	k_spinlock_চাবি_t  চাবি;

	চাবি = k_spin_lock(&usage_lock);

	_current_cpu->usage0 = usage_now();   /* Always update */

	যদি (thread->base.usage.track_usage) {
		thread->base.usage.অঙ্ক_windows++;
		thread->base.usage.current = 0;
	}

	k_spin_unlock(&usage_lock, চাবি);
#অন্যথা
	/* One write through a volatile pointer doesn't require
	 * synchronization as long as _usage() treats it as volatile
	 * (we can't race with _stop() by design).
	 */

	_current_cpu->usage0 = usage_now();
#endif /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */
}

শূন্য z_sched_usage_stop(শূন্য)
{
	সংরচনা _cpu *cpu = _current_cpu;

	/* Only this CPU writes usage0, and callers keep local IRQs masked. */
	যদি (cpu->usage0 == 0U) {
		ঘূৰাই;
	}

	k_spinlock_চাবি_t k = k_spin_lock(&usage_lock);

	uint32_t u0 = cpu->usage0;

	যদি (u0 != 0) {
		uint32_t cycles = usage_now() - u0;

		যদি (cpu->current->base.usage.track_usage) {
			sched_thread_update_usage(cpu->current, cycles);
		}

		sched_cpu_update_usage(cpu, cycles);
	}

	cpu->usage0 = 0;
	k_spin_unlock(&usage_lock, k);
}

#ifdef CONFIG_SCHED_THREAD_USAGE_ALL
শূন্য z_sched_cpu_usage(uint8_t cpu_id, সংরচনা k_thread_runtime_stats *stats)
{
	k_spinlock_চাবি_t  চাবি;
	সংরচনা _cpu *cpu;
	uint32_t pending = 0U;
	bool pending_is_idle = false;

	চাবি = k_spin_lock(&usage_lock);
	cpu = &_kernel.cpus[cpu_id];

	যদি (cpu == _current_cpu) {
		uint32_t  now = usage_now();
		uint32_t cycles = now - cpu->usage0;

		/*
		 * Getting stats for the current CPU. Update both its
		 * current thread stats and the CPU stats as the CPU's
		 * [usage0] field will also get updated. This keeps all
		 * that information up-to-date.
		 */

		যদি (cpu->current->base.usage.track_usage) {
			sched_thread_update_usage(cpu->current, cycles);
		}

		sched_cpu_update_usage(cpu, cycles);

		cpu->usage0 = now;
	} অন্যথা যদি (cpu->usage->track_usage) {
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

	যদি (cpu->usage->অঙ্ক_windows == 0) {
		stats->average_cycles = 0;
	} অন্যথা {
		stats->average_cycles = stats->total_cycles /
					cpu->usage->অঙ্ক_windows;
	}
#endif /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */

	stats->idle_cycles =
		_kernel.cpus[cpu_id].idle_thread->base.usage.total +
		(pending_is_idle ? pending : 0U);

	stats->execution_cycles = stats->total_cycles + stats->idle_cycles;

	k_spin_unlock(&usage_lock, চাবি);
}
#endif /* CONFIG_SCHED_THREAD_USAGE_ALL */

শূন্য z_sched_thread_usage(সংরচনা k_thread *thread,
			  সংরচনা k_thread_runtime_stats *stats)
{
	সংরচনা _cpu *cpu;
	k_spinlock_চাবি_t  চাবি;

	চাবি = k_spin_lock(&usage_lock);
	cpu = _current_cpu;


	যদি (thread == cpu->current) {
		uint32_t now = usage_now();
		uint32_t cycles = now - cpu->usage0;

		/*
		 * Getting stats for the current thread. Update both the
		 * current thread stats and its CPU stats as the CPU's
		 * [usage0] field will also get updated. This keeps all
		 * that information up-to-date.
		 */

		যদি (thread->base.usage.track_usage) {
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

	যদি (thread->base.usage.অঙ্ক_windows == 0) {
		stats->average_cycles = 0;
	} অন্যথা {
		stats->average_cycles = stats->total_cycles /
					thread->base.usage.অঙ্ক_windows;
	}
#endif /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */

#ifdef CONFIG_SCHED_THREAD_USAGE_ALL
	stats->idle_cycles = 0;
#endif /* CONFIG_SCHED_THREAD_USAGE_ALL */

	k_spin_unlock(&usage_lock, চাবি);
}

#ifdef CONFIG_SCHED_THREAD_USAGE_ANALYSIS
পূর্ণাঙ্ক k_thread_runtime_stats_enable(k_tid_t  thread)
{
	k_spinlock_চাবি_t  চাবি;

	CHECKIF(thread == রিক্ত) {
		ঘূৰাই -EINVAL;
	}

	চাবি = k_spin_lock(&usage_lock);
	সংরচনা _cpu *cpu = _current_cpu;

	যদি (!thread->base.usage.track_usage) {
		thread->base.usage.track_usage = true;
		thread->base.usage.অঙ্ক_windows++;
		thread->base.usage.current = 0;

		যদি (thread == cpu->current) {
			uint32_t now = usage_now();

			sched_cpu_update_usage(cpu, now - cpu->usage0);

			cpu->usage0 = now;
		}
	}

	k_spin_unlock(&usage_lock, চাবি);

	ঘূৰাই 0;
}

পূর্ণাঙ্ক k_thread_runtime_stats_disable(k_tid_t  thread)
{
	k_spinlock_চাবি_t চাবি;

	CHECKIF(thread == রিক্ত) {
		ঘূৰাই -EINVAL;
	}

	চাবি = k_spin_lock(&usage_lock);
	সংরচনা _cpu *cpu = _current_cpu;

	যদি (thread->base.usage.track_usage) {
		thread->base.usage.track_usage = false;

		যদি (thread == cpu->current) {
			uint32_t now = usage_now();
			uint32_t cycles = now - cpu->usage0;

			sched_thread_update_usage(thread, cycles);
			sched_cpu_update_usage(cpu, cycles);

			cpu->usage0 = now;
		}
	}

	k_spin_unlock(&usage_lock, চাবি);

	ঘূৰাই 0;
}

bool k_thread_runtime_stats_is_enabled(k_tid_t thread)
{
	ঘূৰাই thread->base.usage.track_usage;
}
#endif /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */

#ifdef CONFIG_SCHED_THREAD_USAGE_ALL
শূন্য k_sys_runtime_stats_enable(শূন্য)
{
	k_spinlock_চাবি_t  চাবি;

	চাবি = k_spin_lock(&usage_lock);

	যদি (_current_cpu->usage->track_usage) {

		/*
		 * Usage tracking is already enabled on the current CPU
		 * and thus on all other CPUs (if applicable). There is
		 * nothing left to do.
		 */

		k_spin_unlock(&usage_lock, চাবি);
		ঘূৰাই;
	}

	/* Enable gathering of runtime stats on each CPU */

	unsigned পূর্ণাঙ্ক অঙ্ক_cpus = arch_অঙ্ক_cpus();

	বাবে (unsigned পূর্ণাঙ্ক i = 0; i < অঙ্ক_cpus; i++) {
		_kernel.cpus[i].usage->track_usage = true;
#ifdef CONFIG_SCHED_THREAD_USAGE_ANALYSIS
		_kernel.cpus[i].usage->অঙ্ক_windows++;
		_kernel.cpus[i].usage->current = 0;
#endif /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */
	}

	k_spin_unlock(&usage_lock, চাবি);
}

শূন্য k_sys_runtime_stats_disable(শূন্য)
{
	সংরচনা _cpu *cpu;
	k_spinlock_চাবি_t চাবি;

	চাবি = k_spin_lock(&usage_lock);

	যদি (!_current_cpu->usage->track_usage) {

		/*
		 * Usage tracking is already disabled on the current CPU
		 * and thus on all other CPUs (if applicable). There is
		 * nothing left to do.
		 */

		k_spin_unlock(&usage_lock, চাবি);
		ঘূৰাই;
	}

	uint32_t now = usage_now();

	unsigned পূর্ণাঙ্ক অঙ্ক_cpus = arch_অঙ্ক_cpus();

	বাবে (unsigned পূর্ণাঙ্ক i = 0; i < অঙ্ক_cpus; i++) {
		cpu = &_kernel.cpus[i];
		যদি (cpu->usage0 != 0) {
			sched_cpu_update_usage(cpu, now - cpu->usage0);
		}
		cpu->usage->track_usage = false;
	}

	k_spin_unlock(&usage_lock, চাবি);
}
#endif /* CONFIG_SCHED_THREAD_USAGE_ALL */

#ifdef CONFIG_OBJ_CORE_STATS_THREAD
পূর্ণাঙ্ক z_thread_stats_raw(সংরচনা k_obj_core *obj_core, শূন্য *stats)
{
	k_spinlock_চাবি_t  চাবি;

	চাবি = k_spin_lock(&usage_lock);
	memcpy(stats, obj_core->stats, পরিমাণ(সংরচনা k_cycle_stats));
	k_spin_unlock(&usage_lock, চাবি);

	ঘূৰাই 0;
}

পূর্ণাঙ্ক z_thread_stats_query(সংরচনা k_obj_core *obj_core, শূন্য *stats)
{
	সংরচনা k_thread *thread;

	thread = CONTAINER_OF(obj_core, সংরচনা k_thread, obj_core);

	z_sched_thread_usage(thread, stats);

	ঘূৰাই 0;
}

পূর্ণাঙ্ক z_thread_stats_reset(সংরচনা k_obj_core *obj_core)
{
	k_spinlock_চাবি_t  চাবি;
	সংরচনা k_cycle_stats  *stats;
	সংরচনা k_thread *thread;

	thread = CONTAINER_OF(obj_core, সংরচনা k_thread, obj_core);
	চাবি = k_spin_lock(&usage_lock);
	stats = obj_core->stats;

	stats->total = 0ULL;
#ifdef CONFIG_SCHED_THREAD_USAGE_ANALYSIS
	stats->current = 0ULL;
	stats->longest = 0ULL;
	stats->অঙ্ক_windows = (thread->base.usage.track_usage) ?  1U : 0U;
#endif /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */

	যদি (thread != _current_cpu->current) {

		/*
		 * If the thread is not running, there is nothing else to do.
		 * If the thread is running on another core, then it is not
		 * safe to do anything else but unlock and return (and pretend
		 * that its stats were reset at the start of its execution
		 * window.
		 */

		k_spin_unlock(&usage_lock, চাবি);

		ঘূৰাই 0;
	}

	/* Update the current CPU stats. */

	uint32_t now = usage_now();

	sched_cpu_update_usage(_current_cpu, now - _current_cpu->usage0);

	_current_cpu->usage0 = now;

	k_spin_unlock(&usage_lock, চাবি);

	ঘূৰাই 0;
}

পূর্ণাঙ্ক z_thread_stats_disable(সংরচনা k_obj_core *obj_core)
{
#ifdef CONFIG_SCHED_THREAD_USAGE_ANALYSIS
	সংরচনা k_thread *thread;

	thread = CONTAINER_OF(obj_core, সংরচনা k_thread, obj_core);

	ঘূৰাই k_thread_runtime_stats_disable(thread);
#অন্যথা
	ঘূৰাই -ENOTSUP;
#endif /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */
}

পূর্ণাঙ্ক z_thread_stats_enable(সংরচনা k_obj_core *obj_core)
{
#ifdef CONFIG_SCHED_THREAD_USAGE_ANALYSIS
	সংরচনা k_thread *thread;

	thread = CONTAINER_OF(obj_core, সংরচনা k_thread, obj_core);

	ঘূৰাই k_thread_runtime_stats_enable(thread);
#অন্যথা
	ঘূৰাই -ENOTSUP;
#endif /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */
}
#endif /* CONFIG_OBJ_CORE_STATS_THREAD */

#ifdef CONFIG_OBJ_CORE_STATS_SYSTEM
পূর্ণাঙ্ক z_cpu_stats_raw(সংরচনা k_obj_core *obj_core, শূন্য *stats)
{
	k_spinlock_চাবি_t  চাবি;

	চাবি = k_spin_lock(&usage_lock);
	memcpy(stats, obj_core->stats, পরিমাণ(সংরচনা k_cycle_stats));
	k_spin_unlock(&usage_lock, চাবি);

	ঘূৰাই 0;
}

পূর্ণাঙ্ক z_cpu_stats_query(সংরচনা k_obj_core *obj_core, শূন্য *stats)
{
	সংরচনা _cpu  *cpu;

	cpu = CONTAINER_OF(obj_core, সংরচনা _cpu, obj_core);

	z_sched_cpu_usage(cpu->id, stats);

	ঘূৰাই 0;
}
#endif /* CONFIG_OBJ_CORE_STATS_SYSTEM */

#ifdef CONFIG_OBJ_CORE_STATS_SYSTEM
পূর্ণাঙ্ক z_kernel_stats_raw(সংরচনা k_obj_core *obj_core, শূন্য *stats)
{
	k_spinlock_চাবি_t  চাবি;

	চাবি = k_spin_lock(&usage_lock);
	memcpy(stats, obj_core->stats,
	       CONFIG_MP_MAX_অঙ্ক_CPUS * পরিমাণ(সংরচনা k_cycle_stats));
	k_spin_unlock(&usage_lock, চাবি);

	ঘূৰাই 0;
}

পূর্ণাঙ্ক z_kernel_stats_query(সংরচনা k_obj_core *obj_core, শূন্য *stats)
{
	ARG_UNUSED(obj_core);

	ঘূৰাই k_thread_runtime_stats_all_get(stats);
}
#endif /* CONFIG_OBJ_CORE_STATS_SYSTEM */
