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

#അന്തര്ഭാവ <zephyr/kernel.h>
#അന്തര്ഭാവ <zephyr/spinlock.h>
#അന്തര്ഭാവ <zephyr/sys/math_extras.h>
#അന്തര്ഭാവ <zephyr/sys/clock.h>
#അന്തര്ഭാവ <ksched.h>
#അന്തര്ഭാവ <kthread.h>
#അന്തര്ഭാവ <wait_q.h>
#അന്തര്ഭാവ <timeout_q.h>
#അന്തര്ഭാവ <zephyr/internal/syscall_handler.h>
#അന്തര്ഭാവ <kernel_internal.h>
#അന്തര്ഭാവ <kswap.h>
#അന്തര്ഭാവ <zephyr/init.h>
#അന്തര്ഭാവ <zephyr/tracing/tracing.h>
#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ <stdbool.h>
#അന്തര്ഭാവ <zephyr/sys/check.h>
#അന്തര്ഭാവ <zephyr/random/random.h>
#അന്തര്ഭാവ <zephyr/sys/atomic.h>
#അന്തര്ഭാവ <zephyr/logging/log.h>
#അന്തര്ഭാവ <zephyr/llext/symbol.h>
#അന്തര്ഭാവ <zephyr/sys/iterable_sections.h>
#അന്തര്ഭാവ <zephyr/sys/minmax.h>

#അന്തര്ഭാവ <usage.h>

LOG_MODULE_DECLARE(os, CONFIG_KERNEL_LOG_LEVEL);

