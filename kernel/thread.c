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

#அந்தர்பாவ <zephyr/kernel.h>
#அந்தர்பாவ <zephyr/spinlock.h>
#அந்தர்பாவ <zephyr/sys/math_extras.h>
#அந்தர்பாவ <zephyr/sys/clock.h>
#அந்தர்பாவ <ksched.h>
#அந்தர்பாவ <kthread.h>
#அந்தர்பாவ <wait_q.h>
#அந்தர்பாவ <timeout_q.h>
#அந்தர்பாவ <zephyr/internal/syscall_handler.h>
#அந்தர்பாவ <kernel_internal.h>
#அந்தர்பாவ <kswap.h>
#அந்தர்பாவ <zephyr/init.h>
#அந்தர்பாவ <zephyr/tracing/tracing.h>
#அந்தர்பாவ <string.h>
#அந்தர்பாவ <stdbool.h>
#அந்தர்பாவ <zephyr/sys/check.h>
#அந்தர்பாவ <zephyr/random/random.h>
#அந்தர்பாவ <zephyr/sys/atomic.h>
#அந்தர்பாவ <zephyr/logging/log.h>
#அந்தர்பாவ <zephyr/llext/symbol.h>
#அந்தர்பாவ <zephyr/sys/iterable_sections.h>
#அந்தர்பாவ <zephyr/sys/minmax.h>

#அந்தர்பாவ <usage.h>

LOG_MODULE_DECLARE(os, CONFIG_KERNEL_LOG_LEVEL);

