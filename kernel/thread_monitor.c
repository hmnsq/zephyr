/*
 * Copyright (c) 2010-2014 Wind River Systems, Inc.
 * Copyright (c) 2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#अन्तर्भाव <zephyr/kernel.h>
#अन्तर्भाव <kthread.h>

संरचना k_spinlock z_thread_monitor_तालकम्;
/*
 * Remove a thread from the kernel's list of active threads.
 */
शून्य z_thread_monitor_exit(संरचना k_thread *thread)
{
	k_spinlock_कुञ्चिका_t कुञ्चिका = k_spin_तालकम्(&z_thread_monitor_तालकम्);

	यदि (thread == _kernel.तन्तवः) {
		_kernel.तन्तवः = _kernel.तन्तवः->अग्र_thread;
	} अन्यथा {
		संरचना k_thread *पूर्व_thread;

		पूर्व_thread = _kernel.तन्तवः;
		यावत् ((पूर्व_thread != रिक्त) &&
			(thread != पूर्व_thread->अग्र_thread)) {
			पूर्व_thread = पूर्व_thread->अग्र_thread;
		}
		यदि (पूर्व_thread != रिक्त) {
			पूर्व_thread->अग्र_thread = thread->अग्र_thread;
		}
	}

	k_spin_unlock(&z_thread_monitor_तालकम्, कुञ्चिका);
}

/*
 * Helper function to iterate over threads with optional filtering and locking behavior.
 */
स्थिर शून्य thread_foreach_helper(k_thread_user_cb_t user_cb, शून्य *user_दत्त,
		bool unlocked, bool filter_by_cpu, अचिह्नित पूर्णाङ्क cpu)
{
	संरचना k_thread *thread;
	k_spinlock_कुञ्चिका_t कुञ्चिका;

	__ASSERT(user_cb != रिक्त, "user_cb can not be NULL");

	यदि (filter_by_cpu) {
		__ASSERT(cpu < CONFIG_MP_MAX_अङ्कः_CPUS, "cpu filter out of bounds");
	}

	कुञ्चिका = k_spin_तालकम्(&z_thread_monitor_तालकम्);

	प्रति (thread = _kernel.तन्तवः; thread; thread = thread->अग्र_thread) {
		/* cpu is only defined when SMP=y*/
#यदि_परिभाषितम् CONFIG_SMP
		bool on_cpu = (thread->आधारः.cpu == cpu);
#अन्यथा
		bool on_cpu = false;
#यदि_अन्तः
		यदि (filter_by_cpu && !on_cpu) {
			अनुवर्त;
		}

		यदि (unlocked) {
			k_spin_unlock(&z_thread_monitor_तालकम्, कुञ्चिका);
			user_cb(thread, user_दत्त);
			कुञ्चिका = k_spin_तालकम्(&z_thread_monitor_तालकम्);
		} अन्यथा {
			user_cb(thread, user_दत्त);
		}
	}

	k_spin_unlock(&z_thread_monitor_तालकम्, कुञ्चिका);
}

/*
 * Public API functions using the helper.
 */
शून्य k_thread_foreach(k_thread_user_cb_t user_cb, शून्य *user_दत्त)
{
	SYS_पत्तनम्_TRACING_FUNC_ENTER(k_thread, foreach);
	thread_foreach_helper(user_cb, user_दत्त, false, false, 0);
	SYS_पत्तनम्_TRACING_FUNC_EXIT(k_thread, foreach);
}

शून्य k_thread_foreach_unlocked(k_thread_user_cb_t user_cb, शून्य *user_दत्त)
{
	SYS_पत्तनम्_TRACING_FUNC_ENTER(k_thread, foreach_unlocked);
	thread_foreach_helper(user_cb, user_दत्त, true, false, 0);
	SYS_पत्तनम्_TRACING_FUNC_EXIT(k_thread, foreach_unlocked);
}

#यदि_परिभाषितम् CONFIG_SMP
शून्य k_thread_foreach_filter_by_cpu(अचिह्नित पूर्णाङ्क cpu, k_thread_user_cb_t user_cb,
		शून्य *user_दत्त)
{
	SYS_पत्तनम्_TRACING_FUNC_ENTER(k_thread, foreach);
	thread_foreach_helper(user_cb, user_दत्त, false, true, cpu);
	SYS_पत्तनम्_TRACING_FUNC_EXIT(k_thread, foreach);
}

शून्य k_thread_foreach_unlocked_filter_by_cpu(अचिह्नित पूर्णाङ्क cpu, k_thread_user_cb_t user_cb,
		शून्य *user_दत्त)
{
	SYS_पत्तनम्_TRACING_FUNC_ENTER(k_thread, foreach_unlocked);
	thread_foreach_helper(user_cb, user_दत्त, true, true, cpu);
	SYS_पत्तनम्_TRACING_FUNC_EXIT(k_thread, foreach_unlocked);
}
#यदि_अन्तः /* CONFIG_SMP */
