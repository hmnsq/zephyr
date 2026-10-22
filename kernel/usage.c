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
#दोषः "No data backend configured for CONFIG_SCHED_THREAD_USAGE"
#यदि_अन्तः /* !CONFIG_USE_SWITCH && !CONFIG_INSTRUMENT_THREAD_SWITCHING */

स्थिर संरचना k_spinlock usage_तालकम्;

स्थिर uint32_t usage_now(शून्य)
{
	uint32_t now;

#यदि_परिभाषितम् CONFIG_THREAD_RUNTIME_STATS_USE_TIMING_FUNCTIONS
	now = (uint32_t)timing_गणक_get();
#अन्यथा
	now = k_आवृत्तिः_get_32();
#यदि_अन्तः /* CONFIG_THREAD_RUNTIME_STATS_USE_TIMING_FUNCTIONS */

	/* Edge case: we use a zero as a null ("stop() already called") */
	प्रत्यागम (now == 0) ? 1 : now;
}

#यदि_परिभाषितम् CONFIG_SCHED_THREAD_USAGE_ALL
स्थिर शून्य sched_cpu_update_usage(संरचना _cpu *cpu, uint32_t आवृत्तयः)
{
	यदि (!cpu->usage->track_usage) {
		प्रत्यागम;
	}

	यदि (cpu->current != cpu->idle_thread) {
		cpu->usage->योग += आवृत्तयः;

#यदि_परिभाषितम् CONFIG_SCHED_THREAD_USAGE_ANALYSIS
		cpu->usage->current += आवृत्तयः;

		यदि (cpu->usage->longest < cpu->usage->current) {
			cpu->usage->longest = cpu->usage->current;
		}
	} अन्यथा {
		cpu->usage->current = 0;
		cpu->usage->अङ्कः_windows++;
#यदि_अन्तः /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */
	}
}
#अन्यथा
#परिभाषय sched_cpu_update_usage(cpu, आवृत्तयः)   कुरु { } यावत् (0)
#यदि_अन्तः /* CONFIG_SCHED_THREAD_USAGE_ALL */

स्थिर शून्य sched_thread_update_usage(संरचना k_thread *thread, uint32_t आवृत्तयः)
{
	thread->आधारः.usage.योग += आवृत्तयः;

#यदि_परिभाषितम् CONFIG_SCHED_THREAD_USAGE_ANALYSIS
	thread->आधारः.usage.current += आवृत्तयः;

	यदि (thread->आधारः.usage.longest < thread->आधारः.usage.current) {
		thread->आधारः.usage.longest = thread->आधारः.usage.current;
	}
#यदि_अन्तः /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */
}

शून्य z_sched_usage_start(संरचना k_thread *thread)
{
#यदि_परिभाषितम् CONFIG_SCHED_THREAD_USAGE_ANALYSIS
	k_spinlock_कुञ्चिका_t  कुञ्चिका;

	कुञ्चिका = k_spin_तालकम्(&usage_तालकम्);

	_current_cpu->usage0 = usage_now();   /* Always update */

	यदि (thread->आधारः.usage.track_usage) {
		thread->आधारः.usage.अङ्कः_windows++;
		thread->आधारः.usage.current = 0;
	}

	k_spin_unlock(&usage_तालकम्, कुञ्चिका);
#अन्यथा
	/* One write through a volatile pointer doesn't require
	 * synchronization as long as _usage() treats it as volatile
	 * (we can't race with _stop() by design).
	 */

	_current_cpu->usage0 = usage_now();
#यदि_अन्तः /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */
}

शून्य z_sched_usage_stop(शून्य)
{
	संरचना _cpu *cpu = _current_cpu;

	/* Only this CPU writes usage0, and callers keep local IRQs masked. */
	यदि (cpu->usage0 == 0U) {
		प्रत्यागम;
	}

	k_spinlock_कुञ्चिका_t k = k_spin_तालकम्(&usage_तालकम्);

	uint32_t u0 = cpu->usage0;

	यदि (u0 != 0) {
		uint32_t आवृत्तयः = usage_now() - u0;

		यदि (cpu->current->आधारः.usage.track_usage) {
			sched_thread_update_usage(cpu->current, आवृत्तयः);
		}

		sched_cpu_update_usage(cpu, आवृत्तयः);
	}

	cpu->usage0 = 0;
	k_spin_unlock(&usage_तालकम्, k);
}

