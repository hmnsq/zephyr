/*
 * Copyright (c) 2010-2014 Wind River Systems, Inc.
 * Copyright (c) 2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#شامل <zephyr/kernel.h>
#شامل <kthread.h>

ساخت k_spinlock z_thread_monitor_تالا;
/*
 * Remove a thread from the kernel's list of active threads.
 */
خالی z_thread_monitor_exit(ساخت k_thread *thread)
{
	k_spinlock_کلید_t کلید = k_spin_تالا(&z_thread_monitor_تالا);

	اگر (thread == _kernel.دھاگے) {
		_kernel.دھاگے = _kernel.دھاگے->اگلا_thread;
	} ورنہ {
		ساخت k_thread *پچھلا_thread;

		پچھلا_thread = _kernel.دھاگے;
		جب ((پچھلا_thread != تہی) &&
			(thread != پچھلا_thread->اگلا_thread)) {
			پچھلا_thread = پچھلا_thread->اگلا_thread;
		}
		اگر (پچھلا_thread != تہی) {
			پچھلا_thread->اگلا_thread = thread->اگلا_thread;
		}
	}

	k_spin_unlock(&z_thread_monitor_تالا, کلید);
}

/*
 * Helper function to iterate over threads with optional filtering and locking behavior.
 */
ساکن خالی thread_foreach_helper(k_thread_user_cb_t user_cb, خالی *user_ڈیٹا,
		bool unlocked, bool filter_by_cpu, غیرعلامتی صحیح cpu)
{
	ساخت k_thread *thread;
	k_spinlock_کلید_t کلید;

	__ASSERT(user_cb != تہی, "user_cb can not be NULL");

	اگر (filter_by_cpu) {
		__ASSERT(cpu < CONFIG_MP_MAX_عدد_CPUS, "cpu filter out of bounds");
	}

	کلید = k_spin_تالا(&z_thread_monitor_تالا);

	برائے (thread = _kernel.دھاگے; thread; thread = thread->اگلا_thread) {
		/* cpu is only defined when SMP=y*/
#اگر_تعریف_شدہ CONFIG_SMP
		bool on_cpu = (thread->بنیاد.cpu == cpu);
#ورنہ
		bool on_cpu = false;
#اگر_ختم
		اگر (filter_by_cpu && !on_cpu) {
			جاری;
		}

		اگر (unlocked) {
			k_spin_unlock(&z_thread_monitor_تالا, کلید);
			user_cb(thread, user_ڈیٹا);
			کلید = k_spin_تالا(&z_thread_monitor_تالا);
		} ورنہ {
			user_cb(thread, user_ڈیٹا);
		}
	}

	k_spin_unlock(&z_thread_monitor_تالا, کلید);
}

/*
 * Public API functions using the helper.
 */
خالی k_thread_foreach(k_thread_user_cb_t user_cb, خالی *user_ڈیٹا)
{
	SYS_پورٹ_TRACING_FUNC_ENTER(k_thread, foreach);
	thread_foreach_helper(user_cb, user_ڈیٹا, false, false, 0);
	SYS_پورٹ_TRACING_FUNC_EXIT(k_thread, foreach);
}

خالی k_thread_foreach_unlocked(k_thread_user_cb_t user_cb, خالی *user_ڈیٹا)
{
	SYS_پورٹ_TRACING_FUNC_ENTER(k_thread, foreach_unlocked);
	thread_foreach_helper(user_cb, user_ڈیٹا, true, false, 0);
	SYS_پورٹ_TRACING_FUNC_EXIT(k_thread, foreach_unlocked);
}

#اگر_تعریف_شدہ CONFIG_SMP
خالی k_thread_foreach_filter_by_cpu(غیرعلامتی صحیح cpu, k_thread_user_cb_t user_cb,
		خالی *user_ڈیٹا)
{
	SYS_پورٹ_TRACING_FUNC_ENTER(k_thread, foreach);
	thread_foreach_helper(user_cb, user_ڈیٹا, false, true, cpu);
	SYS_پورٹ_TRACING_FUNC_EXIT(k_thread, foreach);
}

خالی k_thread_foreach_unlocked_filter_by_cpu(غیرعلامتی صحیح cpu, k_thread_user_cb_t user_cb,
		خالی *user_ڈیٹا)
{
	SYS_پورٹ_TRACING_FUNC_ENTER(k_thread, foreach_unlocked);
	thread_foreach_helper(user_cb, user_ڈیٹا, true, true, cpu);
	SYS_پورٹ_TRACING_FUNC_EXIT(k_thread, foreach_unlocked);
}
#اگر_ختم /* CONFIG_SMP */