#ifdef CONFIG_OBJ_CORE_THREAD
#ifdef CONFIG_OBJ_CORE_STATS_THREAD
நிலையான கட்டமைப்பு k_obj_core_stats_desc  thread_stats_desc = {
	.raw_size = அளவு(கட்டமைப்பு k_cycle_stats),
	.query_size = அளவு(கட்டமைப்பு k_thread_runtime_stats),
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
#இல்லையெனில்
K_OBJ_TYPE_DEFINE_TYPE_ONLY(obj_type_thread, k_thread, K_OBJ_TYPE_THREAD_ID,
			    வெற்று);
#endif /* CONFIG_OBJ_CORE_STATS_THREAD */
#endif /* CONFIG_OBJ_CORE_THREAD */


#define _FOREACH_STATIC_THREAD(thread_data)              \
	STRUCT_SECTION_FOREACH(_static_thread_data, thread_data)

bool k_is_in_isr(வெறுமை)
{
	திரும்பு arch_is_in_isr();
}
EXPORT_குறியீடு(k_is_in_isr);

#ifdef CONFIG_THREAD_CUSTOM_DATA
வெறுமை z_impl_k_thread_custom_data_set(வெறுமை *மதிப்பு)
{
	_current->custom_data = மதிப்பு;
}

#ifdef CONFIG_USERSPACE
நிலையான உள்வரி வெறுமை z_vrfy_k_thread_custom_data_set(வெறுமை *data)
{
	z_impl_k_thread_custom_data_set(data);
}
#அந்தர்பாவ <zephyr/syscalls/k_thread_custom_data_set_mrsh.c>
#endif /* CONFIG_USERSPACE */

வெறுமை *z_impl_k_thread_custom_data_get(வெறுமை)
{
	திரும்பு _current->custom_data;
}

#ifdef CONFIG_USERSPACE
நிலையான உள்வரி வெறுமை *z_vrfy_k_thread_custom_data_get(வெறுமை)
{
	திரும்பு z_impl_k_thread_custom_data_get();
}
#அந்தர்பாவ <zephyr/syscalls/k_thread_custom_data_get_mrsh.c>

#endif /* CONFIG_USERSPACE */
#endif /* CONFIG_THREAD_CUSTOM_DATA */

முழு z_impl_k_is_preempt_thread(வெறுமை)
{
	திரும்பு !arch_is_in_isr() && thread_is_preemptible(_current);
}

#ifdef CONFIG_USERSPACE
நிலையான உள்வரி முழு z_vrfy_k_is_preempt_thread(வெறுமை)
{
	திரும்பு z_impl_k_is_preempt_thread();
}
#அந்தர்பாவ <zephyr/syscalls/k_is_preempt_thread_mrsh.c>
#endif /* CONFIG_USERSPACE */

முழு z_impl_k_thread_priority_get(k_tid_t thread)
{
	திரும்பு thread->base.prio;
}

#ifdef CONFIG_USERSPACE
நிலையான உள்வரி முழு z_vrfy_k_thread_priority_get(k_tid_t thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	திரும்பு z_impl_k_thread_priority_get(thread);
}
#அந்தர்பாவ <zephyr/syscalls/k_thread_priority_get_mrsh.c>
#endif /* CONFIG_USERSPACE */

#ifdef CONFIG_THREAD_NAME
நிலையான வெறுமை set_thread_name(கட்டமைப்பு k_thread *thread, மாறிலி எழுத்து *str)
{
	எனில் (str != வெற்று) {
		strncpy(thread->name, str, CONFIG_THREAD_MAX_NAME_LEN - 1);
		/* Ensure NULL termination, truncate if longer */
		thread->name[CONFIG_THREAD_MAX_NAME_LEN - 1] = '\0';
#ifdef CONFIG_ARCH_HAS_THREAD_NAME_HOOK
		arch_thread_name_set(thread, str);
#endif /* CONFIG_ARCH_HAS_THREAD_NAME_HOOK */
	} இல்லையெனில் {
		thread->name[0] = '\0';
	}
}
#endif /* CONFIG_THREAD_NAME */

முழு z_impl_k_thread_name_set(k_tid_t thread, மாறிலி எழுத்து *str)
{
#ifdef CONFIG_THREAD_NAME
	எனில் (thread == வெற்று) {
		thread = _current;
	}

	set_thread_name(thread, str);

	SYS_PORT_TRACING_OBJ_FUNC(k_thread, name_set, thread, 0);

	திரும்பு 0;
#இல்லையெனில்
	ARG_UNUSED(thread);
	ARG_UNUSED(str);

	SYS_PORT_TRACING_OBJ_FUNC(k_thread, name_set, thread, -ENOSYS);

	திரும்பு -ENOSYS;
#endif /* CONFIG_THREAD_NAME */
}

#ifdef CONFIG_USERSPACE
நிலையான உள்வரி முழு z_vrfy_k_thread_name_set(k_tid_t thread, மாறிலி எழுத்து *str)
{
#ifdef CONFIG_THREAD_NAME
	எழுத்து name[CONFIG_THREAD_MAX_NAME_LEN];

	எனில் (thread != வெற்று) {
		எனில் (K_SYSCALL_OBJ(thread, K_OBJ_THREAD) != 0) {
			திரும்பு -EINVAL;
		}
	}

	/* In theory we could copy directly into thread->name, but
	 * the current z_vrfy / z_impl split does not provide a
	 * means of doing so.
	 */
	எனில் (k_usermode_string_copy(name, str, அளவு(name)) != 0) {
		திரும்பு -EFAULT;
	}

	திரும்பு z_impl_k_thread_name_set(thread, name);
#இல்லையெனில்
	திரும்பு -ENOSYS;
#endif /* CONFIG_THREAD_NAME */
}
#அந்தர்பாவ <zephyr/syscalls/k_thread_name_set_mrsh.c>
#endif /* CONFIG_USERSPACE */

மாறிலி எழுத்து *k_thread_name_get(k_tid_t thread)
{
#ifdef CONFIG_THREAD_NAME
	திரும்பு (மாறிலி எழுத்து *)thread->name;
#இல்லையெனில்
	ARG_UNUSED(thread);
	திரும்பு வெற்று;
#endif /* CONFIG_THREAD_NAME */
}

முழு z_impl_k_thread_name_copy(k_tid_t thread, எழுத்து *buf, size_t size)
{
#ifdef CONFIG_THREAD_NAME
	strncpy(buf, thread->name, size);
	திரும்பு 0;
#இல்லையெனில்
	ARG_UNUSED(thread);
	ARG_UNUSED(buf);
	ARG_UNUSED(size);
	திரும்பு -ENOSYS;
#endif /* CONFIG_THREAD_NAME */
}

நிலையான size_t copy_bytes(எழுத்து *dest, size_t dest_size, மாறிலி எழுத்து *src, size_t src_size)
{
	size_t  bytes_to_copy;

	bytes_to_copy = min(dest_size, src_size);
	memcpy(dest, src, bytes_to_copy);

	திரும்பு bytes_to_copy;
}

மாறிலி எழுத்து *k_thread_state_str(k_tid_t thread_id, எழுத்து *buf, size_t buf_size)
{
	size_t      off = 0;
	uint8_t     bit;
	uint8_t     thread_state = thread_id->base.thread_state;
#define SS_ENT(s) { Z_STATE_STR_##s, _THREAD_##s, அளவு(Z_STATE_STR_##s) - 1 }
	நிலையான மாறிலி கட்டமைப்பு {
		மாறிலி எழுத்து *str;
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

	எனில் ((buf == வெற்று) || (buf_size == 0)) {
		திரும்பு "";
	}

	buf_size--;   /* Reserve 1 byte for end-of-string character */

	/*
	 * Loop through each bit in the thread_state. Stop once all have
	 * been processed. If more than one thread_state bit is set, then
	 * separate the descriptive strings with a '+'.
	 */


	க்கு (unsigned முழு index = 0; thread_state != 0; index++) {
		bit = state_string[index].bit;
		எனில் ((thread_state & bit) == 0) {
			தொடர்;
		}

		off += copy_bytes(buf + off, buf_size - off,
				  state_string[index].str,
				  state_string[index].len);

		thread_state &= ~bit;

		எனில் (thread_state != 0) {
			off += copy_bytes(buf + off, buf_size - off, "+", 1);
		}
	}

	buf[off] = '\0';

	திரும்பு (மாறிலி எழுத்து *)buf;
}

#ifdef CONFIG_USERSPACE
நிலையான உள்வரி முழு z_vrfy_k_thread_name_copy(k_tid_t thread,
					    எழுத்து *buf, size_t size)
{
#ifdef CONFIG_THREAD_NAME
	size_t len;
	கட்டமைப்பு k_object *ko = k_object_find(thread);

	/* Special case: we allow reading the names of initialized threads
	 * even if we don't have permission on them
	 */
	எனில் ((ko == வெற்று) || (ko->type != K_OBJ_THREAD) ||
		((ko->flags & K_OBJ_FLAG_INITIALIZED) == 0)) {
		திரும்பு -EINVAL;
	}
	எனில் (K_SYSCALL_MEMORY_WRITE(buf, size) != 0) {
		திரும்பு -EFAULT;
	}
	len = strlen(thread->name);
	எனில் ((len + 1) > size) {
		திரும்பு -ENOSPC;
	}

	திரும்பு k_usermode_to_copy((வெறுமை *)buf, thread->name, len + 1);
#இல்லையெனில்
	ARG_UNUSED(thread);
	ARG_UNUSED(buf);
	ARG_UNUSED(size);
	திரும்பு -ENOSYS;
#endif /* CONFIG_THREAD_NAME */
}
#அந்தர்பாவ <zephyr/syscalls/k_thread_name_copy_mrsh.c>
#endif /* CONFIG_USERSPACE */

#ifdef CONFIG_அடுக்கு_SENTINEL
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
வெறுமை z_check_அடுக்கு_sentinel(வெறுமை)
{
	uint32_t *அடுக்கு;

	எனில் ((_current->base.thread_state & _THREAD_DUMMY) != 0) {
		திரும்பு;
	}

	அடுக்கு = (uint32_t *)_current->அடுக்கு_info.start;
	எனில் (*அடுக்கு != அடுக்கு_SENTINEL) {
		/* Restore it so further checks don't trigger this same error */
		*அடுக்கு = அடுக்கு_SENTINEL;
		z_except_reason(K_ERR_அடுக்கு_CHK_FAIL);
	}
}
#endif /* CONFIG_STACK_SENTINEL */

#எனில் defined(CONFIG_அடுக்கு_POINTER_RANDOM) && (CONFIG_அடுக்கு_POINTER_RANDOM != 0)
முழு z_அடுக்கு_adjust_initialized;

நிலையான size_t random_offset(size_t அடுக்கு_size)
{
	size_t random_val;

	எனில் (!z_அடுக்கு_adjust_initialized) {
		z_early_rand_get((uint8_t *)&random_val, அளவு(random_val));
	} இல்லையெனில் {
		sys_rand_get((uint8_t *)&random_val, அளவு(random_val));
	}

	/* Don't need to worry about alignment of the size here,
	 * arch_new_thread() is required to do it.
	 *
	 * FIXME: Not the best way to get a random number in a range.
	 * See #6493
	 */
	மாறிலி size_t fuzz = random_val % CONFIG_அடுக்கு_POINTER_RANDOM;

	எனில் (unlikely(fuzz * 2 > அடுக்கு_size)) {
		திரும்பு 0;
	}

	திரும்பு fuzz;
}
#எனில் defined(CONFIG_அடுக்கு_GROWS_UP)
	/* This is so rare not bothering for now */
#error "Stack pointer randomization not implemented for upward growing stacks"
#endif /* CONFIG_STACK_GROWS_UP */
#endif /* CONFIG_STACK_POINTER_RANDOM */

/*
 * Geometry of a thread's stack object, computed once and threaded through the
 * setup_thread_stack() helpers below.
 */
கட்டமைப்பு அடுக்கு_geometry {
	size_t obj_size;	/* size of the whole stack object */
	எழுத்து *buf_start;	/* start of the usable stack buffer */
	size_t buf_size;	/* size of the usable stack buffer */
};

/*
 * Derive the stack object geometry from the caller-supplied stack and size.
 * User-capable stacks and kernel-only stacks reserve different amounts.
 */
நிலையான கட்டமைப்பு அடுக்கு_geometry compute_அடுக்கு_geometry(k_thread_அடுக்கு_t *அடுக்கு,
						    size_t அடுக்கு_size)
{
	கட்டமைப்பு அடுக்கு_geometry geo;

#ifdef CONFIG_USERSPACE
	எனில் (z_அடுக்கு_is_user_capable(அடுக்கு)) {
		geo.obj_size = K_THREAD_அடுக்கு_LEN(அடுக்கு_size);
		geo.buf_start = K_THREAD_அடுக்கு_BUFFER(அடுக்கு);
		geo.buf_size = geo.obj_size - K_THREAD_அடுக்கு_RESERVED;
	} இல்லையெனில்
#endif /* CONFIG_USERSPACE */
	{
		/* Object cannot host a user mode thread */
		geo.obj_size = K_KERNEL_அடுக்கு_LEN(அடுக்கு_size);
		geo.buf_start = K_KERNEL_அடுக்கு_BUFFER(அடுக்கு);
		geo.buf_size = geo.obj_size - K_KERNEL_அடுக்கு_RESERVED;

#எனில் defined(ARCH_KERNEL_அடுக்கு_RESERVED)
		/* Zephyr treats stack overflow as an app bug.  But
		 * this particular overflow can be seen by static
		 * analysis so needs to be handled somehow.
		 */
		எனில் (K_KERNEL_அடுக்கு_RESERVED > geo.obj_size) {
			k_panic();
		}
#endif
	}

	திரும்பு geo;
}

/*
 * Return the initial (high end) stack pointer. When stacks are memory mapped
 * this maps the object into virtual memory, records the mapping so it can be
 * torn down at thread exit, and rebases buf_start onto the mapped address.
 */
நிலையான எழுத்து *map_thread_அடுக்கு(கட்டமைப்பு k_thread *new_thread,
			      k_thread_அடுக்கு_t *அடுக்கு,
			      கட்டமைப்பு அடுக்கு_geometry *geo)
{
	எழுத்து *அடுக்கு_ptr;

#ifdef CONFIG_THREAD_அடுக்கு_MEM_MAPPED
	/* Map the stack into virtual memory and use that as the base to
	 * calculate the initial stack pointer at the high end of the stack
	 * object. The stack pointer may be reduced later by TLS or random
	 * offset.
	 *
	 * K_MEM_MAP_UNINIT is used to mimic the behavior of non-mapped
	 * stack. If CONFIG_INIT_STACKS is enabled, the stack will be
	 * cleared later.
	 */
	வெறுமை *அடுக்கு_mapped = k_mem_map_phys_guard((uintptr_t)அடுக்கு, geo->obj_size,
				K_MEM_PERM_RW | K_MEM_CACHE_WB | K_MEM_MAP_UNINIT,
				false);

	__ASSERT_NO_MSG((uintptr_t)அடுக்கு_mapped != 0);

#ifdef CONFIG_USERSPACE
	எனில் (z_அடுக்கு_is_user_capable(அடுக்கு)) {
		geo->buf_start = K_THREAD_அடுக்கு_BUFFER(அடுக்கு_mapped);
	} இல்லையெனில்
#endif /* CONFIG_USERSPACE */
	{
		geo->buf_start = K_KERNEL_அடுக்கு_BUFFER(அடுக்கு_mapped);
	}

	அடுக்கு_ptr = (எழுத்து *)அடுக்கு_mapped + geo->obj_size;

	/* Need to store the info on mapped stack so we can remove the mappings
	 * when the thread ends.
	 */
	new_thread->அடுக்கு_info.mapped.addr = அடுக்கு_mapped;
	new_thread->அடுக்கு_info.mapped.sz = geo->obj_size;
#இல்லையெனில் /* CONFIG_THREAD_STACK_MEM_MAPPED */
	ARG_UNUSED(new_thread);

	/* Initial stack pointer at the high end of the stack object, may
	 * be reduced later by TLS or random offset
	 */
	அடுக்கு_ptr = (எழுத்து *)அடுக்கு + geo->obj_size;
#endif /* CONFIG_THREAD_STACK_MEM_MAPPED */

	திரும்பு அடுக்கு_ptr;
}

/* Pre-fill the stack buffer with a known pattern for usage measurement. */
நிலையான உள்வரி வெறுமை fill_init_அடுக்கு(எழுத்து *buf_start, size_t buf_size)
{
#ifdef CONFIG_INIT_STACKS
	memset(buf_start, 0xaa, buf_size);
#இல்லையெனில்
	ARG_UNUSED(buf_start);
	ARG_UNUSED(buf_size);
#endif /* CONFIG_INIT_STACKS */
}

/* Place the overflow-detection sentinel at the low end of the stack buffer. */
நிலையான உள்வரி வெறுமை set_அடுக்கு_sentinel(எழுத்து *buf_start)
{
#ifdef CONFIG_அடுக்கு_SENTINEL
	/* Put the stack sentinel at the lowest 4 bytes of the stack area.
	 * We periodically check that it's still present and kill the thread
	 * if it isn't.
	 */
	*((uint32_t *)buf_start) = அடுக்கு_SENTINEL;
#இல்லையெனில்
	ARG_UNUSED(buf_start);
#endif /* CONFIG_STACK_SENTINEL */
}

/*
 * Compute how much to reserve at the top of the stack buffer for TLS,
 * userspace-local data and stack-pointer randomization, and return the
 * aligned amount the initial stack pointer must be lowered by.
 */
நிலையான size_t reserve_அடுக்கு_headroom(கட்டமைப்பு k_thread *new_thread,
				     எழுத்து *அடுக்கு_ptr, size_t buf_size)
{
	size_t delta = 0;

	ARG_UNUSED(new_thread);
	ARG_UNUSED(அடுக்கு_ptr);
	ARG_UNUSED(buf_size);

#ifdef CONFIG_THREAD_LOCAL_STORAGE
	/* TLS is always last within the stack buffer */
	delta += arch_tls_அடுக்கு_setup(new_thread, அடுக்கு_ptr);
#endif /* CONFIG_THREAD_LOCAL_STORAGE */
#ifdef CONFIG_THREAD_USERSPACE_LOCAL_DATA
	size_t tls_size = அளவு(கட்டமைப்பு _thread_userspace_local_data);

	/* reserve space on highest memory of stack buffer for local data */
	delta += tls_size;
	new_thread->userspace_local_data =
		(கட்டமைப்பு _thread_userspace_local_data *)(அடுக்கு_ptr - delta);
#endif /* CONFIG_THREAD_USERSPACE_LOCAL_DATA */
#எனில் defined(CONFIG_அடுக்கு_POINTER_RANDOM) && (CONFIG_அடுக்கு_POINTER_RANDOM != 0)
	delta += random_offset(buf_size);
#endif /* CONFIG_STACK_POINTER_RANDOM */

	திரும்பு ROUND_UP(delta, ARCH_அடுக்கு_PTR_ALIGN);
}

/* Record the thread-accessible stack bounds (and runtime-safety threshold). */
நிலையான உள்வரி வெறுமை set_அடுக்கு_info(கட்டமைப்பு k_thread *new_thread, எழுத்து *buf_start,
				  size_t buf_size, size_t delta)
{
#ifdef CONFIG_THREAD_அடுக்கு_INFO
	/* Initial values. Arches which implement MPU guards that "borrow"
	 * memory from the stack buffer (not tracked in K_THREAD_STACK_RESERVED)
	 * will need to appropriately update this.
	 *
	 * The bounds tracked here correspond to the area of the stack object
	 * that the thread can access, which includes TLS.
	 */
	new_thread->அடுக்கு_info.start = (uintptr_t)buf_start;
	new_thread->அடுக்கு_info.size = buf_size;
	new_thread->அடுக்கு_info.delta = delta;

#ifdef CONFIG_THREAD_RUNTIME_அடுக்கு_SAFETY
	new_thread->அடுக்கு_info.usage.unused_threshold =
		(CONFIG_THREAD_RUNTIME_அடுக்கு_SAFETY_DEFAULT_UNUSED_THRESHOLD_PCT *
		 buf_size) / 100;
#endif
#இல்லையெனில்
	ARG_UNUSED(new_thread);
	ARG_UNUSED(buf_start);
	ARG_UNUSED(buf_size);
	ARG_UNUSED(delta);
#endif /* CONFIG_THREAD_STACK_INFO */
}

நிலையான எழுத்து *setup_thread_அடுக்கு(கட்டமைப்பு k_thread *new_thread,
				k_thread_அடுக்கு_t *அடுக்கு, size_t அடுக்கு_size)
{
	கட்டமைப்பு அடுக்கு_geometry geo = compute_அடுக்கு_geometry(அடுக்கு, அடுக்கு_size);
	எழுத்து *அடுக்கு_ptr = map_thread_அடுக்கு(new_thread, அடுக்கு, &geo);
	size_t delta;

	LOG_DBG("stack %p for thread %p: obj_size=%zu buf_start=%p "
		" buf_size %zu stack_ptr=%p",
		அடுக்கு, new_thread, geo.obj_size, (வெறுமை *)geo.buf_start,
		geo.buf_size, (வெறுமை *)அடுக்கு_ptr);

	fill_init_அடுக்கு(geo.buf_start, geo.buf_size);
	set_அடுக்கு_sentinel(geo.buf_start);
	delta = reserve_அடுக்கு_headroom(new_thread, அடுக்கு_ptr, geo.buf_size);
	set_அடுக்கு_info(new_thread, geo.buf_start, geo.buf_size, delta);

	திரும்பு அடுக்கு_ptr - delta;
}

#ifdef CONFIG_HW_SHADOW_அடுக்கு
நிலையான வெறுமை setup_shadow_அடுக்கு(கட்டமைப்பு k_thread *new_thread,
			 k_thread_அடுக்கு_t *அடுக்கு)
{
	முழு ret = -ENOENT;

	STRUCT_SECTION_FOREACH(அடுக்கு_to_hw_shadow_அடுக்கு, stk_to_hw_shstk) {
		எனில் (stk_to_hw_shstk->அடுக்கு == அடுக்கு) {
			ret = k_thread_hw_shadow_அடுக்கு_attach(new_thread,
							      stk_to_hw_shstk->shstk_addr,
							      stk_to_hw_shstk->size);
			எனில் (ret != 0) {
				LOG_ERR("Could not set thread %p shadow stack %p, got error %d",
					new_thread, stk_to_hw_shstk->shstk_addr, ret);
				k_panic();
			}
			நிறுத்து;
		}
	}

	/* Check if the stack isn't in a stack array, and use the corresponding
	 * shadow stack.
	 */
	எனில் (ret != -ENOENT) {
		திரும்பு;
	}

	STRUCT_SECTION_FOREACH(அடுக்கு_to_hw_shadow_அடுக்கு_arr, stk_to_hw_shstk) {
		எனில் ((uintptr_t)அடுக்கு >= stk_to_hw_shstk->அடுக்கு_addr &&
		    (uintptr_t)அடுக்கு < stk_to_hw_shstk->அடுக்கு_addr +
		    stk_to_hw_shstk->அடுக்கு_size * stk_to_hw_shstk->nmemb) {
			/* Now we have to guess which index of the stack array is being used */
			uintptr_t அடுக்கு_offset = (uintptr_t)அடுக்கு - stk_to_hw_shstk->அடுக்கு_addr;
			uintptr_t அடுக்கு_index = அடுக்கு_offset / stk_to_hw_shstk->அடுக்கு_size;
			uintptr_t addr;

			எனில் (அடுக்கு_index >= stk_to_hw_shstk->nmemb) {
				LOG_ERR("Could not find shadow stack for thread %p, stack %p",
					new_thread, அடுக்கு);
				k_panic();
			}

			addr = stk_to_hw_shstk->shstk_addr +
				stk_to_hw_shstk->shstk_size * அடுக்கு_index;
			ret = k_thread_hw_shadow_அடுக்கு_attach(new_thread,
							      (arch_thread_hw_shadow_அடுக்கு_t *)addr,
							      stk_to_hw_shstk->shstk_size);
			எனில் (ret != 0) {
				LOG_ERR("Could not set thread %p shadow stack 0x%lx, got error %d",
					new_thread, stk_to_hw_shstk->shstk_addr, ret);
				k_panic();
			}
			நிறுத்து;
		}
	}

	எனில் (ret == -ENOENT) {
		LOG_ERR("Could not find shadow stack for thread %p, stack %p",
			new_thread, அடுக்கு);
		k_panic();
	}
}
#இல்லையெனில்
/* No-op stub used when hardware shadow stacks are not configured. */
நிலையான உள்வரி வெறுமை setup_shadow_அடுக்கு(கட்டமைப்பு k_thread *new_thread,
				      k_thread_அடுக்கு_t *அடுக்கு)
{
	ARG_UNUSED(new_thread);
	ARG_UNUSED(அடுக்கு);
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
நிலையான உள்வரி வெறுமை thread_abort_cleanup_check_reuse(கட்டமைப்பு k_thread *thread)
{
#ifdef CONFIG_THREAD_ABORT_NEED_CLEANUP
	k_thread_abort_cleanup_check_reuse(thread);
#இல்லையெனில்
	ARG_UNUSED(thread);
#endif /* CONFIG_THREAD_ABORT_NEED_CLEANUP */
}

/* Register the thread with the object-core framework and its usage stats. */
நிலையான உள்வரி வெறுமை init_thread_obj_core(கட்டமைப்பு k_thread *thread)
{
#ifdef CONFIG_OBJ_CORE_THREAD
	k_obj_core_init_and_link(K_OBJ_CORE(thread), &obj_type_thread);
#ifdef CONFIG_OBJ_CORE_STATS_THREAD
	k_obj_core_stats_register(K_OBJ_CORE(thread), &thread->base.usage,
				  அளவு(thread->base.usage));
#endif /* CONFIG_OBJ_CORE_STATS_THREAD */
#இல்லையெனில்
	ARG_UNUSED(thread);
#endif /* CONFIG_OBJ_CORE_THREAD */
}

/*
 * Initialize userspace bookkeeping: register the thread and stack kernel
 * objects, record stack ownership, and grant the thread access to itself.
 */
நிலையான உள்வரி வெறுமை init_thread_userspace(கட்டமைப்பு k_thread *thread,
					 k_thread_அடுக்கு_t *அடுக்கு,
					 uint32_t options)
{
#ifdef CONFIG_USERSPACE
	__ASSERT((options & K_USER) == 0U || z_அடுக்கு_is_user_capable(அடுக்கு),
		 "user thread %p with kernel-only stack %p", thread, அடுக்கு);
	k_object_init(thread);
	k_object_init(அடுக்கு);
	thread->அடுக்கு_obj = அடுக்கு;
	thread->syscall_frame = வெற்று;

	/* Any given thread has access to itself */
	k_object_access_grant(thread, thread);
#இல்லையெனில்
	ARG_UNUSED(thread);
	ARG_UNUSED(அடுக்கு);
	ARG_UNUSED(options);
#endif /* CONFIG_USERSPACE */
}

/* Assert the thread object is cache-coherent while its stack stays cached. */
நிலையான உள்வரி வெறுமை assert_thread_coherence(கட்டமைப்பு k_thread *thread,
					   k_thread_அடுக்கு_t *அடுக்கு)
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
	__ASSERT_NO_MSG(!sys_cache_is_mem_coherent(அடுக்கு));
#endif /* CONFIG_DYNAMIC_THREAD */
#endif /* CONFIG_KERNEL_COHERENCE */
	ARG_UNUSED(thread);
	ARG_UNUSED(அடுக்கு);
}

/* Assert the arch layer populated switch_handle on USE_SWITCH builds. */
நிலையான உள்வரி வெறுமை assert_switch_handle(கட்டமைப்பு k_thread *thread)
{
#ifdef CONFIG_USE_SWITCH
	/* switch_handle must be non-null except when inside z_swap()
	 * for synchronization reasons.  Historically some notional
	 * USE_SWITCH architectures have actually ignored the field
	 */
	__ASSERT(thread->switch_handle != வெற்று,
		 "arch layer failed to initialize switch_handle");
#endif /* CONFIG_USE_SWITCH */
	ARG_UNUSED(thread);
}

/* Clear the opaque, kernel-managed per-thread custom data pointer. */
நிலையான உள்வரி வெறுமை init_thread_custom_data(கட்டமைப்பு k_thread *thread)
{
#ifdef CONFIG_THREAD_CUSTOM_DATA
	/* Initialize custom data field (value is opaque to kernel) */
	thread->custom_data = வெற்று;
#இல்லையெனில்
	ARG_UNUSED(thread);
#endif /* CONFIG_THREAD_CUSTOM_DATA */
}

/*
 * Record the entry point and parameters, then link the thread into the
 * global thread-monitor list under the monitor spinlock.
 */
நிலையான உள்வரி வெறுமை add_thread_to_monitor(கட்டமைப்பு k_thread *thread,
					 k_thread_entry_t entry,
					 வெறுமை *p1, வெறுமை *p2, வெறுமை *p3)
{
#ifdef CONFIG_THREAD_MONITOR
	thread->entry.pEntry = entry;
	thread->entry.parameter1 = p1;
	thread->entry.parameter2 = p2;
	thread->entry.parameter3 = p3;

	k_spinlock_சாவி_t சாவி = k_spin_lock(&z_thread_monitor_lock);

	thread->next_thread = _kernel.threads;
	_kernel.threads = thread;
	k_spin_unlock(&z_thread_monitor_lock, சாவி);
#இல்லையெனில்
	ARG_UNUSED(thread);
	ARG_UNUSED(entry);
	ARG_UNUSED(p1);
	ARG_UNUSED(p2);
	ARG_UNUSED(p3);
#endif /* CONFIG_THREAD_MONITOR */
}

/* Set the thread's name from the caller-supplied string (may be NULL). */
நிலையான உள்வரி வெறுமை init_thread_name(கட்டமைப்பு k_thread *thread, மாறிலி எழுத்து *name)
{
#ifdef CONFIG_THREAD_NAME
	set_thread_name(thread, name);
#இல்லையெனில்
	ARG_UNUSED(thread);
	ARG_UNUSED(name);
#endif /* CONFIG_THREAD_NAME */
}

/* Set the thread's initial SMP CPU affinity mask. */
நிலையான உள்வரி வெறுமை init_thread_cpu_mask(கட்டமைப்பு k_thread *thread)
{
#ifdef CONFIG_SCHED_CPU_MASK
	எனில் (IS_ENABLED(CONFIG_SCHED_CPU_MASK_PIN_ONLY)) {
		thread->base.cpu_mask = 1; /* must specify only one cpu */
	} இல்லையெனில் {
		thread->base.cpu_mask = -1; /* allow all cpus */
	}
#இல்லையெனில்
	ARG_UNUSED(thread);
#endif /* CONFIG_SCHED_CPU_MASK */
}

/* Attach the thread to its memory domain and inherit caller permissions
 * when K_INHERIT_PERMS is requested.
 */
நிலையான உள்வரி வெறுமை init_thread_userspace_perms(கட்டமைப்பு k_thread *thread,
					       uint32_t options)
{
#ifdef CONFIG_USERSPACE
	z_mem_domain_init_thread(thread);

	எனில் ((options & K_INHERIT_PERMS) != 0U) {
		k_thread_perms_inherit(_current, thread);
	}
#இல்லையெனில்
	ARG_UNUSED(thread);
	ARG_UNUSED(options);
#endif /* CONFIG_USERSPACE */
}

/* Clear the EDF scheduling deadline. */
நிலையான உள்வரி வெறுமை init_thread_deadline(கட்டமைப்பு k_thread *thread)
{
#ifdef CONFIG_SCHED_DEADLINE
	thread->base.prio_deadline = 0;
#இல்லையெனில்
	ARG_UNUSED(thread);
#endif /* CONFIG_SCHED_DEADLINE */
}

/* Initialize the wait queue used to synchronize SMP thread halt/abort. */
நிலையான உள்வரி வெறுமை init_thread_halt_queue(கட்டமைப்பு k_thread *thread)
{
#ifdef CONFIG_SMP
	z_waitq_init(&thread->halt_queue);
#இல்லையெனில்
	ARG_UNUSED(thread);
#endif /* CONFIG_SMP */
}

/* Initialize runtime CPU-usage tracking for the thread. */
நிலையான உள்வரி வெறுமை init_thread_usage(கட்டமைப்பு k_thread *thread)
{
#ifdef CONFIG_SCHED_THREAD_USAGE
	thread->base.usage = (கட்டமைப்பு k_cycle_stats) {};
	thread->base.usage.track_usage = CONFIG_SCHED_THREAD_USAGE_AUTO_ENABLE;
#இல்லையெனில்
	ARG_UNUSED(thread);
#endif /* CONFIG_SCHED_THREAD_USAGE */
}

/* Assert the thread wasn't already linked into the monitor list. */
நிலையான உள்வரி வெறுமை assert_thread_not_reused(கட்டமைப்பு k_thread *new_thread)
{
#ifdef CONFIG_THREAD_MONITOR
#ifdef CONFIG_ASSERT
	k_spinlock_சாவி_t சாவி = k_spin_lock(&z_thread_monitor_lock);
	bool reused = false;

	/* Check if the thread is already in the list */
	க்கு (கட்டமைப்பு k_thread *t = _kernel.threads; t; t = t->next_thread) {
		எனில் (t == new_thread) {
			reused = true;
			நிறுத்து;
		}
	}

	k_spin_unlock(&z_thread_monitor_lock, சாவி);

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
எழுத்து *z_setup_new_thread(கட்டமைப்பு k_thread *new_thread,
			 k_thread_அடுக்கு_t *அடுக்கு, size_t அடுக்கு_size,
			 k_thread_entry_t entry,
			 வெறுமை *p1, வெறுமை *p2, வெறுமை *p3,
			 முழு prio, uint32_t options, மாறிலி எழுத்து *name)
{
	எழுத்து *அடுக்கு_ptr;

	Z_ASSERT_VALID_PRIO(prio, entry);

	assert_thread_not_reused(new_thread);

	thread_abort_cleanup_check_reuse(new_thread);
	init_thread_obj_core(new_thread);
	init_thread_userspace(new_thread, அடுக்கு, options);

	z_waitq_init(&new_thread->join_queue);

	/* Initialize various struct k_thread members */
	z_init_thread_base(&new_thread->base, prio, _THREAD_SLEEPING, options);
	அடுக்கு_ptr = setup_thread_அடுக்கு(new_thread, அடுக்கு, அடுக்கு_size);

#எனில் Z_MUTEX_PI_ENABLED
	sys_slist_init(&new_thread->held_mutexes);
	new_thread->mutex_pended_on = வெற்று;
	new_thread->orig_prio = new_thread->base.prio;
#endif /* Z_MUTEX_PI_ENABLED */

	setup_shadow_அடுக்கு(new_thread, அடுக்கு);
	assert_thread_coherence(new_thread, அடுக்கு);

	arch_new_thread(new_thread, அடுக்கு, அடுக்கு_ptr, entry, p1, p2, p3);

	/* static threads overwrite it afterwards with real value */
	new_thread->init_data = வெற்று;

	assert_switch_handle(new_thread);
	init_thread_custom_data(new_thread);
	add_thread_to_monitor(new_thread, entry, p1, p2, p3);
	init_thread_name(new_thread, name);
	init_thread_cpu_mask(new_thread);

	/* _current may be NULL if the dummy thread is not used */
	எனில் (IS_ENABLED(CONFIG_ARCH_HAS_CUSTOM_SWAP_TO_MAIN) &&
	    (_current == வெற்று)) {
		new_thread->resource_pool = வெற்று;
		திரும்பு அடுக்கு_ptr;
	}

	init_thread_userspace_perms(new_thread, options);
	init_thread_deadline(new_thread);
	new_thread->resource_pool = _current->resource_pool;
	init_thread_halt_queue(new_thread);
	init_thread_usage(new_thread);

	SYS_PORT_TRACING_OBJ_FUNC(k_thread, create, new_thread);

	திரும்பு அடுக்கு_ptr;
}

#ifdef CONFIG_THREAD_RUNTIME_அடுக்கு_SAFETY
முழு z_impl_k_thread_runtime_அடுக்கு_unused_threshold_pct_set(கட்டமைப்பு k_thread *thread,
							   uint32_t pct)
{
	size_t unused_threshold;

	எனில் (pct > 99) {
		திரும்பு -EINVAL;   /* 100% unused stack and up is invalid */
	}

	unused_threshold = (thread->அடுக்கு_info.size * pct) / 100;

	thread->அடுக்கு_info.usage.unused_threshold = unused_threshold;

	திரும்பு 0;
}

முழு z_impl_k_thread_runtime_அடுக்கு_unused_threshold_set(கட்டமைப்பு k_thread *thread,
						       size_t threshold)
{
	எனில் (threshold > thread->அடுக்கு_info.size) {
		திரும்பு -EINVAL;
	}

	thread->அடுக்கு_info.usage.unused_threshold = threshold;

	திரும்பு 0;
}

size_t z_impl_k_thread_runtime_அடுக்கு_unused_threshold_get(கட்டமைப்பு k_thread *thread)
{
	திரும்பு thread->அடுக்கு_info.usage.unused_threshold;
}

#ifdef CONFIG_USERSPACE
முழு z_vrfy_k_thread_runtime_அடுக்கு_unused_threshold_pct_set(கட்டமைப்பு k_thread *thread,
							   uint32_t pct)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));

	திரும்பு z_impl_k_thread_runtime_அடுக்கு_unused_threshold_pct_set(thread, pct);
}
#அந்தர்பாவ <zephyr/syscalls/k_thread_runtime_stack_unused_threshold_pct_set_mrsh.c>

