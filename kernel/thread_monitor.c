/*
 * Copyright (c) 2010-2014 Wind River Systems, Inc.
 * Copyright (c) 2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ضمن <zephyr/kernel.h>
#ضمن <kthread.h>

بنية k_spinlock z_thread_monitor_lock;
/*
 * Remove a thread from the kernel's list of active threads.
 */
فراغ z_thread_monitor_exit(بنية k_thread *thread)
{
	k_spinlock_key_t key = k_spin_lock(&z_thread_monitor_lock);

	𐤀𐤌 (thread == _kernel.threads) {
		_kernel.threads = _kernel.threads->next_thread;
	} وإلا {
		بنية k_thread *prev_thread;

		prev_thread = _kernel.threads;
		طالما ((prev_thread != عدم) &&
			(thread != prev_thread->next_thread)) {
			prev_thread = prev_thread->next_thread;
		}
		𐤀𐤌 (prev_thread != عدم) {
			prev_thread->next_thread = thread->next_thread;
		}
	}

	k_spin_unlock(&z_thread_monitor_lock, key);
}

/*
 * Helper function to iterate over threads with optional filtering and locking behavior.
 */
ساكن فراغ thread_foreach_helper(k_thread_user_cb_t user_cb, فراغ *user_data,
		bool unlocked, bool filter_by_cpu, unsigned صحيح cpu)
{
	بنية k_thread *thread;
	k_spinlock_key_t key;

	__ASSERT(user_cb != عدم, "user_cb can not be NULL");

	𐤀𐤌 (filter_by_cpu) {
		__ASSERT(cpu < CONFIG_MP_MAX_NUM_CPUS, "cpu filter out of bounds");
	}

	key = k_spin_lock(&z_thread_monitor_lock);

	لكل (thread = _kernel.threads; thread; thread = thread->next_thread) {
		/* cpu is only defined when SMP=y*/
#ifdef CONFIG_SMP
		bool on_cpu = (thread->base.cpu == cpu);
#وإلا
		bool on_cpu = false;
#endif
		𐤀𐤌 (filter_by_cpu && !on_cpu) {
			استمر;
		}

		𐤀𐤌 (unlocked) {
			k_spin_unlock(&z_thread_monitor_lock, key);
			user_cb(thread, user_data);
			key = k_spin_lock(&z_thread_monitor_lock);
		} وإلا {
			user_cb(thread, user_data);
		}
	}

	k_spin_unlock(&z_thread_monitor_lock, key);
}

/*
 * Public API functions using the helper.
 */
فراغ k_thread_foreach(k_thread_user_cb_t user_cb, فراغ *user_data)
{
	SYS_PORT_TRACING_FUNC_ENTER(k_thread, foreach);
	thread_foreach_helper(user_cb, user_data, false, false, 0);
	SYS_PORT_TRACING_FUNC_EXIT(k_thread, foreach);
}

فراغ k_thread_foreach_unlocked(k_thread_user_cb_t user_cb, فراغ *user_data)
{
	SYS_PORT_TRACING_FUNC_ENTER(k_thread, foreach_unlocked);
	thread_foreach_helper(user_cb, user_data, true, false, 0);
	SYS_PORT_TRACING_FUNC_EXIT(k_thread, foreach_unlocked);
}

#ifdef CONFIG_SMP
فراغ k_thread_foreach_filter_by_cpu(unsigned صحيح cpu, k_thread_user_cb_t user_cb,
		فراغ *user_data)
{
	SYS_PORT_TRACING_FUNC_ENTER(k_thread, foreach);
	thread_foreach_helper(user_cb, user_data, false, true, cpu);
	SYS_PORT_TRACING_FUNC_EXIT(k_thread, foreach);
}

فراغ k_thread_foreach_unlocked_filter_by_cpu(unsigned صحيح cpu, k_thread_user_cb_t user_cb,
		فراغ *user_data)
{
	SYS_PORT_TRACING_FUNC_ENTER(k_thread, foreach_unlocked);
	thread_foreach_helper(user_cb, user_data, true, true, cpu);
	SYS_PORT_TRACING_FUNC_EXIT(k_thread, foreach_unlocked);
}
#endif /* CONFIG_SMP */
