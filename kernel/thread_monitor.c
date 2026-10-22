/*
 * Copyright (c) 2010-2014 Wind River Systems, Inc.
 * Copyright (c) 2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#അന്തര്ഭാവ <zephyr/kernel.h>
#അന്തര്ഭാവ <kthread.h>

ഘടന k_spinlock z_thread_monitor_lock;
/*
 * Remove a thread from the kernel's list of active threads.
 */
ശൂന്യം z_thread_monitor_exit(ഘടന k_thread *thread)
{
	k_spinlock_താക്കോൽ_t താക്കോൽ = k_spin_lock(&z_thread_monitor_lock);

	എങ്കിൽ (thread == _kernel.threads) {
		_kernel.threads = _kernel.threads->next_thread;
	} അല്ലെങ്കിൽ {
		ഘടന k_thread *prev_thread;

		prev_thread = _kernel.threads;
		ആകുമ്പോൾ ((prev_thread != ശൂന്യ) &&
			(thread != prev_thread->next_thread)) {
			prev_thread = prev_thread->next_thread;
		}
		എങ്കിൽ (prev_thread != ശൂന്യ) {
			prev_thread->next_thread = thread->next_thread;
		}
	}

	k_spin_unlock(&z_thread_monitor_lock, താക്കോൽ);
}

/*
 * Helper function to iterate over threads with optional filtering and locking behavior.
 */
സ്ഥിതം ശൂന്യം thread_foreach_helper(k_thread_user_cb_t user_cb, ശൂന്യം *user_data,
		bool unlocked, bool filter_by_cpu, unsigned പൂർണ്ണസംഖ്യ cpu)
{
	ഘടന k_thread *thread;
	k_spinlock_താക്കോൽ_t താക്കോൽ;

	__ASSERT(user_cb != ശൂന്യ, "user_cb can not be NULL");

	എങ്കിൽ (filter_by_cpu) {
		__ASSERT(cpu < CONFIG_MP_MAX_അക്കം_CPUS, "cpu filter out of bounds");
	}

	താക്കോൽ = k_spin_lock(&z_thread_monitor_lock);

	വേണ്ടി (thread = _kernel.threads; thread; thread = thread->next_thread) {
		/* cpu is only defined when SMP=y*/
#ifdef CONFIG_SMP
		bool on_cpu = (thread->base.cpu == cpu);
#അല്ലെങ്കിൽ
		bool on_cpu = false;
#endif
		എങ്കിൽ (filter_by_cpu && !on_cpu) {
			തുടരുക;
		}

		എങ്കിൽ (unlocked) {
			k_spin_unlock(&z_thread_monitor_lock, താക്കോൽ);
			user_cb(thread, user_data);
			താക്കോൽ = k_spin_lock(&z_thread_monitor_lock);
		} അല്ലെങ്കിൽ {
			user_cb(thread, user_data);
		}
	}

	k_spin_unlock(&z_thread_monitor_lock, താക്കോൽ);
}

/*
 * Public API functions using the helper.
 */
ശൂന്യം k_thread_foreach(k_thread_user_cb_t user_cb, ശൂന്യം *user_data)
{
	SYS_PORT_TRACING_FUNC_ENTER(k_thread, foreach);
	thread_foreach_helper(user_cb, user_data, false, false, 0);
	SYS_PORT_TRACING_FUNC_EXIT(k_thread, foreach);
}

ശൂന്യം k_thread_foreach_unlocked(k_thread_user_cb_t user_cb, ശൂന്യം *user_data)
{
	SYS_PORT_TRACING_FUNC_ENTER(k_thread, foreach_unlocked);
	thread_foreach_helper(user_cb, user_data, true, false, 0);
	SYS_PORT_TRACING_FUNC_EXIT(k_thread, foreach_unlocked);
}

#ifdef CONFIG_SMP
ശൂന്യം k_thread_foreach_filter_by_cpu(unsigned പൂർണ്ണസംഖ്യ cpu, k_thread_user_cb_t user_cb,
		ശൂന്യം *user_data)
{
	SYS_PORT_TRACING_FUNC_ENTER(k_thread, foreach);
	thread_foreach_helper(user_cb, user_data, false, true, cpu);
	SYS_PORT_TRACING_FUNC_EXIT(k_thread, foreach);
}

ശൂന്യം k_thread_foreach_unlocked_filter_by_cpu(unsigned പൂർണ്ണസംഖ്യ cpu, k_thread_user_cb_t user_cb,
		ശൂന്യം *user_data)
{
	SYS_PORT_TRACING_FUNC_ENTER(k_thread, foreach_unlocked);
	thread_foreach_helper(user_cb, user_data, true, true, cpu);
	SYS_PORT_TRACING_FUNC_EXIT(k_thread, foreach_unlocked);
}
#endif /* CONFIG_SMP */
