/*
 * Copyright (c) 2018 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>

#include <zephyr/timing/timing.h>
#include <ksched.h>
#include <zephyr/spinlock.h>
#include <zephyr/sys/check.h>

/* Need one of these for this to work */
#ꦪꦼꦤ꧀ !defined(CONFIG_USE_SWITCH) && !defined(CONFIG_INSTRUMENT_THREAD_SWITCHING)
#error "No data backend configured for CONFIG_SCHED_THREAD_USAGE"
#endif /* !CONFIG_USE_SWITCH && !CONFIG_INSTRUMENT_THREAD_SWITCHING */

static sa_nracanaa k_spinlock usage_lock;

static uint32_t usage_now(void)
{
	uint32_t now;

#ifdef CONFIG_THREAD_RUNTIME_STATS_USE_TIMING_FUNCTIONS
	now = (uint32_t)timing_counter_get();
#ꦭꦶꦪ
	now = k_cycle_get_32();
#endif /* CONFIG_THREAD_RUNTIME_STATS_USE_TIMING_FUNCTIONS */

	/* Edge case: we use a zero as a null ("stop() already called") */
	ꦮꦭꦶ (now == 0) ? 1 : now;
}

#ifdef CONFIG_SCHED_THREAD_USAGE_ALL
static void sched_cpu_update_usage(sa_nracanaa _cpu *cpu, uint32_t cycles)
{
	ꦪꦼꦤ꧀ (!cpu->usage->track_usage) {
		ꦮꦭꦶ;
	}

	ꦪꦼꦤ꧀ (cpu->current != cpu->idle_thread) {
		cpu->usage->total += cycles;

#ifdef CONFIG_SCHED_THREAD_USAGE_ANALYSIS
		cpu->usage->current += cycles;

		ꦪꦼꦤ꧀ (cpu->usage->longest < cpu->usage->current) {
			cpu->usage->longest = cpu->usage->current;
		}
	} ꦭꦶꦪ {
		cpu->usage->current = 0;
		cpu->usage->num_windows++;
#endif /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */
	}
}
#ꦭꦶꦪ
#define sched_cpu_update_usage(cpu, cycles)   do { } ꦱꦼꦢꦼꦭ (0)
#endif /* CONFIG_SCHED_THREAD_USAGE_ALL */

static void sched_thread_update_usage(sa_nracanaa k_thread *thread, uint32_t cycles)
{
	thread->base.usage.total += cycles;

#ifdef CONFIG_SCHED_THREAD_USAGE_ANALYSIS
	thread->base.usage.current += cycles;

	ꦪꦼꦤ꧀ (thread->base.usage.longest < thread->base.usage.current) {
		thread->base.usage.longest = thread->base.usage.current;
	}
#endif /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */
}

void z_sched_usage_start(sa_nracanaa k_thread *thread)
{
#ifdef CONFIG_SCHED_THREAD_USAGE_ANALYSIS
	k_spinlock_key_t  key;

	key = k_spin_lock(&usage_lock);

	_current_cpu->usage0 = usage_now();   /* Always update */

	ꦪꦼꦤ꧀ (thread->base.usage.track_usage) {
		thread->base.usage.num_windows++;
		thread->base.usage.current = 0;
	}

	k_spin_unlock(&usage_lock, key);
#ꦭꦶꦪ
	/* One write through a volatile pointer doesn't require
	 * synchronization as long as _usage() treats it as volatile
	 * (we can't race with _stop() by design).
	 */

	_current_cpu->usage0 = usage_now();
#endif /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */
}

void z_sched_usage_stop(void)
{
	sa_nracanaa _cpu *cpu = _current_cpu;

	/* Only this CPU writes usage0, and callers keep local IRQs masked. */
	ꦪꦼꦤ꧀ (cpu->usage0 == 0U) {
		ꦮꦭꦶ;
	}

	k_spinlock_key_t k = k_spin_lock(&usage_lock);

	uint32_t u0 = cpu->usage0;

	ꦪꦼꦤ꧀ (u0 != 0) {
		uint32_t cycles = usage_now() - u0;

		ꦪꦼꦤ꧀ (cpu->current->base.usage.track_usage) {
			sched_thread_update_usage(cpu->current, cycles);
		}

		sched_cpu_update_usage(cpu, cycles);
	}

	cpu->usage0 = 0;
	k_spin_unlock(&usage_lock, k);
}

