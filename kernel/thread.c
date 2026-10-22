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

#অন্তর্ভাৱ <zephyr/kernel.h>
#অন্তর্ভাৱ <zephyr/spinlock.h>
#অন্তর্ভাৱ <zephyr/sys/math_extras.h>
#অন্তর্ভাৱ <zephyr/sys/clock.h>
#অন্তর্ভাৱ <ksched.h>
#অন্তর্ভাৱ <kthread.h>
#অন্তর্ভাৱ <wait_q.h>
#অন্তর্ভাৱ <timeout_q.h>
#অন্তর্ভাৱ <zephyr/internal/syscall_handler.h>
#অন্তর্ভাৱ <kernel_internal.h>
#অন্তর্ভাৱ <kswap.h>
#অন্তর্ভাৱ <zephyr/init.h>
#অন্তর্ভাৱ <zephyr/tracing/tracing.h>
#অন্তর্ভাৱ <string.h>
#অন্তর্ভাৱ <stdbool.h>
#অন্তর্ভাৱ <zephyr/sys/check.h>
#অন্তর্ভাৱ <zephyr/random/random.h>
#অন্তর্ভাৱ <zephyr/sys/atomic.h>
#অন্তর্ভাৱ <zephyr/logging/log.h>
#অন্তর্ভাৱ <zephyr/llext/symbol.h>
#অন্তর্ভাৱ <zephyr/sys/iterable_sections.h>
#অন্তর্ভাৱ <zephyr/sys/minmax.h>

#অন্তর্ভাৱ <usage.h>

LOG_MODULE_DECLARE(os, CONFIG_KERNEL_LOG_LEVEL);