#ifdef CONFIG_OBJ_CORE_THREAD
#ifdef CONFIG_OBJ_CORE_STATS_THREAD
സ്ഥിതം ഘടന k_obj_core_stats_desc  thread_stats_desc = {
	.raw_size = വലുപ്പം(ഘടന k_cycle_stats),
	.query_size = വലുപ്പം(ഘടന k_thread_runtime_stats),
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
#അല്ലെങ്കിൽ
K_OBJ_TYPE_DEFINE_TYPE_ONLY(obj_type_thread, k_thread, K_OBJ_TYPE_THREAD_ID,
			    ശൂന്യ);
#endif /* CONFIG_OBJ_CORE_STATS_THREAD */
#endif /* CONFIG_OBJ_CORE_THREAD */


#define _FOREACH_STATIC_THREAD(thread_data)              \
	STRUCT_SECTION_FOREACH(_static_thread_data, thread_data)

bool k_is_in_isr(ശൂന്യം)
{
	തിരികെ arch_is_in_isr();
}
EXPORT_ചിഹ്നം(k_is_in_isr);

#ifdef CONFIG_THREAD_CUSTOM_DATA
ശൂന്യം z_impl_k_thread_custom_data_set(ശൂന്യം *മൂല്യം)
{
	_current->custom_data = മൂല്യം;
}

#ifdef CONFIG_USERSPACE
സ്ഥിതം വരിയിൽ ശൂന്യം z_vrfy_k_thread_custom_data_set(ശൂന്യം *data)
{
	z_impl_k_thread_custom_data_set(data);
}
#അന്തര്ഭാവ <zephyr/syscalls/k_thread_custom_data_set_mrsh.c>
#endif /* CONFIG_USERSPACE */

ശൂന്യം *z_impl_k_thread_custom_data_get(ശൂന്യം)
{
	തിരികെ _current->custom_data;
}

#ifdef CONFIG_USERSPACE
സ്ഥിതം വരിയിൽ ശൂന്യം *z_vrfy_k_thread_custom_data_get(ശൂന്യം)
{
	തിരികെ z_impl_k_thread_custom_data_get();
}
#അന്തര്ഭാവ <zephyr/syscalls/k_thread_custom_data_get_mrsh.c>

#endif /* CONFIG_USERSPACE */
#endif /* CONFIG_THREAD_CUSTOM_DATA */

പൂർണ്ണസംഖ്യ z_impl_k_is_preempt_thread(ശൂന്യം)
{
	തിരികെ !arch_is_in_isr() && thread_is_preemptible(_current);
}

#ifdef CONFIG_USERSPACE
സ്ഥിതം വരിയിൽ പൂർണ്ണസംഖ്യ z_vrfy_k_is_preempt_thread(ശൂന്യം)
{
	തിരികെ z_impl_k_is_preempt_thread();
}
#അന്തര്ഭാവ <zephyr/syscalls/k_is_preempt_thread_mrsh.c>
#endif /* CONFIG_USERSPACE */

പൂർണ്ണസംഖ്യ z_impl_k_thread_priority_get(k_tid_t thread)
{
	തിരികെ thread->base.prio;
}

#ifdef CONFIG_USERSPACE
സ്ഥിതം വരിയിൽ പൂർണ്ണസംഖ്യ z_vrfy_k_thread_priority_get(k_tid_t thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	തിരികെ z_impl_k_thread_priority_get(thread);
}
#അന്തര്ഭാവ <zephyr/syscalls/k_thread_priority_get_mrsh.c>
#endif /* CONFIG_USERSPACE */

#ifdef CONFIG_THREAD_NAME
സ്ഥിതം ശൂന്യം set_thread_name(ഘടന k_thread *thread, സ്ഥിരം അക്ഷരം *str)
{
	എങ്കിൽ (str != ശൂന്യ) {
		strncpy(thread->name, str, CONFIG_THREAD_MAX_NAME_LEN - 1);
		/* Ensure NULL termination, truncate if longer */
		thread->name[CONFIG_THREAD_MAX_NAME_LEN - 1] = '\0';
#ifdef CONFIG_ARCH_HAS_THREAD_NAME_HOOK
		arch_thread_name_set(thread, str);
#endif /* CONFIG_ARCH_HAS_THREAD_NAME_HOOK */
	} അല്ലെങ്കിൽ {
		thread->name[0] = '\0';
	}
}
#endif /* CONFIG_THREAD_NAME */

പൂർണ്ണസംഖ്യ z_impl_k_thread_name_set(k_tid_t thread, സ്ഥിരം അക്ഷരം *str)
{
#ifdef CONFIG_THREAD_NAME
	എങ്കിൽ (thread == ശൂന്യ) {
		thread = _current;
	}

	set_thread_name(thread, str);

	SYS_PORT_TRACING_OBJ_FUNC(k_thread, name_set, thread, 0);

	തിരികെ 0;
#അല്ലെങ്കിൽ
	ARG_UNUSED(thread);
	ARG_UNUSED(str);

	SYS_PORT_TRACING_OBJ_FUNC(k_thread, name_set, thread, -ENOSYS);

	തിരികെ -ENOSYS;
#endif /* CONFIG_THREAD_NAME */
}

#ifdef CONFIG_USERSPACE
സ്ഥിതം വരിയിൽ പൂർണ്ണസംഖ്യ z_vrfy_k_thread_name_set(k_tid_t thread, സ്ഥിരം അക്ഷരം *str)
{
#ifdef CONFIG_THREAD_NAME
	അക്ഷരം name[CONFIG_THREAD_MAX_NAME_LEN];

	എങ്കിൽ (thread != ശൂന്യ) {
		എങ്കിൽ (K_SYSCALL_OBJ(thread, K_OBJ_THREAD) != 0) {
			തിരികെ -EINVAL;
		}
	}

	/* In theory we could copy directly into thread->name, but
	 * the current z_vrfy / z_impl split does not provide a
	 * means of doing so.
	 */
	എങ്കിൽ (k_usermode_string_copy(name, str, വലുപ്പം(name)) != 0) {
		തിരികെ -EFAULT;
	}

	തിരികെ z_impl_k_thread_name_set(thread, name);
#അല്ലെങ്കിൽ
	തിരികെ -ENOSYS;
#endif /* CONFIG_THREAD_NAME */
}
#അന്തര്ഭാവ <zephyr/syscalls/k_thread_name_set_mrsh.c>
#endif /* CONFIG_USERSPACE */

സ്ഥിരം അക്ഷരം *k_thread_name_get(k_tid_t thread)
{
#ifdef CONFIG_THREAD_NAME
	തിരികെ (സ്ഥിരം അക്ഷരം *)thread->name;
#അല്ലെങ്കിൽ
	ARG_UNUSED(thread);
	തിരികെ ശൂന്യ;
#endif /* CONFIG_THREAD_NAME */
}

പൂർണ്ണസംഖ്യ z_impl_k_thread_name_copy(k_tid_t thread, അക്ഷരം *buf, size_t size)
{
#ifdef CONFIG_THREAD_NAME
	strncpy(buf, thread->name, size);
	തിരികെ 0;
#അല്ലെങ്കിൽ
	ARG_UNUSED(thread);
	ARG_UNUSED(buf);
	ARG_UNUSED(size);
	തിരികെ -ENOSYS;
#endif /* CONFIG_THREAD_NAME */
}

സ്ഥിതം size_t copy_bytes(അക്ഷരം *dest, size_t dest_size, സ്ഥിരം അക്ഷരം *src, size_t src_size)
{
	size_t  bytes_to_copy;

	bytes_to_copy = min(dest_size, src_size);
	memcpy(dest, src, bytes_to_copy);

	തിരികെ bytes_to_copy;
}

സ്ഥിരം അക്ഷരം *k_thread_state_str(k_tid_t thread_id, അക്ഷരം *buf, size_t buf_size)
{
	size_t      off = 0;
	uint8_t     bit;
	uint8_t     thread_state = thread_id->base.thread_state;
#define SS_ENT(s) { Z_STATE_STR_##s, _THREAD_##s, വലുപ്പം(Z_STATE_STR_##s) - 1 }
	സ്ഥിതം സ്ഥിരം ഘടന {
		സ്ഥിരം അക്ഷരം *str;
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

	എങ്കിൽ ((buf == ശൂന്യ) || (buf_size == 0)) {
		തിരികെ "";
	}

	buf_size--;   /* Reserve 1 byte for end-of-string character */

	/*
	 * Loop through each bit in the thread_state. Stop once all have
	 * been processed. If more than one thread_state bit is set, then
	 * separate the descriptive strings with a '+'.
	 */


	വേണ്ടി (unsigned പൂർണ്ണസംഖ്യ index = 0; thread_state != 0; index++) {
		bit = state_string[index].bit;
		എങ്കിൽ ((thread_state & bit) == 0) {
			തുടരുക;
		}

		off += copy_bytes(buf + off, buf_size - off,
				  state_string[index].str,
				  state_string[index].len);

		thread_state &= ~bit;

		എങ്കിൽ (thread_state != 0) {
			off += copy_bytes(buf + off, buf_size - off, "+", 1);
		}
	}

	buf[off] = '\0';

	തിരികെ (സ്ഥിരം അക്ഷരം *)buf;
}

#ifdef CONFIG_USERSPACE
സ്ഥിതം വരിയിൽ പൂർണ്ണസംഖ്യ z_vrfy_k_thread_name_copy(k_tid_t thread,
					    അക്ഷരം *buf, size_t size)
{
#ifdef CONFIG_THREAD_NAME
	size_t len;
	ഘടന k_object *ko = k_object_find(thread);

	/* Special case: we allow reading the names of initialized threads
	 * even if we don't have permission on them
	 */
	എങ്കിൽ ((ko == ശൂന്യ) || (ko->type != K_OBJ_THREAD) ||
		((ko->flags & K_OBJ_FLAG_INITIALIZED) == 0)) {
		തിരികെ -EINVAL;
	}
	എങ്കിൽ (K_SYSCALL_MEMORY_WRITE(buf, size) != 0) {
		തിരികെ -EFAULT;
	}
	len = strlen(thread->name);
	എങ്കിൽ ((len + 1) > size) {
		തിരികെ -ENOSPC;
	}

	തിരികെ k_usermode_to_copy((ശൂന്യം *)buf, thread->name, len + 1);
#അല്ലെങ്കിൽ
	ARG_UNUSED(thread);
	ARG_UNUSED(buf);
	ARG_UNUSED(size);
	തിരികെ -ENOSYS;
#endif /* CONFIG_THREAD_NAME */
}
#അന്തര്ഭാവ <zephyr/syscalls/k_thread_name_copy_mrsh.c>
#endif /* CONFIG_USERSPACE */

#ifdef CONFIG_അട്ടി_SENTINEL
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
ശൂന്യം z_check_അട്ടി_sentinel(ശൂന്യം)
{
	uint32_t *അട്ടി;

	എങ്കിൽ ((_current->base.thread_state & _THREAD_DUMMY) != 0) {
		തിരികെ;
	}

	അട്ടി = (uint32_t *)_current->അട്ടി_info.start;
	എങ്കിൽ (*അട്ടി != അട്ടി_SENTINEL) {
		/* Restore it so further checks don't trigger this same error */
		*അട്ടി = അട്ടി_SENTINEL;
		z_except_reason(K_ERR_അട്ടി_CHK_FAIL);
	}
}
#endif /* CONFIG_STACK_SENTINEL */

#എങ്കിൽ defined(CONFIG_അട്ടി_POINTER_RANDOM) && (CONFIG_അട്ടി_POINTER_RANDOM != 0)
പൂർണ്ണസംഖ്യ z_അട്ടി_adjust_initialized;

സ്ഥിതം size_t random_offset(size_t അട്ടി_size)
{
	size_t random_val;

	എങ്കിൽ (!z_അട്ടി_adjust_initialized) {
		z_early_rand_get((uint8_t *)&random_val, വലുപ്പം(random_val));
	} അല്ലെങ്കിൽ {
		sys_rand_get((uint8_t *)&random_val, വലുപ്പം(random_val));
	}

	/* Don't need to worry about alignment of the size here,
	 * arch_new_thread() is required to do it.
	 *
	 * FIXME: Not the best way to get a random number in a range.
	 * See #6493
	 */
	സ്ഥിരം size_t fuzz = random_val % CONFIG_അട്ടി_POINTER_RANDOM;

	എങ്കിൽ (unlikely(fuzz * 2 > അട്ടി_size)) {
		തിരികെ 0;
	}

	തിരികെ fuzz;
}
#എങ്കിൽ defined(CONFIG_അട്ടി_GROWS_UP)
	/* This is so rare not bothering for now */
#error "Stack pointer randomization not implemented for upward growing stacks"
#endif /* CONFIG_STACK_GROWS_UP */
#endif /* CONFIG_STACK_POINTER_RANDOM */

/*
 * Geometry of a thread's stack object, computed once and threaded through the
 * setup_thread_stack() helpers below.
 */
ഘടന അട്ടി_geometry {
	size_t obj_size;	/* size of the whole stack object */
	അക്ഷരം *buf_start;	/* start of the usable stack buffer */
	size_t buf_size;	/* size of the usable stack buffer */
};

/*
 * Derive the stack object geometry from the caller-supplied stack and size.
 * User-capable stacks and kernel-only stacks reserve different amounts.
 */
സ്ഥിതം ഘടന അട്ടി_geometry compute_അട്ടി_geometry(k_thread_അട്ടി_t *അട്ടി,
						    size_t അട്ടി_size)
{
	ഘടന അട്ടി_geometry geo;

#ifdef CONFIG_USERSPACE
	എങ്കിൽ (z_അട്ടി_is_user_capable(അട്ടി)) {
		geo.obj_size = K_THREAD_അട്ടി_LEN(അട്ടി_size);
		geo.buf_start = K_THREAD_അട്ടി_BUFFER(അട്ടി);
		geo.buf_size = geo.obj_size - K_THREAD_അട്ടി_RESERVED;
	} അല്ലെങ്കിൽ
#endif /* CONFIG_USERSPACE */
	{
		/* Object cannot host a user mode thread */
		geo.obj_size = K_KERNEL_അട്ടി_LEN(അട്ടി_size);
		geo.buf_start = K_KERNEL_അട്ടി_BUFFER(അട്ടി);
		geo.buf_size = geo.obj_size - K_KERNEL_അട്ടി_RESERVED;

#എങ്കിൽ defined(ARCH_KERNEL_അട്ടി_RESERVED)
		/* Zephyr treats stack overflow as an app bug.  But
		 * this particular overflow can be seen by static
		 * analysis so needs to be handled somehow.
		 */
		എങ്കിൽ (K_KERNEL_അട്ടി_RESERVED > geo.obj_size) {
			k_panic();
		}
#endif
	}

	തിരികെ geo;
}

/*
 * Return the initial (high end) stack pointer. When stacks are memory mapped
 * this maps the object into virtual memory, records the mapping so it can be
 * torn down at thread exit, and rebases buf_start onto the mapped address.
 */
സ്ഥിതം അക്ഷരം *map_thread_അട്ടി(ഘടന k_thread *new_thread,
			      k_thread_അട്ടി_t *അട്ടി,
			      ഘടന അട്ടി_geometry *geo)
{
	അക്ഷരം *അട്ടി_ptr;

#ifdef CONFIG_THREAD_അട്ടി_MEM_MAPPED
	/* Map the stack into virtual memory and use that as the base to
	 * calculate the initial stack pointer at the high end of the stack
	 * object. The stack pointer may be reduced later by TLS or random
	 * offset.
	 *
	 * K_MEM_MAP_UNINIT is used to mimic the behavior of non-mapped
	 * stack. If CONFIG_INIT_STACKS is enabled, the stack will be
	 * cleared later.
	 */
	ശൂന്യം *അട്ടി_mapped = k_mem_map_phys_guard((uintptr_t)അട്ടി, geo->obj_size,
				K_MEM_PERM_RW | K_MEM_CACHE_WB | K_MEM_MAP_UNINIT,
				false);

	__ASSERT_NO_MSG((uintptr_t)അട്ടി_mapped != 0);

#ifdef CONFIG_USERSPACE
	എങ്കിൽ (z_അട്ടി_is_user_capable(അട്ടി)) {
		geo->buf_start = K_THREAD_അട്ടി_BUFFER(അട്ടി_mapped);
	} അല്ലെങ്കിൽ
#endif /* CONFIG_USERSPACE */
	{
		geo->buf_start = K_KERNEL_അട്ടി_BUFFER(അട്ടി_mapped);
	}

	അട്ടി_ptr = (അക്ഷരം *)അട്ടി_mapped + geo->obj_size;

	/* Need to store the info on mapped stack so we can remove the mappings
	 * when the thread ends.
	 */
	new_thread->അട്ടി_info.mapped.addr = അട്ടി_mapped;
	new_thread->അട്ടി_info.mapped.sz = geo->obj_size;
#അല്ലെങ്കിൽ /* CONFIG_THREAD_STACK_MEM_MAPPED */
	ARG_UNUSED(new_thread);

	/* Initial stack pointer at the high end of the stack object, may
	 * be reduced later by TLS or random offset
	 */
	അട്ടി_ptr = (അക്ഷരം *)അട്ടി + geo->obj_size;
#endif /* CONFIG_THREAD_STACK_MEM_MAPPED */

	തിരികെ അട്ടി_ptr;
}

/* Pre-fill the stack buffer with a known pattern for usage measurement. */
സ്ഥിതം വരിയിൽ ശൂന്യം fill_init_അട്ടി(അക്ഷരം *buf_start, size_t buf_size)
{
#ifdef CONFIG_INIT_STACKS
	memset(buf_start, 0xaa, buf_size);
#അല്ലെങ്കിൽ
	ARG_UNUSED(buf_start);
	ARG_UNUSED(buf_size);
#endif /* CONFIG_INIT_STACKS */
}

/* Place the overflow-detection sentinel at the low end of the stack buffer. */
സ്ഥിതം വരിയിൽ ശൂന്യം set_അട്ടി_sentinel(അക്ഷരം *buf_start)
{
#ifdef CONFIG_അട്ടി_SENTINEL
	/* Put the stack sentinel at the lowest 4 bytes of the stack area.
	 * We periodically check that it's still present and kill the thread
	 * if it isn't.
	 */
	*((uint32_t *)buf_start) = അട്ടി_SENTINEL;
#അല്ലെങ്കിൽ
	ARG_UNUSED(buf_start);
#endif /* CONFIG_STACK_SENTINEL */
}

/*
 * Compute how much to reserve at the top of the stack buffer for TLS,
 * userspace-local data and stack-pointer randomization, and return the
 * aligned amount the initial stack pointer must be lowered by.
 */
സ്ഥിതം size_t reserve_അട്ടി_headroom(ഘടന k_thread *new_thread,
				     അക്ഷരം *അട്ടി_ptr, size_t buf_size)
{
	size_t delta = 0;

	ARG_UNUSED(new_thread);
	ARG_UNUSED(അട്ടി_ptr);
	ARG_UNUSED(buf_size);

#ifdef CONFIG_THREAD_LOCAL_STORAGE
	/* TLS is always last within the stack buffer */
	delta += arch_tls_അട്ടി_setup(new_thread, അട്ടി_ptr);
#endif /* CONFIG_THREAD_LOCAL_STORAGE */
#ifdef CONFIG_THREAD_USERSPACE_LOCAL_DATA
	size_t tls_size = വലുപ്പം(ഘടന _thread_userspace_local_data);

	/* reserve space on highest memory of stack buffer for local data */
	delta += tls_size;
	new_thread->userspace_local_data =
		(ഘടന _thread_userspace_local_data *)(അട്ടി_ptr - delta);
#endif /* CONFIG_THREAD_USERSPACE_LOCAL_DATA */
#എങ്കിൽ defined(CONFIG_അട്ടി_POINTER_RANDOM) && (CONFIG_അട്ടി_POINTER_RANDOM != 0)
	delta += random_offset(buf_size);
#endif /* CONFIG_STACK_POINTER_RANDOM */

	തിരികെ ROUND_UP(delta, ARCH_അട്ടി_PTR_ALIGN);
}

/* Record the thread-accessible stack bounds (and runtime-safety threshold). */
സ്ഥിതം വരിയിൽ ശൂന്യം set_അട്ടി_info(ഘടന k_thread *new_thread, അക്ഷരം *buf_start,
				  size_t buf_size, size_t delta)
{
#ifdef CONFIG_THREAD_അട്ടി_INFO
	/* Initial values. Arches which implement MPU guards that "borrow"
	 * memory from the stack buffer (not tracked in K_THREAD_STACK_RESERVED)
	 * will need to appropriately update this.
	 *
	 * The bounds tracked here correspond to the area of the stack object
	 * that the thread can access, which includes TLS.
	 */
	new_thread->അട്ടി_info.start = (uintptr_t)buf_start;
	new_thread->അട്ടി_info.size = buf_size;
	new_thread->അട്ടി_info.delta = delta;

#ifdef CONFIG_THREAD_RUNTIME_അട്ടി_SAFETY
	new_thread->അട്ടി_info.usage.unused_threshold =
		(CONFIG_THREAD_RUNTIME_അട്ടി_SAFETY_DEFAULT_UNUSED_THRESHOLD_PCT *
		 buf_size) / 100;
#endif
#അല്ലെങ്കിൽ
	ARG_UNUSED(new_thread);
	ARG_UNUSED(buf_start);
	ARG_UNUSED(buf_size);
	ARG_UNUSED(delta);
#endif /* CONFIG_THREAD_STACK_INFO */
}

സ്ഥിതം അക്ഷരം *setup_thread_അട്ടി(ഘടന k_thread *new_thread,
				k_thread_അട്ടി_t *അട്ടി, size_t അട്ടി_size)
{
	ഘടന അട്ടി_geometry geo = compute_അട്ടി_geometry(അട്ടി, അട്ടി_size);
	അക്ഷരം *അട്ടി_ptr = map_thread_അട്ടി(new_thread, അട്ടി, &geo);
	size_t delta;

	LOG_DBG("stack %p for thread %p: obj_size=%zu buf_start=%p "
		" buf_size %zu stack_ptr=%p",
		അട്ടി, new_thread, geo.obj_size, (ശൂന്യം *)geo.buf_start,
		geo.buf_size, (ശൂന്യം *)അട്ടി_ptr);

	fill_init_അട്ടി(geo.buf_start, geo.buf_size);
	set_അട്ടി_sentinel(geo.buf_start);
	delta = reserve_അട്ടി_headroom(new_thread, അട്ടി_ptr, geo.buf_size);
	set_അട്ടി_info(new_thread, geo.buf_start, geo.buf_size, delta);

	തിരികെ അട്ടി_ptr - delta;
}

#ifdef CONFIG_HW_SHADOW_അട്ടി
സ്ഥിതം ശൂന്യം setup_shadow_അട്ടി(ഘടന k_thread *new_thread,
			 k_thread_അട്ടി_t *അട്ടി)
{
	പൂർണ്ണസംഖ്യ ret = -ENOENT;

	STRUCT_SECTION_FOREACH(അട്ടി_to_hw_shadow_അട്ടി, stk_to_hw_shstk) {
		എങ്കിൽ (stk_to_hw_shstk->അട്ടി == അട്ടി) {
			ret = k_thread_hw_shadow_അട്ടി_attach(new_thread,
							      stk_to_hw_shstk->shstk_addr,
							      stk_to_hw_shstk->size);
			എങ്കിൽ (ret != 0) {
				LOG_ERR("Could not set thread %p shadow stack %p, got error %d",
					new_thread, stk_to_hw_shstk->shstk_addr, ret);
				k_panic();
			}
			നിർത്തുക;
		}
	}

	/* Check if the stack isn't in a stack array, and use the corresponding
	 * shadow stack.
	 */
	എങ്കിൽ (ret != -ENOENT) {
		തിരികെ;
	}

	STRUCT_SECTION_FOREACH(അട്ടി_to_hw_shadow_അട്ടി_arr, stk_to_hw_shstk) {
		എങ്കിൽ ((uintptr_t)അട്ടി >= stk_to_hw_shstk->അട്ടി_addr &&
		    (uintptr_t)അട്ടി < stk_to_hw_shstk->അട്ടി_addr +
		    stk_to_hw_shstk->അട്ടി_size * stk_to_hw_shstk->nmemb) {
			/* Now we have to guess which index of the stack array is being used */
			uintptr_t അട്ടി_offset = (uintptr_t)അട്ടി - stk_to_hw_shstk->അട്ടി_addr;
			uintptr_t അട്ടി_index = അട്ടി_offset / stk_to_hw_shstk->അട്ടി_size;
			uintptr_t addr;

			എങ്കിൽ (അട്ടി_index >= stk_to_hw_shstk->nmemb) {
				LOG_ERR("Could not find shadow stack for thread %p, stack %p",
					new_thread, അട്ടി);
				k_panic();
			}

			addr = stk_to_hw_shstk->shstk_addr +
				stk_to_hw_shstk->shstk_size * അട്ടി_index;
			ret = k_thread_hw_shadow_അട്ടി_attach(new_thread,
							      (arch_thread_hw_shadow_അട്ടി_t *)addr,
							      stk_to_hw_shstk->shstk_size);
			എങ്കിൽ (ret != 0) {
				LOG_ERR("Could not set thread %p shadow stack 0x%lx, got error %d",
					new_thread, stk_to_hw_shstk->shstk_addr, ret);
				k_panic();
			}
			നിർത്തുക;
		}
	}

	എങ്കിൽ (ret == -ENOENT) {
		LOG_ERR("Could not find shadow stack for thread %p, stack %p",
			new_thread, അട്ടി);
		k_panic();
	}
}
#അല്ലെങ്കിൽ
/* No-op stub used when hardware shadow stacks are not configured. */
സ്ഥിതം വരിയിൽ ശൂന്യം setup_shadow_അട്ടി(ഘടന k_thread *new_thread,
				      k_thread_അട്ടി_t *അട്ടി)
{
	ARG_UNUSED(new_thread);
	ARG_UNUSED(അട്ടി);
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
സ്ഥിതം വരിയിൽ ശൂന്യം thread_abort_cleanup_check_reuse(ഘടന k_thread *thread)
{
#ifdef CONFIG_THREAD_ABORT_NEED_CLEANUP
	k_thread_abort_cleanup_check_reuse(thread);
#അല്ലെങ്കിൽ
	ARG_UNUSED(thread);
#endif /* CONFIG_THREAD_ABORT_NEED_CLEANUP */
}

/* Register the thread with the object-core framework and its usage stats. */
സ്ഥിതം വരിയിൽ ശൂന്യം init_thread_obj_core(ഘടന k_thread *thread)
{
#ifdef CONFIG_OBJ_CORE_THREAD
	k_obj_core_init_and_link(K_OBJ_CORE(thread), &obj_type_thread);
#ifdef CONFIG_OBJ_CORE_STATS_THREAD
	k_obj_core_stats_register(K_OBJ_CORE(thread), &thread->base.usage,
				  വലുപ്പം(thread->base.usage));
#endif /* CONFIG_OBJ_CORE_STATS_THREAD */
#അല്ലെങ്കിൽ
	ARG_UNUSED(thread);
#endif /* CONFIG_OBJ_CORE_THREAD */
}

/*
 * Initialize userspace bookkeeping: register the thread and stack kernel
 * objects, record stack ownership, and grant the thread access to itself.
 */
സ്ഥിതം വരിയിൽ ശൂന്യം init_thread_userspace(ഘടന k_thread *thread,
					 k_thread_അട്ടി_t *അട്ടി,
					 uint32_t options)
{
#ifdef CONFIG_USERSPACE
	__ASSERT((options & K_USER) == 0U || z_അട്ടി_is_user_capable(അട്ടി),
		 "user thread %p with kernel-only stack %p", thread, അട്ടി);
	k_object_init(thread);
	k_object_init(അട്ടി);
	thread->അട്ടി_obj = അട്ടി;
	thread->syscall_frame = ശൂന്യ;

	/* Any given thread has access to itself */
	k_object_access_grant(thread, thread);
#അല്ലെങ്കിൽ
	ARG_UNUSED(thread);
	ARG_UNUSED(അട്ടി);
	ARG_UNUSED(options);
#endif /* CONFIG_USERSPACE */
}

/* Assert the thread object is cache-coherent while its stack stays cached. */
സ്ഥിതം വരിയിൽ ശൂന്യം assert_thread_coherence(ഘടന k_thread *thread,
					   k_thread_അട്ടി_t *അട്ടി)
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
	__ASSERT_NO_MSG(!sys_cache_is_mem_coherent(അട്ടി));
#endif /* CONFIG_DYNAMIC_THREAD */
#endif /* CONFIG_KERNEL_COHERENCE */
	ARG_UNUSED(thread);
	ARG_UNUSED(അട്ടി);
}

/* Assert the arch layer populated switch_handle on USE_SWITCH builds. */
സ്ഥിതം വരിയിൽ ശൂന്യം assert_switch_handle(ഘടന k_thread *thread)
{
#ifdef CONFIG_USE_SWITCH
	/* switch_handle must be non-null except when inside z_swap()
	 * for synchronization reasons.  Historically some notional
	 * USE_SWITCH architectures have actually ignored the field
	 */
	__ASSERT(thread->switch_handle != ശൂന്യ,
		 "arch layer failed to initialize switch_handle");
#endif /* CONFIG_USE_SWITCH */
	ARG_UNUSED(thread);
}

/* Clear the opaque, kernel-managed per-thread custom data pointer. */
സ്ഥിതം വരിയിൽ ശൂന്യം init_thread_custom_data(ഘടന k_thread *thread)
{
#ifdef CONFIG_THREAD_CUSTOM_DATA
	/* Initialize custom data field (value is opaque to kernel) */
	thread->custom_data = ശൂന്യ;
#അല്ലെങ്കിൽ
	ARG_UNUSED(thread);
#endif /* CONFIG_THREAD_CUSTOM_DATA */
}

/*
 * Record the entry point and parameters, then link the thread into the
 * global thread-monitor list under the monitor spinlock.
 */
സ്ഥിതം വരിയിൽ ശൂന്യം add_thread_to_monitor(ഘടന k_thread *thread,
					 k_thread_entry_t entry,
					 ശൂന്യം *p1, ശൂന്യം *p2, ശൂന്യം *p3)
{
#ifdef CONFIG_THREAD_MONITOR
	thread->entry.pEntry = entry;
	thread->entry.parameter1 = p1;
	thread->entry.parameter2 = p2;
	thread->entry.parameter3 = p3;

	k_spinlock_താക്കോൽ_t താക്കോൽ = k_spin_lock(&z_thread_monitor_lock);

	thread->next_thread = _kernel.threads;
	_kernel.threads = thread;
	k_spin_unlock(&z_thread_monitor_lock, താക്കോൽ);
#അല്ലെങ്കിൽ
	ARG_UNUSED(thread);
	ARG_UNUSED(entry);
	ARG_UNUSED(p1);
	ARG_UNUSED(p2);
	ARG_UNUSED(p3);
#endif /* CONFIG_THREAD_MONITOR */
}

/* Set the thread's name from the caller-supplied string (may be NULL). */
സ്ഥിതം വരിയിൽ ശൂന്യം init_thread_name(ഘടന k_thread *thread, സ്ഥിരം അക്ഷരം *name)
{
#ifdef CONFIG_THREAD_NAME
	set_thread_name(thread, name);
#അല്ലെങ്കിൽ
	ARG_UNUSED(thread);
	ARG_UNUSED(name);
#endif /* CONFIG_THREAD_NAME */
}

/* Set the thread's initial SMP CPU affinity mask. */
സ്ഥിതം വരിയിൽ ശൂന്യം init_thread_cpu_mask(ഘടന k_thread *thread)
{
#ifdef CONFIG_SCHED_CPU_MASK
	എങ്കിൽ (IS_ENABLED(CONFIG_SCHED_CPU_MASK_PIN_ONLY)) {
		thread->base.cpu_mask = 1; /* must specify only one cpu */
	} അല്ലെങ്കിൽ {
		thread->base.cpu_mask = -1; /* allow all cpus */
	}
#അല്ലെങ്കിൽ
	ARG_UNUSED(thread);
#endif /* CONFIG_SCHED_CPU_MASK */
}

/* Attach the thread to its memory domain and inherit caller permissions
 * when K_INHERIT_PERMS is requested.
 */
സ്ഥിതം വരിയിൽ ശൂന്യം init_thread_userspace_perms(ഘടന k_thread *thread,
					       uint32_t options)
{
#ifdef CONFIG_USERSPACE
	z_mem_domain_init_thread(thread);

	എങ്കിൽ ((options & K_INHERIT_PERMS) != 0U) {
		k_thread_perms_inherit(_current, thread);
	}
#അല്ലെങ്കിൽ
	ARG_UNUSED(thread);
	ARG_UNUSED(options);
#endif /* CONFIG_USERSPACE */
}

/* Clear the EDF scheduling deadline. */
സ്ഥിതം വരിയിൽ ശൂന്യം init_thread_deadline(ഘടന k_thread *thread)
{
#ifdef CONFIG_SCHED_DEADLINE
	thread->base.prio_deadline = 0;
#അല്ലെങ്കിൽ
	ARG_UNUSED(thread);
#endif /* CONFIG_SCHED_DEADLINE */
}

/* Initialize the wait queue used to synchronize SMP thread halt/abort. */
സ്ഥിതം വരിയിൽ ശൂന്യം init_thread_halt_queue(ഘടന k_thread *thread)
{
#ifdef CONFIG_SMP
	z_waitq_init(&thread->halt_queue);
#അല്ലെങ്കിൽ
	ARG_UNUSED(thread);
#endif /* CONFIG_SMP */
}

/* Initialize runtime CPU-usage tracking for the thread. */
സ്ഥിതം വരിയിൽ ശൂന്യം init_thread_usage(ഘടന k_thread *thread)
{
#ifdef CONFIG_SCHED_THREAD_USAGE
	thread->base.usage = (ഘടന k_cycle_stats) {};
	thread->base.usage.track_usage = CONFIG_SCHED_THREAD_USAGE_AUTO_ENABLE;
#അല്ലെങ്കിൽ
	ARG_UNUSED(thread);
#endif /* CONFIG_SCHED_THREAD_USAGE */
}

/* Assert the thread wasn't already linked into the monitor list. */
സ്ഥിതം വരിയിൽ ശൂന്യം assert_thread_not_reused(ഘടന k_thread *new_thread)
{
#ifdef CONFIG_THREAD_MONITOR
#ifdef CONFIG_ASSERT
	k_spinlock_താക്കോൽ_t താക്കോൽ = k_spin_lock(&z_thread_monitor_lock);
	bool reused = false;

	/* Check if the thread is already in the list */
	വേണ്ടി (ഘടന k_thread *t = _kernel.threads; t; t = t->next_thread) {
		എങ്കിൽ (t == new_thread) {
			reused = true;
			നിർത്തുക;
		}
	}

	k_spin_unlock(&z_thread_monitor_lock, താക്കോൽ);

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
അക്ഷരം *z_setup_new_thread(ഘടന k_thread *new_thread,
			 k_thread_അട്ടി_t *അട്ടി, size_t അട്ടി_size,
			 k_thread_entry_t entry,
			 ശൂന്യം *p1, ശൂന്യം *p2, ശൂന്യം *p3,
			 പൂർണ്ണസംഖ്യ prio, uint32_t options, സ്ഥിരം അക്ഷരം *name)
{
	അക്ഷരം *അട്ടി_ptr;

	Z_ASSERT_VALID_PRIO(prio, entry);

	assert_thread_not_reused(new_thread);

	thread_abort_cleanup_check_reuse(new_thread);
	init_thread_obj_core(new_thread);
	init_thread_userspace(new_thread, അട്ടി, options);

	z_waitq_init(&new_thread->join_queue);

	/* Initialize various struct k_thread members */
	z_init_thread_base(&new_thread->base, prio, _THREAD_SLEEPING, options);
	അട്ടി_ptr = setup_thread_അട്ടി(new_thread, അട്ടി, അട്ടി_size);

#എങ്കിൽ Z_MUTEX_PI_ENABLED
	sys_slist_init(&new_thread->held_mutexes);
	new_thread->mutex_pended_on = ശൂന്യ;
	new_thread->orig_prio = new_thread->base.prio;
#endif /* Z_MUTEX_PI_ENABLED */

	setup_shadow_അട്ടി(new_thread, അട്ടി);
	assert_thread_coherence(new_thread, അട്ടി);

	arch_new_thread(new_thread, അട്ടി, അട്ടി_ptr, entry, p1, p2, p3);

	/* static threads overwrite it afterwards with real value */
	new_thread->init_data = ശൂന്യ;

	assert_switch_handle(new_thread);
	init_thread_custom_data(new_thread);
	add_thread_to_monitor(new_thread, entry, p1, p2, p3);
	init_thread_name(new_thread, name);
	init_thread_cpu_mask(new_thread);

	/* _current may be NULL if the dummy thread is not used */
	എങ്കിൽ (IS_ENABLED(CONFIG_ARCH_HAS_CUSTOM_SWAP_TO_MAIN) &&
	    (_current == ശൂന്യ)) {
		new_thread->resource_pool = ശൂന്യ;
		തിരികെ അട്ടി_ptr;
	}

	init_thread_userspace_perms(new_thread, options);
	init_thread_deadline(new_thread);
	new_thread->resource_pool = _current->resource_pool;
	init_thread_halt_queue(new_thread);
	init_thread_usage(new_thread);

	SYS_PORT_TRACING_OBJ_FUNC(k_thread, create, new_thread);

	തിരികെ അട്ടി_ptr;
}

#ifdef CONFIG_THREAD_RUNTIME_അട്ടി_SAFETY
പൂർണ്ണസംഖ്യ z_impl_k_thread_runtime_അട്ടി_unused_threshold_pct_set(ഘടന k_thread *thread,
							   uint32_t pct)
{
	size_t unused_threshold;

	എങ്കിൽ (pct > 99) {
		തിരികെ -EINVAL;   /* 100% unused stack and up is invalid */
	}

	unused_threshold = (thread->അട്ടി_info.size * pct) / 100;

	thread->അട്ടി_info.usage.unused_threshold = unused_threshold;

	തിരികെ 0;
}

പൂർണ്ണസംഖ്യ z_impl_k_thread_runtime_അട്ടി_unused_threshold_set(ഘടന k_thread *thread,
						       size_t threshold)
{
	എങ്കിൽ (threshold > thread->അട്ടി_info.size) {
		തിരികെ -EINVAL;
	}

	thread->അട്ടി_info.usage.unused_threshold = threshold;

	തിരികെ 0;
}

size_t z_impl_k_thread_runtime_അട്ടി_unused_threshold_get(ഘടന k_thread *thread)
{
	തിരികെ thread->അട്ടി_info.usage.unused_threshold;
}

#ifdef CONFIG_USERSPACE
പൂർണ്ണസംഖ്യ z_vrfy_k_thread_runtime_അട്ടി_unused_threshold_pct_set(ഘടന k_thread *thread,
							   uint32_t pct)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));

	തിരികെ z_impl_k_thread_runtime_അട്ടി_unused_threshold_pct_set(thread, pct);
}
#അന്തര്ഭാവ <zephyr/syscalls/k_thread_runtime_stack_unused_threshold_pct_set_mrsh.c>

