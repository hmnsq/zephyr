/*
 * Copyright (c) 2018 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#समावेश <zephyr/kernel.h>

#समावेश <zephyr/timing/timing.h>
#समावेश <ksched.h>
#समावेश <zephyr/spinlock.h>
#समावेश <zephyr/sys/check.h>

/* Need one of these for this to work */
#अगर !defined(CONFIG_USE_SWITCH) && !defined(CONFIG_INSTRUMENT_THREAD_SWITCHING)
#त्रुटि "No data backend configured for CONFIG_SCHED_THREAD_USAGE"
#पूर्ण_अगर /* !CONFIG_USE_SWITCH && !CONFIG_INSTRUMENT_THREAD_SWITCHING */

अटल काष्ठा k_spinlock usage_lock;

अटल uint32_t usage_now(व्योम)
{
	uint32_t now;

#अगर_घोषित CONFIG_THREAD_RUNTIME_STATS_USE_TIMING_FUNCTIONS
	now = (uint32_t)timing_गणक_get();
#अन्यथा
	now = k_cycle_get_32();
#पूर्ण_अगर /* CONFIG_THREAD_RUNTIME_STATS_USE_TIMING_FUNCTIONS */

	/* Edge case: we use a zero as a null ("stop() already called") */
	वापस (now == 0) ? 1 : now;
}

#अगर_घोषित CONFIG_SCHED_THREAD_USAGE_ALL
अटल व्योम sched_cpu_update_usage(काष्ठा _cpu *cpu, uint32_t cycles)
{
	अगर (!cpu->usage->track_usage) {
		वापस;
	}

	अगर (cpu->current != cpu->idle_thread) {
		cpu->usage->योग += cycles;

#अगर_घोषित CONFIG_SCHED_THREAD_USAGE_ANALYSIS
		cpu->usage->current += cycles;

		अगर (cpu->usage->longest < cpu->usage->current) {
			cpu->usage->longest = cpu->usage->current;
		}
	} अन्यथा {
		cpu->usage->current = 0;
		cpu->usage->अंक_windows++;
#पूर्ण_अगर /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */
	}
}
#अन्यथा
#घोषणा sched_cpu_update_usage(cpu, cycles)   करो { } जबतक (0)
#पूर्ण_अगर /* CONFIG_SCHED_THREAD_USAGE_ALL */

अटल व्योम sched_thread_update_usage(काष्ठा k_thread *thread, uint32_t cycles)
{
	thread->base.usage.योग += cycles;

#अगर_घोषित CONFIG_SCHED_THREAD_USAGE_ANALYSIS
	thread->base.usage.current += cycles;

	अगर (thread->base.usage.longest < thread->base.usage.current) {
		thread->base.usage.longest = thread->base.usage.current;
	}
#पूर्ण_अगर /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */
}

व्योम z_sched_usage_start(काष्ठा k_thread *thread)
{
#अगर_घोषित CONFIG_SCHED_THREAD_USAGE_ANALYSIS
	k_spinlock_कुंजी_t  कुंजी;

	कुंजी = k_spin_lock(&usage_lock);

	_current_cpu->usage0 = usage_now();   /* Always update */

	अगर (thread->base.usage.track_usage) {
		thread->base.usage.अंक_windows++;
		thread->base.usage.current = 0;
	}

	k_spin_unlock(&usage_lock, कुंजी);
#अन्यथा
	/* One write through a volatile pointer doesn't require
	 * synchronization as long as _usage() treats it as volatile
	 * (we can't race with _stop() by design).
	 */

	_current_cpu->usage0 = usage_now();
#पूर्ण_अगर /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */
}

व्योम z_sched_usage_stop(व्योम)
{
	काष्ठा _cpu *cpu = _current_cpu;

	/* Only this CPU writes usage0, and callers keep local IRQs masked. */
	अगर (cpu->usage0 == 0U) {
		वापस;
	}

	k_spinlock_कुंजी_t k = k_spin_lock(&usage_lock);

	uint32_t u0 = cpu->usage0;

	अगर (u0 != 0) {
		uint32_t cycles = usage_now() - u0;

		अगर (cpu->current->base.usage.track_usage) {
			sched_thread_update_usage(cpu->current, cycles);
		}

		sched_cpu_update_usage(cpu, cycles);
	}

	cpu->usage0 = 0;
	k_spin_unlock(&usage_lock, k);
}

