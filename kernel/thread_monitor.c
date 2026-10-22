/*
 * Copyright (c) 2010-2014 Wind River Systems, Inc.
 * Copyright (c) 2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ಅನ್ತರ್ಭಾವ <zephyr/kernel.h>
#ಅನ್ತರ್ಭಾವ <kthread.h>

ಸಂರಚನಾ k_spinlock z_thread_monitor_lock;
/*
 * Remove a thread from the kernel's list of active threads.
 */
ಶೂನ್ಯ z_thread_monitor_exit(ಸಂರಚನಾ k_thread *thread)
{
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_lock(&z_thread_monitor_lock);

	ಯದಿ (thread == _kernel.threads) {
		_kernel.threads = _kernel.threads->next_thread;
	} ಅನ್ಯಥಾ {
		ಸಂರಚನಾ k_thread *prev_thread;

		prev_thread = _kernel.threads;
		ಯಾವತ್ ((prev_thread != ರಿಕ್ತ) &&
			(thread != prev_thread->next_thread)) {
			prev_thread = prev_thread->next_thread;
		}
		ಯದಿ (prev_thread != ರಿಕ್ತ) {
			prev_thread->next_thread = thread->next_thread;
		}
	}

	k_spin_unlock(&z_thread_monitor_lock, ಕೀಲಿ);
}

/*
 * Helper function to iterate over threads with optional filtering and locking behavior.
 */
ಸ್ಥಿರ ಶೂನ್ಯ thread_foreach_helper(k_thread_user_cb_t user_cb, ಶೂನ್ಯ *user_data,
		bool unlocked, bool filter_by_cpu, unsigned ಪೂರ್ಣಾಙ್ಕ cpu)
{
	ಸಂರಚನಾ k_thread *thread;
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ;

	__ASSERT(user_cb != ರಿಕ್ತ, "user_cb can not be NULL");

	ಯದಿ (filter_by_cpu) {
		__ASSERT(cpu < CONFIG_MP_MAX_ಅಂಕೆ_CPUS, "cpu filter out of bounds");
	}

	ಕೀಲಿ = k_spin_lock(&z_thread_monitor_lock);

	ಪ್ರತಿ (thread = _kernel.threads; thread; thread = thread->next_thread) {
		/* cpu is only defined when SMP=y*/
#ifdef CONFIG_SMP
		bool on_cpu = (thread->base.cpu == cpu);
#ಅನ್ಯಥಾ
		bool on_cpu = false;
#endif
		ಯದಿ (filter_by_cpu && !on_cpu) {
			ಅನುವರ್ತ;
		}

		ಯದಿ (unlocked) {
			k_spin_unlock(&z_thread_monitor_lock, ಕೀಲಿ);
			user_cb(thread, user_data);
			ಕೀಲಿ = k_spin_lock(&z_thread_monitor_lock);
		} ಅನ್ಯಥಾ {
			user_cb(thread, user_data);
		}
	}

	k_spin_unlock(&z_thread_monitor_lock, ಕೀಲಿ);
}

/*
 * Public API functions using the helper.
 */
ಶೂನ್ಯ k_thread_foreach(k_thread_user_cb_t user_cb, ಶೂನ್ಯ *user_data)
{
	SYS_PORT_TRACING_FUNC_ENTER(k_thread, foreach);
	thread_foreach_helper(user_cb, user_data, false, false, 0);
	SYS_PORT_TRACING_FUNC_EXIT(k_thread, foreach);
}

ಶೂನ್ಯ k_thread_foreach_unlocked(k_thread_user_cb_t user_cb, ಶೂನ್ಯ *user_data)
{
	SYS_PORT_TRACING_FUNC_ENTER(k_thread, foreach_unlocked);
	thread_foreach_helper(user_cb, user_data, true, false, 0);
	SYS_PORT_TRACING_FUNC_EXIT(k_thread, foreach_unlocked);
}

#ifdef CONFIG_SMP
ಶೂನ್ಯ k_thread_foreach_filter_by_cpu(unsigned ಪೂರ್ಣಾಙ್ಕ cpu, k_thread_user_cb_t user_cb,
		ಶೂನ್ಯ *user_data)
{
	SYS_PORT_TRACING_FUNC_ENTER(k_thread, foreach);
	thread_foreach_helper(user_cb, user_data, false, true, cpu);
	SYS_PORT_TRACING_FUNC_EXIT(k_thread, foreach);
}

ಶೂನ್ಯ k_thread_foreach_unlocked_filter_by_cpu(unsigned ಪೂರ್ಣಾಙ್ಕ cpu, k_thread_user_cb_t user_cb,
		ಶೂನ್ಯ *user_data)
{
	SYS_PORT_TRACING_FUNC_ENTER(k_thread, foreach_unlocked);
	thread_foreach_helper(user_cb, user_data, true, true, cpu);
	SYS_PORT_TRACING_FUNC_EXIT(k_thread, foreach_unlocked);
}
#endif /* CONFIG_SMP */