#ifdef CONFIG_OBJ_CORE_THREAD
#ifdef CONFIG_OBJ_CORE_STATS_THREAD
স্থির সংরচনা k_obj_core_stats_desc  thread_stats_desc = {
	.raw_size = পরিমাণ(সংরচনা k_cycle_stats),
	.query_size = পরিমাণ(সংরচনা k_thread_runtime_stats),
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
#অন্যথা
K_OBJ_TYPE_DEFINE_TYPE_ONLY(obj_type_thread, k_thread, K_OBJ_TYPE_THREAD_ID,
			    রিক্ত);
#endif /* CONFIG_OBJ_CORE_STATS_THREAD */
#endif /* CONFIG_OBJ_CORE_THREAD */


#define _FOREACH_STATIC_THREAD(thread_data)              \
	STRUCT_SECTION_FOREACH(_static_thread_data, thread_data)

bool k_is_in_isr(শূন্য)
{
	ঘূৰাই arch_is_in_isr();
}
EXPORT_প্ৰতীক(k_is_in_isr);

#ifdef CONFIG_THREAD_CUSTOM_DATA
শূন্য z_impl_k_thread_custom_data_set(শূন্য *মান)
{
	_current->custom_data = মান;
}

#ifdef CONFIG_USERSPACE
স্থির অন্তঃস্থ শূন্য z_vrfy_k_thread_custom_data_set(শূন্য *data)
{
	z_impl_k_thread_custom_data_set(data);
}
#অন্তর্ভাৱ <zephyr/syscalls/k_thread_custom_data_set_mrsh.c>
#endif /* CONFIG_USERSPACE */

শূন্য *z_impl_k_thread_custom_data_get(শূন্য)
{
	ঘূৰাই _current->custom_data;
}

#ifdef CONFIG_USERSPACE
স্থির অন্তঃস্থ শূন্য *z_vrfy_k_thread_custom_data_get(শূন্য)
{
	ঘূৰাই z_impl_k_thread_custom_data_get();
}
#অন্তর্ভাৱ <zephyr/syscalls/k_thread_custom_data_get_mrsh.c>

#endif /* CONFIG_USERSPACE */
#endif /* CONFIG_THREAD_CUSTOM_DATA */

পূর্ণাঙ্ক z_impl_k_is_preempt_thread(শূন্য)
{
	ঘূৰাই !arch_is_in_isr() && thread_is_preemptible(_current);
}

#ifdef CONFIG_USERSPACE
স্থির অন্তঃস্থ পূর্ণাঙ্ক z_vrfy_k_is_preempt_thread(শূন্য)
{
	ঘূৰাই z_impl_k_is_preempt_thread();
}
#অন্তর্ভাৱ <zephyr/syscalls/k_is_preempt_thread_mrsh.c>
#endif /* CONFIG_USERSPACE */

পূর্ণাঙ্ক z_impl_k_thread_priority_get(k_tid_t thread)
{
	ঘূৰাই thread->base.prio;
}

#ifdef CONFIG_USERSPACE
স্থির অন্তঃস্থ পূর্ণাঙ্ক z_vrfy_k_thread_priority_get(k_tid_t thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	ঘূৰাই z_impl_k_thread_priority_get(thread);
}
#অন্তর্ভাৱ <zephyr/syscalls/k_thread_priority_get_mrsh.c>
#endif /* CONFIG_USERSPACE */

#ifdef CONFIG_THREAD_NAME
স্থির শূন্য set_thread_name(সংরচনা k_thread *thread, ধ্রুৱ ৱর্ণ *str)
{
	যদি (str != রিক্ত) {
		strncpy(thread->name, str, CONFIG_THREAD_MAX_NAME_LEN - 1);
		/* Ensure NULL termination, truncate if longer */
		thread->name[CONFIG_THREAD_MAX_NAME_LEN - 1] = '\0';
#ifdef CONFIG_ARCH_HAS_THREAD_NAME_HOOK
		arch_thread_name_set(thread, str);
#endif /* CONFIG_ARCH_HAS_THREAD_NAME_HOOK */
	} অন্যথা {
		thread->name[0] = '\0';
	}
}
#endif /* CONFIG_THREAD_NAME */

পূর্ণাঙ্ক z_impl_k_thread_name_set(k_tid_t thread, ধ্রুৱ ৱর্ণ *str)
{
#ifdef CONFIG_THREAD_NAME
	যদি (thread == রিক্ত) {
		thread = _current;
	}

	set_thread_name(thread, str);

	SYS_PORT_TRACING_OBJ_FUNC(k_thread, name_set, thread, 0);

	ঘূৰাই 0;
#অন্যথা
	ARG_UNUSED(thread);
	ARG_UNUSED(str);

	SYS_PORT_TRACING_OBJ_FUNC(k_thread, name_set, thread, -ENOSYS);

	ঘূৰাই -ENOSYS;
#endif /* CONFIG_THREAD_NAME */
}

#ifdef CONFIG_USERSPACE
স্থির অন্তঃস্থ পূর্ণাঙ্ক z_vrfy_k_thread_name_set(k_tid_t thread, ধ্রুৱ ৱর্ণ *str)
{
#ifdef CONFIG_THREAD_NAME
	ৱর্ণ name[CONFIG_THREAD_MAX_NAME_LEN];

	যদি (thread != রিক্ত) {
		যদি (K_SYSCALL_OBJ(thread, K_OBJ_THREAD) != 0) {
			ঘূৰাই -EINVAL;
		}
	}

	/* In theory we could copy directly into thread->name, but
	 * the current z_vrfy / z_impl split does not provide a
	 * means of doing so.
	 */
	যদি (k_usermode_string_copy(name, str, পরিমাণ(name)) != 0) {
		ঘূৰাই -EFAULT;
	}

	ঘূৰাই z_impl_k_thread_name_set(thread, name);
#অন্যথা
	ঘূৰাই -ENOSYS;
#endif /* CONFIG_THREAD_NAME */
}
#অন্তর্ভাৱ <zephyr/syscalls/k_thread_name_set_mrsh.c>
#endif /* CONFIG_USERSPACE */

ধ্রুৱ ৱর্ণ *k_thread_name_get(k_tid_t thread)
{
#ifdef CONFIG_THREAD_NAME
	ঘূৰাই (ধ্রুৱ ৱর্ণ *)thread->name;
#অন্যথা
	ARG_UNUSED(thread);
	ঘূৰাই রিক্ত;
#endif /* CONFIG_THREAD_NAME */
}

পূর্ণাঙ্ক z_impl_k_thread_name_copy(k_tid_t thread, ৱর্ণ *buf, size_t size)
{
#ifdef CONFIG_THREAD_NAME
	strncpy(buf, thread->name, size);
	ঘূৰাই 0;
#অন্যথা
	ARG_UNUSED(thread);
	ARG_UNUSED(buf);
	ARG_UNUSED(size);
	ঘূৰাই -ENOSYS;
#endif /* CONFIG_THREAD_NAME */
}

স্থির size_t copy_bytes(ৱর্ণ *dest, size_t dest_size, ধ্রুৱ ৱর্ণ *src, size_t src_size)
{
	size_t  bytes_to_copy;

	bytes_to_copy = min(dest_size, src_size);
	memcpy(dest, src, bytes_to_copy);

	ঘূৰাই bytes_to_copy;
}

ধ্রুৱ ৱর্ণ *k_thread_state_str(k_tid_t thread_id, ৱর্ণ *buf, size_t buf_size)
{
	size_t      off = 0;
	uint8_t     bit;
	uint8_t     thread_state = thread_id->base.thread_state;
#define SS_ENT(s) { Z_STATE_STR_##s, _THREAD_##s, পরিমাণ(Z_STATE_STR_##s) - 1 }
	স্থির ধ্রুৱ সংরচনা {
		ধ্রুৱ ৱর্ণ *str;
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

	যদি ((buf == রিক্ত) || (buf_size == 0)) {
		ঘূৰাই "";
	}

	buf_size--;   /* Reserve 1 byte for end-of-string character */

	/*
	 * Loop through each bit in the thread_state. Stop once all have
	 * been processed. If more than one thread_state bit is set, then
	 * separate the descriptive strings with a '+'.
	 */


	বাবে (unsigned পূর্ণাঙ্ক index = 0; thread_state != 0; index++) {
		bit = state_string[index].bit;
		যদি ((thread_state & bit) == 0) {
			অনুৱর্ত;
		}

		off += copy_bytes(buf + off, buf_size - off,
				  state_string[index].str,
				  state_string[index].len);

		thread_state &= ~bit;

		যদি (thread_state != 0) {
			off += copy_bytes(buf + off, buf_size - off, "+", 1);
		}
	}

	buf[off] = '\0';

	ঘূৰাই (ধ্রুৱ ৱর্ণ *)buf;
}

#ifdef CONFIG_USERSPACE
স্থির অন্তঃস্থ পূর্ণাঙ্ক z_vrfy_k_thread_name_copy(k_tid_t thread,
					    ৱর্ণ *buf, size_t size)
{
#ifdef CONFIG_THREAD_NAME
	size_t len;
	সংরচনা k_object *ko = k_object_find(thread);

	/* Special case: we allow reading the names of initialized threads
	 * even if we don't have permission on them
	 */
	যদি ((ko == রিক্ত) || (ko->type != K_OBJ_THREAD) ||
		((ko->flags & K_OBJ_FLAG_INITIALIZED) == 0)) {
		ঘূৰাই -EINVAL;
	}
	যদি (K_SYSCALL_MEMORY_WRITE(buf, size) != 0) {
		ঘূৰাই -EFAULT;
	}
	len = strlen(thread->name);
	যদি ((len + 1) > size) {
		ঘূৰাই -ENOSPC;
	}

	ঘূৰাই k_usermode_to_copy((শূন্য *)buf, thread->name, len + 1);
#অন্যথা
	ARG_UNUSED(thread);
	ARG_UNUSED(buf);
	ARG_UNUSED(size);
	ঘূৰাই -ENOSYS;
#endif /* CONFIG_THREAD_NAME */
}
#অন্তর্ভাৱ <zephyr/syscalls/k_thread_name_copy_mrsh.c>
#endif /* CONFIG_USERSPACE */

#ifdef CONFIG_স্তূপ_SENTINEL
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
শূন্য z_check_স্তূপ_sentinel(শূন্য)
{
	uint32_t *স্তূপ;

	যদি ((_current->base.thread_state & _THREAD_DUMMY) != 0) {
		ঘূৰাই;
	}

	স্তূপ = (uint32_t *)_current->স্তূপ_info.start;
	যদি (*স্তূপ != স্তূপ_SENTINEL) {
		/* Restore it so further checks don't trigger this same error */
		*স্তূপ = স্তূপ_SENTINEL;
		z_except_reason(K_ERR_স্তূপ_CHK_FAIL);
	}
}
#endif /* CONFIG_STACK_SENTINEL */

#যদি defined(CONFIG_স্তূপ_POINTER_RANDOM) && (CONFIG_স্তূপ_POINTER_RANDOM != 0)
পূর্ণাঙ্ক z_স্তূপ_adjust_initialized;

স্থির size_t random_offset(size_t স্তূপ_size)
{
	size_t random_val;

	যদি (!z_স্তূপ_adjust_initialized) {
		z_early_rand_get((uint8_t *)&random_val, পরিমাণ(random_val));
	} অন্যথা {
		sys_rand_get((uint8_t *)&random_val, পরিমাণ(random_val));
	}

	/* Don't need to worry about alignment of the size here,
	 * arch_new_thread() is required to do it.
	 *
	 * FIXME: Not the best way to get a random number in a range.
	 * See #6493
	 */
	ধ্রুৱ size_t fuzz = random_val % CONFIG_স্তূপ_POINTER_RANDOM;

	যদি (unlikely(fuzz * 2 > স্তূপ_size)) {
		ঘূৰাই 0;
	}

	ঘূৰাই fuzz;
}
#যদি defined(CONFIG_স্তূপ_GROWS_UP)
	/* This is so rare not bothering for now */
#error "Stack pointer randomization not implemented for upward growing stacks"
#endif /* CONFIG_STACK_GROWS_UP */
#endif /* CONFIG_STACK_POINTER_RANDOM */

/*
 * Geometry of a thread's stack object, computed once and threaded through the
 * setup_thread_stack() helpers below.
 */
সংরচনা স্তূপ_geometry {
	size_t obj_size;	/* size of the whole stack object */
	ৱর্ণ *buf_start;	/* start of the usable stack buffer */
	size_t buf_size;	/* size of the usable stack buffer */
};

/*
 * Derive the stack object geometry from the caller-supplied stack and size.
 * User-capable stacks and kernel-only stacks reserve different amounts.
 */
স্থির সংরচনা স্তূপ_geometry compute_স্তূপ_geometry(k_thread_স্তূপ_t *স্তূপ,
						    size_t স্তূপ_size)
{
	সংরচনা স্তূপ_geometry geo;

#ifdef CONFIG_USERSPACE
	যদি (z_স্তূপ_is_user_capable(স্তূপ)) {
		geo.obj_size = K_THREAD_স্তূপ_LEN(স্তূপ_size);
		geo.buf_start = K_THREAD_স্তূপ_BUFFER(স্তূপ);
		geo.buf_size = geo.obj_size - K_THREAD_স্তূপ_RESERVED;
	} অন্যথা
#endif /* CONFIG_USERSPACE */
	{
		/* Object cannot host a user mode thread */
		geo.obj_size = K_KERNEL_স্তূপ_LEN(স্তূপ_size);
		geo.buf_start = K_KERNEL_স্তূপ_BUFFER(স্তূপ);
		geo.buf_size = geo.obj_size - K_KERNEL_স্তূপ_RESERVED;

#যদি defined(ARCH_KERNEL_স্তূপ_RESERVED)
		/* Zephyr treats stack overflow as an app bug.  But
		 * this particular overflow can be seen by static
		 * analysis so needs to be handled somehow.
		 */
		যদি (K_KERNEL_স্তূপ_RESERVED > geo.obj_size) {
			k_panic();
		}
#endif
	}

	ঘূৰাই geo;
}

/*
 * Return the initial (high end) stack pointer. When stacks are memory mapped
 * this maps the object into virtual memory, records the mapping so it can be
 * torn down at thread exit, and rebases buf_start onto the mapped address.
 */
স্থির ৱর্ণ *map_thread_স্তূপ(সংরচনা k_thread *new_thread,
			      k_thread_স্তূপ_t *স্তূপ,
			      সংরচনা স্তূপ_geometry *geo)
{
	ৱর্ণ *স্তূপ_ptr;

#ifdef CONFIG_THREAD_স্তূপ_MEM_MAPPED
	/* Map the stack into virtual memory and use that as the base to
	 * calculate the initial stack pointer at the high end of the stack
	 * object. The stack pointer may be reduced later by TLS or random
	 * offset.
	 *
	 * K_MEM_MAP_UNINIT is used to mimic the behavior of non-mapped
	 * stack. If CONFIG_INIT_STACKS is enabled, the stack will be
	 * cleared later.
	 */
	শূন্য *স্তূপ_mapped = k_mem_map_phys_guard((uintptr_t)স্তূপ, geo->obj_size,
				K_MEM_PERM_RW | K_MEM_CACHE_WB | K_MEM_MAP_UNINIT,
				false);

	__ASSERT_NO_MSG((uintptr_t)স্তূপ_mapped != 0);

#ifdef CONFIG_USERSPACE
	যদি (z_স্তূপ_is_user_capable(স্তূপ)) {
		geo->buf_start = K_THREAD_স্তূপ_BUFFER(স্তূপ_mapped);
	} অন্যথা
#endif /* CONFIG_USERSPACE */
	{
		geo->buf_start = K_KERNEL_স্তূপ_BUFFER(স্তূপ_mapped);
	}

	স্তূপ_ptr = (ৱর্ণ *)স্তূপ_mapped + geo->obj_size;

	/* Need to store the info on mapped stack so we can remove the mappings
	 * when the thread ends.
	 */
	new_thread->স্তূপ_info.mapped.addr = স্তূপ_mapped;
	new_thread->স্তূপ_info.mapped.sz = geo->obj_size;
#অন্যথা /* CONFIG_THREAD_STACK_MEM_MAPPED */
	ARG_UNUSED(new_thread);

	/* Initial stack pointer at the high end of the stack object, may
	 * be reduced later by TLS or random offset
	 */
	স্তূপ_ptr = (ৱর্ণ *)স্তূপ + geo->obj_size;
#endif /* CONFIG_THREAD_STACK_MEM_MAPPED */

	ঘূৰাই স্তূপ_ptr;
}

/* Pre-fill the stack buffer with a known pattern for usage measurement. */
স্থির অন্তঃস্থ শূন্য fill_init_স্তূপ(ৱর্ণ *buf_start, size_t buf_size)
{
#ifdef CONFIG_INIT_STACKS
	memset(buf_start, 0xaa, buf_size);
#অন্যথা
	ARG_UNUSED(buf_start);
	ARG_UNUSED(buf_size);
#endif /* CONFIG_INIT_STACKS */
}

/* Place the overflow-detection sentinel at the low end of the stack buffer. */
স্থির অন্তঃস্থ শূন্য set_স্তূপ_sentinel(ৱর্ণ *buf_start)
{
#ifdef CONFIG_স্তূপ_SENTINEL
	/* Put the stack sentinel at the lowest 4 bytes of the stack area.
	 * We periodically check that it's still present and kill the thread
	 * if it isn't.
	 */
	*((uint32_t *)buf_start) = স্তূপ_SENTINEL;
#অন্যথা
	ARG_UNUSED(buf_start);
#endif /* CONFIG_STACK_SENTINEL */
}

/*
 * Compute how much to reserve at the top of the stack buffer for TLS,
 * userspace-local data and stack-pointer randomization, and return the
 * aligned amount the initial stack pointer must be lowered by.
 */
স্থির size_t reserve_স্তূপ_headroom(সংরচনা k_thread *new_thread,
				     ৱর্ণ *স্তূপ_ptr, size_t buf_size)
{
	size_t delta = 0;

	ARG_UNUSED(new_thread);
	ARG_UNUSED(স্তূপ_ptr);
	ARG_UNUSED(buf_size);

#ifdef CONFIG_THREAD_LOCAL_STORAGE
	/* TLS is always last within the stack buffer */
	delta += arch_tls_স্তূপ_setup(new_thread, স্তূপ_ptr);
#endif /* CONFIG_THREAD_LOCAL_STORAGE */
#ifdef CONFIG_THREAD_USERSPACE_LOCAL_DATA
	size_t tls_size = পরিমাণ(সংরচনা _thread_userspace_local_data);

	/* reserve space on highest memory of stack buffer for local data */
	delta += tls_size;
	new_thread->userspace_local_data =
		(সংরচনা _thread_userspace_local_data *)(স্তূপ_ptr - delta);
#endif /* CONFIG_THREAD_USERSPACE_LOCAL_DATA */
#যদি defined(CONFIG_স্তূপ_POINTER_RANDOM) && (CONFIG_স্তূপ_POINTER_RANDOM != 0)
	delta += random_offset(buf_size);
#endif /* CONFIG_STACK_POINTER_RANDOM */

	ঘূৰাই ROUND_UP(delta, ARCH_স্তূপ_PTR_ALIGN);
}

/* Record the thread-accessible stack bounds (and runtime-safety threshold). */
স্থির অন্তঃস্থ শূন্য set_স্তূপ_info(সংরচনা k_thread *new_thread, ৱর্ণ *buf_start,
				  size_t buf_size, size_t delta)
{
#ifdef CONFIG_THREAD_স্তূপ_INFO
	/* Initial values. Arches which implement MPU guards that "borrow"
	 * memory from the stack buffer (not tracked in K_THREAD_STACK_RESERVED)
	 * will need to appropriately update this.
	 *
	 * The bounds tracked here correspond to the area of the stack object
	 * that the thread can access, which includes TLS.
	 */
	new_thread->স্তূপ_info.start = (uintptr_t)buf_start;
	new_thread->স্তূপ_info.size = buf_size;
	new_thread->স্তূপ_info.delta = delta;

#ifdef CONFIG_THREAD_RUNTIME_স্তূপ_SAFETY
	new_thread->স্তূপ_info.usage.unused_threshold =
		(CONFIG_THREAD_RUNTIME_স্তূপ_SAFETY_DEFAULT_UNUSED_THRESHOLD_PCT *
		 buf_size) / 100;
#endif
#অন্যথা
	ARG_UNUSED(new_thread);
	ARG_UNUSED(buf_start);
	ARG_UNUSED(buf_size);
	ARG_UNUSED(delta);
#endif /* CONFIG_THREAD_STACK_INFO */
}

স্থির ৱর্ণ *setup_thread_স্তূপ(সংরচনা k_thread *new_thread,
				k_thread_স্তূপ_t *স্তূপ, size_t স্তূপ_size)
{
	সংরচনা স্তূপ_geometry geo = compute_স্তূপ_geometry(স্তূপ, স্তূপ_size);
	ৱর্ণ *স্তূপ_ptr = map_thread_স্তূপ(new_thread, স্তূপ, &geo);
	size_t delta;

	LOG_DBG("stack %p for thread %p: obj_size=%zu buf_start=%p "
		" buf_size %zu stack_ptr=%p",
		স্তূপ, new_thread, geo.obj_size, (শূন্য *)geo.buf_start,
		geo.buf_size, (শূন্য *)স্তূপ_ptr);

	fill_init_স্তূপ(geo.buf_start, geo.buf_size);
	set_স্তূপ_sentinel(geo.buf_start);
	delta = reserve_স্তূপ_headroom(new_thread, স্তূপ_ptr, geo.buf_size);
	set_স্তূপ_info(new_thread, geo.buf_start, geo.buf_size, delta);

	ঘূৰাই স্তূপ_ptr - delta;
}

#ifdef CONFIG_HW_SHADOW_স্তূপ
স্থির শূন্য setup_shadow_স্তূপ(সংরচনা k_thread *new_thread,
			 k_thread_স্তূপ_t *স্তূপ)
{
	পূর্ণাঙ্ক ret = -ENOENT;

	STRUCT_SECTION_FOREACH(স্তূপ_to_hw_shadow_স্তূপ, stk_to_hw_shstk) {
		যদি (stk_to_hw_shstk->স্তূপ == স্তূপ) {
			ret = k_thread_hw_shadow_স্তূপ_attach(new_thread,
							      stk_to_hw_shstk->shstk_addr,
							      stk_to_hw_shstk->size);
			যদি (ret != 0) {
				LOG_ERR("Could not set thread %p shadow stack %p, got error %d",
					new_thread, stk_to_hw_shstk->shstk_addr, ret);
				k_panic();
			}
			ভঙ্গ;
		}
	}

	/* Check if the stack isn't in a stack array, and use the corresponding
	 * shadow stack.
	 */
	যদি (ret != -ENOENT) {
		ঘূৰাই;
	}

	STRUCT_SECTION_FOREACH(স্তূপ_to_hw_shadow_স্তূপ_arr, stk_to_hw_shstk) {
		যদি ((uintptr_t)স্তূপ >= stk_to_hw_shstk->স্তূপ_addr &&
		    (uintptr_t)স্তূপ < stk_to_hw_shstk->স্তূপ_addr +
		    stk_to_hw_shstk->স্তূপ_size * stk_to_hw_shstk->nmemb) {
			/* Now we have to guess which index of the stack array is being used */
			uintptr_t স্তূপ_offset = (uintptr_t)স্তূপ - stk_to_hw_shstk->স্তূপ_addr;
			uintptr_t স্তূপ_index = স্তূপ_offset / stk_to_hw_shstk->স্তূপ_size;
			uintptr_t addr;

			যদি (স্তূপ_index >= stk_to_hw_shstk->nmemb) {
				LOG_ERR("Could not find shadow stack for thread %p, stack %p",
					new_thread, স্তূপ);
				k_panic();
			}

			addr = stk_to_hw_shstk->shstk_addr +
				stk_to_hw_shstk->shstk_size * স্তূপ_index;
			ret = k_thread_hw_shadow_স্তূপ_attach(new_thread,
							      (arch_thread_hw_shadow_স্তূপ_t *)addr,
							      stk_to_hw_shstk->shstk_size);
			যদি (ret != 0) {
				LOG_ERR("Could not set thread %p shadow stack 0x%lx, got error %d",
					new_thread, stk_to_hw_shstk->shstk_addr, ret);
				k_panic();
			}
			ভঙ্গ;
		}
	}

	যদি (ret == -ENOENT) {
		LOG_ERR("Could not find shadow stack for thread %p, stack %p",
			new_thread, স্তূপ);
		k_panic();
	}
}
#অন্যথা
/* No-op stub used when hardware shadow stacks are not configured. */
স্থির অন্তঃস্থ শূন্য setup_shadow_স্তূপ(সংরচনা k_thread *new_thread,
				      k_thread_স্তূপ_t *স্তূপ)
{
	ARG_UNUSED(new_thread);
	ARG_UNUSED(স্তূপ);
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
স্থির অন্তঃস্থ শূন্য thread_abort_cleanup_check_reuse(সংরচনা k_thread *thread)
{
#ifdef CONFIG_THREAD_ABORT_NEED_CLEANUP
	k_thread_abort_cleanup_check_reuse(thread);
#অন্যথা
	ARG_UNUSED(thread);
#endif /* CONFIG_THREAD_ABORT_NEED_CLEANUP */
}

/* Register the thread with the object-core framework and its usage stats. */
স্থির অন্তঃস্থ শূন্য init_thread_obj_core(সংরচনা k_thread *thread)
{
#ifdef CONFIG_OBJ_CORE_THREAD
	k_obj_core_init_and_link(K_OBJ_CORE(thread), &obj_type_thread);
#ifdef CONFIG_OBJ_CORE_STATS_THREAD
	k_obj_core_stats_register(K_OBJ_CORE(thread), &thread->base.usage,
				  পরিমাণ(thread->base.usage));
#endif /* CONFIG_OBJ_CORE_STATS_THREAD */
#অন্যথা
	ARG_UNUSED(thread);
#endif /* CONFIG_OBJ_CORE_THREAD */
}

/*
 * Initialize userspace bookkeeping: register the thread and stack kernel
 * objects, record stack ownership, and grant the thread access to itself.
 */
স্থির অন্তঃস্থ শূন্য init_thread_userspace(সংরচনা k_thread *thread,
					 k_thread_স্তূপ_t *স্তূপ,
					 uint32_t options)
{
#ifdef CONFIG_USERSPACE
	__ASSERT((options & K_USER) == 0U || z_স্তূপ_is_user_capable(স্তূপ),
		 "user thread %p with kernel-only stack %p", thread, স্তূপ);
	k_object_init(thread);
	k_object_init(স্তূপ);
	thread->স্তূপ_obj = স্তূপ;
	thread->syscall_frame = রিক্ত;

	/* Any given thread has access to itself */
	k_object_access_grant(thread, thread);
#অন্যথা
	ARG_UNUSED(thread);
	ARG_UNUSED(স্তূপ);
	ARG_UNUSED(options);
#endif /* CONFIG_USERSPACE */
}

/* Assert the thread object is cache-coherent while its stack stays cached. */
স্থির অন্তঃস্থ শূন্য assert_thread_coherence(সংরচনা k_thread *thread,
					   k_thread_স্তূপ_t *স্তূপ)
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
	__ASSERT_NO_MSG(!sys_cache_is_mem_coherent(স্তূপ));
#endif /* CONFIG_DYNAMIC_THREAD */
#endif /* CONFIG_KERNEL_COHERENCE */
	ARG_UNUSED(thread);
	ARG_UNUSED(স্তূপ);
}

/* Assert the arch layer populated switch_handle on USE_SWITCH builds. */
স্থির অন্তঃস্থ শূন্য assert_switch_handle(সংরচনা k_thread *thread)
{
#ifdef CONFIG_USE_SWITCH
	/* switch_handle must be non-null except when inside z_swap()
	 * for synchronization reasons.  Historically some notional
	 * USE_SWITCH architectures have actually ignored the field
	 */
	__ASSERT(thread->switch_handle != রিক্ত,
		 "arch layer failed to initialize switch_handle");
#endif /* CONFIG_USE_SWITCH */
	ARG_UNUSED(thread);
}

/* Clear the opaque, kernel-managed per-thread custom data pointer. */
স্থির অন্তঃস্থ শূন্য init_thread_custom_data(সংরচনা k_thread *thread)
{
#ifdef CONFIG_THREAD_CUSTOM_DATA
	/* Initialize custom data field (value is opaque to kernel) */
	thread->custom_data = রিক্ত;
#অন্যথা
	ARG_UNUSED(thread);
#endif /* CONFIG_THREAD_CUSTOM_DATA */
}

/*
 * Record the entry point and parameters, then link the thread into the
 * global thread-monitor list under the monitor spinlock.
 */
স্থির অন্তঃস্থ শূন্য add_thread_to_monitor(সংরচনা k_thread *thread,
					 k_thread_entry_t entry,
					 শূন্য *p1, শূন্য *p2, শূন্য *p3)
{
#ifdef CONFIG_THREAD_MONITOR
	thread->entry.pEntry = entry;
	thread->entry.parameter1 = p1;
	thread->entry.parameter2 = p2;
	thread->entry.parameter3 = p3;

	k_spinlock_চাবি_t চাবি = k_spin_lock(&z_thread_monitor_lock);

	thread->next_thread = _kernel.threads;
	_kernel.threads = thread;
	k_spin_unlock(&z_thread_monitor_lock, চাবি);
#অন্যথা
	ARG_UNUSED(thread);
	ARG_UNUSED(entry);
	ARG_UNUSED(p1);
	ARG_UNUSED(p2);
	ARG_UNUSED(p3);
#endif /* CONFIG_THREAD_MONITOR */
}

/* Set the thread's name from the caller-supplied string (may be NULL). */
স্থির অন্তঃস্থ শূন্য init_thread_name(সংরচনা k_thread *thread, ধ্রুৱ ৱর্ণ *name)
{
#ifdef CONFIG_THREAD_NAME
	set_thread_name(thread, name);
#অন্যথা
	ARG_UNUSED(thread);
	ARG_UNUSED(name);
#endif /* CONFIG_THREAD_NAME */
}

/* Set the thread's initial SMP CPU affinity mask. */
স্থির অন্তঃস্থ শূন্য init_thread_cpu_mask(সংরচনা k_thread *thread)
{
#ifdef CONFIG_SCHED_CPU_MASK
	যদি (IS_ENABLED(CONFIG_SCHED_CPU_MASK_PIN_ONLY)) {
		thread->base.cpu_mask = 1; /* must specify only one cpu */
	} অন্যথা {
		thread->base.cpu_mask = -1; /* allow all cpus */
	}
#অন্যথা
	ARG_UNUSED(thread);
#endif /* CONFIG_SCHED_CPU_MASK */
}

/* Attach the thread to its memory domain and inherit caller permissions
 * when K_INHERIT_PERMS is requested.
 */
স্থির অন্তঃস্থ শূন্য init_thread_userspace_perms(সংরচনা k_thread *thread,
					       uint32_t options)
{
#ifdef CONFIG_USERSPACE
	z_mem_domain_init_thread(thread);

	যদি ((options & K_INHERIT_PERMS) != 0U) {
		k_thread_perms_inherit(_current, thread);
	}
#অন্যথা
	ARG_UNUSED(thread);
	ARG_UNUSED(options);
#endif /* CONFIG_USERSPACE */
}

/* Clear the EDF scheduling deadline. */
স্থির অন্তঃস্থ শূন্য init_thread_deadline(সংরচনা k_thread *thread)
{
#ifdef CONFIG_SCHED_DEADLINE
	thread->base.prio_deadline = 0;
#অন্যথা
	ARG_UNUSED(thread);
#endif /* CONFIG_SCHED_DEADLINE */
}

/* Initialize the wait queue used to synchronize SMP thread halt/abort. */
স্থির অন্তঃস্থ শূন্য init_thread_halt_queue(সংরচনা k_thread *thread)
{
#ifdef CONFIG_SMP
	z_waitq_init(&thread->halt_queue);
#অন্যথা
	ARG_UNUSED(thread);
#endif /* CONFIG_SMP */
}

/* Initialize runtime CPU-usage tracking for the thread. */
স্থির অন্তঃস্থ শূন্য init_thread_usage(সংরচনা k_thread *thread)
{
#ifdef CONFIG_SCHED_THREAD_USAGE
	thread->base.usage = (সংরচনা k_cycle_stats) {};
	thread->base.usage.track_usage = CONFIG_SCHED_THREAD_USAGE_AUTO_ENABLE;
#অন্যথা
	ARG_UNUSED(thread);
#endif /* CONFIG_SCHED_THREAD_USAGE */
}

/* Assert the thread wasn't already linked into the monitor list. */
স্থির অন্তঃস্থ শূন্য assert_thread_not_reused(সংরচনা k_thread *new_thread)
{
#ifdef CONFIG_THREAD_MONITOR
#ifdef CONFIG_ASSERT
	k_spinlock_চাবি_t চাবি = k_spin_lock(&z_thread_monitor_lock);
	bool reused = false;

	/* Check if the thread is already in the list */
	বাবে (সংরচনা k_thread *t = _kernel.threads; t; t = t->next_thread) {
		যদি (t == new_thread) {
			reused = true;
			ভঙ্গ;
		}
	}

	k_spin_unlock(&z_thread_monitor_lock, চাবি);

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
ৱর্ণ *z_setup_new_thread(সংরচনা k_thread *new_thread,
			 k_thread_স্তূপ_t *স্তূপ, size_t স্তূপ_size,
			 k_thread_entry_t entry,
			 শূন্য *p1, শূন্য *p2, শূন্য *p3,
			 পূর্ণাঙ্ক prio, uint32_t options, ধ্রুৱ ৱর্ণ *name)
{
	ৱর্ণ *স্তূপ_ptr;

	Z_ASSERT_VALID_PRIO(prio, entry);

	assert_thread_not_reused(new_thread);

	thread_abort_cleanup_check_reuse(new_thread);
	init_thread_obj_core(new_thread);
	init_thread_userspace(new_thread, স্তূপ, options);

	z_waitq_init(&new_thread->join_queue);

	/* Initialize various struct k_thread members */
	z_init_thread_base(&new_thread->base, prio, _THREAD_SLEEPING, options);
	স্তূপ_ptr = setup_thread_স্তূপ(new_thread, স্তূপ, স্তূপ_size);

#যদি Z_MUTEX_PI_ENABLED
	sys_slist_init(&new_thread->held_mutexes);
	new_thread->mutex_pended_on = রিক্ত;
	new_thread->orig_prio = new_thread->base.prio;
#endif /* Z_MUTEX_PI_ENABLED */

	setup_shadow_স্তূপ(new_thread, স্তূপ);
	assert_thread_coherence(new_thread, স্তূপ);

	arch_new_thread(new_thread, স্তূপ, স্তূপ_ptr, entry, p1, p2, p3);

	/* static threads overwrite it afterwards with real value */
	new_thread->init_data = রিক্ত;

	assert_switch_handle(new_thread);
	init_thread_custom_data(new_thread);
	add_thread_to_monitor(new_thread, entry, p1, p2, p3);
	init_thread_name(new_thread, name);
	init_thread_cpu_mask(new_thread);

	/* _current may be NULL if the dummy thread is not used */
	যদি (IS_ENABLED(CONFIG_ARCH_HAS_CUSTOM_SWAP_TO_MAIN) &&
	    (_current == রিক্ত)) {
		new_thread->resource_pool = রিক্ত;
		ঘূৰাই স্তূপ_ptr;
	}

	init_thread_userspace_perms(new_thread, options);
	init_thread_deadline(new_thread);
	new_thread->resource_pool = _current->resource_pool;
	init_thread_halt_queue(new_thread);
	init_thread_usage(new_thread);

	SYS_PORT_TRACING_OBJ_FUNC(k_thread, create, new_thread);

	ঘূৰাই স্তূপ_ptr;
}

#ifdef CONFIG_THREAD_RUNTIME_স্তূপ_SAFETY
পূর্ণাঙ্ক z_impl_k_thread_runtime_স্তূপ_unused_threshold_pct_set(সংরচনা k_thread *thread,
							   uint32_t pct)
{
	size_t unused_threshold;

	যদি (pct > 99) {
		ঘূৰাই -EINVAL;   /* 100% unused stack and up is invalid */
	}

	unused_threshold = (thread->স্তূপ_info.size * pct) / 100;

	thread->স্তূপ_info.usage.unused_threshold = unused_threshold;

	ঘূৰাই 0;
}

পূর্ণাঙ্ক z_impl_k_thread_runtime_স্তূপ_unused_threshold_set(সংরচনা k_thread *thread,
						       size_t threshold)
{
	যদি (threshold > thread->স্তূপ_info.size) {
		ঘূৰাই -EINVAL;
	}

	thread->স্তূপ_info.usage.unused_threshold = threshold;

	ঘূৰাই 0;
}

size_t z_impl_k_thread_runtime_স্তূপ_unused_threshold_get(সংরচনা k_thread *thread)
{
	ঘূৰাই thread->স্তূপ_info.usage.unused_threshold;
}

#ifdef CONFIG_USERSPACE
পূর্ণাঙ্ক z_vrfy_k_thread_runtime_স্তূপ_unused_threshold_pct_set(সংরচনা k_thread *thread,
							   uint32_t pct)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));

	ঘূৰাই z_impl_k_thread_runtime_স্তূপ_unused_threshold_pct_set(thread, pct);
}
#অন্তর্ভাৱ <zephyr/syscalls/k_thread_runtime_stack_unused_threshold_pct_set_mrsh.c>

