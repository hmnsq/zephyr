/*
 * Copyright (c) 2018 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#شامل <zephyr/kernel.h>

#شامل <zephyr/timing/timing.h>
#شامل <ksched.h>
#شامل <zephyr/spinlock.h>
#شامل <zephyr/sys/check.h>

/* Need one of these for this to work */
#اگر !defined(CONFIG_USE_SWITCH) && !defined(CONFIG_INSTRUMENT_THREAD_SWITCHING)
#error "No data backend configured for CONFIG_SCHED_THREAD_USAGE"
#endif /* !CONFIG_USE_SWITCH && !CONFIG_INSTRUMENT_THREAD_SWITCHING */

ایستا ساختار k_spinlock usage_lock;

ایستا uint32_t usage_now(تهی)
{
	uint32_t now;

#ifdef CONFIG_THREAD_RUNTIME_STATS_USE_TIMING_FUNCTIONS
	now = (uint32_t)timing_counter_get();
#وگرنه
	now = k_cycle_get_32();
#endif /* CONFIG_THREAD_RUNTIME_STATS_USE_TIMING_FUNCTIONS */

	/* Edge case: we use a zero as a null ("stop() already called") */
	بازگشت (now == 0) ? 1 : now;
}

#ifdef CONFIG_SCHED_THREAD_USAGE_ALL
ایستا تهی sched_cpu_update_usage(ساختار _cpu *cpu, uint32_t cycles)
{
	اگر (!cpu->usage->track_usage) {
		بازگشت;
	}

	اگر (cpu->current != cpu->idle_thread) {
		cpu->usage->total += cycles;

#ifdef CONFIG_SCHED_THREAD_USAGE_ANALYSIS
		cpu->usage->current += cycles;

		اگر (cpu->usage->longest < cpu->usage->current) {
			cpu->usage->longest = cpu->usage->current;
		}
	} وگرنه {
		cpu->usage->current = 0;
		cpu->usage->رقم_windows++;
#endif /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */
	}
}
#وگرنه
#define sched_cpu_update_usage(cpu, cycles)   انجامبده { } وقتی (0)
#endif /* CONFIG_SCHED_THREAD_USAGE_ALL */

ایستا تهی sched_thread_update_usage(ساختار k_thread *thread, uint32_t cycles)
{
	thread->base.usage.total += cycles;

#ifdef CONFIG_SCHED_THREAD_USAGE_ANALYSIS
	thread->base.usage.current += cycles;

	اگر (thread->base.usage.longest < thread->base.usage.current) {
		thread->base.usage.longest = thread->base.usage.current;
	}
#endif /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */
}

تهی z_sched_usage_start(ساختار k_thread *thread)
{
#ifdef CONFIG_SCHED_THREAD_USAGE_ANALYSIS
	k_spinlock_کلید_t  کلید;

	کلید = k_spin_lock(&usage_lock);

	_current_cpu->usage0 = usage_now();   /* Always update */

	اگر (thread->base.usage.track_usage) {
		thread->base.usage.رقم_windows++;
		thread->base.usage.current = 0;
	}

	k_spin_unlock(&usage_lock, کلید);
#وگرنه
	/* One write through a volatile pointer doesn't require
	 * synchronization as long as _usage() treats it as volatile
	 * (we can't race with _stop() by design).
	 */

	_current_cpu->usage0 = usage_now();
#endif /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */
}

تهی z_sched_usage_stop(تهی)
{
	ساختار _cpu *cpu = _current_cpu;

	/* Only this CPU writes usage0, and callers keep local IRQs masked. */
	اگر (cpu->usage0 == 0U) {
		بازگشت;
	}

	k_spinlock_کلید_t k = k_spin_lock(&usage_lock);

	uint32_t u0 = cpu->usage0;

	اگر (u0 != 0) {
		uint32_t cycles = usage_now() - u0;

		اگر (cpu->current->base.usage.track_usage) {
			sched_thread_update_usage(cpu->current, cycles);
		}

		sched_cpu_update_usage(cpu, cycles);
	}

	cpu->usage0 = 0;
	k_spin_unlock(&usage_lock, k);
}

