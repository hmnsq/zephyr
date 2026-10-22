/*
 * Copyright (c) 2010-2014 Wind River Systems, Inc.
 * Copyright (c) 2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ضمن <zephyr/kernel.h>
#ضمن <kthread.h>

بنية k_spinlock z_thread_monitor_قلف;
/*
 * Remove a thread from the kernel's list of active threads.
 */
فراغ z_thread_monitor_exit(بنية k_thread *thread)
{
	k_spinlock_کلی_t کلی = k_spin_قلف(&z_thread_monitor_قلف);

	إذا (thread == _kernel.تارونه) {
		_kernel.تارونه = _kernel.تارونه->بل_thread;
	} وإلا {
		بنية k_thread *prev_thread;

		prev_thread = _kernel.تارونه;
		طالما ((prev_thread != عدم) &&
			(thread != prev_thread->بل_thread)) {
			prev_thread = prev_thread->بل_thread;
		}
		إذا (prev_thread != عدم) {
			prev_thread->بل_thread = thread->بل_thread;
		}
	}

	k_spin_unlock(&z_thread_monitor_قلف, کلی);
}

/*
 * Helper function to iterate over threads with optional filtering and locking behavior.
 */
ساكن فراغ thread_foreach_helper(k_thread_user_cb_t user_cb, فراغ *user_معلومات,
		bool unlocked, bool filter_by_cpu, بینښه صحيح cpu)
{
	بنية k_thread *thread;
	k_spinlock_کلی_t کلی;

	__ASSERT(user_cb != عدم, "user_cb can not be NULL");

	إذا (filter_by_cpu) {
		__ASSERT(cpu < CONFIG_MP_MAX_عدد_CPUS, "cpu filter out of bounds");
	}

	کلی = k_spin_قلف(&z_thread_monitor_قلف);

	لكل (thread = _kernel.تارونه; thread; thread = thread->بل_thread) {
		/* cpu is only defined when SMP=y*/
#که_تعریف_شوی CONFIG_SMP
		bool on_cpu = (thread->بنسټ.cpu == cpu);
#وإلا
		bool on_cpu = false;
#د_که_پای
		إذا (filter_by_cpu && !on_cpu) {
			استمر;
		}

		إذا (unlocked) {
			k_spin_unlock(&z_thread_monitor_قلف, کلی);
			user_cb(thread, user_معلومات);
			کلی = k_spin_قلف(&z_thread_monitor_قلف);
		} وإلا {
			user_cb(thread, user_معلومات);
		}
	}

	k_spin_unlock(&z_thread_monitor_قلف, کلی);
}

/*
 * Public API functions using the helper.
 */
فراغ k_thread_foreach(k_thread_user_cb_t user_cb, فراغ *user_معلومات)
{
	SYS_پورټ_TRACING_FUNC_ENTER(k_thread, foreach);
	thread_foreach_helper(user_cb, user_معلومات, false, false, 0);
	SYS_پورټ_TRACING_FUNC_EXIT(k_thread, foreach);
}

فراغ k_thread_foreach_unlocked(k_thread_user_cb_t user_cb, فراغ *user_معلومات)
{
	SYS_پورټ_TRACING_FUNC_ENTER(k_thread, foreach_unlocked);
	thread_foreach_helper(user_cb, user_معلومات, true, false, 0);
	SYS_پورټ_TRACING_FUNC_EXIT(k_thread, foreach_unlocked);
}

#که_تعریف_شوی CONFIG_SMP
فراغ k_thread_foreach_filter_by_cpu(بینښه صحيح cpu, k_thread_user_cb_t user_cb,
		فراغ *user_معلومات)
{
	SYS_پورټ_TRACING_FUNC_ENTER(k_thread, foreach);
	thread_foreach_helper(user_cb, user_معلومات, false, true, cpu);
	SYS_پورټ_TRACING_FUNC_EXIT(k_thread, foreach);
}

فراغ k_thread_foreach_unlocked_filter_by_cpu(بینښه صحيح cpu, k_thread_user_cb_t user_cb,
		فراغ *user_معلومات)
{
	SYS_پورټ_TRACING_FUNC_ENTER(k_thread, foreach_unlocked);
	thread_foreach_helper(user_cb, user_معلومات, true, true, cpu);
	SYS_پورټ_TRACING_FUNC_EXIT(k_thread, foreach_unlocked);
}
#د_که_پای /* CONFIG_SMP */