পূর্ণাঙ্ক z_vrfy_k_thread_runtime_স্তূপ_unused_threshold_set(সংরচনা k_thread *thread,
						       size_t threshold)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));

	ঘূৰাই z_impl_k_thread_runtime_স্তূপ_unused_threshold_set(thread, threshold);
}
#অন্তর্ভাৱ <zephyr/syscalls/k_thread_runtime_stack_unused_threshold_set_mrsh.c>

size_t z_vrfy_k_thread_runtime_স্তূপ_unused_threshold_get(সংরচনা k_thread *thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));

	ঘূৰাই z_impl_k_thread_runtime_স্তূপ_unused_threshold_get(thread);
}
#অন্তর্ভাৱ <zephyr/syscalls/k_thread_runtime_stack_unused_threshold_get_mrsh.c>
#endif /* CONFIG_USERSPACE */
#endif /* CONFIG_THREAD_RUNTIME_STACK_SAFETY */

k_tid_t z_impl_k_thread_create(সংরচনা k_thread *new_thread,
			      k_thread_স্তূপ_t *স্তূপ,
			      size_t স্তূপ_size, k_thread_entry_t entry,
			      শূন্য *p1, শূন্য *p2, শূন্য *p3,
			      পূর্ণাঙ্ক prio, uint32_t options, k_timeout_t delay)
{
	__ASSERT(!arch_is_in_isr(), "Threads may not be created in ISRs");

	z_setup_new_thread(new_thread, স্তূপ, স্তূপ_size, entry, p1, p2, p3,
			  prio, options, রিক্ত);

	thread_schedule_new(new_thread, delay);

	ঘূৰাই new_thread;
}

