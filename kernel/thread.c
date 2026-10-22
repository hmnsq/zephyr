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

#ଅନ୍ତର୍ଭାଵ <zephyr/kernel.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/spinlock.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/sys/math_extras.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/sys/clock.h>
#ଅନ୍ତର୍ଭାଵ <ksched.h>
#ଅନ୍ତର୍ଭାଵ <kthread.h>
#ଅନ୍ତର୍ଭାଵ <wait_q.h>
#ଅନ୍ତର୍ଭାଵ <timeout_q.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/internal/syscall_handler.h>
#ଅନ୍ତର୍ଭାଵ <kernel_internal.h>
#ଅନ୍ତର୍ଭାଵ <kswap.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/init.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/tracing/tracing.h>
#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ <stdbool.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/sys/check.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/random/random.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/sys/atomic.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/logging/log.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/llext/symbol.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/sys/iterable_sections.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/sys/minmax.h>

#ଅନ୍ତର୍ଭାଵ <usage.h>

LOG_MODULE_DECLARE(os, CONFIG_KERNEL_LOG_LEVEL);

#ifdef CONFIG_OBJ_CORE_THREAD
#ifdef CONFIG_OBJ_CORE_STATS_THREAD
ସ୍ଥିର ସଂରଚନା k_obj_core_stats_desc  thread_stats_desc = {
	.raw_size = ଆକାର(ସଂରଚନା k_cycle_stats),
	.query_size = ଆକାର(ସଂରଚନା k_thread_runtime_stats),
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
#ଅନ୍ୟଥା
K_OBJ_TYPE_DEFINE_TYPE_ONLY(obj_type_thread, k_thread, K_OBJ_TYPE_THREAD_ID,
			    ଖାଲି);
#endif /* CONFIG_OBJ_CORE_STATS_THREAD */
#endif /* CONFIG_OBJ_CORE_THREAD */


#define _FOREACH_STATIC_THREAD(thread_data)              \
	STRUCT_SECTION_FOREACH(_static_thread_data, thread_data)

bool k_is_in_isr(ଶୂନ୍ୟ)
{
	ଫେରନ୍ତୁ arch_is_in_isr();
}
EXPORT_ପ୍ରତୀକ(k_is_in_isr);

#ifdef CONFIG_THREAD_CUSTOM_DATA
ଶୂନ୍ୟ z_impl_k_thread_custom_data_set(ଶୂନ୍ୟ *ମୂଲ୍ୟ)
{
	_current->custom_data = ମୂଲ୍ୟ;
}

#ifdef CONFIG_USERSPACE
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ z_vrfy_k_thread_custom_data_set(ଶୂନ୍ୟ *data)
{
	z_impl_k_thread_custom_data_set(data);
}
#ଅନ୍ତର୍ଭାଵ <zephyr/syscalls/k_thread_custom_data_set_mrsh.c>
#endif /* CONFIG_USERSPACE */

ଶୂନ୍ୟ *z_impl_k_thread_custom_data_get(ଶୂନ୍ୟ)
{
	ଫେରନ୍ତୁ _current->custom_data;
}

#ifdef CONFIG_USERSPACE
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ *z_vrfy_k_thread_custom_data_get(ଶୂନ୍ୟ)
{
	ଫେରନ୍ତୁ z_impl_k_thread_custom_data_get();
}
#ଅନ୍ତର୍ଭାଵ <zephyr/syscalls/k_thread_custom_data_get_mrsh.c>

#endif /* CONFIG_USERSPACE */
#endif /* CONFIG_THREAD_CUSTOM_DATA */

ପୂର୍ଣ୍ଣସଂଖ୍ୟା z_impl_k_is_preempt_thread(ଶୂନ୍ୟ)
{
	ଫେରନ୍ତୁ !arch_is_in_isr() && thread_is_preemptible(_current);
}

#ifdef CONFIG_USERSPACE
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ପୂର୍ଣ୍ଣସଂଖ୍ୟା z_vrfy_k_is_preempt_thread(ଶୂନ୍ୟ)
{
	ଫେରନ୍ତୁ z_impl_k_is_preempt_thread();
}
#ଅନ୍ତର୍ଭାଵ <zephyr/syscalls/k_is_preempt_thread_mrsh.c>
#endif /* CONFIG_USERSPACE */

ପୂର୍ଣ୍ଣସଂଖ୍ୟା z_impl_k_thread_priority_get(k_tid_t thread)
{
	ଫେରନ୍ତୁ thread->base.prio;
}

#ifdef CONFIG_USERSPACE
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ପୂର୍ଣ୍ଣସଂଖ୍ୟା z_vrfy_k_thread_priority_get(k_tid_t thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	ଫେରନ୍ତୁ z_impl_k_thread_priority_get(thread);
}
#ଅନ୍ତର୍ଭାଵ <zephyr/syscalls/k_thread_priority_get_mrsh.c>
#endif /* CONFIG_USERSPACE */

#ifdef CONFIG_THREAD_NAME
ସ୍ଥିର ଶୂନ୍ୟ set_thread_name(ସଂରଚନା k_thread *thread, ଅଚଳ ଅକ୍ଷର *str)
{
	ଯଦି (str != ଖାଲି) {
		strncpy(thread->name, str, CONFIG_THREAD_MAX_NAME_LEN - 1);
		/* Ensure NULL termination, truncate if longer */
		thread->name[CONFIG_THREAD_MAX_NAME_LEN - 1] = '\0';
#ifdef CONFIG_ARCH_HAS_THREAD_NAME_HOOK
		arch_thread_name_set(thread, str);
#endif /* CONFIG_ARCH_HAS_THREAD_NAME_HOOK */
	} ଅନ୍ୟଥା {
		thread->name[0] = '\0';
	}
}
#endif /* CONFIG_THREAD_NAME */

ପୂର୍ଣ୍ଣସଂଖ୍ୟା z_impl_k_thread_name_set(k_tid_t thread, ଅଚଳ ଅକ୍ଷର *str)
{
#ifdef CONFIG_THREAD_NAME
	ଯଦି (thread == ଖାଲି) {
		thread = _current;
	}

	set_thread_name(thread, str);

	SYS_PORT_TRACING_OBJ_FUNC(k_thread, name_set, thread, 0);

	ଫେରନ୍ତୁ 0;
#ଅନ୍ୟଥା
	ARG_UNUSED(thread);
	ARG_UNUSED(str);

	SYS_PORT_TRACING_OBJ_FUNC(k_thread, name_set, thread, -ENOSYS);

	ଫେରନ୍ତୁ -ENOSYS;
#endif /* CONFIG_THREAD_NAME */
}

#ifdef CONFIG_USERSPACE
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ପୂର୍ଣ୍ଣସଂଖ୍ୟା z_vrfy_k_thread_name_set(k_tid_t thread, ଅଚଳ ଅକ୍ଷର *str)
{
#ifdef CONFIG_THREAD_NAME
	ଅକ୍ଷର name[CONFIG_THREAD_MAX_NAME_LEN];

	ଯଦି (thread != ଖାଲି) {
		ଯଦି (K_SYSCALL_OBJ(thread, K_OBJ_THREAD) != 0) {
			ଫେରନ୍ତୁ -EINVAL;
		}
	}

	/* In theory we could copy directly into thread->name, but
	 * the current z_vrfy / z_impl split does not provide a
	 * means of doing so.
	 */
	ଯଦି (k_usermode_string_copy(name, str, ଆକାର(name)) != 0) {
		ଫେରନ୍ତୁ -EFAULT;
	}

	ଫେରନ୍ତୁ z_impl_k_thread_name_set(thread, name);
#ଅନ୍ୟଥା
	ଫେରନ୍ତୁ -ENOSYS;
#endif /* CONFIG_THREAD_NAME */
}
#ଅନ୍ତର୍ଭାଵ <zephyr/syscalls/k_thread_name_set_mrsh.c>
#endif /* CONFIG_USERSPACE */

ଅଚଳ ଅକ୍ଷର *k_thread_name_get(k_tid_t thread)
{
#ifdef CONFIG_THREAD_NAME
	ଫେରନ୍ତୁ (ଅଚଳ ଅକ୍ଷର *)thread->name;
#ଅନ୍ୟଥା
	ARG_UNUSED(thread);
	ଫେରନ୍ତୁ ଖାଲି;
#endif /* CONFIG_THREAD_NAME */
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା z_impl_k_thread_name_copy(k_tid_t thread, ଅକ୍ଷର *buf, size_t size)
{
#ifdef CONFIG_THREAD_NAME
	strncpy(buf, thread->name, size);
	ଫେରନ୍ତୁ 0;
#ଅନ୍ୟଥା
	ARG_UNUSED(thread);
	ARG_UNUSED(buf);
	ARG_UNUSED(size);
	ଫେରନ୍ତୁ -ENOSYS;
#endif /* CONFIG_THREAD_NAME */
}

ସ୍ଥିର size_t copy_bytes(ଅକ୍ଷର *dest, size_t dest_size, ଅଚଳ ଅକ୍ଷର *src, size_t src_size)
{
	size_t  bytes_to_copy;

	bytes_to_copy = min(dest_size, src_size);
	memcpy(dest, src, bytes_to_copy);

	ଫେରନ୍ତୁ bytes_to_copy;
}

ଅଚଳ ଅକ୍ଷର *k_thread_state_str(k_tid_t thread_id, ଅକ୍ଷର *buf, size_t buf_size)
{
	size_t      off = 0;
	uint8_t     bit;
	uint8_t     thread_state = thread_id->base.thread_state;
#define SS_ENT(s) { Z_STATE_STR_##s, _THREAD_##s, ଆକାର(Z_STATE_STR_##s) - 1 }
	ସ୍ଥିର ଅଚଳ ସଂରଚନା {
		ଅଚଳ ଅକ୍ଷର *str;
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

	ଯଦି ((buf == ଖାଲି) || (buf_size == 0)) {
		ଫେରନ୍ତୁ "";
	}

	buf_size--;   /* Reserve 1 byte for end-of-string character */

	/*
	 * Loop through each bit in the thread_state. Stop once all have
	 * been processed. If more than one thread_state bit is set, then
	 * separate the descriptive strings with a '+'.
	 */


	ପାଇଁ (unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା index = 0; thread_state != 0; index++) {
		bit = state_string[index].bit;
		ଯଦି ((thread_state & bit) == 0) {
			ଜାରି;
		}

		off += copy_bytes(buf + off, buf_size - off,
				  state_string[index].str,
				  state_string[index].len);

		thread_state &= ~bit;

		ଯଦି (thread_state != 0) {
			off += copy_bytes(buf + off, buf_size - off, "+", 1);
		}
	}

	buf[off] = '\0';

	ଫେରନ୍ତୁ (ଅଚଳ ଅକ୍ଷର *)buf;
}

#ifdef CONFIG_USERSPACE
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ପୂର୍ଣ୍ଣସଂଖ୍ୟା z_vrfy_k_thread_name_copy(k_tid_t thread,
					    ଅକ୍ଷର *buf, size_t size)
{
#ifdef CONFIG_THREAD_NAME
	size_t len;
	ସଂରଚନା k_object *ko = k_object_find(thread);

	/* Special case: we allow reading the names of initialized threads
	 * even if we don't have permission on them
	 */
	ଯଦି ((ko == ଖାଲି) || (ko->type != K_OBJ_THREAD) ||
		((ko->flags & K_OBJ_FLAG_INITIALIZED) == 0)) {
		ଫେରନ୍ତୁ -EINVAL;
	}
	ଯଦି (K_SYSCALL_MEMORY_WRITE(buf, size) != 0) {
		ଫେରନ୍ତୁ -EFAULT;
	}
	len = strlen(thread->name);
	ଯଦି ((len + 1) > size) {
		ଫେରନ୍ତୁ -ENOSPC;
	}

	ଫେରନ୍ତୁ k_usermode_to_copy((ଶୂନ୍ୟ *)buf, thread->name, len + 1);
#ଅନ୍ୟଥା
	ARG_UNUSED(thread);
	ARG_UNUSED(buf);
	ARG_UNUSED(size);
	ଫେରନ୍ତୁ -ENOSYS;
#endif /* CONFIG_THREAD_NAME */
}
#ଅନ୍ତର୍ଭାଵ <zephyr/syscalls/k_thread_name_copy_mrsh.c>
#endif /* CONFIG_USERSPACE */

#ifdef CONFIG_ଗଦା_SENTINEL
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
ଶୂନ୍ୟ z_check_ଗଦା_sentinel(ଶୂନ୍ୟ)
{
	uint32_t *ଗଦା;

	ଯଦି ((_current->base.thread_state & _THREAD_DUMMY) != 0) {
		ଫେରନ୍ତୁ;
	}

	ଗଦା = (uint32_t *)_current->ଗଦା_info.start;
	ଯଦି (*ଗଦା != ଗଦା_SENTINEL) {
		/* Restore it so further checks don't trigger this same error */
		*ଗଦା = ଗଦା_SENTINEL;
		z_except_reason(K_ERR_ଗଦା_CHK_FAIL);
	}
}
#endif /* CONFIG_STACK_SENTINEL */

#ଯଦି defined(CONFIG_ଗଦା_POINTER_RANDOM) && (CONFIG_ଗଦା_POINTER_RANDOM != 0)
ପୂର୍ଣ୍ଣସଂଖ୍ୟା z_ଗଦା_adjust_initialized;

ସ୍ଥିର size_t random_offset(size_t ଗଦା_size)
{
	size_t random_val;

	ଯଦି (!z_ଗଦା_adjust_initialized) {
		z_early_rand_get((uint8_t *)&random_val, ଆକାର(random_val));
	} ଅନ୍ୟଥା {
		sys_rand_get((uint8_t *)&random_val, ଆକାର(random_val));
	}

	/* Don't need to worry about alignment of the size here,
	 * arch_new_thread() is required to do it.
	 *
	 * FIXME: Not the best way to get a random number in a range.
	 * See #6493
	 */
	ଅଚଳ size_t fuzz = random_val % CONFIG_ଗଦା_POINTER_RANDOM;

	ଯଦି (unlikely(fuzz * 2 > ଗଦା_size)) {
		ଫେରନ୍ତୁ 0;
	}

	ଫେରନ୍ତୁ fuzz;
}
#ଯଦି defined(CONFIG_ଗଦା_GROWS_UP)
	/* This is so rare not bothering for now */
#error "Stack pointer randomization not implemented for upward growing stacks"
#endif /* CONFIG_STACK_GROWS_UP */
#endif /* CONFIG_STACK_POINTER_RANDOM */

/*
 * Geometry of a thread's stack object, computed once and threaded through the
 * setup_thread_stack() helpers below.
 */
ସଂରଚନା ଗଦା_geometry {
	size_t obj_size;	/* size of the whole stack object */
	ଅକ୍ଷର *buf_start;	/* start of the usable stack buffer */
	size_t buf_size;	/* size of the usable stack buffer */
};

/*
 * Derive the stack object geometry from the caller-supplied stack and size.
 * User-capable stacks and kernel-only stacks reserve different amounts.
 */
ସ୍ଥିର ସଂରଚନା ଗଦା_geometry compute_ଗଦା_geometry(k_thread_ଗଦା_t *ଗଦା,
						    size_t ଗଦା_size)
{
	ସଂରଚନା ଗଦା_geometry geo;

#ifdef CONFIG_USERSPACE
	ଯଦି (z_ଗଦା_is_user_capable(ଗଦା)) {
		geo.obj_size = K_THREAD_ଗଦା_LEN(ଗଦା_size);
		geo.buf_start = K_THREAD_ଗଦା_BUFFER(ଗଦା);
		geo.buf_size = geo.obj_size - K_THREAD_ଗଦା_RESERVED;
	} ଅନ୍ୟଥା
#endif /* CONFIG_USERSPACE */
	{
		/* Object cannot host a user mode thread */
		geo.obj_size = K_KERNEL_ଗଦା_LEN(ଗଦା_size);
		geo.buf_start = K_KERNEL_ଗଦା_BUFFER(ଗଦା);
		geo.buf_size = geo.obj_size - K_KERNEL_ଗଦା_RESERVED;

#ଯଦି defined(ARCH_KERNEL_ଗଦା_RESERVED)
		/* Zephyr treats stack overflow as an app bug.  But
		 * this particular overflow can be seen by static
		 * analysis so needs to be handled somehow.
		 */
		ଯଦି (K_KERNEL_ଗଦା_RESERVED > geo.obj_size) {
			k_panic();
		}
#endif
	}

	ଫେରନ୍ତୁ geo;
}

/*
 * Return the initial (high end) stack pointer. When stacks are memory mapped
 * this maps the object into virtual memory, records the mapping so it can be
 * torn down at thread exit, and rebases buf_start onto the mapped address.
 */
ସ୍ଥିର ଅକ୍ଷର *map_thread_ଗଦା(ସଂରଚନା k_thread *new_thread,
			      k_thread_ଗଦା_t *ଗଦା,
			      ସଂରଚନା ଗଦା_geometry *geo)
{
	ଅକ୍ଷର *ଗଦା_ptr;

#ifdef CONFIG_THREAD_ଗଦା_MEM_MAPPED
	/* Map the stack into virtual memory and use that as the base to
	 * calculate the initial stack pointer at the high end of the stack
	 * object. The stack pointer may be reduced later by TLS or random
	 * offset.
	 *
	 * K_MEM_MAP_UNINIT is used to mimic the behavior of non-mapped
	 * stack. If CONFIG_INIT_STACKS is enabled, the stack will be
	 * cleared later.
	 */
	ଶୂନ୍ୟ *ଗଦା_mapped = k_mem_map_phys_guard((uintptr_t)ଗଦା, geo->obj_size,
				K_MEM_PERM_RW | K_MEM_CACHE_WB | K_MEM_MAP_UNINIT,
				false);

	__ASSERT_NO_MSG((uintptr_t)ଗଦା_mapped != 0);

#ifdef CONFIG_USERSPACE
	ଯଦି (z_ଗଦା_is_user_capable(ଗଦା)) {
		geo->buf_start = K_THREAD_ଗଦା_BUFFER(ଗଦା_mapped);
	} ଅନ୍ୟଥା
#endif /* CONFIG_USERSPACE */
	{
		geo->buf_start = K_KERNEL_ଗଦା_BUFFER(ଗଦା_mapped);
	}

	ଗଦା_ptr = (ଅକ୍ଷର *)ଗଦା_mapped + geo->obj_size;

	/* Need to store the info on mapped stack so we can remove the mappings
	 * when the thread ends.
	 */
	new_thread->ଗଦା_info.mapped.addr = ଗଦା_mapped;
	new_thread->ଗଦା_info.mapped.sz = geo->obj_size;
#ଅନ୍ୟଥା /* CONFIG_THREAD_STACK_MEM_MAPPED */
	ARG_UNUSED(new_thread);

	/* Initial stack pointer at the high end of the stack object, may
	 * be reduced later by TLS or random offset
	 */
	ଗଦା_ptr = (ଅକ୍ଷର *)ଗଦା + geo->obj_size;
#endif /* CONFIG_THREAD_STACK_MEM_MAPPED */

	ଫେରନ୍ତୁ ଗଦା_ptr;
}

/* Pre-fill the stack buffer with a known pattern for usage measurement. */
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ fill_init_ଗଦା(ଅକ୍ଷର *buf_start, size_t buf_size)
{
#ifdef CONFIG_INIT_STACKS
	memset(buf_start, 0xaa, buf_size);
#ଅନ୍ୟଥା
	ARG_UNUSED(buf_start);
	ARG_UNUSED(buf_size);
#endif /* CONFIG_INIT_STACKS */
}

/* Place the overflow-detection sentinel at the low end of the stack buffer. */
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ set_ଗଦା_sentinel(ଅକ୍ଷର *buf_start)
{
#ifdef CONFIG_ଗଦା_SENTINEL
	/* Put the stack sentinel at the lowest 4 bytes of the stack area.
	 * We periodically check that it's still present and kill the thread
	 * if it isn't.
	 */
	*((uint32_t *)buf_start) = ଗଦା_SENTINEL;
#ଅନ୍ୟଥା
	ARG_UNUSED(buf_start);
#endif /* CONFIG_STACK_SENTINEL */
}

/*
 * Compute how much to reserve at the top of the stack buffer for TLS,
 * userspace-local data and stack-pointer randomization, and return the
 * aligned amount the initial stack pointer must be lowered by.
 */
ସ୍ଥିର size_t reserve_ଗଦା_headroom(ସଂରଚନା k_thread *new_thread,
				     ଅକ୍ଷର *ଗଦା_ptr, size_t buf_size)
{
	size_t delta = 0;

	ARG_UNUSED(new_thread);
	ARG_UNUSED(ଗଦା_ptr);
	ARG_UNUSED(buf_size);

#ifdef CONFIG_THREAD_LOCAL_STORAGE
	/* TLS is always last within the stack buffer */
	delta += arch_tls_ଗଦା_setup(new_thread, ଗଦା_ptr);
#endif /* CONFIG_THREAD_LOCAL_STORAGE */
#ifdef CONFIG_THREAD_USERSPACE_LOCAL_DATA
	size_t tls_size = ଆକାର(ସଂରଚନା _thread_userspace_local_data);

	/* reserve space on highest memory of stack buffer for local data */
	delta += tls_size;
	new_thread->userspace_local_data =
		(ସଂରଚନା _thread_userspace_local_data *)(ଗଦା_ptr - delta);
#endif /* CONFIG_THREAD_USERSPACE_LOCAL_DATA */
#ଯଦି defined(CONFIG_ଗଦା_POINTER_RANDOM) && (CONFIG_ଗଦା_POINTER_RANDOM != 0)
	delta += random_offset(buf_size);
#endif /* CONFIG_STACK_POINTER_RANDOM */

	ଫେରନ୍ତୁ ROUND_UP(delta, ARCH_ଗଦା_PTR_ALIGN);
}

/* Record the thread-accessible stack bounds (and runtime-safety threshold). */
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ set_ଗଦା_info(ସଂରଚନା k_thread *new_thread, ଅକ୍ଷର *buf_start,
				  size_t buf_size, size_t delta)
{
#ifdef CONFIG_THREAD_ଗଦା_INFO
	/* Initial values. Arches which implement MPU guards that "borrow"
	 * memory from the stack buffer (not tracked in K_THREAD_STACK_RESERVED)
	 * will need to appropriately update this.
	 *
	 * The bounds tracked here correspond to the area of the stack object
	 * that the thread can access, which includes TLS.
	 */
	new_thread->ଗଦା_info.start = (uintptr_t)buf_start;
	new_thread->ଗଦା_info.size = buf_size;
	new_thread->ଗଦା_info.delta = delta;

#ifdef CONFIG_THREAD_RUNTIME_ଗଦା_SAFETY
	new_thread->ଗଦା_info.usage.unused_threshold =
		(CONFIG_THREAD_RUNTIME_ଗଦା_SAFETY_DEFAULT_UNUSED_THRESHOLD_PCT *
		 buf_size) / 100;
#endif
#ଅନ୍ୟଥା
	ARG_UNUSED(new_thread);
	ARG_UNUSED(buf_start);
	ARG_UNUSED(buf_size);
	ARG_UNUSED(delta);
#endif /* CONFIG_THREAD_STACK_INFO */
}

ସ୍ଥିର ଅକ୍ଷର *setup_thread_ଗଦା(ସଂରଚନା k_thread *new_thread,
				k_thread_ଗଦା_t *ଗଦା, size_t ଗଦା_size)
{
	ସଂରଚନା ଗଦା_geometry geo = compute_ଗଦା_geometry(ଗଦା, ଗଦା_size);
	ଅକ୍ଷର *ଗଦା_ptr = map_thread_ଗଦା(new_thread, ଗଦା, &geo);
	size_t delta;

	LOG_DBG("stack %p for thread %p: obj_size=%zu buf_start=%p "
		" buf_size %zu stack_ptr=%p",
		ଗଦା, new_thread, geo.obj_size, (ଶୂନ୍ୟ *)geo.buf_start,
		geo.buf_size, (ଶୂନ୍ୟ *)ଗଦା_ptr);

	fill_init_ଗଦା(geo.buf_start, geo.buf_size);
	set_ଗଦା_sentinel(geo.buf_start);
	delta = reserve_ଗଦା_headroom(new_thread, ଗଦା_ptr, geo.buf_size);
	set_ଗଦା_info(new_thread, geo.buf_start, geo.buf_size, delta);

	ଫେରନ୍ତୁ ଗଦା_ptr - delta;
}

#ifdef CONFIG_HW_SHADOW_ଗଦା
ସ୍ଥିର ଶୂନ୍ୟ setup_shadow_ଗଦା(ସଂରଚନା k_thread *new_thread,
			 k_thread_ଗଦା_t *ଗଦା)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = -ENOENT;

	STRUCT_SECTION_FOREACH(ଗଦା_to_hw_shadow_ଗଦା, stk_to_hw_shstk) {
		ଯଦି (stk_to_hw_shstk->ଗଦା == ଗଦା) {
			ret = k_thread_hw_shadow_ଗଦା_attach(new_thread,
							      stk_to_hw_shstk->shstk_addr,
							      stk_to_hw_shstk->size);
			ଯଦି (ret != 0) {
				LOG_ERR("Could not set thread %p shadow stack %p, got error %d",
					new_thread, stk_to_hw_shstk->shstk_addr, ret);
				k_panic();
			}
			ଭାଙ୍ଗ;
		}
	}

	/* Check if the stack isn't in a stack array, and use the corresponding
	 * shadow stack.
	 */
	ଯଦି (ret != -ENOENT) {
		ଫେରନ୍ତୁ;
	}

	STRUCT_SECTION_FOREACH(ଗଦା_to_hw_shadow_ଗଦା_arr, stk_to_hw_shstk) {
		ଯଦି ((uintptr_t)ଗଦା >= stk_to_hw_shstk->ଗଦା_addr &&
		    (uintptr_t)ଗଦା < stk_to_hw_shstk->ଗଦା_addr +
		    stk_to_hw_shstk->ଗଦା_size * stk_to_hw_shstk->nmemb) {
			/* Now we have to guess which index of the stack array is being used */
			uintptr_t ଗଦା_offset = (uintptr_t)ଗଦା - stk_to_hw_shstk->ଗଦା_addr;
			uintptr_t ଗଦା_index = ଗଦା_offset / stk_to_hw_shstk->ଗଦା_size;
			uintptr_t addr;

			ଯଦି (ଗଦା_index >= stk_to_hw_shstk->nmemb) {
				LOG_ERR("Could not find shadow stack for thread %p, stack %p",
					new_thread, ଗଦା);
				k_panic();
			}

			addr = stk_to_hw_shstk->shstk_addr +
				stk_to_hw_shstk->shstk_size * ଗଦା_index;
			ret = k_thread_hw_shadow_ଗଦା_attach(new_thread,
							      (arch_thread_hw_shadow_ଗଦା_t *)addr,
							      stk_to_hw_shstk->shstk_size);
			ଯଦି (ret != 0) {
				LOG_ERR("Could not set thread %p shadow stack 0x%lx, got error %d",
					new_thread, stk_to_hw_shstk->shstk_addr, ret);
				k_panic();
			}
			ଭାଙ୍ଗ;
		}
	}

	ଯଦି (ret == -ENOENT) {
		LOG_ERR("Could not find shadow stack for thread %p, stack %p",
			new_thread, ଗଦା);
		k_panic();
	}
}
#ଅନ୍ୟଥା
/* No-op stub used when hardware shadow stacks are not configured. */
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ setup_shadow_ଗଦା(ସଂରଚନା k_thread *new_thread,
				      k_thread_ଗଦା_t *ଗଦା)
{
	ARG_UNUSED(new_thread);
	ARG_UNUSED(ଗଦା);
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
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ thread_abort_cleanup_check_reuse(ସଂରଚନା k_thread *thread)
{
#ifdef CONFIG_THREAD_ABORT_NEED_CLEANUP
	k_thread_abort_cleanup_check_reuse(thread);
#ଅନ୍ୟଥା
	ARG_UNUSED(thread);
#endif /* CONFIG_THREAD_ABORT_NEED_CLEANUP */
}

/* Register the thread with the object-core framework and its usage stats. */
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ init_thread_obj_core(ସଂରଚନା k_thread *thread)
{
#ifdef CONFIG_OBJ_CORE_THREAD
	k_obj_core_init_and_link(K_OBJ_CORE(thread), &obj_type_thread);
#ifdef CONFIG_OBJ_CORE_STATS_THREAD
	k_obj_core_stats_register(K_OBJ_CORE(thread), &thread->base.usage,
				  ଆକାର(thread->base.usage));
#endif /* CONFIG_OBJ_CORE_STATS_THREAD */
#ଅନ୍ୟଥା
	ARG_UNUSED(thread);
#endif /* CONFIG_OBJ_CORE_THREAD */
}

/*
 * Initialize userspace bookkeeping: register the thread and stack kernel
 * objects, record stack ownership, and grant the thread access to itself.
 */
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ init_thread_userspace(ସଂରଚନା k_thread *thread,
					 k_thread_ଗଦା_t *ଗଦା,
					 uint32_t options)
{
#ifdef CONFIG_USERSPACE
	__ASSERT((options & K_USER) == 0U || z_ଗଦା_is_user_capable(ଗଦା),
		 "user thread %p with kernel-only stack %p", thread, ଗଦା);
	k_object_init(thread);
	k_object_init(ଗଦା);
	thread->ଗଦା_obj = ଗଦା;
	thread->syscall_frame = ଖାଲି;

	/* Any given thread has access to itself */
	k_object_access_grant(thread, thread);
#ଅନ୍ୟଥା
	ARG_UNUSED(thread);
	ARG_UNUSED(ଗଦା);
	ARG_UNUSED(options);
#endif /* CONFIG_USERSPACE */
}

/* Assert the thread object is cache-coherent while its stack stays cached. */
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ assert_thread_coherence(ସଂରଚନା k_thread *thread,
					   k_thread_ଗଦା_t *ଗଦା)
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
	__ASSERT_NO_MSG(!sys_cache_is_mem_coherent(ଗଦା));
#endif /* CONFIG_DYNAMIC_THREAD */
#endif /* CONFIG_KERNEL_COHERENCE */
	ARG_UNUSED(thread);
	ARG_UNUSED(ଗଦା);
}

/* Assert the arch layer populated switch_handle on USE_SWITCH builds. */
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ assert_switch_handle(ସଂରଚନା k_thread *thread)
{
#ifdef CONFIG_USE_SWITCH
	/* switch_handle must be non-null except when inside z_swap()
	 * for synchronization reasons.  Historically some notional
	 * USE_SWITCH architectures have actually ignored the field
	 */
	__ASSERT(thread->switch_handle != ଖାଲି,
		 "arch layer failed to initialize switch_handle");
#endif /* CONFIG_USE_SWITCH */
	ARG_UNUSED(thread);
}

/* Clear the opaque, kernel-managed per-thread custom data pointer. */
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ init_thread_custom_data(ସଂରଚନା k_thread *thread)
{
#ifdef CONFIG_THREAD_CUSTOM_DATA
	/* Initialize custom data field (value is opaque to kernel) */
	thread->custom_data = ଖାଲି;
#ଅନ୍ୟଥା
	ARG_UNUSED(thread);
#endif /* CONFIG_THREAD_CUSTOM_DATA */
}

/*
 * Record the entry point and parameters, then link the thread into the
 * global thread-monitor list under the monitor spinlock.
 */
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ add_thread_to_monitor(ସଂରଚନା k_thread *thread,
					 k_thread_entry_t entry,
					 ଶୂନ୍ୟ *p1, ଶୂନ୍ୟ *p2, ଶୂନ୍ୟ *p3)
{
#ifdef CONFIG_THREAD_MONITOR
	thread->entry.pEntry = entry;
	thread->entry.parameter1 = p1;
	thread->entry.parameter2 = p2;
	thread->entry.parameter3 = p3;

	k_spinlock_ଚାବି_t ଚାବି = k_spin_lock(&z_thread_monitor_lock);

	thread->next_thread = _kernel.threads;
	_kernel.threads = thread;
	k_spin_unlock(&z_thread_monitor_lock, ଚାବି);
#ଅନ୍ୟଥା
	ARG_UNUSED(thread);
	ARG_UNUSED(entry);
	ARG_UNUSED(p1);
	ARG_UNUSED(p2);
	ARG_UNUSED(p3);
#endif /* CONFIG_THREAD_MONITOR */
}

/* Set the thread's name from the caller-supplied string (may be NULL). */
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ init_thread_name(ସଂରଚନା k_thread *thread, ଅଚଳ ଅକ୍ଷର *name)
{
#ifdef CONFIG_THREAD_NAME
	set_thread_name(thread, name);
#ଅନ୍ୟଥା
	ARG_UNUSED(thread);
	ARG_UNUSED(name);
#endif /* CONFIG_THREAD_NAME */
}

/* Set the thread's initial SMP CPU affinity mask. */
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ init_thread_cpu_mask(ସଂରଚନା k_thread *thread)
{
#ifdef CONFIG_SCHED_CPU_MASK
	ଯଦି (IS_ENABLED(CONFIG_SCHED_CPU_MASK_PIN_ONLY)) {
		thread->base.cpu_mask = 1; /* must specify only one cpu */
	} ଅନ୍ୟଥା {
		thread->base.cpu_mask = -1; /* allow all cpus */
	}
#ଅନ୍ୟଥା
	ARG_UNUSED(thread);
#endif /* CONFIG_SCHED_CPU_MASK */
}

/* Attach the thread to its memory domain and inherit caller permissions
 * when K_INHERIT_PERMS is requested.
 */
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ init_thread_userspace_perms(ସଂରଚନା k_thread *thread,
					       uint32_t options)
{
#ifdef CONFIG_USERSPACE
	z_mem_domain_init_thread(thread);

	ଯଦି ((options & K_INHERIT_PERMS) != 0U) {
		k_thread_perms_inherit(_current, thread);
	}
#ଅନ୍ୟଥା
	ARG_UNUSED(thread);
	ARG_UNUSED(options);
#endif /* CONFIG_USERSPACE */
}

/* Clear the EDF scheduling deadline. */
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ init_thread_deadline(ସଂରଚନା k_thread *thread)
{
#ifdef CONFIG_SCHED_DEADLINE
	thread->base.prio_deadline = 0;
#ଅନ୍ୟଥା
	ARG_UNUSED(thread);
#endif /* CONFIG_SCHED_DEADLINE */
}

/* Initialize the wait queue used to synchronize SMP thread halt/abort. */
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ init_thread_halt_queue(ସଂରଚନା k_thread *thread)
{
#ifdef CONFIG_SMP
	z_waitq_init(&thread->halt_queue);
#ଅନ୍ୟଥା
	ARG_UNUSED(thread);
#endif /* CONFIG_SMP */
}

/* Initialize runtime CPU-usage tracking for the thread. */
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ init_thread_usage(ସଂରଚନା k_thread *thread)
{
#ifdef CONFIG_SCHED_THREAD_USAGE
	thread->base.usage = (ସଂରଚନା k_cycle_stats) {};
	thread->base.usage.track_usage = CONFIG_SCHED_THREAD_USAGE_AUTO_ENABLE;
#ଅନ୍ୟଥା
	ARG_UNUSED(thread);
#endif /* CONFIG_SCHED_THREAD_USAGE */
}

/* Assert the thread wasn't already linked into the monitor list. */
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ assert_thread_not_reused(ସଂରଚନା k_thread *new_thread)
{
#ifdef CONFIG_THREAD_MONITOR
#ifdef CONFIG_ASSERT
	k_spinlock_ଚାବି_t ଚାବି = k_spin_lock(&z_thread_monitor_lock);
	bool reused = false;

	/* Check if the thread is already in the list */
	ପାଇଁ (ସଂରଚନା k_thread *t = _kernel.threads; t; t = t->next_thread) {
		ଯଦି (t == new_thread) {
			reused = true;
			ଭାଙ୍ଗ;
		}
	}

	k_spin_unlock(&z_thread_monitor_lock, ଚାବି);

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
ଅକ୍ଷର *z_setup_new_thread(ସଂରଚନା k_thread *new_thread,
			 k_thread_ଗଦା_t *ଗଦା, size_t ଗଦା_size,
			 k_thread_entry_t entry,
			 ଶୂନ୍ୟ *p1, ଶୂନ୍ୟ *p2, ଶୂନ୍ୟ *p3,
			 ପୂର୍ଣ୍ଣସଂଖ୍ୟା prio, uint32_t options, ଅଚଳ ଅକ୍ଷର *name)
{
	ଅକ୍ଷର *ଗଦା_ptr;

	Z_ASSERT_VALID_PRIO(prio, entry);

	assert_thread_not_reused(new_thread);

	thread_abort_cleanup_check_reuse(new_thread);
	init_thread_obj_core(new_thread);
	init_thread_userspace(new_thread, ଗଦା, options);

	z_waitq_init(&new_thread->join_queue);

	/* Initialize various struct k_thread members */
	z_init_thread_base(&new_thread->base, prio, _THREAD_SLEEPING, options);
	ଗଦା_ptr = setup_thread_ଗଦା(new_thread, ଗଦା, ଗଦା_size);

#ଯଦି Z_MUTEX_PI_ENABLED
	sys_slist_init(&new_thread->held_mutexes);
	new_thread->mutex_pended_on = ଖାଲି;
	new_thread->orig_prio = new_thread->base.prio;
#endif /* Z_MUTEX_PI_ENABLED */

	setup_shadow_ଗଦା(new_thread, ଗଦା);
	assert_thread_coherence(new_thread, ଗଦା);

	arch_new_thread(new_thread, ଗଦା, ଗଦା_ptr, entry, p1, p2, p3);

	/* static threads overwrite it afterwards with real value */
	new_thread->init_data = ଖାଲି;

	assert_switch_handle(new_thread);
	init_thread_custom_data(new_thread);
	add_thread_to_monitor(new_thread, entry, p1, p2, p3);
	init_thread_name(new_thread, name);
	init_thread_cpu_mask(new_thread);

	/* _current may be NULL if the dummy thread is not used */
	ଯଦି (IS_ENABLED(CONFIG_ARCH_HAS_CUSTOM_SWAP_TO_MAIN) &&
	    (_current == ଖାଲି)) {
		new_thread->resource_pool = ଖାଲି;
		ଫେରନ୍ତୁ ଗଦା_ptr;
	}

	init_thread_userspace_perms(new_thread, options);
	init_thread_deadline(new_thread);
	new_thread->resource_pool = _current->resource_pool;
	init_thread_halt_queue(new_thread);
	init_thread_usage(new_thread);

	SYS_PORT_TRACING_OBJ_FUNC(k_thread, create, new_thread);

	ଫେରନ୍ତୁ ଗଦା_ptr;
}

#ifdef CONFIG_THREAD_RUNTIME_ଗଦା_SAFETY
ପୂର୍ଣ୍ଣସଂଖ୍ୟା z_impl_k_thread_runtime_ଗଦା_unused_threshold_pct_set(ସଂରଚନା k_thread *thread,
							   uint32_t pct)
{
	size_t unused_threshold;

	ଯଦି (pct > 99) {
		ଫେରନ୍ତୁ -EINVAL;   /* 100% unused stack and up is invalid */
	}

	unused_threshold = (thread->ଗଦା_info.size * pct) / 100;

	thread->ଗଦା_info.usage.unused_threshold = unused_threshold;

	ଫେରନ୍ତୁ 0;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା z_impl_k_thread_runtime_ଗଦା_unused_threshold_set(ସଂରଚନା k_thread *thread,
						       size_t threshold)
{
	ଯଦି (threshold > thread->ଗଦା_info.size) {
		ଫେରନ୍ତୁ -EINVAL;
	}

	thread->ଗଦା_info.usage.unused_threshold = threshold;

	ଫେରନ୍ତୁ 0;
}

size_t z_impl_k_thread_runtime_ଗଦା_unused_threshold_get(ସଂରଚନା k_thread *thread)
{
	ଫେରନ୍ତୁ thread->ଗଦା_info.usage.unused_threshold;
}

#ifdef CONFIG_USERSPACE
ପୂର୍ଣ୍ଣସଂଖ୍ୟା z_vrfy_k_thread_runtime_ଗଦା_unused_threshold_pct_set(ସଂରଚନା k_thread *thread,
							   uint32_t pct)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));

	ଫେରନ୍ତୁ z_impl_k_thread_runtime_ଗଦା_unused_threshold_pct_set(thread, pct);
}
#ଅନ୍ତର୍ଭାଵ <zephyr/syscalls/k_thread_runtime_stack_unused_threshold_pct_set_mrsh.c>

ପୂର୍ଣ୍ଣସଂଖ୍ୟା z_vrfy_k_thread_runtime_ଗଦା_unused_threshold_set(ସଂରଚନା k_thread *thread,
						       size_t threshold)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));

	ଫେରନ୍ତୁ z_impl_k_thread_runtime_ଗଦା_unused_threshold_set(thread, threshold);
}
#ଅନ୍ତର୍ଭାଵ <zephyr/syscalls/k_thread_runtime_stack_unused_threshold_set_mrsh.c>

