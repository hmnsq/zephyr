/*
 * Copyright (c) 2010-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief Kernel thread support
 *
 * This module provides general purpose thread support.
 */

#ಅನ್ತರ್ಭಾವ <zephyr/kernel.h>
#ಅನ್ತರ್ಭಾವ <zephyr/spinlock.h>
#ಅನ್ತರ್ಭಾವ <zephyr/sys/math_extras.h>
#ಅನ್ತರ್ಭಾವ <zephyr/sys/clock.h>
#ಅನ್ತರ್ಭಾವ <ksched.h>
#ಅನ್ತರ್ಭಾವ <kthread.h>
#ಅನ್ತರ್ಭಾವ <wait_q.h>
#ಅನ್ತರ್ಭಾವ <timeout_q.h>
#ಅನ್ತರ್ಭಾವ <zephyr/internal/syscall_handler.h>
#ಅನ್ತರ್ಭಾವ <kernel_internal.h>
#ಅನ್ತರ್ಭಾವ <kswap.h>
#ಅನ್ತರ್ಭಾವ <zephyr/init.h>
#ಅನ್ತರ್ಭಾವ <zephyr/tracing/tracing.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <stdbool.h>
#ಅನ್ತರ್ಭಾವ <zephyr/sys/check.h>
#ಅನ್ತರ್ಭಾವ <zephyr/random/random.h>
#ಅನ್ತರ್ಭಾವ <zephyr/sys/atomic.h>
#ಅನ್ತರ್ಭಾವ <zephyr/logging/log.h>
#ಅನ್ತರ್ಭಾವ <zephyr/llext/symbol.h>
#ಅನ್ತರ್ಭಾವ <zephyr/sys/iterable_sections.h>
#ಅನ್ತರ್ಭಾವ <zephyr/sys/minmax.h>

#ಅನ್ತರ್ಭಾವ <usage.h>

LOG_MODULE_DECLARE(os, CONFIG_KERNEL_LOG_LEVEL);

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_OBJ_CORE_THREAD
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_OBJ_CORE_STATS_THREAD
ಸ್ಥಿರ ರಚನೆ k_obj_core_stats_desc  thread_stats_desc = {
	.raw_ಗಾತ್ರ = ಗಾತ್ರ(ರಚನೆ k_ಚಕ್ರ_stats),
	.query_ಗಾತ್ರ = ಗಾತ್ರ(ರಚನೆ k_thread_runtime_stats),
	.raw   = z_thread_stats_raw,
	.query = z_thread_stats_query,
	.reset = z_thread_stats_reset,
	.disable = z_thread_stats_disable,
	.enable  = z_thread_stats_enable,
};
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_OBJ_CORE_STATS_THREAD */

/* Threads have no statically defined instances to walk; each thread links its
 * own object core as it is created (see z_setup_new_thread()). Register the
 * type only.
 */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_OBJ_CORE_STATS_THREAD
K_OBJ_TYPE_DEFINE_TYPE_ONLY(obj_type_thread, k_thread, K_OBJ_TYPE_THREAD_ID,
			    &thread_stats_desc);
#ಇಲ್ಲದಿದ್ದರೆ
K_OBJ_TYPE_DEFINE_TYPE_ONLY(obj_type_thread, k_thread, K_OBJ_TYPE_THREAD_ID,
			    ಖಾಲಿ);
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_OBJ_CORE_STATS_THREAD */
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_OBJ_CORE_THREAD */


#ವ್ಯಾಖ್ಯಾನಿಸು _FOREACH_STATIC_THREAD(thread_ದತ್ತಾಂಶ)              \
	STRUCT_SECTION_FOREACH(static_thread_ದತ್ತಾಂಶ, thread_ದತ್ತಾಂಶ)