#ifdef CONFIG_USERSPACE
bool z_স্তূপ_is_user_capable(k_thread_স্তূপ_t *স্তূপ)
{
	ঘূৰাই k_object_find(স্তূপ) != রিক্ত;
}

k_tid_t z_vrfy_k_thread_create(সংরচনা k_thread *new_thread,
			       k_thread_স্তূপ_t *স্তূপ,
			       size_t স্তূপ_size, k_thread_entry_t entry,
			       শূন্য *p1, শূন্য *p2, শূন্য *p3,
			       পূর্ণাঙ্ক prio, uint32_t options, k_timeout_t delay)
{
	size_t total_size, স্তূপ_obj_size;
	সংরচনা k_object *স্তূপ_object;

	/* The thread and stack objects *must* be in an uninitialized state */
	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(new_thread, K_OBJ_THREAD));

	/* No need to check z_stack_is_user_capable(), it won't be in the
	 * object table if it isn't
	 */
	স্তূপ_object = k_object_find(স্তূপ);
	K_OOPS(K_SYSCALL_VERIFY_বাৰ্তা(k_object_validation_check(স্তূপ_object, স্তূপ,
						K_OBJ_THREAD_স্তূপ_ELEMENT,
						_OBJ_INIT_FALSE) == 0,
				    "bad stack object"));

	/* Verify that the stack size passed in is OK by computing the total
	 * size and comparing it with the size value in the object metadata
	 */
	K_OOPS(K_SYSCALL_VERIFY_বাৰ্তা(!size_add_overflow(K_THREAD_স্তূপ_RESERVED,
						       স্তূপ_size, &total_size),
				    "stack size overflow (%zu+%zu)",
				    স্তূপ_size,
				    K_THREAD_স্তূপ_RESERVED));

	/* Testing less-than-or-equal since additional room may have been
	 * allocated for alignment constraints
	 */
