/*
 * Copyright (c) 2010-2014 Wind River Systems, Inc.
 * Copyright (c) 2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ضمن <zephyr/kernel.h>
#ضمن <kthread.h>

بنية k_spinlock z_thread_monitor_جندرا;
/*
 * Remove a thread from the kernel's list of active threads.
 */
فراغ z_thread_monitor_exit(بنية k_thread *thread)
{
	k_spinlock_کلید_t کلید = k_spin_جندرا(&z_thread_monitor_جندرا);

	إذا (thread == _kernel.دھاگے) {
		_kernel.دھاگے = _kernel.دھاگے->اگلا_thread;
	} وإلا {
		بنية k_thread *prev_thread;

		prev_thread = _kernel.دھاگے;
		طالما ((prev_thread != عدم) &&
			(thread != prev_thread->اگلا_thread)) {
			prev_thread = prev_thread->اگلا_thread;
		}
		إذا (prev_thread != عدم) {
			prev_thread->اگلا_thread = thread->اگلا_thread;
		}
	}

	k_spin_unlock(&z_thread_monitor_جندرا, کلید);
}

/*
 * Helper function to iterate over threads with optional filtering and locking behavior.
 */
ساكن فراغ thread_foreach_helper(k_thread_user_cb_t user_cb, فراغ *user_انکڑے,
		bool unlocked, bool filter_by_cpu, بنانشانی صحيح cpu)
{
	بنية k_thread *thread;
	k_spinlock_کلید_t کلید;

	__ASSERT(user_cb != عدم, "user_cb can not be NULL");

	إذا (filter_by_cpu) {
		__ASSERT(cpu < CONFIG_MP_MAX_عدد_CPUS, "cpu filter out of bounds");
	}

	کلید = k_spin_جندرا(&z_thread_monitor_جندرا);

	لكل (thread = _kernel.دھاگے; thread; thread = thread->اگلا_thread) {
		/* cpu is only defined when SMP=y*/
#جے_تعریف_شدہ CONFIG_SMP
		bool on_cpu = (thread->ادھار.cpu == cpu);
#وإلا
		bool on_cpu = false;
#جے_انت
		إذا (filter_by_cpu && !on_cpu) {
			استمر;
		}

		إذا (unlocked) {
			k_spin_unlock(&z_thread_monitor_جندرا, کلید);
			user_cb(thread, user_انکڑے);
			کلید = k_spin_جندرا(&z_thread_monitor_جندرا);
		} وإلا {
			user_cb(thread, user_انکڑے);
		}
	}

	k_spin_unlock(&z_thread_monitor_جندرا, کلید);
}

/*
 * Public API functions using the helper.
 */
فراغ k_thread_foreach(k_thread_user_cb_t user_cb, فراغ *user_انکڑے)
{
	SYS_پورٹ_TRACING_FUNC_ENTER(k_thread, foreach);
	thread_foreach_helper(user_cb, user_انکڑے, false, false, 0);
	SYS_پورٹ_TRACING_FUNC_EXIT(k_thread, foreach);
}

فراغ k_thread_foreach_unlocked(k_thread_user_cb_t user_cb, فراغ *user_انکڑے)
{
	SYS_پورٹ_TRACING_FUNC_ENTER(k_thread, foreach_unlocked);
	thread_foreach_helper(user_cb, user_انکڑے, true, false, 0);
	SYS_پورٹ_TRACING_FUNC_EXIT(k_thread, foreach_unlocked);
}

#جے_تعریف_شدہ CONFIG_SMP
فراغ k_thread_foreach_filter_by_cpu(بنانشانی صحيح cpu, k_thread_user_cb_t user_cb,
		فراغ *user_انکڑے)
{
	SYS_پورٹ_TRACING_FUNC_ENTER(k_thread, foreach);
	thread_foreach_helper(user_cb, user_انکڑے, false, true, cpu);
	SYS_پورٹ_TRACING_FUNC_EXIT(k_thread, foreach);
}

فراغ k_thread_foreach_unlocked_filter_by_cpu(بنانشانی صحيح cpu, k_thread_user_cb_t user_cb,
		فراغ *user_انکڑے)
{
	SYS_پورٹ_TRACING_FUNC_ENTER(k_thread, foreach_unlocked);
	thread_foreach_helper(user_cb, user_انکڑے, true, true, cpu);
	SYS_پورٹ_TRACING_FUNC_EXIT(k_thread, foreach_unlocked);
}
#جے_انت /* CONFIG_SMP */
