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

#ifdef CONFIG_OBJ_CORE_THREAD
#ifdef CONFIG_OBJ_CORE_STATS_THREAD
ಸ್ಥಿರ ಸಂರಚನಾ k_obj_core_stats_desc  thread_stats_desc = {
	.raw_size = ಪರಿಮಾಣ(ಸಂರಚನಾ k_cycle_stats),
	.query_size = ಪರಿಮಾಣ(ಸಂರಚನಾ k_thread_runtime_stats),
	.raw   = z_thread_stats_raw,
	.query = z_thread_stats_query,
	.reset = z_thread_stats_reset,
	.disable = z_thread_stats_disable,
	.enable  = z_thread_stats_enable,
};
#endif /* CONFIG_OBJ_CORE_STATS_THREAD */

/* Threads have no statically defined instances to walk; each thread links its
 * own object core as it is created (see z_setup_new_thread()). Register the
 * type only.
 */
#ifdef CONFIG_OBJ_CORE_STATS_THREAD
K_OBJ_TYPE_DEFINE_TYPE_ONLY(obj_type_thread, k_thread, K_OBJ_TYPE_THREAD_ID,
			    &thread_stats_desc);
#ಅನ್ಯಥಾ
K_OBJ_TYPE_DEFINE_TYPE_ONLY(obj_type_thread, k_thread, K_OBJ_TYPE_THREAD_ID,
			    ರಿಕ್ತ);
#endif /* CONFIG_OBJ_CORE_STATS_THREAD */
#endif /* CONFIG_OBJ_CORE_THREAD */


#define _FOREACH_STATIC_THREAD(thread_data)              \
	STRUCT_SECTION_FOREACH(_static_thread_data, thread_data)

bool k_is_in_isr(ಶೂನ್ಯ)
{
	ಪ್ರತ್ಯಾಗಮ arch_is_in_isr();
}
EXPORT_ಚಿಹ್ನೆ(k_is_in_isr);

#ifdef CONFIG_THREAD_CUSTOM_DATA
ಶೂನ್ಯ z_impl_k_thread_custom_data_set(ಶೂನ್ಯ *ಮೌಲ್ಯ)
{
	_current->custom_data = ಮೌಲ್ಯ;
}

