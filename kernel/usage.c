/*
 * Copyright (c) 2018 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ଅନ୍ତର୍ଭାଵ <zephyr/kernel.h>

#ଅନ୍ତର୍ଭାଵ <zephyr/timing/timing.h>
#ଅନ୍ତର୍ଭାଵ <ksched.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/spinlock.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/sys/check.h>

/* Need one of these for this to work */
#ଯଦି !defined(CONFIG_USE_SWITCH) && !defined(CONFIG_INSTRUMENT_THREAD_SWITCHING)
#error "No data backend configured for CONFIG_SCHED_THREAD_USAGE"
#endif /* !CONFIG_USE_SWITCH && !CONFIG_INSTRUMENT_THREAD_SWITCHING */

ସ୍ଥିର ସଂରଚନା k_spinlock usage_lock;

ସ୍ଥିର uint32_t usage_now(ଶୂନ୍ୟ)
{
	uint32_t now;

#ifdef CONFIG_THREAD_RUNTIME_STATS_USE_TIMING_FUNCTIONS
	now = (uint32_t)timing_counter_get();
#ଅନ୍ୟଥା
	now = k_cycle_get_32();
#endif /* CONFIG_THREAD_RUNTIME_STATS_USE_TIMING_FUNCTIONS */

	/* Edge case: we use a zero as a null ("stop() already called") */
	ଫେରନ୍ତୁ (now == 0) ? 1 : now;
}

#ifdef CONFIG_SCHED_THREAD_USAGE_ALL
ସ୍ଥିର ଶୂନ୍ୟ sched_cpu_update_usage(ସଂରଚନା _cpu *cpu, uint32_t cycles)
{
	ଯଦି (!cpu->usage->track_usage) {
		ଫେରନ୍ତୁ;
	}

	ଯଦି (cpu->current != cpu->idle_thread) {
		cpu->usage->total += cycles;

#ifdef CONFIG_SCHED_THREAD_USAGE_ANALYSIS
		cpu->usage->current += cycles;

		ଯଦି (cpu->usage->longest < cpu->usage->current) {
			cpu->usage->longest = cpu->usage->current;
		}
	} ଅନ୍ୟଥା {
		cpu->usage->current = 0;
		cpu->usage->ଅଙ୍କ_windows++;
#endif /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */
	}
}
#ଅନ୍ୟଥା
#define sched_cpu_update_usage(cpu, cycles)   କର { } ଯେତେବେଳେ (0)
#endif /* CONFIG_SCHED_THREAD_USAGE_ALL */

ସ୍ଥିର ଶୂନ୍ୟ sched_thread_update_usage(ସଂରଚନା k_thread *thread, uint32_t cycles)
{
	thread->base.usage.total += cycles;

#ifdef CONFIG_SCHED_THREAD_USAGE_ANALYSIS
	thread->base.usage.current += cycles;

	ଯଦି (thread->base.usage.longest < thread->base.usage.current) {
		thread->base.usage.longest = thread->base.usage.current;
	}
#endif /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */
}

ଶୂନ୍ୟ z_sched_usage_start(ସଂରଚନା k_thread *thread)
{
#ifdef CONFIG_SCHED_THREAD_USAGE_ANALYSIS
	k_spinlock_ଚାବି_t  ଚାବି;

	ଚାବି = k_spin_lock(&usage_lock);

	_current_cpu->usage0 = usage_now();   /* Always update */

	ଯଦି (thread->base.usage.track_usage) {
		thread->base.usage.ଅଙ୍କ_windows++;
		thread->base.usage.current = 0;
	}

	k_spin_unlock(&usage_lock, ଚାବି);
#ଅନ୍ୟଥା
	/* One write through a volatile pointer doesn't require
	 * synchronization as long as _usage() treats it as volatile
	 * (we can't race with _stop() by design).
	 */

	_current_cpu->usage0 = usage_now();
#endif /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */
}

ଶୂନ୍ୟ z_sched_usage_stop(ଶୂନ୍ୟ)
{
	ସଂରଚନା _cpu *cpu = _current_cpu;

	/* Only this CPU writes usage0, and callers keep local IRQs masked. */
	ଯଦି (cpu->usage0 == 0U) {
		ଫେରନ୍ତୁ;
	}

	k_spinlock_ଚାବି_t k = k_spin_lock(&usage_lock);

	uint32_t u0 = cpu->usage0;

	ଯଦି (u0 != 0) {
		uint32_t cycles = usage_now() - u0;

		ଯଦି (cpu->current->base.usage.track_usage) {
			sched_thread_update_usage(cpu->current, cycles);
		}

		sched_cpu_update_usage(cpu, cycles);
	}

	cpu->usage0 = 0;
	k_spin_unlock(&usage_lock, k);
}

