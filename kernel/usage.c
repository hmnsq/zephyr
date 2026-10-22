/*
 * Copyright (c) 2018 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/kernel.h>

#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/timing/timing.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <ksched.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/spinlock.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/sys/check.h>

/* Need one of these for this to work */
#ਜੇ !defined(CONFIG_USE_SWITCH) && !defined(CONFIG_INSTRUMENT_THREAD_SWITCHING)
#ਗਲਤੀ "No data backend configured for CONFIG_SCHED_THREAD_USAGE"
#ਜੇ_ਅੰਤ /* !CONFIG_USE_SWITCH && !CONFIG_INSTRUMENT_THREAD_SWITCHING */

ਸਥਿਰ ਬਣਤਰ k_spinlock usage_ਤਾਲਾ;

ਸਥਿਰ uint32_t usage_now(ਖਾਲੀ)
{
	uint32_t now;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_THREAD_RUNTIME_STATS_USE_TIMING_FUNCTIONS
	now = (uint32_t)timing_counter_get();
#ਨਹੀਂਤਾਂ
	now = k_ਚੱਕਰ_get_32();
#ਜੇ_ਅੰਤ /* CONFIG_THREAD_RUNTIME_STATS_USE_TIMING_FUNCTIONS */

	/* Edge case: we use a zero as a null ("stop() already called") */
	ਵਾਪਸ (now == 0) ? 1 : now;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SCHED_THREAD_USAGE_ALL
ਸਥਿਰ ਖਾਲੀ sched_cpu_update_usage(ਬਣਤਰ _cpu *cpu, uint32_t ਚੱਕਰਾਂ)
{
	ਜੇ (!cpu->usage->track_usage) {
		ਵਾਪਸ;
	}

	ਜੇ (cpu->current != cpu->idle_thread) {
		cpu->usage->ਕੁੱਲ += ਚੱਕਰਾਂ;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SCHED_THREAD_USAGE_ANALYSIS
		cpu->usage->current += ਚੱਕਰਾਂ;

		ਜੇ (cpu->usage->longest < cpu->usage->current) {
			cpu->usage->longest = cpu->usage->current;
		}
	} ਨਹੀਂਤਾਂ {
		cpu->usage->current = 0;
		cpu->usage->ਅੰਕ_windows++;
#ਜੇ_ਅੰਤ /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */
	}
}
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sched_cpu_update_usage(cpu, ਚੱਕਰਾਂ)   ਕਰੋ { } ਜਦੋਂ (0)
#ਜੇ_ਅੰਤ /* CONFIG_SCHED_THREAD_USAGE_ALL */

ਸਥਿਰ ਖਾਲੀ sched_thread_update_usage(ਬਣਤਰ k_thread *thread, uint32_t ਚੱਕਰਾਂ)
{
	thread->ਅਧਾਰ.usage.ਕੁੱਲ += ਚੱਕਰਾਂ;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SCHED_THREAD_USAGE_ANALYSIS
	thread->ਅਧਾਰ.usage.current += ਚੱਕਰਾਂ;

	ਜੇ (thread->ਅਧਾਰ.usage.longest < thread->ਅਧਾਰ.usage.current) {
		thread->ਅਧਾਰ.usage.longest = thread->ਅਧਾਰ.usage.current;
	}
#ਜੇ_ਅੰਤ /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */
}

ਖਾਲੀ z_sched_usage_ਸ਼ੁਰੂ(ਬਣਤਰ k_thread *thread)
{
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SCHED_THREAD_USAGE_ANALYSIS
	k_spinlock_ਕੁੰਜੀ_t  ਕੁੰਜੀ;

	ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&usage_ਤਾਲਾ);

	_current_cpu->usage0 = usage_now();   /* Always update */

	ਜੇ (thread->ਅਧਾਰ.usage.track_usage) {
		thread->ਅਧਾਰ.usage.ਅੰਕ_windows++;
		thread->ਅਧਾਰ.usage.current = 0;
	}

	k_spin_unlock(&usage_ਤਾਲਾ, ਕੁੰਜੀ);
#ਨਹੀਂਤਾਂ
	/* One write through a volatile pointer doesn't require
	 * synchronization as long as _usage() treats it as volatile
	 * (we can't race with _stop() by design).
	 */

	_current_cpu->usage0 = usage_now();
#ਜੇ_ਅੰਤ /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */
}

