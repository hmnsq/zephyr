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

#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/kernel.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/spinlock.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/sys/math_extras.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/sys/clock.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <ksched.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <kthread.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <wait_q.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <timeout_q.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/internal/syscall_handler.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <kernel_internal.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <kswap.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/init.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/tracing/tracing.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdbool.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/sys/check.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/random/random.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/sys/atomic.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/logging/log.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/llext/symbol.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/sys/iterable_sections.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/sys/minmax.h>

#ਅਨ੍ਤਰ੍ਭਾਵ <usage.h>

LOG_MODULE_DECLARE(os, CONFIG_KERNEL_LOG_LEVEL);

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_OBJ_CORE_THREAD
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_OBJ_CORE_STATS_THREAD
ਸਥਿਰ ਬਣਤਰ k_obj_core_stats_desc  thread_stats_desc = {
	.raw_ਮਾਪ = ਆਕਾਰ(ਬਣਤਰ k_ਚੱਕਰ_stats),
	.query_ਮਾਪ = ਆਕਾਰ(ਬਣਤਰ k_thread_runtime_stats),
	.raw   = z_thread_stats_raw,
	.query = z_thread_stats_query,
	.reset = z_thread_stats_reset,
	.disable = z_thread_stats_disable,
	.enable  = z_thread_stats_enable,
};
#ਜੇ_ਅੰਤ /* CONFIG_OBJ_CORE_STATS_THREAD */

/* Threads have no statically defined instances to walk; each thread links its
 * own object core as it is created (see z_setup_new_thread()). Register the
 * type only.
 */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_OBJ_CORE_STATS_THREAD
K_OBJ_TYPE_DEFINE_TYPE_ONLY(obj_type_thread, k_thread, K_OBJ_TYPE_THREAD_ID,
			    &thread_stats_desc);
#ਨਹੀਂਤਾਂ
K_OBJ_TYPE_DEFINE_TYPE_ONLY(obj_type_thread, k_thread, K_OBJ_TYPE_THREAD_ID,
			    ਨਿਰਾ);
#ਜੇ_ਅੰਤ /* CONFIG_OBJ_CORE_STATS_THREAD */
#ਜੇ_ਅੰਤ /* CONFIG_OBJ_CORE_THREAD */


#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ _FOREACH_STATIC_THREAD(thread_ਅੰਕੜੇ)              \
	STRUCT_SECTION_FOREACH(static_thread_ਅੰਕੜੇ, thread_ਅੰਕੜੇ)