#ifdef CONFIG_USERSPACE
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ z_vrfy_k_thread_custom_data_set(ಶೂನ್ಯ *data)
{
	z_impl_k_thread_custom_data_set(data);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_thread_custom_data_set_mrsh.c>
#endif /* CONFIG_USERSPACE */

ಶೂನ್ಯ *z_impl_k_thread_custom_data_get(ಶೂನ್ಯ)
{
	ಪ್ರತ್ಯಾಗಮ _current->custom_data;
}

#ifdef CONFIG_USERSPACE
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ *z_vrfy_k_thread_custom_data_get(ಶೂನ್ಯ)
{
	ಪ್ರತ್ಯಾಗಮ z_impl_k_thread_custom_data_get();
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_thread_custom_data_get_mrsh.c>

#endif /* CONFIG_USERSPACE */
#endif /* CONFIG_THREAD_CUSTOM_DATA */

ಪೂರ್ಣಾಙ್ಕ z_impl_k_is_preempt_thread(ಶೂನ್ಯ)
{
	ಪ್ರತ್ಯಾಗಮ !arch_is_in_isr() && thread_is_preemptible(_current);
}

#ifdef CONFIG_USERSPACE
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಪೂರ್ಣಾಙ್ಕ z_vrfy_k_is_preempt_thread(ಶೂನ್ಯ)
{
	ಪ್ರತ್ಯಾಗಮ z_impl_k_is_preempt_thread();
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_is_preempt_thread_mrsh.c>
#endif /* CONFIG_USERSPACE */

ಪೂರ್ಣಾಙ್ಕ z_impl_k_thread_priority_get(k_tid_t thread)
{
	ಪ್ರತ್ಯಾಗಮ thread->base.prio;
}

#ifdef CONFIG_USERSPACE
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಪೂರ್ಣಾಙ್ಕ z_vrfy_k_thread_priority_get(k_tid_t thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	ಪ್ರತ್ಯಾಗಮ z_impl_k_thread_priority_get(thread);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_thread_priority_get_mrsh.c>
#endif /* CONFIG_USERSPACE */

#ifdef CONFIG_THREAD_NAME
ಸ್ಥಿರ ಶೂನ್ಯ set_thread_name(ಸಂರಚನಾ k_thread *thread, ಧ್ರುವ ವರ್ಣ *str)
{
	ಯದಿ (str != ರಿಕ್ತ) {
		strncpy(thread->name, str, CONFIG_THREAD_MAX_NAME_LEN - 1);
		/* Ensure NULL termination, truncate if longer */
		thread->name[CONFIG_THREAD_MAX_NAME_LEN - 1] = '\0';
#ifdef CONFIG_ARCH_HAS_THREAD_NAME_HOOK
		arch_thread_name_set(thread, str);
#endif /* CONFIG_ARCH_HAS_THREAD_NAME_HOOK */
	} ಅನ್ಯಥಾ {
		thread->name[0] = '\0';
	}
}
#endif /* CONFIG_THREAD_NAME */

ಪೂರ್ಣಾಙ್ಕ z_impl_k_thread_name_set(k_tid_t thread, ಧ್ರುವ ವರ್ಣ *str)
{
#ifdef CONFIG_THREAD_NAME
	ಯದಿ (thread == ರಿಕ್ತ) {
		thread = _current;
	}

	set_thread_name(thread, str);

	SYS_PORT_TRACING_OBJ_FUNC(k_thread, name_set, thread, 0);

	ಪ್ರತ್ಯಾಗಮ 0;
#ಅನ್ಯಥಾ
	ARG_UNUSED(thread);
	ARG_UNUSED(str);

	SYS_PORT_TRACING_OBJ_FUNC(k_thread, name_set, thread, -ENOSYS);

	ಪ್ರತ್ಯಾಗಮ -ENOSYS;
#endif /* CONFIG_THREAD_NAME */
}

#ifdef CONFIG_USERSPACE
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಪೂರ್ಣಾಙ್ಕ z_vrfy_k_thread_name_set(k_tid_t thread, ಧ್ರುವ ವರ್ಣ *str)
{
#ifdef CONFIG_THREAD_NAME
	ವರ್ಣ name[CONFIG_THREAD_MAX_NAME_LEN];

	ಯದಿ (thread != ರಿಕ್ತ) {
		ಯದಿ (K_SYSCALL_OBJ(thread, K_OBJ_THREAD) != 0) {
			ಪ್ರತ್ಯಾಗಮ -EINVAL;
		}
	}

	/* In theory we could copy directly into thread->name, but
	 * the current z_vrfy / z_impl split does not provide a
	 * means of doing so.
	 */
	ಯದಿ (k_usermode_string_copy(name, str, ಪರಿಮಾಣ(name)) != 0) {
		ಪ್ರತ್ಯಾಗಮ -EFAULT;
	}

	ಪ್ರತ್ಯಾಗಮ z_impl_k_thread_name_set(thread, name);
#ಅನ್ಯಥಾ
	ಪ್ರತ್ಯಾಗಮ -ENOSYS;
#endif /* CONFIG_THREAD_NAME */
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_thread_name_set_mrsh.c>
#endif /* CONFIG_USERSPACE */

ಧ್ರುವ ವರ್ಣ *k_thread_name_get(k_tid_t thread)
{
#ifdef CONFIG_THREAD_NAME
	ಪ್ರತ್ಯಾಗಮ (ಧ್ರುವ ವರ್ಣ *)thread->name;
#ಅನ್ಯಥಾ
	ARG_UNUSED(thread);
	ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
#endif /* CONFIG_THREAD_NAME */
}

ಪೂರ್ಣಾಙ್ಕ z_impl_k_thread_name_copy(k_tid_t thread, ವರ್ಣ *buf, size_t size)
{
#ifdef CONFIG_THREAD_NAME
	strncpy(buf, thread->name, size);
	ಪ್ರತ್ಯಾಗಮ 0;
#ಅನ್ಯಥಾ
	ARG_UNUSED(thread);
	ARG_UNUSED(buf);
	ARG_UNUSED(size);
	ಪ್ರತ್ಯಾಗಮ -ENOSYS;
#endif /* CONFIG_THREAD_NAME */
}

ಸ್ಥಿರ size_t copy_bytes(ವರ್ಣ *dest, size_t dest_size, ಧ್ರುವ ವರ್ಣ *src, size_t src_size)
{
	size_t  bytes_to_copy;

	bytes_to_copy = min(dest_size, src_size);
	memcpy(dest, src, bytes_to_copy);

	ಪ್ರತ್ಯಾಗಮ bytes_to_copy;
}

ಧ್ರುವ ವರ್ಣ *k_thread_state_str(k_tid_t thread_id, ವರ್ಣ *buf, size_t buf_size)
{
	size_t      off = 0;
	uint8_t     bit;
	uint8_t     thread_state = thread_id->base.thread_state;
#define SS_ENT(s) { Z_STATE_STR_##s, _THREAD_##s, ಪರಿಮಾಣ(Z_STATE_STR_##s) - 1 }
	ಸ್ಥಿರ ಧ್ರುವ ಸಂರಚನಾ {
		ಧ್ರುವ ವರ್ಣ *str;
		uint16_t    bit;
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
#undef SS_ENT

	ಯದಿ ((buf == ರಿಕ್ತ) || (buf_size == 0)) {
		ಪ್ರತ್ಯಾಗಮ "";
	}

	buf_size--;   /* Reserve 1 byte for end-of-string character */

	/*
	 * Loop through each bit in the thread_state. Stop once all have
	 * been processed. If more than one thread_state bit is set, then
	 * separate the descriptive strings with a '+'.
	 */


	ಪ್ರತಿ (unsigned ಪೂರ್ಣಾಙ್ಕ index = 0; thread_state != 0; index++) {
		bit = state_string[index].bit;
		ಯದಿ ((thread_state & bit) == 0) {
			ಅನುವರ್ತ;
		}

		off += copy_bytes(buf + off, buf_size - off,
				  state_string[index].str,
				  state_string[index].len);

		thread_state &= ~bit;

		ಯದಿ (thread_state != 0) {
			off += copy_bytes(buf + off, buf_size - off, "+", 1);
		}
	}

	buf[off] = '\0';

	ಪ್ರತ್ಯಾಗಮ (ಧ್ರುವ ವರ್ಣ *)buf;
}

#ifdef CONFIG_USERSPACE
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಪೂರ್ಣಾಙ್ಕ z_vrfy_k_thread_name_copy(k_tid_t thread,
					    ವರ್ಣ *buf, size_t size)
{
#ifdef CONFIG_THREAD_NAME
	size_t len;
	ಸಂರಚನಾ k_object *ko = k_object_find(thread);

	/* Special case: we allow reading the names of initialized threads
	 * even if we don't have permission on them
	 */
	ಯದಿ ((ko == ರಿಕ್ತ) || (ko->type != K_OBJ_THREAD) ||
		((ko->flags & K_OBJ_FLAG_INITIALIZED) == 0)) {
		ಪ್ರತ್ಯಾಗಮ -EINVAL;
	}
	ಯದಿ (K_SYSCALL_MEMORY_WRITE(buf, size) != 0) {
		ಪ್ರತ್ಯಾಗಮ -EFAULT;
	}
	len = strlen(thread->name);
	ಯದಿ ((len + 1) > size) {
		ಪ್ರತ್ಯಾಗಮ -ENOSPC;
	}

	ಪ್ರತ್ಯಾಗಮ k_usermode_to_copy((ಶೂನ್ಯ *)buf, thread->name, len + 1);
#ಅನ್ಯಥಾ
	ARG_UNUSED(thread);
	ARG_UNUSED(buf);
	ARG_UNUSED(size);
	ಪ್ರತ್ಯಾಗಮ -ENOSYS;
#endif /* CONFIG_THREAD_NAME */
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_thread_name_copy_mrsh.c>
#endif /* CONFIG_USERSPACE */

#ifdef CONFIG_ರಾಶಿ_SENTINEL
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

	ಯದಿ ((_current->base.thread_state & _THREAD_DUMMY) != 0) {
		ಪ್ರತ್ಯಾಗಮ;
	}

	ರಾಶಿ = (uint32_t *)_current->ರಾಶಿ_info.start;
	ಯದಿ (*ರಾಶಿ != ರಾಶಿ_SENTINEL) {
		/* Restore it so further checks don't trigger this same error */
		*ರಾಶಿ = ರಾಶಿ_SENTINEL;
		z_except_reason(K_ERR_ರಾಶಿ_CHK_FAIL);
	}
}
#endif /* CONFIG_STACK_SENTINEL */

#ಯದಿ defined(CONFIG_ರಾಶಿ_POINTER_RANDOM) && (CONFIG_ರಾಶಿ_POINTER_RANDOM != 0)
ಪೂರ್ಣಾಙ್ಕ z_ರಾಶಿ_adjust_initialized;

ಸ್ಥಿರ size_t random_offset(size_t ರಾಶಿ_size)
{
	size_t random_val;

	ಯದಿ (!z_ರಾಶಿ_adjust_initialized) {
		z_early_rand_get((uint8_t *)&random_val, ಪರಿಮಾಣ(random_val));
	} ಅನ್ಯಥಾ {
		sys_rand_get((uint8_t *)&random_val, ಪರಿಮಾಣ(random_val));
	}

	/* Don't need to worry about alignment of the size here,
	 * arch_new_thread() is required to do it.
	 *
	 * FIXME: Not the best way to get a random number in a range.
	 * See #6493
	 */
	ಧ್ರುವ size_t fuzz = random_val % CONFIG_ರಾಶಿ_POINTER_RANDOM;

	ಯದಿ (unlikely(fuzz * 2 > ರಾಶಿ_size)) {
		ಪ್ರತ್ಯಾಗಮ 0;
	}

	ಪ್ರತ್ಯಾಗಮ fuzz;
}
#ಯದಿ defined(CONFIG_ರಾಶಿ_GROWS_UP)
	/* This is so rare not bothering for now */
#error "Stack pointer randomization not implemented for upward growing stacks"
#endif /* CONFIG_STACK_GROWS_UP */
#endif /* CONFIG_STACK_POINTER_RANDOM */

/*
 * Geometry of a thread's stack object, computed once and threaded through the
 * setup_thread_stack() helpers below.
 */
ಸಂರಚನಾ ರಾಶಿ_geometry {
	size_t obj_size;	/* size of the whole stack object */
	ವರ್ಣ *buf_start;	/* start of the usable stack buffer */
	size_t buf_size;	/* size of the usable stack buffer */
};

/*
 * Derive the stack object geometry from the caller-supplied stack and size.
 * User-capable stacks and kernel-only stacks reserve different amounts.
 */
ಸ್ಥಿರ ಸಂರಚನಾ ರಾಶಿ_geometry compute_ರಾಶಿ_geometry(k_thread_ರಾಶಿ_t *ರಾಶಿ,
						    size_t ರಾಶಿ_size)
{
	ಸಂರಚನಾ ರಾಶಿ_geometry geo;

#ifdef CONFIG_USERSPACE
	ಯದಿ (z_ರಾಶಿ_is_user_capable(ರಾಶಿ)) {
		geo.obj_size = K_THREAD_ರಾಶಿ_LEN(ರಾಶಿ_size);
		geo.buf_start = K_THREAD_ರಾಶಿ_BUFFER(ರಾಶಿ);
		geo.buf_size = geo.obj_size - K_THREAD_ರಾಶಿ_RESERVED;
	} ಅನ್ಯಥಾ
#endif /* CONFIG_USERSPACE */
	{
		/* Object cannot host a user mode thread */
		geo.obj_size = K_KERNEL_ರಾಶಿ_LEN(ರಾಶಿ_size);
		geo.buf_start = K_KERNEL_ರಾಶಿ_BUFFER(ರಾಶಿ);
		geo.buf_size = geo.obj_size - K_KERNEL_ರಾಶಿ_RESERVED;

#ಯದಿ defined(ARCH_KERNEL_ರಾಶಿ_RESERVED)
		/* Zephyr treats stack overflow as an app bug.  But
		 * this particular overflow can be seen by static
		 * analysis so needs to be handled somehow.
		 */
		ಯದಿ (K_KERNEL_ರಾಶಿ_RESERVED > geo.obj_size) {
			k_panic();
		}
#endif
	}

	ಪ್ರತ್ಯಾಗಮ geo;
}

/*
 * Return the initial (high end) stack pointer. When stacks are memory mapped
 * this maps the object into virtual memory, records the mapping so it can be
 * torn down at thread exit, and rebases buf_start onto the mapped address.
 */
ಸ್ಥಿರ ವರ್ಣ *map_thread_ರಾಶಿ(ಸಂರಚನಾ k_thread *new_thread,
			      k_thread_ರಾಶಿ_t *ರಾಶಿ,
			      ಸಂರಚನಾ ರಾಶಿ_geometry *geo)
{
	ವರ್ಣ *ರಾಶಿ_ptr;

#ifdef CONFIG_THREAD_ರಾಶಿ_MEM_MAPPED
	/* Map the stack into virtual memory and use that as the base to
	 * calculate the initial stack pointer at the high end of the stack
	 * object. The stack pointer may be reduced later by TLS or random
	 * offset.
	 *
	 * K_MEM_MAP_UNINIT is used to mimic the behavior of non-mapped
	 * stack. If CONFIG_INIT_STACKS is enabled, the stack will be
	 * cleared later.
	 */
	ಶೂನ್ಯ *ರಾಶಿ_mapped = k_mem_map_phys_guard((uintptr_t)ರಾಶಿ, geo->obj_size,
				K_MEM_PERM_RW | K_MEM_CACHE_WB | K_MEM_MAP_UNINIT,
				false);

	__ASSERT_NO_MSG((uintptr_t)ರಾಶಿ_mapped != 0);

#ifdef CONFIG_USERSPACE
	ಯದಿ (z_ರಾಶಿ_is_user_capable(ರಾಶಿ)) {
		geo->buf_start = K_THREAD_ರಾಶಿ_BUFFER(ರಾಶಿ_mapped);
	} ಅನ್ಯಥಾ
#endif /* CONFIG_USERSPACE */
	{
		geo->buf_start = K_KERNEL_ರಾಶಿ_BUFFER(ರಾಶಿ_mapped);
	}

	ರಾಶಿ_ptr = (ವರ್ಣ *)ರಾಶಿ_mapped + geo->obj_size;

	/* Need to store the info on mapped stack so we can remove the mappings
	 * when the thread ends.
	 */
	new_thread->ರಾಶಿ_info.mapped.addr = ರಾಶಿ_mapped;
	new_thread->ರಾಶಿ_info.mapped.sz = geo->obj_size;
#ಅನ್ಯಥಾ /* CONFIG_THREAD_STACK_MEM_MAPPED */
	ARG_UNUSED(new_thread);

	/* Initial stack pointer at the high end of the stack object, may
	 * be reduced later by TLS or random offset
	 */
	ರಾಶಿ_ptr = (ವರ್ಣ *)ರಾಶಿ + geo->obj_size;
#endif /* CONFIG_THREAD_STACK_MEM_MAPPED */

	ಪ್ರತ್ಯಾಗಮ ರಾಶಿ_ptr;
}

/* Pre-fill the stack buffer with a known pattern for usage measurement. */
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ fill_init_ರಾಶಿ(ವರ್ಣ *buf_start, size_t buf_size)
{
#ifdef CONFIG_INIT_STACKS
	memset(buf_start, 0xaa, buf_size);
#ಅನ್ಯಥಾ
	ARG_UNUSED(buf_start);
	ARG_UNUSED(buf_size);
#endif /* CONFIG_INIT_STACKS */
}

/* Place the overflow-detection sentinel at the low end of the stack buffer. */
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ set_ರಾಶಿ_sentinel(ವರ್ಣ *buf_start)
{
#ifdef CONFIG_ರಾಶಿ_SENTINEL
	/* Put the stack sentinel at the lowest 4 bytes of the stack area.
	 * We periodically check that it's still present and kill the thread
	 * if it isn't.
	 */
	*((uint32_t *)buf_start) = ರಾಶಿ_SENTINEL;
#ಅನ್ಯಥಾ
	ARG_UNUSED(buf_start);
#endif /* CONFIG_STACK_SENTINEL */
}

/*
 * Compute how much to reserve at the top of the stack buffer for TLS,
 * userspace-local data and stack-pointer randomization, and return the
 * aligned amount the initial stack pointer must be lowered by.
 */
ಸ್ಥಿರ size_t reserve_ರಾಶಿ_headroom(ಸಂರಚನಾ k_thread *new_thread,
				     ವರ್ಣ *ರಾಶಿ_ptr, size_t buf_size)
{
	size_t delta = 0;

	ARG_UNUSED(new_thread);
	ARG_UNUSED(ರಾಶಿ_ptr);
	ARG_UNUSED(buf_size);

#ifdef CONFIG_THREAD_LOCAL_STORAGE
	/* TLS is always last within the stack buffer */
	delta += arch_tls_ರಾಶಿ_setup(new_thread, ರಾಶಿ_ptr);
#endif /* CONFIG_THREAD_LOCAL_STORAGE */
#ifdef CONFIG_THREAD_USERSPACE_LOCAL_DATA
	size_t tls_size = ಪರಿಮಾಣ(ಸಂರಚನಾ _thread_userspace_local_data);

	/* reserve space on highest memory of stack buffer for local data */
	delta += tls_size;
	new_thread->userspace_local_data =
		(ಸಂರಚನಾ _thread_userspace_local_data *)(ರಾಶಿ_ptr - delta);
#endif /* CONFIG_THREAD_USERSPACE_LOCAL_DATA */
#ಯದಿ defined(CONFIG_ರಾಶಿ_POINTER_RANDOM) && (CONFIG_ರಾಶಿ_POINTER_RANDOM != 0)
	delta += random_offset(buf_size);
#endif /* CONFIG_STACK_POINTER_RANDOM */

	ಪ್ರತ್ಯಾಗಮ ROUND_UP(delta, ARCH_ರಾಶಿ_PTR_ALIGN);
}

/* Record the thread-accessible stack bounds (and runtime-safety threshold). */
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ set_ರಾಶಿ_info(ಸಂರಚನಾ k_thread *new_thread, ವರ್ಣ *buf_start,
				  size_t buf_size, size_t delta)
{
#ifdef CONFIG_THREAD_ರಾಶಿ_INFO
	/* Initial values. Arches which implement MPU guards that "borrow"
	 * memory from the stack buffer (not tracked in K_THREAD_STACK_RESERVED)
	 * will need to appropriately update this.
	 *
	 * The bounds tracked here correspond to the area of the stack object
	 * that the thread can access, which includes TLS.
	 */
	new_thread->ರಾಶಿ_info.start = (uintptr_t)buf_start;
	new_thread->ರಾಶಿ_info.size = buf_size;
	new_thread->ರಾಶಿ_info.delta = delta;

#ifdef CONFIG_THREAD_RUNTIME_ರಾಶಿ_SAFETY
	new_thread->ರಾಶಿ_info.usage.unused_threshold =
		(CONFIG_THREAD_RUNTIME_ರಾಶಿ_SAFETY_DEFAULT_UNUSED_THRESHOLD_PCT *
		 buf_size) / 100;
#endif
#ಅನ್ಯಥಾ
	ARG_UNUSED(new_thread);
	ARG_UNUSED(buf_start);
	ARG_UNUSED(buf_size);
	ARG_UNUSED(delta);
#endif /* CONFIG_THREAD_STACK_INFO */
}

ಸ್ಥಿರ ವರ್ಣ *setup_thread_ರಾಶಿ(ಸಂರಚನಾ k_thread *new_thread,
				k_thread_ರಾಶಿ_t *ರಾಶಿ, size_t ರಾಶಿ_size)
{
	ಸಂರಚನಾ ರಾಶಿ_geometry geo = compute_ರಾಶಿ_geometry(ರಾಶಿ, ರಾಶಿ_size);
	ವರ್ಣ *ರಾಶಿ_ptr = map_thread_ರಾಶಿ(new_thread, ರಾಶಿ, &geo);
	size_t delta;

	LOG_DBG("stack %p for thread %p: obj_size=%zu buf_start=%p "
		" buf_size %zu stack_ptr=%p",
		ರಾಶಿ, new_thread, geo.obj_size, (ಶೂನ್ಯ *)geo.buf_start,
		geo.buf_size, (ಶೂನ್ಯ *)ರಾಶಿ_ptr);

	fill_init_ರಾಶಿ(geo.buf_start, geo.buf_size);
	set_ರಾಶಿ_sentinel(geo.buf_start);
	delta = reserve_ರಾಶಿ_headroom(new_thread, ರಾಶಿ_ptr, geo.buf_size);
	set_ರಾಶಿ_info(new_thread, geo.buf_start, geo.buf_size, delta);

	ಪ್ರತ್ಯಾಗಮ ರಾಶಿ_ptr - delta;
}

#ifdef CONFIG_HW_SHADOW_ರಾಶಿ
ಸ್ಥಿರ ಶೂನ್ಯ setup_shadow_ರಾಶಿ(ಸಂರಚನಾ k_thread *new_thread,
			 k_thread_ರಾಶಿ_t *ರಾಶಿ)
{
	ಪೂರ್ಣಾಙ್ಕ ret = -ENOENT;

	STRUCT_SECTION_FOREACH(ರಾಶಿ_to_hw_shadow_ರಾಶಿ, stk_to_hw_shstk) {
		ಯದಿ (stk_to_hw_shstk->ರಾಶಿ == ರಾಶಿ) {
			ret = k_thread_hw_shadow_ರಾಶಿ_attach(new_thread,
							      stk_to_hw_shstk->shstk_addr,
							      stk_to_hw_shstk->size);
			ಯದಿ (ret != 0) {
				LOG_ERR("Could not set thread %p shadow stack %p, got error %d",
					new_thread, stk_to_hw_shstk->shstk_addr, ret);
				k_panic();
			}
			ಭಙ್ಗ;
		}
	}

	/* Check if the stack isn't in a stack array, and use the corresponding
	 * shadow stack.
	 */
	ಯದಿ (ret != -ENOENT) {
		ಪ್ರತ್ಯಾಗಮ;
	}

	STRUCT_SECTION_FOREACH(ರಾಶಿ_to_hw_shadow_ರಾಶಿ_arr, stk_to_hw_shstk) {
		ಯದಿ ((uintptr_t)ರಾಶಿ >= stk_to_hw_shstk->ರಾಶಿ_addr &&
		    (uintptr_t)ರಾಶಿ < stk_to_hw_shstk->ರಾಶಿ_addr +
		    stk_to_hw_shstk->ರಾಶಿ_size * stk_to_hw_shstk->nmemb) {
			/* Now we have to guess which index of the stack array is being used */
			uintptr_t ರಾಶಿ_offset = (uintptr_t)ರಾಶಿ - stk_to_hw_shstk->ರಾಶಿ_addr;
			uintptr_t ರಾಶಿ_index = ರಾಶಿ_offset / stk_to_hw_shstk->ರಾಶಿ_size;
			uintptr_t addr;

			ಯದಿ (ರಾಶಿ_index >= stk_to_hw_shstk->nmemb) {
				LOG_ERR("Could not find shadow stack for thread %p, stack %p",
					new_thread, ರಾಶಿ);
				k_panic();
			}

			addr = stk_to_hw_shstk->shstk_addr +
				stk_to_hw_shstk->shstk_size * ರಾಶಿ_index;
			ret = k_thread_hw_shadow_ರಾಶಿ_attach(new_thread,
							      (arch_thread_hw_shadow_ರಾಶಿ_t *)addr,
							      stk_to_hw_shstk->shstk_size);
			ಯದಿ (ret != 0) {
				LOG_ERR("Could not set thread %p shadow stack 0x%lx, got error %d",
					new_thread, stk_to_hw_shstk->shstk_addr, ret);
				k_panic();
			}
			ಭಙ್ಗ;
		}
	}

	ಯದಿ (ret == -ENOENT) {
		LOG_ERR("Could not find shadow stack for thread %p, stack %p",
			new_thread, ರಾಶಿ);
		k_panic();
	}
}
#ಅನ್ಯಥಾ
/* No-op stub used when hardware shadow stacks are not configured. */
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ setup_shadow_ರಾಶಿ(ಸಂರಚನಾ k_thread *new_thread,
				      k_thread_ರಾಶಿ_t *ರಾಶಿ)
{
	ARG_UNUSED(new_thread);
	ARG_UNUSED(ರಾಶಿ);
}
#endif

/*
 * The following init_thread_*()/assert_*() helpers each own one optional
 * kernel feature. Building z_setup_new_thread() out of these calls keeps its
 * body a readable, linear narrative instead of a wall of #ifdefs. Every helper
 * is static inline and collapses to nothing when its feature is disabled, so
 * this is purely a readability change with no runtime cost.
 */

/* Reset per-thread cleanup bookkeeping before a k_thread object is reused. */
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ thread_abort_cleanup_check_reuse(ಸಂರಚನಾ k_thread *thread)
{
#ifdef CONFIG_THREAD_ABORT_NEED_CLEANUP
	k_thread_abort_cleanup_check_reuse(thread);
#ಅನ್ಯಥಾ
	ARG_UNUSED(thread);
#endif /* CONFIG_THREAD_ABORT_NEED_CLEANUP */
}

/* Register the thread with the object-core framework and its usage stats. */
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ init_thread_obj_core(ಸಂರಚನಾ k_thread *thread)
{
#ifdef CONFIG_OBJ_CORE_THREAD
	k_obj_core_init_and_link(K_OBJ_CORE(thread), &obj_type_thread);
#ifdef CONFIG_OBJ_CORE_STATS_THREAD
	k_obj_core_stats_register(K_OBJ_CORE(thread), &thread->base.usage,
				  ಪರಿಮಾಣ(thread->base.usage));
#endif /* CONFIG_OBJ_CORE_STATS_THREAD */
#ಅನ್ಯಥಾ
	ARG_UNUSED(thread);
#endif /* CONFIG_OBJ_CORE_THREAD */
}

/*
 * Initialize userspace bookkeeping: register the thread and stack kernel
 * objects, record stack ownership, and grant the thread access to itself.
 */
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ init_thread_userspace(ಸಂರಚನಾ k_thread *thread,
					 k_thread_ರಾಶಿ_t *ರಾಶಿ,
					 uint32_t options)
{
#ifdef CONFIG_USERSPACE
	__ASSERT((options & K_USER) == 0U || z_ರಾಶಿ_is_user_capable(ರಾಶಿ),
		 "user thread %p with kernel-only stack %p", thread, ರಾಶಿ);
	k_object_init(thread);
	k_object_init(ರಾಶಿ);
	thread->ರಾಶಿ_obj = ರಾಶಿ;
	thread->syscall_frame = ರಿಕ್ತ;

	/* Any given thread has access to itself */
	k_object_access_grant(thread, thread);
#ಅನ್ಯಥಾ
	ARG_UNUSED(thread);
	ARG_UNUSED(ರಾಶಿ);
	ARG_UNUSED(options);
#endif /* CONFIG_USERSPACE */
}

/* Assert the thread object is cache-coherent while its stack stays cached. */
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ assert_thread_coherence(ಸಂರಚನಾ k_thread *thread,
					   k_thread_ರಾಶಿ_t *ರಾಶಿ)
{
#ifdef CONFIG_KERNEL_COHERENCE
	/* Check that the thread object is safe, but that the stack is
	 * still cached!
	 */
	__ASSERT_NO_MSG(sys_cache_is_mem_coherent(thread));

	/* When dynamic thread stack is available, the stack may come from
	 * uncached area.
	 */
#ifndef CONFIG_DYNAMIC_THREAD
	__ASSERT_NO_MSG(!sys_cache_is_mem_coherent(ರಾಶಿ));
#endif /* CONFIG_DYNAMIC_THREAD */
#endif /* CONFIG_KERNEL_COHERENCE */
	ARG_UNUSED(thread);
	ARG_UNUSED(ರಾಶಿ);
}

/* Assert the arch layer populated switch_handle on USE_SWITCH builds. */
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ assert_switch_handle(ಸಂರಚನಾ k_thread *thread)
{
#ifdef CONFIG_USE_SWITCH
	/* switch_handle must be non-null except when inside z_swap()
	 * for synchronization reasons.  Historically some notional
	 * USE_SWITCH architectures have actually ignored the field
	 */
	__ASSERT(thread->switch_handle != ರಿಕ್ತ,
		 "arch layer failed to initialize switch_handle");
#endif /* CONFIG_USE_SWITCH */
	ARG_UNUSED(thread);
}

/* Clear the opaque, kernel-managed per-thread custom data pointer. */
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ init_thread_custom_data(ಸಂರಚನಾ k_thread *thread)
{
#ifdef CONFIG_THREAD_CUSTOM_DATA
	/* Initialize custom data field (value is opaque to kernel) */
	thread->custom_data = ರಿಕ್ತ;
#ಅನ್ಯಥಾ
	ARG_UNUSED(thread);
#endif /* CONFIG_THREAD_CUSTOM_DATA */
}

/*
 * Record the entry point and parameters, then link the thread into the
 * global thread-monitor list under the monitor spinlock.
 */
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ add_thread_to_monitor(ಸಂರಚನಾ k_thread *thread,
					 k_thread_entry_t entry,
					 ಶೂನ್ಯ *p1, ಶೂನ್ಯ *p2, ಶೂನ್ಯ *p3)
{
#ifdef CONFIG_THREAD_MONITOR
	thread->entry.pEntry = entry;
	thread->entry.parameter1 = p1;
	thread->entry.parameter2 = p2;
	thread->entry.parameter3 = p3;

	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_lock(&z_thread_monitor_lock);

	thread->next_thread = _kernel.threads;
	_kernel.threads = thread;
	k_spin_unlock(&z_thread_monitor_lock, ಕೀಲಿ);
#ಅನ್ಯಥಾ
	ARG_UNUSED(thread);
	ARG_UNUSED(entry);
	ARG_UNUSED(p1);
	ARG_UNUSED(p2);
	ARG_UNUSED(p3);
#endif /* CONFIG_THREAD_MONITOR */
}

/* Set the thread's name from the caller-supplied string (may be NULL). */
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ init_thread_name(ಸಂರಚನಾ k_thread *thread, ಧ್ರುವ ವರ್ಣ *name)
{
#ifdef CONFIG_THREAD_NAME
	set_thread_name(thread, name);
#ಅನ್ಯಥಾ
	ARG_UNUSED(thread);
	ARG_UNUSED(name);
#endif /* CONFIG_THREAD_NAME */
}

/* Set the thread's initial SMP CPU affinity mask. */
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ init_thread_cpu_mask(ಸಂರಚನಾ k_thread *thread)
{
#ifdef CONFIG_SCHED_CPU_MASK
	ಯದಿ (IS_ENABLED(CONFIG_SCHED_CPU_MASK_PIN_ONLY)) {
		thread->base.cpu_mask = 1; /* must specify only one cpu */
	} ಅನ್ಯಥಾ {
		thread->base.cpu_mask = -1; /* allow all cpus */
	}
#ಅನ್ಯಥಾ
	ARG_UNUSED(thread);
#endif /* CONFIG_SCHED_CPU_MASK */
}

/* Attach the thread to its memory domain and inherit caller permissions
 * when K_INHERIT_PERMS is requested.
 */
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ init_thread_userspace_perms(ಸಂರಚನಾ k_thread *thread,
					       uint32_t options)
{
#ifdef CONFIG_USERSPACE
	z_mem_domain_init_thread(thread);

	ಯದಿ ((options & K_INHERIT_PERMS) != 0U) {
		k_thread_perms_inherit(_current, thread);
	}
#ಅನ್ಯಥಾ
	ARG_UNUSED(thread);
	ARG_UNUSED(options);
#endif /* CONFIG_USERSPACE */
}

/* Clear the EDF scheduling deadline. */
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ init_thread_deadline(ಸಂರಚನಾ k_thread *thread)
{
#ifdef CONFIG_SCHED_DEADLINE
	thread->base.prio_deadline = 0;
#ಅನ್ಯಥಾ
	ARG_UNUSED(thread);
#endif /* CONFIG_SCHED_DEADLINE */
}

/* Initialize the wait queue used to synchronize SMP thread halt/abort. */
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ init_thread_halt_queue(ಸಂರಚನಾ k_thread *thread)
{
#ifdef CONFIG_SMP
	z_waitq_init(&thread->halt_queue);
#ಅನ್ಯಥಾ
	ARG_UNUSED(thread);
#endif /* CONFIG_SMP */
}

/* Initialize runtime CPU-usage tracking for the thread. */
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ init_thread_usage(ಸಂರಚನಾ k_thread *thread)
{
#ifdef CONFIG_SCHED_THREAD_USAGE
	thread->base.usage = (ಸಂರಚನಾ k_cycle_stats) {};
	thread->base.usage.track_usage = CONFIG_SCHED_THREAD_USAGE_AUTO_ENABLE;
#ಅನ್ಯಥಾ
	ARG_UNUSED(thread);
#endif /* CONFIG_SCHED_THREAD_USAGE */
}

/* Assert the thread wasn't already linked into the monitor list. */
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ assert_thread_not_reused(ಸಂರಚನಾ k_thread *new_thread)
{
#ifdef CONFIG_THREAD_MONITOR
#ifdef CONFIG_ASSERT
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_lock(&z_thread_monitor_lock);
	bool reused = false;

	/* Check if the thread is already in the list */
	ಪ್ರತಿ (ಸಂರಚನಾ k_thread *t = _kernel.threads; t; t = t->next_thread) {
		ಯದಿ (t == new_thread) {
			reused = true;
			ಭಙ್ಗ;
		}
	}

	k_spin_unlock(&z_thread_monitor_lock, ಕೀಲಿ);

	__ASSERT(!reused, "thread %p is already in the running list", new_thread);
#endif /* CONFIG_ASSERT */
#endif /* CONFIG_THREAD_MONITOR */
	ARG_UNUSED(new_thread);
}

/*
 * The provided stack_size value is presumed to be either the result of
 * K_THREAD_STACK_SIZEOF(stack), or the size value passed to the instance
 * of K_THREAD_STACK_DEFINE() which defined 'stack'.
 */
ವರ್ಣ *z_setup_new_thread(ಸಂರಚನಾ k_thread *new_thread,
			 k_thread_ರಾಶಿ_t *ರಾಶಿ, size_t ರಾಶಿ_size,
			 k_thread_entry_t entry,
			 ಶೂನ್ಯ *p1, ಶೂನ್ಯ *p2, ಶೂನ್ಯ *p3,
			 ಪೂರ್ಣಾಙ್ಕ prio, uint32_t options, ಧ್ರುವ ವರ್ಣ *name)
{
	ವರ್ಣ *ರಾಶಿ_ptr;

	Z_ASSERT_VALID_PRIO(prio, entry);

	assert_thread_not_reused(new_thread);

	thread_abort_cleanup_check_reuse(new_thread);
	init_thread_obj_core(new_thread);
	init_thread_userspace(new_thread, ರಾಶಿ, options);

	z_waitq_init(&new_thread->join_queue);

	/* Initialize various struct k_thread members */
	z_init_thread_base(&new_thread->base, prio, _THREAD_SLEEPING, options);
	ರಾಶಿ_ptr = setup_thread_ರಾಶಿ(new_thread, ರಾಶಿ, ರಾಶಿ_size);

#ಯದಿ Z_MUTEX_PI_ENABLED
	sys_slist_init(&new_thread->held_mutexes);
	new_thread->mutex_pended_on = ರಿಕ್ತ;
	new_thread->orig_prio = new_thread->base.prio;
#endif /* Z_MUTEX_PI_ENABLED */

	setup_shadow_ರಾಶಿ(new_thread, ರಾಶಿ);
	assert_thread_coherence(new_thread, ರಾಶಿ);

	arch_new_thread(new_thread, ರಾಶಿ, ರಾಶಿ_ptr, entry, p1, p2, p3);

	/* static threads overwrite it afterwards with real value */
	new_thread->init_data = ರಿಕ್ತ;

	assert_switch_handle(new_thread);
	init_thread_custom_data(new_thread);
	add_thread_to_monitor(new_thread, entry, p1, p2, p3);
	init_thread_name(new_thread, name);
	init_thread_cpu_mask(new_thread);

	/* _current may be NULL if the dummy thread is not used */
	ಯದಿ (IS_ENABLED(CONFIG_ARCH_HAS_CUSTOM_SWAP_TO_MAIN) &&
	    (_current == ರಿಕ್ತ)) {
		new_thread->resource_pool = ರಿಕ್ತ;
		ಪ್ರತ್ಯಾಗಮ ರಾಶಿ_ptr;
	}

	init_thread_userspace_perms(new_thread, options);
	init_thread_deadline(new_thread);
	new_thread->resource_pool = _current->resource_pool;
	init_thread_halt_queue(new_thread);
	init_thread_usage(new_thread);

	SYS_PORT_TRACING_OBJ_FUNC(k_thread, create, new_thread);

	ಪ್ರತ್ಯಾಗಮ ರಾಶಿ_ptr;
}

#ifdef CONFIG_THREAD_RUNTIME_ರಾಶಿ_SAFETY
ಪೂರ್ಣಾಙ್ಕ z_impl_k_thread_runtime_ರಾಶಿ_unused_threshold_pct_set(ಸಂರಚನಾ k_thread *thread,
							   uint32_t pct)
{
	size_t unused_threshold;

	ಯದಿ (pct > 99) {
		ಪ್ರತ್ಯಾಗಮ -EINVAL;   /* 100% unused stack and up is invalid */
	}

	unused_threshold = (thread->ರಾಶಿ_info.size * pct) / 100;

	thread->ರಾಶಿ_info.usage.unused_threshold = unused_threshold;

	ಪ್ರತ್ಯಾಗಮ 0;
}

ಪೂರ್ಣಾಙ್ಕ z_impl_k_thread_runtime_ರಾಶಿ_unused_threshold_set(ಸಂರಚನಾ k_thread *thread,
						       size_t threshold)
{
	ಯದಿ (threshold > thread->ರಾಶಿ_info.size) {
		ಪ್ರತ್ಯಾಗಮ -EINVAL;
	}

	thread->ರಾಶಿ_info.usage.unused_threshold = threshold;

	ಪ್ರತ್ಯಾಗಮ 0;
}

size_t z_impl_k_thread_runtime_ರಾಶಿ_unused_threshold_get(ಸಂರಚನಾ k_thread *thread)
{
	ಪ್ರತ್ಯಾಗಮ thread->ರಾಶಿ_info.usage.unused_threshold;
}

#ifdef CONFIG_USERSPACE
ಪೂರ್ಣಾಙ್ಕ z_vrfy_k_thread_runtime_ರಾಶಿ_unused_threshold_pct_set(ಸಂರಚನಾ k_thread *thread,
							   uint32_t pct)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));

	ಪ್ರತ್ಯಾಗಮ z_impl_k_thread_runtime_ರಾಶಿ_unused_threshold_pct_set(thread, pct);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_thread_runtime_stack_unused_threshold_pct_set_mrsh.c>

ಪೂರ್ಣಾಙ್ಕ z_vrfy_k_thread_runtime_ರಾಶಿ_unused_threshold_set(ಸಂರಚನಾ k_thread *thread,
						       size_t threshold)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));

	ಪ್ರತ್ಯಾಗಮ z_impl_k_thread_runtime_ರಾಶಿ_unused_threshold_set(thread, threshold);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_thread_runtime_stack_unused_threshold_set_mrsh.c>

size_t z_vrfy_k_thread_runtime_ರಾಶಿ_unused_threshold_get(ಸಂರಚನಾ k_thread *thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));

	ಪ್ರತ್ಯಾಗಮ z_impl_k_thread_runtime_ರಾಶಿ_unused_threshold_get(thread);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_thread_runtime_stack_unused_threshold_get_mrsh.c>
#endif /* CONFIG_USERSPACE */
#endif /* CONFIG_THREAD_RUNTIME_STACK_SAFETY */

k_tid_t z_impl_k_thread_create(ಸಂರಚನಾ k_thread *new_thread,
			      k_thread_ರಾಶಿ_t *ರಾಶಿ,
			      size_t ರಾಶಿ_size, k_thread_entry_t entry,
			      ಶೂನ್ಯ *p1, ಶೂನ್ಯ *p2, ಶೂನ್ಯ *p3,
			      ಪೂರ್ಣಾಙ್ಕ prio, uint32_t options, k_timeout_t delay)
{
	__ASSERT(!arch_is_in_isr(), "Threads may not be created in ISRs");

	z_setup_new_thread(new_thread, ರಾಶಿ, ರಾಶಿ_size, entry, p1, p2, p3,
			  prio, options, ರಿಕ್ತ);

	thread_schedule_new(new_thread, delay);

	ಪ್ರತ್ಯಾಗಮ new_thread;
}

#ifdef CONFIG_USERSPACE
bool z_ರಾಶಿ_is_user_capable(k_thread_ರಾಶಿ_t *ರಾಶಿ)
{
	ಪ್ರತ್ಯಾಗಮ k_object_find(ರಾಶಿ) != ರಿಕ್ತ;
}

k_tid_t z_vrfy_k_thread_create(ಸಂರಚನಾ k_thread *new_thread,
			       k_thread_ರಾಶಿ_t *ರಾಶಿ,
			       size_t ರಾಶಿ_size, k_thread_entry_t entry,
			       ಶೂನ್ಯ *p1, ಶೂನ್ಯ *p2, ಶೂನ್ಯ *p3,
			       ಪೂರ್ಣಾಙ್ಕ prio, uint32_t options, k_timeout_t delay)
{
	size_t total_size, ರಾಶಿ_obj_size;
	ಸಂರಚನಾ k_object *ರಾಶಿ_object;

	/* The thread and stack objects *must* be in an uninitialized state */
	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(new_thread, K_OBJ_THREAD));

	/* No need to check z_stack_is_user_capable(), it won't be in the
	 * object table if it isn't
	 */
	ರಾಶಿ_object = k_object_find(ರಾಶಿ);
	K_OOPS(K_SYSCALL_VERIFY_ಸಂದೇಶ(k_object_validation_check(ರಾಶಿ_object, ರಾಶಿ,
						K_OBJ_THREAD_ರಾಶಿ_ELEMENT,
						_OBJ_INIT_FALSE) == 0,
				    "bad stack object"));

	/* Verify that the stack size passed in is OK by computing the total
	 * size and comparing it with the size value in the object metadata
	 */
	K_OOPS(K_SYSCALL_VERIFY_ಸಂದೇಶ(!size_add_overflow(K_THREAD_ರಾಶಿ_RESERVED,
						       ರಾಶಿ_size, &total_size),
				    "stack size overflow (%zu+%zu)",
				    ರಾಶಿ_size,
				    K_THREAD_ರಾಶಿ_RESERVED));

	/* Testing less-than-or-equal since additional room may have been
	 * allocated for alignment constraints
	 */
#ifdef CONFIG_GEN_PRIV_STACKS
	ರಾಶಿ_obj_size = ರಾಶಿ_object->data.ರಾಶಿ_data->size;
#ಅನ್ಯಥಾ
	ರಾಶಿ_obj_size = ರಾಶಿ_object->data.ರಾಶಿ_size;
#endif /* CONFIG_GEN_PRIV_STACKS */
	K_OOPS(K_SYSCALL_VERIFY_ಸಂದೇಶ(total_size <= ರಾಶಿ_obj_size,
				    "stack size %zu is too big, max is %zu",
				    total_size, ರಾಶಿ_obj_size));

	/* User threads may only create other user threads and they can't
	 * be marked as essential
	 */
	K_OOPS(K_SYSCALL_VERIFY(options & K_USER));
	K_OOPS(K_SYSCALL_VERIFY(!(options & K_ESSENTIAL)));

	/* Check validity of prio argument; must be the same or worse priority
	 * than the caller
	 */
	K_OOPS(K_SYSCALL_VERIFY(_is_valid_prio(prio, ರಿಕ್ತ)));
	K_OOPS(K_SYSCALL_VERIFY(z_is_prio_lower_or_equal(prio,
							_current->base.prio)));

	z_setup_new_thread(new_thread, ರಾಶಿ, ರಾಶಿ_size,
			   entry, p1, p2, p3, prio, options, ರಿಕ್ತ);

	thread_schedule_new(new_thread, delay);

	ಪ್ರತ್ಯಾಗಮ new_thread;
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_thread_create_mrsh.c>
#endif /* CONFIG_USERSPACE */

ಶೂನ್ಯ z_init_thread_base(ಸಂರಚನಾ _thread_base *thread_base, ಪೂರ್ಣಾಙ್ಕ priority,
		       uint32_t initial_state, unsigned ಪೂರ್ಣಾಙ್ಕ options)
{
	/* k_q_node is initialized upon first insertion in a list */
	thread_base->pended_on = ರಿಕ್ತ;
	thread_base->user_options = (uint16_t)options;
	thread_base->thread_state = (uint8_t)initial_state;

	thread_base->prio = priority;

	thread_base->sched_locked = 0U;

#ifdef CONFIG_SMP
	thread_base->is_idle = 0;

	/*
	 * Pretend that the thread was last executing on CPU0 to prevent
	 * out-of-bounds memory accesses to the _kernel.cpus[] array.
	 */
	thread_base->cpu = 0;
#endif /* CONFIG_SMP */

#ifdef CONFIG_TIMESLICE_PER_THREAD
	thread_base->slice_ticks = 0;
	thread_base->slice_expired = ರಿಕ್ತ;
#endif /* CONFIG_TIMESLICE_PER_THREAD */

#ifdef CONFIG_SPIN_VALIDATE
	thread_base->swap_data = ರಿಕ್ತ;
	/* otherwise swap_data does not need to be initialized */
#endif

	z_init_thread_timeout(thread_base);
}

FUNC_NORETURN ಶೂನ್ಯ k_thread_user_mode_enter(k_thread_entry_t entry,
					    ಶೂನ್ಯ *p1, ಶೂನ್ಯ *p2, ಶೂನ್ಯ *p3)
{
	SYS_PORT_TRACING_FUNC(k_thread, user_mode_enter);

	_current->base.user_options |= K_USER;
	z_thread_essential_clear(_current);
#ifdef CONFIG_THREAD_MONITOR
	_current->entry.pEntry = entry;
	_current->entry.parameter1 = p1;
	_current->entry.parameter2 = p2;
	_current->entry.parameter3 = p3;
#endif /* CONFIG_THREAD_MONITOR */
#ifdef CONFIG_USERSPACE
	__ASSERT(z_ರಾಶಿ_is_user_capable(_current->ರಾಶಿ_obj),
		 "dropping to user mode with kernel-only stack object");
#ifdef CONFIG_THREAD_USERSPACE_LOCAL_DATA
	memset(_current->userspace_local_data, 0,
	       ಪರಿಮಾಣ(ಸಂರಚನಾ _thread_userspace_local_data));
#endif /* CONFIG_THREAD_USERSPACE_LOCAL_DATA */
#ifdef CONFIG_THREAD_LOCAL_STORAGE
	arch_tls_ರಾಶಿ_setup(_current,
			     (ವರ್ಣ *)(_current->ರಾಶಿ_info.start +
				      _current->ರಾಶಿ_info.size));
#endif /* CONFIG_THREAD_LOCAL_STORAGE */
	arch_user_mode_enter(entry, p1, p2, p3);
#ಅನ್ಯಥಾ
	/* XXX In this case we do not reset the stack */
	z_thread_entry(entry, p1, p2, p3);
#endif /* CONFIG_USERSPACE */
}

#ಯದಿ defined(CONFIG_INIT_STACKS) && defined(CONFIG_THREAD_ರಾಶಿ_INFO)
#ifdef CONFIG_ರಾಶಿ_GROWS_UP
#error "Unsupported configuration for stack analysis"
#endif /* CONFIG_STACK_GROWS_UP */

ಪೂರ್ಣಾಙ್ಕ z_ರಾಶಿ_space_get(ಧ್ರುವ uint8_t *ರಾಶಿ_start, size_t size, size_t *unused_ptr)
{
	size_t unused = 0;
	ಧ್ರುವ uint8_t *checked_ರಾಶಿ = ರಾಶಿ_start;
	/* Take the address of any local variable as a shallow bound for the
	 * stack pointer.  Addresses above it are guaranteed to be
	 * accessible.
	 */
	ಧ್ರುವ uint8_t *ರಾಶಿ_pointer = (ಧ್ರುವ uint8_t *)&ರಾಶಿ_start;

	/* If we are currently running on the stack being analyzed, some
	 * memory management hardware will generate an exception if we
	 * read unused stack memory.
	 *
	 * This never happens when invoked from user mode, as user mode
	 * will always run this function on the privilege elevation stack.
	 */
	ಯದಿ ((ರಾಶಿ_pointer > ರಾಶಿ_start) && (ರಾಶಿ_pointer <= (ರಾಶಿ_start + size)) &&
	    IS_ENABLED(CONFIG_NO_UNUSED_ರಾಶಿ_INSPECTION)) {
		/* TODO: We could add an arch_ API call to temporarily
		 * disable the stack checking in the CPU, but this would
		 * need to be properly managed wrt context switches/interrupts
		 */
		ಪ್ರತ್ಯಾಗಮ -ENOTSUP;
	}

	ಯದಿ (IS_ENABLED(CONFIG_ರಾಶಿ_SENTINEL)) {
		/* First 4 bytes of the stack buffer reserved for the
		 * sentinel value, it won't be 0xAAAAAAAA for thread
		 * stacks.
		 *
		 * FIXME: thread->stack_info.start ought to reflect
		 * this!
		 */
		checked_ರಾಶಿ += 4;
		size -= 4;
	}

	ಪ್ರತಿ (size_t i = 0; i < size; i++) {
		ಯದಿ ((checked_ರಾಶಿ[i]) == 0xaaU) {
			unused++;
		} ಅನ್ಯಥಾ {
			ಭಙ್ಗ;
		}
	}

	*unused_ptr = unused;

	ಪ್ರತ್ಯಾಗಮ 0;
}

#ifdef CONFIG_THREAD_RUNTIME_ರಾಶಿ_SAFETY
ಪೂರ್ಣಾಙ್ಕ k_thread_runtime_ರಾಶಿ_safety_full_check(ಧ್ರುವ ಸಂರಚನಾ k_thread *thread,
					     size_t *unused_ptr,
					     k_thread_ರಾಶಿ_safety_handler_t handler,
					     ಶೂನ್ಯ *arg)
{
	ಪೂರ್ಣಾಙ್ಕ    rv;
	size_t unused_space;

	__ASSERT_NO_MSG(thread != ರಿಕ್ತ);

	rv = z_ರಾಶಿ_space_get((ಧ್ರುವ uint8_t *)thread->ರಾಶಿ_info.start,
			       thread->ರಾಶಿ_info.size, &unused_space);

	ಯದಿ (rv != 0) {
		ಪ್ರತ್ಯಾಗಮ rv;
	}

	ಯದಿ (unused_ptr != ರಿಕ್ತ) {
		*unused_ptr = unused_space;
	}

	ಯದಿ ((unused_space < thread->ರಾಶಿ_info.usage.unused_threshold) &&
	    (handler != ರಿಕ್ತ)) {
		handler(thread, unused_space, arg);
	}

	ಪ್ರತ್ಯಾಗಮ 0;
}

ಪೂರ್ಣಾಙ್ಕ k_thread_runtime_ರಾಶಿ_safety_threshold_check(ಧ್ರುವ ಸಂರಚನಾ k_thread *thread,
						  size_t *unused_ptr,
						  k_thread_ರಾಶಿ_safety_handler_t handler,
						  ಶೂನ್ಯ *arg)
{
	ಪೂರ್ಣಾಙ್ಕ    rv;
	size_t unused_space;
	size_t threshold;
	size_t scan_size;

	__ASSERT_NO_MSG(thread != ರಿಕ್ತ);

	threshold = thread->ರಾಶಿ_info.usage.unused_threshold;
	scan_size = threshold;

	/* z_stack_space_get() reserves the first bytes of the stack buffer for
	 * the stack sentinel and shrinks its scan window by that amount. Grow
	 * the requested window by the same reservation so that up to
	 * 'threshold' bytes of the usable stack are actually examined;
	 * otherwise a fully unused window would report fewer than 'threshold'
	 * unused bytes and spuriously trip the threshold.
	 */
	ಯದಿ (IS_ENABLED(CONFIG_ರಾಶಿ_SENTINEL) && (threshold != 0U)) {
		scan_size += ಪರಿಮಾಣ(uint32_t);
	}

	rv = z_ರಾಶಿ_space_get((ಧ್ರುವ uint8_t *)thread->ರಾಶಿ_info.start,
			       scan_size, &unused_space);

	ಯದಿ (rv != 0) {
		ಪ್ರತ್ಯಾಗಮ rv;
	}

	ಯದಿ (unused_ptr != ರಿಕ್ತ) {
		*unused_ptr = unused_space;
	}

	ಯದಿ ((unused_space < threshold) && (handler != ರಿಕ್ತ)) {
		handler(thread, unused_space, arg);
	}

	ಪ್ರತ್ಯಾಗಮ 0;
}
#endif /* CONFIG_THREAD_RUNTIME_STACK_SAFETY */

ಪೂರ್ಣಾಙ್ಕ z_impl_k_thread_ರಾಶಿ_space_get(ಧ್ರುವ ಸಂರಚನಾ k_thread *thread,
				    size_t *unused_ptr)
{
#ifdef CONFIG_THREAD_ರಾಶಿ_MEM_MAPPED
	ಯದಿ (thread->ರಾಶಿ_info.mapped.addr == ರಿಕ್ತ) {
		ಪ್ರತ್ಯಾಗಮ -EINVAL;
	}
#endif /* CONFIG_THREAD_STACK_MEM_MAPPED */

	ಪ್ರತ್ಯಾಗಮ z_ರಾಶಿ_space_get((ಧ್ರುವ uint8_t *)thread->ರಾಶಿ_info.start,
				 thread->ರಾಶಿ_info.size, unused_ptr);
}

#ifdef CONFIG_USERSPACE
ಪೂರ್ಣಾಙ್ಕ z_vrfy_k_thread_ರಾಶಿ_space_get(ಧ್ರುವ ಸಂರಚನಾ k_thread *thread,
				    size_t *unused_ptr)
{
	size_t unused;
	ಪೂರ್ಣಾಙ್ಕ ret;

	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));

	ret = z_impl_k_thread_ರಾಶಿ_space_get(thread, &unused);
	CHECKIF(ret != 0) {
		ಪ್ರತ್ಯಾಗಮ ret;
	}

	ret = k_usermode_to_copy(unused_ptr, &unused, ಪರಿಮಾಣ(size_t));
	CHECKIF(ret != 0) {
		ಪ್ರತ್ಯಾಗಮ ret;
	}

	ಪ್ರತ್ಯಾಗಮ 0;
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_thread_stack_space_get_mrsh.c>
#endif /* CONFIG_USERSPACE */
#endif /* CONFIG_INIT_STACKS && CONFIG_THREAD_STACK_INFO */