முழு z_vrfy_k_thread_runtime_அடுக்கு_unused_threshold_set(கட்டமைப்பு k_thread *thread,
						       size_t threshold)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));

	திரும்பு z_impl_k_thread_runtime_அடுக்கு_unused_threshold_set(thread, threshold);
}
#அந்தர்பாவ <zephyr/syscalls/k_thread_runtime_stack_unused_threshold_set_mrsh.c>

size_t z_vrfy_k_thread_runtime_அடுக்கு_unused_threshold_get(கட்டமைப்பு k_thread *thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));

	திரும்பு z_impl_k_thread_runtime_அடுக்கு_unused_threshold_get(thread);
}
#அந்தர்பாவ <zephyr/syscalls/k_thread_runtime_stack_unused_threshold_get_mrsh.c>
#endif /* CONFIG_USERSPACE */
#endif /* CONFIG_THREAD_RUNTIME_STACK_SAFETY */

k_tid_t z_impl_k_thread_create(கட்டமைப்பு k_thread *new_thread,
			      k_thread_அடுக்கு_t *அடுக்கு,
			      size_t அடுக்கு_size, k_thread_entry_t entry,
			      வெறுமை *p1, வெறுமை *p2, வெறுமை *p3,
			      முழு prio, uint32_t options, k_timeout_t delay)
{
	__ASSERT(!arch_is_in_isr(), "Threads may not be created in ISRs");

	z_setup_new_thread(new_thread, அடுக்கு, அடுக்கு_size, entry, p1, p2, p3,
			  prio, options, வெற்று);

	thread_schedule_new(new_thread, delay);

	திரும்பு new_thread;
}