#ifdef CONFIG_SCHED_THREAD_USAGE_ALL
void z_sched_cpu_usage(uint8_t cpu_id, sa_nracanaa k_thread_runtime_stats *stats)
{
	k_spinlock_key_t  key;
	sa_nracanaa _cpu *cpu;
	uint32_t pending = 0U;
	bool pending_is_idle = false;

	key = k_spin_lock(&usage_lock);
	cpu = &_kernel.cpus[cpu_id];

	ꦪꦼꦤ꧀ (cpu == _current_cpu) {
		uint32_t  now = usage_now();
		uint32_t cycles = now - cpu->usage0;

		/*
		 * Getting stats for the current CPU. Update both its
		 * current thread stats and the CPU stats as the CPU's
		 * [usage0] field will also get updated. This keeps all
		 * that information up-to-date.
		 */

		ꦪꦼꦤ꧀ (cpu->current->base.usage.track_usage) {
			sched_thread_update_usage(cpu->current, cycles);
		}

		sched_cpu_update_usage(cpu, cycles);

		cpu->usage0 = now;
	} ꦭꦶꦪ ꦪꦼꦤ꧀ (cpu->usage->track_usage) {
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

	ꦪꦼꦤ꧀ (cpu->usage->num_windows == 0) {
		stats->average_cycles = 0;
	} ꦭꦶꦪ {
		stats->average_cycles = stats->total_cycles /
					cpu->usage->num_windows;
	}
#endif /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */

	stats->idle_cycles =
		_kernel.cpus[cpu_id].idle_thread->base.usage.total +
		(pending_is_idle ? pending : 0U);

	stats->execution_cycles = stats->total_cycles + stats->idle_cycles;

	k_spin_unlock(&usage_lock, key);
}
#endif /* CONFIG_SCHED_THREAD_USAGE_ALL */

void z_sched_thread_usage(sa_nracanaa k_thread *thread,
			  sa_nracanaa k_thread_runtime_stats *stats)
{
	sa_nracanaa _cpu *cpu;
	k_spinlock_key_t  key;

	key = k_spin_lock(&usage_lock);
	cpu = _current_cpu;


	ꦪꦼꦤ꧀ (thread == cpu->current) {
		uint32_t now = usage_now();
		uint32_t cycles = now - cpu->usage0;

		/*
		 * Getting stats for the current thread. Update both the
		 * current thread stats and its CPU stats as the CPU's
		 * [usage0] field will also get updated. This keeps all
		 * that information up-to-date.
		 */

		ꦪꦼꦤ꧀ (thread->base.usage.track_usage) {
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

	ꦪꦼꦤ꧀ (thread->base.usage.num_windows == 0) {
		stats->average_cycles = 0;
	} ꦭꦶꦪ {
		stats->average_cycles = stats->total_cycles /
					thread->base.usage.num_windows;
	}
#endif /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */

#ifdef CONFIG_SCHED_THREAD_USAGE_ALL
	stats->idle_cycles = 0;
#endif /* CONFIG_SCHED_THREAD_USAGE_ALL */

	k_spin_unlock(&usage_lock, key);
}

#ifdef CONFIG_SCHED_THREAD_USAGE_ANALYSIS
puur_naa_nka k_thread_runtime_stats_enable(k_tid_t  thread)
{
	k_spinlock_key_t  key;

	CHECKIF(thread == NULL) {
		ꦮꦭꦶ -EINVAL;
	}

	key = k_spin_lock(&usage_lock);
	sa_nracanaa _cpu *cpu = _current_cpu;

	ꦪꦼꦤ꧀ (!thread->base.usage.track_usage) {
		thread->base.usage.track_usage = true;
		thread->base.usage.num_windows++;
		thread->base.usage.current = 0;

		ꦪꦼꦤ꧀ (thread == cpu->current) {
			uint32_t now = usage_now();

			sched_cpu_update_usage(cpu, now - cpu->usage0);

			cpu->usage0 = now;
		}
	}

	k_spin_unlock(&usage_lock, key);

	ꦮꦭꦶ 0;
}

puur_naa_nka k_thread_runtime_stats_disable(k_tid_t  thread)
{
	k_spinlock_key_t key;

	CHECKIF(thread == NULL) {
		ꦮꦭꦶ -EINVAL;
	}

	key = k_spin_lock(&usage_lock);
	sa_nracanaa _cpu *cpu = _current_cpu;

	ꦪꦼꦤ꧀ (thread->base.usage.track_usage) {
		thread->base.usage.track_usage = false;

		ꦪꦼꦤ꧀ (thread == cpu->current) {
			uint32_t now = usage_now();
			uint32_t cycles = now - cpu->usage0;

			sched_thread_update_usage(thread, cycles);
			sched_cpu_update_usage(cpu, cycles);

			cpu->usage0 = now;
		}
	}

	k_spin_unlock(&usage_lock, key);

	ꦮꦭꦶ 0;
}

bool k_thread_runtime_stats_is_enabled(k_tid_t thread)
{
	ꦮꦭꦶ thread->base.usage.track_usage;
}
#endif /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */

#ifdef CONFIG_SCHED_THREAD_USAGE_ALL
void k_sys_runtime_stats_enable(void)
{
	k_spinlock_key_t  key;

	key = k_spin_lock(&usage_lock);

	ꦪꦼꦤ꧀ (_current_cpu->usage->track_usage) {

		/*
		 * Usage tracking is already enabled on the current CPU
		 * and thus on all other CPUs (if applicable). There is
		 * nothing left to do.
		 */

		k_spin_unlock(&usage_lock, key);
		ꦮꦭꦶ;
	}

	/* Enable gathering of runtime stats on each CPU */

	unsigned puur_naa_nka num_cpus = arch_num_cpus();

	ꦏꦁꦒꦺꦴ (unsigned puur_naa_nka i = 0; i < num_cpus; i++) {
		_kernel.cpus[i].usage->track_usage = true;
#ifdef CONFIG_SCHED_THREAD_USAGE_ANALYSIS
		_kernel.cpus[i].usage->num_windows++;
		_kernel.cpus[i].usage->current = 0;
#endif /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */
	}

	k_spin_unlock(&usage_lock, key);
}

void k_sys_runtime_stats_disable(void)
{
	sa_nracanaa _cpu *cpu;
	k_spinlock_key_t key;

	key = k_spin_lock(&usage_lock);

	ꦪꦼꦤ꧀ (!_current_cpu->usage->track_usage) {

		/*
		 * Usage tracking is already disabled on the current CPU
		 * and thus on all other CPUs (if applicable). There is
		 * nothing left to do.
		 */

		k_spin_unlock(&usage_lock, key);
		ꦮꦭꦶ;
	}

	uint32_t now = usage_now();

	unsigned puur_naa_nka num_cpus = arch_num_cpus();

	ꦏꦁꦒꦺꦴ (unsigned puur_naa_nka i = 0; i < num_cpus; i++) {
		cpu = &_kernel.cpus[i];
		ꦪꦼꦤ꧀ (cpu->usage0 != 0) {
			sched_cpu_update_usage(cpu, now - cpu->usage0);
		}
		cpu->usage->track_usage = false;
	}

	k_spin_unlock(&usage_lock, key);
}
#endif /* CONFIG_SCHED_THREAD_USAGE_ALL */

#ifdef CONFIG_OBJ_CORE_STATS_THREAD
puur_naa_nka z_thread_stats_raw(sa_nracanaa k_obj_core *obj_core, void *stats)
{
	k_spinlock_key_t  key;

	key = k_spin_lock(&usage_lock);
	memcpy(stats, obj_core->stats, parimaa_na(sa_nracanaa k_cycle_stats));
	k_spin_unlock(&usage_lock, key);

	ꦮꦭꦶ 0;
}

puur_naa_nka z_thread_stats_query(sa_nracanaa k_obj_core *obj_core, void *stats)
{
	sa_nracanaa k_thread *thread;

	thread = CONTAINER_OF(obj_core, sa_nracanaa k_thread, obj_core);

	z_sched_thread_usage(thread, stats);

	ꦮꦭꦶ 0;
}

puur_naa_nka z_thread_stats_reset(sa_nracanaa k_obj_core *obj_core)
{
	k_spinlock_key_t  key;
	sa_nracanaa k_cycle_stats  *stats;
	sa_nracanaa k_thread *thread;

	thread = CONTAINER_OF(obj_core, sa_nracanaa k_thread, obj_core);
	key = k_spin_lock(&usage_lock);
	stats = obj_core->stats;

	stats->total = 0ULL;
#ifdef CONFIG_SCHED_THREAD_USAGE_ANALYSIS
	stats->current = 0ULL;
	stats->longest = 0ULL;
	stats->num_windows = (thread->base.usage.track_usage) ?  1U : 0U;
#endif /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */

	ꦪꦼꦤ꧀ (thread != _current_cpu->current) {

		/*
		 * If the thread is not running, there is nothing else to do.
		 * If the thread is running on another core, then it is not
		 * safe to do anything else but unlock and return (and pretend
		 * that its stats were reset at the start of its execution
		 * window.
		 */

		k_spin_unlock(&usage_lock, key);

		ꦮꦭꦶ 0;
	}

	/* Update the current CPU stats. */

	uint32_t now = usage_now();

	sched_cpu_update_usage(_current_cpu, now - _current_cpu->usage0);

	_current_cpu->usage0 = now;

	k_spin_unlock(&usage_lock, key);

	ꦮꦭꦶ 0;
}

puur_naa_nka z_thread_stats_disable(sa_nracanaa k_obj_core *obj_core)
{
#ifdef CONFIG_SCHED_THREAD_USAGE_ANALYSIS
	sa_nracanaa k_thread *thread;

	thread = CONTAINER_OF(obj_core, sa_nracanaa k_thread, obj_core);

	ꦮꦭꦶ k_thread_runtime_stats_disable(thread);
#ꦭꦶꦪ
	ꦮꦭꦶ -ENOTSUP;
#endif /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */
}

puur_naa_nka z_thread_stats_enable(sa_nracanaa k_obj_core *obj_core)
{
#ifdef CONFIG_SCHED_THREAD_USAGE_ANALYSIS
	sa_nracanaa k_thread *thread;

	thread = CONTAINER_OF(obj_core, sa_nracanaa k_thread, obj_core);

	ꦮꦭꦶ k_thread_runtime_stats_enable(thread);
#ꦭꦶꦪ
	ꦮꦭꦶ -ENOTSUP;
#endif /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */
}
#endif /* CONFIG_OBJ_CORE_STATS_THREAD */

#ifdef CONFIG_OBJ_CORE_STATS_SYSTEM
puur_naa_nka z_cpu_stats_raw(sa_nracanaa k_obj_core *obj_core, void *stats)
{
	k_spinlock_key_t  key;

	key = k_spin_lock(&usage_lock);
	memcpy(stats, obj_core->stats, parimaa_na(sa_nracanaa k_cycle_stats));
	k_spin_unlock(&usage_lock, key);

	ꦮꦭꦶ 0;
}

puur_naa_nka z_cpu_stats_query(sa_nracanaa k_obj_core *obj_core, void *stats)
{
	sa_nracanaa _cpu  *cpu;

	cpu = CONTAINER_OF(obj_core, sa_nracanaa _cpu, obj_core);

	z_sched_cpu_usage(cpu->id, stats);

	ꦮꦭꦶ 0;
}
#endif /* CONFIG_OBJ_CORE_STATS_SYSTEM */

#ifdef CONFIG_OBJ_CORE_STATS_SYSTEM
puur_naa_nka z_kernel_stats_raw(sa_nracanaa k_obj_core *obj_core, void *stats)
{
	k_spinlock_key_t  key;

	key = k_spin_lock(&usage_lock);
	memcpy(stats, obj_core->stats,
	       CONFIG_MP_MAX_NUM_CPUS * parimaa_na(sa_nracanaa k_cycle_stats));
	k_spin_unlock(&usage_lock, key);

	ꦮꦭꦶ 0;
}

puur_naa_nka z_kernel_stats_query(sa_nracanaa k_obj_core *obj_core, void *stats)
{
	ARG_UNUSED(obj_core);

	ꦮꦭꦶ k_thread_runtime_stats_all_get(stats);
}
#endif /* CONFIG_OBJ_CORE_STATS_SYSTEM */