#ifdef CONFIG_SCHED_THREAD_USAGE_ALL
تهی z_sched_cpu_usage(uint8_t cpu_id, ساختار k_thread_runtime_stats *stats)
{
	k_spinlock_کلید_t  کلید;
	ساختار _cpu *cpu;
	uint32_t pending = 0U;
	bool pending_is_idle = false;

	کلید = k_spin_lock(&usage_lock);
	cpu = &_kernel.cpus[cpu_id];

	اگر (cpu == _current_cpu) {
		uint32_t  now = usage_now();
		uint32_t cycles = now - cpu->usage0;

		/*
		 * Getting stats for the current CPU. Update both its
		 * current thread stats and the CPU stats as the CPU's
		 * [usage0] field will also get updated. This keeps all
		 * that information up-to-date.
		 */

		اگر (cpu->current->base.usage.track_usage) {
			sched_thread_update_usage(cpu->current, cycles);
		}

		sched_cpu_update_usage(cpu, cycles);

		cpu->usage0 = now;
	} وگرنه اگر (cpu->usage->track_usage) {
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

	اگر (cpu->usage->رقم_windows == 0) {
		stats->average_cycles = 0;
	} وگرنه {
		stats->average_cycles = stats->total_cycles /
					cpu->usage->رقم_windows;
	}
#endif /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */

	stats->idle_cycles =
		_kernel.cpus[cpu_id].idle_thread->base.usage.total +
		(pending_is_idle ? pending : 0U);

	stats->execution_cycles = stats->total_cycles + stats->idle_cycles;

	k_spin_unlock(&usage_lock, کلید);
}
#endif /* CONFIG_SCHED_THREAD_USAGE_ALL */

تهی z_sched_thread_usage(ساختار k_thread *thread,
			  ساختار k_thread_runtime_stats *stats)
{
	ساختار _cpu *cpu;
	k_spinlock_کلید_t  کلید;

	کلید = k_spin_lock(&usage_lock);
	cpu = _current_cpu;


	اگر (thread == cpu->current) {
		uint32_t now = usage_now();
		uint32_t cycles = now - cpu->usage0;

		/*
		 * Getting stats for the current thread. Update both the
		 * current thread stats and its CPU stats as the CPU's
		 * [usage0] field will also get updated. This keeps all
		 * that information up-to-date.
		 */

		اگر (thread->base.usage.track_usage) {
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

	اگر (thread->base.usage.رقم_windows == 0) {
		stats->average_cycles = 0;
	} وگرنه {
		stats->average_cycles = stats->total_cycles /
					thread->base.usage.رقم_windows;
	}
#endif /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */

#ifdef CONFIG_SCHED_THREAD_USAGE_ALL
	stats->idle_cycles = 0;
#endif /* CONFIG_SCHED_THREAD_USAGE_ALL */

	k_spin_unlock(&usage_lock, کلید);
}

#ifdef CONFIG_SCHED_THREAD_USAGE_ANALYSIS
صحیح k_thread_runtime_stats_enable(k_tid_t  thread)
{
	k_spinlock_کلید_t  کلید;

	CHECKIF(thread == پوچ) {
		بازگشت -EINVAL;
	}

	کلید = k_spin_lock(&usage_lock);
	ساختار _cpu *cpu = _current_cpu;

	اگر (!thread->base.usage.track_usage) {
		thread->base.usage.track_usage = true;
		thread->base.usage.رقم_windows++;
		thread->base.usage.current = 0;

		اگر (thread == cpu->current) {
			uint32_t now = usage_now();

			sched_cpu_update_usage(cpu, now - cpu->usage0);

			cpu->usage0 = now;
		}
	}

	k_spin_unlock(&usage_lock, کلید);

	بازگشت 0;
}

صحیح k_thread_runtime_stats_disable(k_tid_t  thread)
{
	k_spinlock_کلید_t کلید;

	CHECKIF(thread == پوچ) {
		بازگشت -EINVAL;
	}

	کلید = k_spin_lock(&usage_lock);
	ساختار _cpu *cpu = _current_cpu;

	اگر (thread->base.usage.track_usage) {
		thread->base.usage.track_usage = false;

		اگر (thread == cpu->current) {
			uint32_t now = usage_now();
			uint32_t cycles = now - cpu->usage0;

			sched_thread_update_usage(thread, cycles);
			sched_cpu_update_usage(cpu, cycles);

			cpu->usage0 = now;
		}
	}

	k_spin_unlock(&usage_lock, کلید);

	بازگشت 0;
}

bool k_thread_runtime_stats_is_enabled(k_tid_t thread)
{
	بازگشت thread->base.usage.track_usage;
}
#endif /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */

#ifdef CONFIG_SCHED_THREAD_USAGE_ALL
تهی k_sys_runtime_stats_enable(تهی)
{
	k_spinlock_کلید_t  کلید;

	کلید = k_spin_lock(&usage_lock);

	اگر (_current_cpu->usage->track_usage) {

		/*
		 * Usage tracking is already enabled on the current CPU
		 * and thus on all other CPUs (if applicable). There is
		 * nothing left to do.
		 */

		k_spin_unlock(&usage_lock, کلید);
		بازگشت;
	}

	/* Enable gathering of runtime stats on each CPU */

	unsigned صحیح رقم_cpus = arch_رقم_cpus();

	برای (unsigned صحیح i = 0; i < رقم_cpus; i++) {
		_kernel.cpus[i].usage->track_usage = true;
#ifdef CONFIG_SCHED_THREAD_USAGE_ANALYSIS
		_kernel.cpus[i].usage->رقم_windows++;
		_kernel.cpus[i].usage->current = 0;
#endif /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */
	}

	k_spin_unlock(&usage_lock, کلید);
}

تهی k_sys_runtime_stats_disable(تهی)
{
	ساختار _cpu *cpu;
	k_spinlock_کلید_t کلید;

	کلید = k_spin_lock(&usage_lock);

	اگر (!_current_cpu->usage->track_usage) {

		/*
		 * Usage tracking is already disabled on the current CPU
		 * and thus on all other CPUs (if applicable). There is
		 * nothing left to do.
		 */

		k_spin_unlock(&usage_lock, کلید);
		بازگشت;
	}

	uint32_t now = usage_now();

	unsigned صحیح رقم_cpus = arch_رقم_cpus();

	برای (unsigned صحیح i = 0; i < رقم_cpus; i++) {
		cpu = &_kernel.cpus[i];
		اگر (cpu->usage0 != 0) {
			sched_cpu_update_usage(cpu, now - cpu->usage0);
		}
		cpu->usage->track_usage = false;
	}

	k_spin_unlock(&usage_lock, کلید);
}
#endif /* CONFIG_SCHED_THREAD_USAGE_ALL */

#ifdef CONFIG_OBJ_CORE_STATS_THREAD
صحیح z_thread_stats_raw(ساختار k_obj_core *obj_core, تهی *stats)
{
	k_spinlock_کلید_t  کلید;

	کلید = k_spin_lock(&usage_lock);
	memcpy(stats, obj_core->stats, اندازه(ساختار k_cycle_stats));
	k_spin_unlock(&usage_lock, کلید);

	بازگشت 0;
}

صحیح z_thread_stats_query(ساختار k_obj_core *obj_core, تهی *stats)
{
	ساختار k_thread *thread;

	thread = CONTAINER_OF(obj_core, ساختار k_thread, obj_core);

	z_sched_thread_usage(thread, stats);

	بازگشت 0;
}

صحیح z_thread_stats_reset(ساختار k_obj_core *obj_core)
{
	k_spinlock_کلید_t  کلید;
	ساختار k_cycle_stats  *stats;
	ساختار k_thread *thread;

	thread = CONTAINER_OF(obj_core, ساختار k_thread, obj_core);
	کلید = k_spin_lock(&usage_lock);
	stats = obj_core->stats;

	stats->total = 0ULL;
#ifdef CONFIG_SCHED_THREAD_USAGE_ANALYSIS
	stats->current = 0ULL;
	stats->longest = 0ULL;
	stats->رقم_windows = (thread->base.usage.track_usage) ?  1U : 0U;
#endif /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */

	اگر (thread != _current_cpu->current) {

		/*
		 * If the thread is not running, there is nothing else to do.
		 * If the thread is running on another core, then it is not
		 * safe to do anything else but unlock and return (and pretend
		 * that its stats were reset at the start of its execution
		 * window.
		 */

		k_spin_unlock(&usage_lock, کلید);

		بازگشت 0;
	}

	/* Update the current CPU stats. */

	uint32_t now = usage_now();

	sched_cpu_update_usage(_current_cpu, now - _current_cpu->usage0);

	_current_cpu->usage0 = now;

	k_spin_unlock(&usage_lock, کلید);

	بازگشت 0;
}

صحیح z_thread_stats_disable(ساختار k_obj_core *obj_core)
{
#ifdef CONFIG_SCHED_THREAD_USAGE_ANALYSIS
	ساختار k_thread *thread;

	thread = CONTAINER_OF(obj_core, ساختار k_thread, obj_core);

	بازگشت k_thread_runtime_stats_disable(thread);
#وگرنه
	بازگشت -ENOTSUP;
#endif /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */
}

صحیح z_thread_stats_enable(ساختار k_obj_core *obj_core)
{
#ifdef CONFIG_SCHED_THREAD_USAGE_ANALYSIS
	ساختار k_thread *thread;

	thread = CONTAINER_OF(obj_core, ساختار k_thread, obj_core);

	بازگشت k_thread_runtime_stats_enable(thread);
#وگرنه
	بازگشت -ENOTSUP;
#endif /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */
}
#endif /* CONFIG_OBJ_CORE_STATS_THREAD */

#ifdef CONFIG_OBJ_CORE_STATS_SYSTEM
صحیح z_cpu_stats_raw(ساختار k_obj_core *obj_core, تهی *stats)
{
	k_spinlock_کلید_t  کلید;

	کلید = k_spin_lock(&usage_lock);
	memcpy(stats, obj_core->stats, اندازه(ساختار k_cycle_stats));
	k_spin_unlock(&usage_lock, کلید);

	بازگشت 0;
}

صحیح z_cpu_stats_query(ساختار k_obj_core *obj_core, تهی *stats)
{
	ساختار _cpu  *cpu;

	cpu = CONTAINER_OF(obj_core, ساختار _cpu, obj_core);

	z_sched_cpu_usage(cpu->id, stats);

	بازگشت 0;
}
#endif /* CONFIG_OBJ_CORE_STATS_SYSTEM */

#ifdef CONFIG_OBJ_CORE_STATS_SYSTEM
صحیح z_kernel_stats_raw(ساختار k_obj_core *obj_core, تهی *stats)
{
	k_spinlock_کلید_t  کلید;

	کلید = k_spin_lock(&usage_lock);
	memcpy(stats, obj_core->stats,
	       CONFIG_MP_MAX_رقم_CPUS * اندازه(ساختار k_cycle_stats));
	k_spin_unlock(&usage_lock, کلید);

	بازگشت 0;
}

صحیح z_kernel_stats_query(ساختار k_obj_core *obj_core, تهی *stats)
{
	ARG_UNUSED(obj_core);

	بازگشت k_thread_runtime_stats_all_get(stats);
}
#endif /* CONFIG_OBJ_CORE_STATS_SYSTEM */