ਖਾਲੀ z_sched_usage_stop(ਖਾਲੀ)
{
	ਬਣਤਰ _cpu *cpu = _current_cpu;

	/* Only this CPU writes usage0, and callers keep local IRQs masked. */
	ਜੇ (cpu->usage0 == 0U) {
		ਵਾਪਸ;
	}

	k_spinlock_ਕੁੰਜੀ_t k = k_spin_ਤਾਲਾ(&usage_ਤਾਲਾ);

	uint32_t u0 = cpu->usage0;

	ਜੇ (u0 != 0) {
		uint32_t ਚੱਕਰਾਂ = usage_now() - u0;

		ਜੇ (cpu->current->ਅਧਾਰ.usage.track_usage) {
			sched_thread_update_usage(cpu->current, ਚੱਕਰਾਂ);
		}

		sched_cpu_update_usage(cpu, ਚੱਕਰਾਂ);
	}

	cpu->usage0 = 0;
	k_spin_unlock(&usage_ਤਾਲਾ, k);
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SCHED_THREAD_USAGE_ALL
ਖਾਲੀ z_sched_cpu_usage(uint8_t cpu_id, ਬਣਤਰ k_thread_runtime_stats *stats)
{
	k_spinlock_ਕੁੰਜੀ_t  ਕੁੰਜੀ;
	ਬਣਤਰ _cpu *cpu;
	uint32_t pending = 0U;
	bool pending_is_idle = false;

	ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&usage_ਤਾਲਾ);
	cpu = &_kernel.cpus[cpu_id];

	ਜੇ (cpu == _current_cpu) {
		uint32_t  now = usage_now();
		uint32_t ਚੱਕਰਾਂ = now - cpu->usage0;

		/*
		 * Getting stats for the current CPU. Update both its
		 * current thread stats and the CPU stats as the CPU's
		 * [usage0] field will also get updated. This keeps all
		 * that information up-to-date.
		 */

		ਜੇ (cpu->current->ਅਧਾਰ.usage.track_usage) {
			sched_thread_update_usage(cpu->current, ਚੱਕਰਾਂ);
		}

		sched_cpu_update_usage(cpu, ਚੱਕਰਾਂ);

		cpu->usage0 = now;
	} ਨਹੀਂਤਾਂ ਜੇ (cpu->usage->track_usage) {
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

	stats->ਕੁੱਲ_ਚੱਕਰਾਂ     = cpu->usage->ਕੁੱਲ +
				  (pending_is_idle ? 0U : pending);
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SCHED_THREAD_USAGE_ANALYSIS
	stats->current_ਚੱਕਰਾਂ   = cpu->usage->current;
	stats->peak_ਚੱਕਰਾਂ      = cpu->usage->longest;

	ਜੇ (cpu->usage->ਅੰਕ_windows == 0) {
		stats->ਔਸਤਨ_ਚੱਕਰਾਂ = 0;
	} ਨਹੀਂਤਾਂ {
		stats->ਔਸਤਨ_ਚੱਕਰਾਂ = stats->ਕੁੱਲ_ਚੱਕਰਾਂ /
					cpu->usage->ਅੰਕ_windows;
	}
#ਜੇ_ਅੰਤ /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */

	stats->idle_ਚੱਕਰਾਂ =
		_kernel.cpus[cpu_id].idle_thread->ਅਧਾਰ.usage.ਕੁੱਲ +
		(pending_is_idle ? pending : 0U);

	stats->execution_ਚੱਕਰਾਂ = stats->ਕੁੱਲ_ਚੱਕਰਾਂ + stats->idle_ਚੱਕਰਾਂ;

	k_spin_unlock(&usage_ਤਾਲਾ, ਕੁੰਜੀ);
}
#ਜੇ_ਅੰਤ /* CONFIG_SCHED_THREAD_USAGE_ALL */

ਖਾਲੀ z_sched_thread_usage(ਬਣਤਰ k_thread *thread,
			  ਬਣਤਰ k_thread_runtime_stats *stats)
{
	ਬਣਤਰ _cpu *cpu;
	k_spinlock_ਕੁੰਜੀ_t  ਕੁੰਜੀ;

	ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&usage_ਤਾਲਾ);
	cpu = _current_cpu;


	ਜੇ (thread == cpu->current) {
		uint32_t now = usage_now();
		uint32_t ਚੱਕਰਾਂ = now - cpu->usage0;

		/*
		 * Getting stats for the current thread. Update both the
		 * current thread stats and its CPU stats as the CPU's
		 * [usage0] field will also get updated. This keeps all
		 * that information up-to-date.
		 */

		ਜੇ (thread->ਅਧਾਰ.usage.track_usage) {
			sched_thread_update_usage(thread, ਚੱਕਰਾਂ);
		}

		sched_cpu_update_usage(cpu, ਚੱਕਰਾਂ);

		cpu->usage0 = now;
	}

	stats->execution_ਚੱਕਰਾਂ = thread->ਅਧਾਰ.usage.ਕੁੱਲ;
	stats->ਕੁੱਲ_ਚੱਕਰਾਂ     = thread->ਅਧਾਰ.usage.ਕੁੱਲ;

	/* Copy-out the thread's usage stats */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SCHED_THREAD_USAGE_ANALYSIS
	stats->current_ਚੱਕਰਾਂ = thread->ਅਧਾਰ.usage.current;
	stats->peak_ਚੱਕਰਾਂ    = thread->ਅਧਾਰ.usage.longest;

	ਜੇ (thread->ਅਧਾਰ.usage.ਅੰਕ_windows == 0) {
		stats->ਔਸਤਨ_ਚੱਕਰਾਂ = 0;
	} ਨਹੀਂਤਾਂ {
		stats->ਔਸਤਨ_ਚੱਕਰਾਂ = stats->ਕੁੱਲ_ਚੱਕਰਾਂ /
					thread->ਅਧਾਰ.usage.ਅੰਕ_windows;
	}
#ਜੇ_ਅੰਤ /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SCHED_THREAD_USAGE_ALL
	stats->idle_ਚੱਕਰਾਂ = 0;
#ਜੇ_ਅੰਤ /* CONFIG_SCHED_THREAD_USAGE_ALL */

	k_spin_unlock(&usage_ਤਾਲਾ, ਕੁੰਜੀ);
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SCHED_THREAD_USAGE_ANALYSIS
ਪੂਰਨ k_thread_runtime_stats_enable(k_tid_t  thread)
{
	k_spinlock_ਕੁੰਜੀ_t  ਕੁੰਜੀ;

	CHECKIF(thread == ਨਿਰਾ) {
		ਵਾਪਸ -EINVAL;
	}

	ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&usage_ਤਾਲਾ);
	ਬਣਤਰ _cpu *cpu = _current_cpu;

	ਜੇ (!thread->ਅਧਾਰ.usage.track_usage) {
		thread->ਅਧਾਰ.usage.track_usage = true;
		thread->ਅਧਾਰ.usage.ਅੰਕ_windows++;
		thread->ਅਧਾਰ.usage.current = 0;

		ਜੇ (thread == cpu->current) {
			uint32_t now = usage_now();

			sched_cpu_update_usage(cpu, now - cpu->usage0);

			cpu->usage0 = now;
		}
	}

	k_spin_unlock(&usage_ਤਾਲਾ, ਕੁੰਜੀ);

	ਵਾਪਸ 0;
}