bool k_is_ਵਿੱਚ_isr(ਖਾਲੀ)
{
	ਵਾਪਸ arch_is_ਵਿੱਚ_isr();
}
EXPORT_ਚਿੰਨ੍ਹ(k_is_ਵਿੱਚ_isr);

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_THREAD_CUSTOM_ਅੰਕੜੇ
ਖਾਲੀ z_impl_k_thread_custom_ਅੰਕੜੇ_ਸੈੱਟ(ਖਾਲੀ *ਮੁੱਲ)
{
	_current->custom_ਅੰਕੜੇ = ਮੁੱਲ;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_USERSPACE
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ z_vrfy_k_thread_custom_ਅੰਕੜੇ_ਸੈੱਟ(ਖਾਲੀ *ਅੰਕੜੇ)
{
	z_impl_k_thread_custom_ਅੰਕੜੇ_ਸੈੱਟ(ਅੰਕੜੇ);
}
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/syscalls/k_thread_custom_data_set_mrsh.c>
#ਜੇ_ਅੰਤ /* CONFIG_USERSPACE */

ਖਾਲੀ *z_impl_k_thread_custom_ਅੰਕੜੇ_get(ਖਾਲੀ)
{
	ਵਾਪਸ _current->custom_ਅੰਕੜੇ;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_USERSPACE
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ *z_vrfy_k_thread_custom_ਅੰਕੜੇ_get(ਖਾਲੀ)
{
	ਵਾਪਸ z_impl_k_thread_custom_ਅੰਕੜੇ_get();
}
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/syscalls/k_thread_custom_data_get_mrsh.c>

#ਜੇ_ਅੰਤ /* CONFIG_USERSPACE */
#ਜੇ_ਅੰਤ /* CONFIG_THREAD_CUSTOM_DATA */

ਪੂਰਨ z_impl_k_is_preempt_thread(ਖਾਲੀ)
{
	ਵਾਪਸ !arch_is_ਵਿੱਚ_isr() && thread_is_preemptible(_current);
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_USERSPACE
ਸਥਿਰ ਸਤਰਅੰਦਰ ਪੂਰਨ z_vrfy_k_is_preempt_thread(ਖਾਲੀ)
{
	ਵਾਪਸ z_impl_k_is_preempt_thread();
}
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/syscalls/k_is_preempt_thread_mrsh.c>
#ਜੇ_ਅੰਤ /* CONFIG_USERSPACE */

ਪੂਰਨ z_impl_k_thread_ਤਰਜੀਹ_get(k_tid_t thread)
{
	ਵਾਪਸ thread->ਅਧਾਰ.prio;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_USERSPACE
ਸਥਿਰ ਸਤਰਅੰਦਰ ਪੂਰਨ z_vrfy_k_thread_ਤਰਜੀਹ_get(k_tid_t thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	ਵਾਪਸ z_impl_k_thread_ਤਰਜੀਹ_get(thread);
}
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/syscalls/k_thread_priority_get_mrsh.c>
#ਜੇ_ਅੰਤ /* CONFIG_USERSPACE */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_THREAD_NAME
ਸਥਿਰ ਖਾਲੀ ਸੈੱਟ_thread_name(ਬਣਤਰ k_thread *thread, ਅਚਲ ਅੱਖਰ *str)
{
	ਜੇ (str != ਨਿਰਾ) {
		strncpy(thread->name, str, CONFIG_THREAD_MAX_NAME_LEN - 1);
		/* Ensure NULL termination, truncate if longer */
		thread->name[CONFIG_THREAD_MAX_NAME_LEN - 1] = '\0';
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_ARCH_HAS_THREAD_NAME_HOOK
		arch_thread_name_ਸੈੱਟ(thread, str);
#ਜੇ_ਅੰਤ /* CONFIG_ARCH_HAS_THREAD_NAME_HOOK */
	} ਨਹੀਂਤਾਂ {
		thread->name[0] = '\0';
	}
}
#ਜੇ_ਅੰਤ /* CONFIG_THREAD_NAME */

ਪੂਰਨ z_impl_k_thread_name_ਸੈੱਟ(k_tid_t thread, ਅਚਲ ਅੱਖਰ *str)
{
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_THREAD_NAME
	ਜੇ (thread == ਨਿਰਾ) {
		thread = _current;
	}

	ਸੈੱਟ_thread_name(thread, str);

	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC(k_thread, name_ਸੈੱਟ, thread, 0);

	ਵਾਪਸ 0;
#ਨਹੀਂਤਾਂ
	ARG_ਅਣਵਰਤਿਆ(thread);
	ARG_ਅਣਵਰਤਿਆ(str);

	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC(k_thread, name_ਸੈੱਟ, thread, -ENOSYS);

	ਵਾਪਸ -ENOSYS;
#ਜੇ_ਅੰਤ /* CONFIG_THREAD_NAME */
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_USERSPACE
ਸਥਿਰ ਸਤਰਅੰਦਰ ਪੂਰਨ z_vrfy_k_thread_name_ਸੈੱਟ(k_tid_t thread, ਅਚਲ ਅੱਖਰ *str)
{
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_THREAD_NAME
	ਅੱਖਰ name[CONFIG_THREAD_MAX_NAME_LEN];

	ਜੇ (thread != ਨਿਰਾ) {
		ਜੇ (K_SYSCALL_OBJ(thread, K_OBJ_THREAD) != 0) {
			ਵਾਪਸ -EINVAL;
		}
	}

	/* In theory we could copy directly into thread->name, but
	 * the current z_vrfy / z_impl split does not provide a
	 * means of doing so.
	 */
	ਜੇ (k_usermode_string_copy(name, str, ਆਕਾਰ(name)) != 0) {
		ਵਾਪਸ -EFAULT;
	}

	ਵਾਪਸ z_impl_k_thread_name_ਸੈੱਟ(thread, name);
#ਨਹੀਂਤਾਂ
	ਵਾਪਸ -ENOSYS;
#ਜੇ_ਅੰਤ /* CONFIG_THREAD_NAME */
}
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/syscalls/k_thread_name_set_mrsh.c>
#ਜੇ_ਅੰਤ /* CONFIG_USERSPACE */

ਅਚਲ ਅੱਖਰ *k_thread_name_get(k_tid_t thread)
{
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_THREAD_NAME
	ਵਾਪਸ (ਅਚਲ ਅੱਖਰ *)thread->name;
#ਨਹੀਂਤਾਂ
	ARG_ਅਣਵਰਤਿਆ(thread);
	ਵਾਪਸ ਨਿਰਾ;
#ਜੇ_ਅੰਤ /* CONFIG_THREAD_NAME */
}

ਪੂਰਨ z_impl_k_thread_name_copy(k_tid_t thread, ਅੱਖਰ *buf, size_t ਮਾਪ)
{
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_THREAD_NAME
	strncpy(buf, thread->name, ਮਾਪ);
	ਵਾਪਸ 0;
#ਨਹੀਂਤਾਂ
	ARG_ਅਣਵਰਤਿਆ(thread);
	ARG_ਅਣਵਰਤਿਆ(buf);
	ARG_ਅਣਵਰਤਿਆ(ਮਾਪ);
	ਵਾਪਸ -ENOSYS;
#ਜੇ_ਅੰਤ /* CONFIG_THREAD_NAME */
}

ਸਥਿਰ size_t copy_ਬਾਈਟਾਂ(ਅੱਖਰ *dest, size_t dest_ਮਾਪ, ਅਚਲ ਅੱਖਰ *src, size_t src_ਮਾਪ)
{
	size_t  ਬਾਈਟਾਂ_ਮੰਜ਼ਿਲ_copy;

	ਬਾਈਟਾਂ_ਮੰਜ਼ਿਲ_copy = min(dest_ਮਾਪ, src_ਮਾਪ);
	ਸਮ੍ਰਿਤੀ_ਨਕਲ(dest, src, ਬਾਈਟਾਂ_ਮੰਜ਼ਿਲ_copy);

	ਵਾਪਸ ਬਾਈਟਾਂ_ਮੰਜ਼ਿਲ_copy;
}

ਅਚਲ ਅੱਖਰ *k_thread_state_str(k_tid_t thread_id, ਅੱਖਰ *buf, size_t buf_ਮਾਪ)
{
	size_t      off = 0;
	uint8_t     ਦੋਆਧਾਰੀ_ਅੰਕ_2;
	uint8_t     thread_state = thread_id->ਅਧਾਰ.thread_state;
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SS_ENT(s) { Z_STATE_STR_##s, _THREAD_##s, ਆਕਾਰ(Z_STATE_STR_##s) - 1 }
	ਸਥਿਰ ਅਚਲ ਬਣਤਰ {
		ਅਚਲ ਅੱਖਰ *str;
		uint16_t    ਦੋਆਧਾਰੀ_ਅੰਕ_2;
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
#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ SS_ENT

	ਜੇ ((buf == ਨਿਰਾ) || (buf_ਮਾਪ == 0)) {
		ਵਾਪਸ "";
	}

	buf_ਮਾਪ--;   /* Reserve 1 byte for end-of-string character */

	/*
	 * Loop through each bit in the thread_state. Stop once all have
	 * been processed. If more than one thread_state bit is set, then
	 * separate the descriptive strings with a '+'.
	 */


	ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਸੂਚਕਾਂਕ = 0; thread_state != 0; ਸੂਚਕਾਂਕ++) {
		ਦੋਆਧਾਰੀ_ਅੰਕ_2 = state_string[ਸੂਚਕਾਂਕ].ਦੋਆਧਾਰੀ_ਅੰਕ_2;
		ਜੇ ((thread_state & ਦੋਆਧਾਰੀ_ਅੰਕ_2) == 0) {
			ਜਾਰੀ;
		}

		off += copy_ਬਾਈਟਾਂ(buf + off, buf_ਮਾਪ - off,
				  state_string[ਸੂਚਕਾਂਕ].str,
				  state_string[ਸੂਚਕਾਂਕ].len);

		thread_state &= ~ਦੋਆਧਾਰੀ_ਅੰਕ_2;

		ਜੇ (thread_state != 0) {
			off += copy_ਬਾਈਟਾਂ(buf + off, buf_ਮਾਪ - off, "+", 1);
		}
	}

	buf[off] = '\0';

	ਵਾਪਸ (ਅਚਲ ਅੱਖਰ *)buf;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_USERSPACE
ਸਥਿਰ ਸਤਰਅੰਦਰ ਪੂਰਨ z_vrfy_k_thread_name_copy(k_tid_t thread,
					    ਅੱਖਰ *buf, size_t ਮਾਪ)
{
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_THREAD_NAME
	size_t len;
	ਬਣਤਰ k_object *ko = k_object_ਲੱਭੋ(thread);

	/* Special case: we allow reading the names of initialized threads
	 * even if we don't have permission on them
	 */
	ਜੇ ((ko == ਨਿਰਾ) || (ko->type != K_OBJ_THREAD) ||
		((ko->flags & K_OBJ_FLAG_INITIALIZED) == 0)) {
		ਵਾਪਸ -EINVAL;
	}
	ਜੇ (K_SYSCALL_ਸਮ੍ਰਿਤੀ_WRITE(buf, ਮਾਪ) != 0) {
		ਵਾਪਸ -EFAULT;
	}
	len = ਲੜੀ_ਲੰਬਾਈ(thread->name);
	ਜੇ ((len + 1) > ਮਾਪ) {
		ਵਾਪਸ -ENOSPC;
	}

	ਵਾਪਸ k_usermode_ਮੰਜ਼ਿਲ_copy((ਖਾਲੀ *)buf, thread->name, len + 1);
#ਨਹੀਂਤਾਂ
	ARG_ਅਣਵਰਤਿਆ(thread);
	ARG_ਅਣਵਰਤਿਆ(buf);
	ARG_ਅਣਵਰਤਿਆ(ਮਾਪ);
	ਵਾਪਸ -ENOSYS;
#ਜੇ_ਅੰਤ /* CONFIG_THREAD_NAME */
}
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/syscalls/k_thread_name_copy_mrsh.c>
#ਜੇ_ਅੰਤ /* CONFIG_USERSPACE */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_ਢੇਰ_SENTINEL
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
ਖਾਲੀ z_check_ਢੇਰ_sentinel(ਖਾਲੀ)
{
	uint32_t *ਢੇਰ;

	ਜੇ ((_current->ਅਧਾਰ.thread_state & _THREAD_DUMMY) != 0) {
		ਵਾਪਸ;
	}

	ਢੇਰ = (uint32_t *)_current->ਢੇਰ_info.ਸ਼ੁਰੂ;
	ਜੇ (*ਢੇਰ != ਢੇਰ_SENTINEL) {
		/* Restore it so further checks don't trigger this same error */
		*ਢੇਰ = ਢੇਰ_SENTINEL;
		z_except_reason(K_ERR_ਢੇਰ_CHK_FAIL);
	}
}
#ਜੇ_ਅੰਤ /* CONFIG_STACK_SENTINEL */

#ਜੇ defined(CONFIG_ਢੇਰ_ਸੂਚਕ_RANDOM) && (CONFIG_ਢੇਰ_ਸੂਚਕ_RANDOM != 0)
ਪੂਰਨ z_ਢੇਰ_adjust_initialized;

ਸਥਿਰ size_t random_ਵਿੱਥ(size_t ਢੇਰ_ਮਾਪ)
{
	size_t random_val;

	ਜੇ (!z_ਢੇਰ_adjust_initialized) {
		z_early_rand_get((uint8_t *)&random_val, ਆਕਾਰ(random_val));
	} ਨਹੀਂਤਾਂ {
		sys_rand_get((uint8_t *)&random_val, ਆਕਾਰ(random_val));
	}

	/* Don't need to worry about alignment of the size here,
	 * arch_new_thread() is required to do it.
	 *
	 * FIXME: Not the best way to get a random number in a range.
	 * See #6493
	 */
	ਅਚਲ size_t fuzz = random_val % CONFIG_ਢੇਰ_ਸੂਚਕ_RANDOM;

	ਜੇ (unlikely(fuzz * 2 > ਢੇਰ_ਮਾਪ)) {
		ਵਾਪਸ 0;
	}

	ਵਾਪਸ fuzz;
}
#ਜੇ defined(CONFIG_ਢੇਰ_GROWS_ਉੱਪਰ)
	/* This is so rare not bothering for now */
#ਗਲਤੀ "Stack pointer randomization not implemented for upward growing stacks"
#ਜੇ_ਅੰਤ /* CONFIG_STACK_GROWS_UP */
#ਜੇ_ਅੰਤ /* CONFIG_STACK_POINTER_RANDOM */

/*
 * Geometry of a thread's stack object, computed once and threaded through the
 * setup_thread_stack() helpers below.
 */
ਬਣਤਰ ਢੇਰ_geometry {
	size_t obj_ਮਾਪ;	/* size of the whole stack object */
	ਅੱਖਰ *buf_ਸ਼ੁਰੂ;	/* start of the usable stack buffer */
	size_t buf_ਮਾਪ;	/* size of the usable stack buffer */
};

/*
 * Derive the stack object geometry from the caller-supplied stack and size.
 * User-capable stacks and kernel-only stacks reserve different amounts.
 */
ਸਥਿਰ ਬਣਤਰ ਢੇਰ_geometry compute_ਢੇਰ_geometry(k_thread_ਢੇਰ_t *ਢੇਰ,
						    size_t ਢੇਰ_ਮਾਪ)
{
	ਬਣਤਰ ਢੇਰ_geometry geo;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_USERSPACE
	ਜੇ (z_ਢੇਰ_is_user_capable(ਢੇਰ)) {
		geo.obj_ਮਾਪ = K_THREAD_ਢੇਰ_LEN(ਢੇਰ_ਮਾਪ);
		geo.buf_ਸ਼ੁਰੂ = K_THREAD_ਢੇਰ_ਬਫਰ(ਢੇਰ);
		geo.buf_ਮਾਪ = geo.obj_ਮਾਪ - K_THREAD_ਢੇਰ_RESERVED;
	} ਨਹੀਂਤਾਂ
#ਜੇ_ਅੰਤ /* CONFIG_USERSPACE */
	{
		/* Object cannot host a user mode thread */
		geo.obj_ਮਾਪ = K_KERNEL_ਢੇਰ_LEN(ਢੇਰ_ਮਾਪ);
		geo.buf_ਸ਼ੁਰੂ = K_KERNEL_ਢੇਰ_ਬਫਰ(ਢੇਰ);
		geo.buf_ਮਾਪ = geo.obj_ਮਾਪ - K_KERNEL_ਢੇਰ_RESERVED;

#ਜੇ defined(ARCH_KERNEL_ਢੇਰ_RESERVED)
		/* Zephyr treats stack overflow as an app bug.  But
		 * this particular overflow can be seen by static
		 * analysis so needs to be handled somehow.
		 */
		ਜੇ (K_KERNEL_ਢੇਰ_RESERVED > geo.obj_ਮਾਪ) {
			k_panic();
		}
#ਜੇ_ਅੰਤ
	}

	ਵਾਪਸ geo;
}

/*
 * Return the initial (high end) stack pointer. When stacks are memory mapped
 * this maps the object into virtual memory, records the mapping so it can be
 * torn down at thread exit, and rebases buf_start onto the mapped address.
 */
ਸਥਿਰ ਅੱਖਰ *map_thread_ਢੇਰ(ਬਣਤਰ k_thread *new_thread,
			      k_thread_ਢੇਰ_t *ਢੇਰ,
			      ਬਣਤਰ ਢੇਰ_geometry *geo)
{
	ਅੱਖਰ *ਢੇਰ_ptr;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_THREAD_ਢੇਰ_MEM_MAPPED
	/* Map the stack into virtual memory and use that as the base to
	 * calculate the initial stack pointer at the high end of the stack
	 * object. The stack pointer may be reduced later by TLS or random
	 * offset.
	 *
	 * K_MEM_MAP_UNINIT is used to mimic the behavior of non-mapped
	 * stack. If CONFIG_INIT_STACKS is enabled, the stack will be
	 * cleared later.
	 */
	ਖਾਲੀ *ਢੇਰ_mapped = k_mem_map_phys_guard((uintptr_t)ਢੇਰ, geo->obj_ਮਾਪ,
				K_MEM_PERM_RW | K_MEM_CACHE_WB | K_MEM_MAP_UNINIT,
				false);

	__ASSERT_NO_MSG((uintptr_t)ਢੇਰ_mapped != 0);

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_USERSPACE
	ਜੇ (z_ਢੇਰ_is_user_capable(ਢੇਰ)) {
		geo->buf_ਸ਼ੁਰੂ = K_THREAD_ਢੇਰ_ਬਫਰ(ਢੇਰ_mapped);
	} ਨਹੀਂਤਾਂ
#ਜੇ_ਅੰਤ /* CONFIG_USERSPACE */
	{
		geo->buf_ਸ਼ੁਰੂ = K_KERNEL_ਢੇਰ_ਬਫਰ(ਢੇਰ_mapped);
	}

	ਢੇਰ_ptr = (ਅੱਖਰ *)ਢੇਰ_mapped + geo->obj_ਮਾਪ;

	/* Need to store the info on mapped stack so we can remove the mappings
	 * when the thread ends.
	 */
	new_thread->ਢੇਰ_info.mapped.addr = ਢੇਰ_mapped;
	new_thread->ਢੇਰ_info.mapped.sz = geo->obj_ਮਾਪ;
#ਨਹੀਂਤਾਂ /* CONFIG_THREAD_STACK_MEM_MAPPED */
	ARG_ਅਣਵਰਤਿਆ(new_thread);

	/* Initial stack pointer at the high end of the stack object, may
	 * be reduced later by TLS or random offset
	 */
	ਢੇਰ_ptr = (ਅੱਖਰ *)ਢੇਰ + geo->obj_ਮਾਪ;
#ਜੇ_ਅੰਤ /* CONFIG_THREAD_STACK_MEM_MAPPED */

	ਵਾਪਸ ਢੇਰ_ptr;
}

/* Pre-fill the stack buffer with a known pattern for usage measurement. */
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ fill_init_ਢੇਰ(ਅੱਖਰ *buf_ਸ਼ੁਰੂ, size_t buf_ਮਾਪ)
{
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_INIT_STACKS
	memset(buf_ਸ਼ੁਰੂ, 0xaa, buf_ਮਾਪ);
#ਨਹੀਂਤਾਂ
	ARG_ਅਣਵਰਤਿਆ(buf_ਸ਼ੁਰੂ);
	ARG_ਅਣਵਰਤਿਆ(buf_ਮਾਪ);
#ਜੇ_ਅੰਤ /* CONFIG_INIT_STACKS */
}

/* Place the overflow-detection sentinel at the low end of the stack buffer. */
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ ਸੈੱਟ_ਢੇਰ_sentinel(ਅੱਖਰ *buf_ਸ਼ੁਰੂ)
{
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_ਢੇਰ_SENTINEL
	/* Put the stack sentinel at the lowest 4 bytes of the stack area.
	 * We periodically check that it's still present and kill the thread
	 * if it isn't.
	 */
	*((uint32_t *)buf_ਸ਼ੁਰੂ) = ਢੇਰ_SENTINEL;
#ਨਹੀਂਤਾਂ
	ARG_ਅਣਵਰਤਿਆ(buf_ਸ਼ੁਰੂ);
#ਜੇ_ਅੰਤ /* CONFIG_STACK_SENTINEL */
}

/*
 * Compute how much to reserve at the top of the stack buffer for TLS,
 * userspace-local data and stack-pointer randomization, and return the
 * aligned amount the initial stack pointer must be lowered by.
 */
ਸਥਿਰ size_t reserve_ਢੇਰ_headroom(ਬਣਤਰ k_thread *new_thread,
				     ਅੱਖਰ *ਢੇਰ_ptr, size_t buf_ਮਾਪ)
{
	size_t delta = 0;

	ARG_ਅਣਵਰਤਿਆ(new_thread);
	ARG_ਅਣਵਰਤਿਆ(ਢੇਰ_ptr);
	ARG_ਅਣਵਰਤਿਆ(buf_ਮਾਪ);

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_THREAD_LOCAL_STORAGE
	/* TLS is always last within the stack buffer */
	delta += arch_tls_ਢੇਰ_ਤਿਆਰੀ(new_thread, ਢੇਰ_ptr);
#ਜੇ_ਅੰਤ /* CONFIG_THREAD_LOCAL_STORAGE */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_THREAD_USERSPACE_LOCAL_ਅੰਕੜੇ
	size_t tls_ਮਾਪ = ਆਕਾਰ(ਬਣਤਰ thread_userspace_local_ਅੰਕੜੇ);

	/* reserve space on highest memory of stack buffer for local data */
	delta += tls_ਮਾਪ;
	new_thread->userspace_local_ਅੰਕੜੇ =
		(ਬਣਤਰ thread_userspace_local_ਅੰਕੜੇ *)(ਢੇਰ_ptr - delta);
#ਜੇ_ਅੰਤ /* CONFIG_THREAD_USERSPACE_LOCAL_DATA */
#ਜੇ defined(CONFIG_ਢੇਰ_ਸੂਚਕ_RANDOM) && (CONFIG_ਢੇਰ_ਸੂਚਕ_RANDOM != 0)
	delta += random_ਵਿੱਥ(buf_ਮਾਪ);
#ਜੇ_ਅੰਤ /* CONFIG_STACK_POINTER_RANDOM */

	ਵਾਪਸ ਗੇੜਾ_ਉੱਪਰ(delta, ARCH_ਢੇਰ_PTR_ALIGN);
}

/* Record the thread-accessible stack bounds (and runtime-safety threshold). */
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ ਸੈੱਟ_ਢੇਰ_info(ਬਣਤਰ k_thread *new_thread, ਅੱਖਰ *buf_ਸ਼ੁਰੂ,
				  size_t buf_ਮਾਪ, size_t delta)
{
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_THREAD_ਢੇਰ_INFO
	/* Initial values. Arches which implement MPU guards that "borrow"
	 * memory from the stack buffer (not tracked in K_THREAD_STACK_RESERVED)
	 * will need to appropriately update this.
	 *
	 * The bounds tracked here correspond to the area of the stack object
	 * that the thread can access, which includes TLS.
	 */
	new_thread->ਢੇਰ_info.ਸ਼ੁਰੂ = (uintptr_t)buf_ਸ਼ੁਰੂ;
	new_thread->ਢੇਰ_info.ਮਾਪ = buf_ਮਾਪ;
	new_thread->ਢੇਰ_info.delta = delta;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_THREAD_RUNTIME_ਢੇਰ_SAFETY
	new_thread->ਢੇਰ_info.usage.ਅਣਵਰਤਿਆ_ਹੱਦ =
		(CONFIG_THREAD_RUNTIME_ਢੇਰ_SAFETY_DEFAULT_ਅਣਵਰਤਿਆ_ਹੱਦ_PCT *
		 buf_ਮਾਪ) / 100;
#ਜੇ_ਅੰਤ
#ਨਹੀਂਤਾਂ
	ARG_ਅਣਵਰਤਿਆ(new_thread);
	ARG_ਅਣਵਰਤਿਆ(buf_ਸ਼ੁਰੂ);
	ARG_ਅਣਵਰਤਿਆ(buf_ਮਾਪ);
	ARG_ਅਣਵਰਤਿਆ(delta);
#ਜੇ_ਅੰਤ /* CONFIG_THREAD_STACK_INFO */
}

ਸਥਿਰ ਅੱਖਰ *ਤਿਆਰੀ_thread_ਢੇਰ(ਬਣਤਰ k_thread *new_thread,
				k_thread_ਢੇਰ_t *ਢੇਰ, size_t ਢੇਰ_ਮਾਪ)
{
	ਬਣਤਰ ਢੇਰ_geometry geo = compute_ਢੇਰ_geometry(ਢੇਰ, ਢੇਰ_ਮਾਪ);
	ਅੱਖਰ *ਢੇਰ_ptr = map_thread_ਢੇਰ(new_thread, ਢੇਰ, &geo);
	size_t delta;

	LOG_DBG("stack %p for thread %p: obj_size=%zu buf_start=%p "
		" buf_size %zu stack_ptr=%p",
		ਢੇਰ, new_thread, geo.obj_ਮਾਪ, (ਖਾਲੀ *)geo.buf_ਸ਼ੁਰੂ,
		geo.buf_ਮਾਪ, (ਖਾਲੀ *)ਢੇਰ_ptr);

	fill_init_ਢੇਰ(geo.buf_ਸ਼ੁਰੂ, geo.buf_ਮਾਪ);
	ਸੈੱਟ_ਢੇਰ_sentinel(geo.buf_ਸ਼ੁਰੂ);
	delta = reserve_ਢੇਰ_headroom(new_thread, ਢੇਰ_ptr, geo.buf_ਮਾਪ);
	ਸੈੱਟ_ਢੇਰ_info(new_thread, geo.buf_ਸ਼ੁਰੂ, geo.buf_ਮਾਪ, delta);

	ਵਾਪਸ ਢੇਰ_ptr - delta;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_HW_SHADOW_ਢੇਰ
ਸਥਿਰ ਖਾਲੀ ਤਿਆਰੀ_shadow_ਢੇਰ(ਬਣਤਰ k_thread *new_thread,
			 k_thread_ਢੇਰ_t *ਢੇਰ)
{
	ਪੂਰਨ ret = -ENOENT;

	STRUCT_SECTION_FOREACH(ਢੇਰ_ਮੰਜ਼ਿਲ_hw_shadow_ਢੇਰ, stk_ਮੰਜ਼ਿਲ_hw_shstk) {
		ਜੇ (stk_ਮੰਜ਼ਿਲ_hw_shstk->ਢੇਰ == ਢੇਰ) {
			ret = k_thread_hw_shadow_ਢੇਰ_attach(new_thread,
							      stk_ਮੰਜ਼ਿਲ_hw_shstk->shstk_addr,
							      stk_ਮੰਜ਼ਿਲ_hw_shstk->ਮਾਪ);
			ਜੇ (ret != 0) {
				LOG_ERR("Could not set thread %p shadow stack %p, got error %d",
					new_thread, stk_ਮੰਜ਼ਿਲ_hw_shstk->shstk_addr, ret);
				k_panic();
			}
			ਰੋਕੋ;
		}
	}

	/* Check if the stack isn't in a stack array, and use the corresponding
	 * shadow stack.
	 */
	ਜੇ (ret != -ENOENT) {
		ਵਾਪਸ;
	}

	STRUCT_SECTION_FOREACH(ਢੇਰ_ਮੰਜ਼ਿਲ_hw_shadow_ਢੇਰ_arr, stk_ਮੰਜ਼ਿਲ_hw_shstk) {
		ਜੇ ((uintptr_t)ਢੇਰ >= stk_ਮੰਜ਼ਿਲ_hw_shstk->ਢੇਰ_addr &&
		    (uintptr_t)ਢੇਰ < stk_ਮੰਜ਼ਿਲ_hw_shstk->ਢੇਰ_addr +
		    stk_ਮੰਜ਼ਿਲ_hw_shstk->ਢੇਰ_ਮਾਪ * stk_ਮੰਜ਼ਿਲ_hw_shstk->nmemb) {
			/* Now we have to guess which index of the stack array is being used */
			uintptr_t ਢੇਰ_ਵਿੱਥ = (uintptr_t)ਢੇਰ - stk_ਮੰਜ਼ਿਲ_hw_shstk->ਢੇਰ_addr;
			uintptr_t ਢੇਰ_ਸੂਚਕਾਂਕ = ਢੇਰ_ਵਿੱਥ / stk_ਮੰਜ਼ਿਲ_hw_shstk->ਢੇਰ_ਮਾਪ;
			uintptr_t addr;

			ਜੇ (ਢੇਰ_ਸੂਚਕਾਂਕ >= stk_ਮੰਜ਼ਿਲ_hw_shstk->nmemb) {
				LOG_ERR("Could not find shadow stack for thread %p, stack %p",
					new_thread, ਢੇਰ);
				k_panic();
			}

			addr = stk_ਮੰਜ਼ਿਲ_hw_shstk->shstk_addr +
				stk_ਮੰਜ਼ਿਲ_hw_shstk->shstk_ਮਾਪ * ਢੇਰ_ਸੂਚਕਾਂਕ;
			ret = k_thread_hw_shadow_ਢੇਰ_attach(new_thread,
							      (arch_thread_hw_shadow_ਢੇਰ_t *)addr,
							      stk_ਮੰਜ਼ਿਲ_hw_shstk->shstk_ਮਾਪ);
			ਜੇ (ret != 0) {
				LOG_ERR("Could not set thread %p shadow stack 0x%lx, got error %d",
					new_thread, stk_ਮੰਜ਼ਿਲ_hw_shstk->shstk_addr, ret);
				k_panic();
			}
			ਰੋਕੋ;
		}
	}

	ਜੇ (ret == -ENOENT) {
		LOG_ERR("Could not find shadow stack for thread %p, stack %p",
			new_thread, ਢੇਰ);
		k_panic();
	}
}
#ਨਹੀਂਤਾਂ
/* No-op stub used when hardware shadow stacks are not configured. */
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ ਤਿਆਰੀ_shadow_ਢੇਰ(ਬਣਤਰ k_thread *new_thread,
				      k_thread_ਢੇਰ_t *ਢੇਰ)
{
	ARG_ਅਣਵਰਤਿਆ(new_thread);
	ARG_ਅਣਵਰਤਿਆ(ਢੇਰ);
}
#ਜੇ_ਅੰਤ

/*
 * The following init_thread_*()/assert_*() helpers each own one optional
 * kernel feature. Building z_setup_new_thread() out of these calls keeps its
 * body a readable, linear narrative instead of a wall of #ifdefs. Every helper
 * is static inline and collapses to nothing when its feature is disabled, so
 * this is purely a readability change with no runtime cost.
 */

/* Reset per-thread cleanup bookkeeping before a k_thread object is reused. */
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ thread_abort_cleanup_check_reuse(ਬਣਤਰ k_thread *thread)
{
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_THREAD_ABORT_NEED_CLEANUP
	k_thread_abort_cleanup_check_reuse(thread);
#ਨਹੀਂਤਾਂ
	ARG_ਅਣਵਰਤਿਆ(thread);
#ਜੇ_ਅੰਤ /* CONFIG_THREAD_ABORT_NEED_CLEANUP */
}

/* Register the thread with the object-core framework and its usage stats. */
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ init_thread_obj_core(ਬਣਤਰ k_thread *thread)
{
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_OBJ_CORE_THREAD
	k_obj_core_init_and_link(K_OBJ_CORE(thread), &obj_type_thread);
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_OBJ_CORE_STATS_THREAD
	k_obj_core_stats_ਰਜਿਸਟਰ(K_OBJ_CORE(thread), &thread->ਅਧਾਰ.usage,
				  ਆਕਾਰ(thread->ਅਧਾਰ.usage));
#ਜੇ_ਅੰਤ /* CONFIG_OBJ_CORE_STATS_THREAD */
#ਨਹੀਂਤਾਂ
	ARG_ਅਣਵਰਤਿਆ(thread);
#ਜੇ_ਅੰਤ /* CONFIG_OBJ_CORE_THREAD */
}

/*
 * Initialize userspace bookkeeping: register the thread and stack kernel
 * objects, record stack ownership, and grant the thread access to itself.
 */
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ init_thread_userspace(ਬਣਤਰ k_thread *thread,
					 k_thread_ਢੇਰ_t *ਢੇਰ,
					 uint32_t options)
{
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_USERSPACE
	__ASSERT((options & K_USER) == 0U || z_ਢੇਰ_is_user_capable(ਢੇਰ),
		 "user thread %p with kernel-only stack %p", thread, ਢੇਰ);
	k_object_init(thread);
	k_object_init(ਢੇਰ);
	thread->ਢੇਰ_obj = ਢੇਰ;
	thread->syscall_ਚੌਖਟਾ = ਨਿਰਾ;

	/* Any given thread has access to itself */
	k_object_access_grant(thread, thread);
#ਨਹੀਂਤਾਂ
	ARG_ਅਣਵਰਤਿਆ(thread);
	ARG_ਅਣਵਰਤਿਆ(ਢੇਰ);
	ARG_ਅਣਵਰਤਿਆ(options);
#ਜੇ_ਅੰਤ /* CONFIG_USERSPACE */
}

/* Assert the thread object is cache-coherent while its stack stays cached. */
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ assert_thread_coherence(ਬਣਤਰ k_thread *thread,
					   k_thread_ਢੇਰ_t *ਢੇਰ)
{
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_KERNEL_COHERENCE
	/* Check that the thread object is safe, but that the stack is
	 * still cached!
	 */
	__ASSERT_NO_MSG(sys_cache_is_mem_coherent(thread));

	/* When dynamic thread stack is available, the stack may come from
	 * uncached area.
	 */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ CONFIG_DYNAMIC_THREAD
	__ASSERT_NO_MSG(!sys_cache_is_mem_coherent(ਢੇਰ));
#ਜੇ_ਅੰਤ /* CONFIG_DYNAMIC_THREAD */
#ਜੇ_ਅੰਤ /* CONFIG_KERNEL_COHERENCE */
	ARG_ਅਣਵਰਤਿਆ(thread);
	ARG_ਅਣਵਰਤਿਆ(ਢੇਰ);
}

/* Assert the arch layer populated switch_handle on USE_SWITCH builds. */
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ assert_switch_handle(ਬਣਤਰ k_thread *thread)
{
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_USE_SWITCH
	/* switch_handle must be non-null except when inside z_swap()
	 * for synchronization reasons.  Historically some notional
	 * USE_SWITCH architectures have actually ignored the field
	 */
	__ASSERT(thread->switch_handle != ਨਿਰਾ,
		 "arch layer failed to initialize switch_handle");
#ਜੇ_ਅੰਤ /* CONFIG_USE_SWITCH */
	ARG_ਅਣਵਰਤਿਆ(thread);
}

/* Clear the opaque, kernel-managed per-thread custom data pointer. */
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ init_thread_custom_ਅੰਕੜੇ(ਬਣਤਰ k_thread *thread)
{
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_THREAD_CUSTOM_ਅੰਕੜੇ
	/* Initialize custom data field (value is opaque to kernel) */
	thread->custom_ਅੰਕੜੇ = ਨਿਰਾ;
#ਨਹੀਂਤਾਂ
	ARG_ਅਣਵਰਤਿਆ(thread);
#ਜੇ_ਅੰਤ /* CONFIG_THREAD_CUSTOM_DATA */
}

/*
 * Record the entry point and parameters, then link the thread into the
 * global thread-monitor list under the monitor spinlock.
 */
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ ਜੋੜੋ_thread_ਮੰਜ਼ਿਲ_monitor(ਬਣਤਰ k_thread *thread,
					 k_thread_ਇੰਦਰਾਜ_t ਇੰਦਰਾਜ,
					 ਖਾਲੀ *p1, ਖਾਲੀ *p2, ਖਾਲੀ *p3)
{
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_THREAD_MONITOR
	thread->ਇੰਦਰਾਜ.p_ਇੰਦਰਾਜ = ਇੰਦਰਾਜ;
	thread->ਇੰਦਰਾਜ.parameter1 = p1;
	thread->ਇੰਦਰਾਜ.parameter2 = p2;
	thread->ਇੰਦਰਾਜ.parameter3 = p3;

	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&z_thread_monitor_ਤਾਲਾ);

	thread->ਅਗਲਾ_thread = _kernel.ਧਾਗੇ;
	_kernel.ਧਾਗੇ = thread;
	k_spin_unlock(&z_thread_monitor_ਤਾਲਾ, ਕੁੰਜੀ);
#ਨਹੀਂਤਾਂ
	ARG_ਅਣਵਰਤਿਆ(thread);
	ARG_ਅਣਵਰਤਿਆ(ਇੰਦਰਾਜ);
	ARG_ਅਣਵਰਤਿਆ(p1);
	ARG_ਅਣਵਰਤਿਆ(p2);
	ARG_ਅਣਵਰਤਿਆ(p3);
#ਜੇ_ਅੰਤ /* CONFIG_THREAD_MONITOR */
}

/* Set the thread's name from the caller-supplied string (may be NULL). */
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ init_thread_name(ਬਣਤਰ k_thread *thread, ਅਚਲ ਅੱਖਰ *name)
{
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_THREAD_NAME
	ਸੈੱਟ_thread_name(thread, name);
#ਨਹੀਂਤਾਂ
	ARG_ਅਣਵਰਤਿਆ(thread);
	ARG_ਅਣਵਰਤਿਆ(name);
#ਜੇ_ਅੰਤ /* CONFIG_THREAD_NAME */
}

/* Set the thread's initial SMP CPU affinity mask. */
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ init_thread_cpu_ਨਕਾਬ(ਬਣਤਰ k_thread *thread)
{
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SCHED_CPU_ਨਕਾਬ
	ਜੇ (IS_ENABLED(CONFIG_SCHED_CPU_ਨਕਾਬ_PIN_ONLY)) {
		thread->ਅਧਾਰ.cpu_ਨਕਾਬ = 1; /* must specify only one cpu */
	} ਨਹੀਂਤਾਂ {
		thread->ਅਧਾਰ.cpu_ਨਕਾਬ = -1; /* allow all cpus */
	}
#ਨਹੀਂਤਾਂ
	ARG_ਅਣਵਰਤਿਆ(thread);
#ਜੇ_ਅੰਤ /* CONFIG_SCHED_CPU_MASK */
}

/* Attach the thread to its memory domain and inherit caller permissions
 * when K_INHERIT_PERMS is requested.
 */
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ init_thread_userspace_perms(ਬਣਤਰ k_thread *thread,
					       uint32_t options)
{
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_USERSPACE
	z_mem_domain_init_thread(thread);

	ਜੇ ((options & K_INHERIT_PERMS) != 0U) {
		k_thread_perms_inherit(_current, thread);
	}
#ਨਹੀਂਤਾਂ
	ARG_ਅਣਵਰਤਿਆ(thread);
	ARG_ਅਣਵਰਤਿਆ(options);
#ਜੇ_ਅੰਤ /* CONFIG_USERSPACE */
}

/* Clear the EDF scheduling deadline. */
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ init_thread_deadline(ਬਣਤਰ k_thread *thread)
{
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SCHED_DEADLINE
	thread->ਅਧਾਰ.prio_deadline = 0;
#ਨਹੀਂਤਾਂ
	ARG_ਅਣਵਰਤਿਆ(thread);
#ਜੇ_ਅੰਤ /* CONFIG_SCHED_DEADLINE */
}

/* Initialize the wait queue used to synchronize SMP thread halt/abort. */
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ init_thread_halt_ਕਤਾਰ(ਬਣਤਰ k_thread *thread)
{
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SMP
	z_waitq_init(&thread->halt_ਕਤਾਰ);
#ਨਹੀਂਤਾਂ
	ARG_ਅਣਵਰਤਿਆ(thread);
#ਜੇ_ਅੰਤ /* CONFIG_SMP */
}

/* Initialize runtime CPU-usage tracking for the thread. */
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ init_thread_usage(ਬਣਤਰ k_thread *thread)
{
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SCHED_THREAD_USAGE
	thread->ਅਧਾਰ.usage = (ਬਣਤਰ k_ਚੱਕਰ_stats) {};
	thread->ਅਧਾਰ.usage.track_usage = CONFIG_SCHED_THREAD_USAGE_AUTO_ENABLE;
#ਨਹੀਂਤਾਂ
	ARG_ਅਣਵਰਤਿਆ(thread);
#ਜੇ_ਅੰਤ /* CONFIG_SCHED_THREAD_USAGE */
}

/* Assert the thread wasn't already linked into the monitor list. */
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ assert_thread_not_reused(ਬਣਤਰ k_thread *new_thread)
{
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_THREAD_MONITOR
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_ASSERT
	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&z_thread_monitor_ਤਾਲਾ);
	bool reused = false;

	/* Check if the thread is already in the list */
	ਲਈ (ਬਣਤਰ k_thread *t = _kernel.ਧਾਗੇ; t; t = t->ਅਗਲਾ_thread) {
		ਜੇ (t == new_thread) {
			reused = true;
			ਰੋਕੋ;
		}
	}

	k_spin_unlock(&z_thread_monitor_ਤਾਲਾ, ਕੁੰਜੀ);

	__ASSERT(!reused, "thread %p is already in the running list", new_thread);
#ਜੇ_ਅੰਤ /* CONFIG_ASSERT */
#ਜੇ_ਅੰਤ /* CONFIG_THREAD_MONITOR */
	ARG_ਅਣਵਰਤਿਆ(new_thread);
}

/*
 * The provided stack_size value is presumed to be either the result of
 * K_THREAD_STACK_SIZEOF(stack), or the size value passed to the instance
 * of K_THREAD_STACK_DEFINE() which defined 'stack'.
 */
ਅੱਖਰ *z_ਤਿਆਰੀ_new_thread(ਬਣਤਰ k_thread *new_thread,
			 k_thread_ਢੇਰ_t *ਢੇਰ, size_t ਢੇਰ_ਮਾਪ,
			 k_thread_ਇੰਦਰਾਜ_t ਇੰਦਰਾਜ,
			 ਖਾਲੀ *p1, ਖਾਲੀ *p2, ਖਾਲੀ *p3,
			 ਪੂਰਨ prio, uint32_t options, ਅਚਲ ਅੱਖਰ *name)
{
	ਅੱਖਰ *ਢੇਰ_ptr;

	Z_ASSERT_VALID_PRIO(prio, ਇੰਦਰਾਜ);

	assert_thread_not_reused(new_thread);

	thread_abort_cleanup_check_reuse(new_thread);
	init_thread_obj_core(new_thread);
	init_thread_userspace(new_thread, ਢੇਰ, options);

	z_waitq_init(&new_thread->join_ਕਤਾਰ);

	/* Initialize various struct k_thread members */
	z_init_thread_ਅਧਾਰ(&new_thread->ਅਧਾਰ, prio, _THREAD_SLEEPING, options);
	ਢੇਰ_ptr = ਤਿਆਰੀ_thread_ਢੇਰ(new_thread, ਢੇਰ, ਢੇਰ_ਮਾਪ);

#ਜੇ Z_MUTEX_PI_ENABLED
	sys_slist_init(&new_thread->held_mutexes);
	new_thread->mutex_pended_on = ਨਿਰਾ;
	new_thread->orig_prio = new_thread->ਅਧਾਰ.prio;
#ਜੇ_ਅੰਤ /* Z_MUTEX_PI_ENABLED */

	ਤਿਆਰੀ_shadow_ਢੇਰ(new_thread, ਢੇਰ);
	assert_thread_coherence(new_thread, ਢੇਰ);

	arch_new_thread(new_thread, ਢੇਰ, ਢੇਰ_ptr, ਇੰਦਰਾਜ, p1, p2, p3);

	/* static threads overwrite it afterwards with real value */
	new_thread->init_ਅੰਕੜੇ = ਨਿਰਾ;

	assert_switch_handle(new_thread);
	init_thread_custom_ਅੰਕੜੇ(new_thread);
	ਜੋੜੋ_thread_ਮੰਜ਼ਿਲ_monitor(new_thread, ਇੰਦਰਾਜ, p1, p2, p3);
	init_thread_name(new_thread, name);
	init_thread_cpu_ਨਕਾਬ(new_thread);

	/* _current may be NULL if the dummy thread is not used */
	ਜੇ (IS_ENABLED(CONFIG_ARCH_HAS_CUSTOM_ਅਦਲਾ_ਬਦਲੀ_ਮੰਜ਼ਿਲ_MAIN) &&
	    (_current == ਨਿਰਾ)) {
		new_thread->resource_pool = ਨਿਰਾ;
		ਵਾਪਸ ਢੇਰ_ptr;
	}

	init_thread_userspace_perms(new_thread, options);
	init_thread_deadline(new_thread);
	new_thread->resource_pool = _current->resource_pool;
	init_thread_halt_ਕਤਾਰ(new_thread);
	init_thread_usage(new_thread);

	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC(k_thread, create, new_thread);

	ਵਾਪਸ ਢੇਰ_ptr;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_THREAD_RUNTIME_ਢੇਰ_SAFETY
ਪੂਰਨ z_impl_k_thread_runtime_ਢੇਰ_ਅਣਵਰਤਿਆ_ਹੱਦ_pct_ਸੈੱਟ(ਬਣਤਰ k_thread *thread,
							   uint32_t pct)
{
	size_t ਅਣਵਰਤਿਆ_ਹੱਦ;

	ਜੇ (pct > 99) {
		ਵਾਪਸ -EINVAL;   /* 100% unused stack and up is invalid */
	}

	ਅਣਵਰਤਿਆ_ਹੱਦ = (thread->ਢੇਰ_info.ਮਾਪ * pct) / 100;

	thread->ਢੇਰ_info.usage.ਅਣਵਰਤਿਆ_ਹੱਦ = ਅਣਵਰਤਿਆ_ਹੱਦ;

	ਵਾਪਸ 0;
}

ਪੂਰਨ z_impl_k_thread_runtime_ਢੇਰ_ਅਣਵਰਤਿਆ_ਹੱਦ_ਸੈੱਟ(ਬਣਤਰ k_thread *thread,
						       size_t ਹੱਦ)
{
	ਜੇ (ਹੱਦ > thread->ਢੇਰ_info.ਮਾਪ) {
		ਵਾਪਸ -EINVAL;
	}

	thread->ਢੇਰ_info.usage.ਅਣਵਰਤਿਆ_ਹੱਦ = ਹੱਦ;

	ਵਾਪਸ 0;
}

size_t z_impl_k_thread_runtime_ਢੇਰ_ਅਣਵਰਤਿਆ_ਹੱਦ_get(ਬਣਤਰ k_thread *thread)
{
	ਵਾਪਸ thread->ਢੇਰ_info.usage.ਅਣਵਰਤਿਆ_ਹੱਦ;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_USERSPACE
ਪੂਰਨ z_vrfy_k_thread_runtime_ਢੇਰ_ਅਣਵਰਤਿਆ_ਹੱਦ_pct_ਸੈੱਟ(ਬਣਤਰ k_thread *thread,
							   uint32_t pct)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));

	ਵਾਪਸ z_impl_k_thread_runtime_ਢੇਰ_ਅਣਵਰਤਿਆ_ਹੱਦ_pct_ਸੈੱਟ(thread, pct);
}
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/syscalls/k_thread_runtime_stack_unused_threshold_pct_set_mrsh.c>

ਪੂਰਨ z_vrfy_k_thread_runtime_ਢੇਰ_ਅਣਵਰਤਿਆ_ਹੱਦ_ਸੈੱਟ(ਬਣਤਰ k_thread *thread,
						       size_t ਹੱਦ)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));

	ਵਾਪਸ z_impl_k_thread_runtime_ਢੇਰ_ਅਣਵਰਤਿਆ_ਹੱਦ_ਸੈੱਟ(thread, ਹੱਦ);
}
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/syscalls/k_thread_runtime_stack_unused_threshold_set_mrsh.c>

size_t z_vrfy_k_thread_runtime_ਢੇਰ_ਅਣਵਰਤਿਆ_ਹੱਦ_get(ਬਣਤਰ k_thread *thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));

	ਵਾਪਸ z_impl_k_thread_runtime_ਢੇਰ_ਅਣਵਰਤਿਆ_ਹੱਦ_get(thread);
}
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/syscalls/k_thread_runtime_stack_unused_threshold_get_mrsh.c>
#ਜੇ_ਅੰਤ /* CONFIG_USERSPACE */
#ਜੇ_ਅੰਤ /* CONFIG_THREAD_RUNTIME_STACK_SAFETY */