പൂർണ്ണസംഖ്യ z_vrfy_k_thread_runtime_അട്ടി_unused_threshold_set(ഘടന k_thread *thread,
						       size_t threshold)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));

	തിരികെ z_impl_k_thread_runtime_അട്ടി_unused_threshold_set(thread, threshold);
}
#അന്തര്ഭാവ <zephyr/syscalls/k_thread_runtime_stack_unused_threshold_set_mrsh.c>

size_t z_vrfy_k_thread_runtime_അട്ടി_unused_threshold_get(ഘടന k_thread *thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));

	തിരികെ z_impl_k_thread_runtime_അട്ടി_unused_threshold_get(thread);
}
#അന്തര്ഭാവ <zephyr/syscalls/k_thread_runtime_stack_unused_threshold_get_mrsh.c>
#endif /* CONFIG_USERSPACE */
#endif /* CONFIG_THREAD_RUNTIME_STACK_SAFETY */

k_tid_t z_impl_k_thread_create(ഘടന k_thread *new_thread,
			      k_thread_അട്ടി_t *അട്ടി,
			      size_t അട്ടി_size, k_thread_entry_t entry,
			      ശൂന്യം *p1, ശൂന്യം *p2, ശൂന്യം *p3,
			      പൂർണ്ണസംഖ്യ prio, uint32_t options, k_timeout_t delay)
{
	__ASSERT(!arch_is_in_isr(), "Threads may not be created in ISRs");

	z_setup_new_thread(new_thread, അട്ടി, അട്ടി_size, entry, p1, p2, p3,
			  prio, options, ശൂന്യ);

	thread_schedule_new(new_thread, delay);

	തിരികെ new_thread;
}