#अगर_घोषित CONFIG_SCHED_THREAD_USAGE_ALL
व्योम z_sched_cpu_usage(uint8_t cpu_id, काष्ठा k_thread_runtime_stats *stats)
{
	k_spinlock_कुंजी_t  कुंजी;
	काष्ठा _cpu *cpu;
	uint32_t pending = 0U;
	bool pending_is_idle = false;

	कुंजी = k_spin_lock(&usage_lock);
	cpu = &_kernel.cpus[cpu_id];

	अगर (cpu == _current_cpu) {
		uint32_t  now = usage_now();
		uint32_t cycles = now - cpu->usage0;

		/*
		 * Getting stats for the current CPU. Update both its
		 * current thread stats and the CPU stats as the CPU's
		 * [usage0] field will also get updated. This keeps all
		 * that information up-to-date.
		 */

		अगर (cpu->current->base.usage.track_usage) {
			sched_thread_update_usage(cpu->current, cycles);
		}

		sched_cpu_update_usage(cpu, cycles);

		cpu->usage0 = now;
	} अन्यथा अगर (cpu->usage->track_usage) {
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

	stats->योग_cycles     = cpu->usage->योग +
				  (pending_is_idle ? 0U : pending);
#अगर_घोषित CONFIG_SCHED_THREAD_USAGE_ANALYSIS
	stats->current_cycles   = cpu->usage->current;
	stats->peak_cycles      = cpu->usage->longest;

	अगर (cpu->usage->अंक_windows == 0) {
		stats->average_cycles = 0;
	} अन्यथा {
		stats->average_cycles = stats->योग_cycles /
					cpu->usage->अंक_windows;
	}
#पूर्ण_अगर /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */

	stats->idle_cycles =
		_kernel.cpus[cpu_id].idle_thread->base.usage.योग +
		(pending_is_idle ? pending : 0U);

	stats->execution_cycles = stats->योग_cycles + stats->idle_cycles;

	k_spin_unlock(&usage_lock, कुंजी);
}
#पूर्ण_अगर /* CONFIG_SCHED_THREAD_USAGE_ALL */

व्योम z_sched_thread_usage(काष्ठा k_thread *thread,
			  काष्ठा k_thread_runtime_stats *stats)
{
	काष्ठा _cpu *cpu;
	k_spinlock_कुंजी_t  कुंजी;

	कुंजी = k_spin_lock(&usage_lock);
	cpu = _current_cpu;


	अगर (thread == cpu->current) {
		uint32_t now = usage_now();
		uint32_t cycles = now - cpu->usage0;

		/*
		 * Getting stats for the current thread. Update both the
		 * current thread stats and its CPU stats as the CPU's
		 * [usage0] field will also get updated. This keeps all
		 * that information up-to-date.
		 */

		अगर (thread->base.usage.track_usage) {
			sched_thread_update_usage(thread, cycles);
		}

		sched_cpu_update_usage(cpu, cycles);

		cpu->usage0 = now;
	}

	stats->execution_cycles = thread->base.usage.योग;
	stats->योग_cycles     = thread->base.usage.योग;

	/* Copy-out the thread's usage stats */

#अगर_घोषित CONFIG_SCHED_THREAD_USAGE_ANALYSIS
	stats->current_cycles = thread->base.usage.current;
	stats->peak_cycles    = thread->base.usage.longest;

	अगर (thread->base.usage.अंक_windows == 0) {
		stats->average_cycles = 0;
	} अन्यथा {
		stats->average_cycles = stats->योग_cycles /
					thread->base.usage.अंक_windows;
	}
#पूर्ण_अगर /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */

#अगर_घोषित CONFIG_SCHED_THREAD_USAGE_ALL
	stats->idle_cycles = 0;
#पूर्ण_अगर /* CONFIG_SCHED_THREAD_USAGE_ALL */

	k_spin_unlock(&usage_lock, कुंजी);
}

#अगर_घोषित CONFIG_SCHED_THREAD_USAGE_ANALYSIS
पूर्णांक k_thread_runtime_stats_enable(k_tid_t  thread)
{
	k_spinlock_कुंजी_t  कुंजी;

	CHECKIF(thread == शून्य) {
		वापस -EINVAL;
	}

	कुंजी = k_spin_lock(&usage_lock);
	काष्ठा _cpu *cpu = _current_cpu;

	अगर (!thread->base.usage.track_usage) {
		thread->base.usage.track_usage = true;
		thread->base.usage.अंक_windows++;
		thread->base.usage.current = 0;

		अगर (thread == cpu->current) {
			uint32_t now = usage_now();

			sched_cpu_update_usage(cpu, now - cpu->usage0);

			cpu->usage0 = now;
		}
	}

	k_spin_unlock(&usage_lock, कुंजी);

	वापस 0;
}

पूर्णांक k_thread_runtime_stats_disable(k_tid_t  thread)
{
	k_spinlock_कुंजी_t कुंजी;

	CHECKIF(thread == शून्य) {
		वापस -EINVAL;
	}

	कुंजी = k_spin_lock(&usage_lock);
	काष्ठा _cpu *cpu = _current_cpu;

	अगर (thread->base.usage.track_usage) {
		thread->base.usage.track_usage = false;

		अगर (thread == cpu->current) {
			uint32_t now = usage_now();
			uint32_t cycles = now - cpu->usage0;

			sched_thread_update_usage(thread, cycles);
			sched_cpu_update_usage(cpu, cycles);

			cpu->usage0 = now;
		}
	}

	k_spin_unlock(&usage_lock, कुंजी);

	वापस 0;
}

bool k_thread_runtime_stats_is_enabled(k_tid_t thread)
{
	वापस thread->base.usage.track_usage;
}
#पूर्ण_अगर /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */

#अगर_घोषित CONFIG_SCHED_THREAD_USAGE_ALL
व्योम k_sys_runtime_stats_enable(व्योम)
{
	k_spinlock_कुंजी_t  कुंजी;

	कुंजी = k_spin_lock(&usage_lock);

	अगर (_current_cpu->usage->track_usage) {

		/*
		 * Usage tracking is already enabled on the current CPU
		 * and thus on all other CPUs (if applicable). There is
		 * nothing left to do.
		 */

		k_spin_unlock(&usage_lock, कुंजी);
		वापस;
	}

	/* Enable gathering of runtime stats on each CPU */

	अचिन्हित पूर्णांक अंक_cpus = arch_अंक_cpus();

	क्रम (अचिन्हित पूर्णांक i = 0; i < अंक_cpus; i++) {
		_kernel.cpus[i].usage->track_usage = true;
#अगर_घोषित CONFIG_SCHED_THREAD_USAGE_ANALYSIS
		_kernel.cpus[i].usage->अंक_windows++;
		_kernel.cpus[i].usage->current = 0;
#पूर्ण_अगर /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */
	}

	k_spin_unlock(&usage_lock, कुंजी);
}

व्योम k_sys_runtime_stats_disable(व्योम)
{
	काष्ठा _cpu *cpu;
	k_spinlock_कुंजी_t कुंजी;

	कुंजी = k_spin_lock(&usage_lock);

	अगर (!_current_cpu->usage->track_usage) {

		/*
		 * Usage tracking is already disabled on the current CPU
		 * and thus on all other CPUs (if applicable). There is
		 * nothing left to do.
		 */

		k_spin_unlock(&usage_lock, कुंजी);
		वापस;
	}

	uint32_t now = usage_now();

	अचिन्हित पूर्णांक अंक_cpus = arch_अंक_cpus();

	क्रम (अचिन्हित पूर्णांक i = 0; i < अंक_cpus; i++) {
		cpu = &_kernel.cpus[i];
		अगर (cpu->usage0 != 0) {
			sched_cpu_update_usage(cpu, now - cpu->usage0);
		}
		cpu->usage->track_usage = false;
	}

	k_spin_unlock(&usage_lock, कुंजी);
}
#पूर्ण_अगर /* CONFIG_SCHED_THREAD_USAGE_ALL */

#अगर_घोषित CONFIG_OBJ_CORE_STATS_THREAD
पूर्णांक z_thread_stats_raw(काष्ठा k_obj_core *obj_core, व्योम *stats)
{
	k_spinlock_कुंजी_t  कुंजी;

	कुंजी = k_spin_lock(&usage_lock);
	स_नकल(stats, obj_core->stats, माप(काष्ठा k_cycle_stats));
	k_spin_unlock(&usage_lock, कुंजी);

	वापस 0;
}

पूर्णांक z_thread_stats_query(काष्ठा k_obj_core *obj_core, व्योम *stats)
{
	काष्ठा k_thread *thread;

	thread = CONTAINER_OF(obj_core, काष्ठा k_thread, obj_core);

	z_sched_thread_usage(thread, stats);

	वापस 0;
}

पूर्णांक z_thread_stats_reset(काष्ठा k_obj_core *obj_core)
{
	k_spinlock_कुंजी_t  कुंजी;
	काष्ठा k_cycle_stats  *stats;
	काष्ठा k_thread *thread;

	thread = CONTAINER_OF(obj_core, काष्ठा k_thread, obj_core);
	कुंजी = k_spin_lock(&usage_lock);
	stats = obj_core->stats;

	stats->योग = 0ULL;
#अगर_घोषित CONFIG_SCHED_THREAD_USAGE_ANALYSIS
	stats->current = 0ULL;
	stats->longest = 0ULL;
	stats->अंक_windows = (thread->base.usage.track_usage) ?  1U : 0U;
#पूर्ण_अगर /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */

	अगर (thread != _current_cpu->current) {

		/*
		 * If the thread is not running, there is nothing else to do.
		 * If the thread is running on another core, then it is not
		 * safe to do anything else but unlock and return (and pretend
		 * that its stats were reset at the start of its execution
		 * window.
		 */

		k_spin_unlock(&usage_lock, कुंजी);

		वापस 0;
	}

	/* Update the current CPU stats. */

	uint32_t now = usage_now();

	sched_cpu_update_usage(_current_cpu, now - _current_cpu->usage0);

	_current_cpu->usage0 = now;

	k_spin_unlock(&usage_lock, कुंजी);

	वापस 0;
}

पूर्णांक z_thread_stats_disable(काष्ठा k_obj_core *obj_core)
{
#अगर_घोषित CONFIG_SCHED_THREAD_USAGE_ANALYSIS
	काष्ठा k_thread *thread;

	thread = CONTAINER_OF(obj_core, काष्ठा k_thread, obj_core);

	वापस k_thread_runtime_stats_disable(thread);
#अन्यथा
	वापस -ENOTSUP;
#पूर्ण_अगर /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */
}

पूर्णांक z_thread_stats_enable(काष्ठा k_obj_core *obj_core)
{
#अगर_घोषित CONFIG_SCHED_THREAD_USAGE_ANALYSIS
	काष्ठा k_thread *thread;

	thread = CONTAINER_OF(obj_core, काष्ठा k_thread, obj_core);

	वापस k_thread_runtime_stats_enable(thread);
#अन्यथा
	वापस -ENOTSUP;
#पूर्ण_अगर /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */
}
#पूर्ण_अगर /* CONFIG_OBJ_CORE_STATS_THREAD */

#अगर_घोषित CONFIG_OBJ_CORE_STATS_SYSTEM
पूर्णांक z_cpu_stats_raw(काष्ठा k_obj_core *obj_core, व्योम *stats)
{
	k_spinlock_कुंजी_t  कुंजी;

	कुंजी = k_spin_lock(&usage_lock);
	स_नकल(stats, obj_core->stats, माप(काष्ठा k_cycle_stats));
	k_spin_unlock(&usage_lock, कुंजी);

	वापस 0;
}

पूर्णांक z_cpu_stats_query(काष्ठा k_obj_core *obj_core, व्योम *stats)
{
	काष्ठा _cpu  *cpu;

	cpu = CONTAINER_OF(obj_core, काष्ठा _cpu, obj_core);

	z_sched_cpu_usage(cpu->id, stats);

	वापस 0;
}
#पूर्ण_अगर /* CONFIG_OBJ_CORE_STATS_SYSTEM */

#अगर_घोषित CONFIG_OBJ_CORE_STATS_SYSTEM
पूर्णांक z_kernel_stats_raw(काष्ठा k_obj_core *obj_core, व्योम *stats)
{
	k_spinlock_कुंजी_t  कुंजी;

	कुंजी = k_spin_lock(&usage_lock);
	स_नकल(stats, obj_core->stats,
	       CONFIG_MP_MAX_अंक_CPUS * माप(काष्ठा k_cycle_stats));
	k_spin_unlock(&usage_lock, कुंजी);

	वापस 0;
}

पूर्णांक z_kernel_stats_query(काष्ठा k_obj_core *obj_core, व्योम *stats)
{
	ARG_UNUSED(obj_core);

	वापस k_thread_runtime_stats_all_get(stats);
}
#पूर्ण_अगर /* CONFIG_OBJ_CORE_STATS_SYSTEM */
