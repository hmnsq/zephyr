/*
 * Copyright (c) 2010-2014 Wind River Systems, Inc.
 * Copyright (c) 2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#અન્તર્ભાવ <zephyr/kernel.h>
#અન્તર્ભાવ <kthread.h>

સંરચના k_spinlock z_thread_monitor_lock;
/*
 * Remove a thread from the kernel's list of active threads.
 */
શૂન્ય z_thread_monitor_exit(સંરચના k_thread *thread)
{
	k_spinlock_ચાવી_t ચાવી = k_spin_lock(&z_thread_monitor_lock);

	જો (thread == _kernel.threads) {
		_kernel.threads = _kernel.threads->next_thread;
	} નહીંતર {
		સંરચના k_thread *prev_thread;

		prev_thread = _kernel.threads;
		જ્યારે ((prev_thread != ખાલી) &&
			(thread != prev_thread->next_thread)) {
			prev_thread = prev_thread->next_thread;
		}
		જો (prev_thread != ખાલી) {
			prev_thread->next_thread = thread->next_thread;
		}
	}

	k_spin_unlock(&z_thread_monitor_lock, ચાવી);
}

/*
 * Helper function to iterate over threads with optional filtering and locking behavior.
 */
સ્થિર શૂન્ય thread_foreach_helper(k_thread_user_cb_t user_cb, શૂન્ય *user_data,
		bool unlocked, bool filter_by_cpu, unsigned પૂર્ણાંક cpu)
{
	સંરચના k_thread *thread;
	k_spinlock_ચાવી_t ચાવી;

	__ASSERT(user_cb != ખાલી, "user_cb can not be NULL");

	જો (filter_by_cpu) {
		__ASSERT(cpu < CONFIG_MP_MAX_અંક_CPUS, "cpu filter out of bounds");
	}

	ચાવી = k_spin_lock(&z_thread_monitor_lock);

	માટે (thread = _kernel.threads; thread; thread = thread->next_thread) {
		/* cpu is only defined when SMP=y*/
#ifdef CONFIG_SMP
		bool on_cpu = (thread->base.cpu == cpu);
#નહીંતર
		bool on_cpu = false;
#endif
		જો (filter_by_cpu && !on_cpu) {
			ચાલુ;
		}

		જો (unlocked) {
			k_spin_unlock(&z_thread_monitor_lock, ચાવી);
			user_cb(thread, user_data);
			ચાવી = k_spin_lock(&z_thread_monitor_lock);
		} નહીંતર {
			user_cb(thread, user_data);
		}
	}

	k_spin_unlock(&z_thread_monitor_lock, ચાવી);
}

/*
 * Public API functions using the helper.
 */
શૂન્ય k_thread_foreach(k_thread_user_cb_t user_cb, શૂન્ય *user_data)
{
	SYS_PORT_TRACING_FUNC_ENTER(k_thread, foreach);
	thread_foreach_helper(user_cb, user_data, false, false, 0);
	SYS_PORT_TRACING_FUNC_EXIT(k_thread, foreach);
}

શૂન્ય k_thread_foreach_unlocked(k_thread_user_cb_t user_cb, શૂન્ય *user_data)
{
	SYS_PORT_TRACING_FUNC_ENTER(k_thread, foreach_unlocked);
	thread_foreach_helper(user_cb, user_data, true, false, 0);
	SYS_PORT_TRACING_FUNC_EXIT(k_thread, foreach_unlocked);
}

#ifdef CONFIG_SMP
શૂન્ય k_thread_foreach_filter_by_cpu(unsigned પૂર્ણાંક cpu, k_thread_user_cb_t user_cb,
		શૂન્ય *user_data)
{
	SYS_PORT_TRACING_FUNC_ENTER(k_thread, foreach);
	thread_foreach_helper(user_cb, user_data, false, true, cpu);
	SYS_PORT_TRACING_FUNC_EXIT(k_thread, foreach);
}

શૂન્ય k_thread_foreach_unlocked_filter_by_cpu(unsigned પૂર્ણાંક cpu, k_thread_user_cb_t user_cb,
		શૂન્ય *user_data)
{
	SYS_PORT_TRACING_FUNC_ENTER(k_thread, foreach_unlocked);
	thread_foreach_helper(user_cb, user_data, true, true, cpu);
	SYS_PORT_TRACING_FUNC_EXIT(k_thread, foreach_unlocked);
}
#endif /* CONFIG_SMP */