size_t z_vrfy_k_thread_runtime_ଗଦା_unused_threshold_get(ସଂରଚନା k_thread *thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));

	ଫେରନ୍ତୁ z_impl_k_thread_runtime_ଗଦା_unused_threshold_get(thread);
}
#ଅନ୍ତର୍ଭାଵ <zephyr/syscalls/k_thread_runtime_stack_unused_threshold_get_mrsh.c>
#endif /* CONFIG_USERSPACE */
#endif /* CONFIG_THREAD_RUNTIME_STACK_SAFETY */

k_tid_t z_impl_k_thread_create(ସଂରଚନା k_thread *new_thread,
			      k_thread_ଗଦା_t *ଗଦା,
			      size_t ଗଦା_size, k_thread_entry_t entry,
			      ଶୂନ୍ୟ *p1, ଶୂନ୍ୟ *p2, ଶୂନ୍ୟ *p3,
			      ପୂର୍ଣ୍ଣସଂଖ୍ୟା prio, uint32_t options, k_timeout_t delay)
{
	__ASSERT(!arch_is_in_isr(), "Threads may not be created in ISRs");

	z_setup_new_thread(new_thread, ଗଦା, ଗଦା_size, entry, p1, p2, p3,
			  prio, options, ଖାଲି);

	thread_schedule_new(new_thread, delay);

	ଫେରନ୍ତୁ new_thread;
}