ਪੂਰਨ k_thread_runtime_stats_disable(k_tid_t  thread)
{
	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ;

	CHECKIF(thread == ਨਿਰਾ) {
		ਵਾਪਸ -EINVAL;
	}

	ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&usage_ਤਾਲਾ);
	ਬਣਤਰ _cpu *cpu = _current_cpu;

	ਜੇ (thread->ਅਧਾਰ.usage.track_usage) {
		thread->ਅਧਾਰ.usage.track_usage = false;

		ਜੇ (thread == cpu->current) {
			uint32_t now = usage_now();
			uint32_t ਚੱਕਰਾਂ = now - cpu->usage0;

			sched_thread_update_usage(thread, ਚੱਕਰਾਂ);
			sched_cpu_update_usage(cpu, ਚੱਕਰਾਂ);

			cpu->usage0 = now;
		}
	}

	k_spin_unlock(&usage_ਤਾਲਾ, ਕੁੰਜੀ);

	ਵਾਪਸ 0;
}

bool k_thread_runtime_stats_is_enabled(k_tid_t thread)
{
	ਵਾਪਸ thread->ਅਧਾਰ.usage.track_usage;
}
#ਜੇ_ਅੰਤ /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SCHED_THREAD_USAGE_ALL
ਖਾਲੀ k_sys_runtime_stats_enable(ਖਾਲੀ)
{
	k_spinlock_ਕੁੰਜੀ_t  ਕੁੰਜੀ;

	ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&usage_ਤਾਲਾ);

	ਜੇ (_current_cpu->usage->track_usage) {

		/*
		 * Usage tracking is already enabled on the current CPU
		 * and thus on all other CPUs (if applicable). There is
		 * nothing left to do.
		 */

		k_spin_unlock(&usage_ਤਾਲਾ, ਕੁੰਜੀ);
		ਵਾਪਸ;
	}

	/* Enable gathering of runtime stats on each CPU */

	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਅੰਕ_cpus = arch_ਅੰਕ_cpus();

	ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ i = 0; i < ਅੰਕ_cpus; i++) {
		_kernel.cpus[i].usage->track_usage = true;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SCHED_THREAD_USAGE_ANALYSIS
		_kernel.cpus[i].usage->ਅੰਕ_windows++;
		_kernel.cpus[i].usage->current = 0;
#ਜੇ_ਅੰਤ /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */
	}

	k_spin_unlock(&usage_ਤਾਲਾ, ਕੁੰਜੀ);
}

