/*
 * Copyright (c) 2010-2014 Wind River Systems, Inc.
 * Copyright (c) 2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/kernel.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <kthread.h>

ਬਣਤਰ k_spinlock z_thread_monitor_ਤਾਲਾ;
/*
 * Remove a thread from the kernel's list of active threads.
 */
ਖਾਲੀ z_thread_monitor_exit(ਬਣਤਰ k_thread *thread)
{
	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&z_thread_monitor_ਤਾਲਾ);

	ਜੇ (thread == _kernel.ਧਾਗੇ) {
		_kernel.ਧਾਗੇ = _kernel.ਧਾਗੇ->ਅਗਲਾ_thread;
	} ਨਹੀਂਤਾਂ {
		ਬਣਤਰ k_thread *prev_thread;

		prev_thread = _kernel.ਧਾਗੇ;
		ਜਦੋਂ ((prev_thread != ਨਿਰਾ) &&
			(thread != prev_thread->ਅਗਲਾ_thread)) {
			prev_thread = prev_thread->ਅਗਲਾ_thread;
		}
		ਜੇ (prev_thread != ਨਿਰਾ) {
			prev_thread->ਅਗਲਾ_thread = thread->ਅਗਲਾ_thread;
		}
	}

	k_spin_unlock(&z_thread_monitor_ਤਾਲਾ, ਕੁੰਜੀ);
}

/*
 * Helper function to iterate over threads with optional filtering and locking behavior.
 */
ਸਥਿਰ ਖਾਲੀ thread_foreach_helper(k_thread_user_cb_t user_cb, ਖਾਲੀ *user_ਅੰਕੜੇ,
		bool unlocked, bool filter_by_cpu, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ cpu)
{
	ਬਣਤਰ k_thread *thread;
	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ;

	__ASSERT(user_cb != ਨਿਰਾ, "user_cb can not be NULL");

	ਜੇ (filter_by_cpu) {
		__ASSERT(cpu < CONFIG_MP_MAX_ਅੰਕ_CPUS, "cpu filter out of bounds");
	}

	ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&z_thread_monitor_ਤਾਲਾ);

	ਲਈ (thread = _kernel.ਧਾਗੇ; thread; thread = thread->ਅਗਲਾ_thread) {
		/* cpu is only defined when SMP=y*/
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SMP
		bool on_cpu = (thread->ਅਧਾਰ.cpu == cpu);
#ਨਹੀਂਤਾਂ
		bool on_cpu = false;
#ਜੇ_ਅੰਤ
		ਜੇ (filter_by_cpu && !on_cpu) {
			ਜਾਰੀ;
		}

		ਜੇ (unlocked) {
			k_spin_unlock(&z_thread_monitor_ਤਾਲਾ, ਕੁੰਜੀ);
			user_cb(thread, user_ਅੰਕੜੇ);
			ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&z_thread_monitor_ਤਾਲਾ);
		} ਨਹੀਂਤਾਂ {
			user_cb(thread, user_ਅੰਕੜੇ);
		}
	}

	k_spin_unlock(&z_thread_monitor_ਤਾਲਾ, ਕੁੰਜੀ);
}

/*
 * Public API functions using the helper.
 */
ਖਾਲੀ k_thread_foreach(k_thread_user_cb_t user_cb, ਖਾਲੀ *user_ਅੰਕੜੇ)
{
	SYS_ਪੋਰਟ_TRACING_FUNC_ENTER(k_thread, foreach);
	thread_foreach_helper(user_cb, user_ਅੰਕੜੇ, false, false, 0);
	SYS_ਪੋਰਟ_TRACING_FUNC_EXIT(k_thread, foreach);
}

ਖਾਲੀ k_thread_foreach_unlocked(k_thread_user_cb_t user_cb, ਖਾਲੀ *user_ਅੰਕੜੇ)
{
	SYS_ਪੋਰਟ_TRACING_FUNC_ENTER(k_thread, foreach_unlocked);
	thread_foreach_helper(user_cb, user_ਅੰਕੜੇ, true, false, 0);
	SYS_ਪੋਰਟ_TRACING_FUNC_EXIT(k_thread, foreach_unlocked);
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SMP
ਖਾਲੀ k_thread_foreach_filter_by_cpu(ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ cpu, k_thread_user_cb_t user_cb,
		ਖਾਲੀ *user_ਅੰਕੜੇ)
{
	SYS_ਪੋਰਟ_TRACING_FUNC_ENTER(k_thread, foreach);
	thread_foreach_helper(user_cb, user_ਅੰਕੜੇ, false, true, cpu);
	SYS_ਪੋਰਟ_TRACING_FUNC_EXIT(k_thread, foreach);
}

ਖਾਲੀ k_thread_foreach_unlocked_filter_by_cpu(ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ cpu, k_thread_user_cb_t user_cb,
		ਖਾਲੀ *user_ਅੰਕੜੇ)
{
	SYS_ਪੋਰਟ_TRACING_FUNC_ENTER(k_thread, foreach_unlocked);
	thread_foreach_helper(user_cb, user_ਅੰਕੜੇ, true, true, cpu);
	SYS_ਪੋਰਟ_TRACING_FUNC_EXIT(k_thread, foreach_unlocked);
}
#ਜੇ_ਅੰਤ /* CONFIG_SMP */