#ifdef CONFIG_USERSPACE
bool z_ଗଦା_is_user_capable(k_thread_ଗଦା_t *ଗଦା)
{
	ଫେରନ୍ତୁ k_object_find(ଗଦା) != ଖାଲି;
}

k_tid_t z_vrfy_k_thread_create(ସଂରଚନା k_thread *new_thread,
			       k_thread_ଗଦା_t *ଗଦା,
			       size_t ଗଦା_size, k_thread_entry_t entry,
			       ଶୂନ୍ୟ *p1, ଶୂନ୍ୟ *p2, ଶୂନ୍ୟ *p3,
			       ପୂର୍ଣ୍ଣସଂଖ୍ୟା prio, uint32_t options, k_timeout_t delay)
{
	size_t total_size, ଗଦା_obj_size;
	ସଂରଚନା k_object *ଗଦା_object;

	/* The thread and stack objects *must* be in an uninitialized state */
	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(new_thread, K_OBJ_THREAD));

	/* No need to check z_stack_is_user_capable(), it won't be in the
	 * object table if it isn't
	 */
	ଗଦା_object = k_object_find(ଗଦା);
	K_OOPS(K_SYSCALL_VERIFY_ସନ୍ଦେଶ(k_object_validation_check(ଗଦା_object, ଗଦା,
						K_OBJ_THREAD_ଗଦା_ELEMENT,
						_OBJ_INIT_FALSE) == 0,
				    "bad stack object"));

	/* Verify that the stack size passed in is OK by computing the total
	 * size and comparing it with the size value in the object metadata
	 */
	K_OOPS(K_SYSCALL_VERIFY_ସନ୍ଦେଶ(!size_add_overflow(K_THREAD_ଗଦା_RESERVED,
						       ଗଦା_size, &total_size),
				    "stack size overflow (%zu+%zu)",
				    ଗଦା_size,
				    K_THREAD_ଗଦା_RESERVED));

	/* Testing less-than-or-equal since additional room may have been
	 * allocated for alignment constraints
	 */