#ifdef CONFIG_GEN_PRIV_STACKS
	স্তূপ_obj_size = স্তূপ_object->data.স্তূপ_data->size;
#অন্যথা
	স্তূপ_obj_size = স্তূপ_object->data.স্তূপ_size;
#endif /* CONFIG_GEN_PRIV_STACKS */
	K_OOPS(K_SYSCALL_VERIFY_বাৰ্তা(total_size <= স্তূপ_obj_size,
				    "stack size %zu is too big, max is %zu",
				    total_size, স্তূপ_obj_size));

	/* User threads may only create other user threads and they can't
	 * be marked as essential
	 */
	K_OOPS(K_SYSCALL_VERIFY(options & K_USER));
	K_OOPS(K_SYSCALL_VERIFY(!(options & K_ESSENTIAL)));

	/* Check validity of prio argument; must be the same or worse priority
	 * than the caller
	 */
	K_OOPS(K_SYSCALL_VERIFY(_is_valid_prio(prio, রিক্ত)));
	K_OOPS(K_SYSCALL_VERIFY(z_is_prio_lower_or_equal(prio,
							_current->base.prio)));

	z_setup_new_thread(new_thread, স্তূপ, স্তূপ_size,
			   entry, p1, p2, p3, prio, options, রিক্ত);

	thread_schedule_new(new_thread, delay);

	ঘূৰাই new_thread;
}
#অন্তর্ভাৱ <zephyr/syscalls/k_thread_create_mrsh.c>
#endif /* CONFIG_USERSPACE */

শূন্য z_init_thread_base(সংরচনা _thread_base *thread_base, পূর্ণাঙ্ক priority,
		       uint32_t initial_state, unsigned পূর্ণাঙ্ক options)
{
	/* k_q_node is initialized upon first insertion in a list */
	thread_base->pended_on = রিক্ত;
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
	thread_base->slice_expired = রিক্ত;
#endif /* CONFIG_TIMESLICE_PER_THREAD */

#ifdef CONFIG_SPIN_VALIDATE
	thread_base->swap_data = রিক্ত;
	/* otherwise swap_data does not need to be initialized */
#endif

	z_init_thread_timeout(thread_base);
}

FUNC_NORETURN শূন্য k_thread_user_mode_enter(k_thread_entry_t entry,
					    শূন্য *p1, শূন্য *p2, শূন্য *p3)
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
	__ASSERT(z_স্তূপ_is_user_capable(_current->স্তূপ_obj),
		 "dropping to user mode with kernel-only stack object");
#ifdef CONFIG_THREAD_USERSPACE_LOCAL_DATA
	memset(_current->userspace_local_data, 0,
	       পরিমাণ(সংরচনা _thread_userspace_local_data));
