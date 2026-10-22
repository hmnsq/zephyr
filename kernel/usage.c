/*
 * Copyright (c) 2018 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ضمن <zephyr/kernel.h>

#ضمن <zephyr/timing/timing.h>
#ضمن <ksched.h>
#ضمن <zephyr/spinlock.h>
#ضمن <zephyr/sys/check.h>

/* Need one of these for this to work */
#إذا !defined(CONFIG_USE_SWITCH) && !defined(CONFIG_INSTRUMENT_THREAD_SWITCHING)
#تېروتنه "No data backend configured for CONFIG_SCHED_THREAD_USAGE"
#د_که_پای /* !CONFIG_USE_SWITCH && !CONFIG_INSTRUMENT_THREAD_SWITCHING */

ساكن بنية k_spinlock usage_قلف;

ساكن uint32_t usage_now(فراغ)
{
	uint32_t now;

#که_تعریف_شوی CONFIG_THREAD_RUNTIME_STATS_USE_TIMING_FUNCTIONS
	now = (uint32_t)timing_counter_get();
#وإلا
	now = k_دوره_get_32();
#د_که_پای /* CONFIG_THREAD_RUNTIME_STATS_USE_TIMING_FUNCTIONS */

	/* Edge case: we use a zero as a null ("stop() already called") */
	أرجع (now == 0) ? 1 : now;
}

#که_تعریف_شوی CONFIG_SCHED_THREAD_USAGE_ALL
ساكن فراغ sched_cpu_update_usage(بنية _cpu *cpu, uint32_t دورې)
{
	إذا (!cpu->usage->track_usage) {
		أرجع;
	}

	إذا (cpu->current != cpu->idle_thread) {
		cpu->usage->ټول += دورې;

#که_تعریف_شوی CONFIG_SCHED_THREAD_USAGE_ANALYSIS
		cpu->usage->current += دورې;

		إذا (cpu->usage->longest < cpu->usage->current) {
			cpu->usage->longest = cpu->usage->current;
		}
	} وإلا {
		cpu->usage->current = 0;
		cpu->usage->عدد_windows++;
#د_که_پای /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */
	}
}
#وإلا
#تعریف_کړه sched_cpu_update_usage(cpu, دورې)   افعل { } طالما (0)
#د_که_پای /* CONFIG_SCHED_THREAD_USAGE_ALL */

ساكن فراغ sched_thread_update_usage(بنية k_thread *thread, uint32_t دورې)
{
	thread->بنسټ.usage.ټول += دورې;

#که_تعریف_شوی CONFIG_SCHED_THREAD_USAGE_ANALYSIS
	thread->بنسټ.usage.current += دورې;

	إذا (thread->بنسټ.usage.longest < thread->بنسټ.usage.current) {
		thread->بنسټ.usage.longest = thread->بنسټ.usage.current;
	}
#د_که_پای /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */
}

فراغ z_sched_usage_پیل(بنية k_thread *thread)
{
#که_تعریف_شوی CONFIG_SCHED_THREAD_USAGE_ANALYSIS
	k_spinlock_کلی_t  کلی;

	کلی = k_spin_قلف(&usage_قلف);

	_current_cpu->usage0 = usage_now();   /* Always update */

	إذا (thread->بنسټ.usage.track_usage) {
		thread->بنسټ.usage.عدد_windows++;
		thread->بنسټ.usage.current = 0;
	}

	k_spin_unlock(&usage_قلف, کلی);
#وإلا
	/* One write through a volatile pointer doesn't require
	 * synchronization as long as _usage() treats it as volatile
	 * (we can't race with _stop() by design).
	 */

	_current_cpu->usage0 = usage_now();
#د_که_پای /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */
}

فراغ z_sched_usage_stop(فراغ)
{
	بنية _cpu *cpu = _current_cpu;

	/* Only this CPU writes usage0, and callers keep local IRQs masked. */
	إذا (cpu->usage0 == 0U) {
		أرجع;
	}

	k_spinlock_کلی_t k = k_spin_قلف(&usage_قلف);

	uint32_t u0 = cpu->usage0;

	إذا (u0 != 0) {
		uint32_t دورې = usage_now() - u0;

		إذا (cpu->current->بنسټ.usage.track_usage) {
			sched_thread_update_usage(cpu->current, دورې);
		}

		sched_cpu_update_usage(cpu, دورې);
	}

	cpu->usage0 = 0;
	k_spin_unlock(&usage_قلف, k);
}