#ifdef CONFIG_USERSPACE
bool z_அடுக்கு_is_user_capable(k_thread_அடுக்கு_t *அடுக்கு)
{
	திரும்பு k_object_find(அடுக்கு) != வெற்று;
}

k_tid_t z_vrfy_k_thread_create(கட்டமைப்பு k_thread *new_thread,
			       k_thread_அடுக்கு_t *அடுக்கு,
			       size_t அடுக்கு_size, k_thread_entry_t entry,
			       வெறுமை *p1, வெறுமை *p2, வெறுமை *p3,
			       முழு prio, uint32_t options, k_timeout_t delay)
{
	size_t total_size, அடுக்கு_obj_size;
	கட்டமைப்பு k_object *அடுக்கு_object;

	/* The thread and stack objects *must* be in an uninitialized state */
	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(new_thread, K_OBJ_THREAD));

	/* No need to check z_stack_is_user_capable(), it won't be in the
	 * object table if it isn't
	 */
	அடுக்கு_object = k_object_find(அடுக்கு);
	K_OOPS(K_SYSCALL_VERIFY_செய்தி(k_object_validation_check(அடுக்கு_object, அடுக்கு,
						K_OBJ_THREAD_அடுக்கு_ELEMENT,
						_OBJ_INIT_FALSE) == 0,
				    "bad stack object"));

	/* Verify that the stack size passed in is OK by computing the total
	 * size and comparing it with the size value in the object metadata
	 */
	K_OOPS(K_SYSCALL_VERIFY_செய்தி(!size_add_overflow(K_THREAD_அடுக்கு_RESERVED,
						       அடுக்கு_size, &total_size),
				    "stack size overflow (%zu+%zu)",
				    அடுக்கு_size,
				    K_THREAD_அடுக்கு_RESERVED));

	/* Testing less-than-or-equal since additional room may have been
	 * allocated for alignment constraints
	 */
