/*
 * Copyright (c) 2010-2014 Wind River Systems, Inc.
 * Copyright (c) 2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#شامل <zephyr/kernel.h>
#شامل <kthread.h>

ساختار k_spinlock z_thread_monitor_lock;
/*
 * Remove a thread from the kernel's list of active threads.
 */
تهی z_thread_monitor_exit(ساختار k_thread *thread)
{
	k_spinlock_کلید_t کلید = k_spin_lock(&z_thread_monitor_lock);

	اگر (thread == _kernel.threads) {
		_kernel.threads = _kernel.threads->next_thread;
	} وگرنه {
		ساختار k_thread *prev_thread;

		prev_thread = _kernel.threads;
		وقتی ((prev_thread != پوچ) &&
			(thread != prev_thread->next_thread)) {
			prev_thread = prev_thread->next_thread;
		}
		اگر (prev_thread != پوچ) {
			prev_thread->next_thread = thread->next_thread;
		}
	}

	k_spin_unlock(&z_thread_monitor_lock, کلید);
}

/*
 * Helper function to iterate over threads with optional filtering and locking behavior.
 */
ایستا تهی thread_foreach_helper(k_thread_user_cb_t user_cb, تهی *user_data,
		bool unlocked, bool filter_by_cpu, unsigned صحیح cpu)
{
	ساختار k_thread *thread;
	k_spinlock_کلید_t کلید;

	__ASSERT(user_cb != پوچ, "user_cb can not be NULL");

	اگر (filter_by_cpu) {
		__ASSERT(cpu < CONFIG_MP_MAX_رقم_CPUS, "cpu filter out of bounds");
	}

	کلید = k_spin_lock(&z_thread_monitor_lock);

	برای (thread = _kernel.threads; thread; thread = thread->next_thread) {
		/* cpu is only defined when SMP=y*/
#ifdef CONFIG_SMP
		bool on_cpu = (thread->base.cpu == cpu);
#وگرنه
		bool on_cpu = false;
#endif
		اگر (filter_by_cpu && !on_cpu) {
			ادامه;
		}

		اگر (unlocked) {
			k_spin_unlock(&z_thread_monitor_lock, کلید);
			user_cb(thread, user_data);
			کلید = k_spin_lock(&z_thread_monitor_lock);
		} وگرنه {
			user_cb(thread, user_data);
		}
	}

	k_spin_unlock(&z_thread_monitor_lock, کلید);
}

/*
 * Public API functions using the helper.
 */
تهی k_thread_foreach(k_thread_user_cb_t user_cb, تهی *user_data)
{
	SYS_PORT_TRACING_FUNC_ENTER(k_thread, foreach);
	thread_foreach_helper(user_cb, user_data, false, false, 0);
	SYS_PORT_TRACING_FUNC_EXIT(k_thread, foreach);
}

تهی k_thread_foreach_unlocked(k_thread_user_cb_t user_cb, تهی *user_data)
{
	SYS_PORT_TRACING_FUNC_ENTER(k_thread, foreach_unlocked);
	thread_foreach_helper(user_cb, user_data, true, false, 0);
	SYS_PORT_TRACING_FUNC_EXIT(k_thread, foreach_unlocked);
}

#ifdef CONFIG_SMP
تهی k_thread_foreach_filter_by_cpu(unsigned صحیح cpu, k_thread_user_cb_t user_cb,
		تهی *user_data)
{
	SYS_PORT_TRACING_FUNC_ENTER(k_thread, foreach);
	thread_foreach_helper(user_cb, user_data, false, true, cpu);
	SYS_PORT_TRACING_FUNC_EXIT(k_thread, foreach);
}

تهی k_thread_foreach_unlocked_filter_by_cpu(unsigned صحیح cpu, k_thread_user_cb_t user_cb,
		تهی *user_data)
{
	SYS_PORT_TRACING_FUNC_ENTER(k_thread, foreach_unlocked);
	thread_foreach_helper(user_cb, user_data, true, true, cpu);
	SYS_PORT_TRACING_FUNC_EXIT(k_thread, foreach_unlocked);
}
#endif /* CONFIG_SMP */
