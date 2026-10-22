/*
 * Copyright (c) 2010-2014 Wind River Systems, Inc.
 * Copyright (c) 2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#అన్తర్భావ <zephyr/kernel.h>
#అన్తర్భావ <kthread.h>

నిర్మాణం k_spinlock z_thread_monitor_తాళం;
/*
 * Remove a thread from the kernel's list of active threads.
 */
శూన్య z_thread_monitor_exit(నిర్మాణం k_thread *thread)
{
	k_spinlock_కీలకం_t కీలకం = k_spin_తాళం(&z_thread_monitor_తాళం);

	ఒకవేళ (thread == _kernel.దారాలు) {
		_kernel.దారాలు = _kernel.దారాలు->తదుపరి_thread;
	} లేకపోతే {
		నిర్మాణం k_thread *prev_thread;

		prev_thread = _kernel.దారాలు;
		అయితే ((prev_thread != ఖాళీ) &&
			(thread != prev_thread->తదుపరి_thread)) {
			prev_thread = prev_thread->తదుపరి_thread;
		}
		ఒకవేళ (prev_thread != ఖాళీ) {
			prev_thread->తదుపరి_thread = thread->తదుపరి_thread;
		}
	}

	k_spin_unlock(&z_thread_monitor_తాళం, కీలకం);
}

/*
 * Helper function to iterate over threads with optional filtering and locking behavior.
 */
స్థిర శూన్య thread_foreach_helper(k_thread_user_cb_t user_cb, శూన్య *user_దత్తాంశం,
		bool unlocked, bool filter_by_cpu, గుర్తులేని పూర్ణాంకం cpu)
{
	నిర్మాణం k_thread *thread;
	k_spinlock_కీలకం_t కీలకం;

	__ASSERT(user_cb != ఖాళీ, "user_cb can not be NULL");

	ఒకవేళ (filter_by_cpu) {
		__ASSERT(cpu < CONFIG_MP_MAX_అంకె_CPUS, "cpu filter out of bounds");
	}

	కీలకం = k_spin_తాళం(&z_thread_monitor_తాళం);

	కొరకు (thread = _kernel.దారాలు; thread; thread = thread->తదుపరి_thread) {
		/* cpu is only defined when SMP=y*/
#నిర్వచితమైతే CONFIG_SMP
		bool on_cpu = (thread->పునాది.cpu == cpu);
#లేకపోతే
		bool on_cpu = false;
#అయితే_ముగింపు
		ఒకవేళ (filter_by_cpu && !on_cpu) {
			కొనసాగు;
		}

		ఒకవేళ (unlocked) {
			k_spin_unlock(&z_thread_monitor_తాళం, కీలకం);
			user_cb(thread, user_దత్తాంశం);
			కీలకం = k_spin_తాళం(&z_thread_monitor_తాళం);
		} లేకపోతే {
			user_cb(thread, user_దత్తాంశం);
		}
	}

	k_spin_unlock(&z_thread_monitor_తాళం, కీలకం);
}

/*
 * Public API functions using the helper.
 */
శూన్య k_thread_foreach(k_thread_user_cb_t user_cb, శూన్య *user_దత్తాంశం)
{
	SYS_పోర్టు_TRACING_FUNC_ENTER(k_thread, foreach);
	thread_foreach_helper(user_cb, user_దత్తాంశం, false, false, 0);
	SYS_పోర్టు_TRACING_FUNC_EXIT(k_thread, foreach);
}

శూన్య k_thread_foreach_unlocked(k_thread_user_cb_t user_cb, శూన్య *user_దత్తాంశం)
{
	SYS_పోర్టు_TRACING_FUNC_ENTER(k_thread, foreach_unlocked);
	thread_foreach_helper(user_cb, user_దత్తాంశం, true, false, 0);
	SYS_పోర్టు_TRACING_FUNC_EXIT(k_thread, foreach_unlocked);
}

#నిర్వచితమైతే CONFIG_SMP
శూన్య k_thread_foreach_filter_by_cpu(గుర్తులేని పూర్ణాంకం cpu, k_thread_user_cb_t user_cb,
		శూన్య *user_దత్తాంశం)
{
	SYS_పోర్టు_TRACING_FUNC_ENTER(k_thread, foreach);
	thread_foreach_helper(user_cb, user_దత్తాంశం, false, true, cpu);
	SYS_పోర్టు_TRACING_FUNC_EXIT(k_thread, foreach);
}

శూన్య k_thread_foreach_unlocked_filter_by_cpu(గుర్తులేని పూర్ణాంకం cpu, k_thread_user_cb_t user_cb,
		శూన్య *user_దత్తాంశం)
{
	SYS_పోర్టు_TRACING_FUNC_ENTER(k_thread, foreach_unlocked);
	thread_foreach_helper(user_cb, user_దత్తాంశం, true, true, cpu);
	SYS_పోర్టు_TRACING_FUNC_EXIT(k_thread, foreach_unlocked);
}
#అయితే_ముగింపు /* CONFIG_SMP */
