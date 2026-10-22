/*
 * Copyright (c) 2010-2014 Wind River Systems, Inc.
 * Copyright (c) 2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#अन्तर्भाव <zephyr/kernel.h>
#अन्तर्भाव <kthread.h>

संरचना k_spinlock z_thread_monitor_lock;
/*
 * Remove a thread from the kernel's list of active threads.
 */
शून्य z_thread_monitor_exit(संरचना k_thread *thread)
{
	k_spinlock_साँचो_t साँचो = k_spin_lock(&z_thread_monitor_lock);

	यदि (thread == _kernel.threads) {
		_kernel.threads = _kernel.threads->next_thread;
	} अन्यथा {
		संरचना k_thread *prev_thread;

		prev_thread = _kernel.threads;
		जबसम्म ((prev_thread != रिक्त) &&
			(thread != prev_thread->next_thread)) {
			prev_thread = prev_thread->next_thread;
		}
		यदि (prev_thread != रिक्त) {
			prev_thread->next_thread = thread->next_thread;
		}
	}

	k_spin_unlock(&z_thread_monitor_lock, साँचो);
}

/*
 * Helper function to iterate over threads with optional filtering and locking behavior.
 */
स्थिर शून्य thread_foreach_helper(k_thread_user_cb_t user_cb, शून्य *user_data,
		bool unlocked, bool filter_by_cpu, अचिन्हित पूर्णाङ्क cpu)
{
	संरचना k_thread *thread;
	k_spinlock_साँचो_t साँचो;

	__ASSERT(user_cb != रिक्त, "user_cb can not be NULL");

	यदि (filter_by_cpu) {
		__ASSERT(cpu < CONFIG_MP_MAX_अङ्क_CPUS, "cpu filter out of bounds");
	}

	साँचो = k_spin_lock(&z_thread_monitor_lock);

	लागि (thread = _kernel.threads; thread; thread = thread->next_thread) {
		/* cpu is only defined when SMP=y*/
#ifdef CONFIG_SMP
		bool on_cpu = (thread->base.cpu == cpu);
#अन्यथा
		bool on_cpu = false;
#endif
		यदि (filter_by_cpu && !on_cpu) {
			जारी;
		}

		यदि (unlocked) {
			k_spin_unlock(&z_thread_monitor_lock, साँचो);
			user_cb(thread, user_data);
			साँचो = k_spin_lock(&z_thread_monitor_lock);
		} अन्यथा {
			user_cb(thread, user_data);
		}
	}

	k_spin_unlock(&z_thread_monitor_lock, साँचो);
}

/*
 * Public API functions using the helper.
 */
शून्य k_thread_foreach(k_thread_user_cb_t user_cb, शून्य *user_data)
{
	SYS_PORT_TRACING_FUNC_ENTER(k_thread, foreach);
	thread_foreach_helper(user_cb, user_data, false, false, 0);
	SYS_PORT_TRACING_FUNC_EXIT(k_thread, foreach);
}

शून्य k_thread_foreach_unlocked(k_thread_user_cb_t user_cb, शून्य *user_data)
{
	SYS_PORT_TRACING_FUNC_ENTER(k_thread, foreach_unlocked);
	thread_foreach_helper(user_cb, user_data, true, false, 0);
	SYS_PORT_TRACING_FUNC_EXIT(k_thread, foreach_unlocked);
}

#ifdef CONFIG_SMP
शून्य k_thread_foreach_filter_by_cpu(अचिन्हित पूर्णाङ्क cpu, k_thread_user_cb_t user_cb,
		शून्य *user_data)
{
	SYS_PORT_TRACING_FUNC_ENTER(k_thread, foreach);
	thread_foreach_helper(user_cb, user_data, false, true, cpu);
	SYS_PORT_TRACING_FUNC_EXIT(k_thread, foreach);
}

शून्य k_thread_foreach_unlocked_filter_by_cpu(अचिन्हित पूर्णाङ्क cpu, k_thread_user_cb_t user_cb,
		शून्य *user_data)
{
	SYS_PORT_TRACING_FUNC_ENTER(k_thread, foreach_unlocked);
	thread_foreach_helper(user_cb, user_data, true, true, cpu);
	SYS_PORT_TRACING_FUNC_EXIT(k_thread, foreach_unlocked);
}
#endif /* CONFIG_SMP */