#ifdef CONFIG_SCHED_THREAD_USAGE_ALL
ଶୂନ୍ୟ z_sched_cpu_usage(uint8_t cpu_id, ସଂରଚନା k_thread_runtime_stats *stats)
{
	k_spinlock_ଚାବି_t  ଚାବି;
	ସଂରଚନା _cpu *cpu;
	uint32_t pending = 0U;
	bool pending_is_idle = false;

	ଚାବି = k_spin_lock(&usage_lock);
	cpu = &_kernel.cpus[cpu_id];

	ଯଦି (cpu == _current_cpu) {
		uint32_t  now = usage_now();
		uint32_t cycles = now - cpu->usage0;

		/*
		 * Getting stats for the current CPU. Update both its
		 * current thread stats and the CPU stats as the CPU's
		 * [usage0] field will also get updated. This keeps all
		 * that information up-to-date.
		 */

		ଯଦି (cpu->current->base.usage.track_usage) {
			sched_thread_update_usage(cpu->current, cycles);
		}

		sched_cpu_update_usage(cpu, cycles);

		cpu->usage0 = now;
	} ଅନ୍ୟଥା ଯଦି (cpu->usage->track_usage) {
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

	ଯଦି (cpu->usage->ଅଙ୍କ_windows == 0) {
		stats->average_cycles = 0;
	} ଅନ୍ୟଥା {
		stats->average_cycles = stats->total_cycles /
					cpu->usage->ଅଙ୍କ_windows;
	}
#endif /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */

	stats->idle_cycles =
		_kernel.cpus[cpu_id].idle_thread->base.usage.total +
		(pending_is_idle ? pending : 0U);

	stats->execution_cycles = stats->total_cycles + stats->idle_cycles;

	k_spin_unlock(&usage_lock, ଚାବି);
}
#endif /* CONFIG_SCHED_THREAD_USAGE_ALL */

ଶୂନ୍ୟ z_sched_thread_usage(ସଂରଚନା k_thread *thread,
			  ସଂରଚନା k_thread_runtime_stats *stats)
{
	ସଂରଚନା _cpu *cpu;
	k_spinlock_ଚାବି_t  ଚାବି;

	ଚାବି = k_spin_lock(&usage_lock);
	cpu = _current_cpu;


	ଯଦି (thread == cpu->current) {
		uint32_t now = usage_now();
		uint32_t cycles = now - cpu->usage0;

		/*
		 * Getting stats for the current thread. Update both the
		 * current thread stats and its CPU stats as the CPU's
		 * [usage0] field will also get updated. This keeps all
		 * that information up-to-date.
		 */

		ଯଦି (thread->base.usage.track_usage) {
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

	ଯଦି (thread->base.usage.ଅଙ୍କ_windows == 0) {
		stats->average_cycles = 0;
	} ଅନ୍ୟଥା {
		stats->average_cycles = stats->total_cycles /
					thread->base.usage.ଅଙ୍କ_windows;
	}
#endif /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */

#ifdef CONFIG_SCHED_THREAD_USAGE_ALL
	stats->idle_cycles = 0;
#endif /* CONFIG_SCHED_THREAD_USAGE_ALL */

	k_spin_unlock(&usage_lock, ଚାବି);
}

#ifdef CONFIG_SCHED_THREAD_USAGE_ANALYSIS
ପୂର୍ଣ୍ଣସଂଖ୍ୟା k_thread_runtime_stats_enable(k_tid_t  thread)
{
	k_spinlock_ଚାବି_t  ଚାବି;

	CHECKIF(thread == ଖାଲି) {
		ଫେରନ୍ତୁ -EINVAL;
	}

	ଚାବି = k_spin_lock(&usage_lock);
	ସଂରଚନା _cpu *cpu = _current_cpu;

	ଯଦି (!thread->base.usage.track_usage) {
		thread->base.usage.track_usage = true;
		thread->base.usage.ଅଙ୍କ_windows++;
		thread->base.usage.current = 0;

		ଯଦି (thread == cpu->current) {
			uint32_t now = usage_now();

			sched_cpu_update_usage(cpu, now - cpu->usage0);

			cpu->usage0 = now;
		}
	}

	k_spin_unlock(&usage_lock, ଚାବି);

	ଫେରନ୍ତୁ 0;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା k_thread_runtime_stats_disable(k_tid_t  thread)
{
	k_spinlock_ଚାବି_t ଚାବି;

	CHECKIF(thread == ଖାଲି) {
		ଫେରନ୍ତୁ -EINVAL;
	}

	ଚାବି = k_spin_lock(&usage_lock);
	ସଂରଚନା _cpu *cpu = _current_cpu;

	ଯଦି (thread->base.usage.track_usage) {
		thread->base.usage.track_usage = false;

		ଯଦି (thread == cpu->current) {
			uint32_t now = usage_now();
			uint32_t cycles = now - cpu->usage0;

			sched_thread_update_usage(thread, cycles);
			sched_cpu_update_usage(cpu, cycles);

			cpu->usage0 = now;
		}
	}

	k_spin_unlock(&usage_lock, ଚାବି);

	ଫେରନ୍ତୁ 0;
}

bool k_thread_runtime_stats_is_enabled(k_tid_t thread)
{
	ଫେରନ୍ତୁ thread->base.usage.track_usage;
}
#endif /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */

#ifdef CONFIG_SCHED_THREAD_USAGE_ALL
ଶୂନ୍ୟ k_sys_runtime_stats_enable(ଶୂନ୍ୟ)
{
	k_spinlock_ଚାବି_t  ଚାବି;

	ଚାବି = k_spin_lock(&usage_lock);

	ଯଦି (_current_cpu->usage->track_usage) {

		/*
		 * Usage tracking is already enabled on the current CPU
		 * and thus on all other CPUs (if applicable). There is
		 * nothing left to do.
		 */

		k_spin_unlock(&usage_lock, ଚାବି);
		ଫେରନ୍ତୁ;
	}

	/* Enable gathering of runtime stats on each CPU */

	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଅଙ୍କ_cpus = arch_ଅଙ୍କ_cpus();

	ପାଇଁ (unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା i = 0; i < ଅଙ୍କ_cpus; i++) {
		_kernel.cpus[i].usage->track_usage = true;
#ifdef CONFIG_SCHED_THREAD_USAGE_ANALYSIS
		_kernel.cpus[i].usage->ଅଙ୍କ_windows++;
		_kernel.cpus[i].usage->current = 0;
#endif /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */
	}

	k_spin_unlock(&usage_lock, ଚାବି);
}

ଶୂନ୍ୟ k_sys_runtime_stats_disable(ଶୂନ୍ୟ)
{
	ସଂରଚନା _cpu *cpu;
	k_spinlock_ଚାବି_t ଚାବି;

	ଚାବି = k_spin_lock(&usage_lock);

	ଯଦି (!_current_cpu->usage->track_usage) {

		/*
		 * Usage tracking is already disabled on the current CPU
		 * and thus on all other CPUs (if applicable). There is
		 * nothing left to do.
		 */

		k_spin_unlock(&usage_lock, ଚାବି);
		ଫେରନ୍ତୁ;
	}

	uint32_t now = usage_now();

	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଅଙ୍କ_cpus = arch_ଅଙ୍କ_cpus();

	ପାଇଁ (unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା i = 0; i < ଅଙ୍କ_cpus; i++) {
		cpu = &_kernel.cpus[i];
		ଯଦି (cpu->usage0 != 0) {
			sched_cpu_update_usage(cpu, now - cpu->usage0);
		}
		cpu->usage->track_usage = false;
	}

	k_spin_unlock(&usage_lock, ଚାବି);
}
#endif /* CONFIG_SCHED_THREAD_USAGE_ALL */

#ifdef CONFIG_OBJ_CORE_STATS_THREAD
ପୂର୍ଣ୍ଣସଂଖ୍ୟା z_thread_stats_raw(ସଂରଚନା k_obj_core *obj_core, ଶୂନ୍ୟ *stats)
{
	k_spinlock_ଚାବି_t  ଚାବି;

	ଚାବି = k_spin_lock(&usage_lock);
	memcpy(stats, obj_core->stats, ଆକାର(ସଂରଚନା k_cycle_stats));
	k_spin_unlock(&usage_lock, ଚାବି);

	ଫେରନ୍ତୁ 0;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା z_thread_stats_query(ସଂରଚନା k_obj_core *obj_core, ଶୂନ୍ୟ *stats)
{
	ସଂରଚନା k_thread *thread;

	thread = CONTAINER_OF(obj_core, ସଂରଚନା k_thread, obj_core);

	z_sched_thread_usage(thread, stats);

	ଫେରନ୍ତୁ 0;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା z_thread_stats_reset(ସଂରଚନା k_obj_core *obj_core)
{
	k_spinlock_ଚାବି_t  ଚାବି;
	ସଂରଚନା k_cycle_stats  *stats;
	ସଂରଚନା k_thread *thread;

	thread = CONTAINER_OF(obj_core, ସଂରଚନା k_thread, obj_core);
	ଚାବି = k_spin_lock(&usage_lock);
	stats = obj_core->stats;

	stats->total = 0ULL;
#ifdef CONFIG_SCHED_THREAD_USAGE_ANALYSIS
	stats->current = 0ULL;
	stats->longest = 0ULL;
	stats->ଅଙ୍କ_windows = (thread->base.usage.track_usage) ?  1U : 0U;
#endif /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */

	ଯଦି (thread != _current_cpu->current) {

		/*
		 * If the thread is not running, there is nothing else to do.
		 * If the thread is running on another core, then it is not
		 * safe to do anything else but unlock and return (and pretend
		 * that its stats were reset at the start of its execution
		 * window.
		 */

		k_spin_unlock(&usage_lock, ଚାବି);

		ଫେରନ୍ତୁ 0;
	}

	/* Update the current CPU stats. */

	uint32_t now = usage_now();

	sched_cpu_update_usage(_current_cpu, now - _current_cpu->usage0);

	_current_cpu->usage0 = now;

	k_spin_unlock(&usage_lock, ଚାବି);

	ଫେରନ୍ତୁ 0;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା z_thread_stats_disable(ସଂରଚନା k_obj_core *obj_core)
{
#ifdef CONFIG_SCHED_THREAD_USAGE_ANALYSIS
	ସଂରଚନା k_thread *thread;

	thread = CONTAINER_OF(obj_core, ସଂରଚନା k_thread, obj_core);

	ଫେରନ୍ତୁ k_thread_runtime_stats_disable(thread);
#ଅନ୍ୟଥା
	ଫେରନ୍ତୁ -ENOTSUP;
#endif /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା z_thread_stats_enable(ସଂରଚନା k_obj_core *obj_core)
{
#ifdef CONFIG_SCHED_THREAD_USAGE_ANALYSIS
	ସଂରଚନା k_thread *thread;

	thread = CONTAINER_OF(obj_core, ସଂରଚନା k_thread, obj_core);

	ଫେରନ୍ତୁ k_thread_runtime_stats_enable(thread);
#ଅନ୍ୟଥା
	ଫେରନ୍ତୁ -ENOTSUP;
#endif /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */
}
#endif /* CONFIG_OBJ_CORE_STATS_THREAD */

#ifdef CONFIG_OBJ_CORE_STATS_SYSTEM
ପୂର୍ଣ୍ଣସଂଖ୍ୟା z_cpu_stats_raw(ସଂରଚନା k_obj_core *obj_core, ଶୂନ୍ୟ *stats)
{
	k_spinlock_ଚାବି_t  ଚାବି;

	ଚାବି = k_spin_lock(&usage_lock);
	memcpy(stats, obj_core->stats, ଆକାର(ସଂରଚନା k_cycle_stats));
	k_spin_unlock(&usage_lock, ଚାବି);

	ଫେରନ୍ତୁ 0;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା z_cpu_stats_query(ସଂରଚନା k_obj_core *obj_core, ଶୂନ୍ୟ *stats)
{
	ସଂରଚନା _cpu  *cpu;

	cpu = CONTAINER_OF(obj_core, ସଂରଚନା _cpu, obj_core);

	z_sched_cpu_usage(cpu->id, stats);

	ଫେରନ୍ତୁ 0;
}
#endif /* CONFIG_OBJ_CORE_STATS_SYSTEM */

#ifdef CONFIG_OBJ_CORE_STATS_SYSTEM
ପୂର୍ଣ୍ଣସଂଖ୍ୟା z_kernel_stats_raw(ସଂରଚନା k_obj_core *obj_core, ଶୂନ୍ୟ *stats)
{
	k_spinlock_ଚାବି_t  ଚାବି;

	ଚାବି = k_spin_lock(&usage_lock);
	memcpy(stats, obj_core->stats,
	       CONFIG_MP_MAX_ଅଙ୍କ_CPUS * ଆକାର(ସଂରଚନା k_cycle_stats));
	k_spin_unlock(&usage_lock, ଚାବି);

	ଫେରନ୍ତୁ 0;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା z_kernel_stats_query(ସଂରଚନା k_obj_core *obj_core, ଶୂନ୍ୟ *stats)
{
	ARG_UNUSED(obj_core);

	ଫେରନ୍ତୁ k_thread_runtime_stats_all_get(stats);
}
#endif /* CONFIG_OBJ_CORE_STATS_SYSTEM */