#که_تعریف_شوی CONFIG_SCHED_THREAD_USAGE_ALL
فراغ z_sched_cpu_usage(uint8_t cpu_id, بنية k_thread_runtime_stats *stats)
{
	k_spinlock_کلی_t  کلی;
	بنية _cpu *cpu;
	uint32_t pending = 0U;
	bool pending_is_idle = false;

	کلی = k_spin_قلف(&usage_قلف);
	cpu = &_kernel.cpus[cpu_id];

	إذا (cpu == _current_cpu) {
		uint32_t  now = usage_now();
		uint32_t دورې = now - cpu->usage0;

		/*
		 * Getting stats for the current CPU. Update both its
		 * current thread stats and the CPU stats as the CPU's
		 * [usage0] field will also get updated. This keeps all
		 * that information up-to-date.
		 */

		إذا (cpu->current->بنسټ.usage.track_usage) {
			sched_thread_update_usage(cpu->current, دورې);
		}

		sched_cpu_update_usage(cpu, دورې);

		cpu->usage0 = now;
	} وإلا إذا (cpu->usage->track_usage) {
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

	stats->ټول_دورې     = cpu->usage->ټول +
				  (pending_is_idle ? 0U : pending);
#که_تعریف_شوی CONFIG_SCHED_THREAD_USAGE_ANALYSIS
	stats->current_دورې   = cpu->usage->current;
	stats->peak_دورې      = cpu->usage->longest;

	إذا (cpu->usage->عدد_windows == 0) {
		stats->اوسط_دورې = 0;
	} وإلا {
		stats->اوسط_دورې = stats->ټول_دورې /
					cpu->usage->عدد_windows;
	}
#د_که_پای /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */

	stats->idle_دورې =
		_kernel.cpus[cpu_id].idle_thread->بنسټ.usage.ټول +
		(pending_is_idle ? pending : 0U);

	stats->execution_دورې = stats->ټول_دورې + stats->idle_دورې;

	k_spin_unlock(&usage_قلف, کلی);
}
#د_که_پای /* CONFIG_SCHED_THREAD_USAGE_ALL */

فراغ z_sched_thread_usage(بنية k_thread *thread,
			  بنية k_thread_runtime_stats *stats)
{
	بنية _cpu *cpu;
	k_spinlock_کلی_t  کلی;

	کلی = k_spin_قلف(&usage_قلف);
	cpu = _current_cpu;


	إذا (thread == cpu->current) {
		uint32_t now = usage_now();
		uint32_t دورې = now - cpu->usage0;

		/*
		 * Getting stats for the current thread. Update both the
		 * current thread stats and its CPU stats as the CPU's
		 * [usage0] field will also get updated. This keeps all
		 * that information up-to-date.
		 */

		إذا (thread->بنسټ.usage.track_usage) {
			sched_thread_update_usage(thread, دورې);
		}

		sched_cpu_update_usage(cpu, دورې);

		cpu->usage0 = now;
	}

	stats->execution_دورې = thread->بنسټ.usage.ټول;
	stats->ټول_دورې     = thread->بنسټ.usage.ټول;

	/* Copy-out the thread's usage stats */

#که_تعریف_شوی CONFIG_SCHED_THREAD_USAGE_ANALYSIS
	stats->current_دورې = thread->بنسټ.usage.current;
	stats->peak_دورې    = thread->بنسټ.usage.longest;

	إذا (thread->بنسټ.usage.عدد_windows == 0) {
		stats->اوسط_دورې = 0;
	} وإلا {
		stats->اوسط_دورې = stats->ټول_دورې /
					thread->بنسټ.usage.عدد_windows;
	}
#د_که_پای /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */

#که_تعریف_شوی CONFIG_SCHED_THREAD_USAGE_ALL
	stats->idle_دورې = 0;
#د_که_پای /* CONFIG_SCHED_THREAD_USAGE_ALL */

	k_spin_unlock(&usage_قلف, کلی);
}

#که_تعریف_شوی CONFIG_SCHED_THREAD_USAGE_ANALYSIS
صحيح k_thread_runtime_stats_enable(k_tid_t  thread)
{
	k_spinlock_کلی_t  کلی;

	CHECKIF(thread == عدم) {
		أرجع -EINVAL;
	}

	کلی = k_spin_قلف(&usage_قلف);
	بنية _cpu *cpu = _current_cpu;

	إذا (!thread->بنسټ.usage.track_usage) {
		thread->بنسټ.usage.track_usage = true;
		thread->بنسټ.usage.عدد_windows++;
		thread->بنسټ.usage.current = 0;

		إذا (thread == cpu->current) {
			uint32_t now = usage_now();

			sched_cpu_update_usage(cpu, now - cpu->usage0);

			cpu->usage0 = now;
		}
	}

	k_spin_unlock(&usage_قلف, کلی);

	أرجع 0;
}

صحيح k_thread_runtime_stats_disable(k_tid_t  thread)
{
	k_spinlock_کلی_t کلی;

	CHECKIF(thread == عدم) {
		أرجع -EINVAL;
	}

	کلی = k_spin_قلف(&usage_قلف);
	بنية _cpu *cpu = _current_cpu;

	إذا (thread->بنسټ.usage.track_usage) {
		thread->بنسټ.usage.track_usage = false;

		إذا (thread == cpu->current) {
			uint32_t now = usage_now();
			uint32_t دورې = now - cpu->usage0;

			sched_thread_update_usage(thread, دورې);
			sched_cpu_update_usage(cpu, دورې);

			cpu->usage0 = now;
		}
	}

	k_spin_unlock(&usage_قلف, کلی);

	أرجع 0;
}

bool k_thread_runtime_stats_is_enabled(k_tid_t thread)
{
	أرجع thread->بنسټ.usage.track_usage;
}
#د_که_پای /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */

#که_تعریف_شوی CONFIG_SCHED_THREAD_USAGE_ALL
فراغ k_sys_runtime_stats_enable(فراغ)
{
	k_spinlock_کلی_t  کلی;

	کلی = k_spin_قلف(&usage_قلف);

	إذا (_current_cpu->usage->track_usage) {

		/*
		 * Usage tracking is already enabled on the current CPU
		 * and thus on all other CPUs (if applicable). There is
		 * nothing left to do.
		 */

		k_spin_unlock(&usage_قلف, کلی);
		أرجع;
	}

	/* Enable gathering of runtime stats on each CPU */

	بینښه صحيح عدد_cpus = arch_عدد_cpus();

	لكل (بینښه صحيح i = 0; i < عدد_cpus; i++) {
		_kernel.cpus[i].usage->track_usage = true;
#که_تعریف_شوی CONFIG_SCHED_THREAD_USAGE_ANALYSIS
		_kernel.cpus[i].usage->عدد_windows++;
		_kernel.cpus[i].usage->current = 0;
#د_که_پای /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */
	}

	k_spin_unlock(&usage_قلف, کلی);
}

فراغ k_sys_runtime_stats_disable(فراغ)
{
	بنية _cpu *cpu;
	k_spinlock_کلی_t کلی;

	کلی = k_spin_قلف(&usage_قلف);

	إذا (!_current_cpu->usage->track_usage) {

		/*
		 * Usage tracking is already disabled on the current CPU
		 * and thus on all other CPUs (if applicable). There is
		 * nothing left to do.
		 */

		k_spin_unlock(&usage_قلف, کلی);
		أرجع;
	}

	uint32_t now = usage_now();

	بینښه صحيح عدد_cpus = arch_عدد_cpus();

	لكل (بینښه صحيح i = 0; i < عدد_cpus; i++) {
		cpu = &_kernel.cpus[i];
		إذا (cpu->usage0 != 0) {
			sched_cpu_update_usage(cpu, now - cpu->usage0);
		}
		cpu->usage->track_usage = false;
	}

	k_spin_unlock(&usage_قلف, کلی);
}
#د_که_پای /* CONFIG_SCHED_THREAD_USAGE_ALL */

#که_تعریف_شوی CONFIG_OBJ_CORE_STATS_THREAD
صحيح z_thread_stats_raw(بنية k_obj_core *obj_core, فراغ *stats)
{
	k_spinlock_کلی_t  کلی;

	کلی = k_spin_قلف(&usage_قلف);
	حافظه_نقل(stats, obj_core->stats, حجم(بنية k_دوره_stats));
	k_spin_unlock(&usage_قلف, کلی);

	أرجع 0;
}

صحيح z_thread_stats_query(بنية k_obj_core *obj_core, فراغ *stats)
{
	بنية k_thread *thread;

	thread = CONTAINER_د(obj_core, بنية k_thread, obj_core);

	z_sched_thread_usage(thread, stats);

	أرجع 0;
}

صحيح z_thread_stats_reset(بنية k_obj_core *obj_core)
{
	k_spinlock_کلی_t  کلی;
	بنية k_دوره_stats  *stats;
	بنية k_thread *thread;

	thread = CONTAINER_د(obj_core, بنية k_thread, obj_core);
	کلی = k_spin_قلف(&usage_قلف);
	stats = obj_core->stats;

	stats->ټول = 0ULL;
#که_تعریف_شوی CONFIG_SCHED_THREAD_USAGE_ANALYSIS
	stats->current = 0ULL;
	stats->longest = 0ULL;
	stats->عدد_windows = (thread->بنسټ.usage.track_usage) ?  1U : 0U;
#د_که_پای /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */

	إذا (thread != _current_cpu->current) {

		/*
		 * If the thread is not running, there is nothing else to do.
		 * If the thread is running on another core, then it is not
		 * safe to do anything else but unlock and return (and pretend
		 * that its stats were reset at the start of its execution
		 * window.
		 */

		k_spin_unlock(&usage_قلف, کلی);

		أرجع 0;
	}

	/* Update the current CPU stats. */

	uint32_t now = usage_now();

	sched_cpu_update_usage(_current_cpu, now - _current_cpu->usage0);

	_current_cpu->usage0 = now;

	k_spin_unlock(&usage_قلف, کلی);

	أرجع 0;
}

صحيح z_thread_stats_disable(بنية k_obj_core *obj_core)
{
#که_تعریف_شوی CONFIG_SCHED_THREAD_USAGE_ANALYSIS
	بنية k_thread *thread;

	thread = CONTAINER_د(obj_core, بنية k_thread, obj_core);

	أرجع k_thread_runtime_stats_disable(thread);
#وإلا
	أرجع -ENOTSUP;
#د_که_پای /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */
}

صحيح z_thread_stats_enable(بنية k_obj_core *obj_core)
{
#که_تعریف_شوی CONFIG_SCHED_THREAD_USAGE_ANALYSIS
	بنية k_thread *thread;

	thread = CONTAINER_د(obj_core, بنية k_thread, obj_core);

	أرجع k_thread_runtime_stats_enable(thread);
#وإلا
	أرجع -ENOTSUP;
#د_که_پای /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */
}
#د_که_پای /* CONFIG_OBJ_CORE_STATS_THREAD */

#که_تعریف_شوی CONFIG_OBJ_CORE_STATS_SYSTEM
صحيح z_cpu_stats_raw(بنية k_obj_core *obj_core, فراغ *stats)
{
	k_spinlock_کلی_t  کلی;

	کلی = k_spin_قلف(&usage_قلف);
	حافظه_نقل(stats, obj_core->stats, حجم(بنية k_دوره_stats));
	k_spin_unlock(&usage_قلف, کلی);

	أرجع 0;
}

صحيح z_cpu_stats_query(بنية k_obj_core *obj_core, فراغ *stats)
{
	بنية _cpu  *cpu;

	cpu = CONTAINER_د(obj_core, بنية _cpu, obj_core);

	z_sched_cpu_usage(cpu->id, stats);

	أرجع 0;
}
#د_که_پای /* CONFIG_OBJ_CORE_STATS_SYSTEM */

#که_تعریف_شوی CONFIG_OBJ_CORE_STATS_SYSTEM
صحيح z_kernel_stats_raw(بنية k_obj_core *obj_core, فراغ *stats)
{
	k_spinlock_کلی_t  کلی;

	کلی = k_spin_قلف(&usage_قلف);
	حافظه_نقل(stats, obj_core->stats,
	       CONFIG_MP_MAX_عدد_CPUS * حجم(بنية k_دوره_stats));
	k_spin_unlock(&usage_قلف, کلی);

	أرجع 0;
}

صحيح z_kernel_stats_query(بنية k_obj_core *obj_core, فراغ *stats)
{
	ARG_ناکارول_شوی(obj_core);

	أرجع k_thread_runtime_stats_all_get(stats);
}
#د_که_پای /* CONFIG_OBJ_CORE_STATS_SYSTEM */