ਖਾਲੀ k_sys_runtime_stats_disable(ਖਾਲੀ)
{
	ਬਣਤਰ _cpu *cpu;
	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ;

	ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&usage_ਤਾਲਾ);

	ਜੇ (!_current_cpu->usage->track_usage) {

		/*
		 * Usage tracking is already disabled on the current CPU
		 * and thus on all other CPUs (if applicable). There is
		 * nothing left to do.
		 */

		k_spin_unlock(&usage_ਤਾਲਾ, ਕੁੰਜੀ);
		ਵਾਪਸ;
	}

	uint32_t now = usage_now();

	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਅੰਕ_cpus = arch_ਅੰਕ_cpus();

	ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ i = 0; i < ਅੰਕ_cpus; i++) {
		cpu = &_kernel.cpus[i];
		ਜੇ (cpu->usage0 != 0) {
			sched_cpu_update_usage(cpu, now - cpu->usage0);
		}
		cpu->usage->track_usage = false;
	}

	k_spin_unlock(&usage_ਤਾਲਾ, ਕੁੰਜੀ);
}
#ਜੇ_ਅੰਤ /* CONFIG_SCHED_THREAD_USAGE_ALL */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_OBJ_CORE_STATS_THREAD
ਪੂਰਨ z_thread_stats_raw(ਬਣਤਰ k_obj_core *obj_core, ਖਾਲੀ *stats)
{
	k_spinlock_ਕੁੰਜੀ_t  ਕੁੰਜੀ;

	ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&usage_ਤਾਲਾ);
	ਸਮ੍ਰਿਤੀ_ਨਕਲ(stats, obj_core->stats, ਆਕਾਰ(ਬਣਤਰ k_ਚੱਕਰ_stats));
	k_spin_unlock(&usage_ਤਾਲਾ, ਕੁੰਜੀ);

	ਵਾਪਸ 0;
}

ਪੂਰਨ z_thread_stats_query(ਬਣਤਰ k_obj_core *obj_core, ਖਾਲੀ *stats)
{
	ਬਣਤਰ k_thread *thread;

	thread = CONTAINER_ਦਾ(obj_core, ਬਣਤਰ k_thread, obj_core);

	z_sched_thread_usage(thread, stats);

	ਵਾਪਸ 0;
}

ਪੂਰਨ z_thread_stats_reset(ਬਣਤਰ k_obj_core *obj_core)
{
	k_spinlock_ਕੁੰਜੀ_t  ਕੁੰਜੀ;
	ਬਣਤਰ k_ਚੱਕਰ_stats  *stats;
	ਬਣਤਰ k_thread *thread;

	thread = CONTAINER_ਦਾ(obj_core, ਬਣਤਰ k_thread, obj_core);
	ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&usage_ਤਾਲਾ);
	stats = obj_core->stats;

	stats->ਕੁੱਲ = 0ULL;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SCHED_THREAD_USAGE_ANALYSIS
	stats->current = 0ULL;
	stats->longest = 0ULL;
	stats->ਅੰਕ_windows = (thread->ਅਧਾਰ.usage.track_usage) ?  1U : 0U;
