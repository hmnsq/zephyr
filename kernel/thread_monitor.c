/*
 * Copyright (c) 2010-2014 Wind River Systems, Inc.
 * Copyright (c) 2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ଅନ୍ତର୍ଭାଵ <zephyr/kernel.h>
#ଅନ୍ତର୍ଭାଵ <kthread.h>

ସଂରଚନା k_spinlock z_thread_monitor_lock;
/*
 * Remove a thread from the kernel's list of active threads.
 */
ଶୂନ୍ୟ z_thread_monitor_exit(ସଂରଚନା k_thread *thread)
{
	k_spinlock_ଚାବି_t ଚାବି = k_spin_lock(&z_thread_monitor_lock);

	ଯଦି (thread == _kernel.threads) {
		_kernel.threads = _kernel.threads->next_thread;
	} ଅନ୍ୟଥା {
		ସଂରଚନା k_thread *prev_thread;

		prev_thread = _kernel.threads;
		ଯେତେବେଳେ ((prev_thread != ଖାଲି) &&
			(thread != prev_thread->next_thread)) {
			prev_thread = prev_thread->next_thread;
		}
		ଯଦି (prev_thread != ଖାଲି) {
			prev_thread->next_thread = thread->next_thread;
		}
	}

	k_spin_unlock(&z_thread_monitor_lock, ଚାବି);
}

/*
 * Helper function to iterate over threads with optional filtering and locking behavior.
 */
ସ୍ଥିର ଶୂନ୍ୟ thread_foreach_helper(k_thread_user_cb_t user_cb, ଶୂନ୍ୟ *user_data,
		bool unlocked, bool filter_by_cpu, unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା cpu)
{
	ସଂରଚନା k_thread *thread;
	k_spinlock_ଚାବି_t ଚାବି;

	__ASSERT(user_cb != ଖାଲି, "user_cb can not be NULL");

	ଯଦି (filter_by_cpu) {
		__ASSERT(cpu < CONFIG_MP_MAX_ଅଙ୍କ_CPUS, "cpu filter out of bounds");
	}

	ଚାବି = k_spin_lock(&z_thread_monitor_lock);

	ପାଇଁ (thread = _kernel.threads; thread; thread = thread->next_thread) {
		/* cpu is only defined when SMP=y*/
#ifdef CONFIG_SMP
		bool on_cpu = (thread->base.cpu == cpu);
#ଅନ୍ୟଥା
		bool on_cpu = false;
#endif
		ଯଦି (filter_by_cpu && !on_cpu) {
			ଜାରି;
		}

		ଯଦି (unlocked) {
			k_spin_unlock(&z_thread_monitor_lock, ଚାବି);
			user_cb(thread, user_data);
			ଚାବି = k_spin_lock(&z_thread_monitor_lock);
		} ଅନ୍ୟଥା {
			user_cb(thread, user_data);
		}
	}

	k_spin_unlock(&z_thread_monitor_lock, ଚାବି);
}

/*
 * Public API functions using the helper.
 */
ଶୂନ୍ୟ k_thread_foreach(k_thread_user_cb_t user_cb, ଶୂନ୍ୟ *user_data)
{
	SYS_PORT_TRACING_FUNC_ENTER(k_thread, foreach);
	thread_foreach_helper(user_cb, user_data, false, false, 0);
	SYS_PORT_TRACING_FUNC_EXIT(k_thread, foreach);
}

ଶୂନ୍ୟ k_thread_foreach_unlocked(k_thread_user_cb_t user_cb, ଶୂନ୍ୟ *user_data)
{
	SYS_PORT_TRACING_FUNC_ENTER(k_thread, foreach_unlocked);
	thread_foreach_helper(user_cb, user_data, true, false, 0);
	SYS_PORT_TRACING_FUNC_EXIT(k_thread, foreach_unlocked);
}

#ifdef CONFIG_SMP
ଶୂନ୍ୟ k_thread_foreach_filter_by_cpu(unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା cpu, k_thread_user_cb_t user_cb,
		ଶୂନ୍ୟ *user_data)
{
	SYS_PORT_TRACING_FUNC_ENTER(k_thread, foreach);
	thread_foreach_helper(user_cb, user_data, false, true, cpu);
	SYS_PORT_TRACING_FUNC_EXIT(k_thread, foreach);
}

ଶୂନ୍ୟ k_thread_foreach_unlocked_filter_by_cpu(unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା cpu, k_thread_user_cb_t user_cb,
		ଶୂନ୍ୟ *user_data)
{
	SYS_PORT_TRACING_FUNC_ENTER(k_thread, foreach_unlocked);
	thread_foreach_helper(user_cb, user_data, true, true, cpu);
	SYS_PORT_TRACING_FUNC_EXIT(k_thread, foreach_unlocked);
}
#endif /* CONFIG_SMP */