#ifdef CONFIG_GEN_PRIV_STACKS
	அடுக்கு_obj_size = அடுக்கு_object->data.அடுக்கு_data->size;
#இல்லையெனில்
	அடுக்கு_obj_size = அடுக்கு_object->data.அடுக்கு_size;
#endif /* CONFIG_GEN_PRIV_STACKS */
	K_OOPS(K_SYSCALL_VERIFY_செய்தி(total_size <= அடுக்கு_obj_size,
				    "stack size %zu is too big, max is %zu",
				    total_size, அடுக்கு_obj_size));

	/* User threads may only create other user threads and they can't
	 * be marked as essential
	 */
	K_OOPS(K_SYSCALL_VERIFY(options & K_USER));
	K_OOPS(K_SYSCALL_VERIFY(!(options & K_ESSENTIAL)));

	/* Check validity of prio argument; must be the same or worse priority
	 * than the caller
	 */
	K_OOPS(K_SYSCALL_VERIFY(_is_valid_prio(prio, வெற்று)));
	K_OOPS(K_SYSCALL_VERIFY(z_is_prio_lower_or_equal(prio,
							_current->base.prio)));

	z_setup_new_thread(new_thread, அடுக்கு, அடுக்கு_size,
			   entry, p1, p2, p3, prio, options, வெற்று);

	thread_schedule_new(new_thread, delay);

	திரும்பு new_thread;
}
#அந்தர்பாவ <zephyr/syscalls/k_thread_create_mrsh.c>
#endif /* CONFIG_USERSPACE */

வெறுமை z_init_thread_base(கட்டமைப்பு _thread_base *thread_base, முழு priority,
		       uint32_t initial_state, unsigned முழு options)
{
	/* k_q_node is initialized upon first insertion in a list */
	thread_base->pended_on = வெற்று;
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
	thread_base->slice_expired = வெற்று;
#endif /* CONFIG_TIMESLICE_PER_THREAD */

#ifdef CONFIG_SPIN_VALIDATE
	thread_base->swap_data = வெற்று;
	/* otherwise swap_data does not need to be initialized */
#endif

	z_init_thread_timeout(thread_base);
}

FUNC_NORETURN வெறுமை k_thread_user_mode_enter(k_thread_entry_t entry,
					    வெறுமை *p1, வெறுமை *p2, வெறுமை *p3)
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
	__ASSERT(z_அடுக்கு_is_user_capable(_current->அடுக்கு_obj),
		 "dropping to user mode with kernel-only stack object");
