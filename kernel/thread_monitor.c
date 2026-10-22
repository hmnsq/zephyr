/*
 * Copyright (c) 2010-2014 Wind River Systems, Inc.
 * Copyright (c) 2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#அந்தர்பாவ <zephyr/kernel.h>
#அந்தர்பாவ <kthread.h>

கட்டமைப்பு k_spinlock z_thread_monitor_lock;
/*
 * Remove a thread from the kernel's list of active threads.
 */
வெறுமை z_thread_monitor_exit(கட்டமைப்பு k_thread *thread)
{
	k_spinlock_சாவி_t சாவி = k_spin_lock(&z_thread_monitor_lock);

	எனில் (thread == _kernel.threads) {
		_kernel.threads = _kernel.threads->next_thread;
	} இல்லையெனில் {
		கட்டமைப்பு k_thread *prev_thread;

		prev_thread = _kernel.threads;
		போது ((prev_thread != வெற்று) &&
			(thread != prev_thread->next_thread)) {
			prev_thread = prev_thread->next_thread;
		}
		எனில் (prev_thread != வெற்று) {
			prev_thread->next_thread = thread->next_thread;
		}
	}

	k_spin_unlock(&z_thread_monitor_lock, சாவி);
}

/*
 * Helper function to iterate over threads with optional filtering and locking behavior.
 */
நிலையான வெறுமை thread_foreach_helper(k_thread_user_cb_t user_cb, வெறுமை *user_data,
		bool unlocked, bool filter_by_cpu, unsigned முழு cpu)
{
	கட்டமைப்பு k_thread *thread;
	k_spinlock_சாவி_t சாவி;

	__ASSERT(user_cb != வெற்று, "user_cb can not be NULL");

	எனில் (filter_by_cpu) {
		__ASSERT(cpu < CONFIG_MP_MAX_இலக்கம்_CPUS, "cpu filter out of bounds");
	}

	சாவி = k_spin_lock(&z_thread_monitor_lock);

	க்கு (thread = _kernel.threads; thread; thread = thread->next_thread) {
		/* cpu is only defined when SMP=y*/
#ifdef CONFIG_SMP
		bool on_cpu = (thread->base.cpu == cpu);
#இல்லையெனில்
		bool on_cpu = false;
#endif
		எனில் (filter_by_cpu && !on_cpu) {
			தொடர்;
		}

		எனில் (unlocked) {
			k_spin_unlock(&z_thread_monitor_lock, சாவி);
			user_cb(thread, user_data);
			சாவி = k_spin_lock(&z_thread_monitor_lock);
		} இல்லையெனில் {
			user_cb(thread, user_data);
		}
	}

	k_spin_unlock(&z_thread_monitor_lock, சாவி);
}

/*
 * Public API functions using the helper.
 */
வெறுமை k_thread_foreach(k_thread_user_cb_t user_cb, வெறுமை *user_data)
{
	SYS_PORT_TRACING_FUNC_ENTER(k_thread, foreach);
	thread_foreach_helper(user_cb, user_data, false, false, 0);
	SYS_PORT_TRACING_FUNC_EXIT(k_thread, foreach);
}

வெறுமை k_thread_foreach_unlocked(k_thread_user_cb_t user_cb, வெறுமை *user_data)
{
	SYS_PORT_TRACING_FUNC_ENTER(k_thread, foreach_unlocked);
	thread_foreach_helper(user_cb, user_data, true, false, 0);
	SYS_PORT_TRACING_FUNC_EXIT(k_thread, foreach_unlocked);
}

#ifdef CONFIG_SMP
வெறுமை k_thread_foreach_filter_by_cpu(unsigned முழு cpu, k_thread_user_cb_t user_cb,
		வெறுமை *user_data)
{
	SYS_PORT_TRACING_FUNC_ENTER(k_thread, foreach);
	thread_foreach_helper(user_cb, user_data, false, true, cpu);
	SYS_PORT_TRACING_FUNC_EXIT(k_thread, foreach);
}

வெறுமை k_thread_foreach_unlocked_filter_by_cpu(unsigned முழு cpu, k_thread_user_cb_t user_cb,
		வெறுமை *user_data)
{
	SYS_PORT_TRACING_FUNC_ENTER(k_thread, foreach_unlocked);
	thread_foreach_helper(user_cb, user_data, true, true, cpu);
	SYS_PORT_TRACING_FUNC_EXIT(k_thread, foreach_unlocked);
}
#endif /* CONFIG_SMP */
