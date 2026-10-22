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
#ಆದರೆ !defined(CONFIG_USE_SWITCH) && !defined(CONFIG_INSTRUMENT_THREAD_SWITCHING)
#ದೋಷ "No data backend configured for CONFIG_SCHED_THREAD_USAGE"
#ಆದರೆ_ಅಂತ್ಯ /* !CONFIG_USE_SWITCH && !CONFIG_INSTRUMENT_THREAD_SWITCHING */

ಸ್ಥಿರ ರಚನೆ k_spinlock usage_ಬೀಗ;

ಸ್ಥಿರ uint32_t usage_now(ಶೂನ್ಯ)
{
	uint32_t now;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_THREAD_RUNTIME_STATS_USE_TIMING_FUNCTIONS
	now = (uint32_t)timing_counter_get();
#ಇಲ್ಲದಿದ್ದರೆ
	now = k_ಚಕ್ರ_get_32();
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_THREAD_RUNTIME_STATS_USE_TIMING_FUNCTIONS */

	/* Edge case: we use a zero as a null ("stop() already called") */
	ಹಿಂತಿರುಗಿ (now == 0) ? 1 : now;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SCHED_THREAD_USAGE_ALL
ಸ್ಥಿರ ಶೂನ್ಯ sched_cpu_update_usage(ರಚನೆ _cpu *cpu, uint32_t ಚಕ್ರಗಳು)
{
	ಆದರೆ (!cpu->usage->track_usage) {
		ಹಿಂತಿರುಗಿ;
	}

	ಆದರೆ (cpu->current != cpu->idle_thread) {
		cpu->usage->ಒಟ್ಟು += ಚಕ್ರಗಳು;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SCHED_THREAD_USAGE_ANALYSIS
		cpu->usage->current += ಚಕ್ರಗಳು;

		ಆದರೆ (cpu->usage->longest < cpu->usage->current) {
			cpu->usage->longest = cpu->usage->current;
		}
	} ಇಲ್ಲದಿದ್ದರೆ {
		cpu->usage->current = 0;
		cpu->usage->ಅಂಕೆ_windows++;
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */
	}
}
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು sched_cpu_update_usage(cpu, ಚಕ್ರಗಳು)   ಮಾಡು { } ಆಗ (0)
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_SCHED_THREAD_USAGE_ALL */

ಸ್ಥಿರ ಶೂನ್ಯ sched_thread_update_usage(ರಚನೆ k_thread *thread, uint32_t ಚಕ್ರಗಳು)
{
	thread->ತಳ.usage.ಒಟ್ಟು += ಚಕ್ರಗಳು;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SCHED_THREAD_USAGE_ANALYSIS
	thread->ತಳ.usage.current += ಚಕ್ರಗಳು;

	ಆದರೆ (thread->ತಳ.usage.longest < thread->ತಳ.usage.current) {
		thread->ತಳ.usage.longest = thread->ತಳ.usage.current;
	}
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */
}

ಶೂನ್ಯ z_sched_usage_ಆರಂಭ(ರಚನೆ k_thread *thread)
{
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SCHED_THREAD_USAGE_ANALYSIS
	k_spinlock_ಕೀಲಿ_t  ಕೀಲಿ;

	ಕೀಲಿ = k_spin_ಬೀಗ(&usage_ಬೀಗ);

	_current_cpu->usage0 = usage_now();   /* Always update */

	ಆದರೆ (thread->ತಳ.usage.track_usage) {
		thread->ತಳ.usage.ಅಂಕೆ_windows++;
		thread->ತಳ.usage.current = 0;
	}

	k_spin_unlock(&usage_ಬೀಗ, ಕೀಲಿ);
#ಇಲ್ಲದಿದ್ದರೆ
	/* One write through a volatile pointer doesn't require
	 * synchronization as long as _usage() treats it as volatile
	 * (we can't race with _stop() by design).
	 */

	_current_cpu->usage0 = usage_now();
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */
}

ಶೂನ್ಯ z_sched_usage_stop(ಶೂನ್ಯ)
{
	ರಚನೆ _cpu *cpu = _current_cpu;

	/* Only this CPU writes usage0, and callers keep local IRQs masked. */
	ಆದರೆ (cpu->usage0 == 0U) {
		ಹಿಂತಿರುಗಿ;
	}

	k_spinlock_ಕೀಲಿ_t k = k_spin_ಬೀಗ(&usage_ಬೀಗ);

	uint32_t u0 = cpu->usage0;

	ಆದರೆ (u0 != 0) {
		uint32_t ಚಕ್ರಗಳು = usage_now() - u0;

		ಆದರೆ (cpu->current->ತಳ.usage.track_usage) {
			sched_thread_update_usage(cpu->current, ಚಕ್ರಗಳು);
		}

		sched_cpu_update_usage(cpu, ಚಕ್ರಗಳು);
	}

	cpu->usage0 = 0;
	k_spin_unlock(&usage_ಬೀಗ, k);
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SCHED_THREAD_USAGE_ALL
ಶೂನ್ಯ z_sched_cpu_usage(uint8_t cpu_id, ರಚನೆ k_thread_runtime_stats *stats)
{
	k_spinlock_ಕೀಲಿ_t  ಕೀಲಿ;
	ರಚನೆ _cpu *cpu;
	uint32_t pending = 0U;
	bool pending_is_idle = false;

	ಕೀಲಿ = k_spin_ಬೀಗ(&usage_ಬೀಗ);
	cpu = &_kernel.cpus[cpu_id];

	ಆದರೆ (cpu == _current_cpu) {
		uint32_t  now = usage_now();
		uint32_t ಚಕ್ರಗಳು = now - cpu->usage0;

		/*
		 * Getting stats for the current CPU. Update both its
		 * current thread stats and the CPU stats as the CPU's
		 * [usage0] field will also get updated. This keeps all
		 * that information up-to-date.
		 */

		ಆದರೆ (cpu->current->ತಳ.usage.track_usage) {
			sched_thread_update_usage(cpu->current, ಚಕ್ರಗಳು);
		}

		sched_cpu_update_usage(cpu, ಚಕ್ರಗಳು);

		cpu->usage0 = now;
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (cpu->usage->track_usage) {
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

	stats->ಒಟ್ಟು_ಚಕ್ರಗಳು     = cpu->usage->ಒಟ್ಟು +
				  (pending_is_idle ? 0U : pending);
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SCHED_THREAD_USAGE_ANALYSIS
	stats->current_ಚಕ್ರಗಳು   = cpu->usage->current;
	stats->peak_ಚಕ್ರಗಳು      = cpu->usage->longest;

	ಆದರೆ (cpu->usage->ಅಂಕೆ_windows == 0) {
		stats->ಸರಾಸರಿ_ಮೌಲ್ಯ_ಚಕ್ರಗಳು = 0;
	} ಇಲ್ಲದಿದ್ದರೆ {
		stats->ಸರಾಸರಿ_ಮೌಲ್ಯ_ಚಕ್ರಗಳು = stats->ಒಟ್ಟು_ಚಕ್ರಗಳು /
					cpu->usage->ಅಂಕೆ_windows;
	}
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */

	stats->idle_ಚಕ್ರಗಳು =
		_kernel.cpus[cpu_id].idle_thread->ತಳ.usage.ಒಟ್ಟು +
		(pending_is_idle ? pending : 0U);

	stats->execution_ಚಕ್ರಗಳು = stats->ಒಟ್ಟು_ಚಕ್ರಗಳು + stats->idle_ಚಕ್ರಗಳು;

	k_spin_unlock(&usage_ಬೀಗ, ಕೀಲಿ);
}
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_SCHED_THREAD_USAGE_ALL */

ಶೂನ್ಯ z_sched_thread_usage(ರಚನೆ k_thread *thread,
			  ರಚನೆ k_thread_runtime_stats *stats)
{
	ರಚನೆ _cpu *cpu;
	k_spinlock_ಕೀಲಿ_t  ಕೀಲಿ;

	ಕೀಲಿ = k_spin_ಬೀಗ(&usage_ಬೀಗ);
	cpu = _current_cpu;


	ಆದರೆ (thread == cpu->current) {
		uint32_t now = usage_now();
		uint32_t ಚಕ್ರಗಳು = now - cpu->usage0;

		/*
		 * Getting stats for the current thread. Update both the
		 * current thread stats and its CPU stats as the CPU's
		 * [usage0] field will also get updated. This keeps all
		 * that information up-to-date.
		 */

		ಆದರೆ (thread->ತಳ.usage.track_usage) {
			sched_thread_update_usage(thread, ಚಕ್ರಗಳು);
		}

		sched_cpu_update_usage(cpu, ಚಕ್ರಗಳು);

		cpu->usage0 = now;
	}

	stats->execution_ಚಕ್ರಗಳು = thread->ತಳ.usage.ಒಟ್ಟು;
	stats->ಒಟ್ಟು_ಚಕ್ರಗಳು     = thread->ತಳ.usage.ಒಟ್ಟು;

	/* Copy-out the thread's usage stats */

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SCHED_THREAD_USAGE_ANALYSIS
	stats->current_ಚಕ್ರಗಳು = thread->ತಳ.usage.current;
	stats->peak_ಚಕ್ರಗಳು    = thread->ತಳ.usage.longest;

	ಆದರೆ (thread->ತಳ.usage.ಅಂಕೆ_windows == 0) {
		stats->ಸರಾಸರಿ_ಮೌಲ್ಯ_ಚಕ್ರಗಳು = 0;
	} ಇಲ್ಲದಿದ್ದರೆ {
		stats->ಸರಾಸರಿ_ಮೌಲ್ಯ_ಚಕ್ರಗಳು = stats->ಒಟ್ಟು_ಚಕ್ರಗಳು /
					thread->ತಳ.usage.ಅಂಕೆ_windows;
	}
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SCHED_THREAD_USAGE_ALL
	stats->idle_ಚಕ್ರಗಳು = 0;
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_SCHED_THREAD_USAGE_ALL */

	k_spin_unlock(&usage_ಬೀಗ, ಕೀಲಿ);
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SCHED_THREAD_USAGE_ANALYSIS
ಪೂರ್ಣಾಂಕ k_thread_runtime_stats_enable(k_tid_t  thread)
{
	k_spinlock_ಕೀಲಿ_t  ಕೀಲಿ;

	CHECKIF(thread == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ -EINVAL;
	}

	ಕೀಲಿ = k_spin_ಬೀಗ(&usage_ಬೀಗ);
	ರಚನೆ _cpu *cpu = _current_cpu;

	ಆದರೆ (!thread->ತಳ.usage.track_usage) {
		thread->ತಳ.usage.track_usage = true;
		thread->ತಳ.usage.ಅಂಕೆ_windows++;
		thread->ತಳ.usage.current = 0;

		ಆದರೆ (thread == cpu->current) {
			uint32_t now = usage_now();

			sched_cpu_update_usage(cpu, now - cpu->usage0);

			cpu->usage0 = now;
		}
	}

	k_spin_unlock(&usage_ಬೀಗ, ಕೀಲಿ);

	ಹಿಂತಿರುಗಿ 0;
}

ಪೂರ್ಣಾಂಕ k_thread_runtime_stats_disable(k_tid_t  thread)
{
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ;

	CHECKIF(thread == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ -EINVAL;
	}

	ಕೀಲಿ = k_spin_ಬೀಗ(&usage_ಬೀಗ);
	ರಚನೆ _cpu *cpu = _current_cpu;

	ಆದರೆ (thread->ತಳ.usage.track_usage) {
		thread->ತಳ.usage.track_usage = false;

		ಆದರೆ (thread == cpu->current) {
			uint32_t now = usage_now();
			uint32_t ಚಕ್ರಗಳು = now - cpu->usage0;

			sched_thread_update_usage(thread, ಚಕ್ರಗಳು);
			sched_cpu_update_usage(cpu, ಚಕ್ರಗಳು);

			cpu->usage0 = now;
		}
	}

	k_spin_unlock(&usage_ಬೀಗ, ಕೀಲಿ);

	ಹಿಂತಿರುಗಿ 0;
}

bool k_thread_runtime_stats_is_enabled(k_tid_t thread)
{
	ಹಿಂತಿರುಗಿ thread->ತಳ.usage.track_usage;
}
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SCHED_THREAD_USAGE_ALL
ಶೂನ್ಯ k_sys_runtime_stats_enable(ಶೂನ್ಯ)
{
	k_spinlock_ಕೀಲಿ_t  ಕೀಲಿ;

	ಕೀಲಿ = k_spin_ಬೀಗ(&usage_ಬೀಗ);

	ಆದರೆ (_current_cpu->usage->track_usage) {

		/*
		 * Usage tracking is already enabled on the current CPU
		 * and thus on all other CPUs (if applicable). There is
		 * nothing left to do.
		 */

		k_spin_unlock(&usage_ಬೀಗ, ಕೀಲಿ);
		ಹಿಂತಿರುಗಿ;
	}

	/* Enable gathering of runtime stats on each CPU */

	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಅಂಕೆ_cpus = arch_ಅಂಕೆ_cpus();

	ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ i = 0; i < ಅಂಕೆ_cpus; i++) {
		_kernel.cpus[i].usage->track_usage = true;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SCHED_THREAD_USAGE_ANALYSIS
		_kernel.cpus[i].usage->ಅಂಕೆ_windows++;
		_kernel.cpus[i].usage->current = 0;
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */
	}

	k_spin_unlock(&usage_ಬೀಗ, ಕೀಲಿ);
}

ಶೂನ್ಯ k_sys_runtime_stats_disable(ಶೂನ್ಯ)
{
	ರಚನೆ _cpu *cpu;
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ;

	ಕೀಲಿ = k_spin_ಬೀಗ(&usage_ಬೀಗ);

	ಆದರೆ (!_current_cpu->usage->track_usage) {

		/*
		 * Usage tracking is already disabled on the current CPU
		 * and thus on all other CPUs (if applicable). There is
		 * nothing left to do.
		 */

		k_spin_unlock(&usage_ಬೀಗ, ಕೀಲಿ);
		ಹಿಂತಿರುಗಿ;
	}

	uint32_t now = usage_now();

	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಅಂಕೆ_cpus = arch_ಅಂಕೆ_cpus();

	ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ i = 0; i < ಅಂಕೆ_cpus; i++) {
		cpu = &_kernel.cpus[i];
		ಆದರೆ (cpu->usage0 != 0) {
			sched_cpu_update_usage(cpu, now - cpu->usage0);
		}
		cpu->usage->track_usage = false;
	}

	k_spin_unlock(&usage_ಬೀಗ, ಕೀಲಿ);
}
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_SCHED_THREAD_USAGE_ALL */

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_OBJ_CORE_STATS_THREAD
ಪೂರ್ಣಾಂಕ z_thread_stats_raw(ರಚನೆ k_obj_core *obj_core, ಶೂನ್ಯ *stats)
{
	k_spinlock_ಕೀಲಿ_t  ಕೀಲಿ;

	ಕೀಲಿ = k_spin_ಬೀಗ(&usage_ಬೀಗ);
	ಸ್ಮರಣೆ_ನಕಲು(stats, obj_core->stats, ಗಾತ್ರ(ರಚನೆ k_ಚಕ್ರ_stats));
	k_spin_unlock(&usage_ಬೀಗ, ಕೀಲಿ);

	ಹಿಂತಿರುಗಿ 0;
}

ಪೂರ್ಣಾಂಕ z_thread_stats_query(ರಚನೆ k_obj_core *obj_core, ಶೂನ್ಯ *stats)
{
	ರಚನೆ k_thread *thread;

	thread = CONTAINER_ದ(obj_core, ರಚನೆ k_thread, obj_core);

	z_sched_thread_usage(thread, stats);

	ಹಿಂತಿರುಗಿ 0;
}

ಪೂರ್ಣಾಂಕ z_thread_stats_reset(ರಚನೆ k_obj_core *obj_core)
{
	k_spinlock_ಕೀಲಿ_t  ಕೀಲಿ;
	ರಚನೆ k_ಚಕ್ರ_stats  *stats;
	ರಚನೆ k_thread *thread;

	thread = CONTAINER_ದ(obj_core, ರಚನೆ k_thread, obj_core);
	ಕೀಲಿ = k_spin_ಬೀಗ(&usage_ಬೀಗ);
	stats = obj_core->stats;

	stats->ಒಟ್ಟು = 0ULL;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SCHED_THREAD_USAGE_ANALYSIS
	stats->current = 0ULL;
	stats->longest = 0ULL;
	stats->ಅಂಕೆ_windows = (thread->ತಳ.usage.track_usage) ?  1U : 0U;
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */

	ಆದರೆ (thread != _current_cpu->current) {

		/*
		 * If the thread is not running, there is nothing else to do.
		 * If the thread is running on another core, then it is not
		 * safe to do anything else but unlock and return (and pretend
		 * that its stats were reset at the start of its execution
		 * window.
		 */

		k_spin_unlock(&usage_ಬೀಗ, ಕೀಲಿ);

		ಹಿಂತಿರುಗಿ 0;
	}

	/* Update the current CPU stats. */

	uint32_t now = usage_now();

	sched_cpu_update_usage(_current_cpu, now - _current_cpu->usage0);

	_current_cpu->usage0 = now;

	k_spin_unlock(&usage_ಬೀಗ, ಕೀಲಿ);

	ಹಿಂತಿರುಗಿ 0;
}

ಪೂರ್ಣಾಂಕ z_thread_stats_disable(ರಚನೆ k_obj_core *obj_core)
{
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SCHED_THREAD_USAGE_ANALYSIS
	ರಚನೆ k_thread *thread;

	thread = CONTAINER_ದ(obj_core, ರಚನೆ k_thread, obj_core);

	ಹಿಂತಿರುಗಿ k_thread_runtime_stats_disable(thread);
#ಇಲ್ಲದಿದ್ದರೆ
	ಹಿಂತಿರುಗಿ -ENOTSUP;
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */
}

ಪೂರ್ಣಾಂಕ z_thread_stats_enable(ರಚನೆ k_obj_core *obj_core)
{
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SCHED_THREAD_USAGE_ANALYSIS
	ರಚನೆ k_thread *thread;

	thread = CONTAINER_ದ(obj_core, ರಚನೆ k_thread, obj_core);

	ಹಿಂತಿರುಗಿ k_thread_runtime_stats_enable(thread);
#ಇಲ್ಲದಿದ್ದರೆ
	ಹಿಂತಿರುಗಿ -ENOTSUP;
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */
}
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_OBJ_CORE_STATS_THREAD */

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_OBJ_CORE_STATS_SYSTEM
ಪೂರ್ಣಾಂಕ z_cpu_stats_raw(ರಚನೆ k_obj_core *obj_core, ಶೂನ್ಯ *stats)
{
	k_spinlock_ಕೀಲಿ_t  ಕೀಲಿ;

	ಕೀಲಿ = k_spin_ಬೀಗ(&usage_ಬೀಗ);
	ಸ್ಮರಣೆ_ನಕಲು(stats, obj_core->stats, ಗಾತ್ರ(ರಚನೆ k_ಚಕ್ರ_stats));
	k_spin_unlock(&usage_ಬೀಗ, ಕೀಲಿ);

	ಹಿಂತಿರುಗಿ 0;
}

ಪೂರ್ಣಾಂಕ z_cpu_stats_query(ರಚನೆ k_obj_core *obj_core, ಶೂನ್ಯ *stats)
{
	ರಚನೆ _cpu  *cpu;

	cpu = CONTAINER_ದ(obj_core, ರಚನೆ _cpu, obj_core);

	z_sched_cpu_usage(cpu->id, stats);

	ಹಿಂತಿರುಗಿ 0;
}
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_OBJ_CORE_STATS_SYSTEM */

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_OBJ_CORE_STATS_SYSTEM
ಪೂರ್ಣಾಂಕ z_kernel_stats_raw(ರಚನೆ k_obj_core *obj_core, ಶೂನ್ಯ *stats)
{
	k_spinlock_ಕೀಲಿ_t  ಕೀಲಿ;

	ಕೀಲಿ = k_spin_ಬೀಗ(&usage_ಬೀಗ);
	ಸ್ಮರಣೆ_ನಕಲು(stats, obj_core->stats,
	       CONFIG_MP_MAX_ಅಂಕೆ_CPUS * ಗಾತ್ರ(ರಚನೆ k_ಚಕ್ರ_stats));
	k_spin_unlock(&usage_ಬೀಗ, ಕೀಲಿ);

	ಹಿಂತಿರುಗಿ 0;
}

ಪೂರ್ಣಾಂಕ z_kernel_stats_query(ರಚನೆ k_obj_core *obj_core, ಶೂನ್ಯ *stats)
{
	ARG_ಬಳಸದ(obj_core);

	ಹಿಂತಿರುಗಿ k_thread_runtime_stats_all_get(stats);
}
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_OBJ_CORE_STATS_SYSTEM */