#ifdef CONFIG_USERSPACE
bool z_അട്ടി_is_user_capable(k_thread_അട്ടി_t *അട്ടി)
{
	തിരികെ k_object_find(അട്ടി) != ശൂന്യ;
}

k_tid_t z_vrfy_k_thread_create(ഘടന k_thread *new_thread,
			       k_thread_അട്ടി_t *അട്ടി,
			       size_t അട്ടി_size, k_thread_entry_t entry,
			       ശൂന്യം *p1, ശൂന്യം *p2, ശൂന്യം *p3,
			       പൂർണ്ണസംഖ്യ prio, uint32_t options, k_timeout_t delay)
{
	size_t total_size, അട്ടി_obj_size;
	ഘടന k_object *അട്ടി_object;

	/* The thread and stack objects *must* be in an uninitialized state */
	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(new_thread, K_OBJ_THREAD));

	/* No need to check z_stack_is_user_capable(), it won't be in the
	 * object table if it isn't
	 */
	അട്ടി_object = k_object_find(അട്ടി);
	K_OOPS(K_SYSCALL_VERIFY_സന്ദേശം(k_object_validation_check(അട്ടി_object, അട്ടി,
						K_OBJ_THREAD_അട്ടി_ELEMENT,
						_OBJ_INIT_FALSE) == 0,
				    "bad stack object"));

	/* Verify that the stack size passed in is OK by computing the total
	 * size and comparing it with the size value in the object metadata
	 */
	K_OOPS(K_SYSCALL_VERIFY_സന്ദേശം(!size_add_overflow(K_THREAD_അട്ടി_RESERVED,
						       അട്ടി_size, &total_size),
				    "stack size overflow (%zu+%zu)",
				    അട്ടി_size,
				    K_THREAD_അട്ടി_RESERVED));

	/* Testing less-than-or-equal since additional room may have been
	 * allocated for alignment constraints
	 */