k_tid_t z_impl_k_thread_create(ਬਣਤਰ k_thread *new_thread,
			      k_thread_ਢੇਰ_t *ਢੇਰ,
			      size_t ਢੇਰ_ਮਾਪ, k_thread_ਇੰਦਰਾਜ_t ਇੰਦਰਾਜ,
			      ਖਾਲੀ *p1, ਖਾਲੀ *p2, ਖਾਲੀ *p3,
			      ਪੂਰਨ prio, uint32_t options, k_timeout_t delay)
{
	__ASSERT(!arch_is_ਵਿੱਚ_isr(), "Threads may not be created in ISRs");

	z_ਤਿਆਰੀ_new_thread(new_thread, ਢੇਰ, ਢੇਰ_ਮਾਪ, ਇੰਦਰਾਜ, p1, p2, p3,
			  prio, options, ਨਿਰਾ);

	thread_ਸਮਾਂ_ਸਾਰਣੀ_new(new_thread, delay);

	ਵਾਪਸ new_thread;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_USERSPACE
bool z_ਢੇਰ_is_user_capable(k_thread_ਢੇਰ_t *ਢੇਰ)
{
	ਵਾਪਸ k_object_ਲੱਭੋ(ਢੇਰ) != ਨਿਰਾ;
}

k_tid_t z_vrfy_k_thread_create(ਬਣਤਰ k_thread *new_thread,
			       k_thread_ਢੇਰ_t *ਢੇਰ,
			       size_t ਢੇਰ_ਮਾਪ, k_thread_ਇੰਦਰਾਜ_t ਇੰਦਰਾਜ,
			       ਖਾਲੀ *p1, ਖਾਲੀ *p2, ਖਾਲੀ *p3,
			       ਪੂਰਨ prio, uint32_t options, k_timeout_t delay)
{
	size_t ਕੁੱਲ_ਮਾਪ, ਢੇਰ_obj_ਮਾਪ;
	ਬਣਤਰ k_object *ਢੇਰ_object;

	/* The thread and stack objects *must* be in an uninitialized state */
	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(new_thread, K_OBJ_THREAD));

	/* No need to check z_stack_is_user_capable(), it won't be in the
	 * object table if it isn't
	 */
	ਢੇਰ_object = k_object_ਲੱਭੋ(ਢੇਰ);
	K_OOPS(K_SYSCALL_VERIFY_ਸੁਨੇਹਾ(k_object_validation_check(ਢੇਰ_object, ਢੇਰ,
						K_OBJ_THREAD_ਢੇਰ_ELEMENT,
						_OBJ_INIT_FALSE) == 0,
				    "bad stack object"));

	/* Verify that the stack size passed in is OK by computing the total
	 * size and comparing it with the size value in the object metadata
	 */
	K_OOPS(K_SYSCALL_VERIFY_ਸੁਨੇਹਾ(!ਮਾਪ_ਜੋੜੋ_ਉਛਾਲ(K_THREAD_ਢੇਰ_RESERVED,
						       ਢੇਰ_ਮਾਪ, &ਕੁੱਲ_ਮਾਪ),
				    "stack size overflow (%zu+%zu)",
				    ਢੇਰ_ਮਾਪ,
				    K_THREAD_ਢੇਰ_RESERVED));

	/* Testing less-than-or-equal since additional room may have been
	 * allocated for alignment constraints
	 */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_GEN_PRIV_STACKS
	ਢੇਰ_obj_ਮਾਪ = ਢੇਰ_object->ਅੰਕੜੇ.ਢੇਰ_ਅੰਕੜੇ->ਮਾਪ;
#ਨਹੀਂਤਾਂ
	ਢੇਰ_obj_ਮਾਪ = ਢੇਰ_object->ਅੰਕੜੇ.ਢੇਰ_ਮਾਪ;
#ਜੇ_ਅੰਤ /* CONFIG_GEN_PRIV_STACKS */
	K_OOPS(K_SYSCALL_VERIFY_ਸੁਨੇਹਾ(ਕੁੱਲ_ਮਾਪ <= ਢੇਰ_obj_ਮਾਪ,
				    "stack size %zu is too big, max is %zu",
				    ਕੁੱਲ_ਮਾਪ, ਢੇਰ_obj_ਮਾਪ));

	/* User threads may only create other user threads and they can't
	 * be marked as essential
	 */
	K_OOPS(K_SYSCALL_VERIFY(options & K_USER));
	K_OOPS(K_SYSCALL_VERIFY(!(options & K_ESSENTIAL)));

	/* Check validity of prio argument; must be the same or worse priority
	 * than the caller
	 */
	K_OOPS(K_SYSCALL_VERIFY(_is_valid_prio(prio, ਨਿਰਾ)));
	K_OOPS(K_SYSCALL_VERIFY(z_is_prio_ਹੇਠਲਾ_or_equal(prio,
							_current->ਅਧਾਰ.prio)));

	z_ਤਿਆਰੀ_new_thread(new_thread, ਢੇਰ, ਢੇਰ_ਮਾਪ,
			   ਇੰਦਰਾਜ, p1, p2, p3, prio, options, ਨਿਰਾ);

	thread_ਸਮਾਂ_ਸਾਰਣੀ_new(new_thread, delay);

	ਵਾਪਸ new_thread;
}
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/syscalls/k_thread_create_mrsh.c>
#ਜੇ_ਅੰਤ /* CONFIG_USERSPACE */

