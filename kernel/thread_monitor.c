/*
 * Copyright (c) 2010-2014 Wind River Systems, Inc.
 * Copyright (c) 2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#অন্তর্ভাব <zephyr/kernel.h>
#অন্তর্ভাব <kthread.h>

সংরচনা k_spinlock z_thread_monitor_lock;
/*
 * Remove a thread from the kernel's list of active threads.
 */
শূন্য z_thread_monitor_exit(সংরচনা k_thread *thread)
{
	k_spinlock_চাবি_t চাবি = k_spin_lock(&z_thread_monitor_lock);

	যদি (thread == _kernel.threads) {
		_kernel.threads = _kernel.threads->next_thread;
	} অন্যথা {
		সংরচনা k_thread *prev_thread;

		prev_thread = _kernel.threads;
		যাবত্ ((prev_thread != রিক্ত) &&
			(thread != prev_thread->next_thread)) {
			prev_thread = prev_thread->next_thread;
		}
		যদি (prev_thread != রিক্ত) {
			prev_thread->next_thread = thread->next_thread;
		}
	}

	k_spin_unlock(&z_thread_monitor_lock, চাবি);
}

/*
 * Helper function to iterate over threads with optional filtering and locking behavior.
 */
স্থির শূন্য thread_foreach_helper(k_thread_user_cb_t user_cb, শূন্য *user_data,
		bool unlocked, bool filter_by_cpu, unsigned পূর্ণাঙ্ক cpu)
{
	সংরচনা k_thread *thread;
	k_spinlock_চাবি_t চাবি;

	__ASSERT(user_cb != রিক্ত, "user_cb can not be NULL");

	যদি (filter_by_cpu) {
		__ASSERT(cpu < CONFIG_MP_MAX_অঙ্ক_CPUS, "cpu filter out of bounds");
	}

	চাবি = k_spin_lock(&z_thread_monitor_lock);

	প্রতি (thread = _kernel.threads; thread; thread = thread->next_thread) {
		/* cpu is only defined when SMP=y*/
#ifdef CONFIG_SMP
		bool on_cpu = (thread->base.cpu == cpu);
#অন্যথা
		bool on_cpu = false;
#endif
		যদি (filter_by_cpu && !on_cpu) {
			অনুবর্ত;
		}

		যদি (unlocked) {
			k_spin_unlock(&z_thread_monitor_lock, চাবি);
			user_cb(thread, user_data);
			চাবি = k_spin_lock(&z_thread_monitor_lock);
		} অন্যথা {
			user_cb(thread, user_data);
		}
	}

	k_spin_unlock(&z_thread_monitor_lock, চাবি);
}

/*
 * Public API functions using the helper.
 */
শূন্য k_thread_foreach(k_thread_user_cb_t user_cb, শূন্য *user_data)
{
	SYS_PORT_TRACING_FUNC_ENTER(k_thread, foreach);
	thread_foreach_helper(user_cb, user_data, false, false, 0);
	SYS_PORT_TRACING_FUNC_EXIT(k_thread, foreach);
}

শূন্য k_thread_foreach_unlocked(k_thread_user_cb_t user_cb, শূন্য *user_data)
{
	SYS_PORT_TRACING_FUNC_ENTER(k_thread, foreach_unlocked);
	thread_foreach_helper(user_cb, user_data, true, false, 0);
	SYS_PORT_TRACING_FUNC_EXIT(k_thread, foreach_unlocked);
}

#ifdef CONFIG_SMP
শূন্য k_thread_foreach_filter_by_cpu(unsigned পূর্ণাঙ্ক cpu, k_thread_user_cb_t user_cb,
		শূন্য *user_data)
{
	SYS_PORT_TRACING_FUNC_ENTER(k_thread, foreach);
	thread_foreach_helper(user_cb, user_data, false, true, cpu);
	SYS_PORT_TRACING_FUNC_EXIT(k_thread, foreach);
}

শূন্য k_thread_foreach_unlocked_filter_by_cpu(unsigned পূর্ণাঙ্ক cpu, k_thread_user_cb_t user_cb,
		শূন্য *user_data)
{
	SYS_PORT_TRACING_FUNC_ENTER(k_thread, foreach_unlocked);
	thread_foreach_helper(user_cb, user_data, true, true, cpu);
	SYS_PORT_TRACING_FUNC_EXIT(k_thread, foreach_unlocked);
}
#endif /* CONFIG_SMP */