#ifdef CONFIG_USERSPACE
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ k_ticks_t z_vrfy_k_thread_timeout_remaining_ticks(
						    ಧ್ರುವ ಸಂರಚನಾ k_thread *thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	ಪ್ರತ್ಯಾಗಮ z_impl_k_thread_timeout_remaining_ticks(thread);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_thread_timeout_remaining_ticks_mrsh.c>

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ k_ticks_t z_vrfy_k_thread_timeout_expires_ticks(
						  ಧ್ರುವ ಸಂರಚನಾ k_thread *thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	ಪ್ರತ್ಯಾಗಮ z_impl_k_thread_timeout_expires_ticks(thread);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_thread_timeout_expires_ticks_mrsh.c>
#endif /* CONFIG_USERSPACE */

#ifdef CONFIG_INSTRUMENT_THREAD_SWITCHING
ಶೂನ್ಯ z_thread_mark_switched_in(ಶೂನ್ಯ)
{
#ಯದಿ defined(CONFIG_SCHED_THREAD_USAGE) && !defined(CONFIG_USE_SWITCH)
	z_sched_usage_start(_current);
#endif /* CONFIG_SCHED_THREAD_USAGE && !CONFIG_USE_SWITCH */

#ifdef CONFIG_TRACING
	SYS_PORT_TRACING_FUNC(k_thread, switched_in);
#endif /* CONFIG_TRACING */
}

ಶೂನ್ಯ z_thread_mark_switched_out(ಶೂನ್ಯ)
{
#ಯದಿ defined(CONFIG_SCHED_THREAD_USAGE) && !defined(CONFIG_USE_SWITCH)
	z_sched_usage_stop();
#endif /*CONFIG_SCHED_THREAD_USAGE && !CONFIG_USE_SWITCH */

#ifdef CONFIG_TRACING
#ifdef CONFIG_THREAD_LOCAL_STORAGE
	/* Dummy thread won't have TLS set up to run arbitrary code */
	ಯದಿ (!_current ||
	    (_current->base.thread_state & _THREAD_DUMMY) != 0) {
		ಪ್ರತ್ಯಾಗಮ;
	}
#endif /* CONFIG_THREAD_LOCAL_STORAGE */
	SYS_PORT_TRACING_FUNC(k_thread, switched_out);
#endif /* CONFIG_TRACING */
}
#endif /* CONFIG_INSTRUMENT_THREAD_SWITCHING */

ಪೂರ್ಣಾಙ್ಕ k_thread_runtime_stats_get(k_tid_t thread,
			       k_thread_runtime_stats_t *stats)
{
	ಯದಿ ((thread == ರಿಕ್ತ) || (stats == ರಿಕ್ತ)) {
		ಪ್ರತ್ಯಾಗಮ -EINVAL;
	}

#ifdef CONFIG_SCHED_THREAD_USAGE
	z_sched_thread_usage(thread, stats);
#ಅನ್ಯಥಾ
	*stats = (k_thread_runtime_stats_t) {};
#endif /* CONFIG_SCHED_THREAD_USAGE */

	ಪ್ರತ್ಯಾಗಮ 0;
}

ಪೂರ್ಣಾಙ್ಕ k_thread_runtime_stats_all_get(k_thread_runtime_stats_t *stats)
{
#ifdef CONFIG_SCHED_THREAD_USAGE_ALL
	k_thread_runtime_stats_t  tmp_stats;
#endif /* CONFIG_SCHED_THREAD_USAGE_ALL */

	ಯದಿ (stats == ರಿಕ್ತ) {
		ಪ್ರತ್ಯಾಗಮ -EINVAL;
	}

	*stats = (k_thread_runtime_stats_t) {};

#ifdef CONFIG_SCHED_THREAD_USAGE_ALL
	/* Retrieve the usage stats for each core and amalgamate them. */

	unsigned ಪೂರ್ಣಾಙ್ಕ ಅಂಕೆ_cpus = arch_ಅಂಕೆ_cpus();

	ಪ್ರತಿ (unsigned ಪೂರ್ಣಾಙ್ಕ i = 0; i < ಅಂಕೆ_cpus; i++) {
		z_sched_cpu_usage(i, &tmp_stats);

		stats->execution_cycles += tmp_stats.execution_cycles;
		stats->total_cycles     += tmp_stats.total_cycles;
#ifdef CONFIG_SCHED_THREAD_USAGE_ANALYSIS
		stats->current_cycles   += tmp_stats.current_cycles;
		stats->peak_cycles      += tmp_stats.peak_cycles;
		stats->average_cycles   += tmp_stats.average_cycles;
#endif /* CONFIG_SCHED_THREAD_USAGE_ANALYSIS */
		stats->idle_cycles      += tmp_stats.idle_cycles;
	}
#endif /* CONFIG_SCHED_THREAD_USAGE_ALL */

	ಪ್ರತ್ಯಾಗಮ 0;
}

ಪೂರ್ಣಾಙ್ಕ k_thread_runtime_stats_cpu_get(ಪೂರ್ಣಾಙ್ಕ cpu, k_thread_runtime_stats_t *stats)
{
	ಯದಿ (stats == ರಿಕ್ತ) {
		ಪ್ರತ್ಯಾಗಮ -EINVAL;
	}

	*stats = (k_thread_runtime_stats_t) {};

#ifdef CONFIG_SCHED_THREAD_USAGE_ALL
	CHECKIF((cpu < 0) || ((unsigned ಪೂರ್ಣಾಙ್ಕ)cpu >= arch_ಅಂಕೆ_cpus())) {
		ಪ್ರತ್ಯಾಗಮ -EINVAL;
	}

	z_sched_cpu_usage(cpu, stats);
#ಅನ್ಯಥಾ
	ARG_UNUSED(cpu);
#endif

	ಪ್ರತ್ಯಾಗಮ 0;
}

#ifdef CONFIG_THREAD_ABORT_NEED_CLEANUP
/** Pointer to thread which needs to be cleaned up. */
ಸ್ಥಿರ ಸಂರಚನಾ k_thread *thread_to_cleanup;

/** Spinlock for thread abort cleanup. */
ಸ್ಥಿರ ಸಂರಚನಾ k_spinlock thread_cleanup_lock;

#ifdef CONFIG_THREAD_ರಾಶಿ_MEM_MAPPED
ಸ್ಥಿರ ಶೂನ್ಯ *thread_cleanup_ರಾಶಿ_addr;
ಸ್ಥಿರ size_t thread_cleanup_ರಾಶಿ_sz;

ಸ್ಥಿರ ಶೂನ್ಯ unmap_thread_ರಾಶಿ(ಶೂನ್ಯ *addr, size_t sz)
{
	ಯದಿ (addr != ರಿಕ್ತ) {
		k_mem_unmap_phys_guard(addr, sz, false);
	}
}
#endif /* CONFIG_THREAD_STACK_MEM_MAPPED */

ಶೂನ್ಯ defer_thread_cleanup(ಸಂರಚನಾ k_thread *thread)
{
	/* Note when adding new deferred cleanup steps:
	 * - The thread object may have been overwritten by the time
	 *   the actual cleanup is being done (e.g. thread object
	 *   allocated on a stack). So stash any necessary data here
	 *   that will be used in the actual cleanup steps.
	 */
	thread_to_cleanup = thread;

#ifdef CONFIG_THREAD_ರಾಶಿ_MEM_MAPPED
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
	thread->ರಾಶಿ_info.mapped.addr = ರಿಕ್ತ;
	thread->ರಾಶಿ_info.mapped.sz = 0;
#endif /* CONFIG_THREAD_STACK_MEM_MAPPED */
}

ಶೂನ್ಯ do_deferred_thread_cleanup(ಶೂನ್ಯ)
{
	/* Note when adding new actual cleanup steps:
	 * - The thread object may have been overwritten when this is
	 *   called. So only data stashed by defer_thread_cleanup() can
	 *   be used here, never anything from the thread object.
	 */

#ifdef CONFIG_THREAD_ರಾಶಿ_MEM_MAPPED
	unmap_thread_ರಾಶಿ(thread_cleanup_ರಾಶಿ_addr, thread_cleanup_ರಾಶಿ_sz);

	thread_cleanup_ರಾಶಿ_addr = ರಿಕ್ತ;
	thread_cleanup_ರಾಶಿ_sz = 0;
#endif /* CONFIG_THREAD_STACK_MEM_MAPPED */
}

ಶೂನ್ಯ do_thread_cleanup(ಸಂರಚನಾ k_thread *thread)
{
	/* This is only for threads which are no longer running and are not
	 * the current thread, so the thread object is still valid here and
	 * the cleanup can be driven from it directly.
	 */

#ifdef CONFIG_THREAD_ರಾಶಿ_MEM_MAPPED
	unmap_thread_ರಾಶಿ(thread->ರಾಶಿ_info.mapped.addr,
			   thread->ರಾಶಿ_info.mapped.sz);

	/* The stack is now unmapped and thus un-usable. Clear the mapping
	 * info so nothing looks into the stack afterwards, and so that
	 * a repeated cleanup of the same thread object is a no-op,
	 * e.g., z_stack_space_get().
	 */
	thread->ರಾಶಿ_info.mapped.addr = ರಿಕ್ತ;
	thread->ರಾಶಿ_info.mapped.sz = 0;
#ಅನ್ಯಥಾ /* CONFIG_THREAD_STACK_MEM_MAPPED */
	ARG_UNUSED(thread);
#endif /* CONFIG_THREAD_STACK_MEM_MAPPED */
}

ಶೂನ್ಯ k_thread_abort_cleanup(ಸಂರಚನಾ k_thread *thread)
{
	K_SPINLOCK(&thread_cleanup_lock) {
		ಯದಿ (thread_to_cleanup != ರಿಕ್ತ) {
			/* Finish the pending one first. */
			do_deferred_thread_cleanup();
			thread_to_cleanup = ರಿಕ್ತ;
		}

		ಯದಿ (thread == _current) {
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
		} ಅನ್ಯಥಾ {
			/* Not the current running thread, so we are safe to do
			 * cleanups.
			 */
			do_thread_cleanup(thread);
		}
	}
}

ಶೂನ್ಯ k_thread_abort_cleanup_check_reuse(ಸಂರಚನಾ k_thread *thread)
{
	K_SPINLOCK(&thread_cleanup_lock) {
		/* This is to guard reuse of the same thread object and make sure
		 * any pending cleanups of it needs to be finished before the thread
		 * object can be reused.
		 */
		ಯದಿ (thread_to_cleanup == thread) {
			do_deferred_thread_cleanup();
			thread_to_cleanup = ರಿಕ್ತ;
		}
	}
}

#endif /* CONFIG_THREAD_ABORT_NEED_CLEANUP */

ಶೂನ್ಯ z_dummy_thread_init(ಸಂರಚನಾ k_thread *dummy_thread)
{
	dummy_thread->base.thread_state = _THREAD_DUMMY;
#ifdef CONFIG_SCHED_CPU_MASK
	dummy_thread->base.cpu_mask = -1;
#endif /* CONFIG_SCHED_CPU_MASK */
	dummy_thread->base.user_options = K_ESSENTIAL;
#ಯದಿ Z_MUTEX_PI_ENABLED
	sys_slist_init(&dummy_thread->held_mutexes);
	dummy_thread->mutex_pended_on = ರಿಕ್ತ;
	dummy_thread->orig_prio = dummy_thread->base.prio;
#endif /* Z_MUTEX_PI_ENABLED */
#ifdef CONFIG_THREAD_ರಾಶಿ_INFO
	dummy_thread->ರಾಶಿ_info.start = 0U;
	dummy_thread->ರಾಶಿ_info.size = 0U;
#endif /* CONFIG_THREAD_STACK_INFO */
#ifdef CONFIG_USERSPACE
	dummy_thread->mem_domain_info.mem_domain = &k_mem_domain_default;
#endif /* CONFIG_USERSPACE */
#ಯದಿ (K_HEAP_MEM_POOL_SIZE > 0)
	k_thread_system_pool_assign(dummy_thread);
#ಅನ್ಯಥಾ
	dummy_thread->resource_pool = ರಿಕ್ತ;
#endif /* K_HEAP_MEM_POOL_SIZE */

#ifdef CONFIG_USE_SWITCH
	dummy_thread->switch_handle = ರಿಕ್ತ;
#endif /* CONFIG_USE_SWITCH */

#ifdef CONFIG_TIMESLICE_PER_THREAD
	dummy_thread->base.slice_ticks = 0;
#endif /* CONFIG_TIMESLICE_PER_THREAD */

	z_current_thread_set(dummy_thread);
}

/*
 * Thread lifecycle APIs: suspend, resume, priority, wakeup, abort, join.
 *
 * These APIs operate on thread objects and belong here in thread.c.  They
 * depend on a small set of scheduler internals (z_thread_halt,
 * z_sched_ready_locked, z_sched_add_to_waitq_locked) exposed via ksched.h.
 */

ಶೂನ್ಯ z_impl_k_thread_priority_set(k_tid_t thread, ಪೂರ್ಣಾಙ್ಕ prio)
{
	/*
	 * Use NULL, since we cannot know what the entry point is (we do not
	 * keep track of it) and idle cannot change its priority.
	 */
	Z_ASSERT_VALID_PRIO(prio, ರಿಕ್ತ);

	bool need_sched = z_thread_prio_set((ಸಂರಚನಾ k_thread *)thread, prio);

	ಯದಿ ((need_sched) && (IS_ENABLED(CONFIG_SMP) ||
			     (_current->base.sched_locked == 0U))) {
		z_reschedule_unlocked();
	}
}

#ifdef CONFIG_USERSPACE
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ z_vrfy_k_thread_priority_set(k_tid_t thread, ಪೂರ್ಣಾಙ್ಕ prio)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	K_OOPS(K_SYSCALL_VERIFY_ಸಂದೇಶ(_is_valid_prio(prio, ರಿಕ್ತ),
				    "invalid thread priority %d", prio));
#ifndef CONFIG_USERSPACE_THREAD_MAY_RAISE_PRIORITY
	K_OOPS(K_SYSCALL_VERIFY_ಸಂದೇಶ((int8_t)prio >= thread->base.prio,
				    "thread priority may only be downgraded (%d < %d)",
				    prio, thread->base.prio));
#endif /* CONFIG_USERSPACE_THREAD_MAY_RAISE_PRIORITY */
	z_impl_k_thread_priority_set(thread, prio);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_thread_priority_set_mrsh.c>
#endif /* CONFIG_USERSPACE */

ಶೂನ್ಯ z_impl_k_thread_suspend(k_tid_t thread)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_thread, suspend, thread);

	/* Special case "suspend the current thread" as it doesn't
	 * need the async complexity below.
	 */
	ಯದಿ (!IS_ENABLED(CONFIG_SMP) &&
	    likely((thread == _current) && !arch_is_in_isr())) {
		z_thread_suspend_current(thread);
		ಪ್ರತ್ಯಾಗಮ;
	}

	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = z_sched_spinlock_lock();

	ಯದಿ (unlikely(z_is_thread_suspended(thread))) {

		/* The target thread is already suspended. Nothing to do. */

		z_sched_spinlock_unlock(ಕೀಲಿ);
		ಪ್ರತ್ಯಾಗಮ;
	}

	z_thread_halt(thread, ಕೀಲಿ, false);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_thread, suspend, thread);
}