ਖਾਲੀ z_init_thread_ਅਧਾਰ(ਬਣਤਰ thread_ਅਧਾਰ *thread_ਅਧਾਰ_2, ਪੂਰਨ ਤਰਜੀਹ,
		       uint32_t initial_state, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ options)
{
	/* k_q_node is initialized upon first insertion in a list */
	thread_ਅਧਾਰ_2->pended_on = ਨਿਰਾ;
	thread_ਅਧਾਰ_2->user_options = (uint16_t)options;
	thread_ਅਧਾਰ_2->thread_state = (uint8_t)initial_state;

	thread_ਅਧਾਰ_2->prio = ਤਰਜੀਹ;

	thread_ਅਧਾਰ_2->sched_locked = 0U;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SMP
	thread_ਅਧਾਰ_2->is_idle = 0;

	/*
	 * Pretend that the thread was last executing on CPU0 to prevent
	 * out-of-bounds memory accesses to the _kernel.cpus[] array.
	 */
	thread_ਅਧਾਰ_2->cpu = 0;
#ਜੇ_ਅੰਤ /* CONFIG_SMP */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_TIMESLICE_ਪ੍ਰਤੀ_THREAD
	thread_ਅਧਾਰ_2->ਟੁਕੜਾ_ticks = 0;
	thread_ਅਧਾਰ_2->ਟੁਕੜਾ_expired = ਨਿਰਾ;
#ਜੇ_ਅੰਤ /* CONFIG_TIMESLICE_PER_THREAD */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SPIN_VALIDATE
	thread_ਅਧਾਰ_2->ਅਦਲਾ_ਬਦਲੀ_ਅੰਕੜੇ = ਨਿਰਾ;
	/* otherwise swap_data does not need to be initialized */
#ਜੇ_ਅੰਤ

	z_init_thread_timeout(thread_ਅਧਾਰ_2);
}