#ifdef CONFIG_GEN_PRIV_STACKS
	അട്ടി_obj_size = അട്ടി_object->data.അട്ടി_data->size;
#അല്ലെങ്കിൽ
	അട്ടി_obj_size = അട്ടി_object->data.അട്ടി_size;
#endif /* CONFIG_GEN_PRIV_STACKS */
	K_OOPS(K_SYSCALL_VERIFY_സന്ദേശം(total_size <= അട്ടി_obj_size,
				    "stack size %zu is too big, max is %zu",
				    total_size, അട്ടി_obj_size));

	/* User threads may only create other user threads and they can't
	 * be marked as essential
	 */
	K_OOPS(K_SYSCALL_VERIFY(options & K_USER));
	K_OOPS(K_SYSCALL_VERIFY(!(options & K_ESSENTIAL)));

	/* Check validity of prio argument; must be the same or worse priority
	 * than the caller
	 */
	K_OOPS(K_SYSCALL_VERIFY(_is_valid_prio(prio, ശൂന്യ)));
	K_OOPS(K_SYSCALL_VERIFY(z_is_prio_lower_or_equal(prio,
							_current->base.prio)));

	z_setup_new_thread(new_thread, അട്ടി, അട്ടി_size,
			   entry, p1, p2, p3, prio, options, ശൂന്യ);

	thread_schedule_new(new_thread, delay);

	തിരികെ new_thread;
}
#അന്തര്ഭാവ <zephyr/syscalls/k_thread_create_mrsh.c>
#endif /* CONFIG_USERSPACE */

ശൂന്യം z_init_thread_base(ഘടന _thread_base *thread_base, പൂർണ്ണസംഖ്യ priority,
		       uint32_t initial_state, unsigned പൂർണ്ണസംഖ്യ options)
{
	/* k_q_node is initialized upon first insertion in a list */
	thread_base->pended_on = ശൂന്യ;
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
	thread_base->slice_expired = ശൂന്യ;
#endif /* CONFIG_TIMESLICE_PER_THREAD */

#ifdef CONFIG_SPIN_VALIDATE
	thread_base->swap_data = ശൂന്യ;
	/* otherwise swap_data does not need to be initialized */
#endif

	z_init_thread_timeout(thread_base);
}

FUNC_NORETURN ശൂന്യം k_thread_user_mode_enter(k_thread_entry_t entry,
					    ശൂന്യം *p1, ശൂന്യം *p2, ശൂന്യം *p3)
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
	__ASSERT(z_അട്ടി_is_user_capable(_current->അട്ടി_obj),
		 "dropping to user mode with kernel-only stack object");
#ifdef CONFIG_THREAD_USERSPACE_LOCAL_DATA
	memset(_current->userspace_local_data, 0,
	       വലുപ്പം(ഘടന _thread_userspace_local_data));
#endif /* CONFIG_THREAD_USERSPACE_LOCAL_DATA */
#ifdef CONFIG_THREAD_LOCAL_STORAGE
	arch_tls_അട്ടി_setup(_current,
			     (അക്ഷരം *)(_current->അട്ടി_info.start +
				      _current->അട്ടി_info.size));
