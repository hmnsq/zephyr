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
#غلطی "No data backend configured for CONFIG_SCHED_THREAD_USAGE"
#جے_انت /* !CONFIG_USE_SWITCH && !CONFIG_INSTRUMENT_THREAD_SWITCHING */

ساكن بنية k_spinlock usage_جندرا;

ساكن uint32_t usage_now(فراغ)
{
	uint32_t now;

#جے_تعریف_شدہ CONFIG_THREAD_RUNTIME_STATS_USE_TIMING_FUNCTIONS
	now = (uint32_t)timing_counter_get();
#وإلا
	now = k_چکر_get_32();
#جے_انت /* CONFIG_THREAD_RUNTIME_STATS_USE_TIMING_FUNCTIONS */

	/* Edge case: we use a zero as a null ("stop() already called") */
	أرجع (now == 0) ? 1 : now;
}

#جے_تعریف_شدہ CONFIG_SCHED_THREAD_USAGE_ALL
ساكن فراغ sched_cpu_update_usage(بنية _cpu *cpu, uint32_t چکراں)
{
	إذا (!cpu->usage->track_usage) {
		أرجع;
	}

	إذا (cpu->current != cpu->idle_thread) {
		cpu->usage->کل += چکراں;

#جے_تعریف_شدہ CONFIG_SCHED_THREAD_USAGE_ANALYSIS
		cpu->usage->current += چکراں;

		إذا (cpu->usage->longest < cpu->usage->current) {
			cpu->usage->longest = cpu->usage->current;
		}
	} وإلا {
		cpu->usage->current = 0;
		cpu->usage->عدد_windows++;
#جے_انت /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */
	}
}
#وإلا
#تعریف_کرو sched_cpu_update_usage(cpu, چکراں)   افعل { } طالما (0)
#جے_انت /* CONFIG_SCHED_THREAD_USAGE_ALL */

ساكن فراغ sched_thread_update_usage(بنية k_thread *thread, uint32_t چکراں)
{
	thread->ادھار.usage.کل += چکراں;

#جے_تعریف_شدہ CONFIG_SCHED_THREAD_USAGE_ANALYSIS
	thread->ادھار.usage.current += چکراں;

	إذا (thread->ادھار.usage.longest < thread->ادھار.usage.current) {
		thread->ادھار.usage.longest = thread->ادھار.usage.current;
	}
#جے_انت /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */
}

فراغ z_sched_usage_شروع(بنية k_thread *thread)
{
#جے_تعریف_شدہ CONFIG_SCHED_THREAD_USAGE_ANALYSIS
	k_spinlock_کلید_t  کلید;

	کلید = k_spin_جندرا(&usage_جندرا);

	_current_cpu->usage0 = usage_now();   /* Always update */

	إذا (thread->ادھار.usage.track_usage) {
		thread->ادھار.usage.عدد_windows++;
		thread->ادھار.usage.current = 0;
	}

	k_spin_unlock(&usage_جندرا, کلید);
#وإلا
	/* One write through a volatile pointer doesn't require
	 * synchronization as long as _usage() treats it as volatile
	 * (we can't race with _stop() by design).
	 */

	_current_cpu->usage0 = usage_now();
#جے_انت /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */
}

فراغ z_sched_usage_stop(فراغ)
{
	بنية _cpu *cpu = _current_cpu;

	/* Only this CPU writes usage0, and callers keep local IRQs masked. */
	إذا (cpu->usage0 == 0U) {
		أرجع;
	}

	k_spinlock_کلید_t k = k_spin_جندرا(&usage_جندرا);

	uint32_t u0 = cpu->usage0;

	إذا (u0 != 0) {
		uint32_t چکراں = usage_now() - u0;

		إذا (cpu->current->ادھار.usage.track_usage) {
			sched_thread_update_usage(cpu->current, چکراں);
		}

		sched_cpu_update_usage(cpu, چکراں);
	}

	cpu->usage0 = 0;
	k_spin_unlock(&usage_جندرا, k);
}