FUNC_NORETURN ਖਾਲੀ k_thread_user_ਬਹੁਲਕ_enter(k_thread_ਇੰਦਰਾਜ_t ਇੰਦਰਾਜ,
					    ਖਾਲੀ *p1, ਖਾਲੀ *p2, ਖਾਲੀ *p3)
{
	SYS_ਪੋਰਟ_TRACING_FUNC(k_thread, user_ਬਹੁਲਕ_enter);

	_current->ਅਧਾਰ.user_options |= K_USER;
	z_thread_essential_clear(_current);
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_THREAD_MONITOR
	_current->ਇੰਦਰਾਜ.p_ਇੰਦਰਾਜ = ਇੰਦਰਾਜ;
	_current->ਇੰਦਰਾਜ.parameter1 = p1;
	_current->ਇੰਦਰਾਜ.parameter2 = p2;
	_current->ਇੰਦਰਾਜ.parameter3 = p3;
#ਜੇ_ਅੰਤ /* CONFIG_THREAD_MONITOR */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_USERSPACE
	__ASSERT(z_ਢੇਰ_is_user_capable(_current->ਢੇਰ_obj),
		 "dropping to user mode with kernel-only stack object");
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_THREAD_USERSPACE_LOCAL_ਅੰਕੜੇ
	memset(_current->userspace_local_ਅੰਕੜੇ, 0,
	       ਆਕਾਰ(ਬਣਤਰ thread_userspace_local_ਅੰਕੜੇ));
#ਜੇ_ਅੰਤ /* CONFIG_THREAD_USERSPACE_LOCAL_DATA */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_THREAD_LOCAL_STORAGE
	arch_tls_ਢੇਰ_ਤਿਆਰੀ(_current,
			     (ਅੱਖਰ *)(_current->ਢੇਰ_info.ਸ਼ੁਰੂ +
				      _current->ਢੇਰ_info.ਮਾਪ));
#ਜੇ_ਅੰਤ /* CONFIG_THREAD_LOCAL_STORAGE */
	arch_user_ਬਹੁਲਕ_enter(ਇੰਦਰਾਜ, p1, p2, p3);
#ਨਹੀਂਤਾਂ
	/* XXX In this case we do not reset the stack */
	z_thread_ਇੰਦਰਾਜ(ਇੰਦਰਾਜ, p1, p2, p3);
#ਜੇ_ਅੰਤ /* CONFIG_USERSPACE */
}