#ifdef CONFIG_GEN_PRIV_STACKS
	ଗଦା_obj_size = ଗଦା_object->data.ଗଦା_data->size;
#ଅନ୍ୟଥା
	ଗଦା_obj_size = ଗଦା_object->data.ଗଦା_size;
#endif /* CONFIG_GEN_PRIV_STACKS */
	K_OOPS(K_SYSCALL_VERIFY_ସନ୍ଦେଶ(total_size <= ଗଦା_obj_size,
				    "stack size %zu is too big, max is %zu",
				    total_size, ଗଦା_obj_size));

	/* User threads may only create other user threads and they can't
	 * be marked as essential
	 */
	K_OOPS(K_SYSCALL_VERIFY(options & K_USER));
	K_OOPS(K_SYSCALL_VERIFY(!(options & K_ESSENTIAL)));

	/* Check validity of prio argument; must be the same or worse priority
	 * than the caller
	 */
	K_OOPS(K_SYSCALL_VERIFY(_is_valid_prio(prio, ଖାଲି)));
	K_OOPS(K_SYSCALL_VERIFY(z_is_prio_lower_or_equal(prio,
							_current->base.prio)));

	z_setup_new_thread(new_thread, ଗଦା, ଗଦା_size,
			   entry, p1, p2, p3, prio, options, ଖାଲି);

	thread_schedule_new(new_thread, delay);

	ଫେରନ୍ତୁ new_thread;
}
#ଅନ୍ତର୍ଭାଵ <zephyr/syscalls/k_thread_create_mrsh.c>
#endif /* CONFIG_USERSPACE */

