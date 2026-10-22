/*
 * Copyright (c) 2018 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#अन्तर्भाव <zephyr/kernel.h>

#अन्तर्भाव <zephyr/timing/timing.h>
#अन्तर्भाव <ksched.h>
#अन्तर्भाव <zephyr/spinlock.h>
#अन्तर्भाव <zephyr/sys/check.h>

/* Need one of these for this to work */
#यदि !defined(CONFIG_USE_SWITCH) && !defined(CONFIG_INSTRUMENT_THREAD_SWITCHING)
#error "No data backend configured for CONFIG_SCHED_THREAD_USAGE"
#endif /* !CONFIG_USE_SWITCH && !CONFIG_INSTRUMENT_THREAD_SWITCHING */

स्थिर संरचना k_spinlock usage_lock;

स्थिर uint32_t usage_now(शून्य)
{
	uint32_t now;

#ifdef CONFIG_THREAD_RUNTIME_STATS_USE_TIMING_FUNCTIONS
	now = (uint32_t)timing_counter_get();
#अन्यथा
	now = k_cycle_get_32();
#endif /* CONFIG_THREAD_RUNTIME_STATS_USE_TIMING_FUNCTIONS */

	/* Edge case: we use a zero as a null ("stop() already called") */
	फर्काऊ (now == 0) ? 1 : now;
}

#ifdef CONFIG_SCHED_THREAD_USAGE_ALL
स्थिर शून्य sched_cpu_update_usage(संरचना _cpu *cpu, uint32_t cycles)
{
	यदि (!cpu->usage->track_usage) {
		फर्काऊ;
	}

	यदि (cpu->current != cpu->idle_thread) {
		cpu->usage->total += cycles;

#ifdef CONFIG_SCHED_THREAD_USAGE_ANALYSIS
		cpu->usage->current += cycles;

		यदि (cpu->usage->longest < cpu->usage->current) {
			cpu->usage->longest = cpu->usage->current;
		}
	} अन्यथा {
		cpu->usage->current = 0;
		cpu->usage->अङ्क_windows++;
#endif /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */
	}
}
#अन्यथा
#define sched_cpu_update_usage(cpu, cycles)   गर { } जबसम्म (0)
#endif /* CONFIG_SCHED_THREAD_USAGE_ALL */

स्थिर शून्य sched_thread_update_usage(संरचना k_thread *thread, uint32_t cycles)
{
	thread->base.usage.total += cycles;

#ifdef CONFIG_SCHED_THREAD_USAGE_ANALYSIS
	thread->base.usage.current += cycles;

	यदि (thread->base.usage.longest < thread->base.usage.current) {
		thread->base.usage.longest = thread->base.usage.current;
	}
#endif /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */
}

शून्य z_sched_usage_start(संरचना k_thread *thread)
{
#ifdef CONFIG_SCHED_THREAD_USAGE_ANALYSIS
	k_spinlock_साँचो_t  साँचो;

	साँचो = k_spin_lock(&usage_lock);

	_current_cpu->usage0 = usage_now();   /* Always update */

	यदि (thread->base.usage.track_usage) {
		thread->base.usage.अङ्क_windows++;
		thread->base.usage.current = 0;
	}

	k_spin_unlock(&usage_lock, साँचो);
#अन्यथा
	/* One write through a volatile pointer doesn't require
	 * synchronization as long as _usage() treats it as volatile
	 * (we can't race with _stop() by design).
	 */

	_current_cpu->usage0 = usage_now();
#endif /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */
}

शून्य z_sched_usage_stop(शून्य)
{
	संरचना _cpu *cpu = _current_cpu;

	/* Only this CPU writes usage0, and callers keep local IRQs masked. */
	यदि (cpu->usage0 == 0U) {
		फर्काऊ;
	}

	k_spinlock_साँचो_t k = k_spin_lock(&usage_lock);

	uint32_t u0 = cpu->usage0;

	यदि (u0 != 0) {
		uint32_t cycles = usage_now() - u0;

		यदि (cpu->current->base.usage.track_usage) {
			sched_thread_update_usage(cpu->current, cycles);
		}

		sched_cpu_update_usage(cpu, cycles);
	}

	cpu->usage0 = 0;
	k_spin_unlock(&usage_lock, k);
}

