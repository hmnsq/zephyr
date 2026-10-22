/*
 * Copyright (c) 2010-2014 Wind River Systems, Inc.
 * Copyright (c) 2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ضمن <zephyr/kernel.h>
#ضمن <kthread.h>

بنية k_spinlock z_thread_monitor_تالو;
/*
 * Remove a thread from the kernel's list of active threads.
 */
فراغ z_thread_monitor_exit(بنية k_thread *thread)
{
	k_spinlock_ڪنجي_t ڪنجي = k_spin_تالو(&z_thread_monitor_تالو);

	إذا (thread == _kernel.ڌاڳا) {
		_kernel.ڌاڳا = _kernel.ڌاڳا->ايندڙ_thread;
	} وإلا {
		بنية k_thread *prev_thread;

		prev_thread = _kernel.ڌاڳا;
		طالما ((prev_thread != عدم) &&
			(thread != prev_thread->ايندڙ_thread)) {
			prev_thread = prev_thread->ايندڙ_thread;
		}
		إذا (prev_thread != عدم) {
			prev_thread->ايندڙ_thread = thread->ايندڙ_thread;
		}
	}

	k_spin_unlock(&z_thread_monitor_تالو, ڪنجي);
}

/*
 * Helper function to iterate over threads with optional filtering and locking behavior.
 */
ساكن فراغ thread_foreach_helper(k_thread_user_cb_t user_cb, فراغ *user_معلومات,
		bool unlocked, bool filter_by_cpu, بنانشان صحيح cpu)
{
	بنية k_thread *thread;
	k_spinlock_ڪنجي_t ڪنجي;

	__ASSERT(user_cb != عدم, "user_cb can not be NULL");

	إذا (filter_by_cpu) {
		__ASSERT(cpu < CONFIG_MP_MAX_انگ_CPUS, "cpu filter out of bounds");
	}

	ڪنجي = k_spin_تالو(&z_thread_monitor_تالو);

	لكل (thread = _kernel.ڌاڳا; thread; thread = thread->ايندڙ_thread) {
		/* cpu is only defined when SMP=y*/
#جيڪڏهن_وصف_ٿيل CONFIG_SMP
		bool on_cpu = (thread->بنياد.cpu == cpu);
#وإلا
		bool on_cpu = false;
#جيڪڏهن_پورو
		إذا (filter_by_cpu && !on_cpu) {
			استمر;
		}

		إذا (unlocked) {
			k_spin_unlock(&z_thread_monitor_تالو, ڪنجي);
			user_cb(thread, user_معلومات);
			ڪنجي = k_spin_تالو(&z_thread_monitor_تالو);
		} وإلا {
			user_cb(thread, user_معلومات);
		}
	}

	k_spin_unlock(&z_thread_monitor_تالو, ڪنجي);
}

/*
 * Public API functions using the helper.
 */
فراغ k_thread_foreach(k_thread_user_cb_t user_cb, فراغ *user_معلومات)
{
	SYS_پورٽ_TRACING_FUNC_ENTER(k_thread, foreach);
	thread_foreach_helper(user_cb, user_معلومات, false, false, 0);
	SYS_پورٽ_TRACING_FUNC_EXIT(k_thread, foreach);
}

فراغ k_thread_foreach_unlocked(k_thread_user_cb_t user_cb, فراغ *user_معلومات)
{
	SYS_پورٽ_TRACING_FUNC_ENTER(k_thread, foreach_unlocked);
	thread_foreach_helper(user_cb, user_معلومات, true, false, 0);
	SYS_پورٽ_TRACING_FUNC_EXIT(k_thread, foreach_unlocked);
}

#جيڪڏهن_وصف_ٿيل CONFIG_SMP
فراغ k_thread_foreach_filter_by_cpu(بنانشان صحيح cpu, k_thread_user_cb_t user_cb,
		فراغ *user_معلومات)
{
	SYS_پورٽ_TRACING_FUNC_ENTER(k_thread, foreach);
	thread_foreach_helper(user_cb, user_معلومات, false, true, cpu);
	SYS_پورٽ_TRACING_FUNC_EXIT(k_thread, foreach);
}

فراغ k_thread_foreach_unlocked_filter_by_cpu(بنانشان صحيح cpu, k_thread_user_cb_t user_cb,
		فراغ *user_معلومات)
{
	SYS_پورٽ_TRACING_FUNC_ENTER(k_thread, foreach_unlocked);
	thread_foreach_helper(user_cb, user_معلومات, true, true, cpu);
	SYS_پورٽ_TRACING_FUNC_EXIT(k_thread, foreach_unlocked);
}
#جيڪڏهن_پورو /* CONFIG_SMP */