#ifdef CONFIG_THREAD_USERSPACE_LOCAL_DATA
	memset(_current->userspace_local_data, 0,
	       அளவு(கட்டமைப்பு _thread_userspace_local_data));
#endif /* CONFIG_THREAD_USERSPACE_LOCAL_DATA */
#ifdef CONFIG_THREAD_LOCAL_STORAGE
	arch_tls_அடுக்கு_setup(_current,
			     (எழுத்து *)(_current->அடுக்கு_info.start +
				      _current->அடுக்கு_info.size));
#endif /* CONFIG_THREAD_LOCAL_STORAGE */
	arch_user_mode_enter(entry, p1, p2, p3);
#இல்லையெனில்
	/* XXX In this case we do not reset the stack */
	z_thread_entry(entry, p1, p2, p3);
#endif /* CONFIG_USERSPACE */
}

#எனில் defined(CONFIG_INIT_STACKS) && defined(CONFIG_THREAD_அடுக்கு_INFO)
#ifdef CONFIG_அடுக்கு_GROWS_UP
#error "Unsupported configuration for stack analysis"
#endif /* CONFIG_STACK_GROWS_UP */

முழு z_அடுக்கு_space_get(மாறிலி uint8_t *அடுக்கு_start, size_t size, size_t *unused_ptr)
{
	size_t unused = 0;
	மாறிலி uint8_t *checked_அடுக்கு = அடுக்கு_start;
	/* Take the address of any local variable as a shallow bound for the
	 * stack pointer.  Addresses above it are guaranteed to be
	 * accessible.
	 */
	மாறிலி uint8_t *அடுக்கு_pointer = (மாறிலி uint8_t *)&அடுக்கு_start;

	/* If we are currently running on the stack being analyzed, some
	 * memory management hardware will generate an exception if we
	 * read unused stack memory.
	 *
	 * This never happens when invoked from user mode, as user mode
	 * will always run this function on the privilege elevation stack.
	 */
	எனில் ((அடுக்கு_pointer > அடுக்கு_start) && (அடுக்கு_pointer <= (அடுக்கு_start + size)) &&
	    IS_ENABLED(CONFIG_NO_UNUSED_அடுக்கு_INSPECTION)) {
		/* TODO: We could add an arch_ API call to temporarily
		 * disable the stack checking in the CPU, but this would
		 * need to be properly managed wrt context switches/interrupts
		 */
		திரும்பு -ENOTSUP;
	}

	எனில் (IS_ENABLED(CONFIG_அடுக்கு_SENTINEL)) {
		/* First 4 bytes of the stack buffer reserved for the
		 * sentinel value, it won't be 0xAAAAAAAA for thread
		 * stacks.
		 *
		 * FIXME: thread->stack_info.start ought to reflect
		 * this!
		 */
		checked_அடுக்கு += 4;
		size -= 4;
	}

	க்கு (size_t i = 0; i < size; i++) {
		எனில் ((checked_அடுக்கு[i]) == 0xaaU) {
			unused++;
		} இல்லையெனில் {
			நிறுத்து;
		}
	}

	*unused_ptr = unused;

	திரும்பு 0;
}

#ifdef CONFIG_THREAD_RUNTIME_அடுக்கு_SAFETY
முழு k_thread_runtime_அடுக்கு_safety_full_check(மாறிலி கட்டமைப்பு k_thread *thread,
					     size_t *unused_ptr,
					     k_thread_அடுக்கு_safety_handler_t handler,
					     வெறுமை *arg)
{
	முழு    rv;
	size_t unused_space;

	__ASSERT_NO_MSG(thread != வெற்று);

	rv = z_அடுக்கு_space_get((மாறிலி uint8_t *)thread->அடுக்கு_info.start,
			       thread->அடுக்கு_info.size, &unused_space);

	எனில் (rv != 0) {
		திரும்பு rv;
	}

	எனில் (unused_ptr != வெற்று) {
		*unused_ptr = unused_space;
	}

	எனில் ((unused_space < thread->அடுக்கு_info.usage.unused_threshold) &&
	    (handler != வெற்று)) {
		handler(thread, unused_space, arg);
	}

	திரும்பு 0;
}

முழு k_thread_runtime_அடுக்கு_safety_threshold_check(மாறிலி கட்டமைப்பு k_thread *thread,
						  size_t *unused_ptr,
						  k_thread_அடுக்கு_safety_handler_t handler,
						  வெறுமை *arg)
{
	முழு    rv;
	size_t unused_space;
	size_t threshold;
	size_t scan_size;

	__ASSERT_NO_MSG(thread != வெற்று);

	threshold = thread->அடுக்கு_info.usage.unused_threshold;
	scan_size = threshold;

	/* z_stack_space_get() reserves the first bytes of the stack buffer for
	 * the stack sentinel and shrinks its scan window by that amount. Grow
	 * the requested window by the same reservation so that up to
	 * 'threshold' bytes of the usable stack are actually examined;
	 * otherwise a fully unused window would report fewer than 'threshold'
	 * unused bytes and spuriously trip the threshold.
	 */
	எனில் (IS_ENABLED(CONFIG_அடுக்கு_SENTINEL) && (threshold != 0U)) {
		scan_size += அளவு(uint32_t);
	}

	rv = z_அடுக்கு_space_get((மாறிலி uint8_t *)thread->அடுக்கு_info.start,
			       scan_size, &unused_space);

	எனில் (rv != 0) {
		திரும்பு rv;
	}

	எனில் (unused_ptr != வெற்று) {
		*unused_ptr = unused_space;
	}

	எனில் ((unused_space < threshold) && (handler != வெற்று)) {
		handler(thread, unused_space, arg);
	}

	திரும்பு 0;
}
#endif /* CONFIG_THREAD_RUNTIME_STACK_SAFETY */

முழு z_impl_k_thread_அடுக்கு_space_get(மாறிலி கட்டமைப்பு k_thread *thread,
				    size_t *unused_ptr)
{
#ifdef CONFIG_THREAD_அடுக்கு_MEM_MAPPED
	எனில் (thread->அடுக்கு_info.mapped.addr == வெற்று) {
		திரும்பு -EINVAL;
	}
#endif /* CONFIG_THREAD_STACK_MEM_MAPPED */

	திரும்பு z_அடுக்கு_space_get((மாறிலி uint8_t *)thread->அடுக்கு_info.start,
				 thread->அடுக்கு_info.size, unused_ptr);
}

#ifdef CONFIG_USERSPACE
முழு z_vrfy_k_thread_அடுக்கு_space_get(மாறிலி கட்டமைப்பு k_thread *thread,
				    size_t *unused_ptr)
{
	size_t unused;
	முழு ret;

	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));

	ret = z_impl_k_thread_அடுக்கு_space_get(thread, &unused);
	CHECKIF(ret != 0) {
		திரும்பு ret;
	}

	ret = k_usermode_to_copy(unused_ptr, &unused, அளவு(size_t));
	CHECKIF(ret != 0) {
		திரும்பு ret;
	}

	திரும்பு 0;
}
#அந்தர்பாவ <zephyr/syscalls/k_thread_stack_space_get_mrsh.c>
#endif /* CONFIG_USERSPACE */
#endif /* CONFIG_INIT_STACKS && CONFIG_THREAD_STACK_INFO */

#ifdef CONFIG_USERSPACE
நிலையான உள்வரி k_ticks_t z_vrfy_k_thread_timeout_remaining_ticks(
						    மாறிலி கட்டமைப்பு k_thread *thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	திரும்பு z_impl_k_thread_timeout_remaining_ticks(thread);
}
#அந்தர்பாவ <zephyr/syscalls/k_thread_timeout_remaining_ticks_mrsh.c>

நிலையான உள்வரி k_ticks_t z_vrfy_k_thread_timeout_expires_ticks(
						  மாறிலி கட்டமைப்பு k_thread *thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	திரும்பு z_impl_k_thread_timeout_expires_ticks(thread);
}
#அந்தர்பாவ <zephyr/syscalls/k_thread_timeout_expires_ticks_mrsh.c>
#endif /* CONFIG_USERSPACE */

#ifdef CONFIG_INSTRUMENT_THREAD_SWITCHING
வெறுமை z_thread_mark_switched_in(வெறுமை)
{
#எனில் defined(CONFIG_SCHED_THREAD_USAGE) && !defined(CONFIG_USE_SWITCH)
	z_sched_usage_start(_current);
#endif /* CONFIG_SCHED_THREAD_USAGE && !CONFIG_USE_SWITCH */

#ifdef CONFIG_TRACING
	SYS_PORT_TRACING_FUNC(k_thread, switched_in);
#endif /* CONFIG_TRACING */
}

வெறுமை z_thread_mark_switched_out(வெறுமை)
{
#எனில் defined(CONFIG_SCHED_THREAD_USAGE) && !defined(CONFIG_USE_SWITCH)
	z_sched_usage_stop();
#endif /*CONFIG_SCHED_THREAD_USAGE && !CONFIG_USE_SWITCH */

#ifdef CONFIG_TRACING
#ifdef CONFIG_THREAD_LOCAL_STORAGE
	/* Dummy thread won't have TLS set up to run arbitrary code */
	எனில் (!_current ||
	    (_current->base.thread_state & _THREAD_DUMMY) != 0) {
		திரும்பு;
	}
#endif /* CONFIG_THREAD_LOCAL_STORAGE */
	SYS_PORT_TRACING_FUNC(k_thread, switched_out);
#endif /* CONFIG_TRACING */
}
#endif /* CONFIG_INSTRUMENT_THREAD_SWITCHING */