ଶୂନ୍ୟ z_init_thread_base(ସଂରଚନା _thread_base *thread_base, ପୂର୍ଣ୍ଣସଂଖ୍ୟା priority,
		       uint32_t initial_state, unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା options)
{
	/* k_q_node is initialized upon first insertion in a list */
	thread_base->pended_on = ଖାଲି;
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
	thread_base->slice_expired = ଖାଲି;
#endif /* CONFIG_TIMESLICE_PER_THREAD */

#ifdef CONFIG_SPIN_VALIDATE
	thread_base->swap_data = ଖାଲି;
	/* otherwise swap_data does not need to be initialized */
#endif

	z_init_thread_timeout(thread_base);
}

FUNC_NORETURN ଶୂନ୍ୟ k_thread_user_mode_enter(k_thread_entry_t entry,
					    ଶୂନ୍ୟ *p1, ଶୂନ୍ୟ *p2, ଶୂନ୍ୟ *p3)
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
	__ASSERT(z_ଗଦା_is_user_capable(_current->ଗଦା_obj),
		 "dropping to user mode with kernel-only stack object");
#ifdef CONFIG_THREAD_USERSPACE_LOCAL_DATA
	memset(_current->userspace_local_data, 0,
	       ଆକାର(ସଂରଚନା _thread_userspace_local_data));
#endif /* CONFIG_THREAD_USERSPACE_LOCAL_DATA */
#ifdef CONFIG_THREAD_LOCAL_STORAGE
	arch_tls_ଗଦା_setup(_current,
			     (ଅକ୍ଷର *)(_current->ଗଦା_info.start +
				      _current->ଗଦା_info.size));
#endif /* CONFIG_THREAD_LOCAL_STORAGE */
	arch_user_mode_enter(entry, p1, p2, p3);