#endif /* CONFIG_THREAD_USERSPACE_LOCAL_DATA */
#ifdef CONFIG_THREAD_LOCAL_STORAGE
	arch_tls_স্তূপ_setup(_current,
			     (ৱর্ণ *)(_current->স্তূপ_info.start +
				      _current->স্তূপ_info.size));
#endif /* CONFIG_THREAD_LOCAL_STORAGE */
	arch_user_mode_enter(entry, p1, p2, p3);
#অন্যথা
	/* XXX In this case we do not reset the stack */
	z_thread_entry(entry, p1, p2, p3);
#endif /* CONFIG_USERSPACE */
}

#যদি defined(CONFIG_INIT_STACKS) && defined(CONFIG_THREAD_স্তূপ_INFO)
#ifdef CONFIG_স্তূপ_GROWS_UP
#error "Unsupported configuration for stack analysis"
#endif /* CONFIG_STACK_GROWS_UP */

পূর্ণাঙ্ক z_স্তূপ_space_get(ধ্রুৱ uint8_t *স্তূপ_start, size_t size, size_t *unused_ptr)
{
	size_t unused = 0;
	ধ্রুৱ uint8_t *checked_স্তূপ = স্তূপ_start;
	/* Take the address of any local variable as a shallow bound for the
	 * stack pointer.  Addresses above it are guaranteed to be
	 * accessible.
	 */
	ধ্রুৱ uint8_t *স্তূপ_pointer = (ধ্রুৱ uint8_t *)&স্তূপ_start;

	/* If we are currently running on the stack being analyzed, some
	 * memory management hardware will generate an exception if we
	 * read unused stack memory.
	 *
	 * This never happens when invoked from user mode, as user mode
	 * will always run this function on the privilege elevation stack.
	 */
	যদি ((স্তূপ_pointer > স্তূপ_start) && (স্তূপ_pointer <= (স্তূপ_start + size)) &&
	    IS_ENABLED(CONFIG_NO_UNUSED_স্তূপ_INSPECTION)) {
		/* TODO: We could add an arch_ API call to temporarily
		 * disable the stack checking in the CPU, but this would
		 * need to be properly managed wrt context switches/interrupts
		 */
		ঘূৰাই -ENOTSUP;
	}

	যদি (IS_ENABLED(CONFIG_স্তূপ_SENTINEL)) {
		/* First 4 bytes of the stack buffer reserved for the
		 * sentinel value, it won't be 0xAAAAAAAA for thread
		 * stacks.
		 *
		 * FIXME: thread->stack_info.start ought to reflect
		 * this!
		 */
		checked_স্তূপ += 4;
		size -= 4;
	}

	বাবে (size_t i = 0; i < size; i++) {
		যদি ((checked_স্তূপ[i]) == 0xaaU) {
			unused++;
		} অন্যথা {
			ভঙ্গ;
		}
	}

	*unused_ptr = unused;

	ঘূৰাই 0;
}

#ifdef CONFIG_THREAD_RUNTIME_স্তূপ_SAFETY
পূর্ণাঙ্ক k_thread_runtime_স্তূপ_safety_full_check(ধ্রুৱ সংরচনা k_thread *thread,
					     size_t *unused_ptr,
					     k_thread_স্তূপ_safety_handler_t handler,
					     শূন্য *arg)
{
	পূর্ণাঙ্ক    rv;
	size_t unused_space;

	__ASSERT_NO_MSG(thread != রিক্ত);

	rv = z_স্তূপ_space_get((ধ্রুৱ uint8_t *)thread->স্তূপ_info.start,
			       thread->স্তূপ_info.size, &unused_space);

	যদি (rv != 0) {
		ঘূৰাই rv;
	}

	যদি (unused_ptr != রিক্ত) {
		*unused_ptr = unused_space;
	}

	যদি ((unused_space < thread->স্তূপ_info.usage.unused_threshold) &&
	    (handler != রিক্ত)) {
		handler(thread, unused_space, arg);
	}

	ঘূৰাই 0;
}

পূর্ণাঙ্ক k_thread_runtime_স্তূপ_safety_threshold_check(ধ্রুৱ সংরচনা k_thread *thread,
						  size_t *unused_ptr,
						  k_thread_স্তূপ_safety_handler_t handler,
						  শূন্য *arg)
{
	পূর্ণাঙ্ক    rv;
	size_t unused_space;
	size_t threshold;
	size_t scan_size;

	__ASSERT_NO_MSG(thread != রিক্ত);

	threshold = thread->স্তূপ_info.usage.unused_threshold;
	scan_size = threshold;

	/* z_stack_space_get() reserves the first bytes of the stack buffer for
	 * the stack sentinel and shrinks its scan window by that amount. Grow
	 * the requested window by the same reservation so that up to
	 * 'threshold' bytes of the usable stack are actually examined;
	 * otherwise a fully unused window would report fewer than 'threshold'
	 * unused bytes and spuriously trip the threshold.
	 */
	যদি (IS_ENABLED(CONFIG_স্তূপ_SENTINEL) && (threshold != 0U)) {
		scan_size += পরিমাণ(uint32_t);
	}

	rv = z_স্তূপ_space_get((ধ্রুৱ uint8_t *)thread->স্তূপ_info.start,
			       scan_size, &unused_space);

	যদি (rv != 0) {
		ঘূৰাই rv;
	}

	যদি (unused_ptr != রিক্ত) {
		*unused_ptr = unused_space;
	}

	যদি ((unused_space < threshold) && (handler != রিক্ত)) {
		handler(thread, unused_space, arg);
	}

	ঘূৰাই 0;
}
#endif /* CONFIG_THREAD_RUNTIME_STACK_SAFETY */

পূর্ণাঙ্ক z_impl_k_thread_স্তূপ_space_get(ধ্রুৱ সংরচনা k_thread *thread,
				    size_t *unused_ptr)
{
#ifdef CONFIG_THREAD_স্তূপ_MEM_MAPPED
	যদি (thread->স্তূপ_info.mapped.addr == রিক্ত) {
		ঘূৰাই -EINVAL;
	}
#endif /* CONFIG_THREAD_STACK_MEM_MAPPED */

	ঘূৰাই z_স্তূপ_space_get((ধ্রুৱ uint8_t *)thread->স্তূপ_info.start,
				 thread->স্তূপ_info.size, unused_ptr);
}

#ifdef CONFIG_USERSPACE
পূর্ণাঙ্ক z_vrfy_k_thread_স্তূপ_space_get(ধ্রুৱ সংরচনা k_thread *thread,
				    size_t *unused_ptr)
{
	size_t unused;
	পূর্ণাঙ্ক ret;

	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));

	ret = z_impl_k_thread_স্তূপ_space_get(thread, &unused);
	CHECKIF(ret != 0) {
		ঘূৰাই ret;
	}

	ret = k_usermode_to_copy(unused_ptr, &unused, পরিমাণ(size_t));
	CHECKIF(ret != 0) {
		ঘূৰাই ret;
	}

	ঘূৰাই 0;
}
#অন্তর্ভাৱ <zephyr/syscalls/k_thread_stack_space_get_mrsh.c>
#endif /* CONFIG_USERSPACE */
#endif /* CONFIG_INIT_STACKS && CONFIG_THREAD_STACK_INFO */

#ifdef CONFIG_USERSPACE
স্থির অন্তঃস্থ k_ticks_t z_vrfy_k_thread_timeout_remaining_ticks(
						    ধ্রুৱ সংরচনা k_thread *thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	ঘূৰাই z_impl_k_thread_timeout_remaining_ticks(thread);
}
#অন্তর্ভাৱ <zephyr/syscalls/k_thread_timeout_remaining_ticks_mrsh.c>

স্থির অন্তঃস্থ k_ticks_t z_vrfy_k_thread_timeout_expires_ticks(
						  ধ্রুৱ সংরচনা k_thread *thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	ঘূৰাই z_impl_k_thread_timeout_expires_ticks(thread);
}
#অন্তর্ভাৱ <zephyr/syscalls/k_thread_timeout_expires_ticks_mrsh.c>
#endif /* CONFIG_USERSPACE */

#ifdef CONFIG_INSTRUMENT_THREAD_SWITCHING
শূন্য z_thread_mark_switched_in(শূন্য)
{
#যদি defined(CONFIG_SCHED_THREAD_USAGE) && !defined(CONFIG_USE_SWITCH)
	z_sched_usage_start(_current);
#endif /* CONFIG_SCHED_THREAD_USAGE && !CONFIG_USE_SWITCH */

#ifdef CONFIG_TRACING
	SYS_PORT_TRACING_FUNC(k_thread, switched_in);
#endif /* CONFIG_TRACING */
}

শূন্য z_thread_mark_switched_out(শূন্য)
{
#যদি defined(CONFIG_SCHED_THREAD_USAGE) && !defined(CONFIG_USE_SWITCH)
	z_sched_usage_stop();
#endif /*CONFIG_SCHED_THREAD_USAGE && !CONFIG_USE_SWITCH */

#ifdef CONFIG_TRACING
#ifdef CONFIG_THREAD_LOCAL_STORAGE
	/* Dummy thread won't have TLS set up to run arbitrary code */
	যদি (!_current ||
	    (_current->base.thread_state & _THREAD_DUMMY) != 0) {
		ঘূৰাই;
	}
#endif /* CONFIG_THREAD_LOCAL_STORAGE */
	SYS_PORT_TRACING_FUNC(k_thread, switched_out);
#endif /* CONFIG_TRACING */
}
#endif /* CONFIG_INSTRUMENT_THREAD_SWITCHING */