#ਜੇ defined(CONFIG_INIT_STACKS) && defined(CONFIG_THREAD_ਢੇਰ_INFO)
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_ਢੇਰ_GROWS_ਉੱਪਰ
#ਗਲਤੀ "Unsupported configuration for stack analysis"
#ਜੇ_ਅੰਤ /* CONFIG_STACK_GROWS_UP */

ਪੂਰਨ z_ਢੇਰ_space_get(ਅਚਲ uint8_t *ਢੇਰ_ਸ਼ੁਰੂ, size_t ਮਾਪ, size_t *ਅਣਵਰਤਿਆ_ptr)
{
	size_t ਅਣਵਰਤਿਆ = 0;
	ਅਚਲ uint8_t *checked_ਢੇਰ = ਢੇਰ_ਸ਼ੁਰੂ;
	/* Take the address of any local variable as a shallow bound for the
	 * stack pointer.  Addresses above it are guaranteed to be
	 * accessible.
	 */
	ਅਚਲ uint8_t *ਢੇਰ_ਸੂਚਕ = (ਅਚਲ uint8_t *)&ਢੇਰ_ਸ਼ੁਰੂ;

	/* If we are currently running on the stack being analyzed, some
	 * memory management hardware will generate an exception if we
	 * read unused stack memory.
	 *
	 * This never happens when invoked from user mode, as user mode
	 * will always run this function on the privilege elevation stack.
	 */
	ਜੇ ((ਢੇਰ_ਸੂਚਕ > ਢੇਰ_ਸ਼ੁਰੂ) && (ਢੇਰ_ਸੂਚਕ <= (ਢੇਰ_ਸ਼ੁਰੂ + ਮਾਪ)) &&
	    IS_ENABLED(CONFIG_NO_ਅਣਵਰਤਿਆ_ਢੇਰ_INSPECTION)) {
		/* TODO: We could add an arch_ API call to temporarily
		 * disable the stack checking in the CPU, but this would
		 * need to be properly managed wrt context switches/interrupts
		 */
		ਵਾਪਸ -ENOTSUP;
	}

	ਜੇ (IS_ENABLED(CONFIG_ਢੇਰ_SENTINEL)) {
		/* First 4 bytes of the stack buffer reserved for the
		 * sentinel value, it won't be 0xAAAAAAAA for thread
		 * stacks.
		 *
		 * FIXME: thread->stack_info.start ought to reflect
		 * this!
		 */
		checked_ਢੇਰ += 4;
		ਮਾਪ -= 4;
	}

	ਲਈ (size_t i = 0; i < ਮਾਪ; i++) {
		ਜੇ ((checked_ਢੇਰ[i]) == 0xaaU) {
			ਅਣਵਰਤਿਆ++;
		} ਨਹੀਂਤਾਂ {
			ਰੋਕੋ;
		}
	}

	*ਅਣਵਰਤਿਆ_ptr = ਅਣਵਰਤਿਆ;

	ਵਾਪਸ 0;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_THREAD_RUNTIME_ਢੇਰ_SAFETY
ਪੂਰਨ k_thread_runtime_ਢੇਰ_safety_ਭਰਿਆ_check(ਅਚਲ ਬਣਤਰ k_thread *thread,
					     size_t *ਅਣਵਰਤਿਆ_ptr,
					     k_thread_ਢੇਰ_safety_handler_t handler,
					     ਖਾਲੀ *arg)
{
	ਪੂਰਨ    rv;
	size_t ਅਣਵਰਤਿਆ_space;

	__ASSERT_NO_MSG(thread != ਨਿਰਾ);

	rv = z_ਢੇਰ_space_get((ਅਚਲ uint8_t *)thread->ਢੇਰ_info.ਸ਼ੁਰੂ,
			       thread->ਢੇਰ_info.ਮਾਪ, &ਅਣਵਰਤਿਆ_space);

	ਜੇ (rv != 0) {
		ਵਾਪਸ rv;
	}

	ਜੇ (ਅਣਵਰਤਿਆ_ptr != ਨਿਰਾ) {
		*ਅਣਵਰਤਿਆ_ptr = ਅਣਵਰਤਿਆ_space;
	}

	ਜੇ ((ਅਣਵਰਤਿਆ_space < thread->ਢੇਰ_info.usage.ਅਣਵਰਤਿਆ_ਹੱਦ) &&
	    (handler != ਨਿਰਾ)) {
		handler(thread, ਅਣਵਰਤਿਆ_space, arg);
	}

	ਵਾਪਸ 0;
}

ਪੂਰਨ k_thread_runtime_ਢੇਰ_safety_ਹੱਦ_check(ਅਚਲ ਬਣਤਰ k_thread *thread,
						  size_t *ਅਣਵਰਤਿਆ_ptr,
						  k_thread_ਢੇਰ_safety_handler_t handler,
						  ਖਾਲੀ *arg)
{
	ਪੂਰਨ    rv;
	size_t ਅਣਵਰਤਿਆ_space;
	size_t ਹੱਦ;
	size_t ਜਾਂਚ_ਮਾਪ;

	__ASSERT_NO_MSG(thread != ਨਿਰਾ);

	ਹੱਦ = thread->ਢੇਰ_info.usage.ਅਣਵਰਤਿਆ_ਹੱਦ;
	ਜਾਂਚ_ਮਾਪ = ਹੱਦ;

	/* z_stack_space_get() reserves the first bytes of the stack buffer for
	 * the stack sentinel and shrinks its scan window by that amount. Grow
	 * the requested window by the same reservation so that up to
	 * 'threshold' bytes of the usable stack are actually examined;
	 * otherwise a fully unused window would report fewer than 'threshold'
	 * unused bytes and spuriously trip the threshold.
	 */
	ਜੇ (IS_ENABLED(CONFIG_ਢੇਰ_SENTINEL) && (ਹੱਦ != 0U)) {
		ਜਾਂਚ_ਮਾਪ += ਆਕਾਰ(uint32_t);
	}

	rv = z_ਢੇਰ_space_get((ਅਚਲ uint8_t *)thread->ਢੇਰ_info.ਸ਼ੁਰੂ,
			       ਜਾਂਚ_ਮਾਪ, &ਅਣਵਰਤਿਆ_space);

	ਜੇ (rv != 0) {
		ਵਾਪਸ rv;
	}

	ਜੇ (ਅਣਵਰਤਿਆ_ptr != ਨਿਰਾ) {
		*ਅਣਵਰਤਿਆ_ptr = ਅਣਵਰਤਿਆ_space;
	}

	ਜੇ ((ਅਣਵਰਤਿਆ_space < ਹੱਦ) && (handler != ਨਿਰਾ)) {
		handler(thread, ਅਣਵਰਤਿਆ_space, arg);
	}

	ਵਾਪਸ 0;
}
#ਜੇ_ਅੰਤ /* CONFIG_THREAD_RUNTIME_STACK_SAFETY */

ਪੂਰਨ z_impl_k_thread_ਢੇਰ_space_get(ਅਚਲ ਬਣਤਰ k_thread *thread,
				    size_t *ਅਣਵਰਤਿਆ_ptr)
{
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_THREAD_ਢੇਰ_MEM_MAPPED
	ਜੇ (thread->ਢੇਰ_info.mapped.addr == ਨਿਰਾ) {
		ਵਾਪਸ -EINVAL;
	}
#ਜੇ_ਅੰਤ /* CONFIG_THREAD_STACK_MEM_MAPPED */

	ਵਾਪਸ z_ਢੇਰ_space_get((ਅਚਲ uint8_t *)thread->ਢੇਰ_info.ਸ਼ੁਰੂ,
				 thread->ਢੇਰ_info.ਮਾਪ, ਅਣਵਰਤਿਆ_ptr);
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_USERSPACE
ਪੂਰਨ z_vrfy_k_thread_ਢੇਰ_space_get(ਅਚਲ ਬਣਤਰ k_thread *thread,
				    size_t *ਅਣਵਰਤਿਆ_ptr)
{
	size_t ਅਣਵਰਤਿਆ;
	ਪੂਰਨ ret;

	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));

	ret = z_impl_k_thread_ਢੇਰ_space_get(thread, &ਅਣਵਰਤਿਆ);
	CHECKIF(ret != 0) {
		ਵਾਪਸ ret;
	}

	ret = k_usermode_ਮੰਜ਼ਿਲ_copy(ਅਣਵਰਤਿਆ_ptr, &ਅਣਵਰਤਿਆ, ਆਕਾਰ(size_t));
	CHECKIF(ret != 0) {
		ਵਾਪਸ ret;
	}

	ਵਾਪਸ 0;
}
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/syscalls/k_thread_stack_space_get_mrsh.c>
#ਜੇ_ਅੰਤ /* CONFIG_USERSPACE */
#ਜੇ_ਅੰਤ /* CONFIG_INIT_STACKS && CONFIG_THREAD_STACK_INFO */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_USERSPACE
ਸਥਿਰ ਸਤਰਅੰਦਰ k_ticks_t z_vrfy_k_thread_timeout_ਬਚਿਆ_ticks(
						    ਅਚਲ ਬਣਤਰ k_thread *thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	ਵਾਪਸ z_impl_k_thread_timeout_ਬਚਿਆ_ticks(thread);
}
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/syscalls/k_thread_timeout_remaining_ticks_mrsh.c>

ਸਥਿਰ ਸਤਰਅੰਦਰ k_ticks_t z_vrfy_k_thread_timeout_expires_ticks(
						  ਅਚਲ ਬਣਤਰ k_thread *thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	ਵਾਪਸ z_impl_k_thread_timeout_expires_ticks(thread);
}
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/syscalls/k_thread_timeout_expires_ticks_mrsh.c>
#ਜੇ_ਅੰਤ /* CONFIG_USERSPACE */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_INSTRUMENT_THREAD_SWITCHING
ਖਾਲੀ z_thread_mark_switched_ਵਿੱਚ(ਖਾਲੀ)
{
#ਜੇ defined(CONFIG_SCHED_THREAD_USAGE) && !defined(CONFIG_USE_SWITCH)
	z_sched_usage_ਸ਼ੁਰੂ(_current);
#ਜੇ_ਅੰਤ /* CONFIG_SCHED_THREAD_USAGE && !CONFIG_USE_SWITCH */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_TRACING
	SYS_ਪੋਰਟ_TRACING_FUNC(k_thread, switched_ਵਿੱਚ);
#ਜੇ_ਅੰਤ /* CONFIG_TRACING */
}