முழு k_thread_runtime_stats_get(k_tid_t thread,
			       k_thread_runtime_stats_t *stats)
{
	எனில் ((thread == வெற்று) || (stats == வெற்று)) {
		திரும்பு -EINVAL;
	}

#ifdef CONFIG_SCHED_THREAD_USAGE
	z_sched_thread_usage(thread, stats);
#இல்லையெனில்
	*stats = (k_thread_runtime_stats_t) {};
#endif /* CONFIG_SCHED_THREAD_USAGE */

	திரும்பு 0;
}

முழு k_thread_runtime_stats_all_get(k_thread_runtime_stats_t *stats)
{
#ifdef CONFIG_SCHED_THREAD_USAGE_ALL
	k_thread_runtime_stats_t  tmp_stats;
#endif /* CONFIG_SCHED_THREAD_USAGE_ALL */

	எனில் (stats == வெற்று) {
		திரும்பு -EINVAL;
	}

	*stats = (k_thread_runtime_stats_t) {};

#ifdef CONFIG_SCHED_THREAD_USAGE_ALL
	/* Retrieve the usage stats for each core and amalgamate them. */

	unsigned முழு இலக்கம்_cpus = arch_இலக்கம்_cpus();

	க்கு (unsigned முழு i = 0; i < இலக்கம்_cpus; i++) {
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

	திரும்பு 0;
}

முழு k_thread_runtime_stats_cpu_get(முழு cpu, k_thread_runtime_stats_t *stats)
{
	எனில் (stats == வெற்று) {
		திரும்பு -EINVAL;
	}

	*stats = (k_thread_runtime_stats_t) {};

#ifdef CONFIG_SCHED_THREAD_USAGE_ALL
	CHECKIF((cpu < 0) || ((unsigned முழு)cpu >= arch_இலக்கம்_cpus())) {
		திரும்பு -EINVAL;
	}

	z_sched_cpu_usage(cpu, stats);
#இல்லையெனில்
	ARG_UNUSED(cpu);
#endif

	திரும்பு 0;
}

#ifdef CONFIG_THREAD_ABORT_NEED_CLEANUP
/** Pointer to thread which needs to be cleaned up. */
நிலையான கட்டமைப்பு k_thread *thread_to_cleanup;

/** Spinlock for thread abort cleanup. */
நிலையான கட்டமைப்பு k_spinlock thread_cleanup_lock;

#ifdef CONFIG_THREAD_அடுக்கு_MEM_MAPPED
நிலையான வெறுமை *thread_cleanup_அடுக்கு_addr;
நிலையான size_t thread_cleanup_அடுக்கு_sz;

நிலையான வெறுமை unmap_thread_அடுக்கு(வெறுமை *addr, size_t sz)
{
	எனில் (addr != வெற்று) {
		k_mem_unmap_phys_guard(addr, sz, false);
	}
}
#endif /* CONFIG_THREAD_STACK_MEM_MAPPED */

வெறுமை defer_thread_cleanup(கட்டமைப்பு k_thread *thread)
{
	/* Note when adding new deferred cleanup steps:
	 * - The thread object may have been overwritten by the time
	 *   the actual cleanup is being done (e.g. thread object
	 *   allocated on a stack). So stash any necessary data here
	 *   that will be used in the actual cleanup steps.
	 */
	thread_to_cleanup = thread;

#ifdef CONFIG_THREAD_அடுக்கு_MEM_MAPPED
	/* Note that the permission of the stack should have been
	 * stripped of user thread access due to the thread having
	 * already exited from a memory domain. That is done via
	 * k_thread_abort().
	 */

	/* Stash the address and size so the region can be unmapped
	 * later.
	 */
	thread_cleanup_அடுக்கு_addr = thread->அடுக்கு_info.mapped.addr;
	thread_cleanup_அடுக்கு_sz = thread->அடுக்கு_info.mapped.sz;

	/* The stack is now considered un-usable. This should prevent any functions
	 * from looking directly into the mapped stack if they are made to be aware
	 * of memory mapped stacks, e.g., z_stack_space_get().
	 */
	thread->அடுக்கு_info.mapped.addr = வெற்று;
	thread->அடுக்கு_info.mapped.sz = 0;
#endif /* CONFIG_THREAD_STACK_MEM_MAPPED */
}

வெறுமை do_deferred_thread_cleanup(வெறுமை)
{
	/* Note when adding new actual cleanup steps:
	 * - The thread object may have been overwritten when this is
	 *   called. So only data stashed by defer_thread_cleanup() can
	 *   be used here, never anything from the thread object.
	 */

#ifdef CONFIG_THREAD_அடுக்கு_MEM_MAPPED
	unmap_thread_அடுக்கு(thread_cleanup_அடுக்கு_addr, thread_cleanup_அடுக்கு_sz);

	thread_cleanup_அடுக்கு_addr = வெற்று;
	thread_cleanup_அடுக்கு_sz = 0;
#endif /* CONFIG_THREAD_STACK_MEM_MAPPED */
}

வெறுமை do_thread_cleanup(கட்டமைப்பு k_thread *thread)
{
	/* This is only for threads which are no longer running and are not
	 * the current thread, so the thread object is still valid here and
	 * the cleanup can be driven from it directly.
	 */

#ifdef CONFIG_THREAD_அடுக்கு_MEM_MAPPED
	unmap_thread_அடுக்கு(thread->அடுக்கு_info.mapped.addr,
			   thread->அடுக்கு_info.mapped.sz);

	/* The stack is now unmapped and thus un-usable. Clear the mapping
	 * info so nothing looks into the stack afterwards, and so that
	 * a repeated cleanup of the same thread object is a no-op,
	 * e.g., z_stack_space_get().
	 */
	thread->அடுக்கு_info.mapped.addr = வெற்று;
	thread->அடுக்கு_info.mapped.sz = 0;
#இல்லையெனில் /* CONFIG_THREAD_STACK_MEM_MAPPED */
	ARG_UNUSED(thread);
#endif /* CONFIG_THREAD_STACK_MEM_MAPPED */
}

வெறுமை k_thread_abort_cleanup(கட்டமைப்பு k_thread *thread)
{
	K_SPINLOCK(&thread_cleanup_lock) {
		எனில் (thread_to_cleanup != வெற்று) {
			/* Finish the pending one first. */
			do_deferred_thread_cleanup();
			thread_to_cleanup = வெற்று;
		}

		எனில் (thread == _current) {
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
		} இல்லையெனில் {
			/* Not the current running thread, so we are safe to do
			 * cleanups.
			 */
			do_thread_cleanup(thread);
		}
	}
}

வெறுமை k_thread_abort_cleanup_check_reuse(கட்டமைப்பு k_thread *thread)
{
	K_SPINLOCK(&thread_cleanup_lock) {
		/* This is to guard reuse of the same thread object and make sure
		 * any pending cleanups of it needs to be finished before the thread
		 * object can be reused.
		 */
		எனில் (thread_to_cleanup == thread) {
			do_deferred_thread_cleanup();
			thread_to_cleanup = வெற்று;
		}
	}
}

#endif /* CONFIG_THREAD_ABORT_NEED_CLEANUP */

வெறுமை z_dummy_thread_init(கட்டமைப்பு k_thread *dummy_thread)
{
	dummy_thread->base.thread_state = _THREAD_DUMMY;
#ifdef CONFIG_SCHED_CPU_MASK
	dummy_thread->base.cpu_mask = -1;
#endif /* CONFIG_SCHED_CPU_MASK */
	dummy_thread->base.user_options = K_ESSENTIAL;
#எனில் Z_MUTEX_PI_ENABLED
	sys_slist_init(&dummy_thread->held_mutexes);
	dummy_thread->mutex_pended_on = வெற்று;
	dummy_thread->orig_prio = dummy_thread->base.prio;
#endif /* Z_MUTEX_PI_ENABLED */
#ifdef CONFIG_THREAD_அடுக்கு_INFO
	dummy_thread->அடுக்கு_info.start = 0U;
	dummy_thread->அடுக்கு_info.size = 0U;
#endif /* CONFIG_THREAD_STACK_INFO */
#ifdef CONFIG_USERSPACE
	dummy_thread->mem_domain_info.mem_domain = &k_mem_domain_default;
#endif /* CONFIG_USERSPACE */
#எனில் (K_HEAP_MEM_POOL_SIZE > 0)
	k_thread_system_pool_assign(dummy_thread);
#இல்லையெனில்
	dummy_thread->resource_pool = வெற்று;
#endif /* K_HEAP_MEM_POOL_SIZE */

#ifdef CONFIG_USE_SWITCH
	dummy_thread->switch_handle = வெற்று;
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

வெறுமை z_impl_k_thread_priority_set(k_tid_t thread, முழு prio)
{
	/*
	 * Use NULL, since we cannot know what the entry point is (we do not
	 * keep track of it) and idle cannot change its priority.
	 */
	Z_ASSERT_VALID_PRIO(prio, வெற்று);

	bool need_sched = z_thread_prio_set((கட்டமைப்பு k_thread *)thread, prio);

	எனில் ((need_sched) && (IS_ENABLED(CONFIG_SMP) ||
			     (_current->base.sched_locked == 0U))) {
		z_reschedule_unlocked();
	}
}

#ifdef CONFIG_USERSPACE
நிலையான உள்வரி வெறுமை z_vrfy_k_thread_priority_set(k_tid_t thread, முழு prio)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	K_OOPS(K_SYSCALL_VERIFY_செய்தி(_is_valid_prio(prio, வெற்று),
				    "invalid thread priority %d", prio));
#ifndef CONFIG_USERSPACE_THREAD_MAY_RAISE_PRIORITY
	K_OOPS(K_SYSCALL_VERIFY_செய்தி((int8_t)prio >= thread->base.prio,
				    "thread priority may only be downgraded (%d < %d)",
				    prio, thread->base.prio));
#endif /* CONFIG_USERSPACE_THREAD_MAY_RAISE_PRIORITY */
	z_impl_k_thread_priority_set(thread, prio);
}
#அந்தர்பாவ <zephyr/syscalls/k_thread_priority_set_mrsh.c>
#endif /* CONFIG_USERSPACE */

வெறுமை z_impl_k_thread_suspend(k_tid_t thread)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_thread, suspend, thread);

	/* Special case "suspend the current thread" as it doesn't
	 * need the async complexity below.
	 */
	எனில் (!IS_ENABLED(CONFIG_SMP) &&
	    likely((thread == _current) && !arch_is_in_isr())) {
		z_thread_suspend_current(thread);
		திரும்பு;
	}

	k_spinlock_சாவி_t சாவி = z_sched_spinlock_lock();

	எனில் (unlikely(z_is_thread_suspended(thread))) {

		/* The target thread is already suspended. Nothing to do. */

		z_sched_spinlock_unlock(சாவி);
		திரும்பு;
	}

	z_thread_halt(thread, சாவி, false);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_thread, suspend, thread);
}