bool k_is_ಒಳ_isr(ಶೂನ್ಯ)
{
	ಹಿಂತಿರುಗಿ arch_is_ಒಳ_isr();
}
EXPORT_ಚಿಹ್ನೆ(k_is_ಒಳ_isr);

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_THREAD_CUSTOM_ದತ್ತಾಂಶ
ಶೂನ್ಯ z_impl_k_thread_custom_ದತ್ತಾಂಶ_ಗಣ(ಶೂನ್ಯ *ಮೌಲ್ಯ)
{
	_current->custom_ದತ್ತಾಂಶ = ಮೌಲ್ಯ;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_USERSPACE
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ z_vrfy_k_thread_custom_ದತ್ತಾಂಶ_ಗಣ(ಶೂನ್ಯ *ದತ್ತಾಂಶ)
{
	z_impl_k_thread_custom_ದತ್ತಾಂಶ_ಗಣ(ದತ್ತಾಂಶ);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_thread_custom_data_set_mrsh.c>
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_USERSPACE */

ಶೂನ್ಯ *z_impl_k_thread_custom_ದತ್ತಾಂಶ_get(ಶೂನ್ಯ)
{
	ಹಿಂತಿರುಗಿ _current->custom_ದತ್ತಾಂಶ;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_USERSPACE
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ *z_vrfy_k_thread_custom_ದತ್ತಾಂಶ_get(ಶೂನ್ಯ)
{
	ಹಿಂತಿರುಗಿ z_impl_k_thread_custom_ದತ್ತಾಂಶ_get();
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_thread_custom_data_get_mrsh.c>

#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_USERSPACE */
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_THREAD_CUSTOM_DATA */

ಪೂರ್ಣಾಂಕ z_impl_k_is_preempt_thread(ಶೂನ್ಯ)
{
	ಹಿಂತಿರುಗಿ !arch_is_ಒಳ_isr() && thread_is_preemptible(_current);
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_USERSPACE
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಪೂರ್ಣಾಂಕ z_vrfy_k_is_preempt_thread(ಶೂನ್ಯ)
{
	ಹಿಂತಿರುಗಿ z_impl_k_is_preempt_thread();
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_is_preempt_thread_mrsh.c>
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_USERSPACE */

ಪೂರ್ಣಾಂಕ z_impl_k_thread_ಆದ್ಯತೆ_get(k_tid_t thread)
{
	ಹಿಂತಿರುಗಿ thread->ತಳ.prio;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_USERSPACE
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಪೂರ್ಣಾಂಕ z_vrfy_k_thread_ಆದ್ಯತೆ_get(k_tid_t thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	ಹಿಂತಿರುಗಿ z_impl_k_thread_ಆದ್ಯತೆ_get(thread);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_thread_priority_get_mrsh.c>
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_USERSPACE */

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_THREAD_NAME
ಸ್ಥಿರ ಶೂನ್ಯ ಗಣ_thread_name(ರಚನೆ k_thread *thread, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *str)
{
	ಆದರೆ (str != ಖಾಲಿ) {
		strncpy(thread->name, str, CONFIG_THREAD_MAX_NAME_LEN - 1);
		/* Ensure NULL termination, truncate if longer */
		thread->name[CONFIG_THREAD_MAX_NAME_LEN - 1] = '\0';
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_ARCH_HAS_THREAD_NAME_HOOK
		arch_thread_name_ಗಣ(thread, str);
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_ARCH_HAS_THREAD_NAME_HOOK */
	} ಇಲ್ಲದಿದ್ದರೆ {
		thread->name[0] = '\0';
	}
}
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_THREAD_NAME */

ಪೂರ್ಣಾಂಕ z_impl_k_thread_name_ಗಣ(k_tid_t thread, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *str)
{
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_THREAD_NAME
	ಆದರೆ (thread == ಖಾಲಿ) {
		thread = _current;
	}

	ಗಣ_thread_name(thread, str);

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC(k_thread, name_ಗಣ, thread, 0);

	ಹಿಂತಿರುಗಿ 0;
#ಇಲ್ಲದಿದ್ದರೆ
	ARG_ಬಳಸದ(thread);
	ARG_ಬಳಸದ(str);

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC(k_thread, name_ಗಣ, thread, -ENOSYS);

	ಹಿಂತಿರುಗಿ -ENOSYS;
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_THREAD_NAME */
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_USERSPACE
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಪೂರ್ಣಾಂಕ z_vrfy_k_thread_name_ಗಣ(k_tid_t thread, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *str)
{
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_THREAD_NAME
	ಅಕ್ಷರ name[CONFIG_THREAD_MAX_NAME_LEN];

	ಆದರೆ (thread != ಖಾಲಿ) {
		ಆದರೆ (K_SYSCALL_OBJ(thread, K_OBJ_THREAD) != 0) {
			ಹಿಂತಿರುಗಿ -EINVAL;
		}
	}

	/* In theory we could copy directly into thread->name, but
	 * the current z_vrfy / z_impl split does not provide a
	 * means of doing so.
	 */
	ಆದರೆ (k_usermode_string_copy(name, str, ಗಾತ್ರ(name)) != 0) {
		ಹಿಂತಿರುಗಿ -EFAULT;
	}

	ಹಿಂತಿರುಗಿ z_impl_k_thread_name_ಗಣ(thread, name);
#ಇಲ್ಲದಿದ್ದರೆ
	ಹಿಂತಿರುಗಿ -ENOSYS;
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_THREAD_NAME */
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_thread_name_set_mrsh.c>
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_USERSPACE */

ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *k_thread_name_get(k_tid_t thread)
{
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_THREAD_NAME
	ಹಿಂತಿರುಗಿ (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *)thread->name;
#ಇಲ್ಲದಿದ್ದರೆ
	ARG_ಬಳಸದ(thread);
	ಹಿಂತಿರುಗಿ ಖಾಲಿ;
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_THREAD_NAME */
}

ಪೂರ್ಣಾಂಕ z_impl_k_thread_name_copy(k_tid_t thread, ಅಕ್ಷರ *buf, size_t ಗಾತ್ರ_2)
{
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_THREAD_NAME
	strncpy(buf, thread->name, ಗಾತ್ರ_2);
	ಹಿಂತಿರುಗಿ 0;
#ಇಲ್ಲದಿದ್ದರೆ
	ARG_ಬಳಸದ(thread);
	ARG_ಬಳಸದ(buf);
	ARG_ಬಳಸದ(ಗಾತ್ರ_2);
	ಹಿಂತಿರುಗಿ -ENOSYS;
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_THREAD_NAME */
}

ಸ್ಥಿರ size_t copy_ಬೈಟುಗಳು(ಅಕ್ಷರ *dest, size_t dest_ಗಾತ್ರ, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *src, size_t src_ಗಾತ್ರ)
{
	size_t  ಬೈಟುಗಳು_ಗಮ್ಯ_copy;

	ಬೈಟುಗಳು_ಗಮ್ಯ_copy = min(dest_ಗಾತ್ರ, src_ಗಾತ್ರ);
	ಸ್ಮರಣೆ_ನಕಲು(dest, src, ಬೈಟುಗಳು_ಗಮ್ಯ_copy);

	ಹಿಂತಿರುಗಿ ಬೈಟುಗಳು_ಗಮ್ಯ_copy;
}

ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *k_thread_state_str(k_tid_t thread_id, ಅಕ್ಷರ *buf, size_t buf_ಗಾತ್ರ)
{
	size_t      off = 0;
	uint8_t     ಬಿಟ್_2;
	uint8_t     thread_state = thread_id->ತಳ.thread_state;
#ವ್ಯಾಖ್ಯಾನಿಸು SS_ENT(s) { Z_STATE_STR_##s, _THREAD_##s, ಗಾತ್ರ(Z_STATE_STR_##s) - 1 }
	ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ರಚನೆ {
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *str;
		uint16_t    ಬಿಟ್_2;
		uint16_t    len;
	} state_string[] = {
		SS_ENT(DUMMY),
		SS_ENT(PENDING),
		SS_ENT(SLEEPING),
		SS_ENT(DEAD),
		SS_ENT(SUSPENDED),
		SS_ENT(ABORTING),
		SS_ENT(SUSPENDING),
		SS_ENT(QUEUED),
	};
#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು SS_ENT

	ಆದರೆ ((buf == ಖಾಲಿ) || (buf_ಗಾತ್ರ == 0)) {
		ಹಿಂತಿರುಗಿ "";
	}

	buf_ಗಾತ್ರ--;   /* Reserve 1 byte for end-of-string character */

	/*
	 * Loop through each bit in the thread_state. Stop once all have
	 * been processed. If more than one thread_state bit is set, then
	 * separate the descriptive strings with a '+'.
	 */


	ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಸೂಚ್ಯಂಕ = 0; thread_state != 0; ಸೂಚ್ಯಂಕ++) {
		ಬಿಟ್_2 = state_string[ಸೂಚ್ಯಂಕ].ಬಿಟ್_2;
		ಆದರೆ ((thread_state & ಬಿಟ್_2) == 0) {
			ಮುಂದುವರಿ;
		}

		off += copy_ಬೈಟುಗಳು(buf + off, buf_ಗಾತ್ರ - off,
				  state_string[ಸೂಚ್ಯಂಕ].str,
				  state_string[ಸೂಚ್ಯಂಕ].len);

		thread_state &= ~ಬಿಟ್_2;

		ಆದರೆ (thread_state != 0) {
			off += copy_ಬೈಟುಗಳು(buf + off, buf_ಗಾತ್ರ - off, "+", 1);
		}
	}

	buf[off] = '\0';

	ಹಿಂತಿರುಗಿ (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *)buf;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_USERSPACE
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಪೂರ್ಣಾಂಕ z_vrfy_k_thread_name_copy(k_tid_t thread,
					    ಅಕ್ಷರ *buf, size_t ಗಾತ್ರ_2)
{
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_THREAD_NAME
	size_t len;
	ರಚನೆ k_object *ko = k_object_ಕಂಡುಹಿಡಿ(thread);

	/* Special case: we allow reading the names of initialized threads
	 * even if we don't have permission on them
	 */
	ಆದರೆ ((ko == ಖಾಲಿ) || (ko->type != K_OBJ_THREAD) ||
		((ko->flags & K_OBJ_FLAG_INITIALIZED) == 0)) {
		ಹಿಂತಿರುಗಿ -EINVAL;
	}
	ಆದರೆ (K_SYSCALL_ಸ್ಮರಣೆ_WRITE(buf, ಗಾತ್ರ_2) != 0) {
		ಹಿಂತಿರುಗಿ -EFAULT;
	}
	len = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(thread->name);
	ಆದರೆ ((len + 1) > ಗಾತ್ರ_2) {
		ಹಿಂತಿರುಗಿ -ENOSPC;
	}

	ಹಿಂತಿರುಗಿ k_usermode_ಗಮ್ಯ_copy((ಶೂನ್ಯ *)buf, thread->name, len + 1);
#ಇಲ್ಲದಿದ್ದರೆ
	ARG_ಬಳಸದ(thread);
	ARG_ಬಳಸದ(buf);
	ARG_ಬಳಸದ(ಗಾತ್ರ_2);
	ಹಿಂತಿರುಗಿ -ENOSYS;
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_THREAD_NAME */
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_thread_name_copy_mrsh.c>
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_USERSPACE */

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_ರಾಶಿ_SENTINEL
/* Check that the stack sentinel is still present
 *
 * The stack sentinel feature writes a magic value to the lowest 4 bytes of
 * the thread's stack when the thread is initialized. This value gets checked
 * in a few places:
 *
 * 1) In k_yield() if the current thread is not swapped out
 * 2) After servicing a non-nested interrupt
 * 3) In z_swap(), check the sentinel in the outgoing thread
 *
 * Item 2 requires support in arch/ code.
 *
 * If the check fails, the thread will be terminated appropriately through
 * the system fatal error handler.
 */
ಶೂನ್ಯ z_check_ರಾಶಿ_sentinel(ಶೂನ್ಯ)
{
	uint32_t *ರಾಶಿ;

	ಆದರೆ ((_current->ತಳ.thread_state & _THREAD_DUMMY) != 0) {
		ಹಿಂತಿರುಗಿ;
	}

	ರಾಶಿ = (uint32_t *)_current->ರಾಶಿ_info.ಆರಂಭ;
	ಆದರೆ (*ರಾಶಿ != ರಾಶಿ_SENTINEL) {
		/* Restore it so further checks don't trigger this same error */
		*ರಾಶಿ = ರಾಶಿ_SENTINEL;
		z_except_reason(K_ERR_ರಾಶಿ_CHK_FAIL);
	}
}
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_STACK_SENTINEL */

#ಆದರೆ defined(CONFIG_ರಾಶಿ_ಸೂಚಕ_RANDOM) && (CONFIG_ರಾಶಿ_ಸೂಚಕ_RANDOM != 0)
ಪೂರ್ಣಾಂಕ z_ರಾಶಿ_adjust_initialized;

ಸ್ಥಿರ size_t random_ಸ್ಥಾನಾಂತರ(size_t ರಾಶಿ_ಗಾತ್ರ)
{
	size_t random_val;

	ಆದರೆ (!z_ರಾಶಿ_adjust_initialized) {
		z_early_rand_get((uint8_t *)&random_val, ಗಾತ್ರ(random_val));
	} ಇಲ್ಲದಿದ್ದರೆ {
		sys_rand_get((uint8_t *)&random_val, ಗಾತ್ರ(random_val));
	}

	/* Don't need to worry about alignment of the size here,
	 * arch_new_thread() is required to do it.
	 *
	 * FIXME: Not the best way to get a random number in a range.
	 * See #6493
	 */
	ಸ್ಥಿರಾಂಕ size_t fuzz = random_val % CONFIG_ರಾಶಿ_ಸೂಚಕ_RANDOM;

	ಆದರೆ (unlikely(fuzz * 2 > ರಾಶಿ_ಗಾತ್ರ)) {
		ಹಿಂತಿರುಗಿ 0;
	}

	ಹಿಂತಿರುಗಿ fuzz;
}
#ಆದರೆ defined(CONFIG_ರಾಶಿ_GROWS_ಮೇಲೆ)
	/* This is so rare not bothering for now */
#ದೋಷ "Stack pointer randomization not implemented for upward growing stacks"
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_STACK_GROWS_UP */
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_STACK_POINTER_RANDOM */

/*
 * Geometry of a thread's stack object, computed once and threaded through the
 * setup_thread_stack() helpers below.
 */
ರಚನೆ ರಾಶಿ_geometry {
	size_t obj_ಗಾತ್ರ;	/* size of the whole stack object */
	ಅಕ್ಷರ *buf_ಆರಂಭ;	/* start of the usable stack buffer */
	size_t buf_ಗಾತ್ರ;	/* size of the usable stack buffer */
};

/*
 * Derive the stack object geometry from the caller-supplied stack and size.
 * User-capable stacks and kernel-only stacks reserve different amounts.
 */
ಸ್ಥಿರ ರಚನೆ ರಾಶಿ_geometry compute_ರಾಶಿ_geometry(k_thread_ರಾಶಿ_t *ರಾಶಿ,
						    size_t ರಾಶಿ_ಗಾತ್ರ)
{
	ರಚನೆ ರಾಶಿ_geometry geo;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_USERSPACE
	ಆದರೆ (z_ರಾಶಿ_is_user_capable(ರಾಶಿ)) {
		geo.obj_ಗಾತ್ರ = K_THREAD_ರಾಶಿ_LEN(ರಾಶಿ_ಗಾತ್ರ);
		geo.buf_ಆರಂಭ = K_THREAD_ರಾಶಿ_ಬಫರ್(ರಾಶಿ);
		geo.buf_ಗಾತ್ರ = geo.obj_ಗಾತ್ರ - K_THREAD_ರಾಶಿ_RESERVED;
	} ಇಲ್ಲದಿದ್ದರೆ
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_USERSPACE */
	{
		/* Object cannot host a user mode thread */
		geo.obj_ಗಾತ್ರ = K_KERNEL_ರಾಶಿ_LEN(ರಾಶಿ_ಗಾತ್ರ);
		geo.buf_ಆರಂಭ = K_KERNEL_ರಾಶಿ_ಬಫರ್(ರಾಶಿ);
		geo.buf_ಗಾತ್ರ = geo.obj_ಗಾತ್ರ - K_KERNEL_ರಾಶಿ_RESERVED;

#ಆದರೆ defined(ARCH_KERNEL_ರಾಶಿ_RESERVED)
		/* Zephyr treats stack overflow as an app bug.  But
		 * this particular overflow can be seen by static
		 * analysis so needs to be handled somehow.
		 */
		ಆದರೆ (K_KERNEL_ರಾಶಿ_RESERVED > geo.obj_ಗಾತ್ರ) {
			k_panic();
		}
#ಆದರೆ_ಅಂತ್ಯ
	}

	ಹಿಂತಿರುಗಿ geo;
}

/*
 * Return the initial (high end) stack pointer. When stacks are memory mapped
 * this maps the object into virtual memory, records the mapping so it can be
 * torn down at thread exit, and rebases buf_start onto the mapped address.
 */
ಸ್ಥಿರ ಅಕ್ಷರ *map_thread_ರಾಶಿ(ರಚನೆ k_thread *new_thread,
			      k_thread_ರಾಶಿ_t *ರಾಶಿ,
			      ರಚನೆ ರಾಶಿ_geometry *geo)
{
	ಅಕ್ಷರ *ರಾಶಿ_ptr;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_THREAD_ರಾಶಿ_MEM_MAPPED
	/* Map the stack into virtual memory and use that as the base to
	 * calculate the initial stack pointer at the high end of the stack
	 * object. The stack pointer may be reduced later by TLS or random
	 * offset.
	 *
	 * K_MEM_MAP_UNINIT is used to mimic the behavior of non-mapped
	 * stack. If CONFIG_INIT_STACKS is enabled, the stack will be
	 * cleared later.
	 */
	ಶೂನ್ಯ *ರಾಶಿ_mapped = k_mem_map_phys_guard((uintptr_t)ರಾಶಿ, geo->obj_ಗಾತ್ರ,
				K_MEM_PERM_RW | K_MEM_CACHE_WB | K_MEM_MAP_UNINIT,
				false);

	__ASSERT_NO_MSG((uintptr_t)ರಾಶಿ_mapped != 0);

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_USERSPACE
	ಆದರೆ (z_ರಾಶಿ_is_user_capable(ರಾಶಿ)) {
		geo->buf_ಆರಂಭ = K_THREAD_ರಾಶಿ_ಬಫರ್(ರಾಶಿ_mapped);
	} ಇಲ್ಲದಿದ್ದರೆ
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_USERSPACE */
	{
		geo->buf_ಆರಂಭ = K_KERNEL_ರಾಶಿ_ಬಫರ್(ರಾಶಿ_mapped);
	}

	ರಾಶಿ_ptr = (ಅಕ್ಷರ *)ರಾಶಿ_mapped + geo->obj_ಗಾತ್ರ;

	/* Need to store the info on mapped stack so we can remove the mappings
	 * when the thread ends.
	 */
	new_thread->ರಾಶಿ_info.mapped.addr = ರಾಶಿ_mapped;
	new_thread->ರಾಶಿ_info.mapped.sz = geo->obj_ಗಾತ್ರ;
#ಇಲ್ಲದಿದ್ದರೆ /* CONFIG_THREAD_STACK_MEM_MAPPED */
	ARG_ಬಳಸದ(new_thread);

	/* Initial stack pointer at the high end of the stack object, may
	 * be reduced later by TLS or random offset
	 */
	ರಾಶಿ_ptr = (ಅಕ್ಷರ *)ರಾಶಿ + geo->obj_ಗಾತ್ರ;
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_THREAD_STACK_MEM_MAPPED */

	ಹಿಂತಿರುಗಿ ರಾಶಿ_ptr;
}

/* Pre-fill the stack buffer with a known pattern for usage measurement. */
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ fill_init_ರಾಶಿ(ಅಕ್ಷರ *buf_ಆರಂಭ, size_t buf_ಗಾತ್ರ)
{
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_INIT_STACKS
	memset(buf_ಆರಂಭ, 0xaa, buf_ಗಾತ್ರ);
#ಇಲ್ಲದಿದ್ದರೆ
	ARG_ಬಳಸದ(buf_ಆರಂಭ);
	ARG_ಬಳಸದ(buf_ಗಾತ್ರ);
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_INIT_STACKS */
}

/* Place the overflow-detection sentinel at the low end of the stack buffer. */
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ ಗಣ_ರಾಶಿ_sentinel(ಅಕ್ಷರ *buf_ಆರಂಭ)
{
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_ರಾಶಿ_SENTINEL
	/* Put the stack sentinel at the lowest 4 bytes of the stack area.
	 * We periodically check that it's still present and kill the thread
	 * if it isn't.
	 */
	*((uint32_t *)buf_ಆರಂಭ) = ರಾಶಿ_SENTINEL;
#ಇಲ್ಲದಿದ್ದರೆ
	ARG_ಬಳಸದ(buf_ಆರಂಭ);
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_STACK_SENTINEL */
}

/*
 * Compute how much to reserve at the top of the stack buffer for TLS,
 * userspace-local data and stack-pointer randomization, and return the
 * aligned amount the initial stack pointer must be lowered by.
 */
ಸ್ಥಿರ size_t reserve_ರಾಶಿ_headroom(ರಚನೆ k_thread *new_thread,
				     ಅಕ್ಷರ *ರಾಶಿ_ptr, size_t buf_ಗಾತ್ರ)
{
	size_t delta = 0;

	ARG_ಬಳಸದ(new_thread);
	ARG_ಬಳಸದ(ರಾಶಿ_ptr);
	ARG_ಬಳಸದ(buf_ಗಾತ್ರ);

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_THREAD_LOCAL_STORAGE
	/* TLS is always last within the stack buffer */
	delta += arch_tls_ರಾಶಿ_ಸಿದ್ಧತೆ(new_thread, ರಾಶಿ_ptr);
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_THREAD_LOCAL_STORAGE */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_THREAD_USERSPACE_LOCAL_ದತ್ತಾಂಶ
	size_t tls_ಗಾತ್ರ = ಗಾತ್ರ(ರಚನೆ thread_userspace_local_ದತ್ತಾಂಶ);

	/* reserve space on highest memory of stack buffer for local data */
	delta += tls_ಗಾತ್ರ;
	new_thread->userspace_local_ದತ್ತಾಂಶ =
		(ರಚನೆ thread_userspace_local_ದತ್ತಾಂಶ *)(ರಾಶಿ_ptr - delta);
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_THREAD_USERSPACE_LOCAL_DATA */
#ಆದರೆ defined(CONFIG_ರಾಶಿ_ಸೂಚಕ_RANDOM) && (CONFIG_ರಾಶಿ_ಸೂಚಕ_RANDOM != 0)
	delta += random_ಸ್ಥಾನಾಂತರ(buf_ಗಾತ್ರ);
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_STACK_POINTER_RANDOM */

	ಹಿಂತಿರುಗಿ ಸುತ್ತು_ಮೇಲೆ(delta, ARCH_ರಾಶಿ_PTR_ALIGN);
}

/* Record the thread-accessible stack bounds (and runtime-safety threshold). */
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ ಗಣ_ರಾಶಿ_info(ರಚನೆ k_thread *new_thread, ಅಕ್ಷರ *buf_ಆರಂಭ,
				  size_t buf_ಗಾತ್ರ, size_t delta)
{
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_THREAD_ರಾಶಿ_INFO
	/* Initial values. Arches which implement MPU guards that "borrow"
	 * memory from the stack buffer (not tracked in K_THREAD_STACK_RESERVED)
	 * will need to appropriately update this.
	 *
	 * The bounds tracked here correspond to the area of the stack object
	 * that the thread can access, which includes TLS.
	 */
	new_thread->ರಾಶಿ_info.ಆರಂಭ = (uintptr_t)buf_ಆರಂಭ;
	new_thread->ರಾಶಿ_info.ಗಾತ್ರ_2 = buf_ಗಾತ್ರ;
	new_thread->ರಾಶಿ_info.delta = delta;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_THREAD_RUNTIME_ರಾಶಿ_SAFETY
	new_thread->ರಾಶಿ_info.usage.ಬಳಸದ_ಮಿತಿ =
		(CONFIG_THREAD_RUNTIME_ರಾಶಿ_SAFETY_DEFAULT_ಬಳಸದ_ಮಿತಿ_PCT *
		 buf_ಗಾತ್ರ) / 100;
#ಆದರೆ_ಅಂತ್ಯ
#ಇಲ್ಲದಿದ್ದರೆ
	ARG_ಬಳಸದ(new_thread);
	ARG_ಬಳಸದ(buf_ಆರಂಭ);
	ARG_ಬಳಸದ(buf_ಗಾತ್ರ);
	ARG_ಬಳಸದ(delta);
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_THREAD_STACK_INFO */
}

ಸ್ಥಿರ ಅಕ್ಷರ *ಸಿದ್ಧತೆ_thread_ರಾಶಿ(ರಚನೆ k_thread *new_thread,
				k_thread_ರಾಶಿ_t *ರಾಶಿ, size_t ರಾಶಿ_ಗಾತ್ರ)
{
	ರಚನೆ ರಾಶಿ_geometry geo = compute_ರಾಶಿ_geometry(ರಾಶಿ, ರಾಶಿ_ಗಾತ್ರ);
	ಅಕ್ಷರ *ರಾಶಿ_ptr = map_thread_ರಾಶಿ(new_thread, ರಾಶಿ, &geo);
	size_t delta;

	LOG_DBG("stack %p for thread %p: obj_size=%zu buf_start=%p "
		" buf_size %zu stack_ptr=%p",
		ರಾಶಿ, new_thread, geo.obj_ಗಾತ್ರ, (ಶೂನ್ಯ *)geo.buf_ಆರಂಭ,
		geo.buf_ಗಾತ್ರ, (ಶೂನ್ಯ *)ರಾಶಿ_ptr);

	fill_init_ರಾಶಿ(geo.buf_ಆರಂಭ, geo.buf_ಗಾತ್ರ);
	ಗಣ_ರಾಶಿ_sentinel(geo.buf_ಆರಂಭ);
	delta = reserve_ರಾಶಿ_headroom(new_thread, ರಾಶಿ_ptr, geo.buf_ಗಾತ್ರ);
	ಗಣ_ರಾಶಿ_info(new_thread, geo.buf_ಆರಂಭ, geo.buf_ಗಾತ್ರ, delta);

	ಹಿಂತಿರುಗಿ ರಾಶಿ_ptr - delta;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_HW_SHADOW_ರಾಶಿ
ಸ್ಥಿರ ಶೂನ್ಯ ಸಿದ್ಧತೆ_shadow_ರಾಶಿ(ರಚನೆ k_thread *new_thread,
			 k_thread_ರಾಶಿ_t *ರಾಶಿ)
{
	ಪೂರ್ಣಾಂಕ ret = -ENOENT;

	STRUCT_SECTION_FOREACH(ರಾಶಿ_ಗಮ್ಯ_hw_shadow_ರಾಶಿ, stk_ಗಮ್ಯ_hw_shstk) {
		ಆದರೆ (stk_ಗಮ್ಯ_hw_shstk->ರಾಶಿ == ರಾಶಿ) {
			ret = k_thread_hw_shadow_ರಾಶಿ_attach(new_thread,
							      stk_ಗಮ್ಯ_hw_shstk->shstk_addr,
							      stk_ಗಮ್ಯ_hw_shstk->ಗಾತ್ರ_2);
			ಆದರೆ (ret != 0) {
				LOG_ERR("Could not set thread %p shadow stack %p, got error %d",
					new_thread, stk_ಗಮ್ಯ_hw_shstk->shstk_addr, ret);
				k_panic();
			}
			ನಿಲ್ಲಿಸು;
		}
	}

	/* Check if the stack isn't in a stack array, and use the corresponding
	 * shadow stack.
	 */
	ಆದರೆ (ret != -ENOENT) {
		ಹಿಂತಿರುಗಿ;
	}

	STRUCT_SECTION_FOREACH(ರಾಶಿ_ಗಮ್ಯ_hw_shadow_ರಾಶಿ_arr, stk_ಗಮ್ಯ_hw_shstk) {
		ಆದರೆ ((uintptr_t)ರಾಶಿ >= stk_ಗಮ್ಯ_hw_shstk->ರಾಶಿ_addr &&
		    (uintptr_t)ರಾಶಿ < stk_ಗಮ್ಯ_hw_shstk->ರಾಶಿ_addr +
		    stk_ಗಮ್ಯ_hw_shstk->ರಾಶಿ_ಗಾತ್ರ * stk_ಗಮ್ಯ_hw_shstk->nmemb) {
			/* Now we have to guess which index of the stack array is being used */
			uintptr_t ರಾಶಿ_ಸ್ಥಾನಾಂತರ = (uintptr_t)ರಾಶಿ - stk_ಗಮ್ಯ_hw_shstk->ರಾಶಿ_addr;
			uintptr_t ರಾಶಿ_ಸೂಚ್ಯಂಕ = ರಾಶಿ_ಸ್ಥಾನಾಂತರ / stk_ಗಮ್ಯ_hw_shstk->ರಾಶಿ_ಗಾತ್ರ;
			uintptr_t addr;

			ಆದರೆ (ರಾಶಿ_ಸೂಚ್ಯಂಕ >= stk_ಗಮ್ಯ_hw_shstk->nmemb) {
				LOG_ERR("Could not find shadow stack for thread %p, stack %p",
					new_thread, ರಾಶಿ);
				k_panic();
			}

			addr = stk_ಗಮ್ಯ_hw_shstk->shstk_addr +
				stk_ಗಮ್ಯ_hw_shstk->shstk_ಗಾತ್ರ * ರಾಶಿ_ಸೂಚ್ಯಂಕ;
			ret = k_thread_hw_shadow_ರಾಶಿ_attach(new_thread,
							      (arch_thread_hw_shadow_ರಾಶಿ_t *)addr,
							      stk_ಗಮ್ಯ_hw_shstk->shstk_ಗಾತ್ರ);
			ಆದರೆ (ret != 0) {
				LOG_ERR("Could not set thread %p shadow stack 0x%lx, got error %d",
					new_thread, stk_ಗಮ್ಯ_hw_shstk->shstk_addr, ret);
				k_panic();
			}
			ನಿಲ್ಲಿಸು;
		}
	}

	ಆದರೆ (ret == -ENOENT) {
		LOG_ERR("Could not find shadow stack for thread %p, stack %p",
			new_thread, ರಾಶಿ);
		k_panic();
	}
}
#ಇಲ್ಲದಿದ್ದರೆ
/* No-op stub used when hardware shadow stacks are not configured. */
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ ಸಿದ್ಧತೆ_shadow_ರಾಶಿ(ರಚನೆ k_thread *new_thread,
				      k_thread_ರಾಶಿ_t *ರಾಶಿ)
{
	ARG_ಬಳಸದ(new_thread);
	ARG_ಬಳಸದ(ರಾಶಿ);
}
#ಆದರೆ_ಅಂತ್ಯ

/*
 * The following init_thread_*()/assert_*() helpers each own one optional
 * kernel feature. Building z_setup_new_thread() out of these calls keeps its
 * body a readable, linear narrative instead of a wall of #ifdefs. Every helper
 * is static inline and collapses to nothing when its feature is disabled, so
 * this is purely a readability change with no runtime cost.
 */

/* Reset per-thread cleanup bookkeeping before a k_thread object is reused. */
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ thread_abort_cleanup_check_reuse(ರಚನೆ k_thread *thread)
{
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_THREAD_ABORT_NEED_CLEANUP
	k_thread_abort_cleanup_check_reuse(thread);
#ಇಲ್ಲದಿದ್ದರೆ
	ARG_ಬಳಸದ(thread);
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_THREAD_ABORT_NEED_CLEANUP */
}

/* Register the thread with the object-core framework and its usage stats. */
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ init_thread_obj_core(ರಚನೆ k_thread *thread)
{
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_OBJ_CORE_THREAD
	k_obj_core_init_and_link(K_OBJ_CORE(thread), &obj_type_thread);
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_OBJ_CORE_STATS_THREAD
	k_obj_core_stats_ರಿಜಿಸ್ಟರ್(K_OBJ_CORE(thread), &thread->ತಳ.usage,
				  ಗಾತ್ರ(thread->ತಳ.usage));
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_OBJ_CORE_STATS_THREAD */
#ಇಲ್ಲದಿದ್ದರೆ
	ARG_ಬಳಸದ(thread);
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_OBJ_CORE_THREAD */
}

/*
 * Initialize userspace bookkeeping: register the thread and stack kernel
 * objects, record stack ownership, and grant the thread access to itself.
 */
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ init_thread_userspace(ರಚನೆ k_thread *thread,
					 k_thread_ರಾಶಿ_t *ರಾಶಿ,
					 uint32_t options)
{
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_USERSPACE
	__ASSERT((options & K_USER) == 0U || z_ರಾಶಿ_is_user_capable(ರಾಶಿ),
		 "user thread %p with kernel-only stack %p", thread, ರಾಶಿ);
	k_object_init(thread);
	k_object_init(ರಾಶಿ);
	thread->ರಾಶಿ_obj = ರಾಶಿ;
	thread->syscall_ಚೌಕಟ್ಟು = ಖಾಲಿ;

	/* Any given thread has access to itself */
	k_object_access_grant(thread, thread);
#ಇಲ್ಲದಿದ್ದರೆ
	ARG_ಬಳಸದ(thread);
	ARG_ಬಳಸದ(ರಾಶಿ);
	ARG_ಬಳಸದ(options);
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_USERSPACE */
}

/* Assert the thread object is cache-coherent while its stack stays cached. */
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ assert_thread_coherence(ರಚನೆ k_thread *thread,
					   k_thread_ರಾಶಿ_t *ರಾಶಿ)
{
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_KERNEL_COHERENCE
	/* Check that the thread object is safe, but that the stack is
	 * still cached!
	 */
	__ASSERT_NO_MSG(sys_cache_is_mem_coherent(thread));

	/* When dynamic thread stack is available, the stack may come from
	 * uncached area.
	 */
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ CONFIG_DYNAMIC_THREAD
	__ASSERT_NO_MSG(!sys_cache_is_mem_coherent(ರಾಶಿ));
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_DYNAMIC_THREAD */
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_KERNEL_COHERENCE */
	ARG_ಬಳಸದ(thread);
	ARG_ಬಳಸದ(ರಾಶಿ);
}

/* Assert the arch layer populated switch_handle on USE_SWITCH builds. */
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ assert_switch_handle(ರಚನೆ k_thread *thread)
{
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_USE_SWITCH
	/* switch_handle must be non-null except when inside z_swap()
	 * for synchronization reasons.  Historically some notional
	 * USE_SWITCH architectures have actually ignored the field
	 */
	__ASSERT(thread->switch_handle != ಖಾಲಿ,
		 "arch layer failed to initialize switch_handle");
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_USE_SWITCH */
	ARG_ಬಳಸದ(thread);
}

/* Clear the opaque, kernel-managed per-thread custom data pointer. */
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ init_thread_custom_ದತ್ತಾಂಶ(ರಚನೆ k_thread *thread)
{
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_THREAD_CUSTOM_ದತ್ತಾಂಶ
	/* Initialize custom data field (value is opaque to kernel) */
	thread->custom_ದತ್ತಾಂಶ = ಖಾಲಿ;
#ಇಲ್ಲದಿದ್ದರೆ
	ARG_ಬಳಸದ(thread);
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_THREAD_CUSTOM_DATA */
}

/*
 * Record the entry point and parameters, then link the thread into the
 * global thread-monitor list under the monitor spinlock.
 */
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ ಕೂಡಿಸು_thread_ಗಮ್ಯ_monitor(ರಚನೆ k_thread *thread,
					 k_thread_ನಮೂದು_t ನಮೂದು,
					 ಶೂನ್ಯ *p1, ಶೂನ್ಯ *p2, ಶೂನ್ಯ *p3)
{
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_THREAD_MONITOR
	thread->ನಮೂದು.p_ನಮೂದು = ನಮೂದು;
	thread->ನಮೂದು.parameter1 = p1;
	thread->ನಮೂದು.parameter2 = p2;
	thread->ನಮೂದು.parameter3 = p3;

	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_ಬೀಗ(&z_thread_monitor_ಬೀಗ);

	thread->ಮುಂದಿನ_thread = _kernel.ಎಳೆಗಳು;
	_kernel.ಎಳೆಗಳು = thread;
	k_spin_unlock(&z_thread_monitor_ಬೀಗ, ಕೀಲಿ);
#ಇಲ್ಲದಿದ್ದರೆ
	ARG_ಬಳಸದ(thread);
	ARG_ಬಳಸದ(ನಮೂದು);
	ARG_ಬಳಸದ(p1);
	ARG_ಬಳಸದ(p2);
	ARG_ಬಳಸದ(p3);
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_THREAD_MONITOR */
}

/* Set the thread's name from the caller-supplied string (may be NULL). */
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ init_thread_name(ರಚನೆ k_thread *thread, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name)
{
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_THREAD_NAME
	ಗಣ_thread_name(thread, name);
#ಇಲ್ಲದಿದ್ದರೆ
	ARG_ಬಳಸದ(thread);
	ARG_ಬಳಸದ(name);
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_THREAD_NAME */
}

/* Set the thread's initial SMP CPU affinity mask. */
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ init_thread_cpu_ಮುಖವಾಡ(ರಚನೆ k_thread *thread)
{
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SCHED_CPU_ಮುಖವಾಡ
	ಆದರೆ (IS_ENABLED(CONFIG_SCHED_CPU_ಮುಖವಾಡ_PIN_ONLY)) {
		thread->ತಳ.cpu_ಮುಖವಾಡ = 1; /* must specify only one cpu */
	} ಇಲ್ಲದಿದ್ದರೆ {
		thread->ತಳ.cpu_ಮುಖವಾಡ = -1; /* allow all cpus */
	}
#ಇಲ್ಲದಿದ್ದರೆ
	ARG_ಬಳಸದ(thread);
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_SCHED_CPU_MASK */
}

/* Attach the thread to its memory domain and inherit caller permissions
 * when K_INHERIT_PERMS is requested.
 */
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ init_thread_userspace_perms(ರಚನೆ k_thread *thread,
					       uint32_t options)
{
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_USERSPACE
	z_mem_domain_init_thread(thread);

	ಆದರೆ ((options & K_INHERIT_PERMS) != 0U) {
		k_thread_perms_inherit(_current, thread);
	}
#ಇಲ್ಲದಿದ್ದರೆ
	ARG_ಬಳಸದ(thread);
	ARG_ಬಳಸದ(options);
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_USERSPACE */
}

/* Clear the EDF scheduling deadline. */
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ init_thread_deadline(ರಚನೆ k_thread *thread)
{
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SCHED_DEADLINE
	thread->ತಳ.prio_deadline = 0;
#ಇಲ್ಲದಿದ್ದರೆ
	ARG_ಬಳಸದ(thread);
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_SCHED_DEADLINE */
}

/* Initialize the wait queue used to synchronize SMP thread halt/abort. */
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ init_thread_halt_ಸರತಿ(ರಚನೆ k_thread *thread)
{
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SMP
	z_waitq_init(&thread->halt_ಸರತಿ);
#ಇಲ್ಲದಿದ್ದರೆ
	ARG_ಬಳಸದ(thread);
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_SMP */
}

/* Initialize runtime CPU-usage tracking for the thread. */
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ init_thread_usage(ರಚನೆ k_thread *thread)
{
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SCHED_THREAD_USAGE
	thread->ತಳ.usage = (ರಚನೆ k_ಚಕ್ರ_stats) {};
	thread->ತಳ.usage.track_usage = CONFIG_SCHED_THREAD_USAGE_AUTO_ENABLE;
#ಇಲ್ಲದಿದ್ದರೆ
	ARG_ಬಳಸದ(thread);
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_SCHED_THREAD_USAGE */
}

/* Assert the thread wasn't already linked into the monitor list. */
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ assert_thread_not_reused(ರಚನೆ k_thread *new_thread)
{
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_THREAD_MONITOR
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_ASSERT
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_ಬೀಗ(&z_thread_monitor_ಬೀಗ);
	bool reused = false;

	/* Check if the thread is already in the list */
	ಗಾಗಿ (ರಚನೆ k_thread *t = _kernel.ಎಳೆಗಳು; t; t = t->ಮುಂದಿನ_thread) {
		ಆದರೆ (t == new_thread) {
			reused = true;
			ನಿಲ್ಲಿಸು;
		}
	}

	k_spin_unlock(&z_thread_monitor_ಬೀಗ, ಕೀಲಿ);

	__ASSERT(!reused, "thread %p is already in the running list", new_thread);
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_ASSERT */
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_THREAD_MONITOR */
	ARG_ಬಳಸದ(new_thread);
}

/*
 * The provided stack_size value is presumed to be either the result of
 * K_THREAD_STACK_SIZEOF(stack), or the size value passed to the instance
 * of K_THREAD_STACK_DEFINE() which defined 'stack'.
 */
ಅಕ್ಷರ *z_ಸಿದ್ಧತೆ_new_thread(ರಚನೆ k_thread *new_thread,
			 k_thread_ರಾಶಿ_t *ರಾಶಿ, size_t ರಾಶಿ_ಗಾತ್ರ,
			 k_thread_ನಮೂದು_t ನಮೂದು,
			 ಶೂನ್ಯ *p1, ಶೂನ್ಯ *p2, ಶೂನ್ಯ *p3,
			 ಪೂರ್ಣಾಂಕ prio, uint32_t options, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name)
{
	ಅಕ್ಷರ *ರಾಶಿ_ptr;

	Z_ASSERT_VALID_PRIO(prio, ನಮೂದು);

	assert_thread_not_reused(new_thread);

	thread_abort_cleanup_check_reuse(new_thread);
	init_thread_obj_core(new_thread);
	init_thread_userspace(new_thread, ರಾಶಿ, options);

	z_waitq_init(&new_thread->join_ಸರತಿ);

	/* Initialize various struct k_thread members */
	z_init_thread_ತಳ(&new_thread->ತಳ, prio, _THREAD_SLEEPING, options);
	ರಾಶಿ_ptr = ಸಿದ್ಧತೆ_thread_ರಾಶಿ(new_thread, ರಾಶಿ, ರಾಶಿ_ಗಾತ್ರ);

#ಆದರೆ Z_MUTEX_PI_ENABLED
	sys_slist_init(&new_thread->held_mutexes);
	new_thread->mutex_pended_on = ಖಾಲಿ;
	new_thread->orig_prio = new_thread->ತಳ.prio;
#ಆದರೆ_ಅಂತ್ಯ /* Z_MUTEX_PI_ENABLED */

	ಸಿದ್ಧತೆ_shadow_ರಾಶಿ(new_thread, ರಾಶಿ);
	assert_thread_coherence(new_thread, ರಾಶಿ);

	arch_new_thread(new_thread, ರಾಶಿ, ರಾಶಿ_ptr, ನಮೂದು, p1, p2, p3);

	/* static threads overwrite it afterwards with real value */
	new_thread->init_ದತ್ತಾಂಶ = ಖಾಲಿ;

	assert_switch_handle(new_thread);
	init_thread_custom_ದತ್ತಾಂಶ(new_thread);
	ಕೂಡಿಸು_thread_ಗಮ್ಯ_monitor(new_thread, ನಮೂದು, p1, p2, p3);
	init_thread_name(new_thread, name);
	init_thread_cpu_ಮುಖವಾಡ(new_thread);

	/* _current may be NULL if the dummy thread is not used */
	ಆದರೆ (IS_ENABLED(CONFIG_ARCH_HAS_CUSTOM_ಅದಲುಬದಲು_ಗಮ್ಯ_MAIN) &&
	    (_current == ಖಾಲಿ)) {
		new_thread->resource_pool = ಖಾಲಿ;
		ಹಿಂತಿರುಗಿ ರಾಶಿ_ptr;
	}

	init_thread_userspace_perms(new_thread, options);
	init_thread_deadline(new_thread);
	new_thread->resource_pool = _current->resource_pool;
	init_thread_halt_ಸರತಿ(new_thread);
	init_thread_usage(new_thread);

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC(k_thread, create, new_thread);

	ಹಿಂತಿರುಗಿ ರಾಶಿ_ptr;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_THREAD_RUNTIME_ರಾಶಿ_SAFETY
ಪೂರ್ಣಾಂಕ z_impl_k_thread_runtime_ರಾಶಿ_ಬಳಸದ_ಮಿತಿ_pct_ಗಣ(ರಚನೆ k_thread *thread,
							   uint32_t pct)
{
	size_t ಬಳಸದ_ಮಿತಿ;

	ಆದರೆ (pct > 99) {
		ಹಿಂತಿರುಗಿ -EINVAL;   /* 100% unused stack and up is invalid */
	}

	ಬಳಸದ_ಮಿತಿ = (thread->ರಾಶಿ_info.ಗಾತ್ರ_2 * pct) / 100;

	thread->ರಾಶಿ_info.usage.ಬಳಸದ_ಮಿತಿ = ಬಳಸದ_ಮಿತಿ;

	ಹಿಂತಿರುಗಿ 0;
}

ಪೂರ್ಣಾಂಕ z_impl_k_thread_runtime_ರಾಶಿ_ಬಳಸದ_ಮಿತಿ_ಗಣ(ರಚನೆ k_thread *thread,
						       size_t ಮಿತಿ)
{
	ಆದರೆ (ಮಿತಿ > thread->ರಾಶಿ_info.ಗಾತ್ರ_2) {
		ಹಿಂತಿರುಗಿ -EINVAL;
	}

	thread->ರಾಶಿ_info.usage.ಬಳಸದ_ಮಿತಿ = ಮಿತಿ;

	ಹಿಂತಿರುಗಿ 0;
}

size_t z_impl_k_thread_runtime_ರಾಶಿ_ಬಳಸದ_ಮಿತಿ_get(ರಚನೆ k_thread *thread)
{
	ಹಿಂತಿರುಗಿ thread->ರಾಶಿ_info.usage.ಬಳಸದ_ಮಿತಿ;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_USERSPACE
ಪೂರ್ಣಾಂಕ z_vrfy_k_thread_runtime_ರಾಶಿ_ಬಳಸದ_ಮಿತಿ_pct_ಗಣ(ರಚನೆ k_thread *thread,
							   uint32_t pct)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));

	ಹಿಂತಿರುಗಿ z_impl_k_thread_runtime_ರಾಶಿ_ಬಳಸದ_ಮಿತಿ_pct_ಗಣ(thread, pct);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_thread_runtime_stack_unused_threshold_pct_set_mrsh.c>

ಪೂರ್ಣಾಂಕ z_vrfy_k_thread_runtime_ರಾಶಿ_ಬಳಸದ_ಮಿತಿ_ಗಣ(ರಚನೆ k_thread *thread,
						       size_t ಮಿತಿ)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));

	ಹಿಂತಿರುಗಿ z_impl_k_thread_runtime_ರಾಶಿ_ಬಳಸದ_ಮಿತಿ_ಗಣ(thread, ಮಿತಿ);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_thread_runtime_stack_unused_threshold_set_mrsh.c>

size_t z_vrfy_k_thread_runtime_ರಾಶಿ_ಬಳಸದ_ಮಿತಿ_get(ರಚನೆ k_thread *thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));

	ಹಿಂತಿರುಗಿ z_impl_k_thread_runtime_ರಾಶಿ_ಬಳಸದ_ಮಿತಿ_get(thread);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_thread_runtime_stack_unused_threshold_get_mrsh.c>
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_USERSPACE */
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_THREAD_RUNTIME_STACK_SAFETY */

k_tid_t z_impl_k_thread_create(ರಚನೆ k_thread *new_thread,
			      k_thread_ರಾಶಿ_t *ರಾಶಿ,
			      size_t ರಾಶಿ_ಗಾತ್ರ, k_thread_ನಮೂದು_t ನಮೂದು,
			      ಶೂನ್ಯ *p1, ಶೂನ್ಯ *p2, ಶೂನ್ಯ *p3,
			      ಪೂರ್ಣಾಂಕ prio, uint32_t options, k_timeout_t delay)
{
	__ASSERT(!arch_is_ಒಳ_isr(), "Threads may not be created in ISRs");

	z_ಸಿದ್ಧತೆ_new_thread(new_thread, ರಾಶಿ, ರಾಶಿ_ಗಾತ್ರ, ನಮೂದು, p1, p2, p3,
			  prio, options, ಖಾಲಿ);

	thread_ವೇಳಾಪಟ್ಟಿ_new(new_thread, delay);

	ಹಿಂತಿರುಗಿ new_thread;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_USERSPACE
bool z_ರಾಶಿ_is_user_capable(k_thread_ರಾಶಿ_t *ರಾಶಿ)
{
	ಹಿಂತಿರುಗಿ k_object_ಕಂಡುಹಿಡಿ(ರಾಶಿ) != ಖಾಲಿ;
}

k_tid_t z_vrfy_k_thread_create(ರಚನೆ k_thread *new_thread,
			       k_thread_ರಾಶಿ_t *ರಾಶಿ,
			       size_t ರಾಶಿ_ಗಾತ್ರ, k_thread_ನಮೂದು_t ನಮೂದು,
			       ಶೂನ್ಯ *p1, ಶೂನ್ಯ *p2, ಶೂನ್ಯ *p3,
			       ಪೂರ್ಣಾಂಕ prio, uint32_t options, k_timeout_t delay)
{
	size_t ಒಟ್ಟು_ಗಾತ್ರ, ರಾಶಿ_obj_ಗಾತ್ರ;
	ರಚನೆ k_object *ರಾಶಿ_object;

	/* The thread and stack objects *must* be in an uninitialized state */
	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(new_thread, K_OBJ_THREAD));

	/* No need to check z_stack_is_user_capable(), it won't be in the
	 * object table if it isn't
	 */
	ರಾಶಿ_object = k_object_ಕಂಡುಹಿಡಿ(ರಾಶಿ);
	K_OOPS(K_SYSCALL_VERIFY_ಸಂದೇಶ(k_object_validation_check(ರಾಶಿ_object, ರಾಶಿ,
						K_OBJ_THREAD_ರಾಶಿ_ELEMENT,
						_OBJ_INIT_FALSE) == 0,
				    "bad stack object"));

	/* Verify that the stack size passed in is OK by computing the total
	 * size and comparing it with the size value in the object metadata
	 */
	K_OOPS(K_SYSCALL_VERIFY_ಸಂದೇಶ(!ಗಾತ್ರ_ಕೂಡಿಸು_ಅತಿಪ್ರವಾಹ(K_THREAD_ರಾಶಿ_RESERVED,
						       ರಾಶಿ_ಗಾತ್ರ, &ಒಟ್ಟು_ಗಾತ್ರ),
				    "stack size overflow (%zu+%zu)",
				    ರಾಶಿ_ಗಾತ್ರ,
				    K_THREAD_ರಾಶಿ_RESERVED));

	/* Testing less-than-or-equal since additional room may have been
	 * allocated for alignment constraints
	 */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_GEN_PRIV_STACKS
	ರಾಶಿ_obj_ಗಾತ್ರ = ರಾಶಿ_object->ದತ್ತಾಂಶ.ರಾಶಿ_ದತ್ತಾಂಶ->ಗಾತ್ರ_2;
#ಇಲ್ಲದಿದ್ದರೆ
	ರಾಶಿ_obj_ಗಾತ್ರ = ರಾಶಿ_object->ದತ್ತಾಂಶ.ರಾಶಿ_ಗಾತ್ರ;
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_GEN_PRIV_STACKS */
	K_OOPS(K_SYSCALL_VERIFY_ಸಂದೇಶ(ಒಟ್ಟು_ಗಾತ್ರ <= ರಾಶಿ_obj_ಗಾತ್ರ,
				    "stack size %zu is too big, max is %zu",
				    ಒಟ್ಟು_ಗಾತ್ರ, ರಾಶಿ_obj_ಗಾತ್ರ));

	/* User threads may only create other user threads and they can't
	 * be marked as essential
	 */
	K_OOPS(K_SYSCALL_VERIFY(options & K_USER));
	K_OOPS(K_SYSCALL_VERIFY(!(options & K_ESSENTIAL)));

	/* Check validity of prio argument; must be the same or worse priority
	 * than the caller
	 */
	K_OOPS(K_SYSCALL_VERIFY(_is_valid_prio(prio, ಖಾಲಿ)));
	K_OOPS(K_SYSCALL_VERIFY(z_is_prio_ಅಧೋ_or_equal(prio,
							_current->ತಳ.prio)));

	z_ಸಿದ್ಧತೆ_new_thread(new_thread, ರಾಶಿ, ರಾಶಿ_ಗಾತ್ರ,
			   ನಮೂದು, p1, p2, p3, prio, options, ಖಾಲಿ);

	thread_ವೇಳಾಪಟ್ಟಿ_new(new_thread, delay);

	ಹಿಂತಿರುಗಿ new_thread;
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_thread_create_mrsh.c>
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_USERSPACE */

ಶೂನ್ಯ z_init_thread_ತಳ(ರಚನೆ thread_ತಳ *thread_ತಳ_2, ಪೂರ್ಣಾಂಕ ಆದ್ಯತೆ,
		       uint32_t initial_state, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ options)
{
	/* k_q_node is initialized upon first insertion in a list */
	thread_ತಳ_2->pended_on = ಖಾಲಿ;
	thread_ತಳ_2->user_options = (uint16_t)options;
	thread_ತಳ_2->thread_state = (uint8_t)initial_state;

	thread_ತಳ_2->prio = ಆದ್ಯತೆ;

	thread_ತಳ_2->sched_locked = 0U;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SMP
	thread_ತಳ_2->is_idle = 0;

	/*
	 * Pretend that the thread was last executing on CPU0 to prevent
	 * out-of-bounds memory accesses to the _kernel.cpus[] array.
	 */
	thread_ತಳ_2->cpu = 0;
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_SMP */

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_TIMESLICE_ಪ್ರತಿ_THREAD
	thread_ತಳ_2->ತುಂಡು_ticks = 0;
	thread_ತಳ_2->ತುಂಡು_expired = ಖಾಲಿ;
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_TIMESLICE_PER_THREAD */

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SPIN_VALIDATE
	thread_ತಳ_2->ಅದಲುಬದಲು_ದತ್ತಾಂಶ = ಖಾಲಿ;
	/* otherwise swap_data does not need to be initialized */
#ಆದರೆ_ಅಂತ್ಯ

	z_init_thread_timeout(thread_ತಳ_2);
}

FUNC_NORETURN ಶೂನ್ಯ k_thread_user_ಬಹುಲಕ_enter(k_thread_ನಮೂದು_t ನಮೂದು,
					    ಶೂನ್ಯ *p1, ಶೂನ್ಯ *p2, ಶೂನ್ಯ *p3)
{
	SYS_ಪೋರ್ಟ್_TRACING_FUNC(k_thread, user_ಬಹುಲಕ_enter);

	_current->ತಳ.user_options |= K_USER;
	z_thread_essential_clear(_current);
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_THREAD_MONITOR
	_current->ನಮೂದು.p_ನಮೂದು = ನಮೂದು;
	_current->ನಮೂದು.parameter1 = p1;
	_current->ನಮೂದು.parameter2 = p2;
	_current->ನಮೂದು.parameter3 = p3;
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_THREAD_MONITOR */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_USERSPACE
	__ASSERT(z_ರಾಶಿ_is_user_capable(_current->ರಾಶಿ_obj),
		 "dropping to user mode with kernel-only stack object");
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_THREAD_USERSPACE_LOCAL_ದತ್ತಾಂಶ
	memset(_current->userspace_local_ದತ್ತಾಂಶ, 0,
	       ಗಾತ್ರ(ರಚನೆ thread_userspace_local_ದತ್ತಾಂಶ));
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_THREAD_USERSPACE_LOCAL_DATA */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_THREAD_LOCAL_STORAGE
	arch_tls_ರಾಶಿ_ಸಿದ್ಧತೆ(_current,
			     (ಅಕ್ಷರ *)(_current->ರಾಶಿ_info.ಆರಂಭ +
				      _current->ರಾಶಿ_info.ಗಾತ್ರ_2));
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_THREAD_LOCAL_STORAGE */
	arch_user_ಬಹುಲಕ_enter(ನಮೂದು, p1, p2, p3);
#ಇಲ್ಲದಿದ್ದರೆ
	/* XXX In this case we do not reset the stack */
	z_thread_ನಮೂದು(ನಮೂದು, p1, p2, p3);
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_USERSPACE */
}

#ಆದರೆ defined(CONFIG_INIT_STACKS) && defined(CONFIG_THREAD_ರಾಶಿ_INFO)
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_ರಾಶಿ_GROWS_ಮೇಲೆ
#ದೋಷ "Unsupported configuration for stack analysis"
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_STACK_GROWS_UP */

ಪೂರ್ಣಾಂಕ z_ರಾಶಿ_space_get(ಸ್ಥಿರಾಂಕ uint8_t *ರಾಶಿ_ಆರಂಭ, size_t ಗಾತ್ರ_2, size_t *ಬಳಸದ_ptr)
{
	size_t ಬಳಸದ = 0;
	ಸ್ಥಿರಾಂಕ uint8_t *checked_ರಾಶಿ = ರಾಶಿ_ಆರಂಭ;
	/* Take the address of any local variable as a shallow bound for the
	 * stack pointer.  Addresses above it are guaranteed to be
	 * accessible.
	 */
	ಸ್ಥಿರಾಂಕ uint8_t *ರಾಶಿ_ಸೂಚಕ = (ಸ್ಥಿರಾಂಕ uint8_t *)&ರಾಶಿ_ಆರಂಭ;

	/* If we are currently running on the stack being analyzed, some
	 * memory management hardware will generate an exception if we
	 * read unused stack memory.
	 *
	 * This never happens when invoked from user mode, as user mode
	 * will always run this function on the privilege elevation stack.
	 */
	ಆದರೆ ((ರಾಶಿ_ಸೂಚಕ > ರಾಶಿ_ಆರಂಭ) && (ರಾಶಿ_ಸೂಚಕ <= (ರಾಶಿ_ಆರಂಭ + ಗಾತ್ರ_2)) &&
	    IS_ENABLED(CONFIG_NO_ಬಳಸದ_ರಾಶಿ_INSPECTION)) {
		/* TODO: We could add an arch_ API call to temporarily
		 * disable the stack checking in the CPU, but this would
		 * need to be properly managed wrt context switches/interrupts
		 */
		ಹಿಂತಿರುಗಿ -ENOTSUP;
	}

	ಆದರೆ (IS_ENABLED(CONFIG_ರಾಶಿ_SENTINEL)) {
		/* First 4 bytes of the stack buffer reserved for the
		 * sentinel value, it won't be 0xAAAAAAAA for thread
		 * stacks.
		 *
		 * FIXME: thread->stack_info.start ought to reflect
		 * this!
		 */
		checked_ರಾಶಿ += 4;
		ಗಾತ್ರ_2 -= 4;
	}

	ಗಾಗಿ (size_t i = 0; i < ಗಾತ್ರ_2; i++) {
		ಆದರೆ ((checked_ರಾಶಿ[i]) == 0xaaU) {
			ಬಳಸದ++;
		} ಇಲ್ಲದಿದ್ದರೆ {
			ನಿಲ್ಲಿಸು;
		}
	}

	*ಬಳಸದ_ptr = ಬಳಸದ;

	ಹಿಂತಿರುಗಿ 0;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_THREAD_RUNTIME_ರಾಶಿ_SAFETY
ಪೂರ್ಣಾಂಕ k_thread_runtime_ರಾಶಿ_safety_ತುಂಬಿದ_check(ಸ್ಥಿರಾಂಕ ರಚನೆ k_thread *thread,
					     size_t *ಬಳಸದ_ptr,
					     k_thread_ರಾಶಿ_safety_handler_t handler,
					     ಶೂನ್ಯ *arg)
{
	ಪೂರ್ಣಾಂಕ    rv;
	size_t ಬಳಸದ_space;

	__ASSERT_NO_MSG(thread != ಖಾಲಿ);

	rv = z_ರಾಶಿ_space_get((ಸ್ಥಿರಾಂಕ uint8_t *)thread->ರಾಶಿ_info.ಆರಂಭ,
			       thread->ರಾಶಿ_info.ಗಾತ್ರ_2, &ಬಳಸದ_space);

	ಆದರೆ (rv != 0) {
		ಹಿಂತಿರುಗಿ rv;
	}

	ಆದರೆ (ಬಳಸದ_ptr != ಖಾಲಿ) {
		*ಬಳಸದ_ptr = ಬಳಸದ_space;
	}

	ಆದರೆ ((ಬಳಸದ_space < thread->ರಾಶಿ_info.usage.ಬಳಸದ_ಮಿತಿ) &&
	    (handler != ಖಾಲಿ)) {
		handler(thread, ಬಳಸದ_space, arg);
	}

	ಹಿಂತಿರುಗಿ 0;
}

ಪೂರ್ಣಾಂಕ k_thread_runtime_ರಾಶಿ_safety_ಮಿತಿ_check(ಸ್ಥಿರಾಂಕ ರಚನೆ k_thread *thread,
						  size_t *ಬಳಸದ_ptr,
						  k_thread_ರಾಶಿ_safety_handler_t handler,
						  ಶೂನ್ಯ *arg)
{
	ಪೂರ್ಣಾಂಕ    rv;
	size_t ಬಳಸದ_space;
	size_t ಮಿತಿ;
	size_t ಪರಿಶೀಲನೆ_ಗಾತ್ರ;

	__ASSERT_NO_MSG(thread != ಖಾಲಿ);

	ಮಿತಿ = thread->ರಾಶಿ_info.usage.ಬಳಸದ_ಮಿತಿ;
	ಪರಿಶೀಲನೆ_ಗಾತ್ರ = ಮಿತಿ;

	/* z_stack_space_get() reserves the first bytes of the stack buffer for
	 * the stack sentinel and shrinks its scan window by that amount. Grow
	 * the requested window by the same reservation so that up to
	 * 'threshold' bytes of the usable stack are actually examined;
	 * otherwise a fully unused window would report fewer than 'threshold'
	 * unused bytes and spuriously trip the threshold.
	 */
	ಆದರೆ (IS_ENABLED(CONFIG_ರಾಶಿ_SENTINEL) && (ಮಿತಿ != 0U)) {
		ಪರಿಶೀಲನೆ_ಗಾತ್ರ += ಗಾತ್ರ(uint32_t);
	}

	rv = z_ರಾಶಿ_space_get((ಸ್ಥಿರಾಂಕ uint8_t *)thread->ರಾಶಿ_info.ಆರಂಭ,
			       ಪರಿಶೀಲನೆ_ಗಾತ್ರ, &ಬಳಸದ_space);

	ಆದರೆ (rv != 0) {
		ಹಿಂತಿರುಗಿ rv;
	}

	ಆದರೆ (ಬಳಸದ_ptr != ಖಾಲಿ) {
		*ಬಳಸದ_ptr = ಬಳಸದ_space;
	}

	ಆದರೆ ((ಬಳಸದ_space < ಮಿತಿ) && (handler != ಖಾಲಿ)) {
		handler(thread, ಬಳಸದ_space, arg);
	}

	ಹಿಂತಿರುಗಿ 0;
}
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_THREAD_RUNTIME_STACK_SAFETY */

ಪೂರ್ಣಾಂಕ z_impl_k_thread_ರಾಶಿ_space_get(ಸ್ಥಿರಾಂಕ ರಚನೆ k_thread *thread,
				    size_t *ಬಳಸದ_ptr)
{
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_THREAD_ರಾಶಿ_MEM_MAPPED
	ಆದರೆ (thread->ರಾಶಿ_info.mapped.addr == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ -EINVAL;
	}
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_THREAD_STACK_MEM_MAPPED */

	ಹಿಂತಿರುಗಿ z_ರಾಶಿ_space_get((ಸ್ಥಿರಾಂಕ uint8_t *)thread->ರಾಶಿ_info.ಆರಂಭ,
				 thread->ರಾಶಿ_info.ಗಾತ್ರ_2, ಬಳಸದ_ptr);
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_USERSPACE
ಪೂರ್ಣಾಂಕ z_vrfy_k_thread_ರಾಶಿ_space_get(ಸ್ಥಿರಾಂಕ ರಚನೆ k_thread *thread,
				    size_t *ಬಳಸದ_ptr)
{
	size_t ಬಳಸದ;
	ಪೂರ್ಣಾಂಕ ret;

	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));

	ret = z_impl_k_thread_ರಾಶಿ_space_get(thread, &ಬಳಸದ);
	CHECKIF(ret != 0) {
		ಹಿಂತಿರುಗಿ ret;
	}

	ret = k_usermode_ಗಮ್ಯ_copy(ಬಳಸದ_ptr, &ಬಳಸದ, ಗಾತ್ರ(size_t));
	CHECKIF(ret != 0) {
		ಹಿಂತಿರುಗಿ ret;
	}

	ಹಿಂತಿರುಗಿ 0;
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_thread_stack_space_get_mrsh.c>
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_USERSPACE */
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_INIT_STACKS && CONFIG_THREAD_STACK_INFO */

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_USERSPACE
ಸ್ಥಿರ ಸಾಲೊಳಗೆ k_ticks_t z_vrfy_k_thread_timeout_ಉಳಿದ_ticks(
						    ಸ್ಥಿರಾಂಕ ರಚನೆ k_thread *thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	ಹಿಂತಿರುಗಿ z_impl_k_thread_timeout_ಉಳಿದ_ticks(thread);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_thread_timeout_remaining_ticks_mrsh.c>

ಸ್ಥಿರ ಸಾಲೊಳಗೆ k_ticks_t z_vrfy_k_thread_timeout_expires_ticks(
						  ಸ್ಥಿರಾಂಕ ರಚನೆ k_thread *thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	ಹಿಂತಿರುಗಿ z_impl_k_thread_timeout_expires_ticks(thread);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_thread_timeout_expires_ticks_mrsh.c>
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_USERSPACE */

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_INSTRUMENT_THREAD_SWITCHING
ಶೂನ್ಯ z_thread_mark_switched_ಒಳ(ಶೂನ್ಯ)
{
#ಆದರೆ defined(CONFIG_SCHED_THREAD_USAGE) && !defined(CONFIG_USE_SWITCH)
	z_sched_usage_ಆರಂಭ(_current);
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_SCHED_THREAD_USAGE && !CONFIG_USE_SWITCH */

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_TRACING
	SYS_ಪೋರ್ಟ್_TRACING_FUNC(k_thread, switched_ಒಳ);
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_TRACING */
}

ಶೂನ್ಯ z_thread_mark_switched_ಹೊರಹೋಗು(ಶೂನ್ಯ)
{
#ಆದರೆ defined(CONFIG_SCHED_THREAD_USAGE) && !defined(CONFIG_USE_SWITCH)
	z_sched_usage_stop();
#ಆದರೆ_ಅಂತ್ಯ /*CONFIG_SCHED_THREAD_USAGE && !CONFIG_USE_SWITCH */

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_TRACING
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_THREAD_LOCAL_STORAGE
	/* Dummy thread won't have TLS set up to run arbitrary code */
	ಆದರೆ (!_current ||
	    (_current->ತಳ.thread_state & _THREAD_DUMMY) != 0) {
		ಹಿಂತಿರುಗಿ;
	}
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_THREAD_LOCAL_STORAGE */
	SYS_ಪೋರ್ಟ್_TRACING_FUNC(k_thread, switched_ಹೊರಹೋಗು);
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_TRACING */
}
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_INSTRUMENT_THREAD_SWITCHING */

ಪೂರ್ಣಾಂಕ k_thread_runtime_stats_get(k_tid_t thread,
			       k_thread_runtime_stats_t *stats)
{
	ಆದರೆ ((thread == ಖಾಲಿ) || (stats == ಖಾಲಿ)) {
		ಹಿಂತಿರುಗಿ -EINVAL;
	}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SCHED_THREAD_USAGE
	z_sched_thread_usage(thread, stats);
#ಇಲ್ಲದಿದ್ದರೆ
	*stats = (k_thread_runtime_stats_t) {};
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_SCHED_THREAD_USAGE */

	ಹಿಂತಿರುಗಿ 0;
}

ಪೂರ್ಣಾಂಕ k_thread_runtime_stats_all_get(k_thread_runtime_stats_t *stats)
{
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SCHED_THREAD_USAGE_ALL
	k_thread_runtime_stats_t  tmp_stats;
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_SCHED_THREAD_USAGE_ALL */

	ಆದರೆ (stats == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ -EINVAL;
	}

	*stats = (k_thread_runtime_stats_t) {};

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SCHED_THREAD_USAGE_ALL
	/* Retrieve the usage stats for each core and amalgamate them. */

	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಅಂಕೆ_cpus = arch_ಅಂಕೆ_cpus();

	ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ i = 0; i < ಅಂಕೆ_cpus; i++) {
		z_sched_cpu_usage(i, &tmp_stats);

		stats->execution_ಚಕ್ರಗಳು += tmp_stats.execution_ಚಕ್ರಗಳು;
		stats->ಒಟ್ಟು_ಚಕ್ರಗಳು     += tmp_stats.ಒಟ್ಟು_ಚಕ್ರಗಳು;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SCHED_THREAD_USAGE_ANALYSIS
		stats->current_ಚಕ್ರಗಳು   += tmp_stats.current_ಚಕ್ರಗಳು;
		stats->peak_ಚಕ್ರಗಳು      += tmp_stats.peak_ಚಕ್ರಗಳು;
		stats->ಸರಾಸರಿ_ಮೌಲ್ಯ_ಚಕ್ರಗಳು   += tmp_stats.ಸರಾಸರಿ_ಮೌಲ್ಯ_ಚಕ್ರಗಳು;
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */
		stats->idle_ಚಕ್ರಗಳು      += tmp_stats.idle_ಚಕ್ರಗಳು;
	}
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_SCHED_THREAD_USAGE_ALL */

	ಹಿಂತಿರುಗಿ 0;
}

ಪೂರ್ಣಾಂಕ k_thread_runtime_stats_cpu_get(ಪೂರ್ಣಾಂಕ cpu, k_thread_runtime_stats_t *stats)
{
	ಆದರೆ (stats == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ -EINVAL;
	}

	*stats = (k_thread_runtime_stats_t) {};

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SCHED_THREAD_USAGE_ALL
	CHECKIF((cpu < 0) || ((ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ)cpu >= arch_ಅಂಕೆ_cpus())) {
		ಹಿಂತಿರುಗಿ -EINVAL;
	}

	z_sched_cpu_usage(cpu, stats);
#ಇಲ್ಲದಿದ್ದರೆ
	ARG_ಬಳಸದ(cpu);
#ಆದರೆ_ಅಂತ್ಯ

	ಹಿಂತಿರುಗಿ 0;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_THREAD_ABORT_NEED_CLEANUP
/** Pointer to thread which needs to be cleaned up. */
ಸ್ಥಿರ ರಚನೆ k_thread *thread_ಗಮ್ಯ_cleanup;

/** Spinlock for thread abort cleanup. */
ಸ್ಥಿರ ರಚನೆ k_spinlock thread_cleanup_ಬೀಗ;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_THREAD_ರಾಶಿ_MEM_MAPPED
ಸ್ಥಿರ ಶೂನ್ಯ *thread_cleanup_ರಾಶಿ_addr;
ಸ್ಥಿರ size_t thread_cleanup_ರಾಶಿ_sz;

ಸ್ಥಿರ ಶೂನ್ಯ unmap_thread_ರಾಶಿ(ಶೂನ್ಯ *addr, size_t sz)
{
	ಆದರೆ (addr != ಖಾಲಿ) {
		k_mem_unmap_phys_guard(addr, sz, false);
	}
}
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_THREAD_STACK_MEM_MAPPED */

ಶೂನ್ಯ defer_thread_cleanup(ರಚನೆ k_thread *thread)
{
	/* Note when adding new deferred cleanup steps:
	 * - The thread object may have been overwritten by the time
	 *   the actual cleanup is being done (e.g. thread object
	 *   allocated on a stack). So stash any necessary data here
	 *   that will be used in the actual cleanup steps.
	 */
	thread_ಗಮ್ಯ_cleanup = thread;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_THREAD_ರಾಶಿ_MEM_MAPPED
	/* Note that the permission of the stack should have been
	 * stripped of user thread access due to the thread having
	 * already exited from a memory domain. That is done via
	 * k_thread_abort().
	 */

	/* Stash the address and size so the region can be unmapped
	 * later.
	 */
	thread_cleanup_ರಾಶಿ_addr = thread->ರಾಶಿ_info.mapped.addr;
	thread_cleanup_ರಾಶಿ_sz = thread->ರಾಶಿ_info.mapped.sz;

	/* The stack is now considered un-usable. This should prevent any functions
	 * from looking directly into the mapped stack if they are made to be aware
	 * of memory mapped stacks, e.g., z_stack_space_get().
	 */
	thread->ರಾಶಿ_info.mapped.addr = ಖಾಲಿ;
	thread->ರಾಶಿ_info.mapped.sz = 0;
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_THREAD_STACK_MEM_MAPPED */
}

ಶೂನ್ಯ do_deferred_thread_cleanup(ಶೂನ್ಯ)
{
	/* Note when adding new actual cleanup steps:
	 * - The thread object may have been overwritten when this is
	 *   called. So only data stashed by defer_thread_cleanup() can
	 *   be used here, never anything from the thread object.
	 */

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_THREAD_ರಾಶಿ_MEM_MAPPED
	unmap_thread_ರಾಶಿ(thread_cleanup_ರಾಶಿ_addr, thread_cleanup_ರಾಶಿ_sz);

	thread_cleanup_ರಾಶಿ_addr = ಖಾಲಿ;
	thread_cleanup_ರಾಶಿ_sz = 0;
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_THREAD_STACK_MEM_MAPPED */
}

ಶೂನ್ಯ do_thread_cleanup(ರಚನೆ k_thread *thread)
{
	/* This is only for threads which are no longer running and are not
	 * the current thread, so the thread object is still valid here and
	 * the cleanup can be driven from it directly.
	 */

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_THREAD_ರಾಶಿ_MEM_MAPPED
	unmap_thread_ರಾಶಿ(thread->ರಾಶಿ_info.mapped.addr,
			   thread->ರಾಶಿ_info.mapped.sz);

	/* The stack is now unmapped and thus un-usable. Clear the mapping
	 * info so nothing looks into the stack afterwards, and so that
	 * a repeated cleanup of the same thread object is a no-op,
	 * e.g., z_stack_space_get().
	 */
	thread->ರಾಶಿ_info.mapped.addr = ಖಾಲಿ;
	thread->ರಾಶಿ_info.mapped.sz = 0;
#ಇಲ್ಲದಿದ್ದರೆ /* CONFIG_THREAD_STACK_MEM_MAPPED */
	ARG_ಬಳಸದ(thread);
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_THREAD_STACK_MEM_MAPPED */
}

ಶೂನ್ಯ k_thread_abort_cleanup(ರಚನೆ k_thread *thread)
{
	K_SPINLOCK(&thread_cleanup_ಬೀಗ) {
		ಆದರೆ (thread_ಗಮ್ಯ_cleanup != ಖಾಲಿ) {
			/* Finish the pending one first. */
			do_deferred_thread_cleanup();
			thread_ಗಮ್ಯ_cleanup = ಖಾಲಿ;
		}

		ಆದರೆ (thread == _current) {
			/* Need to defer for current running thread as the cleanup
			 * might result in exception. Actual cleanup will be done
			 * at the next time k_thread_abort() is called, or at thread
			 * creation if the same thread object is being reused. This
			 * is to make sure the cleanup code no longer needs this
			 * thread's stack. This is not exactly ideal as the stack
			 * may still be memory mapped for a while. However, this is
			 * a simple solution without a) the need to workaround
			 * the schedule lock during k_thread_abort(), b) creating
			 * another thread to perform the cleanup, and c) does not
			 * require architecture code support (e.g. via exception).
			 */
			defer_thread_cleanup(thread);
		} ಇಲ್ಲದಿದ್ದರೆ {
			/* Not the current running thread, so we are safe to do
			 * cleanups.
			 */
			do_thread_cleanup(thread);
		}
	}
}

ಶೂನ್ಯ k_thread_abort_cleanup_check_reuse(ರಚನೆ k_thread *thread)
{
	K_SPINLOCK(&thread_cleanup_ಬೀಗ) {
		/* This is to guard reuse of the same thread object and make sure
		 * any pending cleanups of it needs to be finished before the thread
		 * object can be reused.
		 */
		ಆದರೆ (thread_ಗಮ್ಯ_cleanup == thread) {
			do_deferred_thread_cleanup();
			thread_ಗಮ್ಯ_cleanup = ಖಾಲಿ;
		}
	}
}

#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_THREAD_ABORT_NEED_CLEANUP */

ಶೂನ್ಯ z_dummy_thread_init(ರಚನೆ k_thread *dummy_thread)
{
	dummy_thread->ತಳ.thread_state = _THREAD_DUMMY;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SCHED_CPU_ಮುಖವಾಡ
	dummy_thread->ತಳ.cpu_ಮುಖವಾಡ = -1;
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_SCHED_CPU_MASK */
	dummy_thread->ತಳ.user_options = K_ESSENTIAL;
#ಆದರೆ Z_MUTEX_PI_ENABLED
	sys_slist_init(&dummy_thread->held_mutexes);
	dummy_thread->mutex_pended_on = ಖಾಲಿ;
	dummy_thread->orig_prio = dummy_thread->ತಳ.prio;
#ಆದರೆ_ಅಂತ್ಯ /* Z_MUTEX_PI_ENABLED */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_THREAD_ರಾಶಿ_INFO
	dummy_thread->ರಾಶಿ_info.ಆರಂಭ = 0U;
	dummy_thread->ರಾಶಿ_info.ಗಾತ್ರ_2 = 0U;
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_THREAD_STACK_INFO */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_USERSPACE
	dummy_thread->mem_domain_info.mem_domain = &k_mem_domain_default;
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_USERSPACE */
#ಆದರೆ (K_ರಾಶಿ_MEM_POOL_ಗಾತ್ರ > 0)
	k_thread_system_pool_ನಿಯೋಜಿಸು(dummy_thread);
#ಇಲ್ಲದಿದ್ದರೆ
	dummy_thread->resource_pool = ಖಾಲಿ;
#ಆದರೆ_ಅಂತ್ಯ /* K_HEAP_MEM_POOL_SIZE */

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_USE_SWITCH
	dummy_thread->switch_handle = ಖಾಲಿ;
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_USE_SWITCH */

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_TIMESLICE_ಪ್ರತಿ_THREAD
	dummy_thread->ತಳ.ತುಂಡು_ticks = 0;
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_TIMESLICE_PER_THREAD */

	z_current_thread_ಗಣ(dummy_thread);
}

/*
 * Thread lifecycle APIs: suspend, resume, priority, wakeup, abort, join.
 *
 * These APIs operate on thread objects and belong here in thread.c.  They
 * depend on a small set of scheduler internals (z_thread_halt,
 * z_sched_ready_locked, z_sched_add_to_waitq_locked) exposed via ksched.h.
 */

ಶೂನ್ಯ z_impl_k_thread_ಆದ್ಯತೆ_ಗಣ(k_tid_t thread, ಪೂರ್ಣಾಂಕ prio)
{
	/*
	 * Use NULL, since we cannot know what the entry point is (we do not
	 * keep track of it) and idle cannot change its priority.
	 */
	Z_ASSERT_VALID_PRIO(prio, ಖಾಲಿ);

	bool need_sched = z_thread_prio_ಗಣ((ರಚನೆ k_thread *)thread, prio);

	ಆದರೆ ((need_sched) && (IS_ENABLED(CONFIG_SMP) ||
			     (_current->ತಳ.sched_locked == 0U))) {
		z_reschedule_unlocked();
	}
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_USERSPACE
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ z_vrfy_k_thread_ಆದ್ಯತೆ_ಗಣ(k_tid_t thread, ಪೂರ್ಣಾಂಕ prio)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	K_OOPS(K_SYSCALL_VERIFY_ಸಂದೇಶ(_is_valid_prio(prio, ಖಾಲಿ),
				    "invalid thread priority %d", prio));
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ CONFIG_USERSPACE_THREAD_MAY_RAISE_ಆದ್ಯತೆ
	K_OOPS(K_SYSCALL_VERIFY_ಸಂದೇಶ((int8_t)prio >= thread->ತಳ.prio,
				    "thread priority may only be downgraded (%d < %d)",
				    prio, thread->ತಳ.prio));
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_USERSPACE_THREAD_MAY_RAISE_PRIORITY */
	z_impl_k_thread_ಆದ್ಯತೆ_ಗಣ(thread, prio);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_thread_priority_set_mrsh.c>
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_USERSPACE */

ಶೂನ್ಯ z_impl_k_thread_suspend(k_tid_t thread)
{
	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_ENTER(k_thread, suspend, thread);

	/* Special case "suspend the current thread" as it doesn't
	 * need the async complexity below.
	 */
	ಆದರೆ (!IS_ENABLED(CONFIG_SMP) &&
	    likely((thread == _current) && !arch_is_ಒಳ_isr())) {
		z_thread_suspend_current(thread);
		ಹಿಂತಿರುಗಿ;
	}

	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = z_sched_spinlock_ಬೀಗ();

	ಆದರೆ (unlikely(z_is_thread_suspended(thread))) {

		/* The target thread is already suspended. Nothing to do. */

		z_sched_spinlock_unlock(ಕೀಲಿ);
		ಹಿಂತಿರುಗಿ;
	}

	z_thread_halt(thread, ಕೀಲಿ, false);

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_EXIT(k_thread, suspend, thread);
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_USERSPACE
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ z_vrfy_k_thread_suspend(k_tid_t thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	z_impl_k_thread_suspend(thread);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_thread_suspend_mrsh.c>
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_USERSPACE */

ಶೂನ್ಯ z_impl_k_thread_resume(k_tid_t thread)
{
	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_ENTER(k_thread, resume, thread);

	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = z_sched_spinlock_ಬೀಗ();

	/* Do not try to resume a thread that was not suspended */
	ಆದರೆ (unlikely(!z_is_thread_suspended(thread))) {
		z_sched_spinlock_unlock(ಕೀಲಿ);
		ಹಿಂತಿರುಗಿ;
	}

	z_mark_thread_as_not_suspended(thread);
	z_sched_ready_locked(thread);

	z_reschedule_locked(ಕೀಲಿ);

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_EXIT(k_thread, resume, thread);
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_USERSPACE
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ z_vrfy_k_thread_resume(k_tid_t thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	z_impl_k_thread_resume(thread);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_thread_resume_mrsh.c>
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_USERSPACE */

ಶೂನ್ಯ z_impl_k_wakeup(k_tid_t thread)
{
	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC(k_thread, wakeup, thread);

	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = z_sched_spinlock_ಬೀಗ();

	ಆದರೆ (z_is_thread_sleeping(thread)) {
		(ಶೂನ್ಯ)z_try_abort_thread_timeout(thread);
		z_mark_thread_as_not_sleeping(thread);
		z_sched_ready_locked(thread);
		z_reschedule_locked(ಕೀಲಿ);
	} ಇಲ್ಲದಿದ್ದರೆ {
		z_sched_spinlock_unlock(ಕೀಲಿ);
	}
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_USERSPACE
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ z_vrfy_k_wakeup(k_tid_t thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	z_impl_k_wakeup(thread);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_wakeup_mrsh.c>
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_USERSPACE */

ಶೂನ್ಯ z_thread_abort(ರಚನೆ k_thread *thread)
{
	bool essential = z_is_thread_essential(thread);
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = z_sched_spinlock_ಬೀಗ();

	ಆದರೆ (z_is_thread_dead(thread)) {
		z_sched_spinlock_unlock(ಕೀಲಿ);
		ಹಿಂತಿರುಗಿ;
	}

	z_thread_halt(thread, ಕೀಲಿ, true);

	ಆದರೆ (essential) {
		__ASSERT(!essential, "aborted essential thread %p", thread);
		k_panic();
	}
}

#ಆದರೆ !defined(CONFIG_ARCH_HAS_THREAD_ABORT)
ಶೂನ್ಯ z_impl_k_thread_abort(k_tid_t thread)
{
	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_ENTER(k_thread, abort, thread);

	z_thread_abort(thread);

	__ASSERT_NO_MSG(z_is_thread_dead(thread));

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_EXIT(k_thread, abort, thread);
}
#ಆದರೆ_ಅಂತ್ಯ /* !CONFIG_ARCH_HAS_THREAD_ABORT */

ಪೂರ್ಣಾಂಕ z_impl_k_thread_join(ರಚನೆ k_thread *thread, k_timeout_t timeout)
{
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = z_sched_spinlock_ಬೀಗ();
	ಪೂರ್ಣಾಂಕ ret;

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_ENTER(k_thread, join, thread, timeout);

	ಆದರೆ (z_is_thread_dead(thread)) {
		z_sched_switch_spin(thread);
		ret = 0;
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		ret = -EBUSY;
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ ((thread == _current) ||
		   (thread->ತಳ.pended_on == &_current->join_ಸರತಿ)) {
		ret = -EDEADLK;
	} ಇಲ್ಲದಿದ್ದರೆ {
		__ASSERT(!arch_is_ಒಳ_isr(), "cannot join in ISR");
		z_sched_ಕೂಡಿಸು_ಗಮ್ಯ_waitq_locked(_current, &thread->join_ಸರತಿ);
		z_ಕೂಡಿಸು_thread_timeout(_current, timeout);

		SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_BLOCKING(k_thread, join, thread, timeout);
		ret = z_ಅದಲುಬದಲು_locked(ಕೀಲಿ);
		SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_EXIT(k_thread, join, thread, timeout, ret);

		ಹಿಂತಿರುಗಿ ret;
	}

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_EXIT(k_thread, join, thread, timeout, ret);

	z_sched_spinlock_unlock(ಕೀಲಿ);
	ಹಿಂತಿರುಗಿ ret;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_USERSPACE
/* Special case: don't oops if the thread is uninitialized.  This is because
 * the initialization bit does double-duty for thread objects; if false, means
 * the thread object is truly uninitialized, or the thread ran and exited for
 * some reason.
 *
 * Return true in this case indicating we should just do nothing and return
 * success to the caller.
 */
ಸ್ಥಿರ bool thread_obj_validate(ರಚನೆ k_thread *thread)
{
	ರಚನೆ k_object *ko = k_object_ಕಂಡುಹಿಡಿ(thread);
	ಪೂರ್ಣಾಂಕ ret = k_object_validate(ko, K_OBJ_THREAD, _OBJ_INIT_TRUE);

	ಆಯ್ಕೆ (ret) {
	ಸ್ಥಿತಿ 0:
		ಹಿಂತಿರುಗಿ false;
	ಸ್ಥಿತಿ -EINVAL:
		ಹಿಂತಿರುಗಿ true;
	ಪೂರ್ವನಿಯೋಜಿತ:
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_LOG
		k_object_dump_error(ret, thread, ko, K_OBJ_THREAD);
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_LOG */
		/* ret is a non-zero error code here (the 0 and -EINVAL cases
		 * are handled above), so this branch must always oops. Passing
		 * ret as the "verify" expression would treat the failure code as
		 * success and fall through to CODE_UNREACHABLE; verify ret == 0
		 * so the oops is actually raised.
		 */
		K_OOPS(K_SYSCALL_VERIFY_ಸಂದೇಶ(ret == 0, "access denied"));
	}
	ಸಂಕೇತ_UNREACHABLE; /* LCOV_EXCL_LINE */
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಪೂರ್ಣಾಂಕ z_vrfy_k_thread_join(ರಚನೆ k_thread *thread,
				       k_timeout_t timeout)
{
	ಆದರೆ (thread_obj_validate(thread)) {
		ಹಿಂತಿರುಗಿ 0;
	}

	ಹಿಂತಿರುಗಿ z_impl_k_thread_join(thread, timeout);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_thread_join_mrsh.c>

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ z_vrfy_k_thread_abort(k_tid_t thread)
{
	ಆದರೆ (thread_obj_validate(thread)) {
		ಹಿಂತಿರುಗಿ;
	}

	K_OOPS(K_SYSCALL_VERIFY_ಸಂದೇಶ(!z_is_thread_essential(thread),
				    "aborting essential thread %p", thread));

	z_impl_k_thread_abort((ರಚನೆ k_thread *)thread);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_thread_abort_mrsh.c>
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_USERSPACE */

bool k_can_yield(ಶೂನ್ಯ)
{
	ಹಿಂತಿರುಗಿ !(k_is_pre_kernel() || k_is_ಒಳ_isr() || !arch_cpu_irqs_are_enabled() ||
		 z_is_idle_thread_object(_current));
}

ಶೂನ್ಯ z_impl_k_yield(ಶೂನ್ಯ)
{
	__ASSERT(!arch_is_ಒಳ_isr(), "");

	SYS_ಪೋರ್ಟ್_TRACING_FUNC(k_thread, yield);

	z_sched_yield();
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_USERSPACE
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ z_vrfy_k_yield(ಶೂನ್ಯ)
{
	z_impl_k_yield();
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_yield_mrsh.c>
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_USERSPACE */