#endif /* CONFIG_THREAD_LOCAL_STORAGE */
	arch_user_mode_enter(entry, p1, p2, p3);
#അല്ലെങ്കിൽ
	/* XXX In this case we do not reset the stack */
	z_thread_entry(entry, p1, p2, p3);
#endif /* CONFIG_USERSPACE */
}

#എങ്കിൽ defined(CONFIG_INIT_STACKS) && defined(CONFIG_THREAD_അട്ടി_INFO)
#ifdef CONFIG_അട്ടി_GROWS_UP
#error "Unsupported configuration for stack analysis"
#endif /* CONFIG_STACK_GROWS_UP */

പൂർണ്ണസംഖ്യ z_അട്ടി_space_get(സ്ഥിരം uint8_t *അട്ടി_start, size_t size, size_t *unused_ptr)
{
	size_t unused = 0;
	സ്ഥിരം uint8_t *checked_അട്ടി = അട്ടി_start;
	/* Take the address of any local variable as a shallow bound for the
	 * stack pointer.  Addresses above it are guaranteed to be
	 * accessible.
	 */
	സ്ഥിരം uint8_t *അട്ടി_pointer = (സ്ഥിരം uint8_t *)&അട്ടി_start;

	/* If we are currently running on the stack being analyzed, some
	 * memory management hardware will generate an exception if we
	 * read unused stack memory.
	 *
	 * This never happens when invoked from user mode, as user mode
	 * will always run this function on the privilege elevation stack.
	 */
	എങ്കിൽ ((അട്ടി_pointer > അട്ടി_start) && (അട്ടി_pointer <= (അട്ടി_start + size)) &&
	    IS_ENABLED(CONFIG_NO_UNUSED_അട്ടി_INSPECTION)) {
		/* TODO: We could add an arch_ API call to temporarily
		 * disable the stack checking in the CPU, but this would
		 * need to be properly managed wrt context switches/interrupts
		 */
		തിരികെ -ENOTSUP;
	}

	എങ്കിൽ (IS_ENABLED(CONFIG_അട്ടി_SENTINEL)) {
		/* First 4 bytes of the stack buffer reserved for the
		 * sentinel value, it won't be 0xAAAAAAAA for thread
		 * stacks.
		 *
		 * FIXME: thread->stack_info.start ought to reflect
		 * this!
		 */
		checked_അട്ടി += 4;
		size -= 4;
	}

	വേണ്ടി (size_t i = 0; i < size; i++) {
		എങ്കിൽ ((checked_അട്ടി[i]) == 0xaaU) {
			unused++;
		} അല്ലെങ്കിൽ {
			നിർത്തുക;
		}
	}

	*unused_ptr = unused;

	തിരികെ 0;
}

#ifdef CONFIG_THREAD_RUNTIME_അട്ടി_SAFETY
പൂർണ്ണസംഖ്യ k_thread_runtime_അട്ടി_safety_full_check(സ്ഥിരം ഘടന k_thread *thread,
					     size_t *unused_ptr,
					     k_thread_അട്ടി_safety_handler_t handler,
					     ശൂന്യം *arg)
{
	പൂർണ്ണസംഖ്യ    rv;
	size_t unused_space;

	__ASSERT_NO_MSG(thread != ശൂന്യ);

	rv = z_അട്ടി_space_get((സ്ഥിരം uint8_t *)thread->അട്ടി_info.start,
			       thread->അട്ടി_info.size, &unused_space);

	എങ്കിൽ (rv != 0) {
		തിരികെ rv;
	}

	എങ്കിൽ (unused_ptr != ശൂന്യ) {
		*unused_ptr = unused_space;
	}

	എങ്കിൽ ((unused_space < thread->അട്ടി_info.usage.unused_threshold) &&
	    (handler != ശൂന്യ)) {
		handler(thread, unused_space, arg);
	}

	തിരികെ 0;
}

പൂർണ്ണസംഖ്യ k_thread_runtime_അട്ടി_safety_threshold_check(സ്ഥിരം ഘടന k_thread *thread,
						  size_t *unused_ptr,
						  k_thread_അട്ടി_safety_handler_t handler,
						  ശൂന്യം *arg)
{
	പൂർണ്ണസംഖ്യ    rv;
	size_t unused_space;
	size_t threshold;
	size_t scan_size;

	__ASSERT_NO_MSG(thread != ശൂന്യ);

	threshold = thread->അട്ടി_info.usage.unused_threshold;
	scan_size = threshold;

	/* z_stack_space_get() reserves the first bytes of the stack buffer for
	 * the stack sentinel and shrinks its scan window by that amount. Grow
	 * the requested window by the same reservation so that up to
	 * 'threshold' bytes of the usable stack are actually examined;
	 * otherwise a fully unused window would report fewer than 'threshold'
	 * unused bytes and spuriously trip the threshold.
	 */
	എങ്കിൽ (IS_ENABLED(CONFIG_അട്ടി_SENTINEL) && (threshold != 0U)) {
		scan_size += വലുപ്പം(uint32_t);
	}

	rv = z_അട്ടി_space_get((സ്ഥിരം uint8_t *)thread->അട്ടി_info.start,
			       scan_size, &unused_space);

	എങ്കിൽ (rv != 0) {
		തിരികെ rv;
	}

	എങ്കിൽ (unused_ptr != ശൂന്യ) {
		*unused_ptr = unused_space;
	}

	എങ്കിൽ ((unused_space < threshold) && (handler != ശൂന്യ)) {
		handler(thread, unused_space, arg);
	}

	തിരികെ 0;
}
#endif /* CONFIG_THREAD_RUNTIME_STACK_SAFETY */

പൂർണ്ണസംഖ്യ z_impl_k_thread_അട്ടി_space_get(സ്ഥിരം ഘടന k_thread *thread,
				    size_t *unused_ptr)
{
#ifdef CONFIG_THREAD_അട്ടി_MEM_MAPPED
	എങ്കിൽ (thread->അട്ടി_info.mapped.addr == ശൂന്യ) {
		തിരികെ -EINVAL;
	}
#endif /* CONFIG_THREAD_STACK_MEM_MAPPED */

	തിരികെ z_അട്ടി_space_get((സ്ഥിരം uint8_t *)thread->അട്ടി_info.start,
				 thread->അട്ടി_info.size, unused_ptr);
}

#ifdef CONFIG_USERSPACE
പൂർണ്ണസംഖ്യ z_vrfy_k_thread_അട്ടി_space_get(സ്ഥിരം ഘടന k_thread *thread,
				    size_t *unused_ptr)
{
	size_t unused;
	പൂർണ്ണസംഖ്യ ret;

	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));

	ret = z_impl_k_thread_അട്ടി_space_get(thread, &unused);
	CHECKIF(ret != 0) {
		തിരികെ ret;
	}

	ret = k_usermode_to_copy(unused_ptr, &unused, വലുപ്പം(size_t));
	CHECKIF(ret != 0) {
		തിരികെ ret;
	}

	തിരികെ 0;
}
#അന്തര്ഭാവ <zephyr/syscalls/k_thread_stack_space_get_mrsh.c>
#endif /* CONFIG_USERSPACE */
#endif /* CONFIG_INIT_STACKS && CONFIG_THREAD_STACK_INFO */

#ifdef CONFIG_USERSPACE
സ്ഥിതം വരിയിൽ k_ticks_t z_vrfy_k_thread_timeout_remaining_ticks(
						    സ്ഥിരം ഘടന k_thread *thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	തിരികെ z_impl_k_thread_timeout_remaining_ticks(thread);
}
#അന്തര്ഭാവ <zephyr/syscalls/k_thread_timeout_remaining_ticks_mrsh.c>

സ്ഥിതം വരിയിൽ k_ticks_t z_vrfy_k_thread_timeout_expires_ticks(
						  സ്ഥിരം ഘടന k_thread *thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	തിരികെ z_impl_k_thread_timeout_expires_ticks(thread);
}
#അന്തര്ഭാവ <zephyr/syscalls/k_thread_timeout_expires_ticks_mrsh.c>
#endif /* CONFIG_USERSPACE */

#ifdef CONFIG_INSTRUMENT_THREAD_SWITCHING
ശൂന്യം z_thread_mark_switched_in(ശൂന്യം)
{
#എങ്കിൽ defined(CONFIG_SCHED_THREAD_USAGE) && !defined(CONFIG_USE_SWITCH)
	z_sched_usage_start(_current);
#endif /* CONFIG_SCHED_THREAD_USAGE && !CONFIG_USE_SWITCH */

#ifdef CONFIG_TRACING
	SYS_PORT_TRACING_FUNC(k_thread, switched_in);
#endif /* CONFIG_TRACING */
}

ശൂന്യം z_thread_mark_switched_out(ശൂന്യം)
{
#എങ്കിൽ defined(CONFIG_SCHED_THREAD_USAGE) && !defined(CONFIG_USE_SWITCH)
	z_sched_usage_stop();
#endif /*CONFIG_SCHED_THREAD_USAGE && !CONFIG_USE_SWITCH */

#ifdef CONFIG_TRACING
#ifdef CONFIG_THREAD_LOCAL_STORAGE
	/* Dummy thread won't have TLS set up to run arbitrary code */
	എങ്കിൽ (!_current ||
	    (_current->base.thread_state & _THREAD_DUMMY) != 0) {
		തിരികെ;
	}
#endif /* CONFIG_THREAD_LOCAL_STORAGE */
	SYS_PORT_TRACING_FUNC(k_thread, switched_out);
#endif /* CONFIG_TRACING */
}
#endif /* CONFIG_INSTRUMENT_THREAD_SWITCHING */