#ਜੇ_ਅੰਤ /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */

	ਜੇ (thread != _current_cpu->current) {

		/*
		 * If the thread is not running, there is nothing else to do.
		 * If the thread is running on another core, then it is not
		 * safe to do anything else but unlock and return (and pretend
		 * that its stats were reset at the start of its execution
		 * window.
		 */

		k_spin_unlock(&usage_ਤਾਲਾ, ਕੁੰਜੀ);

		ਵਾਪਸ 0;
	}

	/* Update the current CPU stats. */

	uint32_t now = usage_now();

	sched_cpu_update_usage(_current_cpu, now - _current_cpu->usage0);

	_current_cpu->usage0 = now;

	k_spin_unlock(&usage_ਤਾਲਾ, ਕੁੰਜੀ);

	ਵਾਪਸ 0;
}

ਪੂਰਨ z_thread_stats_disable(ਬਣਤਰ k_obj_core *obj_core)
{
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SCHED_THREAD_USAGE_ANALYSIS
	ਬਣਤਰ k_thread *thread;

	thread = CONTAINER_ਦਾ(obj_core, ਬਣਤਰ k_thread, obj_core);

	ਵਾਪਸ k_thread_runtime_stats_disable(thread);
#ਨਹੀਂਤਾਂ
	ਵਾਪਸ -ENOTSUP;
#ਜੇ_ਅੰਤ /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */
}

ਪੂਰਨ z_thread_stats_enable(ਬਣਤਰ k_obj_core *obj_core)
{
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SCHED_THREAD_USAGE_ANALYSIS
	ਬਣਤਰ k_thread *thread;

	thread = CONTAINER_ਦਾ(obj_core, ਬਣਤਰ k_thread, obj_core);

	ਵਾਪਸ k_thread_runtime_stats_enable(thread);
#ਨਹੀਂਤਾਂ
	ਵਾਪਸ -ENOTSUP;
#ਜੇ_ਅੰਤ /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */
}
#ਜੇ_ਅੰਤ /* CONFIG_OBJ_CORE_STATS_THREAD */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_OBJ_CORE_STATS_SYSTEM
ਪੂਰਨ z_cpu_stats_raw(ਬਣਤਰ k_obj_core *obj_core, ਖਾਲੀ *stats)
{
	k_spinlock_ਕੁੰਜੀ_t  ਕੁੰਜੀ;

	ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&usage_ਤਾਲਾ);
	ਸਮ੍ਰਿਤੀ_ਨਕਲ(stats, obj_core->stats, ਆਕਾਰ(ਬਣਤਰ k_ਚੱਕਰ_stats));
	k_spin_unlock(&usage_ਤਾਲਾ, ਕੁੰਜੀ);

	ਵਾਪਸ 0;
}

ਪੂਰਨ z_cpu_stats_query(ਬਣਤਰ k_obj_core *obj_core, ਖਾਲੀ *stats)
{
	ਬਣਤਰ _cpu  *cpu;

	cpu = CONTAINER_ਦਾ(obj_core, ਬਣਤਰ _cpu, obj_core);

	z_sched_cpu_usage(cpu->id, stats);

	ਵਾਪਸ 0;
}
#ਜੇ_ਅੰਤ /* CONFIG_OBJ_CORE_STATS_SYSTEM */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_OBJ_CORE_STATS_SYSTEM
ਪੂਰਨ z_kernel_stats_raw(ਬਣਤਰ k_obj_core *obj_core, ਖਾਲੀ *stats)
{
	k_spinlock_ਕੁੰਜੀ_t  ਕੁੰਜੀ;

	ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&usage_ਤਾਲਾ);
	ਸਮ੍ਰਿਤੀ_ਨਕਲ(stats, obj_core->stats,
	       CONFIG_MP_MAX_ਅੰਕ_CPUS * ਆਕਾਰ(ਬਣਤਰ k_ਚੱਕਰ_stats));
	k_spin_unlock(&usage_ਤਾਲਾ, ਕੁੰਜੀ);

	ਵਾਪਸ 0;
}

ਪੂਰਨ z_kernel_stats_query(ਬਣਤਰ k_obj_core *obj_core, ਖਾਲੀ *stats)
{
	ARG_ਅਣਵਰਤਿਆ(obj_core);

	ਵਾਪਸ k_thread_runtime_stats_all_get(stats);
}
#ਜੇ_ਅੰਤ /* CONFIG_OBJ_CORE_STATS_SYSTEM */