#ifdef CONFIG_USERSPACE
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ z_vrfy_k_thread_suspend(k_tid_t thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	z_impl_k_thread_suspend(thread);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_thread_suspend_mrsh.c>
#endif /* CONFIG_USERSPACE */

ಶೂನ್ಯ z_impl_k_thread_resume(k_tid_t thread)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_thread, resume, thread);

	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = z_sched_spinlock_lock();

	/* Do not try to resume a thread that was not suspended */
	ಯದಿ (unlikely(!z_is_thread_suspended(thread))) {
		z_sched_spinlock_unlock(ಕೀಲಿ);
		ಪ್ರತ್ಯಾಗಮ;
	}

	z_mark_thread_as_not_suspended(thread);
	z_sched_ready_locked(thread);

	z_reschedule_locked(ಕೀಲಿ);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_thread, resume, thread);
}

#ifdef CONFIG_USERSPACE
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ z_vrfy_k_thread_resume(k_tid_t thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	z_impl_k_thread_resume(thread);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_thread_resume_mrsh.c>
#endif /* CONFIG_USERSPACE */

ಶೂನ್ಯ z_impl_k_wakeup(k_tid_t thread)
{
	SYS_PORT_TRACING_OBJ_FUNC(k_thread, wakeup, thread);

	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = z_sched_spinlock_lock();

	ಯದಿ (z_is_thread_sleeping(thread)) {
		(ಶೂನ್ಯ)z_try_abort_thread_timeout(thread);
		z_mark_thread_as_not_sleeping(thread);
		z_sched_ready_locked(thread);
		z_reschedule_locked(ಕೀಲಿ);
	} ಅನ್ಯಥಾ {
		z_sched_spinlock_unlock(ಕೀಲಿ);
	}
}

#ifdef CONFIG_USERSPACE
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ z_vrfy_k_wakeup(k_tid_t thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	z_impl_k_wakeup(thread);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_wakeup_mrsh.c>
#endif /* CONFIG_USERSPACE */

ಶೂನ್ಯ z_thread_abort(ಸಂರಚನಾ k_thread *thread)
{
	bool essential = z_is_thread_essential(thread);
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = z_sched_spinlock_lock();

	ಯದಿ (z_is_thread_dead(thread)) {
		z_sched_spinlock_unlock(ಕೀಲಿ);
		ಪ್ರತ್ಯಾಗಮ;
	}

	z_thread_halt(thread, ಕೀಲಿ, true);

	ಯದಿ (essential) {
		__ASSERT(!essential, "aborted essential thread %p", thread);
		k_panic();
	}
}

#ಯದಿ !defined(CONFIG_ARCH_HAS_THREAD_ABORT)
ಶೂನ್ಯ z_impl_k_thread_abort(k_tid_t thread)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_thread, abort, thread);

	z_thread_abort(thread);

	__ASSERT_NO_MSG(z_is_thread_dead(thread));

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_thread, abort, thread);
}
#endif /* !CONFIG_ARCH_HAS_THREAD_ABORT */

