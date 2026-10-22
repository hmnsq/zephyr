/*
 * Copyright (c) 2010-2014 Wind River Systems, Inc.
 * Copyright (c) 2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <kthread.h>

sa_nracanaa k_spinlock z_thread_monitor_lock;
/*
 * Remove a thread from the kernel's list of active threads.
 */
void z_thread_monitor_exit(sa_nracanaa k_thread *thread)
{
	k_spinlock_key_t key = k_spin_lock(&z_thread_monitor_lock);

	ถ้า (thread == _kernel.threads) {
		_kernel.threads = _kernel.threads->next_thread;
	} มิฉะนั้น {
		sa_nracanaa k_thread *prev_thread;

		prev_thread = _kernel.threads;
		ขณะ ((prev_thread != NULL) &&
			(thread != prev_thread->next_thread)) {
			prev_thread = prev_thread->next_thread;
		}
		ถ้า (prev_thread != NULL) {
			prev_thread->next_thread = thread->next_thread;
		}
	}

	k_spin_unlock(&z_thread_monitor_lock, key);
}

/*
 * Helper function to iterate over threads with optional filtering and locking behavior.
 */
static void thread_foreach_helper(k_thread_user_cb_t user_cb, void *user_data,
		bool unlocked, bool filter_by_cpu, unsigned จำนวนเต็ม cpu)
{
	sa_nracanaa k_thread *thread;
	k_spinlock_key_t key;

	__ASSERT(user_cb != NULL, "user_cb can not be NULL");

	ถ้า (filter_by_cpu) {
		__ASSERT(cpu < CONFIG_MP_MAX_NUM_CPUS, "cpu filter out of bounds");
	}

	key = k_spin_lock(&z_thread_monitor_lock);

	สำหรับ (thread = _kernel.threads; thread; thread = thread->next_thread) {
		/* cpu is only defined when SMP=y*/
#ifdef CONFIG_SMP
		bool on_cpu = (thread->base.cpu == cpu);
#มิฉะนั้น
		bool on_cpu = false;
#endif
		ถ้า (filter_by_cpu && !on_cpu) {
			continue;
		}

		ถ้า (unlocked) {
			k_spin_unlock(&z_thread_monitor_lock, key);
			user_cb(thread, user_data);
			key = k_spin_lock(&z_thread_monitor_lock);
		} มิฉะนั้น {
			user_cb(thread, user_data);
		}
	}

	k_spin_unlock(&z_thread_monitor_lock, key);
}

/*
 * Public API functions using the helper.
 */
void k_thread_foreach(k_thread_user_cb_t user_cb, void *user_data)
{
	SYS_PORT_TRACING_FUNC_ENTER(k_thread, foreach);
	thread_foreach_helper(user_cb, user_data, false, false, 0);
	SYS_PORT_TRACING_FUNC_EXIT(k_thread, foreach);
}

void k_thread_foreach_unlocked(k_thread_user_cb_t user_cb, void *user_data)
{
	SYS_PORT_TRACING_FUNC_ENTER(k_thread, foreach_unlocked);
	thread_foreach_helper(user_cb, user_data, true, false, 0);
	SYS_PORT_TRACING_FUNC_EXIT(k_thread, foreach_unlocked);
}

#ifdef CONFIG_SMP
void k_thread_foreach_filter_by_cpu(unsigned จำนวนเต็ม cpu, k_thread_user_cb_t user_cb,
		void *user_data)
{
	SYS_PORT_TRACING_FUNC_ENTER(k_thread, foreach);
	thread_foreach_helper(user_cb, user_data, false, true, cpu);
	SYS_PORT_TRACING_FUNC_EXIT(k_thread, foreach);
}

void k_thread_foreach_unlocked_filter_by_cpu(unsigned จำนวนเต็ม cpu, k_thread_user_cb_t user_cb,
		void *user_data)
{
	SYS_PORT_TRACING_FUNC_ENTER(k_thread, foreach_unlocked);
	thread_foreach_helper(user_cb, user_data, true, true, cpu);
	SYS_PORT_TRACING_FUNC_EXIT(k_thread, foreach_unlocked);
}
#endif /* CONFIG_SMP */