#ଅନ୍ୟଥା
	/* XXX In this case we do not reset the stack */
	z_thread_entry(entry, p1, p2, p3);
#endif /* CONFIG_USERSPACE */
}

#ଯଦି defined(CONFIG_INIT_STACKS) && defined(CONFIG_THREAD_ଗଦା_INFO)
#ifdef CONFIG_ଗଦା_GROWS_UP
#error "Unsupported configuration for stack analysis"
#endif /* CONFIG_STACK_GROWS_UP */

ପୂର୍ଣ୍ଣସଂଖ୍ୟା z_ଗଦା_space_get(ଅଚଳ uint8_t *ଗଦା_start, size_t size, size_t *unused_ptr)
{
	size_t unused = 0;
	ଅଚଳ uint8_t *checked_ଗଦା = ଗଦା_start;
	/* Take the address of any local variable as a shallow bound for the
	 * stack pointer.  Addresses above it are guaranteed to be
	 * accessible.
	 */
	ଅଚଳ uint8_t *ଗଦା_pointer = (ଅଚଳ uint8_t *)&ଗଦା_start;

	/* If we are currently running on the stack being analyzed, some
	 * memory management hardware will generate an exception if we
	 * read unused stack memory.
	 *
	 * This never happens when invoked from user mode, as user mode
	 * will always run this function on the privilege elevation stack.
	 */
	ଯଦି ((ଗଦା_pointer > ଗଦା_start) && (ଗଦା_pointer <= (ଗଦା_start + size)) &&
	    IS_ENABLED(CONFIG_NO_UNUSED_ଗଦା_INSPECTION)) {
		/* TODO: We could add an arch_ API call to temporarily
		 * disable the stack checking in the CPU, but this would
		 * need to be properly managed wrt context switches/interrupts
		 */
		ଫେରନ୍ତୁ -ENOTSUP;
	}

	ଯଦି (IS_ENABLED(CONFIG_ଗଦା_SENTINEL)) {
		/* First 4 bytes of the stack buffer reserved for the
		 * sentinel value, it won't be 0xAAAAAAAA for thread
		 * stacks.
		 *
		 * FIXME: thread->stack_info.start ought to reflect
		 * this!
		 */
		checked_ଗଦା += 4;
		size -= 4;
	}

	ପାଇଁ (size_t i = 0; i < size; i++) {
		ଯଦି ((checked_ଗଦା[i]) == 0xaaU) {
			unused++;
		} ଅନ୍ୟଥା {
			ଭାଙ୍ଗ;
		}
	}

	*unused_ptr = unused;

	ଫେରନ୍ତୁ 0;
}

#ifdef CONFIG_THREAD_RUNTIME_ଗଦା_SAFETY
ପୂର୍ଣ୍ଣସଂଖ୍ୟା k_thread_runtime_ଗଦା_safety_full_check(ଅଚଳ ସଂରଚନା k_thread *thread,
					     size_t *unused_ptr,
					     k_thread_ଗଦା_safety_handler_t handler,
					     ଶୂନ୍ୟ *arg)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା    rv;
	size_t unused_space;

	__ASSERT_NO_MSG(thread != ଖାଲି);

	rv = z_ଗଦା_space_get((ଅଚଳ uint8_t *)thread->ଗଦା_info.start,
			       thread->ଗଦା_info.size, &unused_space);

	ଯଦି (rv != 0) {
		ଫେରନ୍ତୁ rv;
	}

	ଯଦି (unused_ptr != ଖାଲି) {
		*unused_ptr = unused_space;
	}

	ଯଦି ((unused_space < thread->ଗଦା_info.usage.unused_threshold) &&
	    (handler != ଖାଲି)) {
		handler(thread, unused_space, arg);
	}

	ଫେରନ୍ତୁ 0;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା k_thread_runtime_ଗଦା_safety_threshold_check(ଅଚଳ ସଂରଚନା k_thread *thread,
						  size_t *unused_ptr,
						  k_thread_ଗଦା_safety_handler_t handler,
						  ଶୂନ୍ୟ *arg)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା    rv;
	size_t unused_space;
	size_t threshold;
	size_t scan_size;

	__ASSERT_NO_MSG(thread != ଖାଲି);

	threshold = thread->ଗଦା_info.usage.unused_threshold;
	scan_size = threshold;

	/* z_stack_space_get() reserves the first bytes of the stack buffer for
	 * the stack sentinel and shrinks its scan window by that amount. Grow
	 * the requested window by the same reservation so that up to
	 * 'threshold' bytes of the usable stack are actually examined;
	 * otherwise a fully unused window would report fewer than 'threshold'
	 * unused bytes and spuriously trip the threshold.
	 */
	ଯଦି (IS_ENABLED(CONFIG_ଗଦା_SENTINEL) && (threshold != 0U)) {
		scan_size += ଆକାର(uint32_t);
	}

	rv = z_ଗଦା_space_get((ଅଚଳ uint8_t *)thread->ଗଦା_info.start,
			       scan_size, &unused_space);

	ଯଦି (rv != 0) {
		ଫେରନ୍ତୁ rv;
	}

	ଯଦି (unused_ptr != ଖାଲି) {
		*unused_ptr = unused_space;
	}

	ଯଦି ((unused_space < threshold) && (handler != ଖାଲି)) {
		handler(thread, unused_space, arg);
	}

	ଫେରନ୍ତୁ 0;
}
#endif /* CONFIG_THREAD_RUNTIME_STACK_SAFETY */

ପୂର୍ଣ୍ଣସଂଖ୍ୟା z_impl_k_thread_ଗଦା_space_get(ଅଚଳ ସଂରଚନା k_thread *thread,
				    size_t *unused_ptr)
{
#ifdef CONFIG_THREAD_ଗଦା_MEM_MAPPED
	ଯଦି (thread->ଗଦା_info.mapped.addr == ଖାଲି) {
		ଫେରନ୍ତୁ -EINVAL;
	}
#endif /* CONFIG_THREAD_STACK_MEM_MAPPED */

	ଫେରନ୍ତୁ z_ଗଦା_space_get((ଅଚଳ uint8_t *)thread->ଗଦା_info.start,
				 thread->ଗଦା_info.size, unused_ptr);
}

#ifdef CONFIG_USERSPACE
ପୂର୍ଣ୍ଣସଂଖ୍ୟା z_vrfy_k_thread_ଗଦା_space_get(ଅଚଳ ସଂରଚନା k_thread *thread,
				    size_t *unused_ptr)
{
	size_t unused;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;

	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));

	ret = z_impl_k_thread_ଗଦା_space_get(thread, &unused);
	CHECKIF(ret != 0) {
		ଫେରନ୍ତୁ ret;
	}

	ret = k_usermode_to_copy(unused_ptr, &unused, ଆକାର(size_t));
	CHECKIF(ret != 0) {
		ଫେରନ୍ତୁ ret;
	}

	ଫେରନ୍ତୁ 0;
}
#ଅନ୍ତର୍ଭାଵ <zephyr/syscalls/k_thread_stack_space_get_mrsh.c>
#endif /* CONFIG_USERSPACE */
#endif /* CONFIG_INIT_STACKS && CONFIG_THREAD_STACK_INFO */

#ifdef CONFIG_USERSPACE
ସ୍ଥିର ଧାଡ଼ିଭିତରେ k_ticks_t z_vrfy_k_thread_timeout_remaining_ticks(
						    ଅଚଳ ସଂରଚନା k_thread *thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	ଫେରନ୍ତୁ z_impl_k_thread_timeout_remaining_ticks(thread);
}
#ଅନ୍ତର୍ଭାଵ <zephyr/syscalls/k_thread_timeout_remaining_ticks_mrsh.c>

ସ୍ଥିର ଧାଡ଼ିଭିତରେ k_ticks_t z_vrfy_k_thread_timeout_expires_ticks(
						  ଅଚଳ ସଂରଚନା k_thread *thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	ଫେରନ୍ତୁ z_impl_k_thread_timeout_expires_ticks(thread);
}
#ଅନ୍ତର୍ଭାଵ <zephyr/syscalls/k_thread_timeout_expires_ticks_mrsh.c>
#endif /* CONFIG_USERSPACE */

#ifdef CONFIG_INSTRUMENT_THREAD_SWITCHING
ଶୂନ୍ୟ z_thread_mark_switched_in(ଶୂନ୍ୟ)
{
#ଯଦି defined(CONFIG_SCHED_THREAD_USAGE) && !defined(CONFIG_USE_SWITCH)
	z_sched_usage_start(_current);
#endif /* CONFIG_SCHED_THREAD_USAGE && !CONFIG_USE_SWITCH */

#ifdef CONFIG_TRACING
	SYS_PORT_TRACING_FUNC(k_thread, switched_in);
#endif /* CONFIG_TRACING */
}

ଶୂନ୍ୟ z_thread_mark_switched_out(ଶୂନ୍ୟ)
{
#ଯଦି defined(CONFIG_SCHED_THREAD_USAGE) && !defined(CONFIG_USE_SWITCH)
	z_sched_usage_stop();
#endif /*CONFIG_SCHED_THREAD_USAGE && !CONFIG_USE_SWITCH */

#ifdef CONFIG_TRACING
#ifdef CONFIG_THREAD_LOCAL_STORAGE
	/* Dummy thread won't have TLS set up to run arbitrary code */
	ଯଦି (!_current ||
	    (_current->base.thread_state & _THREAD_DUMMY) != 0) {
		ଫେରନ୍ତୁ;
	}
#endif /* CONFIG_THREAD_LOCAL_STORAGE */
	SYS_PORT_TRACING_FUNC(k_thread, switched_out);
#endif /* CONFIG_TRACING */
}
#endif /* CONFIG_INSTRUMENT_THREAD_SWITCHING */