#جے_تعریف_شدہ CONFIG_SCHED_THREAD_USAGE_ALL
فراغ z_sched_cpu_usage(uint8_t cpu_id, بنية k_thread_runtime_stats *stats)
{
	k_spinlock_کلید_t  کلید;
	بنية _cpu *cpu;
	uint32_t pending = 0U;
	bool pending_is_idle = false;

	کلید = k_spin_جندرا(&usage_جندرا);
	cpu = &_kernel.cpus[cpu_id];

	إذا (cpu == _current_cpu) {
		uint32_t  now = usage_now();
		uint32_t چکراں = now - cpu->usage0;

		/*
		 * Getting stats for the current CPU. Update both its
		 * current thread stats and the CPU stats as the CPU's
		 * [usage0] field will also get updated. This keeps all
		 * that information up-to-date.
		 */

		إذا (cpu->current->ادھار.usage.track_usage) {
			sched_thread_update_usage(cpu->current, چکراں);
		}

		sched_cpu_update_usage(cpu, چکراں);

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

	stats->کل_چکراں     = cpu->usage->کل +
				  (pending_is_idle ? 0U : pending);
#جے_تعریف_شدہ CONFIG_SCHED_THREAD_USAGE_ANALYSIS
	stats->current_چکراں   = cpu->usage->current;
	stats->peak_چکراں      = cpu->usage->longest;

	إذا (cpu->usage->عدد_windows == 0) {
		stats->اوسطن_چکراں = 0;
	} وإلا {
		stats->اوسطن_چکراں = stats->کل_چکراں /
					cpu->usage->عدد_windows;
	}
#جے_انت /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */

	stats->idle_چکراں =
		_kernel.cpus[cpu_id].idle_thread->ادھار.usage.کل +
		(pending_is_idle ? pending : 0U);

	stats->execution_چکراں = stats->کل_چکراں + stats->idle_چکراں;

	k_spin_unlock(&usage_جندرا, کلید);
}
#جے_انت /* CONFIG_SCHED_THREAD_USAGE_ALL */

فراغ z_sched_thread_usage(بنية k_thread *thread,
			  بنية k_thread_runtime_stats *stats)
{
	بنية _cpu *cpu;
	k_spinlock_کلید_t  کلید;

	کلید = k_spin_جندرا(&usage_جندرا);
	cpu = _current_cpu;


	إذا (thread == cpu->current) {
		uint32_t now = usage_now();
		uint32_t چکراں = now - cpu->usage0;

		/*
		 * Getting stats for the current thread. Update both the
		 * current thread stats and its CPU stats as the CPU's
		 * [usage0] field will also get updated. This keeps all
		 * that information up-to-date.
		 */

		إذا (thread->ادھار.usage.track_usage) {
			sched_thread_update_usage(thread, چکراں);
		}

		sched_cpu_update_usage(cpu, چکراں);

		cpu->usage0 = now;
	}

	stats->execution_چکراں = thread->ادھار.usage.کل;
	stats->کل_چکراں     = thread->ادھار.usage.کل;

	/* Copy-out the thread's usage stats */

#جے_تعریف_شدہ CONFIG_SCHED_THREAD_USAGE_ANALYSIS
	stats->current_چکراں = thread->ادھار.usage.current;
	stats->peak_چکراں    = thread->ادھار.usage.longest;

	إذا (thread->ادھار.usage.عدد_windows == 0) {
		stats->اوسطن_چکراں = 0;
	} وإلا {
		stats->اوسطن_چکراں = stats->کل_چکراں /
					thread->ادھار.usage.عدد_windows;
	}
#جے_انت /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */

#جے_تعریف_شدہ CONFIG_SCHED_THREAD_USAGE_ALL
	stats->idle_چکراں = 0;
#جے_انت /* CONFIG_SCHED_THREAD_USAGE_ALL */

	k_spin_unlock(&usage_جندرا, کلید);
}

#جے_تعریف_شدہ CONFIG_SCHED_THREAD_USAGE_ANALYSIS
صحيح k_thread_runtime_stats_enable(k_tid_t  thread)
{
	k_spinlock_کلید_t  کلید;

	CHECKIF(thread == عدم) {
		أرجع -EINVAL;
	}

	کلید = k_spin_جندرا(&usage_جندرا);
	بنية _cpu *cpu = _current_cpu;

	إذا (!thread->ادھار.usage.track_usage) {
		thread->ادھار.usage.track_usage = true;
		thread->ادھار.usage.عدد_windows++;
		thread->ادھار.usage.current = 0;

		إذا (thread == cpu->current) {
			uint32_t now = usage_now();

			sched_cpu_update_usage(cpu, now - cpu->usage0);

			cpu->usage0 = now;
		}
	}

	k_spin_unlock(&usage_جندرا, کلید);

	أرجع 0;
}

صحيح k_thread_runtime_stats_disable(k_tid_t  thread)
{
	k_spinlock_کلید_t کلید;

	CHECKIF(thread == عدم) {
		أرجع -EINVAL;
	}

	کلید = k_spin_جندرا(&usage_جندرا);
	بنية _cpu *cpu = _current_cpu;

	إذا (thread->ادھار.usage.track_usage) {
		thread->ادھار.usage.track_usage = false;

		إذا (thread == cpu->current) {
			uint32_t now = usage_now();
			uint32_t چکراں = now - cpu->usage0;

			sched_thread_update_usage(thread, چکراں);
			sched_cpu_update_usage(cpu, چکراں);

			cpu->usage0 = now;
		}
	}

	k_spin_unlock(&usage_جندرا, کلید);

	أرجع 0;
}

bool k_thread_runtime_stats_is_enabled(k_tid_t thread)
{
	أرجع thread->ادھار.usage.track_usage;
}
#جے_انت /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */

#جے_تعریف_شدہ CONFIG_SCHED_THREAD_USAGE_ALL
فراغ k_sys_runtime_stats_enable(فراغ)
{
	k_spinlock_کلید_t  کلید;

	کلید = k_spin_جندرا(&usage_جندرا);

	إذا (_current_cpu->usage->track_usage) {

		/*
		 * Usage tracking is already enabled on the current CPU
		 * and thus on all other CPUs (if applicable). There is
		 * nothing left to do.
		 */

		k_spin_unlock(&usage_جندرا, کلید);
		أرجع;
	}

	/* Enable gathering of runtime stats on each CPU */

	بنانشانی صحيح عدد_cpus = arch_عدد_cpus();

	لكل (بنانشانی صحيح i = 0; i < عدد_cpus; i++) {
		_kernel.cpus[i].usage->track_usage = true;
#جے_تعریف_شدہ CONFIG_SCHED_THREAD_USAGE_ANALYSIS
		_kernel.cpus[i].usage->عدد_windows++;
		_kernel.cpus[i].usage->current = 0;
#جے_انت /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */
	}

	k_spin_unlock(&usage_جندرا, کلید);
}

فراغ k_sys_runtime_stats_disable(فراغ)
{
	بنية _cpu *cpu;
	k_spinlock_کلید_t کلید;

	کلید = k_spin_جندرا(&usage_جندرا);

	إذا (!_current_cpu->usage->track_usage) {

		/*
		 * Usage tracking is already disabled on the current CPU
		 * and thus on all other CPUs (if applicable). There is
		 * nothing left to do.
		 */

		k_spin_unlock(&usage_جندرا, کلید);
		أرجع;
	}

	uint32_t now = usage_now();

	بنانشانی صحيح عدد_cpus = arch_عدد_cpus();

	لكل (بنانشانی صحيح i = 0; i < عدد_cpus; i++) {
		cpu = &_kernel.cpus[i];
		إذا (cpu->usage0 != 0) {
			sched_cpu_update_usage(cpu, now - cpu->usage0);
		}
		cpu->usage->track_usage = false;
	}

	k_spin_unlock(&usage_جندرا, کلید);
}
#جے_انت /* CONFIG_SCHED_THREAD_USAGE_ALL */

#جے_تعریف_شدہ CONFIG_OBJ_CORE_STATS_THREAD
صحيح z_thread_stats_raw(بنية k_obj_core *obj_core, فراغ *stats)
{
	k_spinlock_کلید_t  کلید;

	کلید = k_spin_جندرا(&usage_جندرا);
	حافظہ_نقل(stats, obj_core->stats, حجم(بنية k_چکر_stats));
	k_spin_unlock(&usage_جندرا, کلید);

	أرجع 0;
}

صحيح z_thread_stats_query(بنية k_obj_core *obj_core, فراغ *stats)
{
	بنية k_thread *thread;

	thread = CONTAINER_دا(obj_core, بنية k_thread, obj_core);

	z_sched_thread_usage(thread, stats);

	أرجع 0;
}

صحيح z_thread_stats_reset(بنية k_obj_core *obj_core)
{
	k_spinlock_کلید_t  کلید;
	بنية k_چکر_stats  *stats;
	بنية k_thread *thread;

	thread = CONTAINER_دا(obj_core, بنية k_thread, obj_core);
	کلید = k_spin_جندرا(&usage_جندرا);
	stats = obj_core->stats;

	stats->کل = 0ULL;
#جے_تعریف_شدہ CONFIG_SCHED_THREAD_USAGE_ANALYSIS
	stats->current = 0ULL;
	stats->longest = 0ULL;
	stats->عدد_windows = (thread->ادھار.usage.track_usage) ?  1U : 0U;
#جے_انت /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */

	إذا (thread != _current_cpu->current) {

		/*
		 * If the thread is not running, there is nothing else to do.
		 * If the thread is running on another core, then it is not
		 * safe to do anything else but unlock and return (and pretend
		 * that its stats were reset at the start of its execution
		 * window.
		 */

		k_spin_unlock(&usage_جندرا, کلید);

		أرجع 0;
	}

	/* Update the current CPU stats. */

	uint32_t now = usage_now();

	sched_cpu_update_usage(_current_cpu, now - _current_cpu->usage0);

	_current_cpu->usage0 = now;

	k_spin_unlock(&usage_جندرا, کلید);

	أرجع 0;
}

صحيح z_thread_stats_disable(بنية k_obj_core *obj_core)
{
#جے_تعریف_شدہ CONFIG_SCHED_THREAD_USAGE_ANALYSIS
	بنية k_thread *thread;

	thread = CONTAINER_دا(obj_core, بنية k_thread, obj_core);

	أرجع k_thread_runtime_stats_disable(thread);
#وإلا
	أرجع -ENOTSUP;
#جے_انت /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */
}

صحيح z_thread_stats_enable(بنية k_obj_core *obj_core)
{
#جے_تعریف_شدہ CONFIG_SCHED_THREAD_USAGE_ANALYSIS
	بنية k_thread *thread;

	thread = CONTAINER_دا(obj_core, بنية k_thread, obj_core);

	أرجع k_thread_runtime_stats_enable(thread);
#وإلا
	أرجع -ENOTSUP;
#جے_انت /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */
}
#جے_انت /* CONFIG_OBJ_CORE_STATS_THREAD */

#جے_تعریف_شدہ CONFIG_OBJ_CORE_STATS_SYSTEM
صحيح z_cpu_stats_raw(بنية k_obj_core *obj_core, فراغ *stats)
{
	k_spinlock_کلید_t  کلید;

	کلید = k_spin_جندرا(&usage_جندرا);
	حافظہ_نقل(stats, obj_core->stats, حجم(بنية k_چکر_stats));
	k_spin_unlock(&usage_جندرا, کلید);

	أرجع 0;
}

صحيح z_cpu_stats_query(بنية k_obj_core *obj_core, فراغ *stats)
{
	بنية _cpu  *cpu;

	cpu = CONTAINER_دا(obj_core, بنية _cpu, obj_core);

	z_sched_cpu_usage(cpu->id, stats);

	أرجع 0;
}
#جے_انت /* CONFIG_OBJ_CORE_STATS_SYSTEM */

#جے_تعریف_شدہ CONFIG_OBJ_CORE_STATS_SYSTEM
صحيح z_kernel_stats_raw(بنية k_obj_core *obj_core, فراغ *stats)
{
	k_spinlock_کلید_t  کلید;

	کلید = k_spin_جندرا(&usage_جندرا);
	حافظہ_نقل(stats, obj_core->stats,
	       CONFIG_MP_MAX_عدد_CPUS * حجم(بنية k_چکر_stats));
	k_spin_unlock(&usage_جندرا, کلید);

	أرجع 0;
}

صحيح z_kernel_stats_query(بنية k_obj_core *obj_core, فراغ *stats)
{
	ARG_بن_ورتیا(obj_core);

	أرجع k_thread_runtime_stats_all_get(stats);
}
#جے_انت /* CONFIG_OBJ_CORE_STATS_SYSTEM */