പൂർണ്ണസംഖ്യ k_thread_runtime_stats_get(k_tid_t thread,
			       k_thread_runtime_stats_t *stats)
{
	എങ്കിൽ ((thread == ശൂന്യ) || (stats == ശൂന്യ)) {
		തിരികെ -EINVAL;
	}

#ifdef CONFIG_SCHED_THREAD_USAGE
	z_sched_thread_usage(thread, stats);
#അല്ലെങ്കിൽ
	*stats = (k_thread_runtime_stats_t) {};
#endif /* CONFIG_SCHED_THREAD_USAGE */

	തിരികെ 0;
}

പൂർണ്ണസംഖ്യ k_thread_runtime_stats_all_get(k_thread_runtime_stats_t *stats)
{
#ifdef CONFIG_SCHED_THREAD_USAGE_ALL
	k_thread_runtime_stats_t  tmp_stats;
#endif /* CONFIG_SCHED_THREAD_USAGE_ALL */

	എങ്കിൽ (stats == ശൂന്യ) {
		തിരികെ -EINVAL;
	}

	*stats = (k_thread_runtime_stats_t) {};

#ifdef CONFIG_SCHED_THREAD_USAGE_ALL
	/* Retrieve the usage stats for each core and amalgamate them. */

	unsigned പൂർണ്ണസംഖ്യ അക്കം_cpus = arch_അക്കം_cpus();

	വേണ്ടി (unsigned പൂർണ്ണസംഖ്യ i = 0; i < അക്കം_cpus; i++) {
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

	തിരികെ 0;
}

പൂർണ്ണസംഖ്യ k_thread_runtime_stats_cpu_get(പൂർണ്ണസംഖ്യ cpu, k_thread_runtime_stats_t *stats)
{
	എങ്കിൽ (stats == ശൂന്യ) {
		തിരികെ -EINVAL;
	}

	*stats = (k_thread_runtime_stats_t) {};

#ifdef CONFIG_SCHED_THREAD_USAGE_ALL
	CHECKIF((cpu < 0) || ((unsigned പൂർണ്ണസംഖ്യ)cpu >= arch_അക്കം_cpus())) {
		തിരികെ -EINVAL;
	}

	z_sched_cpu_usage(cpu, stats);
#അല്ലെങ്കിൽ
	ARG_UNUSED(cpu);
#endif

	തിരികെ 0;
}

#ifdef CONFIG_THREAD_ABORT_NEED_CLEANUP
/** Pointer to thread which needs to be cleaned up. */
സ്ഥിതം ഘടന k_thread *thread_to_cleanup;

/** Spinlock for thread abort cleanup. */
സ്ഥിതം ഘടന k_spinlock thread_cleanup_lock;

#ifdef CONFIG_THREAD_അട്ടി_MEM_MAPPED
സ്ഥിതം ശൂന്യം *thread_cleanup_അട്ടി_addr;
സ്ഥിതം size_t thread_cleanup_അട്ടി_sz;

സ്ഥിതം ശൂന്യം unmap_thread_അട്ടി(ശൂന്യം *addr, size_t sz)
{
	എങ്കിൽ (addr != ശൂന്യ) {
		k_mem_unmap_phys_guard(addr, sz, false);
	}
}
#endif /* CONFIG_THREAD_STACK_MEM_MAPPED */

ശൂന്യം defer_thread_cleanup(ഘടന k_thread *thread)
{
	/* Note when adding new deferred cleanup steps:
	 * - The thread object may have been overwritten by the time
	 *   the actual cleanup is being done (e.g. thread object
	 *   allocated on a stack). So stash any necessary data here
	 *   that will be used in the actual cleanup steps.
	 */
	thread_to_cleanup = thread;

#ifdef CONFIG_THREAD_അട്ടി_MEM_MAPPED
	/* Note that the permission of the stack should have been
	 * stripped of user thread access due to the thread having
	 * already exited from a memory domain. That is done via
	 * k_thread_abort().
	 */

	/* Stash the address and size so the region can be unmapped
	 * later.
	 */
	thread_cleanup_അട്ടി_addr = thread->അട്ടി_info.mapped.addr;
	thread_cleanup_അട്ടി_sz = thread->അട്ടി_info.mapped.sz;

	/* The stack is now considered un-usable. This should prevent any functions
	 * from looking directly into the mapped stack if they are made to be aware
	 * of memory mapped stacks, e.g., z_stack_space_get().
	 */
	thread->അട്ടി_info.mapped.addr = ശൂന്യ;
	thread->അട്ടി_info.mapped.sz = 0;
#endif /* CONFIG_THREAD_STACK_MEM_MAPPED */
}

ശൂന്യം do_deferred_thread_cleanup(ശൂന്യം)
{
	/* Note when adding new actual cleanup steps:
	 * - The thread object may have been overwritten when this is
	 *   called. So only data stashed by defer_thread_cleanup() can
	 *   be used here, never anything from the thread object.
	 */

#ifdef CONFIG_THREAD_അട്ടി_MEM_MAPPED
	unmap_thread_അട്ടി(thread_cleanup_അട്ടി_addr, thread_cleanup_അട്ടി_sz);

	thread_cleanup_അട്ടി_addr = ശൂന്യ;
	thread_cleanup_അട്ടി_sz = 0;
#endif /* CONFIG_THREAD_STACK_MEM_MAPPED */
}

ശൂന്യം do_thread_cleanup(ഘടന k_thread *thread)
{
	/* This is only for threads which are no longer running and are not
	 * the current thread, so the thread object is still valid here and
	 * the cleanup can be driven from it directly.
	 */

#ifdef CONFIG_THREAD_അട്ടി_MEM_MAPPED
	unmap_thread_അട്ടി(thread->അട്ടി_info.mapped.addr,
			   thread->അട്ടി_info.mapped.sz);

	/* The stack is now unmapped and thus un-usable. Clear the mapping
	 * info so nothing looks into the stack afterwards, and so that
	 * a repeated cleanup of the same thread object is a no-op,
	 * e.g., z_stack_space_get().
	 */
	thread->അട്ടി_info.mapped.addr = ശൂന്യ;
	thread->അട്ടി_info.mapped.sz = 0;
#അല്ലെങ്കിൽ /* CONFIG_THREAD_STACK_MEM_MAPPED */
	ARG_UNUSED(thread);
#endif /* CONFIG_THREAD_STACK_MEM_MAPPED */
}

ശൂന്യം k_thread_abort_cleanup(ഘടന k_thread *thread)
{
	K_SPINLOCK(&thread_cleanup_lock) {
		എങ്കിൽ (thread_to_cleanup != ശൂന്യ) {
			/* Finish the pending one first. */
			do_deferred_thread_cleanup();
			thread_to_cleanup = ശൂന്യ;
		}

		എങ്കിൽ (thread == _current) {
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
		} അല്ലെങ്കിൽ {
			/* Not the current running thread, so we are safe to do
			 * cleanups.
			 */
			do_thread_cleanup(thread);
		}
	}
}

ശൂന്യം k_thread_abort_cleanup_check_reuse(ഘടന k_thread *thread)
{
	K_SPINLOCK(&thread_cleanup_lock) {
		/* This is to guard reuse of the same thread object and make sure
		 * any pending cleanups of it needs to be finished before the thread
		 * object can be reused.
		 */
		എങ്കിൽ (thread_to_cleanup == thread) {
			do_deferred_thread_cleanup();
			thread_to_cleanup = ശൂന്യ;
		}
	}
}

#endif /* CONFIG_THREAD_ABORT_NEED_CLEANUP */

ശൂന്യം z_dummy_thread_init(ഘടന k_thread *dummy_thread)
{
	dummy_thread->base.thread_state = _THREAD_DUMMY;
#ifdef CONFIG_SCHED_CPU_MASK
	dummy_thread->base.cpu_mask = -1;
#endif /* CONFIG_SCHED_CPU_MASK */
	dummy_thread->base.user_options = K_ESSENTIAL;
#എങ്കിൽ Z_MUTEX_PI_ENABLED
	sys_slist_init(&dummy_thread->held_mutexes);
	dummy_thread->mutex_pended_on = ശൂന്യ;
	dummy_thread->orig_prio = dummy_thread->base.prio;
#endif /* Z_MUTEX_PI_ENABLED */
#ifdef CONFIG_THREAD_അട്ടി_INFO
	dummy_thread->അട്ടി_info.start = 0U;
	dummy_thread->അട്ടി_info.size = 0U;
#endif /* CONFIG_THREAD_STACK_INFO */
#ifdef CONFIG_USERSPACE
	dummy_thread->mem_domain_info.mem_domain = &k_mem_domain_default;
#endif /* CONFIG_USERSPACE */
#എങ്കിൽ (K_HEAP_MEM_POOL_SIZE > 0)
	k_thread_system_pool_assign(dummy_thread);
#അല്ലെങ്കിൽ
	dummy_thread->resource_pool = ശൂന്യ;
#endif /* K_HEAP_MEM_POOL_SIZE */

#ifdef CONFIG_USE_SWITCH
	dummy_thread->switch_handle = ശൂന്യ;
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

ശൂന്യം z_impl_k_thread_priority_set(k_tid_t thread, പൂർണ്ണസംഖ്യ prio)
{
	/*
	 * Use NULL, since we cannot know what the entry point is (we do not
	 * keep track of it) and idle cannot change its priority.
	 */
	Z_ASSERT_VALID_PRIO(prio, ശൂന്യ);

	bool need_sched = z_thread_prio_set((ഘടന k_thread *)thread, prio);

	എങ്കിൽ ((need_sched) && (IS_ENABLED(CONFIG_SMP) ||
			     (_current->base.sched_locked == 0U))) {
		z_reschedule_unlocked();
	}
}

#ifdef CONFIG_USERSPACE
സ്ഥിതം വരിയിൽ ശൂന്യം z_vrfy_k_thread_priority_set(k_tid_t thread, പൂർണ്ണസംഖ്യ prio)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	K_OOPS(K_SYSCALL_VERIFY_സന്ദേശം(_is_valid_prio(prio, ശൂന്യ),
				    "invalid thread priority %d", prio));
#ifndef CONFIG_USERSPACE_THREAD_MAY_RAISE_PRIORITY
	K_OOPS(K_SYSCALL_VERIFY_സന്ദേശം((int8_t)prio >= thread->base.prio,
				    "thread priority may only be downgraded (%d < %d)",
				    prio, thread->base.prio));
#endif /* CONFIG_USERSPACE_THREAD_MAY_RAISE_PRIORITY */
	z_impl_k_thread_priority_set(thread, prio);
}
#അന്തര്ഭാവ <zephyr/syscalls/k_thread_priority_set_mrsh.c>
#endif /* CONFIG_USERSPACE */

ശൂന്യം z_impl_k_thread_suspend(k_tid_t thread)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_thread, suspend, thread);

	/* Special case "suspend the current thread" as it doesn't
	 * need the async complexity below.
	 */
	എങ്കിൽ (!IS_ENABLED(CONFIG_SMP) &&
	    likely((thread == _current) && !arch_is_in_isr())) {
		z_thread_suspend_current(thread);
		തിരികെ;
	}

	k_spinlock_താക്കോൽ_t താക്കോൽ = z_sched_spinlock_lock();

	എങ്കിൽ (unlikely(z_is_thread_suspended(thread))) {

		/* The target thread is already suspended. Nothing to do. */

		z_sched_spinlock_unlock(താക്കോൽ);
		തിരികെ;
	}

	z_thread_halt(thread, താക്കോൽ, false);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_thread, suspend, thread);
}