পূর্ণাঙ্ক k_thread_runtime_stats_get(k_tid_t thread,
			       k_thread_runtime_stats_t *stats)
{
	যদি ((thread == রিক্ত) || (stats == রিক্ত)) {
		ঘূৰাই -EINVAL;
	}

#ifdef CONFIG_SCHED_THREAD_USAGE
	z_sched_thread_usage(thread, stats);
#অন্যথা
	*stats = (k_thread_runtime_stats_t) {};
#endif /* CONFIG_SCHED_THREAD_USAGE */

	ঘূৰাই 0;
}

পূর্ণাঙ্ক k_thread_runtime_stats_all_get(k_thread_runtime_stats_t *stats)
{
#ifdef CONFIG_SCHED_THREAD_USAGE_ALL
	k_thread_runtime_stats_t  tmp_stats;
#endif /* CONFIG_SCHED_THREAD_USAGE_ALL */

	যদি (stats == রিক্ত) {
		ঘূৰাই -EINVAL;
	}

	*stats = (k_thread_runtime_stats_t) {};

#ifdef CONFIG_SCHED_THREAD_USAGE_ALL
	/* Retrieve the usage stats for each core and amalgamate them. */

	unsigned পূর্ণাঙ্ক অঙ্ক_cpus = arch_অঙ্ক_cpus();

	বাবে (unsigned পূর্ণাঙ্ক i = 0; i < অঙ্ক_cpus; i++) {
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

	ঘূৰাই 0;
}

পূর্ণাঙ্ক k_thread_runtime_stats_cpu_get(পূর্ণাঙ্ক cpu, k_thread_runtime_stats_t *stats)
{
	যদি (stats == রিক্ত) {
		ঘূৰাই -EINVAL;
	}

	*stats = (k_thread_runtime_stats_t) {};

#ifdef CONFIG_SCHED_THREAD_USAGE_ALL
	CHECKIF((cpu < 0) || ((unsigned পূর্ণাঙ্ক)cpu >= arch_অঙ্ক_cpus())) {
		ঘূৰাই -EINVAL;
	}

	z_sched_cpu_usage(cpu, stats);
#অন্যথা
	ARG_UNUSED(cpu);
#endif

	ঘূৰাই 0;
}

#ifdef CONFIG_THREAD_ABORT_NEED_CLEANUP
/** Pointer to thread which needs to be cleaned up. */
স্থির সংরচনা k_thread *thread_to_cleanup;

/** Spinlock for thread abort cleanup. */
স্থির সংরচনা k_spinlock thread_cleanup_lock;

#ifdef CONFIG_THREAD_স্তূপ_MEM_MAPPED
স্থির শূন্য *thread_cleanup_স্তূপ_addr;
স্থির size_t thread_cleanup_স্তূপ_sz;

স্থির শূন্য unmap_thread_স্তূপ(শূন্য *addr, size_t sz)
{
	যদি (addr != রিক্ত) {
		k_mem_unmap_phys_guard(addr, sz, false);
	}
}
#endif /* CONFIG_THREAD_STACK_MEM_MAPPED */

শূন্য defer_thread_cleanup(সংরচনা k_thread *thread)
{
	/* Note when adding new deferred cleanup steps:
	 * - The thread object may have been overwritten by the time
	 *   the actual cleanup is being done (e.g. thread object
	 *   allocated on a stack). So stash any necessary data here
	 *   that will be used in the actual cleanup steps.
	 */
	thread_to_cleanup = thread;

#ifdef CONFIG_THREAD_স্তূপ_MEM_MAPPED
	/* Note that the permission of the stack should have been
	 * stripped of user thread access due to the thread having
	 * already exited from a memory domain. That is done via
	 * k_thread_abort().
	 */

	/* Stash the address and size so the region can be unmapped
	 * later.
	 */
	thread_cleanup_স্তূপ_addr = thread->স্তূপ_info.mapped.addr;
	thread_cleanup_স্তূপ_sz = thread->স্তূপ_info.mapped.sz;

	/* The stack is now considered un-usable. This should prevent any functions
	 * from looking directly into the mapped stack if they are made to be aware
	 * of memory mapped stacks, e.g., z_stack_space_get().
	 */
	thread->স্তূপ_info.mapped.addr = রিক্ত;
	thread->স্তূপ_info.mapped.sz = 0;
#endif /* CONFIG_THREAD_STACK_MEM_MAPPED */
}

শূন্য do_deferred_thread_cleanup(শূন্য)
{
	/* Note when adding new actual cleanup steps:
	 * - The thread object may have been overwritten when this is
	 *   called. So only data stashed by defer_thread_cleanup() can
	 *   be used here, never anything from the thread object.
	 */

#ifdef CONFIG_THREAD_স্তূপ_MEM_MAPPED
	unmap_thread_স্তূপ(thread_cleanup_স্তূপ_addr, thread_cleanup_স্তূপ_sz);

	thread_cleanup_স্তূপ_addr = রিক্ত;
	thread_cleanup_স্তূপ_sz = 0;
#endif /* CONFIG_THREAD_STACK_MEM_MAPPED */
}

শূন্য do_thread_cleanup(সংরচনা k_thread *thread)
{
	/* This is only for threads which are no longer running and are not
	 * the current thread, so the thread object is still valid here and
	 * the cleanup can be driven from it directly.
	 */

#ifdef CONFIG_THREAD_স্তূপ_MEM_MAPPED
	unmap_thread_স্তূপ(thread->স্তূপ_info.mapped.addr,
			   thread->স্তূপ_info.mapped.sz);

	/* The stack is now unmapped and thus un-usable. Clear the mapping
	 * info so nothing looks into the stack afterwards, and so that
	 * a repeated cleanup of the same thread object is a no-op,
	 * e.g., z_stack_space_get().
	 */
	thread->স্তূপ_info.mapped.addr = রিক্ত;
	thread->স্তূপ_info.mapped.sz = 0;
#অন্যথা /* CONFIG_THREAD_STACK_MEM_MAPPED */
	ARG_UNUSED(thread);
#endif /* CONFIG_THREAD_STACK_MEM_MAPPED */
}

শূন্য k_thread_abort_cleanup(সংরচনা k_thread *thread)
{
	K_SPINLOCK(&thread_cleanup_lock) {
		যদি (thread_to_cleanup != রিক্ত) {
			/* Finish the pending one first. */
			do_deferred_thread_cleanup();
			thread_to_cleanup = রিক্ত;
		}

		যদি (thread == _current) {
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
		} অন্যথা {
			/* Not the current running thread, so we are safe to do
			 * cleanups.
			 */
			do_thread_cleanup(thread);
		}
	}
}

শূন্য k_thread_abort_cleanup_check_reuse(সংরচনা k_thread *thread)
{
	K_SPINLOCK(&thread_cleanup_lock) {
		/* This is to guard reuse of the same thread object and make sure
		 * any pending cleanups of it needs to be finished before the thread
		 * object can be reused.
		 */
		যদি (thread_to_cleanup == thread) {
			do_deferred_thread_cleanup();
			thread_to_cleanup = রিক্ত;
		}
	}
}

#endif /* CONFIG_THREAD_ABORT_NEED_CLEANUP */

শূন্য z_dummy_thread_init(সংরচনা k_thread *dummy_thread)
{
	dummy_thread->base.thread_state = _THREAD_DUMMY;
#ifdef CONFIG_SCHED_CPU_MASK
	dummy_thread->base.cpu_mask = -1;
#endif /* CONFIG_SCHED_CPU_MASK */
	dummy_thread->base.user_options = K_ESSENTIAL;
#যদি Z_MUTEX_PI_ENABLED
	sys_slist_init(&dummy_thread->held_mutexes);
	dummy_thread->mutex_pended_on = রিক্ত;
	dummy_thread->orig_prio = dummy_thread->base.prio;
#endif /* Z_MUTEX_PI_ENABLED */
#ifdef CONFIG_THREAD_স্তূপ_INFO
	dummy_thread->স্তূপ_info.start = 0U;
	dummy_thread->স্তূপ_info.size = 0U;
#endif /* CONFIG_THREAD_STACK_INFO */
#ifdef CONFIG_USERSPACE
	dummy_thread->mem_domain_info.mem_domain = &k_mem_domain_default;
#endif /* CONFIG_USERSPACE */
#যদি (K_HEAP_MEM_POOL_SIZE > 0)
	k_thread_system_pool_assign(dummy_thread);
#অন্যথা
	dummy_thread->resource_pool = রিক্ত;
#endif /* K_HEAP_MEM_POOL_SIZE */

#ifdef CONFIG_USE_SWITCH
	dummy_thread->switch_handle = রিক্ত;
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

শূন্য z_impl_k_thread_priority_set(k_tid_t thread, পূর্ণাঙ্ক prio)
{
	/*
	 * Use NULL, since we cannot know what the entry point is (we do not
	 * keep track of it) and idle cannot change its priority.
	 */
	Z_ASSERT_VALID_PRIO(prio, রিক্ত);

	bool need_sched = z_thread_prio_set((সংরচনা k_thread *)thread, prio);

	যদি ((need_sched) && (IS_ENABLED(CONFIG_SMP) ||
			     (_current->base.sched_locked == 0U))) {
		z_reschedule_unlocked();
	}
}

#ifdef CONFIG_USERSPACE
স্থির অন্তঃস্থ শূন্য z_vrfy_k_thread_priority_set(k_tid_t thread, পূর্ণাঙ্ক prio)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	K_OOPS(K_SYSCALL_VERIFY_বাৰ্তা(_is_valid_prio(prio, রিক্ত),
				    "invalid thread priority %d", prio));
#ifndef CONFIG_USERSPACE_THREAD_MAY_RAISE_PRIORITY
	K_OOPS(K_SYSCALL_VERIFY_বাৰ্তা((int8_t)prio >= thread->base.prio,
				    "thread priority may only be downgraded (%d < %d)",
				    prio, thread->base.prio));
#endif /* CONFIG_USERSPACE_THREAD_MAY_RAISE_PRIORITY */
	z_impl_k_thread_priority_set(thread, prio);
}
#অন্তর্ভাৱ <zephyr/syscalls/k_thread_priority_set_mrsh.c>
#endif /* CONFIG_USERSPACE */

শূন্য z_impl_k_thread_suspend(k_tid_t thread)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_thread, suspend, thread);

	/* Special case "suspend the current thread" as it doesn't
	 * need the async complexity below.
	 */
	যদি (!IS_ENABLED(CONFIG_SMP) &&
	    likely((thread == _current) && !arch_is_in_isr())) {
		z_thread_suspend_current(thread);
		ঘূৰাই;
	}

	k_spinlock_চাবি_t চাবি = z_sched_spinlock_lock();

	যদি (unlikely(z_is_thread_suspended(thread))) {

		/* The target thread is already suspended. Nothing to do. */

		z_sched_spinlock_unlock(চাবি);
		ঘূৰাই;
	}

	z_thread_halt(thread, চাবি, false);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_thread, suspend, thread);
}