ପୂର୍ଣ୍ଣସଂଖ୍ୟା k_thread_runtime_stats_get(k_tid_t thread,
			       k_thread_runtime_stats_t *stats)
{
	ଯଦି ((thread == ଖାଲି) || (stats == ଖାଲି)) {
		ଫେରନ୍ତୁ -EINVAL;
	}

#ifdef CONFIG_SCHED_THREAD_USAGE
	z_sched_thread_usage(thread, stats);
#ଅନ୍ୟଥା
	*stats = (k_thread_runtime_stats_t) {};
#endif /* CONFIG_SCHED_THREAD_USAGE */

	ଫେରନ୍ତୁ 0;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା k_thread_runtime_stats_all_get(k_thread_runtime_stats_t *stats)
{
#ifdef CONFIG_SCHED_THREAD_USAGE_ALL
	k_thread_runtime_stats_t  tmp_stats;
#endif /* CONFIG_SCHED_THREAD_USAGE_ALL */

	ଯଦି (stats == ଖାଲି) {
		ଫେରନ୍ତୁ -EINVAL;
	}

	*stats = (k_thread_runtime_stats_t) {};

#ifdef CONFIG_SCHED_THREAD_USAGE_ALL
	/* Retrieve the usage stats for each core and amalgamate them. */

	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଅଙ୍କ_cpus = arch_ଅଙ୍କ_cpus();

	ପାଇଁ (unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା i = 0; i < ଅଙ୍କ_cpus; i++) {
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

	ଫେରନ୍ତୁ 0;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା k_thread_runtime_stats_cpu_get(ପୂର୍ଣ୍ଣସଂଖ୍ୟା cpu, k_thread_runtime_stats_t *stats)
{
	ଯଦି (stats == ଖାଲି) {
		ଫେରନ୍ତୁ -EINVAL;
	}

	*stats = (k_thread_runtime_stats_t) {};

#ifdef CONFIG_SCHED_THREAD_USAGE_ALL
	CHECKIF((cpu < 0) || ((unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା)cpu >= arch_ଅଙ୍କ_cpus())) {
		ଫେରନ୍ତୁ -EINVAL;
	}

	z_sched_cpu_usage(cpu, stats);
#ଅନ୍ୟଥା
	ARG_UNUSED(cpu);
#endif

	ଫେରନ୍ତୁ 0;
}

#ifdef CONFIG_THREAD_ABORT_NEED_CLEANUP
/** Pointer to thread which needs to be cleaned up. */
ସ୍ଥିର ସଂରଚନା k_thread *thread_to_cleanup;

/** Spinlock for thread abort cleanup. */
ସ୍ଥିର ସଂରଚନା k_spinlock thread_cleanup_lock;

#ifdef CONFIG_THREAD_ଗଦା_MEM_MAPPED
ସ୍ଥିର ଶୂନ୍ୟ *thread_cleanup_ଗଦା_addr;
ସ୍ଥିର size_t thread_cleanup_ଗଦା_sz;

ସ୍ଥିର ଶୂନ୍ୟ unmap_thread_ଗଦା(ଶୂନ୍ୟ *addr, size_t sz)
{
	ଯଦି (addr != ଖାଲି) {
		k_mem_unmap_phys_guard(addr, sz, false);
	}
}
#endif /* CONFIG_THREAD_STACK_MEM_MAPPED */

ଶୂନ୍ୟ defer_thread_cleanup(ସଂରଚନା k_thread *thread)
{
	/* Note when adding new deferred cleanup steps:
	 * - The thread object may have been overwritten by the time
	 *   the actual cleanup is being done (e.g. thread object
	 *   allocated on a stack). So stash any necessary data here
	 *   that will be used in the actual cleanup steps.
	 */
	thread_to_cleanup = thread;

#ifdef CONFIG_THREAD_ଗଦା_MEM_MAPPED
	/* Note that the permission of the stack should have been
	 * stripped of user thread access due to the thread having
	 * already exited from a memory domain. That is done via
	 * k_thread_abort().
	 */

	/* Stash the address and size so the region can be unmapped
	 * later.
	 */
	thread_cleanup_ଗଦା_addr = thread->ଗଦା_info.mapped.addr;
	thread_cleanup_ଗଦା_sz = thread->ଗଦା_info.mapped.sz;

	/* The stack is now considered un-usable. This should prevent any functions
	 * from looking directly into the mapped stack if they are made to be aware
	 * of memory mapped stacks, e.g., z_stack_space_get().
	 */
	thread->ଗଦା_info.mapped.addr = ଖାଲି;
	thread->ଗଦା_info.mapped.sz = 0;
#endif /* CONFIG_THREAD_STACK_MEM_MAPPED */
}

ଶୂନ୍ୟ do_deferred_thread_cleanup(ଶୂନ୍ୟ)
{
	/* Note when adding new actual cleanup steps:
	 * - The thread object may have been overwritten when this is
	 *   called. So only data stashed by defer_thread_cleanup() can
	 *   be used here, never anything from the thread object.
	 */

#ifdef CONFIG_THREAD_ଗଦା_MEM_MAPPED
	unmap_thread_ଗଦା(thread_cleanup_ଗଦା_addr, thread_cleanup_ଗଦା_sz);

	thread_cleanup_ଗଦା_addr = ଖାଲି;
	thread_cleanup_ଗଦା_sz = 0;
#endif /* CONFIG_THREAD_STACK_MEM_MAPPED */
}

ଶୂନ୍ୟ do_thread_cleanup(ସଂରଚନା k_thread *thread)
{
	/* This is only for threads which are no longer running and are not
	 * the current thread, so the thread object is still valid here and
	 * the cleanup can be driven from it directly.
	 */

#ifdef CONFIG_THREAD_ଗଦା_MEM_MAPPED
	unmap_thread_ଗଦା(thread->ଗଦା_info.mapped.addr,
			   thread->ଗଦା_info.mapped.sz);

	/* The stack is now unmapped and thus un-usable. Clear the mapping
	 * info so nothing looks into the stack afterwards, and so that
	 * a repeated cleanup of the same thread object is a no-op,
	 * e.g., z_stack_space_get().
	 */
	thread->ଗଦା_info.mapped.addr = ଖାଲି;
	thread->ଗଦା_info.mapped.sz = 0;
#ଅନ୍ୟଥା /* CONFIG_THREAD_STACK_MEM_MAPPED */
	ARG_UNUSED(thread);
#endif /* CONFIG_THREAD_STACK_MEM_MAPPED */
}

ଶୂନ୍ୟ k_thread_abort_cleanup(ସଂରଚନା k_thread *thread)
{
	K_SPINLOCK(&thread_cleanup_lock) {
		ଯଦି (thread_to_cleanup != ଖାଲି) {
			/* Finish the pending one first. */
			do_deferred_thread_cleanup();
			thread_to_cleanup = ଖାଲି;
		}

		ଯଦି (thread == _current) {
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
		} ଅନ୍ୟଥା {
			/* Not the current running thread, so we are safe to do
			 * cleanups.
			 */
			do_thread_cleanup(thread);
		}
	}
}

ଶୂନ୍ୟ k_thread_abort_cleanup_check_reuse(ସଂରଚନା k_thread *thread)
{
	K_SPINLOCK(&thread_cleanup_lock) {
		/* This is to guard reuse of the same thread object and make sure
		 * any pending cleanups of it needs to be finished before the thread
		 * object can be reused.
		 */
		ଯଦି (thread_to_cleanup == thread) {
			do_deferred_thread_cleanup();
			thread_to_cleanup = ଖାଲି;
		}
	}
}

#endif /* CONFIG_THREAD_ABORT_NEED_CLEANUP */

ଶୂନ୍ୟ z_dummy_thread_init(ସଂରଚନା k_thread *dummy_thread)
{
	dummy_thread->base.thread_state = _THREAD_DUMMY;
#ifdef CONFIG_SCHED_CPU_MASK
	dummy_thread->base.cpu_mask = -1;
#endif /* CONFIG_SCHED_CPU_MASK */
	dummy_thread->base.user_options = K_ESSENTIAL;
#ଯଦି Z_MUTEX_PI_ENABLED
	sys_slist_init(&dummy_thread->held_mutexes);
	dummy_thread->mutex_pended_on = ଖାଲି;
	dummy_thread->orig_prio = dummy_thread->base.prio;
#endif /* Z_MUTEX_PI_ENABLED */
#ifdef CONFIG_THREAD_ଗଦା_INFO
	dummy_thread->ଗଦା_info.start = 0U;
	dummy_thread->ଗଦା_info.size = 0U;
#endif /* CONFIG_THREAD_STACK_INFO */
#ifdef CONFIG_USERSPACE
	dummy_thread->mem_domain_info.mem_domain = &k_mem_domain_default;
#endif /* CONFIG_USERSPACE */
#ଯଦି (K_HEAP_MEM_POOL_SIZE > 0)
	k_thread_system_pool_assign(dummy_thread);
#ଅନ୍ୟଥା
	dummy_thread->resource_pool = ଖାଲି;
#endif /* K_HEAP_MEM_POOL_SIZE */

#ifdef CONFIG_USE_SWITCH
	dummy_thread->switch_handle = ଖାଲି;
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

ଶୂନ୍ୟ z_impl_k_thread_priority_set(k_tid_t thread, ପୂର୍ଣ୍ଣସଂଖ୍ୟା prio)
{
	/*
	 * Use NULL, since we cannot know what the entry point is (we do not
	 * keep track of it) and idle cannot change its priority.
	 */
	Z_ASSERT_VALID_PRIO(prio, ଖାଲି);

	bool need_sched = z_thread_prio_set((ସଂରଚନା k_thread *)thread, prio);

	ଯଦି ((need_sched) && (IS_ENABLED(CONFIG_SMP) ||
			     (_current->base.sched_locked == 0U))) {
		z_reschedule_unlocked();
	}
}

#ifdef CONFIG_USERSPACE
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ z_vrfy_k_thread_priority_set(k_tid_t thread, ପୂର୍ଣ୍ଣସଂଖ୍ୟା prio)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	K_OOPS(K_SYSCALL_VERIFY_ସନ୍ଦେଶ(_is_valid_prio(prio, ଖାଲି),
				    "invalid thread priority %d", prio));
#ifndef CONFIG_USERSPACE_THREAD_MAY_RAISE_PRIORITY
	K_OOPS(K_SYSCALL_VERIFY_ସନ୍ଦେଶ((int8_t)prio >= thread->base.prio,
				    "thread priority may only be downgraded (%d < %d)",
				    prio, thread->base.prio));
#endif /* CONFIG_USERSPACE_THREAD_MAY_RAISE_PRIORITY */
	z_impl_k_thread_priority_set(thread, prio);
}
#ଅନ୍ତର୍ଭାଵ <zephyr/syscalls/k_thread_priority_set_mrsh.c>
#endif /* CONFIG_USERSPACE */

ଶୂନ୍ୟ z_impl_k_thread_suspend(k_tid_t thread)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_thread, suspend, thread);

	/* Special case "suspend the current thread" as it doesn't
	 * need the async complexity below.
	 */
	ଯଦି (!IS_ENABLED(CONFIG_SMP) &&
	    likely((thread == _current) && !arch_is_in_isr())) {
		z_thread_suspend_current(thread);
		ଫେରନ୍ତୁ;
	}

	k_spinlock_ଚାବି_t ଚାବି = z_sched_spinlock_lock();

	ଯଦି (unlikely(z_is_thread_suspended(thread))) {

		/* The target thread is already suspended. Nothing to do. */

		z_sched_spinlock_unlock(ଚାବି);
		ଫେରନ୍ତୁ;
	}

	z_thread_halt(thread, ଚାବି, false);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_thread, suspend, thread);
}

