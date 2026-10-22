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
#غلطی "No data backend configured for CONFIG_SCHED_THREAD_USAGE"
#اگر_ختم /* !CONFIG_USE_SWITCH && !CONFIG_INSTRUMENT_THREAD_SWITCHING */

ساکن ساخت k_spinlock usage_تالا;

ساکن uint32_t usage_now(خالی)
{
	uint32_t now;

#اگر_تعریف_شدہ CONFIG_THREAD_RUNTIME_STATS_USE_TIMING_FUNCTIONS
	now = (uint32_t)timing_شمار_get();
#ورنہ
	now = k_چکر_get_32();
#اگر_ختم /* CONFIG_THREAD_RUNTIME_STATS_USE_TIMING_FUNCTIONS */

	/* Edge case: we use a zero as a null ("stop() already called") */
	واپس (now == 0) ? 1 : now;
}

#اگر_تعریف_شدہ CONFIG_SCHED_THREAD_USAGE_ALL
ساکن خالی sched_cpu_update_usage(ساخت _cpu *cpu, uint32_t ادوار)
{
	اگر (!cpu->usage->track_usage) {
		واپس;
	}

	اگر (cpu->current != cpu->idle_thread) {
		cpu->usage->کل += ادوار;

#اگر_تعریف_شدہ CONFIG_SCHED_THREAD_USAGE_ANALYSIS
		cpu->usage->current += ادوار;

		اگر (cpu->usage->longest < cpu->usage->current) {
			cpu->usage->longest = cpu->usage->current;
		}
	} ورنہ {
		cpu->usage->current = 0;
		cpu->usage->عدد_windows++;
#اگر_ختم /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */
	}
}
#ورنہ
#تعریف_کرو sched_cpu_update_usage(cpu, ادوار)   کرو { } جب (0)
#اگر_ختم /* CONFIG_SCHED_THREAD_USAGE_ALL */

ساکن خالی sched_thread_update_usage(ساخت k_thread *thread, uint32_t ادوار)
{
	thread->بنیاد.usage.کل += ادوار;

#اگر_تعریف_شدہ CONFIG_SCHED_THREAD_USAGE_ANALYSIS
	thread->بنیاد.usage.current += ادوار;

	اگر (thread->بنیاد.usage.longest < thread->بنیاد.usage.current) {
		thread->بنیاد.usage.longest = thread->بنیاد.usage.current;
	}
#اگر_ختم /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */
}

خالی z_sched_usage_آغاز(ساخت k_thread *thread)
{
#اگر_تعریف_شدہ CONFIG_SCHED_THREAD_USAGE_ANALYSIS
	k_spinlock_کلید_t  کلید;

	کلید = k_spin_تالا(&usage_تالا);

	_current_cpu->usage0 = usage_now();   /* Always update */

	اگر (thread->بنیاد.usage.track_usage) {
		thread->بنیاد.usage.عدد_windows++;
		thread->بنیاد.usage.current = 0;
	}

	k_spin_unlock(&usage_تالا, کلید);
#ورنہ
	/* One write through a volatile pointer doesn't require
	 * synchronization as long as _usage() treats it as volatile
	 * (we can't race with _stop() by design).
	 */

	_current_cpu->usage0 = usage_now();
#اگر_ختم /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */
}

خالی z_sched_usage_stop(خالی)
{
	ساخت _cpu *cpu = _current_cpu;

	/* Only this CPU writes usage0, and callers keep local IRQs masked. */
	اگر (cpu->usage0 == 0U) {
		واپس;
	}

	k_spinlock_کلید_t k = k_spin_تالا(&usage_تالا);

	uint32_t u0 = cpu->usage0;

	اگر (u0 != 0) {
		uint32_t ادوار = usage_now() - u0;

		اگر (cpu->current->بنیاد.usage.track_usage) {
			sched_thread_update_usage(cpu->current, ادوار);
		}

		sched_cpu_update_usage(cpu, ادوار);
	}

	cpu->usage0 = 0;
	k_spin_unlock(&usage_تالا, k);
}