#ifdef CONFIG_USERSPACE
স্থির অন্তঃস্থ শূন্য z_vrfy_k_thread_suspend(k_tid_t thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	z_impl_k_thread_suspend(thread);
}
#অন্তর্ভাৱ <zephyr/syscalls/k_thread_suspend_mrsh.c>
#endif /* CONFIG_USERSPACE */

শূন্য z_impl_k_thread_resume(k_tid_t thread)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_thread, resume, thread);

	k_spinlock_চাবি_t চাবি = z_sched_spinlock_lock();

	/* Do not try to resume a thread that was not suspended */
	যদি (unlikely(!z_is_thread_suspended(thread))) {
		z_sched_spinlock_unlock(চাবি);
		ঘূৰাই;
	}

	z_mark_thread_as_not_suspended(thread);
	z_sched_ready_locked(thread);

	z_reschedule_locked(চাবি);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_thread, resume, thread);
}

#ifdef CONFIG_USERSPACE
স্থির অন্তঃস্থ শূন্য z_vrfy_k_thread_resume(k_tid_t thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	z_impl_k_thread_resume(thread);
}
#অন্তর্ভাৱ <zephyr/syscalls/k_thread_resume_mrsh.c>
#endif /* CONFIG_USERSPACE */

শূন্য z_impl_k_wakeup(k_tid_t thread)
{
	SYS_PORT_TRACING_OBJ_FUNC(k_thread, wakeup, thread);

	k_spinlock_চাবি_t চাবি = z_sched_spinlock_lock();

	যদি (z_is_thread_sleeping(thread)) {
		(শূন্য)z_try_abort_thread_timeout(thread);
		z_mark_thread_as_not_sleeping(thread);
		z_sched_ready_locked(thread);
		z_reschedule_locked(চাবি);
	} অন্যথা {
		z_sched_spinlock_unlock(চাবি);
	}
}

#ifdef CONFIG_USERSPACE
স্থির অন্তঃস্থ শূন্য z_vrfy_k_wakeup(k_tid_t thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	z_impl_k_wakeup(thread);
}
#অন্তর্ভাৱ <zephyr/syscalls/k_wakeup_mrsh.c>
#endif /* CONFIG_USERSPACE */

শূন্য z_thread_abort(সংরচনা k_thread *thread)
{
	bool essential = z_is_thread_essential(thread);
	k_spinlock_চাবি_t চাবি = z_sched_spinlock_lock();

	যদি (z_is_thread_dead(thread)) {
		z_sched_spinlock_unlock(চাবি);
		ঘূৰাই;
	}

	z_thread_halt(thread, চাবি, true);

	যদি (essential) {
		__ASSERT(!essential, "aborted essential thread %p", thread);
		k_panic();
	}
}

#যদি !defined(CONFIG_ARCH_HAS_THREAD_ABORT)
শূন্য z_impl_k_thread_abort(k_tid_t thread)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_thread, abort, thread);

	z_thread_abort(thread);

	__ASSERT_NO_MSG(z_is_thread_dead(thread));

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_thread, abort, thread);
}
#endif /* !CONFIG_ARCH_HAS_THREAD_ABORT */

পূর্ণাঙ্ক z_impl_k_thread_join(সংরচনা k_thread *thread, k_timeout_t timeout)
{
	k_spinlock_চাবি_t চাবি = z_sched_spinlock_lock();
	পূর্ণাঙ্ক ret;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_thread, join, thread, timeout);

	যদি (z_is_thread_dead(thread)) {
		z_sched_switch_spin(thread);
		ret = 0;
	} অন্যথা যদি (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		ret = -EBUSY;
	} অন্যথা যদি ((thread == _current) ||
		   (thread->base.pended_on == &_current->join_queue)) {
		ret = -EDEADLK;
	} অন্যথা {
		__ASSERT(!arch_is_in_isr(), "cannot join in ISR");
		z_sched_add_to_waitq_locked(_current, &thread->join_queue);
		z_add_thread_timeout(_current, timeout);

		SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_thread, join, thread, timeout);
		ret = z_swap_locked(চাবি);
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_thread, join, thread, timeout, ret);

		ঘূৰাই ret;
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_thread, join, thread, timeout, ret);

	z_sched_spinlock_unlock(চাবি);
	ঘূৰাই ret;
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
স্থির bool thread_obj_validate(সংরচনা k_thread *thread)
{
	সংরচনা k_object *ko = k_object_find(thread);
	পূর্ণাঙ্ক ret = k_object_validate(ko, K_OBJ_THREAD, _OBJ_INIT_TRUE);

	ৱিকল্প (ret) {
	পক্ষ 0:
		ঘূৰাই false;
	পক্ষ -EINVAL:
		ঘূৰাই true;
	স্ৱভাৱ:
#ifdef CONFIG_LOG
		k_object_dump_error(ret, thread, ko, K_OBJ_THREAD);
#endif /* CONFIG_LOG */
		/* ret is a non-zero error code here (the 0 and -EINVAL cases
		 * are handled above), so this branch must always oops. Passing
		 * ret as the "verify" expression would treat the failure code as
		 * success and fall through to CODE_UNREACHABLE; verify ret == 0
		 * so the oops is actually raised.
		 */
		K_OOPS(K_SYSCALL_VERIFY_বাৰ্তা(ret == 0, "access denied"));
	}
	CODE_UNREACHABLE; /* LCOV_EXCL_LINE */
}

স্থির অন্তঃস্থ পূর্ণাঙ্ক z_vrfy_k_thread_join(সংরচনা k_thread *thread,
				       k_timeout_t timeout)
{
	যদি (thread_obj_validate(thread)) {
		ঘূৰাই 0;
	}

	ঘূৰাই z_impl_k_thread_join(thread, timeout);
}
#অন্তর্ভাৱ <zephyr/syscalls/k_thread_join_mrsh.c>

স্থির অন্তঃস্থ শূন্য z_vrfy_k_thread_abort(k_tid_t thread)
{
	যদি (thread_obj_validate(thread)) {
		ঘূৰাই;
	}

	K_OOPS(K_SYSCALL_VERIFY_বাৰ্তা(!z_is_thread_essential(thread),
				    "aborting essential thread %p", thread));

	z_impl_k_thread_abort((সংরচনা k_thread *)thread);
}
#অন্তর্ভাৱ <zephyr/syscalls/k_thread_abort_mrsh.c>
#endif /* CONFIG_USERSPACE */

bool k_can_yield(শূন্য)
{
	ঘূৰাই !(k_is_pre_kernel() || k_is_in_isr() || !arch_cpu_irqs_are_enabled() ||
		 z_is_idle_thread_object(_current));
}

শূন্য z_impl_k_yield(শূন্য)
{
	__ASSERT(!arch_is_in_isr(), "");

	SYS_PORT_TRACING_FUNC(k_thread, yield);

	z_sched_yield();
}

#ifdef CONFIG_USERSPACE
স্থির অন্তঃস্থ শূন্য z_vrfy_k_yield(শূন্য)
{
	z_impl_k_yield();
}
#অন্তর্ভাৱ <zephyr/syscalls/k_yield_mrsh.c>
#endif /* CONFIG_USERSPACE */