ਖਾਲੀ z_thread_mark_switched_ਬਾਹਰਲਾ(ਖਾਲੀ)
{
#ਜੇ defined(CONFIG_SCHED_THREAD_USAGE) && !defined(CONFIG_USE_SWITCH)
	z_sched_usage_stop();
#ਜੇ_ਅੰਤ /*CONFIG_SCHED_THREAD_USAGE && !CONFIG_USE_SWITCH */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_TRACING
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_THREAD_LOCAL_STORAGE
	/* Dummy thread won't have TLS set up to run arbitrary code */
	ਜੇ (!_current ||
	    (_current->ਅਧਾਰ.thread_state & _THREAD_DUMMY) != 0) {
		ਵਾਪਸ;
	}
#ਜੇ_ਅੰਤ /* CONFIG_THREAD_LOCAL_STORAGE */
	SYS_ਪੋਰਟ_TRACING_FUNC(k_thread, switched_ਬਾਹਰਲਾ);
#ਜੇ_ਅੰਤ /* CONFIG_TRACING */
}
#ਜੇ_ਅੰਤ /* CONFIG_INSTRUMENT_THREAD_SWITCHING */

ਪੂਰਨ k_thread_runtime_stats_get(k_tid_t thread,
			       k_thread_runtime_stats_t *stats)
{
	ਜੇ ((thread == ਨਿਰਾ) || (stats == ਨਿਰਾ)) {
		ਵਾਪਸ -EINVAL;
	}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SCHED_THREAD_USAGE
	z_sched_thread_usage(thread, stats);
#ਨਹੀਂਤਾਂ
	*stats = (k_thread_runtime_stats_t) {};
#ਜੇ_ਅੰਤ /* CONFIG_SCHED_THREAD_USAGE */

	ਵਾਪਸ 0;
}

ਪੂਰਨ k_thread_runtime_stats_all_get(k_thread_runtime_stats_t *stats)
{
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SCHED_THREAD_USAGE_ALL
	k_thread_runtime_stats_t  tmp_stats;
#ਜੇ_ਅੰਤ /* CONFIG_SCHED_THREAD_USAGE_ALL */

	ਜੇ (stats == ਨਿਰਾ) {
		ਵਾਪਸ -EINVAL;
	}

	*stats = (k_thread_runtime_stats_t) {};

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SCHED_THREAD_USAGE_ALL
	/* Retrieve the usage stats for each core and amalgamate them. */

	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਅੰਕ_cpus = arch_ਅੰਕ_cpus();

	ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ i = 0; i < ਅੰਕ_cpus; i++) {
		z_sched_cpu_usage(i, &tmp_stats);

		stats->execution_ਚੱਕਰਾਂ += tmp_stats.execution_ਚੱਕਰਾਂ;
		stats->ਕੁੱਲ_ਚੱਕਰਾਂ     += tmp_stats.ਕੁੱਲ_ਚੱਕਰਾਂ;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SCHED_THREAD_USAGE_ANALYSIS
		stats->current_ਚੱਕਰਾਂ   += tmp_stats.current_ਚੱਕਰਾਂ;
		stats->peak_ਚੱਕਰਾਂ      += tmp_stats.peak_ਚੱਕਰਾਂ;
		stats->ਔਸਤਨ_ਚੱਕਰਾਂ   += tmp_stats.ਔਸਤਨ_ਚੱਕਰਾਂ;
#ਜੇ_ਅੰਤ /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */
		stats->idle_ਚੱਕਰਾਂ      += tmp_stats.idle_ਚੱਕਰਾਂ;
	}
#ਜੇ_ਅੰਤ /* CONFIG_SCHED_THREAD_USAGE_ALL */

	ਵਾਪਸ 0;
}