#ifdef CONFIG_USERSPACE
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ z_vrfy_k_thread_suspend(k_tid_t thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	z_impl_k_thread_suspend(thread);
}
#ଅନ୍ତର୍ଭାଵ <zephyr/syscalls/k_thread_suspend_mrsh.c>
#endif /* CONFIG_USERSPACE */

ଶୂନ୍ୟ z_impl_k_thread_resume(k_tid_t thread)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_thread, resume, thread);

	k_spinlock_ଚାବି_t ଚାବି = z_sched_spinlock_lock();

	/* Do not try to resume a thread that was not suspended */
	ଯଦି (unlikely(!z_is_thread_suspended(thread))) {
		z_sched_spinlock_unlock(ଚାବି);
		ଫେରନ୍ତୁ;
	}

	z_mark_thread_as_not_suspended(thread);
	z_sched_ready_locked(thread);

	z_reschedule_locked(ଚାବି);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_thread, resume, thread);
}

#ifdef CONFIG_USERSPACE
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ z_vrfy_k_thread_resume(k_tid_t thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	z_impl_k_thread_resume(thread);
}
#ଅନ୍ତର୍ଭାଵ <zephyr/syscalls/k_thread_resume_mrsh.c>
#endif /* CONFIG_USERSPACE */

ଶୂନ୍ୟ z_impl_k_wakeup(k_tid_t thread)
{
	SYS_PORT_TRACING_OBJ_FUNC(k_thread, wakeup, thread);

	k_spinlock_ଚାବି_t ଚାବି = z_sched_spinlock_lock();

	ଯଦି (z_is_thread_sleeping(thread)) {
		(ଶୂନ୍ୟ)z_try_abort_thread_timeout(thread);
		z_mark_thread_as_not_sleeping(thread);
		z_sched_ready_locked(thread);
		z_reschedule_locked(ଚାବି);
	} ଅନ୍ୟଥା {
		z_sched_spinlock_unlock(ଚାବି);
	}
}

#ifdef CONFIG_USERSPACE
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ z_vrfy_k_wakeup(k_tid_t thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	z_impl_k_wakeup(thread);
}
#ଅନ୍ତର୍ଭାଵ <zephyr/syscalls/k_wakeup_mrsh.c>
#endif /* CONFIG_USERSPACE */

ଶୂନ୍ୟ z_thread_abort(ସଂରଚନା k_thread *thread)
{
	bool essential = z_is_thread_essential(thread);
	k_spinlock_ଚାବି_t ଚାବି = z_sched_spinlock_lock();

	ଯଦି (z_is_thread_dead(thread)) {
		z_sched_spinlock_unlock(ଚାବି);
		ଫେରନ୍ତୁ;
	}

	z_thread_halt(thread, ଚାବି, true);

	ଯଦି (essential) {
		__ASSERT(!essential, "aborted essential thread %p", thread);
		k_panic();
	}
}

#ଯଦି !defined(CONFIG_ARCH_HAS_THREAD_ABORT)
ଶୂନ୍ୟ z_impl_k_thread_abort(k_tid_t thread)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_thread, abort, thread);

	z_thread_abort(thread);

	__ASSERT_NO_MSG(z_is_thread_dead(thread));

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_thread, abort, thread);
}
#endif /* !CONFIG_ARCH_HAS_THREAD_ABORT */

ପୂର୍ଣ୍ଣସଂଖ୍ୟା z_impl_k_thread_join(ସଂରଚନା k_thread *thread, k_timeout_t timeout)
{
	k_spinlock_ଚାବି_t ଚାବି = z_sched_spinlock_lock();
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_thread, join, thread, timeout);

	ଯଦି (z_is_thread_dead(thread)) {
		z_sched_switch_spin(thread);
		ret = 0;
	} ଅନ୍ୟଥା ଯଦି (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		ret = -EBUSY;
	} ଅନ୍ୟଥା ଯଦି ((thread == _current) ||
		   (thread->base.pended_on == &_current->join_queue)) {
		ret = -EDEADLK;
	} ଅନ୍ୟଥା {
		__ASSERT(!arch_is_in_isr(), "cannot join in ISR");
		z_sched_add_to_waitq_locked(_current, &thread->join_queue);
		z_add_thread_timeout(_current, timeout);

		SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_thread, join, thread, timeout);
		ret = z_swap_locked(ଚାବି);
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_thread, join, thread, timeout, ret);

		ଫେରନ୍ତୁ ret;
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_thread, join, thread, timeout, ret);

	z_sched_spinlock_unlock(ଚାବି);
	ଫେରନ୍ତୁ ret;
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
ସ୍ଥିର bool thread_obj_validate(ସଂରଚନା k_thread *thread)
{
	ସଂରଚନା k_object *ko = k_object_find(thread);
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = k_object_validate(ko, K_OBJ_THREAD, _OBJ_INIT_TRUE);

	ବାଛ (ret) {
	ସ୍ଥିତି 0:
		ଫେରନ୍ତୁ false;
	ସ୍ଥିତି -EINVAL:
		ଫେରନ୍ତୁ true;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
#ifdef CONFIG_LOG
		k_object_dump_error(ret, thread, ko, K_OBJ_THREAD);
#endif /* CONFIG_LOG */
		/* ret is a non-zero error code here (the 0 and -EINVAL cases
		 * are handled above), so this branch must always oops. Passing
		 * ret as the "verify" expression would treat the failure code as
		 * success and fall through to CODE_UNREACHABLE; verify ret == 0
		 * so the oops is actually raised.
		 */
		K_OOPS(K_SYSCALL_VERIFY_ସନ୍ଦେଶ(ret == 0, "access denied"));
	}
	CODE_UNREACHABLE; /* LCOV_EXCL_LINE */
}

ସ୍ଥିର ଧାଡ଼ିଭିତରେ ପୂର୍ଣ୍ଣସଂଖ୍ୟା z_vrfy_k_thread_join(ସଂରଚନା k_thread *thread,
				       k_timeout_t timeout)
{
	ଯଦି (thread_obj_validate(thread)) {
		ଫେରନ୍ତୁ 0;
	}

	ଫେରନ୍ତୁ z_impl_k_thread_join(thread, timeout);
}
#ଅନ୍ତର୍ଭାଵ <zephyr/syscalls/k_thread_join_mrsh.c>

ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ z_vrfy_k_thread_abort(k_tid_t thread)
{
	ଯଦି (thread_obj_validate(thread)) {
		ଫେରନ୍ତୁ;
	}

	K_OOPS(K_SYSCALL_VERIFY_ସନ୍ଦେଶ(!z_is_thread_essential(thread),
				    "aborting essential thread %p", thread));

	z_impl_k_thread_abort((ସଂରଚନା k_thread *)thread);
}
#ଅନ୍ତର୍ଭାଵ <zephyr/syscalls/k_thread_abort_mrsh.c>
#endif /* CONFIG_USERSPACE */

bool k_can_yield(ଶୂନ୍ୟ)
{
	ଫେରନ୍ତୁ !(k_is_pre_kernel() || k_is_in_isr() || !arch_cpu_irqs_are_enabled() ||
		 z_is_idle_thread_object(_current));
}

ଶୂନ୍ୟ z_impl_k_yield(ଶୂନ୍ୟ)
{
	__ASSERT(!arch_is_in_isr(), "");

	SYS_PORT_TRACING_FUNC(k_thread, yield);

	z_sched_yield();
}

#ifdef CONFIG_USERSPACE
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ z_vrfy_k_yield(ଶୂନ୍ୟ)
{
	z_impl_k_yield();
}
#ଅନ୍ତର୍ଭାଵ <zephyr/syscalls/k_yield_mrsh.c>
#endif /* CONFIG_USERSPACE */