#यदि_परिभाषितम् CONFIG_SCHED_THREAD_USAGE_ALL
शून्य z_sched_cpu_usage(uint8_t cpu_id, संरचना k_thread_runtime_stats *stats)
{
	k_spinlock_कुञ्चिका_t  कुञ्चिका;
	संरचना _cpu *cpu;
	uint32_t pending = 0U;
	bool pending_is_idle = false;

	कुञ्चिका = k_spin_तालकम्(&usage_तालकम्);
	cpu = &_kernel.cpus[cpu_id];

	यदि (cpu == _current_cpu) {
		uint32_t  now = usage_now();
		uint32_t आवृत्तयः = now - cpu->usage0;

		/*
		 * Getting stats for the current CPU. Update both its
		 * current thread stats and the CPU stats as the CPU's
		 * [usage0] field will also get updated. This keeps all
		 * that information up-to-date.
		 */

		यदि (cpu->current->आधारः.usage.track_usage) {
			sched_thread_update_usage(cpu->current, आवृत्तयः);
		}

		sched_cpu_update_usage(cpu, आवृत्तयः);

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

	stats->योग_आवृत्तयः     = cpu->usage->योग +
				  (pending_is_idle ? 0U : pending);
#यदि_परिभाषितम् CONFIG_SCHED_THREAD_USAGE_ANALYSIS
	stats->current_आवृत्तयः   = cpu->usage->current;
	stats->peak_आवृत्तयः      = cpu->usage->longest;

	यदि (cpu->usage->अङ्कः_windows == 0) {
		stats->मध्यमानम्_आवृत्तयः = 0;
	} अन्यथा {
		stats->मध्यमानम्_आवृत्तयः = stats->योग_आवृत्तयः /
					cpu->usage->अङ्कः_windows;
	}
#यदि_अन्तः /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */

	stats->idle_आवृत्तयः =
		_kernel.cpus[cpu_id].idle_thread->आधारः.usage.योग +
		(pending_is_idle ? pending : 0U);

	stats->execution_आवृत्तयः = stats->योग_आवृत्तयः + stats->idle_आवृत्तयः;

	k_spin_unlock(&usage_तालकम्, कुञ्चिका);
}
#यदि_अन्तः /* CONFIG_SCHED_THREAD_USAGE_ALL */

शून्य z_sched_thread_usage(संरचना k_thread *thread,
			  संरचना k_thread_runtime_stats *stats)
{
	संरचना _cpu *cpu;
	k_spinlock_कुञ्चिका_t  कुञ्चिका;

	कुञ्चिका = k_spin_तालकम्(&usage_तालकम्);
	cpu = _current_cpu;


	यदि (thread == cpu->current) {
		uint32_t now = usage_now();
		uint32_t आवृत्तयः = now - cpu->usage0;

		/*
		 * Getting stats for the current thread. Update both the
		 * current thread stats and its CPU stats as the CPU's
		 * [usage0] field will also get updated. This keeps all
		 * that information up-to-date.
		 */

		यदि (thread->आधारः.usage.track_usage) {
			sched_thread_update_usage(thread, आवृत्तयः);
		}

		sched_cpu_update_usage(cpu, आवृत्तयः);

		cpu->usage0 = now;
	}

	stats->execution_आवृत्तयः = thread->आधारः.usage.योग;
	stats->योग_आवृत्तयः     = thread->आधारः.usage.योग;

	/* Copy-out the thread's usage stats */

#यदि_परिभाषितम् CONFIG_SCHED_THREAD_USAGE_ANALYSIS
	stats->current_आवृत्तयः = thread->आधारः.usage.current;
	stats->peak_आवृत्तयः    = thread->आधारः.usage.longest;

	यदि (thread->आधारः.usage.अङ्कः_windows == 0) {
		stats->मध्यमानम्_आवृत्तयः = 0;
	} अन्यथा {
		stats->मध्यमानम्_आवृत्तयः = stats->योग_आवृत्तयः /
					thread->आधारः.usage.अङ्कः_windows;
	}
#यदि_अन्तः /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */

#यदि_परिभाषितम् CONFIG_SCHED_THREAD_USAGE_ALL
	stats->idle_आवृत्तयः = 0;
#यदि_अन्तः /* CONFIG_SCHED_THREAD_USAGE_ALL */

	k_spin_unlock(&usage_तालकम्, कुञ्चिका);
}

#यदि_परिभाषितम् CONFIG_SCHED_THREAD_USAGE_ANALYSIS
पूर्णाङ्क k_thread_runtime_stats_enable(k_tid_t  thread)
{
	k_spinlock_कुञ्चिका_t  कुञ्चिका;

	CHECKIF(thread == रिक्त) {
		प्रत्यागम -EINVAL;
	}

	कुञ्चिका = k_spin_तालकम्(&usage_तालकम्);
	संरचना _cpu *cpu = _current_cpu;

	यदि (!thread->आधारः.usage.track_usage) {
		thread->आधारः.usage.track_usage = true;
		thread->आधारः.usage.अङ्कः_windows++;
		thread->आधारः.usage.current = 0;

		यदि (thread == cpu->current) {
			uint32_t now = usage_now();

			sched_cpu_update_usage(cpu, now - cpu->usage0);

			cpu->usage0 = now;
		}
	}

	k_spin_unlock(&usage_तालकम्, कुञ्चिका);

	प्रत्यागम 0;
}

पूर्णाङ्क k_thread_runtime_stats_disable(k_tid_t  thread)
{
	k_spinlock_कुञ्चिका_t कुञ्चिका;

	CHECKIF(thread == रिक्त) {
		प्रत्यागम -EINVAL;
	}

	कुञ्चिका = k_spin_तालकम्(&usage_तालकम्);
	संरचना _cpu *cpu = _current_cpu;

	यदि (thread->आधारः.usage.track_usage) {
		thread->आधारः.usage.track_usage = false;

		यदि (thread == cpu->current) {
			uint32_t now = usage_now();
			uint32_t आवृत्तयः = now - cpu->usage0;

			sched_thread_update_usage(thread, आवृत्तयः);
			sched_cpu_update_usage(cpu, आवृत्तयः);

			cpu->usage0 = now;
		}
	}

	k_spin_unlock(&usage_तालकम्, कुञ्चिका);

	प्रत्यागम 0;
}

bool k_thread_runtime_stats_is_enabled(k_tid_t thread)
{
	प्रत्यागम thread->आधारः.usage.track_usage;
}
#यदि_अन्तः /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */

#यदि_परिभाषितम् CONFIG_SCHED_THREAD_USAGE_ALL
शून्य k_sys_runtime_stats_enable(शून्य)
{
	k_spinlock_कुञ्चिका_t  कुञ्चिका;

	कुञ्चिका = k_spin_तालकम्(&usage_तालकम्);

	यदि (_current_cpu->usage->track_usage) {

		/*
		 * Usage tracking is already enabled on the current CPU
		 * and thus on all other CPUs (if applicable). There is
		 * nothing left to do.
		 */

		k_spin_unlock(&usage_तालकम्, कुञ्चिका);
		प्रत्यागम;
	}

	/* Enable gathering of runtime stats on each CPU */

	अचिह्नित पूर्णाङ्क अङ्कः_cpus = arch_अङ्कः_cpus();

	प्रति (अचिह्नित पूर्णाङ्क i = 0; i < अङ्कः_cpus; i++) {
		_kernel.cpus[i].usage->track_usage = true;
#यदि_परिभाषितम् CONFIG_SCHED_THREAD_USAGE_ANALYSIS
		_kernel.cpus[i].usage->अङ्कः_windows++;
		_kernel.cpus[i].usage->current = 0;
#यदि_अन्तः /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */
	}

	k_spin_unlock(&usage_तालकम्, कुञ्चिका);
}

शून्य k_sys_runtime_stats_disable(शून्य)
{
	संरचना _cpu *cpu;
	k_spinlock_कुञ्चिका_t कुञ्चिका;

	कुञ्चिका = k_spin_तालकम्(&usage_तालकम्);

	यदि (!_current_cpu->usage->track_usage) {

		/*
		 * Usage tracking is already disabled on the current CPU
		 * and thus on all other CPUs (if applicable). There is
		 * nothing left to do.
		 */

		k_spin_unlock(&usage_तालकम्, कुञ्चिका);
		प्रत्यागम;
	}

	uint32_t now = usage_now();

	अचिह्नित पूर्णाङ्क अङ्कः_cpus = arch_अङ्कः_cpus();

	प्रति (अचिह्नित पूर्णाङ्क i = 0; i < अङ्कः_cpus; i++) {
		cpu = &_kernel.cpus[i];
		यदि (cpu->usage0 != 0) {
			sched_cpu_update_usage(cpu, now - cpu->usage0);
		}
		cpu->usage->track_usage = false;
	}

	k_spin_unlock(&usage_तालकम्, कुञ्चिका);
}
#यदि_अन्तः /* CONFIG_SCHED_THREAD_USAGE_ALL */

#यदि_परिभाषितम् CONFIG_OBJ_CORE_STATS_THREAD
पूर्णाङ्क z_thread_stats_raw(संरचना k_obj_core *obj_core, शून्य *stats)
{
	k_spinlock_कुञ्चिका_t  कुञ्चिका;

	कुञ्चिका = k_spin_तालकम्(&usage_तालकम्);
	स्मृतिं_प्रतिलिख(stats, obj_core->stats, परिमाण(संरचना k_आवृत्तिः_stats));
	k_spin_unlock(&usage_तालकम्, कुञ्चिका);

	प्रत्यागम 0;
}

पूर्णाङ्क z_thread_stats_query(संरचना k_obj_core *obj_core, शून्य *stats)
{
	संरचना k_thread *thread;

	thread = CONTAINER_सम्बन्धि(obj_core, संरचना k_thread, obj_core);

	z_sched_thread_usage(thread, stats);

	प्रत्यागम 0;
}

पूर्णाङ्क z_thread_stats_reset(संरचना k_obj_core *obj_core)
{
	k_spinlock_कुञ्चिका_t  कुञ्चिका;
	संरचना k_आवृत्तिः_stats  *stats;
	संरचना k_thread *thread;

	thread = CONTAINER_सम्बन्धि(obj_core, संरचना k_thread, obj_core);
	कुञ्चिका = k_spin_तालकम्(&usage_तालकम्);
	stats = obj_core->stats;

	stats->योग = 0ULL;
#यदि_परिभाषितम् CONFIG_SCHED_THREAD_USAGE_ANALYSIS
	stats->current = 0ULL;
	stats->longest = 0ULL;
	stats->अङ्कः_windows = (thread->आधारः.usage.track_usage) ?  1U : 0U;
#यदि_अन्तः /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */

	यदि (thread != _current_cpu->current) {

		/*
		 * If the thread is not running, there is nothing else to do.
		 * If the thread is running on another core, then it is not
		 * safe to do anything else but unlock and return (and pretend
		 * that its stats were reset at the start of its execution
		 * window.
		 */

		k_spin_unlock(&usage_तालकम्, कुञ्चिका);

		प्रत्यागम 0;
	}

	/* Update the current CPU stats. */

	uint32_t now = usage_now();

	sched_cpu_update_usage(_current_cpu, now - _current_cpu->usage0);

	_current_cpu->usage0 = now;

	k_spin_unlock(&usage_तालकम्, कुञ्चिका);

	प्रत्यागम 0;
}

पूर्णाङ्क z_thread_stats_disable(संरचना k_obj_core *obj_core)
{
#यदि_परिभाषितम् CONFIG_SCHED_THREAD_USAGE_ANALYSIS
	संरचना k_thread *thread;

	thread = CONTAINER_सम्बन्धि(obj_core, संरचना k_thread, obj_core);

	प्रत्यागम k_thread_runtime_stats_disable(thread);
#अन्यथा
	प्रत्यागम -ENOTSUP;
#यदि_अन्तः /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */
}

पूर्णाङ्क z_thread_stats_enable(संरचना k_obj_core *obj_core)
{
#यदि_परिभाषितम् CONFIG_SCHED_THREAD_USAGE_ANALYSIS
	संरचना k_thread *thread;

	thread = CONTAINER_सम्बन्धि(obj_core, संरचना k_thread, obj_core);

	प्रत्यागम k_thread_runtime_stats_enable(thread);
#अन्यथा
	प्रत्यागम -ENOTSUP;
#यदि_अन्तः /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */
}
#यदि_अन्तः /* CONFIG_OBJ_CORE_STATS_THREAD */

#यदि_परिभाषितम् CONFIG_OBJ_CORE_STATS_SYSTEM
पूर्णाङ्क z_cpu_stats_raw(संरचना k_obj_core *obj_core, शून्य *stats)
{
	k_spinlock_कुञ्चिका_t  कुञ्चिका;

	कुञ्चिका = k_spin_तालकम्(&usage_तालकम्);
	स्मृतिं_प्रतिलिख(stats, obj_core->stats, परिमाण(संरचना k_आवृत्तिः_stats));
	k_spin_unlock(&usage_तालकम्, कुञ्चिका);

	प्रत्यागम 0;
}

पूर्णाङ्क z_cpu_stats_query(संरचना k_obj_core *obj_core, शून्य *stats)
{
	संरचना _cpu  *cpu;

	cpu = CONTAINER_सम्बन्धि(obj_core, संरचना _cpu, obj_core);

	z_sched_cpu_usage(cpu->id, stats);

	प्रत्यागम 0;
}
#यदि_अन्तः /* CONFIG_OBJ_CORE_STATS_SYSTEM */

#यदि_परिभाषितम् CONFIG_OBJ_CORE_STATS_SYSTEM
पूर्णाङ्क z_kernel_stats_raw(संरचना k_obj_core *obj_core, शून्य *stats)
{
	k_spinlock_कुञ्चिका_t  कुञ्चिका;

	कुञ्चिका = k_spin_तालकम्(&usage_तालकम्);
	स्मृतिं_प्रतिलिख(stats, obj_core->stats,
	       CONFIG_MP_MAX_अङ्कः_CPUS * परिमाण(संरचना k_आवृत्तिः_stats));
	k_spin_unlock(&usage_तालकम्, कुञ्चिका);

	प्रत्यागम 0;
}

पूर्णाङ्क z_kernel_stats_query(संरचना k_obj_core *obj_core, शून्य *stats)
{
	ARG_अप्रयुक्तम्(obj_core);

	प्रत्यागम k_thread_runtime_stats_all_get(stats);
}
#यदि_अन्तः /* CONFIG_OBJ_CORE_STATS_SYSTEM */