#ifdef CONFIG_USERSPACE
நிலையான உள்வரி வெறுமை z_vrfy_k_thread_suspend(k_tid_t thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	z_impl_k_thread_suspend(thread);
}
#அந்தர்பாவ <zephyr/syscalls/k_thread_suspend_mrsh.c>
#endif /* CONFIG_USERSPACE */

வெறுமை z_impl_k_thread_resume(k_tid_t thread)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_thread, resume, thread);

	k_spinlock_சாவி_t சாவி = z_sched_spinlock_lock();

	/* Do not try to resume a thread that was not suspended */
	எனில் (unlikely(!z_is_thread_suspended(thread))) {
		z_sched_spinlock_unlock(சாவி);
		திரும்பு;
	}

	z_mark_thread_as_not_suspended(thread);
	z_sched_ready_locked(thread);

	z_reschedule_locked(சாவி);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_thread, resume, thread);
}

#ifdef CONFIG_USERSPACE
நிலையான உள்வரி வெறுமை z_vrfy_k_thread_resume(k_tid_t thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	z_impl_k_thread_resume(thread);
}
#அந்தர்பாவ <zephyr/syscalls/k_thread_resume_mrsh.c>
#endif /* CONFIG_USERSPACE */

வெறுமை z_impl_k_wakeup(k_tid_t thread)
{
	SYS_PORT_TRACING_OBJ_FUNC(k_thread, wakeup, thread);

	k_spinlock_சாவி_t சாவி = z_sched_spinlock_lock();

	எனில் (z_is_thread_sleeping(thread)) {
		(வெறுமை)z_try_abort_thread_timeout(thread);
		z_mark_thread_as_not_sleeping(thread);
		z_sched_ready_locked(thread);
		z_reschedule_locked(சாவி);
	} இல்லையெனில் {
		z_sched_spinlock_unlock(சாவி);
	}
}

#ifdef CONFIG_USERSPACE
நிலையான உள்வரி வெறுமை z_vrfy_k_wakeup(k_tid_t thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	z_impl_k_wakeup(thread);
}
#அந்தர்பாவ <zephyr/syscalls/k_wakeup_mrsh.c>
#endif /* CONFIG_USERSPACE */

வெறுமை z_thread_abort(கட்டமைப்பு k_thread *thread)
{
	bool essential = z_is_thread_essential(thread);
	k_spinlock_சாவி_t சாவி = z_sched_spinlock_lock();

	எனில் (z_is_thread_dead(thread)) {
		z_sched_spinlock_unlock(சாவி);
		திரும்பு;
	}

	z_thread_halt(thread, சாவி, true);

	எனில் (essential) {
		__ASSERT(!essential, "aborted essential thread %p", thread);
		k_panic();
	}
}

#எனில் !defined(CONFIG_ARCH_HAS_THREAD_ABORT)
வெறுமை z_impl_k_thread_abort(k_tid_t thread)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_thread, abort, thread);

	z_thread_abort(thread);

	__ASSERT_NO_MSG(z_is_thread_dead(thread));

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_thread, abort, thread);
}
#endif /* !CONFIG_ARCH_HAS_THREAD_ABORT */

முழு z_impl_k_thread_join(கட்டமைப்பு k_thread *thread, k_timeout_t timeout)
{
	k_spinlock_சாவி_t சாவி = z_sched_spinlock_lock();
	முழு ret;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_thread, join, thread, timeout);

	எனில் (z_is_thread_dead(thread)) {
		z_sched_switch_spin(thread);
		ret = 0;
	} இல்லையெனில் எனில் (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		ret = -EBUSY;
	} இல்லையெனில் எனில் ((thread == _current) ||
		   (thread->base.pended_on == &_current->join_queue)) {
		ret = -EDEADLK;
	} இல்லையெனில் {
		__ASSERT(!arch_is_in_isr(), "cannot join in ISR");
		z_sched_add_to_waitq_locked(_current, &thread->join_queue);
		z_add_thread_timeout(_current, timeout);

		SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_thread, join, thread, timeout);
		ret = z_swap_locked(சாவி);
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_thread, join, thread, timeout, ret);

		திரும்பு ret;
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_thread, join, thread, timeout, ret);

	z_sched_spinlock_unlock(சாவி);
	திரும்பு ret;
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
நிலையான bool thread_obj_validate(கட்டமைப்பு k_thread *thread)
{
	கட்டமைப்பு k_object *ko = k_object_find(thread);
	முழு ret = k_object_validate(ko, K_OBJ_THREAD, _OBJ_INIT_TRUE);

	தேர்வு (ret) {
	நிலை 0:
		திரும்பு false;
	நிலை -EINVAL:
		திரும்பு true;
	இயல்பு:
#ifdef CONFIG_LOG
		k_object_dump_error(ret, thread, ko, K_OBJ_THREAD);
#endif /* CONFIG_LOG */
		/* ret is a non-zero error code here (the 0 and -EINVAL cases
		 * are handled above), so this branch must always oops. Passing
		 * ret as the "verify" expression would treat the failure code as
		 * success and fall through to CODE_UNREACHABLE; verify ret == 0
		 * so the oops is actually raised.
		 */
		K_OOPS(K_SYSCALL_VERIFY_செய்தி(ret == 0, "access denied"));
	}
	CODE_UNREACHABLE; /* LCOV_EXCL_LINE */
}

நிலையான உள்வரி முழு z_vrfy_k_thread_join(கட்டமைப்பு k_thread *thread,
				       k_timeout_t timeout)
{
	எனில் (thread_obj_validate(thread)) {
		திரும்பு 0;
	}

	திரும்பு z_impl_k_thread_join(thread, timeout);
}
#அந்தர்பாவ <zephyr/syscalls/k_thread_join_mrsh.c>

நிலையான உள்வரி வெறுமை z_vrfy_k_thread_abort(k_tid_t thread)
{
	எனில் (thread_obj_validate(thread)) {
		திரும்பு;
	}

	K_OOPS(K_SYSCALL_VERIFY_செய்தி(!z_is_thread_essential(thread),
				    "aborting essential thread %p", thread));

	z_impl_k_thread_abort((கட்டமைப்பு k_thread *)thread);
}
#அந்தர்பாவ <zephyr/syscalls/k_thread_abort_mrsh.c>
#endif /* CONFIG_USERSPACE */

bool k_can_yield(வெறுமை)
{
	திரும்பு !(k_is_pre_kernel() || k_is_in_isr() || !arch_cpu_irqs_are_enabled() ||
		 z_is_idle_thread_object(_current));
}

வெறுமை z_impl_k_yield(வெறுமை)
{
	__ASSERT(!arch_is_in_isr(), "");

	SYS_PORT_TRACING_FUNC(k_thread, yield);

	z_sched_yield();
}

#ifdef CONFIG_USERSPACE
நிலையான உள்வரி வெறுமை z_vrfy_k_yield(வெறுமை)
{
	z_impl_k_yield();
}
#அந்தர்பாவ <zephyr/syscalls/k_yield_mrsh.c>
#endif /* CONFIG_USERSPACE */