ಪೂರ್ಣಾಙ್ಕ z_impl_k_thread_join(ಸಂರಚನಾ k_thread *thread, k_timeout_t timeout)
{
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = z_sched_spinlock_lock();
	ಪೂರ್ಣಾಙ್ಕ ret;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_thread, join, thread, timeout);

	ಯದಿ (z_is_thread_dead(thread)) {
		z_sched_switch_spin(thread);
		ret = 0;
	} ಅನ್ಯಥಾ ಯದಿ (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		ret = -EBUSY;
	} ಅನ್ಯಥಾ ಯದಿ ((thread == _current) ||
		   (thread->base.pended_on == &_current->join_queue)) {
		ret = -EDEADLK;
	} ಅನ್ಯಥಾ {
		__ASSERT(!arch_is_in_isr(), "cannot join in ISR");
		z_sched_add_to_waitq_locked(_current, &thread->join_queue);
		z_add_thread_timeout(_current, timeout);

		SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_thread, join, thread, timeout);
		ret = z_swap_locked(ಕೀಲಿ);
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_thread, join, thread, timeout, ret);

		ಪ್ರತ್ಯಾಗಮ ret;
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_thread, join, thread, timeout, ret);

	z_sched_spinlock_unlock(ಕೀಲಿ);
	ಪ್ರತ್ಯಾಗಮ ret;
}