ਪੂਰਨ k_thread_runtime_stats_cpu_get(ਪੂਰਨ cpu, k_thread_runtime_stats_t *stats)
{
	ਜੇ (stats == ਨਿਰਾ) {
		ਵਾਪਸ -EINVAL;
	}

	*stats = (k_thread_runtime_stats_t) {};

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SCHED_THREAD_USAGE_ALL
	CHECKIF((cpu < 0) || ((ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ)cpu >= arch_ਅੰਕ_cpus())) {
		ਵਾਪਸ -EINVAL;
	}

	z_sched_cpu_usage(cpu, stats);
#ਨਹੀਂਤਾਂ
	ARG_ਅਣਵਰਤਿਆ(cpu);
#ਜੇ_ਅੰਤ

	ਵਾਪਸ 0;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_THREAD_ABORT_NEED_CLEANUP
/** Pointer to thread which needs to be cleaned up. */
ਸਥਿਰ ਬਣਤਰ k_thread *thread_ਮੰਜ਼ਿਲ_cleanup;

/** Spinlock for thread abort cleanup. */
ਸਥਿਰ ਬਣਤਰ k_spinlock thread_cleanup_ਤਾਲਾ;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_THREAD_ਢੇਰ_MEM_MAPPED
ਸਥਿਰ ਖਾਲੀ *thread_cleanup_ਢੇਰ_addr;
ਸਥਿਰ size_t thread_cleanup_ਢੇਰ_sz;

ਸਥਿਰ ਖਾਲੀ unmap_thread_ਢੇਰ(ਖਾਲੀ *addr, size_t sz)
{
	ਜੇ (addr != ਨਿਰਾ) {
		k_mem_unmap_phys_guard(addr, sz, false);
	}
}
#ਜੇ_ਅੰਤ /* CONFIG_THREAD_STACK_MEM_MAPPED */

ਖਾਲੀ defer_thread_cleanup(ਬਣਤਰ k_thread *thread)
{
	/* Note when adding new deferred cleanup steps:
	 * - The thread object may have been overwritten by the time
	 *   the actual cleanup is being done (e.g. thread object
	 *   allocated on a stack). So stash any necessary data here
	 *   that will be used in the actual cleanup steps.
	 */
	thread_ਮੰਜ਼ਿਲ_cleanup = thread;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_THREAD_ਢੇਰ_MEM_MAPPED
	/* Note that the permission of the stack should have been
	 * stripped of user thread access due to the thread having
	 * already exited from a memory domain. That is done via
	 * k_thread_abort().
	 */

	/* Stash the address and size so the region can be unmapped
	 * later.
	 */
	thread_cleanup_ਢੇਰ_addr = thread->ਢੇਰ_info.mapped.addr;
	thread_cleanup_ਢੇਰ_sz = thread->ਢੇਰ_info.mapped.sz;

	/* The stack is now considered un-usable. This should prevent any functions
	 * from looking directly into the mapped stack if they are made to be aware
	 * of memory mapped stacks, e.g., z_stack_space_get().
	 */
	thread->ਢੇਰ_info.mapped.addr = ਨਿਰਾ;
	thread->ਢੇਰ_info.mapped.sz = 0;
#ਜੇ_ਅੰਤ /* CONFIG_THREAD_STACK_MEM_MAPPED */
}

ਖਾਲੀ do_deferred_thread_cleanup(ਖਾਲੀ)
{
	/* Note when adding new actual cleanup steps:
	 * - The thread object may have been overwritten when this is
	 *   called. So only data stashed by defer_thread_cleanup() can
	 *   be used here, never anything from the thread object.
	 */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_THREAD_ਢੇਰ_MEM_MAPPED
	unmap_thread_ਢੇਰ(thread_cleanup_ਢੇਰ_addr, thread_cleanup_ਢੇਰ_sz);

	thread_cleanup_ਢੇਰ_addr = ਨਿਰਾ;
	thread_cleanup_ਢੇਰ_sz = 0;
#ਜੇ_ਅੰਤ /* CONFIG_THREAD_STACK_MEM_MAPPED */
}

ਖਾਲੀ do_thread_cleanup(ਬਣਤਰ k_thread *thread)
{
	/* This is only for threads which are no longer running and are not
	 * the current thread, so the thread object is still valid here and
	 * the cleanup can be driven from it directly.
	 */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_THREAD_ਢੇਰ_MEM_MAPPED
	unmap_thread_ਢੇਰ(thread->ਢੇਰ_info.mapped.addr,
			   thread->ਢੇਰ_info.mapped.sz);

	/* The stack is now unmapped and thus un-usable. Clear the mapping
	 * info so nothing looks into the stack afterwards, and so that
	 * a repeated cleanup of the same thread object is a no-op,
	 * e.g., z_stack_space_get().
	 */
	thread->ਢੇਰ_info.mapped.addr = ਨਿਰਾ;
	thread->ਢੇਰ_info.mapped.sz = 0;
#ਨਹੀਂਤਾਂ /* CONFIG_THREAD_STACK_MEM_MAPPED */
	ARG_ਅਣਵਰਤਿਆ(thread);
#ਜੇ_ਅੰਤ /* CONFIG_THREAD_STACK_MEM_MAPPED */
}

ਖਾਲੀ k_thread_abort_cleanup(ਬਣਤਰ k_thread *thread)
{
	K_SPINLOCK(&thread_cleanup_ਤਾਲਾ) {
		ਜੇ (thread_ਮੰਜ਼ਿਲ_cleanup != ਨਿਰਾ) {
			/* Finish the pending one first. */
			do_deferred_thread_cleanup();
			thread_ਮੰਜ਼ਿਲ_cleanup = ਨਿਰਾ;
		}

		ਜੇ (thread == _current) {
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
		} ਨਹੀਂਤਾਂ {
			/* Not the current running thread, so we are safe to do
			 * cleanups.
			 */
			do_thread_cleanup(thread);
		}
	}
}

ਖਾਲੀ k_thread_abort_cleanup_check_reuse(ਬਣਤਰ k_thread *thread)
{
	K_SPINLOCK(&thread_cleanup_ਤਾਲਾ) {
		/* This is to guard reuse of the same thread object and make sure
		 * any pending cleanups of it needs to be finished before the thread
		 * object can be reused.
		 */
		ਜੇ (thread_ਮੰਜ਼ਿਲ_cleanup == thread) {
			do_deferred_thread_cleanup();
			thread_ਮੰਜ਼ਿਲ_cleanup = ਨਿਰਾ;
		}
	}
}

#ਜੇ_ਅੰਤ /* CONFIG_THREAD_ABORT_NEED_CLEANUP */

ਖਾਲੀ z_dummy_thread_init(ਬਣਤਰ k_thread *dummy_thread)
{
	dummy_thread->ਅਧਾਰ.thread_state = _THREAD_DUMMY;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SCHED_CPU_ਨਕਾਬ
	dummy_thread->ਅਧਾਰ.cpu_ਨਕਾਬ = -1;
#ਜੇ_ਅੰਤ /* CONFIG_SCHED_CPU_MASK */
	dummy_thread->ਅਧਾਰ.user_options = K_ESSENTIAL;
#ਜੇ Z_MUTEX_PI_ENABLED
	sys_slist_init(&dummy_thread->held_mutexes);
	dummy_thread->mutex_pended_on = ਨਿਰਾ;
	dummy_thread->orig_prio = dummy_thread->ਅਧਾਰ.prio;
#ਜੇ_ਅੰਤ /* Z_MUTEX_PI_ENABLED */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_THREAD_ਢੇਰ_INFO
	dummy_thread->ਢੇਰ_info.ਸ਼ੁਰੂ = 0U;
	dummy_thread->ਢੇਰ_info.ਮਾਪ = 0U;
#ਜੇ_ਅੰਤ /* CONFIG_THREAD_STACK_INFO */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_USERSPACE
	dummy_thread->mem_domain_info.mem_domain = &k_mem_domain_default;
#ਜੇ_ਅੰਤ /* CONFIG_USERSPACE */
#ਜੇ (K_ਢੇਰ_MEM_POOL_ਮਾਪ > 0)
	k_thread_system_pool_ਸੌਂਪੋ(dummy_thread);
#ਨਹੀਂਤਾਂ
	dummy_thread->resource_pool = ਨਿਰਾ;
#ਜੇ_ਅੰਤ /* K_HEAP_MEM_POOL_SIZE */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_USE_SWITCH
	dummy_thread->switch_handle = ਨਿਰਾ;
#ਜੇ_ਅੰਤ /* CONFIG_USE_SWITCH */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_TIMESLICE_ਪ੍ਰਤੀ_THREAD
	dummy_thread->ਅਧਾਰ.ਟੁਕੜਾ_ticks = 0;
#ਜੇ_ਅੰਤ /* CONFIG_TIMESLICE_PER_THREAD */

	z_current_thread_ਸੈੱਟ(dummy_thread);
}

/*
 * Thread lifecycle APIs: suspend, resume, priority, wakeup, abort, join.
 *
 * These APIs operate on thread objects and belong here in thread.c.  They
 * depend on a small set of scheduler internals (z_thread_halt,
 * z_sched_ready_locked, z_sched_add_to_waitq_locked) exposed via ksched.h.
 */

ਖਾਲੀ z_impl_k_thread_ਤਰਜੀਹ_ਸੈੱਟ(k_tid_t thread, ਪੂਰਨ prio)
{
	/*
	 * Use NULL, since we cannot know what the entry point is (we do not
	 * keep track of it) and idle cannot change its priority.
	 */
	Z_ASSERT_VALID_PRIO(prio, ਨਿਰਾ);

	bool need_sched = z_thread_prio_ਸੈੱਟ((ਬਣਤਰ k_thread *)thread, prio);

	ਜੇ ((need_sched) && (IS_ENABLED(CONFIG_SMP) ||
			     (_current->ਅਧਾਰ.sched_locked == 0U))) {
		z_reschedule_unlocked();
	}
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_USERSPACE
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ z_vrfy_k_thread_ਤਰਜੀਹ_ਸੈੱਟ(k_tid_t thread, ਪੂਰਨ prio)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	K_OOPS(K_SYSCALL_VERIFY_ਸੁਨੇਹਾ(_is_valid_prio(prio, ਨਿਰਾ),
				    "invalid thread priority %d", prio));
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ CONFIG_USERSPACE_THREAD_MAY_RAISE_ਤਰਜੀਹ
	K_OOPS(K_SYSCALL_VERIFY_ਸੁਨੇਹਾ((int8_t)prio >= thread->ਅਧਾਰ.prio,
				    "thread priority may only be downgraded (%d < %d)",
				    prio, thread->ਅਧਾਰ.prio));
#ਜੇ_ਅੰਤ /* CONFIG_USERSPACE_THREAD_MAY_RAISE_PRIORITY */
	z_impl_k_thread_ਤਰਜੀਹ_ਸੈੱਟ(thread, prio);
}
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/syscalls/k_thread_priority_set_mrsh.c>
#ਜੇ_ਅੰਤ /* CONFIG_USERSPACE */

ਖਾਲੀ z_impl_k_thread_suspend(k_tid_t thread)
{
	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_ENTER(k_thread, suspend, thread);

	/* Special case "suspend the current thread" as it doesn't
	 * need the async complexity below.
	 */
	ਜੇ (!IS_ENABLED(CONFIG_SMP) &&
	    likely((thread == _current) && !arch_is_ਵਿੱਚ_isr())) {
		z_thread_suspend_current(thread);
		ਵਾਪਸ;
	}

	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ = z_sched_spinlock_ਤਾਲਾ();

	ਜੇ (unlikely(z_is_thread_suspended(thread))) {

		/* The target thread is already suspended. Nothing to do. */

		z_sched_spinlock_unlock(ਕੁੰਜੀ);
		ਵਾਪਸ;
	}

	z_thread_halt(thread, ਕੁੰਜੀ, false);

	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_EXIT(k_thread, suspend, thread);
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_USERSPACE
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ z_vrfy_k_thread_suspend(k_tid_t thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	z_impl_k_thread_suspend(thread);
}
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/syscalls/k_thread_suspend_mrsh.c>
#ਜੇ_ਅੰਤ /* CONFIG_USERSPACE */

ਖਾਲੀ z_impl_k_thread_resume(k_tid_t thread)
{
	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_ENTER(k_thread, resume, thread);

	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ = z_sched_spinlock_ਤਾਲਾ();

	/* Do not try to resume a thread that was not suspended */
	ਜੇ (unlikely(!z_is_thread_suspended(thread))) {
		z_sched_spinlock_unlock(ਕੁੰਜੀ);
		ਵਾਪਸ;
	}

	z_mark_thread_as_not_suspended(thread);
	z_sched_ready_locked(thread);

	z_reschedule_locked(ਕੁੰਜੀ);

	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_EXIT(k_thread, resume, thread);
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_USERSPACE
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ z_vrfy_k_thread_resume(k_tid_t thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	z_impl_k_thread_resume(thread);
}
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/syscalls/k_thread_resume_mrsh.c>
#ਜੇ_ਅੰਤ /* CONFIG_USERSPACE */

ਖਾਲੀ z_impl_k_wakeup(k_tid_t thread)
{
	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC(k_thread, wakeup, thread);

	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ = z_sched_spinlock_ਤਾਲਾ();

	ਜੇ (z_is_thread_sleeping(thread)) {
		(ਖਾਲੀ)z_try_abort_thread_timeout(thread);
		z_mark_thread_as_not_sleeping(thread);
		z_sched_ready_locked(thread);
		z_reschedule_locked(ਕੁੰਜੀ);
	} ਨਹੀਂਤਾਂ {
		z_sched_spinlock_unlock(ਕੁੰਜੀ);
	}
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_USERSPACE
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ z_vrfy_k_wakeup(k_tid_t thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	z_impl_k_wakeup(thread);
}
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/syscalls/k_wakeup_mrsh.c>
#ਜੇ_ਅੰਤ /* CONFIG_USERSPACE */

ਖਾਲੀ z_thread_abort(ਬਣਤਰ k_thread *thread)
{
	bool essential = z_is_thread_essential(thread);
	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ = z_sched_spinlock_ਤਾਲਾ();

	ਜੇ (z_is_thread_dead(thread)) {
		z_sched_spinlock_unlock(ਕੁੰਜੀ);
		ਵਾਪਸ;
	}

	z_thread_halt(thread, ਕੁੰਜੀ, true);

	ਜੇ (essential) {
		__ASSERT(!essential, "aborted essential thread %p", thread);
		k_panic();
	}
}

#ਜੇ !defined(CONFIG_ARCH_HAS_THREAD_ABORT)
ਖਾਲੀ z_impl_k_thread_abort(k_tid_t thread)
{
	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_ENTER(k_thread, abort, thread);

	z_thread_abort(thread);

	__ASSERT_NO_MSG(z_is_thread_dead(thread));

	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_EXIT(k_thread, abort, thread);
}
#ਜੇ_ਅੰਤ /* !CONFIG_ARCH_HAS_THREAD_ABORT */

ਪੂਰਨ z_impl_k_thread_join(ਬਣਤਰ k_thread *thread, k_timeout_t timeout)
{
	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ = z_sched_spinlock_ਤਾਲਾ();
	ਪੂਰਨ ret;

	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_ENTER(k_thread, join, thread, timeout);

	ਜੇ (z_is_thread_dead(thread)) {
		z_sched_switch_spin(thread);
		ret = 0;
	} ਨਹੀਂਤਾਂ ਜੇ (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		ret = -EBUSY;
	} ਨਹੀਂਤਾਂ ਜੇ ((thread == _current) ||
		   (thread->ਅਧਾਰ.pended_on == &_current->join_ਕਤਾਰ)) {
		ret = -EDEADLK;
	} ਨਹੀਂਤਾਂ {
		__ASSERT(!arch_is_ਵਿੱਚ_isr(), "cannot join in ISR");
		z_sched_ਜੋੜੋ_ਮੰਜ਼ਿਲ_waitq_locked(_current, &thread->join_ਕਤਾਰ);
		z_ਜੋੜੋ_thread_timeout(_current, timeout);

		SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_BLOCKING(k_thread, join, thread, timeout);
		ret = z_ਅਦਲਾ_ਬਦਲੀ_locked(ਕੁੰਜੀ);
		SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_EXIT(k_thread, join, thread, timeout, ret);

		ਵਾਪਸ ret;
	}

	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_EXIT(k_thread, join, thread, timeout, ret);

	z_sched_spinlock_unlock(ਕੁੰਜੀ);
	ਵਾਪਸ ret;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_USERSPACE
/* Special case: don't oops if the thread is uninitialized.  This is because
 * the initialization bit does double-duty for thread objects; if false, means
 * the thread object is truly uninitialized, or the thread ran and exited for
 * some reason.
 *
 * Return true in this case indicating we should just do nothing and return
 * success to the caller.
 */
ਸਥਿਰ bool thread_obj_validate(ਬਣਤਰ k_thread *thread)
{
	ਬਣਤਰ k_object *ko = k_object_ਲੱਭੋ(thread);
	ਪੂਰਨ ret = k_object_validate(ko, K_OBJ_THREAD, _OBJ_INIT_TRUE);

	ਚੋਣ (ret) {
	ਹਾਲਤ 0:
		ਵਾਪਸ false;
	ਹਾਲਤ -EINVAL:
		ਵਾਪਸ true;
	ਮੂਲ:
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_LOG
		k_object_dump_error(ret, thread, ko, K_OBJ_THREAD);
#ਜੇ_ਅੰਤ /* CONFIG_LOG */
		/* ret is a non-zero error code here (the 0 and -EINVAL cases
		 * are handled above), so this branch must always oops. Passing
		 * ret as the "verify" expression would treat the failure code as
		 * success and fall through to CODE_UNREACHABLE; verify ret == 0
		 * so the oops is actually raised.
		 */
		K_OOPS(K_SYSCALL_VERIFY_ਸੁਨੇਹਾ(ret == 0, "access denied"));
	}
	ਕੋਡ_UNREACHABLE; /* LCOV_EXCL_LINE */
}

ਸਥਿਰ ਸਤਰਅੰਦਰ ਪੂਰਨ z_vrfy_k_thread_join(ਬਣਤਰ k_thread *thread,
				       k_timeout_t timeout)
{
	ਜੇ (thread_obj_validate(thread)) {
		ਵਾਪਸ 0;
	}

	ਵਾਪਸ z_impl_k_thread_join(thread, timeout);
}
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/syscalls/k_thread_join_mrsh.c>

ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ z_vrfy_k_thread_abort(k_tid_t thread)
{
	ਜੇ (thread_obj_validate(thread)) {
		ਵਾਪਸ;
	}

	K_OOPS(K_SYSCALL_VERIFY_ਸੁਨੇਹਾ(!z_is_thread_essential(thread),
				    "aborting essential thread %p", thread));

	z_impl_k_thread_abort((ਬਣਤਰ k_thread *)thread);
}
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/syscalls/k_thread_abort_mrsh.c>
#ਜੇ_ਅੰਤ /* CONFIG_USERSPACE */

bool k_can_yield(ਖਾਲੀ)
{
	ਵਾਪਸ !(k_is_pre_kernel() || k_is_ਵਿੱਚ_isr() || !arch_cpu_irqs_are_enabled() ||
		 z_is_idle_thread_object(_current));
}

ਖਾਲੀ z_impl_k_yield(ਖਾਲੀ)
{
	__ASSERT(!arch_is_ਵਿੱਚ_isr(), "");

	SYS_ਪੋਰਟ_TRACING_FUNC(k_thread, yield);

	z_sched_yield();
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_USERSPACE
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ z_vrfy_k_yield(ਖਾਲੀ)
{
	z_impl_k_yield();
}
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/syscalls/k_yield_mrsh.c>
#ਜੇ_ਅੰਤ /* CONFIG_USERSPACE */
