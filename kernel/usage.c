/*
 * Copyright (c) 2018 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#అన్తర్భావ <zephyr/kernel.h>

#అన్తర్భావ <zephyr/timing/timing.h>
#అన్తర్భావ <ksched.h>
#అన్తర్భావ <zephyr/spinlock.h>
#అన్తర్భావ <zephyr/sys/check.h>

/* Need one of these for this to work */
#ఒకవేళ !defined(CONFIG_USE_SWITCH) && !defined(CONFIG_INSTRUMENT_THREAD_SWITCHING)
#దోషం "No data backend configured for CONFIG_SCHED_THREAD_USAGE"
#అయితే_ముగింపు /* !CONFIG_USE_SWITCH && !CONFIG_INSTRUMENT_THREAD_SWITCHING */

స్థిర నిర్మాణం k_spinlock usage_తాళం;

స్థిర uint32_t usage_now(శూన్య)
{
	uint32_t now;

#నిర్వచితమైతే CONFIG_THREAD_RUNTIME_STATS_USE_TIMING_FUNCTIONS
	now = (uint32_t)timing_counter_get();
#లేకపోతే
	now = k_చక్రం_get_32();
#అయితే_ముగింపు /* CONFIG_THREAD_RUNTIME_STATS_USE_TIMING_FUNCTIONS */

	/* Edge case: we use a zero as a null ("stop() already called") */
	తిరిగి (now == 0) ? 1 : now;
}

#నిర్వచితమైతే CONFIG_SCHED_THREAD_USAGE_ALL
స్థిర శూన్య sched_cpu_update_usage(నిర్మాణం _cpu *cpu, uint32_t చక్రాలు)
{
	ఒకవేళ (!cpu->usage->track_usage) {
		తిరిగి;
	}

	ఒకవేళ (cpu->current != cpu->idle_thread) {
		cpu->usage->మొత్తం += చక్రాలు;

#నిర్వచితమైతే CONFIG_SCHED_THREAD_USAGE_ANALYSIS
		cpu->usage->current += చక్రాలు;

		ఒకవేళ (cpu->usage->longest < cpu->usage->current) {
			cpu->usage->longest = cpu->usage->current;
		}
	} లేకపోతే {
		cpu->usage->current = 0;
		cpu->usage->అంకె_windows++;
#అయితే_ముగింపు /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */
	}
}
#లేకపోతే
#నిర్వచించు sched_cpu_update_usage(cpu, చక్రాలు)   చేయి { } అయితే (0)
#అయితే_ముగింపు /* CONFIG_SCHED_THREAD_USAGE_ALL */

స్థిర శూన్య sched_thread_update_usage(నిర్మాణం k_thread *thread, uint32_t చక్రాలు)
{
	thread->పునాది.usage.మొత్తం += చక్రాలు;

#నిర్వచితమైతే CONFIG_SCHED_THREAD_USAGE_ANALYSIS
	thread->పునాది.usage.current += చక్రాలు;

	ఒకవేళ (thread->పునాది.usage.longest < thread->పునాది.usage.current) {
		thread->పునాది.usage.longest = thread->పునాది.usage.current;
	}
#అయితే_ముగింపు /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */
}

శూన్య z_sched_usage_ప్రారంభం(నిర్మాణం k_thread *thread)
{
#నిర్వచితమైతే CONFIG_SCHED_THREAD_USAGE_ANALYSIS
	k_spinlock_కీలకం_t  కీలకం;

	కీలకం = k_spin_తాళం(&usage_తాళం);

	_current_cpu->usage0 = usage_now();   /* Always update */

	ఒకవేళ (thread->పునాది.usage.track_usage) {
		thread->పునాది.usage.అంకె_windows++;
		thread->పునాది.usage.current = 0;
	}

	k_spin_unlock(&usage_తాళం, కీలకం);
#లేకపోతే
	/* One write through a volatile pointer doesn't require
	 * synchronization as long as _usage() treats it as volatile
	 * (we can't race with _stop() by design).
	 */

	_current_cpu->usage0 = usage_now();
#అయితే_ముగింపు /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */
}