#ifdef CONFIG_SCHED_THREAD_USAGE_ALL
शून्य z_sched_cpu_usage(uint8_t cpu_id, संरचना k_thread_runtime_stats *stats)
{
	k_spinlock_साँचो_t  साँचो;
	संरचना _cpu *cpu;
	uint32_t pending = 0U;
	bool pending_is_idle = false;

	साँचो = k_spin_lock(&usage_lock);
	cpu = &_kernel.cpus[cpu_id];

	यदि (cpu == _current_cpu) {
		uint32_t  now = usage_now();
		uint32_t cycles = now - cpu->usage0;

		/*
		 * Getting stats for the current CPU. Update both its
		 * current thread stats and the CPU stats as the CPU's
		 * [usage0] field will also get updated. This keeps all
		 * that information up-to-date.
		 */

		यदि (cpu->current->base.usage.track_usage) {
			sched_thread_update_usage(cpu->current, cycles);
		}

		sched_cpu_update_usage(cpu, cycles);

		cpu->usage0 = now;
	} अन्यथा यदि (cpu->usage->track_usage) {
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

	यदि (cpu->usage->अङ्क_windows == 0) {
		stats->average_cycles = 0;
	} अन्यथा {
		stats->average_cycles = stats->total_cycles /
					cpu->usage->अङ्क_windows;
	}
#endif /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */

	stats->idle_cycles =
		_kernel.cpus[cpu_id].idle_thread->base.usage.total +
		(pending_is_idle ? pending : 0U);

	stats->execution_cycles = stats->total_cycles + stats->idle_cycles;

	k_spin_unlock(&usage_lock, साँचो);
}
#endif /* CONFIG_SCHED_THREAD_USAGE_ALL */

शून्य z_sched_thread_usage(संरचना k_thread *thread,
			  संरचना k_thread_runtime_stats *stats)
{
	संरचना _cpu *cpu;
	k_spinlock_साँचो_t  साँचो;

	साँचो = k_spin_lock(&usage_lock);
	cpu = _current_cpu;


	यदि (thread == cpu->current) {
		uint32_t now = usage_now();
		uint32_t cycles = now - cpu->usage0;

		/*
		 * Getting stats for the current thread. Update both the
		 * current thread stats and its CPU stats as the CPU's
		 * [usage0] field will also get updated. This keeps all
		 * that information up-to-date.
		 */

		यदि (thread->base.usage.track_usage) {
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

	यदि (thread->base.usage.अङ्क_windows == 0) {
		stats->average_cycles = 0;
	} अन्यथा {
		stats->average_cycles = stats->total_cycles /
					thread->base.usage.अङ्क_windows;
	}
#endif /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */

#ifdef CONFIG_SCHED_THREAD_USAGE_ALL
	stats->idle_cycles = 0;
#endif /* CONFIG_SCHED_THREAD_USAGE_ALL */

	k_spin_unlock(&usage_lock, साँचो);
}

#ifdef CONFIG_SCHED_THREAD_USAGE_ANALYSIS
पूर्णाङ्क k_thread_runtime_stats_enable(k_tid_t  thread)
{
	k_spinlock_साँचो_t  साँचो;

	CHECKIF(thread == रिक्त) {
		फर्काऊ -EINVAL;
	}

	साँचो = k_spin_lock(&usage_lock);
	संरचना _cpu *cpu = _current_cpu;

	यदि (!thread->base.usage.track_usage) {
		thread->base.usage.track_usage = true;
		thread->base.usage.अङ्क_windows++;
		thread->base.usage.current = 0;

		यदि (thread == cpu->current) {
			uint32_t now = usage_now();

			sched_cpu_update_usage(cpu, now - cpu->usage0);

			cpu->usage0 = now;
		}
	}

	k_spin_unlock(&usage_lock, साँचो);

	फर्काऊ 0;
}

पूर्णाङ्क k_thread_runtime_stats_disable(k_tid_t  thread)
{
	k_spinlock_साँचो_t साँचो;

	CHECKIF(thread == रिक्त) {
		फर्काऊ -EINVAL;
	}

	साँचो = k_spin_lock(&usage_lock);
	संरचना _cpu *cpu = _current_cpu;

	यदि (thread->base.usage.track_usage) {
		thread->base.usage.track_usage = false;

		यदि (thread == cpu->current) {
			uint32_t now = usage_now();
			uint32_t cycles = now - cpu->usage0;

			sched_thread_update_usage(thread, cycles);
			sched_cpu_update_usage(cpu, cycles);

			cpu->usage0 = now;
		}
	}

	k_spin_unlock(&usage_lock, साँचो);

	फर्काऊ 0;
}

bool k_thread_runtime_stats_is_enabled(k_tid_t thread)
{
	फर्काऊ thread->base.usage.track_usage;
}
#endif /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */

#ifdef CONFIG_SCHED_THREAD_USAGE_ALL
शून्य k_sys_runtime_stats_enable(शून्य)
{
	k_spinlock_साँचो_t  साँचो;

	साँचो = k_spin_lock(&usage_lock);

	यदि (_current_cpu->usage->track_usage) {

		/*
		 * Usage tracking is already enabled on the current CPU
		 * and thus on all other CPUs (if applicable). There is
		 * nothing left to do.
		 */

		k_spin_unlock(&usage_lock, साँचो);
		फर्काऊ;
	}

	/* Enable gathering of runtime stats on each CPU */

	अचिन्हित पूर्णाङ्क अङ्क_cpus = arch_अङ्क_cpus();

	लागि (अचिन्हित पूर्णाङ्क i = 0; i < अङ्क_cpus; i++) {
		_kernel.cpus[i].usage->track_usage = true;
#ifdef CONFIG_SCHED_THREAD_USAGE_ANALYSIS
		_kernel.cpus[i].usage->अङ्क_windows++;
		_kernel.cpus[i].usage->current = 0;
#endif /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */
	}

	k_spin_unlock(&usage_lock, साँचो);
}

शून्य k_sys_runtime_stats_disable(शून्य)
{
	संरचना _cpu *cpu;
	k_spinlock_साँचो_t साँचो;

	साँचो = k_spin_lock(&usage_lock);

	यदि (!_current_cpu->usage->track_usage) {

		/*
		 * Usage tracking is already disabled on the current CPU
		 * and thus on all other CPUs (if applicable). There is
		 * nothing left to do.
		 */

		k_spin_unlock(&usage_lock, साँचो);
		फर्काऊ;
	}

	uint32_t now = usage_now();

	अचिन्हित पूर्णाङ्क अङ्क_cpus = arch_अङ्क_cpus();

	लागि (अचिन्हित पूर्णाङ्क i = 0; i < अङ्क_cpus; i++) {
		cpu = &_kernel.cpus[i];
		यदि (cpu->usage0 != 0) {
			sched_cpu_update_usage(cpu, now - cpu->usage0);
		}
		cpu->usage->track_usage = false;
	}

	k_spin_unlock(&usage_lock, साँचो);
}
#endif /* CONFIG_SCHED_THREAD_USAGE_ALL */

#ifdef CONFIG_OBJ_CORE_STATS_THREAD
पूर्णाङ्क z_thread_stats_raw(संरचना k_obj_core *obj_core, शून्य *stats)
{
	k_spinlock_साँचो_t  साँचो;

	साँचो = k_spin_lock(&usage_lock);
	स_नकल(stats, obj_core->stats, परिमाण(संरचना k_cycle_stats));
	k_spin_unlock(&usage_lock, साँचो);

	फर्काऊ 0;
}

पूर्णाङ्क z_thread_stats_query(संरचना k_obj_core *obj_core, शून्य *stats)
{
	संरचना k_thread *thread;

	thread = CONTAINER_OF(obj_core, संरचना k_thread, obj_core);

	z_sched_thread_usage(thread, stats);

	फर्काऊ 0;
}

पूर्णाङ्क z_thread_stats_reset(संरचना k_obj_core *obj_core)
{
	k_spinlock_साँचो_t  साँचो;
	संरचना k_cycle_stats  *stats;
	संरचना k_thread *thread;

	thread = CONTAINER_OF(obj_core, संरचना k_thread, obj_core);
	साँचो = k_spin_lock(&usage_lock);
	stats = obj_core->stats;

	stats->total = 0ULL;
#ifdef CONFIG_SCHED_THREAD_USAGE_ANALYSIS
	stats->current = 0ULL;
	stats->longest = 0ULL;
	stats->अङ्क_windows = (thread->base.usage.track_usage) ?  1U : 0U;
#endif /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */

	यदि (thread != _current_cpu->current) {

		/*
		 * If the thread is not running, there is nothing else to do.
		 * If the thread is running on another core, then it is not
		 * safe to do anything else but unlock and return (and pretend
		 * that its stats were reset at the start of its execution
		 * window.
		 */

		k_spin_unlock(&usage_lock, साँचो);

		फर्काऊ 0;
	}

	/* Update the current CPU stats. */

	uint32_t now = usage_now();

	sched_cpu_update_usage(_current_cpu, now - _current_cpu->usage0);

	_current_cpu->usage0 = now;

	k_spin_unlock(&usage_lock, साँचो);

	फर्काऊ 0;
}

पूर्णाङ्क z_thread_stats_disable(संरचना k_obj_core *obj_core)
{
#ifdef CONFIG_SCHED_THREAD_USAGE_ANALYSIS
	संरचना k_thread *thread;

	thread = CONTAINER_OF(obj_core, संरचना k_thread, obj_core);

	फर्काऊ k_thread_runtime_stats_disable(thread);
#अन्यथा
	फर्काऊ -ENOTSUP;
#endif /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */
}

पूर्णाङ्क z_thread_stats_enable(संरचना k_obj_core *obj_core)
{
#ifdef CONFIG_SCHED_THREAD_USAGE_ANALYSIS
	संरचना k_thread *thread;

	thread = CONTAINER_OF(obj_core, संरचना k_thread, obj_core);

	फर्काऊ k_thread_runtime_stats_enable(thread);
#अन्यथा
	फर्काऊ -ENOTSUP;
#endif /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */
}
#endif /* CONFIG_OBJ_CORE_STATS_THREAD */

#ifdef CONFIG_OBJ_CORE_STATS_SYSTEM
पूर्णाङ्क z_cpu_stats_raw(संरचना k_obj_core *obj_core, शून्य *stats)
{
	k_spinlock_साँचो_t  साँचो;

	साँचो = k_spin_lock(&usage_lock);
	स_नकल(stats, obj_core->stats, परिमाण(संरचना k_cycle_stats));
	k_spin_unlock(&usage_lock, साँचो);

	फर्काऊ 0;
}

पूर्णाङ्क z_cpu_stats_query(संरचना k_obj_core *obj_core, शून्य *stats)
{
	संरचना _cpu  *cpu;

	cpu = CONTAINER_OF(obj_core, संरचना _cpu, obj_core);

	z_sched_cpu_usage(cpu->id, stats);

	फर्काऊ 0;
}
#endif /* CONFIG_OBJ_CORE_STATS_SYSTEM */

#ifdef CONFIG_OBJ_CORE_STATS_SYSTEM
पूर्णाङ्क z_kernel_stats_raw(संरचना k_obj_core *obj_core, शून्य *stats)
{
	k_spinlock_साँचो_t  साँचो;

	साँचो = k_spin_lock(&usage_lock);
	स_नकल(stats, obj_core->stats,
	       CONFIG_MP_MAX_अङ्क_CPUS * परिमाण(संरचना k_cycle_stats));
	k_spin_unlock(&usage_lock, साँचो);

	फर्काऊ 0;
}

पूर्णाङ्क z_kernel_stats_query(संरचना k_obj_core *obj_core, शून्य *stats)
{
	ARG_UNUSED(obj_core);

	फर्काऊ k_thread_runtime_stats_all_get(stats);
}
#endif /* CONFIG_OBJ_CORE_STATS_SYSTEM */
