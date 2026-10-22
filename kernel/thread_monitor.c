/*
 * Copyright (c) 2010-2014 Wind River Systems, Inc.
 * Copyright (c) 2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ಅನ್ತರ್ಭಾವ <zephyr/kernel.h>
#ಅನ್ತರ್ಭಾವ <kthread.h>

ರಚನೆ k_spinlock z_thread_monitor_ಬೀಗ;
/*
 * Remove a thread from the kernel's list of active threads.
 */
ಶೂನ್ಯ z_thread_monitor_exit(ರಚನೆ k_thread *thread)
{
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_ಬೀಗ(&z_thread_monitor_ಬೀಗ);

	ಆದರೆ (thread == _kernel.ಎಳೆಗಳು) {
		_kernel.ಎಳೆಗಳು = _kernel.ಎಳೆಗಳು->ಮುಂದಿನ_thread;
	} ಇಲ್ಲದಿದ್ದರೆ {
		ರಚನೆ k_thread *prev_thread;

		prev_thread = _kernel.ಎಳೆಗಳು;
		ಆಗ ((prev_thread != ಖಾಲಿ) &&
			(thread != prev_thread->ಮುಂದಿನ_thread)) {
			prev_thread = prev_thread->ಮುಂದಿನ_thread;
		}
		ಆದರೆ (prev_thread != ಖಾಲಿ) {
			prev_thread->ಮುಂದಿನ_thread = thread->ಮುಂದಿನ_thread;
		}
	}

	k_spin_unlock(&z_thread_monitor_ಬೀಗ, ಕೀಲಿ);
}

/*
 * Helper function to iterate over threads with optional filtering and locking behavior.
 */
ಸ್ಥಿರ ಶೂನ್ಯ thread_foreach_helper(k_thread_user_cb_t user_cb, ಶೂನ್ಯ *user_ದತ್ತಾಂಶ,
		bool unlocked, bool filter_by_cpu, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ cpu)
{
	ರಚನೆ k_thread *thread;
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ;

	__ASSERT(user_cb != ಖಾಲಿ, "user_cb can not be NULL");

	ಆದರೆ (filter_by_cpu) {
		__ASSERT(cpu < CONFIG_MP_MAX_ಅಂಕೆ_CPUS, "cpu filter out of bounds");
	}

	ಕೀಲಿ = k_spin_ಬೀಗ(&z_thread_monitor_ಬೀಗ);

	ಗಾಗಿ (thread = _kernel.ಎಳೆಗಳು; thread; thread = thread->ಮುಂದಿನ_thread) {
		/* cpu is only defined when SMP=y*/
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SMP
		bool on_cpu = (thread->ತಳ.cpu == cpu);
#ಇಲ್ಲದಿದ್ದರೆ
		bool on_cpu = false;
#ಆದರೆ_ಅಂತ್ಯ
		ಆದರೆ (filter_by_cpu && !on_cpu) {
			ಮುಂದುವರಿ;
		}

		ಆದರೆ (unlocked) {
			k_spin_unlock(&z_thread_monitor_ಬೀಗ, ಕೀಲಿ);
			user_cb(thread, user_ದತ್ತಾಂಶ);
			ಕೀಲಿ = k_spin_ಬೀಗ(&z_thread_monitor_ಬೀಗ);
		} ಇಲ್ಲದಿದ್ದರೆ {
			user_cb(thread, user_ದತ್ತಾಂಶ);
		}
	}

	k_spin_unlock(&z_thread_monitor_ಬೀಗ, ಕೀಲಿ);
}

/*
 * Public API functions using the helper.
 */
ಶೂನ್ಯ k_thread_foreach(k_thread_user_cb_t user_cb, ಶೂನ್ಯ *user_ದತ್ತಾಂಶ)
{
	SYS_ಪೋರ್ಟ್_TRACING_FUNC_ENTER(k_thread, foreach);
	thread_foreach_helper(user_cb, user_ದತ್ತಾಂಶ, false, false, 0);
	SYS_ಪೋರ್ಟ್_TRACING_FUNC_EXIT(k_thread, foreach);
}

ಶೂನ್ಯ k_thread_foreach_unlocked(k_thread_user_cb_t user_cb, ಶೂನ್ಯ *user_ದತ್ತಾಂಶ)
{
	SYS_ಪೋರ್ಟ್_TRACING_FUNC_ENTER(k_thread, foreach_unlocked);
	thread_foreach_helper(user_cb, user_ದತ್ತಾಂಶ, true, false, 0);
	SYS_ಪೋರ್ಟ್_TRACING_FUNC_EXIT(k_thread, foreach_unlocked);
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SMP
ಶೂನ್ಯ k_thread_foreach_filter_by_cpu(ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ cpu, k_thread_user_cb_t user_cb,
		ಶೂನ್ಯ *user_ದತ್ತಾಂಶ)
{
	SYS_ಪೋರ್ಟ್_TRACING_FUNC_ENTER(k_thread, foreach);
	thread_foreach_helper(user_cb, user_ದತ್ತಾಂಶ, false, true, cpu);
	SYS_ಪೋರ್ಟ್_TRACING_FUNC_EXIT(k_thread, foreach);
}

ಶೂನ್ಯ k_thread_foreach_unlocked_filter_by_cpu(ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ cpu, k_thread_user_cb_t user_cb,
		ಶೂನ್ಯ *user_ದತ್ತಾಂಶ)
{
	SYS_ಪೋರ್ಟ್_TRACING_FUNC_ENTER(k_thread, foreach_unlocked);
	thread_foreach_helper(user_cb, user_ದತ್ತಾಂಶ, true, true, cpu);
	SYS_ಪೋರ್ಟ್_TRACING_FUNC_EXIT(k_thread, foreach_unlocked);
}
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_SMP */
