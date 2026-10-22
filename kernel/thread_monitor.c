/*
 * Copyright (c) 2010-2014 Wind River Systems, Inc.
 * Copyright (c) 2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ضمن <zephyr/kernel.h>
#ضمن <kthread.h>

بنية k_spinlock z_thread_monitor_قفل;
/*
 * Remove a thread from the kernel's list of active threads.
 */
فراغ z_thread_monitor_exit(بنية k_thread *thread)
{
	k_spinlock_کلید_t کلید = k_spin_قفل(&z_thread_monitor_قفل);

	إذا (thread == _kernel.نخها) {
		_kernel.نخها = _kernel.نخها->بعدی_thread;
	} وإلا {
		بنية k_thread *prev_thread;

		prev_thread = _kernel.نخها;
		طالما ((prev_thread != عدم) &&
			(thread != prev_thread->بعدی_thread)) {
			prev_thread = prev_thread->بعدی_thread;
		}
		إذا (prev_thread != عدم) {
			prev_thread->بعدی_thread = thread->بعدی_thread;
		}
	}

	k_spin_unlock(&z_thread_monitor_قفل, کلید);
}

/*
 * Helper function to iterate over threads with optional filtering and locking behavior.
 */
ساكن فراغ thread_foreach_helper(k_thread_user_cb_t user_cb, فراغ *user_داده,
		bool unlocked, bool filter_by_cpu, بیعلامت صحيح cpu)
{
	بنية k_thread *thread;
	k_spinlock_کلید_t کلید;

	__ASSERT(user_cb != عدم, "user_cb can not be NULL");

	إذا (filter_by_cpu) {
		__ASSERT(cpu < CONFIG_MP_MAX_رقم_CPUS, "cpu filter out of bounds");
	}

	کلید = k_spin_قفل(&z_thread_monitor_قفل);

	لكل (thread = _kernel.نخها; thread; thread = thread->بعدی_thread) {
		/* cpu is only defined when SMP=y*/
#اگر_تعریف_شده CONFIG_SMP
		bool on_cpu = (thread->پایه.cpu == cpu);
#وإلا
		bool on_cpu = false;
#پایان_اگر
		إذا (filter_by_cpu && !on_cpu) {
			استمر;
		}

		إذا (unlocked) {
			k_spin_unlock(&z_thread_monitor_قفل, کلید);
			user_cb(thread, user_داده);
			کلید = k_spin_قفل(&z_thread_monitor_قفل);
		} وإلا {
			user_cb(thread, user_داده);
		}
	}

	k_spin_unlock(&z_thread_monitor_قفل, کلید);
}

/*
 * Public API functions using the helper.
 */
فراغ k_thread_foreach(k_thread_user_cb_t user_cb, فراغ *user_داده)
{
	SYS_پورت_TRACING_FUNC_ENTER(k_thread, foreach);
	thread_foreach_helper(user_cb, user_داده, false, false, 0);
	SYS_پورت_TRACING_FUNC_EXIT(k_thread, foreach);
}

فراغ k_thread_foreach_unlocked(k_thread_user_cb_t user_cb, فراغ *user_داده)
{
	SYS_پورت_TRACING_FUNC_ENTER(k_thread, foreach_unlocked);
	thread_foreach_helper(user_cb, user_داده, true, false, 0);
	SYS_پورت_TRACING_FUNC_EXIT(k_thread, foreach_unlocked);
}

#اگر_تعریف_شده CONFIG_SMP
فراغ k_thread_foreach_filter_by_cpu(بیعلامت صحيح cpu, k_thread_user_cb_t user_cb,
		فراغ *user_داده)
{
	SYS_پورت_TRACING_FUNC_ENTER(k_thread, foreach);
	thread_foreach_helper(user_cb, user_داده, false, true, cpu);
	SYS_پورت_TRACING_FUNC_EXIT(k_thread, foreach);
}

فراغ k_thread_foreach_unlocked_filter_by_cpu(بیعلامت صحيح cpu, k_thread_user_cb_t user_cb,
		فراغ *user_داده)
{
	SYS_پورت_TRACING_FUNC_ENTER(k_thread, foreach_unlocked);
	thread_foreach_helper(user_cb, user_داده, true, true, cpu);
	SYS_پورت_TRACING_FUNC_EXIT(k_thread, foreach_unlocked);
}
#پایان_اگر /* CONFIG_SMP */