#اگر_تعریف_شدہ CONFIG_SCHED_THREAD_USAGE_ALL
خالی z_sched_cpu_usage(uint8_t cpu_id, ساخت k_thread_runtime_stats *stats)
{
	k_spinlock_کلید_t  کلید;
	ساخت _cpu *cpu;
	uint32_t pending = 0U;
	bool pending_is_idle = false;

	کلید = k_spin_تالا(&usage_تالا);
	cpu = &_kernel.cpus[cpu_id];

	اگر (cpu == _current_cpu) {
		uint32_t  now = usage_now();
		uint32_t ادوار = now - cpu->usage0;

		/*
		 * Getting stats for the current CPU. Update both its
		 * current thread stats and the CPU stats as the CPU's
		 * [usage0] field will also get updated. This keeps all
		 * that information up-to-date.
		 */

		اگر (cpu->current->بنیاد.usage.track_usage) {
			sched_thread_update_usage(cpu->current, ادوار);
		}

		sched_cpu_update_usage(cpu, ادوار);

		cpu->usage0 = now;
	} ورنہ اگر (cpu->usage->track_usage) {
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

	stats->کل_ادوار     = cpu->usage->کل +
				  (pending_is_idle ? 0U : pending);
#اگر_تعریف_شدہ CONFIG_SCHED_THREAD_USAGE_ANALYSIS
	stats->current_ادوار   = cpu->usage->current;
	stats->peak_ادوار      = cpu->usage->longest;

	اگر (cpu->usage->عدد_windows == 0) {
		stats->اوسط_قدر_ادوار = 0;
	} ورنہ {
		stats->اوسط_قدر_ادوار = stats->کل_ادوار /
					cpu->usage->عدد_windows;
	}
#اگر_ختم /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */

	stats->idle_ادوار =
		_kernel.cpus[cpu_id].idle_thread->بنیاد.usage.کل +
		(pending_is_idle ? pending : 0U);

	stats->execution_ادوار = stats->کل_ادوار + stats->idle_ادوار;

	k_spin_unlock(&usage_تالا, کلید);
}
#اگر_ختم /* CONFIG_SCHED_THREAD_USAGE_ALL */

خالی z_sched_thread_usage(ساخت k_thread *thread,
			  ساخت k_thread_runtime_stats *stats)
{
	ساخت _cpu *cpu;
	k_spinlock_کلید_t  کلید;

	کلید = k_spin_تالا(&usage_تالا);
	cpu = _current_cpu;


	اگر (thread == cpu->current) {
		uint32_t now = usage_now();
		uint32_t ادوار = now - cpu->usage0;

		/*
		 * Getting stats for the current thread. Update both the
		 * current thread stats and its CPU stats as the CPU's
		 * [usage0] field will also get updated. This keeps all
		 * that information up-to-date.
		 */

		اگر (thread->بنیاد.usage.track_usage) {
			sched_thread_update_usage(thread, ادوار);
		}

		sched_cpu_update_usage(cpu, ادوار);

		cpu->usage0 = now;
	}

	stats->execution_ادوار = thread->بنیاد.usage.کل;
	stats->کل_ادوار     = thread->بنیاد.usage.کل;

	/* Copy-out the thread's usage stats */

#اگر_تعریف_شدہ CONFIG_SCHED_THREAD_USAGE_ANALYSIS
	stats->current_ادوار = thread->بنیاد.usage.current;
	stats->peak_ادوار    = thread->بنیاد.usage.longest;

	اگر (thread->بنیاد.usage.عدد_windows == 0) {
		stats->اوسط_قدر_ادوار = 0;
	} ورنہ {
		stats->اوسط_قدر_ادوار = stats->کل_ادوار /
					thread->بنیاد.usage.عدد_windows;
	}
#اگر_ختم /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */

#اگر_تعریف_شدہ CONFIG_SCHED_THREAD_USAGE_ALL
	stats->idle_ادوار = 0;
#اگر_ختم /* CONFIG_SCHED_THREAD_USAGE_ALL */

	k_spin_unlock(&usage_تالا, کلید);
}

#اگر_تعریف_شدہ CONFIG_SCHED_THREAD_USAGE_ANALYSIS
صحیح k_thread_runtime_stats_enable(k_tid_t  thread)
{
	k_spinlock_کلید_t  کلید;

	CHECKIF(thread == تہی) {
		واپس -EINVAL;
	}

	کلید = k_spin_تالا(&usage_تالا);
	ساخت _cpu *cpu = _current_cpu;

	اگر (!thread->بنیاد.usage.track_usage) {
		thread->بنیاد.usage.track_usage = true;
		thread->بنیاد.usage.عدد_windows++;
		thread->بنیاد.usage.current = 0;

		اگر (thread == cpu->current) {
			uint32_t now = usage_now();

			sched_cpu_update_usage(cpu, now - cpu->usage0);

			cpu->usage0 = now;
		}
	}

	k_spin_unlock(&usage_تالا, کلید);

	واپس 0;
}

صحیح k_thread_runtime_stats_disable(k_tid_t  thread)
{
	k_spinlock_کلید_t کلید;

	CHECKIF(thread == تہی) {
		واپس -EINVAL;
	}

	کلید = k_spin_تالا(&usage_تالا);
	ساخت _cpu *cpu = _current_cpu;

	اگر (thread->بنیاد.usage.track_usage) {
		thread->بنیاد.usage.track_usage = false;

		اگر (thread == cpu->current) {
			uint32_t now = usage_now();
			uint32_t ادوار = now - cpu->usage0;

			sched_thread_update_usage(thread, ادوار);
			sched_cpu_update_usage(cpu, ادوار);

			cpu->usage0 = now;
		}
	}

	k_spin_unlock(&usage_تالا, کلید);

	واپس 0;
}

bool k_thread_runtime_stats_is_enabled(k_tid_t thread)
{
	واپس thread->بنیاد.usage.track_usage;
}
#اگر_ختم /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */

#اگر_تعریف_شدہ CONFIG_SCHED_THREAD_USAGE_ALL
خالی k_sys_runtime_stats_enable(خالی)
{
	k_spinlock_کلید_t  کلید;

	کلید = k_spin_تالا(&usage_تالا);

	اگر (_current_cpu->usage->track_usage) {

		/*
		 * Usage tracking is already enabled on the current CPU
		 * and thus on all other CPUs (if applicable). There is
		 * nothing left to do.
		 */

		k_spin_unlock(&usage_تالا, کلید);
		واپس;
	}

	/* Enable gathering of runtime stats on each CPU */

	غیرعلامتی صحیح عدد_cpus = arch_عدد_cpus();

	برائے (غیرعلامتی صحیح i = 0; i < عدد_cpus; i++) {
		_kernel.cpus[i].usage->track_usage = true;
#اگر_تعریف_شدہ CONFIG_SCHED_THREAD_USAGE_ANALYSIS
		_kernel.cpus[i].usage->عدد_windows++;
		_kernel.cpus[i].usage->current = 0;
#اگر_ختم /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */
	}

	k_spin_unlock(&usage_تالا, کلید);
}

خالی k_sys_runtime_stats_disable(خالی)
{
	ساخت _cpu *cpu;
	k_spinlock_کلید_t کلید;

	کلید = k_spin_تالا(&usage_تالا);

	اگر (!_current_cpu->usage->track_usage) {

		/*
		 * Usage tracking is already disabled on the current CPU
		 * and thus on all other CPUs (if applicable). There is
		 * nothing left to do.
		 */

		k_spin_unlock(&usage_تالا, کلید);
		واپس;
	}

	uint32_t now = usage_now();

	غیرعلامتی صحیح عدد_cpus = arch_عدد_cpus();

	برائے (غیرعلامتی صحیح i = 0; i < عدد_cpus; i++) {
		cpu = &_kernel.cpus[i];
		اگر (cpu->usage0 != 0) {
			sched_cpu_update_usage(cpu, now - cpu->usage0);
		}
		cpu->usage->track_usage = false;
	}

	k_spin_unlock(&usage_تالا, کلید);
}
#اگر_ختم /* CONFIG_SCHED_THREAD_USAGE_ALL */

#اگر_تعریف_شدہ CONFIG_OBJ_CORE_STATS_THREAD
صحیح z_thread_stats_raw(ساخت k_obj_core *obj_core, خالی *stats)
{
	k_spinlock_کلید_t  کلید;

	کلید = k_spin_تالا(&usage_تالا);
	حافظہ_نقل(stats, obj_core->stats, حجم(ساخت k_چکر_stats));
	k_spin_unlock(&usage_تالا, کلید);

	واپس 0;
}

صحیح z_thread_stats_query(ساخت k_obj_core *obj_core, خالی *stats)
{
	ساخت k_thread *thread;

	thread = CONTAINER_کا(obj_core, ساخت k_thread, obj_core);

	z_sched_thread_usage(thread, stats);

	واپس 0;
}

صحیح z_thread_stats_reset(ساخت k_obj_core *obj_core)
{
	k_spinlock_کلید_t  کلید;
	ساخت k_چکر_stats  *stats;
	ساخت k_thread *thread;

	thread = CONTAINER_کا(obj_core, ساخت k_thread, obj_core);
	کلید = k_spin_تالا(&usage_تالا);
	stats = obj_core->stats;

	stats->کل = 0ULL;
#اگر_تعریف_شدہ CONFIG_SCHED_THREAD_USAGE_ANALYSIS
	stats->current = 0ULL;
	stats->longest = 0ULL;
	stats->عدد_windows = (thread->بنیاد.usage.track_usage) ?  1U : 0U;
#اگر_ختم /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */

	اگر (thread != _current_cpu->current) {

		/*
		 * If the thread is not running, there is nothing else to do.
		 * If the thread is running on another core, then it is not
		 * safe to do anything else but unlock and return (and pretend
		 * that its stats were reset at the start of its execution
		 * window.
		 */

		k_spin_unlock(&usage_تالا, کلید);

		واپس 0;
	}

	/* Update the current CPU stats. */

	uint32_t now = usage_now();

	sched_cpu_update_usage(_current_cpu, now - _current_cpu->usage0);

	_current_cpu->usage0 = now;

	k_spin_unlock(&usage_تالا, کلید);

	واپس 0;
}

صحیح z_thread_stats_disable(ساخت k_obj_core *obj_core)
{
#اگر_تعریف_شدہ CONFIG_SCHED_THREAD_USAGE_ANALYSIS
	ساخت k_thread *thread;

	thread = CONTAINER_کا(obj_core, ساخت k_thread, obj_core);

	واپس k_thread_runtime_stats_disable(thread);
#ورنہ
	واپس -ENOTSUP;
#اگر_ختم /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */
}

صحیح z_thread_stats_enable(ساخت k_obj_core *obj_core)
{
#اگر_تعریف_شدہ CONFIG_SCHED_THREAD_USAGE_ANALYSIS
	ساخت k_thread *thread;

	thread = CONTAINER_کا(obj_core, ساخت k_thread, obj_core);

	واپس k_thread_runtime_stats_enable(thread);
#ورنہ
	واپس -ENOTSUP;
#اگر_ختم /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */
}
#اگر_ختم /* CONFIG_OBJ_CORE_STATS_THREAD */

#اگر_تعریف_شدہ CONFIG_OBJ_CORE_STATS_SYSTEM
صحیح z_cpu_stats_raw(ساخت k_obj_core *obj_core, خالی *stats)
{
	k_spinlock_کلید_t  کلید;

	کلید = k_spin_تالا(&usage_تالا);
	حافظہ_نقل(stats, obj_core->stats, حجم(ساخت k_چکر_stats));
	k_spin_unlock(&usage_تالا, کلید);

	واپس 0;
}

صحیح z_cpu_stats_query(ساخت k_obj_core *obj_core, خالی *stats)
{
	ساخت _cpu  *cpu;

	cpu = CONTAINER_کا(obj_core, ساخت _cpu, obj_core);

	z_sched_cpu_usage(cpu->id, stats);

	واپس 0;
}
#اگر_ختم /* CONFIG_OBJ_CORE_STATS_SYSTEM */

#اگر_تعریف_شدہ CONFIG_OBJ_CORE_STATS_SYSTEM
صحیح z_kernel_stats_raw(ساخت k_obj_core *obj_core, خالی *stats)
{
	k_spinlock_کلید_t  کلید;

	کلید = k_spin_تالا(&usage_تالا);
	حافظہ_نقل(stats, obj_core->stats,
	       CONFIG_MP_MAX_عدد_CPUS * حجم(ساخت k_چکر_stats));
	k_spin_unlock(&usage_تالا, کلید);

	واپس 0;
}

صحیح z_kernel_stats_query(ساخت k_obj_core *obj_core, خالی *stats)
{
	ARG_غیر_مستعمل(obj_core);

	واپس k_thread_runtime_stats_all_get(stats);
}
#اگر_ختم /* CONFIG_OBJ_CORE_STATS_SYSTEM */