#ifdef CONFIG_USERSPACE
/* Special case: don't oops if the thread is uninitialized.  This is because
 * the initialization bit does double-duty for thread objects; if false, means
 * the thread object is truly uninitialized, or the thread ran and exited for
 * some reason.
 *
 * Return true in this case indicating we should just do nothing and return
 * success to the caller.
 */
ಸ್ಥಿರ bool thread_obj_validate(ಸಂರಚನಾ k_thread *thread)
{
	ಸಂರಚನಾ k_object *ko = k_object_find(thread);
	ಪೂರ್ಣಾಙ್ಕ ret = k_object_validate(ko, K_OBJ_THREAD, _OBJ_INIT_TRUE);

	ವಿಕಲ್ಪ (ret) {
	ಪಕ್ಷ 0:
		ಪ್ರತ್ಯಾಗಮ false;
	ಪಕ್ಷ -EINVAL:
		ಪ್ರತ್ಯಾಗಮ true;
	ಸ್ವಭಾವ:
#ifdef CONFIG_LOG
		k_object_dump_error(ret, thread, ko, K_OBJ_THREAD);
#endif /* CONFIG_LOG */
		/* ret is a non-zero error code here (the 0 and -EINVAL cases
		 * are handled above), so this branch must always oops. Passing
		 * ret as the "verify" expression would treat the failure code as
		 * success and fall through to CODE_UNREACHABLE; verify ret == 0
		 * so the oops is actually raised.
		 */
		K_OOPS(K_SYSCALL_VERIFY_ಸಂದೇಶ(ret == 0, "access denied"));
	}
	CODE_UNREACHABLE; /* LCOV_EXCL_LINE */
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಪೂರ್ಣಾಙ್ಕ z_vrfy_k_thread_join(ಸಂರಚನಾ k_thread *thread,
				       k_timeout_t timeout)
{
	ಯದಿ (thread_obj_validate(thread)) {
		ಪ್ರತ್ಯಾಗಮ 0;
	}

	ಪ್ರತ್ಯಾಗಮ z_impl_k_thread_join(thread, timeout);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_thread_join_mrsh.c>

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ z_vrfy_k_thread_abort(k_tid_t thread)
{
	ಯದಿ (thread_obj_validate(thread)) {
		ಪ್ರತ್ಯಾಗಮ;
	}

	K_OOPS(K_SYSCALL_VERIFY_ಸಂದೇಶ(!z_is_thread_essential(thread),
				    "aborting essential thread %p", thread));

	z_impl_k_thread_abort((ಸಂರಚನಾ k_thread *)thread);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_thread_abort_mrsh.c>
#endif /* CONFIG_USERSPACE */

bool k_can_yield(ಶೂನ್ಯ)
{
	ಪ್ರತ್ಯಾಗಮ !(k_is_pre_kernel() || k_is_in_isr() || !arch_cpu_irqs_are_enabled() ||
		 z_is_idle_thread_object(_current));
}

ಶೂನ್ಯ z_impl_k_yield(ಶೂನ್ಯ)
{
	__ASSERT(!arch_is_in_isr(), "");

	SYS_PORT_TRACING_FUNC(k_thread, yield);

	z_sched_yield();
}

#ifdef CONFIG_USERSPACE
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ z_vrfy_k_yield(ಶೂನ್ಯ)
{
	z_impl_k_yield();
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_yield_mrsh.c>
#endif /* CONFIG_USERSPACE */