#ifdef CONFIG_USERSPACE
സ്ഥിതം വരിയിൽ ശൂന്യം z_vrfy_k_thread_suspend(k_tid_t thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	z_impl_k_thread_suspend(thread);
}
#അന്തര്ഭാവ <zephyr/syscalls/k_thread_suspend_mrsh.c>
#endif /* CONFIG_USERSPACE */

ശൂന്യം z_impl_k_thread_resume(k_tid_t thread)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_thread, resume, thread);

	k_spinlock_താക്കോൽ_t താക്കോൽ = z_sched_spinlock_lock();

	/* Do not try to resume a thread that was not suspended */
	എങ്കിൽ (unlikely(!z_is_thread_suspended(thread))) {
		z_sched_spinlock_unlock(താക്കോൽ);
		തിരികെ;
	}

	z_mark_thread_as_not_suspended(thread);
	z_sched_ready_locked(thread);

	z_reschedule_locked(താക്കോൽ);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_thread, resume, thread);
}

#ifdef CONFIG_USERSPACE
സ്ഥിതം വരിയിൽ ശൂന്യം z_vrfy_k_thread_resume(k_tid_t thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	z_impl_k_thread_resume(thread);
}
#അന്തര്ഭാവ <zephyr/syscalls/k_thread_resume_mrsh.c>
#endif /* CONFIG_USERSPACE */

ശൂന്യം z_impl_k_wakeup(k_tid_t thread)
{
	SYS_PORT_TRACING_OBJ_FUNC(k_thread, wakeup, thread);

	k_spinlock_താക്കോൽ_t താക്കോൽ = z_sched_spinlock_lock();

	എങ്കിൽ (z_is_thread_sleeping(thread)) {
		(ശൂന്യം)z_try_abort_thread_timeout(thread);
		z_mark_thread_as_not_sleeping(thread);
		z_sched_ready_locked(thread);
		z_reschedule_locked(താക്കോൽ);
	} അല്ലെങ്കിൽ {
		z_sched_spinlock_unlock(താക്കോൽ);
	}
}

#ifdef CONFIG_USERSPACE
സ്ഥിതം വരിയിൽ ശൂന്യം z_vrfy_k_wakeup(k_tid_t thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	z_impl_k_wakeup(thread);
}
#അന്തര്ഭാവ <zephyr/syscalls/k_wakeup_mrsh.c>
#endif /* CONFIG_USERSPACE */

ശൂന്യം z_thread_abort(ഘടന k_thread *thread)
{
	bool essential = z_is_thread_essential(thread);
	k_spinlock_താക്കോൽ_t താക്കോൽ = z_sched_spinlock_lock();

	എങ്കിൽ (z_is_thread_dead(thread)) {
		z_sched_spinlock_unlock(താക്കോൽ);
		തിരികെ;
	}

	z_thread_halt(thread, താക്കോൽ, true);

	എങ്കിൽ (essential) {
		__ASSERT(!essential, "aborted essential thread %p", thread);
		k_panic();
	}
}

#എങ്കിൽ !defined(CONFIG_ARCH_HAS_THREAD_ABORT)
ശൂന്യം z_impl_k_thread_abort(k_tid_t thread)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_thread, abort, thread);

	z_thread_abort(thread);

	__ASSERT_NO_MSG(z_is_thread_dead(thread));

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_thread, abort, thread);
}
#endif /* !CONFIG_ARCH_HAS_THREAD_ABORT */

പൂർണ്ണസംഖ്യ z_impl_k_thread_join(ഘടന k_thread *thread, k_timeout_t timeout)
{
	k_spinlock_താക്കോൽ_t താക്കോൽ = z_sched_spinlock_lock();
	പൂർണ്ണസംഖ്യ ret;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_thread, join, thread, timeout);

	എങ്കിൽ (z_is_thread_dead(thread)) {
		z_sched_switch_spin(thread);
		ret = 0;
	} അല്ലെങ്കിൽ എങ്കിൽ (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		ret = -EBUSY;
	} അല്ലെങ്കിൽ എങ്കിൽ ((thread == _current) ||
		   (thread->base.pended_on == &_current->join_queue)) {
		ret = -EDEADLK;
	} അല്ലെങ്കിൽ {
		__ASSERT(!arch_is_in_isr(), "cannot join in ISR");
		z_sched_add_to_waitq_locked(_current, &thread->join_queue);
		z_add_thread_timeout(_current, timeout);

		SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_thread, join, thread, timeout);
		ret = z_swap_locked(താക്കോൽ);
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_thread, join, thread, timeout, ret);

		തിരികെ ret;
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_thread, join, thread, timeout, ret);

	z_sched_spinlock_unlock(താക്കോൽ);
	തിരികെ ret;
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
സ്ഥിതം bool thread_obj_validate(ഘടന k_thread *thread)
{
	ഘടന k_object *ko = k_object_find(thread);
	പൂർണ്ണസംഖ്യ ret = k_object_validate(ko, K_OBJ_THREAD, _OBJ_INIT_TRUE);

	തിരഞ്ഞെടുപ്പ് (ret) {
	അവസ്ഥ 0:
		തിരികെ false;
	അവസ്ഥ -EINVAL:
		തിരികെ true;
	സ്ഥിരസ്ഥിതി:
#ifdef CONFIG_LOG
		k_object_dump_error(ret, thread, ko, K_OBJ_THREAD);
#endif /* CONFIG_LOG */
		/* ret is a non-zero error code here (the 0 and -EINVAL cases
		 * are handled above), so this branch must always oops. Passing
		 * ret as the "verify" expression would treat the failure code as
		 * success and fall through to CODE_UNREACHABLE; verify ret == 0
		 * so the oops is actually raised.
		 */
		K_OOPS(K_SYSCALL_VERIFY_സന്ദേശം(ret == 0, "access denied"));
	}
	CODE_UNREACHABLE; /* LCOV_EXCL_LINE */
}

സ്ഥിതം വരിയിൽ പൂർണ്ണസംഖ്യ z_vrfy_k_thread_join(ഘടന k_thread *thread,
				       k_timeout_t timeout)
{
	എങ്കിൽ (thread_obj_validate(thread)) {
		തിരികെ 0;
	}

	തിരികെ z_impl_k_thread_join(thread, timeout);
}
#അന്തര്ഭാവ <zephyr/syscalls/k_thread_join_mrsh.c>

സ്ഥിതം വരിയിൽ ശൂന്യം z_vrfy_k_thread_abort(k_tid_t thread)
{
	എങ്കിൽ (thread_obj_validate(thread)) {
		തിരികെ;
	}

	K_OOPS(K_SYSCALL_VERIFY_സന്ദേശം(!z_is_thread_essential(thread),
				    "aborting essential thread %p", thread));

	z_impl_k_thread_abort((ഘടന k_thread *)thread);
}
#അന്തര്ഭാവ <zephyr/syscalls/k_thread_abort_mrsh.c>
#endif /* CONFIG_USERSPACE */

bool k_can_yield(ശൂന്യം)
{
	തിരികെ !(k_is_pre_kernel() || k_is_in_isr() || !arch_cpu_irqs_are_enabled() ||
		 z_is_idle_thread_object(_current));
}

ശൂന്യം z_impl_k_yield(ശൂന്യം)
{
	__ASSERT(!arch_is_in_isr(), "");

	SYS_PORT_TRACING_FUNC(k_thread, yield);

	z_sched_yield();
}

#ifdef CONFIG_USERSPACE
സ്ഥിതം വരിയിൽ ശൂന്യം z_vrfy_k_yield(ശൂന്യം)
{
	z_impl_k_yield();
}
#അന്തര്ഭാവ <zephyr/syscalls/k_yield_mrsh.c>
#endif /* CONFIG_USERSPACE */
