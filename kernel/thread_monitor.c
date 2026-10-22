/*
 * Copyright (c) 2010-2014 Wind River Systems, Inc.
 * Copyright (c) 2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#समावेश <zephyr/kernel.h>
#समावेश <kthread.h>

काष्ठा k_spinlock z_thread_monitor_lock;
/*
 * Remove a thread from the kernel's list of active threads.
 */
व्योम z_thread_monitor_exit(काष्ठा k_thread *thread)
{
	k_spinlock_कुंजी_t कुंजी = k_spin_lock(&z_thread_monitor_lock);

	अगर (thread == _kernel.threads) {
		_kernel.threads = _kernel.threads->अग्र_thread;
	} अन्यथा {
		काष्ठा k_thread *पूर्व_thread;

		पूर्व_thread = _kernel.threads;
		जबतक ((पूर्व_thread != शून्य) &&
			(thread != पूर्व_thread->अग्र_thread)) {
			पूर्व_thread = पूर्व_thread->अग्र_thread;
		}
		अगर (पूर्व_thread != शून्य) {
			पूर्व_thread->अग्र_thread = thread->अग्र_thread;
		}
	}

	k_spin_unlock(&z_thread_monitor_lock, कुंजी);
}

/*
 * Helper function to iterate over threads with optional filtering and locking behavior.
 */
अटल व्योम thread_foreach_helper(k_thread_user_cb_t user_cb, व्योम *user_तथ्य,
		bool unlocked, bool filter_by_cpu, अचिन्हित पूर्णांक cpu)
{
	काष्ठा k_thread *thread;
	k_spinlock_कुंजी_t कुंजी;

	__ASSERT(user_cb != शून्य, "user_cb can not be NULL");

	अगर (filter_by_cpu) {
		__ASSERT(cpu < CONFIG_MP_MAX_अंक_CPUS, "cpu filter out of bounds");
	}

	कुंजी = k_spin_lock(&z_thread_monitor_lock);

	क्रम (thread = _kernel.threads; thread; thread = thread->अग्र_thread) {
		/* cpu is only defined when SMP=y*/
#अगर_घोषित CONFIG_SMP
		bool on_cpu = (thread->base.cpu == cpu);
#अन्यथा
		bool on_cpu = false;
#पूर्ण_अगर
		अगर (filter_by_cpu && !on_cpu) {
			जारी;
		}

		अगर (unlocked) {
			k_spin_unlock(&z_thread_monitor_lock, कुंजी);
			user_cb(thread, user_तथ्य);
			कुंजी = k_spin_lock(&z_thread_monitor_lock);
		} अन्यथा {
			user_cb(thread, user_तथ्य);
		}
	}

	k_spin_unlock(&z_thread_monitor_lock, कुंजी);
}

/*
 * Public API functions using the helper.
 */
व्योम k_thread_foreach(k_thread_user_cb_t user_cb, व्योम *user_तथ्य)
{
	SYS_PORT_TRACING_FUNC_ENTER(k_thread, foreach);
	thread_foreach_helper(user_cb, user_तथ्य, false, false, 0);
	SYS_PORT_TRACING_FUNC_EXIT(k_thread, foreach);
}

व्योम k_thread_foreach_unlocked(k_thread_user_cb_t user_cb, व्योम *user_तथ्य)
{
	SYS_PORT_TRACING_FUNC_ENTER(k_thread, foreach_unlocked);
	thread_foreach_helper(user_cb, user_तथ्य, true, false, 0);
	SYS_PORT_TRACING_FUNC_EXIT(k_thread, foreach_unlocked);
}

#अगर_घोषित CONFIG_SMP
व्योम k_thread_foreach_filter_by_cpu(अचिन्हित पूर्णांक cpu, k_thread_user_cb_t user_cb,
		व्योम *user_तथ्य)
{
	SYS_PORT_TRACING_FUNC_ENTER(k_thread, foreach);
	thread_foreach_helper(user_cb, user_तथ्य, false, true, cpu);
	SYS_PORT_TRACING_FUNC_EXIT(k_thread, foreach);
}

व्योम k_thread_foreach_unlocked_filter_by_cpu(अचिन्हित पूर्णांक cpu, k_thread_user_cb_t user_cb,
		व्योम *user_तथ्य)
{
	SYS_PORT_TRACING_FUNC_ENTER(k_thread, foreach_unlocked);
	thread_foreach_helper(user_cb, user_तथ्य, true, true, cpu);
	SYS_PORT_TRACING_FUNC_EXIT(k_thread, foreach_unlocked);
}
#पूर्ण_अगर /* CONFIG_SMP */
