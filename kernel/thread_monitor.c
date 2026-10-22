/*
 * Copyright (c) 2010-2014 Wind River Systems, Inc.
 * Copyright (c) 2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#অন্তর্ভাব <zephyr/kernel.h>
#অন্তর্ভাব <kthread.h>

কাঠামো k_spinlock z_thread_monitor_lock;
/*
 * Remove a thread from the kernel's list of active threads.
 */
শূন্য z_thread_monitor_exit(কাঠামো k_thread *thread)
{
	k_spinlock_চাবি_t চাবি = k_spin_lock(&z_thread_monitor_lock);

	যদি (thread == _kernel.threads) {
		_kernel.threads = _kernel.threads->পরবর্তী_thread;
	} অন্যথা {
		কাঠামো k_thread *পূর্ব_thread;

		পূর্ব_thread = _kernel.threads;
		যখন ((পূর্ব_thread != ফাঁকা) &&
			(thread != পূর্ব_thread->পরবর্তী_thread)) {
			পূর্ব_thread = পূর্ব_thread->পরবর্তী_thread;
		}
		যদি (পূর্ব_thread != ফাঁকা) {
			পূর্ব_thread->পরবর্তী_thread = thread->পরবর্তী_thread;
		}
	}

	k_spin_unlock(&z_thread_monitor_lock, চাবি);
}

/*
 * Helper function to iterate over threads with optional filtering and locking behavior.
 */
স্থির শূন্য thread_foreach_helper(k_thread_user_cb_t user_cb, শূন্য *user_তথ্য,
		bool unlocked, bool filter_by_cpu, unsigned পূর্ণসংখ্যা cpu)
{
	কাঠামো k_thread *thread;
	k_spinlock_চাবি_t চাবি;

	__ASSERT(user_cb != ফাঁকা, "user_cb can not be NULL");

	যদি (filter_by_cpu) {
		__ASSERT(cpu < CONFIG_MP_MAX_অঙ্ক_CPUS, "cpu filter out of bounds");
	}

	চাবি = k_spin_lock(&z_thread_monitor_lock);

	জন্য (thread = _kernel.threads; thread; thread = thread->পরবর্তী_thread) {
		/* cpu is only defined when SMP=y*/
#ifdef CONFIG_SMP
		bool on_cpu = (thread->base.cpu == cpu);
#অন্যথা
		bool on_cpu = false;
#endif
		যদি (filter_by_cpu && !on_cpu) {
			চালু;
		}

		যদি (unlocked) {
			k_spin_unlock(&z_thread_monitor_lock, চাবি);
			user_cb(thread, user_তথ্য);
			চাবি = k_spin_lock(&z_thread_monitor_lock);
		} অন্যথা {
			user_cb(thread, user_তথ্য);
		}
	}

	k_spin_unlock(&z_thread_monitor_lock, চাবি);
}

/*
 * Public API functions using the helper.
 */
শূন্য k_thread_foreach(k_thread_user_cb_t user_cb, শূন্য *user_তথ্য)
{
	SYS_PORT_TRACING_FUNC_ENTER(k_thread, foreach);
	thread_foreach_helper(user_cb, user_তথ্য, false, false, 0);
	SYS_PORT_TRACING_FUNC_EXIT(k_thread, foreach);
}

শূন্য k_thread_foreach_unlocked(k_thread_user_cb_t user_cb, শূন্য *user_তথ্য)
{
	SYS_PORT_TRACING_FUNC_ENTER(k_thread, foreach_unlocked);
	thread_foreach_helper(user_cb, user_তথ্য, true, false, 0);
	SYS_PORT_TRACING_FUNC_EXIT(k_thread, foreach_unlocked);
}

#ifdef CONFIG_SMP
শূন্য k_thread_foreach_filter_by_cpu(unsigned পূর্ণসংখ্যা cpu, k_thread_user_cb_t user_cb,
		শূন্য *user_তথ্য)
{
	SYS_PORT_TRACING_FUNC_ENTER(k_thread, foreach);
	thread_foreach_helper(user_cb, user_তথ্য, false, true, cpu);
	SYS_PORT_TRACING_FUNC_EXIT(k_thread, foreach);
}

শূন্য k_thread_foreach_unlocked_filter_by_cpu(unsigned পূর্ণসংখ্যা cpu, k_thread_user_cb_t user_cb,
		শূন্য *user_তথ্য)
{
	SYS_PORT_TRACING_FUNC_ENTER(k_thread, foreach_unlocked);
	thread_foreach_helper(user_cb, user_তথ্য, true, true, cpu);
	SYS_PORT_TRACING_FUNC_EXIT(k_thread, foreach_unlocked);
}
#endif /* CONFIG_SMP */