శూన్య z_sched_usage_stop(శూన్య)
{
	నిర్మాణం _cpu *cpu = _current_cpu;

	/* Only this CPU writes usage0, and callers keep local IRQs masked. */
	ఒకవేళ (cpu->usage0 == 0U) {
		తిరిగి;
	}

	k_spinlock_కీలకం_t k = k_spin_తాళం(&usage_తాళం);

	uint32_t u0 = cpu->usage0;

	ఒకవేళ (u0 != 0) {
		uint32_t చక్రాలు = usage_now() - u0;

		ఒకవేళ (cpu->current->పునాది.usage.track_usage) {
			sched_thread_update_usage(cpu->current, చక్రాలు);
		}

		sched_cpu_update_usage(cpu, చక్రాలు);
	}

	cpu->usage0 = 0;
	k_spin_unlock(&usage_తాళం, k);
}

#నిర్వచితమైతే CONFIG_SCHED_THREAD_USAGE_ALL
శూన్య z_sched_cpu_usage(uint8_t cpu_id, నిర్మాణం k_thread_runtime_stats *stats)
{
	k_spinlock_కీలకం_t  కీలకం;
	నిర్మాణం _cpu *cpu;
	uint32_t pending = 0U;
	bool pending_is_idle = false;

	కీలకం = k_spin_తాళం(&usage_తాళం);
	cpu = &_kernel.cpus[cpu_id];

	ఒకవేళ (cpu == _current_cpu) {
		uint32_t  now = usage_now();
		uint32_t చక్రాలు = now - cpu->usage0;

		/*
		 * Getting stats for the current CPU. Update both its
		 * current thread stats and the CPU stats as the CPU's
		 * [usage0] field will also get updated. This keeps all
		 * that information up-to-date.
		 */

		ఒకవేళ (cpu->current->పునాది.usage.track_usage) {
			sched_thread_update_usage(cpu->current, చక్రాలు);
		}

		sched_cpu_update_usage(cpu, చక్రాలు);

		cpu->usage0 = now;
	} లేకపోతే ఒకవేళ (cpu->usage->track_usage) {
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

	stats->మొత్తం_చక్రాలు     = cpu->usage->మొత్తం +
				  (pending_is_idle ? 0U : pending);
#నిర్వచితమైతే CONFIG_SCHED_THREAD_USAGE_ANALYSIS
	stats->current_చక్రాలు   = cpu->usage->current;
	stats->peak_చక్రాలు      = cpu->usage->longest;

	ఒకవేళ (cpu->usage->అంకె_windows == 0) {
		stats->సరాసరి_చక్రాలు = 0;
	} లేకపోతే {
		stats->సరాసరి_చక్రాలు = stats->మొత్తం_చక్రాలు /
					cpu->usage->అంకె_windows;
	}
#అయితే_ముగింపు /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */

	stats->idle_చక్రాలు =
		_kernel.cpus[cpu_id].idle_thread->పునాది.usage.మొత్తం +
		(pending_is_idle ? pending : 0U);

	stats->execution_చక్రాలు = stats->మొత్తం_చక్రాలు + stats->idle_చక్రాలు;

	k_spin_unlock(&usage_తాళం, కీలకం);
}
#అయితే_ముగింపు /* CONFIG_SCHED_THREAD_USAGE_ALL */

శూన్య z_sched_thread_usage(నిర్మాణం k_thread *thread,
			  నిర్మాణం k_thread_runtime_stats *stats)
{
	నిర్మాణం _cpu *cpu;
	k_spinlock_కీలకం_t  కీలకం;

	కీలకం = k_spin_తాళం(&usage_తాళం);
	cpu = _current_cpu;


	ఒకవేళ (thread == cpu->current) {
		uint32_t now = usage_now();
		uint32_t చక్రాలు = now - cpu->usage0;

		/*
		 * Getting stats for the current thread. Update both the
		 * current thread stats and its CPU stats as the CPU's
		 * [usage0] field will also get updated. This keeps all
		 * that information up-to-date.
		 */

		ఒకవేళ (thread->పునాది.usage.track_usage) {
			sched_thread_update_usage(thread, చక్రాలు);
		}

		sched_cpu_update_usage(cpu, చక్రాలు);

		cpu->usage0 = now;
	}

	stats->execution_చక్రాలు = thread->పునాది.usage.మొత్తం;
	stats->మొత్తం_చక్రాలు     = thread->పునాది.usage.మొత్తం;

	/* Copy-out the thread's usage stats */

#నిర్వచితమైతే CONFIG_SCHED_THREAD_USAGE_ANALYSIS
	stats->current_చక్రాలు = thread->పునాది.usage.current;
	stats->peak_చక్రాలు    = thread->పునాది.usage.longest;

	ఒకవేళ (thread->పునాది.usage.అంకె_windows == 0) {
		stats->సరాసరి_చక్రాలు = 0;
	} లేకపోతే {
		stats->సరాసరి_చక్రాలు = stats->మొత్తం_చక్రాలు /
					thread->పునాది.usage.అంకె_windows;
	}
#అయితే_ముగింపు /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */

#నిర్వచితమైతే CONFIG_SCHED_THREAD_USAGE_ALL
	stats->idle_చక్రాలు = 0;
#అయితే_ముగింపు /* CONFIG_SCHED_THREAD_USAGE_ALL */

	k_spin_unlock(&usage_తాళం, కీలకం);
}

#నిర్వచితమైతే CONFIG_SCHED_THREAD_USAGE_ANALYSIS
పూర్ణాంకం k_thread_runtime_stats_enable(k_tid_t  thread)
{
	k_spinlock_కీలకం_t  కీలకం;

	CHECKIF(thread == ఖాళీ) {
		తిరిగి -EINVAL;
	}

	కీలకం = k_spin_తాళం(&usage_తాళం);
	నిర్మాణం _cpu *cpu = _current_cpu;

	ఒకవేళ (!thread->పునాది.usage.track_usage) {
		thread->పునాది.usage.track_usage = true;
		thread->పునాది.usage.అంకె_windows++;
		thread->పునాది.usage.current = 0;

		ఒకవేళ (thread == cpu->current) {
			uint32_t now = usage_now();

			sched_cpu_update_usage(cpu, now - cpu->usage0);

			cpu->usage0 = now;
		}
	}

	k_spin_unlock(&usage_తాళం, కీలకం);

	తిరిగి 0;
}

పూర్ణాంకం k_thread_runtime_stats_disable(k_tid_t  thread)
{
	k_spinlock_కీలకం_t కీలకం;

	CHECKIF(thread == ఖాళీ) {
		తిరిగి -EINVAL;
	}

	కీలకం = k_spin_తాళం(&usage_తాళం);
	నిర్మాణం _cpu *cpu = _current_cpu;

	ఒకవేళ (thread->పునాది.usage.track_usage) {
		thread->పునాది.usage.track_usage = false;

		ఒకవేళ (thread == cpu->current) {
			uint32_t now = usage_now();
			uint32_t చక్రాలు = now - cpu->usage0;

			sched_thread_update_usage(thread, చక్రాలు);
			sched_cpu_update_usage(cpu, చక్రాలు);

			cpu->usage0 = now;
		}
	}

	k_spin_unlock(&usage_తాళం, కీలకం);

	తిరిగి 0;
}

bool k_thread_runtime_stats_is_enabled(k_tid_t thread)
{
	తిరిగి thread->పునాది.usage.track_usage;
}
#అయితే_ముగింపు /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */

#నిర్వచితమైతే CONFIG_SCHED_THREAD_USAGE_ALL
శూన్య k_sys_runtime_stats_enable(శూన్య)
{
	k_spinlock_కీలకం_t  కీలకం;

	కీలకం = k_spin_తాళం(&usage_తాళం);

	ఒకవేళ (_current_cpu->usage->track_usage) {

		/*
		 * Usage tracking is already enabled on the current CPU
		 * and thus on all other CPUs (if applicable). There is
		 * nothing left to do.
		 */

		k_spin_unlock(&usage_తాళం, కీలకం);
		తిరిగి;
	}

	/* Enable gathering of runtime stats on each CPU */

	గుర్తులేని పూర్ణాంకం అంకె_cpus = arch_అంకె_cpus();

	కొరకు (గుర్తులేని పూర్ణాంకం i = 0; i < అంకె_cpus; i++) {
		_kernel.cpus[i].usage->track_usage = true;
#నిర్వచితమైతే CONFIG_SCHED_THREAD_USAGE_ANALYSIS
		_kernel.cpus[i].usage->అంకె_windows++;
		_kernel.cpus[i].usage->current = 0;
#అయితే_ముగింపు /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */
	}

	k_spin_unlock(&usage_తాళం, కీలకం);
}

శూన్య k_sys_runtime_stats_disable(శూన్య)
{
	నిర్మాణం _cpu *cpu;
	k_spinlock_కీలకం_t కీలకం;

	కీలకం = k_spin_తాళం(&usage_తాళం);

	ఒకవేళ (!_current_cpu->usage->track_usage) {

		/*
		 * Usage tracking is already disabled on the current CPU
		 * and thus on all other CPUs (if applicable). There is
		 * nothing left to do.
		 */

		k_spin_unlock(&usage_తాళం, కీలకం);
		తిరిగి;
	}

	uint32_t now = usage_now();

	గుర్తులేని పూర్ణాంకం అంకె_cpus = arch_అంకె_cpus();

	కొరకు (గుర్తులేని పూర్ణాంకం i = 0; i < అంకె_cpus; i++) {
		cpu = &_kernel.cpus[i];
		ఒకవేళ (cpu->usage0 != 0) {
			sched_cpu_update_usage(cpu, now - cpu->usage0);
		}
		cpu->usage->track_usage = false;
	}

	k_spin_unlock(&usage_తాళం, కీలకం);
}
#అయితే_ముగింపు /* CONFIG_SCHED_THREAD_USAGE_ALL */

#నిర్వచితమైతే CONFIG_OBJ_CORE_STATS_THREAD
పూర్ణాంకం z_thread_stats_raw(నిర్మాణం k_obj_core *obj_core, శూన్య *stats)
{
	k_spinlock_కీలకం_t  కీలకం;

	కీలకం = k_spin_తాళం(&usage_తాళం);
	జ్ఞాపకం_నకలు(stats, obj_core->stats, పరిమాణం(నిర్మాణం k_చక్రం_stats));
	k_spin_unlock(&usage_తాళం, కీలకం);

	తిరిగి 0;
}

పూర్ణాంకం z_thread_stats_query(నిర్మాణం k_obj_core *obj_core, శూన్య *stats)
{
	నిర్మాణం k_thread *thread;

	thread = CONTAINER_యొక్క(obj_core, నిర్మాణం k_thread, obj_core);

	z_sched_thread_usage(thread, stats);

	తిరిగి 0;
}

పూర్ణాంకం z_thread_stats_reset(నిర్మాణం k_obj_core *obj_core)
{
	k_spinlock_కీలకం_t  కీలకం;
	నిర్మాణం k_చక్రం_stats  *stats;
	నిర్మాణం k_thread *thread;

	thread = CONTAINER_యొక్క(obj_core, నిర్మాణం k_thread, obj_core);
	కీలకం = k_spin_తాళం(&usage_తాళం);
	stats = obj_core->stats;

	stats->మొత్తం = 0ULL;
#నిర్వచితమైతే CONFIG_SCHED_THREAD_USAGE_ANALYSIS
	stats->current = 0ULL;
	stats->longest = 0ULL;
	stats->అంకె_windows = (thread->పునాది.usage.track_usage) ?  1U : 0U;
#అయితే_ముగింపు /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */

	ఒకవేళ (thread != _current_cpu->current) {

		/*
		 * If the thread is not running, there is nothing else to do.
		 * If the thread is running on another core, then it is not
		 * safe to do anything else but unlock and return (and pretend
		 * that its stats were reset at the start of its execution
		 * window.
		 */

		k_spin_unlock(&usage_తాళం, కీలకం);

		తిరిగి 0;
	}

	/* Update the current CPU stats. */

	uint32_t now = usage_now();

	sched_cpu_update_usage(_current_cpu, now - _current_cpu->usage0);

	_current_cpu->usage0 = now;

	k_spin_unlock(&usage_తాళం, కీలకం);

	తిరిగి 0;
}

పూర్ణాంకం z_thread_stats_disable(నిర్మాణం k_obj_core *obj_core)
{
#నిర్వచితమైతే CONFIG_SCHED_THREAD_USAGE_ANALYSIS
	నిర్మాణం k_thread *thread;

	thread = CONTAINER_యొక్క(obj_core, నిర్మాణం k_thread, obj_core);

	తిరిగి k_thread_runtime_stats_disable(thread);
#లేకపోతే
	తిరిగి -ENOTSUP;
#అయితే_ముగింపు /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */
}

పూర్ణాంకం z_thread_stats_enable(నిర్మాణం k_obj_core *obj_core)
{
#నిర్వచితమైతే CONFIG_SCHED_THREAD_USAGE_ANALYSIS
	నిర్మాణం k_thread *thread;

	thread = CONTAINER_యొక్క(obj_core, నిర్మాణం k_thread, obj_core);

	తిరిగి k_thread_runtime_stats_enable(thread);
#లేకపోతే
	తిరిగి -ENOTSUP;
#అయితే_ముగింపు /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */
}
#అయితే_ముగింపు /* CONFIG_OBJ_CORE_STATS_THREAD */

#నిర్వచితమైతే CONFIG_OBJ_CORE_STATS_SYSTEM
పూర్ణాంకం z_cpu_stats_raw(నిర్మాణం k_obj_core *obj_core, శూన్య *stats)
{
	k_spinlock_కీలకం_t  కీలకం;

	కీలకం = k_spin_తాళం(&usage_తాళం);
	జ్ఞాపకం_నకలు(stats, obj_core->stats, పరిమాణం(నిర్మాణం k_చక్రం_stats));
	k_spin_unlock(&usage_తాళం, కీలకం);

	తిరిగి 0;
}

పూర్ణాంకం z_cpu_stats_query(నిర్మాణం k_obj_core *obj_core, శూన్య *stats)
{
	నిర్మాణం _cpu  *cpu;

	cpu = CONTAINER_యొక్క(obj_core, నిర్మాణం _cpu, obj_core);

	z_sched_cpu_usage(cpu->id, stats);

	తిరిగి 0;
}
#అయితే_ముగింపు /* CONFIG_OBJ_CORE_STATS_SYSTEM */

#నిర్వచితమైతే CONFIG_OBJ_CORE_STATS_SYSTEM
పూర్ణాంకం z_kernel_stats_raw(నిర్మాణం k_obj_core *obj_core, శూన్య *stats)
{
	k_spinlock_కీలకం_t  కీలకం;

	కీలకం = k_spin_తాళం(&usage_తాళం);
	జ్ఞాపకం_నకలు(stats, obj_core->stats,
	       CONFIG_MP_MAX_అంకె_CPUS * పరిమాణం(నిర్మాణం k_చక్రం_stats));
	k_spin_unlock(&usage_తాళం, కీలకం);

	తిరిగి 0;
}

పూర్ణాంకం z_kernel_stats_query(నిర్మాణం k_obj_core *obj_core, శూన్య *stats)
{
	ARG_వాడని(obj_core);

	తిరిగి k_thread_runtime_stats_all_get(stats);
}
#అయితే_ముగింపు /* CONFIG_OBJ_CORE_STATS_SYSTEM */
