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

#અન્તર્ભાવ <zephyr/kernel.h>
#અન્તર્ભાવ <zephyr/spinlock.h>
#અન્તર્ભાવ <zephyr/sys/math_extras.h>
#અન્તર્ભાવ <zephyr/sys/clock.h>
#અન્તર્ભાવ <ksched.h>
#અન્તર્ભાવ <kthread.h>
#અન્તર્ભાવ <wait_q.h>
#અન્તર્ભાવ <timeout_q.h>
#અન્તર્ભાવ <zephyr/internal/syscall_handler.h>
#અન્તર્ભાવ <kernel_internal.h>
#અન્તર્ભાવ <kswap.h>
#અન્તર્ભાવ <zephyr/init.h>
#અન્તર્ભાવ <zephyr/tracing/tracing.h>
#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ <stdbool.h>
#અન્તર્ભાવ <zephyr/sys/check.h>
#અન્તર્ભાવ <zephyr/random/random.h>
#અન્તર્ભાવ <zephyr/sys/atomic.h>
#અન્તર્ભાવ <zephyr/logging/log.h>
#અન્તર્ભાવ <zephyr/llext/symbol.h>
#અન્તર્ભાવ <zephyr/sys/iterable_sections.h>
#અન્તર્ભાવ <zephyr/sys/minmax.h>

#અન્તર્ભાવ <usage.h>

LOG_MODULE_DECLARE(os, CONFIG_KERNEL_LOG_LEVEL);

#ifdef CONFIG_OBJ_CORE_THREAD
#ifdef CONFIG_OBJ_CORE_STATS_THREAD
સ્થિર સંરચના k_obj_core_stats_desc  thread_stats_desc = {
	.raw_size = કદ(સંરચના k_cycle_stats),
	.query_size = કદ(સંરચના k_thread_runtime_stats),
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
#નહીંતર
K_OBJ_TYPE_DEFINE_TYPE_ONLY(obj_type_thread, k_thread, K_OBJ_TYPE_THREAD_ID,
			    ખાલી);
#endif /* CONFIG_OBJ_CORE_STATS_THREAD */
#endif /* CONFIG_OBJ_CORE_THREAD */


#define _FOREACH_STATIC_THREAD(thread_data)              \
	STRUCT_SECTION_FOREACH(_static_thread_data, thread_data)

bool k_is_in_isr(શૂન્ય)
{
	પાછા arch_is_in_isr();
}
EXPORT_પ્રતીક(k_is_in_isr);

#ifdef CONFIG_THREAD_CUSTOM_DATA
શૂન્ય z_impl_k_thread_custom_data_set(શૂન્ય *કિંમત)
{
	_current->custom_data = કિંમત;
}

#ifdef CONFIG_USERSPACE
સ્થિર અંતઃરેખ શૂન્ય z_vrfy_k_thread_custom_data_set(શૂન્ય *data)
{
	z_impl_k_thread_custom_data_set(data);
}
#અન્તર્ભાવ <zephyr/syscalls/k_thread_custom_data_set_mrsh.c>
#endif /* CONFIG_USERSPACE */

શૂન્ય *z_impl_k_thread_custom_data_get(શૂન્ય)
{
	પાછા _current->custom_data;
}

#ifdef CONFIG_USERSPACE
સ્થિર અંતઃરેખ શૂન્ય *z_vrfy_k_thread_custom_data_get(શૂન્ય)
{
	પાછા z_impl_k_thread_custom_data_get();
}
#અન્તર્ભાવ <zephyr/syscalls/k_thread_custom_data_get_mrsh.c>

#endif /* CONFIG_USERSPACE */
#endif /* CONFIG_THREAD_CUSTOM_DATA */

પૂર્ણાંક z_impl_k_is_preempt_thread(શૂન્ય)
{
	પાછા !arch_is_in_isr() && thread_is_preemptible(_current);
}

#ifdef CONFIG_USERSPACE
સ્થિર અંતઃરેખ પૂર્ણાંક z_vrfy_k_is_preempt_thread(શૂન્ય)
{
	પાછા z_impl_k_is_preempt_thread();
}
#અન્તર્ભાવ <zephyr/syscalls/k_is_preempt_thread_mrsh.c>
#endif /* CONFIG_USERSPACE */

પૂર્ણાંક z_impl_k_thread_priority_get(k_tid_t thread)
{
	પાછા thread->base.prio;
}

#ifdef CONFIG_USERSPACE
સ્થિર અંતઃરેખ પૂર્ણાંક z_vrfy_k_thread_priority_get(k_tid_t thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	પાછા z_impl_k_thread_priority_get(thread);
}
#અન્તર્ભાવ <zephyr/syscalls/k_thread_priority_get_mrsh.c>
#endif /* CONFIG_USERSPACE */

#ifdef CONFIG_THREAD_NAME
સ્થિર શૂન્ય set_thread_name(સંરચના k_thread *thread, અચલ અક્ષર *str)
{
	જો (str != ખાલી) {
		strncpy(thread->name, str, CONFIG_THREAD_MAX_NAME_LEN - 1);
		/* Ensure NULL termination, truncate if longer */
		thread->name[CONFIG_THREAD_MAX_NAME_LEN - 1] = '\0';
#ifdef CONFIG_ARCH_HAS_THREAD_NAME_HOOK
		arch_thread_name_set(thread, str);
#endif /* CONFIG_ARCH_HAS_THREAD_NAME_HOOK */
	} નહીંતર {
		thread->name[0] = '\0';
	}
}
#endif /* CONFIG_THREAD_NAME */

પૂર્ણાંક z_impl_k_thread_name_set(k_tid_t thread, અચલ અક્ષર *str)
{
#ifdef CONFIG_THREAD_NAME
	જો (thread == ખાલી) {
		thread = _current;
	}

	set_thread_name(thread, str);

	SYS_PORT_TRACING_OBJ_FUNC(k_thread, name_set, thread, 0);

	પાછા 0;
#નહીંતર
	ARG_UNUSED(thread);
	ARG_UNUSED(str);

	SYS_PORT_TRACING_OBJ_FUNC(k_thread, name_set, thread, -ENOSYS);

	પાછા -ENOSYS;
#endif /* CONFIG_THREAD_NAME */
}

#ifdef CONFIG_USERSPACE
સ્થિર અંતઃરેખ પૂર્ણાંક z_vrfy_k_thread_name_set(k_tid_t thread, અચલ અક્ષર *str)
{
#ifdef CONFIG_THREAD_NAME
	અક્ષર name[CONFIG_THREAD_MAX_NAME_LEN];

	જો (thread != ખાલી) {
		જો (K_SYSCALL_OBJ(thread, K_OBJ_THREAD) != 0) {
			પાછા -EINVAL;
		}
	}

	/* In theory we could copy directly into thread->name, but
	 * the current z_vrfy / z_impl split does not provide a
	 * means of doing so.
	 */
	જો (k_usermode_string_copy(name, str, કદ(name)) != 0) {
		પાછા -EFAULT;
	}

	પાછા z_impl_k_thread_name_set(thread, name);
#નહીંતર
	પાછા -ENOSYS;
#endif /* CONFIG_THREAD_NAME */
}
#અન્તર્ભાવ <zephyr/syscalls/k_thread_name_set_mrsh.c>
#endif /* CONFIG_USERSPACE */

અચલ અક્ષર *k_thread_name_get(k_tid_t thread)
{
#ifdef CONFIG_THREAD_NAME
	પાછા (અચલ અક્ષર *)thread->name;
#નહીંતર
	ARG_UNUSED(thread);
	પાછા ખાલી;
#endif /* CONFIG_THREAD_NAME */
}

પૂર્ણાંક z_impl_k_thread_name_copy(k_tid_t thread, અક્ષર *buf, size_t size)
{
#ifdef CONFIG_THREAD_NAME
	strncpy(buf, thread->name, size);
	પાછા 0;
#નહીંતર
	ARG_UNUSED(thread);
	ARG_UNUSED(buf);
	ARG_UNUSED(size);
	પાછા -ENOSYS;
#endif /* CONFIG_THREAD_NAME */
}

સ્થિર size_t copy_bytes(અક્ષર *dest, size_t dest_size, અચલ અક્ષર *src, size_t src_size)
{
	size_t  bytes_to_copy;

	bytes_to_copy = min(dest_size, src_size);
	memcpy(dest, src, bytes_to_copy);

	પાછા bytes_to_copy;
}

અચલ અક્ષર *k_thread_state_str(k_tid_t thread_id, અક્ષર *buf, size_t buf_size)
{
	size_t      off = 0;
	uint8_t     bit;
	uint8_t     thread_state = thread_id->base.thread_state;
#define SS_ENT(s) { Z_STATE_STR_##s, _THREAD_##s, કદ(Z_STATE_STR_##s) - 1 }
	સ્થિર અચલ સંરચના {
		અચલ અક્ષર *str;
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

	જો ((buf == ખાલી) || (buf_size == 0)) {
		પાછા "";
	}

	buf_size--;   /* Reserve 1 byte for end-of-string character */

	/*
	 * Loop through each bit in the thread_state. Stop once all have
	 * been processed. If more than one thread_state bit is set, then
	 * separate the descriptive strings with a '+'.
	 */


	માટે (unsigned પૂર્ણાંક index = 0; thread_state != 0; index++) {
		bit = state_string[index].bit;
		જો ((thread_state & bit) == 0) {
			ચાલુ;
		}

		off += copy_bytes(buf + off, buf_size - off,
				  state_string[index].str,
				  state_string[index].len);

		thread_state &= ~bit;

		જો (thread_state != 0) {
			off += copy_bytes(buf + off, buf_size - off, "+", 1);
		}
	}

	buf[off] = '\0';

	પાછા (અચલ અક્ષર *)buf;
}

#ifdef CONFIG_USERSPACE
સ્થિર અંતઃરેખ પૂર્ણાંક z_vrfy_k_thread_name_copy(k_tid_t thread,
					    અક્ષર *buf, size_t size)
{
#ifdef CONFIG_THREAD_NAME
	size_t len;
	સંરચના k_object *ko = k_object_find(thread);

	/* Special case: we allow reading the names of initialized threads
	 * even if we don't have permission on them
	 */
	જો ((ko == ખાલી) || (ko->type != K_OBJ_THREAD) ||
		((ko->flags & K_OBJ_FLAG_INITIALIZED) == 0)) {
		પાછા -EINVAL;
	}
	જો (K_SYSCALL_MEMORY_WRITE(buf, size) != 0) {
		પાછા -EFAULT;
	}
	len = strlen(thread->name);
	જો ((len + 1) > size) {
		પાછા -ENOSPC;
	}

	પાછા k_usermode_to_copy((શૂન્ય *)buf, thread->name, len + 1);
#નહીંતર
	ARG_UNUSED(thread);
	ARG_UNUSED(buf);
	ARG_UNUSED(size);
	પાછા -ENOSYS;
#endif /* CONFIG_THREAD_NAME */
}
#અન્તર્ભાવ <zephyr/syscalls/k_thread_name_copy_mrsh.c>
#endif /* CONFIG_USERSPACE */

#ifdef CONFIG_ઢગલો_SENTINEL
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
શૂન્ય z_check_ઢગલો_sentinel(શૂન્ય)
{
	uint32_t *ઢગલો;

	જો ((_current->base.thread_state & _THREAD_DUMMY) != 0) {
		પાછા;
	}

	ઢગલો = (uint32_t *)_current->ઢગલો_info.start;
	જો (*ઢગલો != ઢગલો_SENTINEL) {
		/* Restore it so further checks don't trigger this same error */
		*ઢગલો = ઢગલો_SENTINEL;
		z_except_reason(K_ERR_ઢગલો_CHK_FAIL);
	}
}
#endif /* CONFIG_STACK_SENTINEL */

#જો defined(CONFIG_ઢગલો_POINTER_RANDOM) && (CONFIG_ઢગલો_POINTER_RANDOM != 0)
પૂર્ણાંક z_ઢગલો_adjust_initialized;

સ્થિર size_t random_offset(size_t ઢગલો_size)
{
	size_t random_val;

	જો (!z_ઢગલો_adjust_initialized) {
		z_early_rand_get((uint8_t *)&random_val, કદ(random_val));
	} નહીંતર {
		sys_rand_get((uint8_t *)&random_val, કદ(random_val));
	}

	/* Don't need to worry about alignment of the size here,
	 * arch_new_thread() is required to do it.
	 *
	 * FIXME: Not the best way to get a random number in a range.
	 * See #6493
	 */
	અચલ size_t fuzz = random_val % CONFIG_ઢગલો_POINTER_RANDOM;

	જો (unlikely(fuzz * 2 > ઢગલો_size)) {
		પાછા 0;
	}

	પાછા fuzz;
}
#જો defined(CONFIG_ઢગલો_GROWS_UP)
	/* This is so rare not bothering for now */
#error "Stack pointer randomization not implemented for upward growing stacks"
#endif /* CONFIG_STACK_GROWS_UP */
#endif /* CONFIG_STACK_POINTER_RANDOM */

/*
 * Geometry of a thread's stack object, computed once and threaded through the
 * setup_thread_stack() helpers below.
 */
સંરચના ઢગલો_geometry {
	size_t obj_size;	/* size of the whole stack object */
	અક્ષર *buf_start;	/* start of the usable stack buffer */
	size_t buf_size;	/* size of the usable stack buffer */
};

/*
 * Derive the stack object geometry from the caller-supplied stack and size.
 * User-capable stacks and kernel-only stacks reserve different amounts.
 */
સ્થિર સંરચના ઢગલો_geometry compute_ઢગલો_geometry(k_thread_ઢગલો_t *ઢગલો,
						    size_t ઢગલો_size)
{
	સંરચના ઢગલો_geometry geo;

#ifdef CONFIG_USERSPACE
	જો (z_ઢગલો_is_user_capable(ઢગલો)) {
		geo.obj_size = K_THREAD_ઢગલો_LEN(ઢગલો_size);
		geo.buf_start = K_THREAD_ઢગલો_BUFFER(ઢગલો);
		geo.buf_size = geo.obj_size - K_THREAD_ઢગલો_RESERVED;
	} નહીંતર
#endif /* CONFIG_USERSPACE */
	{
		/* Object cannot host a user mode thread */
		geo.obj_size = K_KERNEL_ઢગલો_LEN(ઢગલો_size);
		geo.buf_start = K_KERNEL_ઢગલો_BUFFER(ઢગલો);
		geo.buf_size = geo.obj_size - K_KERNEL_ઢગલો_RESERVED;

#જો defined(ARCH_KERNEL_ઢગલો_RESERVED)
		/* Zephyr treats stack overflow as an app bug.  But
		 * this particular overflow can be seen by static
		 * analysis so needs to be handled somehow.
		 */
		જો (K_KERNEL_ઢગલો_RESERVED > geo.obj_size) {
			k_panic();
		}
#endif
	}

	પાછા geo;
}

/*
 * Return the initial (high end) stack pointer. When stacks are memory mapped
 * this maps the object into virtual memory, records the mapping so it can be
 * torn down at thread exit, and rebases buf_start onto the mapped address.
 */
સ્થિર અક્ષર *map_thread_ઢગલો(સંરચના k_thread *new_thread,
			      k_thread_ઢગલો_t *ઢગલો,
			      સંરચના ઢગલો_geometry *geo)
{
	અક્ષર *ઢગલો_ptr;

#ifdef CONFIG_THREAD_ઢગલો_MEM_MAPPED
	/* Map the stack into virtual memory and use that as the base to
	 * calculate the initial stack pointer at the high end of the stack
	 * object. The stack pointer may be reduced later by TLS or random
	 * offset.
	 *
	 * K_MEM_MAP_UNINIT is used to mimic the behavior of non-mapped
	 * stack. If CONFIG_INIT_STACKS is enabled, the stack will be
	 * cleared later.
	 */
	શૂન્ય *ઢગલો_mapped = k_mem_map_phys_guard((uintptr_t)ઢગલો, geo->obj_size,
				K_MEM_PERM_RW | K_MEM_CACHE_WB | K_MEM_MAP_UNINIT,
				false);

	__ASSERT_NO_MSG((uintptr_t)ઢગલો_mapped != 0);

#ifdef CONFIG_USERSPACE
	જો (z_ઢગલો_is_user_capable(ઢગલો)) {
		geo->buf_start = K_THREAD_ઢગલો_BUFFER(ઢગલો_mapped);
	} નહીંતર
#endif /* CONFIG_USERSPACE */
	{
		geo->buf_start = K_KERNEL_ઢગલો_BUFFER(ઢગલો_mapped);
	}

	ઢગલો_ptr = (અક્ષર *)ઢગલો_mapped + geo->obj_size;

	/* Need to store the info on mapped stack so we can remove the mappings
	 * when the thread ends.
	 */
	new_thread->ઢગલો_info.mapped.addr = ઢગલો_mapped;
	new_thread->ઢગલો_info.mapped.sz = geo->obj_size;
#નહીંતર /* CONFIG_THREAD_STACK_MEM_MAPPED */
	ARG_UNUSED(new_thread);

	/* Initial stack pointer at the high end of the stack object, may
	 * be reduced later by TLS or random offset
	 */
	ઢગલો_ptr = (અક્ષર *)ઢગલો + geo->obj_size;
#endif /* CONFIG_THREAD_STACK_MEM_MAPPED */

	પાછા ઢગલો_ptr;
}

/* Pre-fill the stack buffer with a known pattern for usage measurement. */
સ્થિર અંતઃરેખ શૂન્ય fill_init_ઢગલો(અક્ષર *buf_start, size_t buf_size)
{
#ifdef CONFIG_INIT_STACKS
	memset(buf_start, 0xaa, buf_size);
#નહીંતર
	ARG_UNUSED(buf_start);
	ARG_UNUSED(buf_size);
#endif /* CONFIG_INIT_STACKS */
}

/* Place the overflow-detection sentinel at the low end of the stack buffer. */
સ્થિર અંતઃરેખ શૂન્ય set_ઢગલો_sentinel(અક્ષર *buf_start)
{
#ifdef CONFIG_ઢગલો_SENTINEL
	/* Put the stack sentinel at the lowest 4 bytes of the stack area.
	 * We periodically check that it's still present and kill the thread
	 * if it isn't.
	 */
	*((uint32_t *)buf_start) = ઢગલો_SENTINEL;
#નહીંતર
	ARG_UNUSED(buf_start);
#endif /* CONFIG_STACK_SENTINEL */
}

/*
 * Compute how much to reserve at the top of the stack buffer for TLS,
 * userspace-local data and stack-pointer randomization, and return the
 * aligned amount the initial stack pointer must be lowered by.
 */
સ્થિર size_t reserve_ઢગલો_headroom(સંરચના k_thread *new_thread,
				     અક્ષર *ઢગલો_ptr, size_t buf_size)
{
	size_t delta = 0;

	ARG_UNUSED(new_thread);
	ARG_UNUSED(ઢગલો_ptr);
	ARG_UNUSED(buf_size);

#ifdef CONFIG_THREAD_LOCAL_STORAGE
	/* TLS is always last within the stack buffer */
	delta += arch_tls_ઢગલો_setup(new_thread, ઢગલો_ptr);
#endif /* CONFIG_THREAD_LOCAL_STORAGE */
#ifdef CONFIG_THREAD_USERSPACE_LOCAL_DATA
	size_t tls_size = કદ(સંરચના _thread_userspace_local_data);

	/* reserve space on highest memory of stack buffer for local data */
	delta += tls_size;
	new_thread->userspace_local_data =
		(સંરચના _thread_userspace_local_data *)(ઢગલો_ptr - delta);
#endif /* CONFIG_THREAD_USERSPACE_LOCAL_DATA */
#જો defined(CONFIG_ઢગલો_POINTER_RANDOM) && (CONFIG_ઢગલો_POINTER_RANDOM != 0)
	delta += random_offset(buf_size);
#endif /* CONFIG_STACK_POINTER_RANDOM */

	પાછા ROUND_UP(delta, ARCH_ઢગલો_PTR_ALIGN);
}

/* Record the thread-accessible stack bounds (and runtime-safety threshold). */
સ્થિર અંતઃરેખ શૂન્ય set_ઢગલો_info(સંરચના k_thread *new_thread, અક્ષર *buf_start,
				  size_t buf_size, size_t delta)
{
#ifdef CONFIG_THREAD_ઢગલો_INFO
	/* Initial values. Arches which implement MPU guards that "borrow"
	 * memory from the stack buffer (not tracked in K_THREAD_STACK_RESERVED)
	 * will need to appropriately update this.
	 *
	 * The bounds tracked here correspond to the area of the stack object
	 * that the thread can access, which includes TLS.
	 */
	new_thread->ઢગલો_info.start = (uintptr_t)buf_start;
	new_thread->ઢગલો_info.size = buf_size;
	new_thread->ઢગલો_info.delta = delta;

#ifdef CONFIG_THREAD_RUNTIME_ઢગલો_SAFETY
	new_thread->ઢગલો_info.usage.unused_threshold =
		(CONFIG_THREAD_RUNTIME_ઢગલો_SAFETY_DEFAULT_UNUSED_THRESHOLD_PCT *
		 buf_size) / 100;
#endif
#નહીંતર
	ARG_UNUSED(new_thread);
	ARG_UNUSED(buf_start);
	ARG_UNUSED(buf_size);
	ARG_UNUSED(delta);
#endif /* CONFIG_THREAD_STACK_INFO */
}

સ્થિર અક્ષર *setup_thread_ઢગલો(સંરચના k_thread *new_thread,
				k_thread_ઢગલો_t *ઢગલો, size_t ઢગલો_size)
{
	સંરચના ઢગલો_geometry geo = compute_ઢગલો_geometry(ઢગલો, ઢગલો_size);
	અક્ષર *ઢગલો_ptr = map_thread_ઢગલો(new_thread, ઢગલો, &geo);
	size_t delta;

	LOG_DBG("stack %p for thread %p: obj_size=%zu buf_start=%p "
		" buf_size %zu stack_ptr=%p",
		ઢગલો, new_thread, geo.obj_size, (શૂન્ય *)geo.buf_start,
		geo.buf_size, (શૂન્ય *)ઢગલો_ptr);

	fill_init_ઢગલો(geo.buf_start, geo.buf_size);
	set_ઢગલો_sentinel(geo.buf_start);
	delta = reserve_ઢગલો_headroom(new_thread, ઢગલો_ptr, geo.buf_size);
	set_ઢગલો_info(new_thread, geo.buf_start, geo.buf_size, delta);

	પાછા ઢગલો_ptr - delta;
}

#ifdef CONFIG_HW_SHADOW_ઢગલો
સ્થિર શૂન્ય setup_shadow_ઢગલો(સંરચના k_thread *new_thread,
			 k_thread_ઢગલો_t *ઢગલો)
{
	પૂર્ણાંક ret = -ENOENT;

	STRUCT_SECTION_FOREACH(ઢગલો_to_hw_shadow_ઢગલો, stk_to_hw_shstk) {
		જો (stk_to_hw_shstk->ઢગલો == ઢગલો) {
			ret = k_thread_hw_shadow_ઢગલો_attach(new_thread,
							      stk_to_hw_shstk->shstk_addr,
							      stk_to_hw_shstk->size);
			જો (ret != 0) {
				LOG_ERR("Could not set thread %p shadow stack %p, got error %d",
					new_thread, stk_to_hw_shstk->shstk_addr, ret);
				k_panic();
			}
			તોડો;
		}
	}

	/* Check if the stack isn't in a stack array, and use the corresponding
	 * shadow stack.
	 */
	જો (ret != -ENOENT) {
		પાછા;
	}

	STRUCT_SECTION_FOREACH(ઢગલો_to_hw_shadow_ઢગલો_arr, stk_to_hw_shstk) {
		જો ((uintptr_t)ઢગલો >= stk_to_hw_shstk->ઢગલો_addr &&
		    (uintptr_t)ઢગલો < stk_to_hw_shstk->ઢગલો_addr +
		    stk_to_hw_shstk->ઢગલો_size * stk_to_hw_shstk->nmemb) {
			/* Now we have to guess which index of the stack array is being used */
			uintptr_t ઢગલો_offset = (uintptr_t)ઢગલો - stk_to_hw_shstk->ઢગલો_addr;
			uintptr_t ઢગલો_index = ઢગલો_offset / stk_to_hw_shstk->ઢગલો_size;
			uintptr_t addr;

			જો (ઢગલો_index >= stk_to_hw_shstk->nmemb) {
				LOG_ERR("Could not find shadow stack for thread %p, stack %p",
					new_thread, ઢગલો);
				k_panic();
			}

			addr = stk_to_hw_shstk->shstk_addr +
				stk_to_hw_shstk->shstk_size * ઢગલો_index;
			ret = k_thread_hw_shadow_ઢગલો_attach(new_thread,
							      (arch_thread_hw_shadow_ઢગલો_t *)addr,
							      stk_to_hw_shstk->shstk_size);
			જો (ret != 0) {
				LOG_ERR("Could not set thread %p shadow stack 0x%lx, got error %d",
					new_thread, stk_to_hw_shstk->shstk_addr, ret);
				k_panic();
			}
			તોડો;
		}
	}

	જો (ret == -ENOENT) {
		LOG_ERR("Could not find shadow stack for thread %p, stack %p",
			new_thread, ઢગલો);
		k_panic();
	}
}
#નહીંતર
/* No-op stub used when hardware shadow stacks are not configured. */
સ્થિર અંતઃરેખ શૂન્ય setup_shadow_ઢગલો(સંરચના k_thread *new_thread,
				      k_thread_ઢગલો_t *ઢગલો)
{
	ARG_UNUSED(new_thread);
	ARG_UNUSED(ઢગલો);
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
સ્થિર અંતઃરેખ શૂન્ય thread_abort_cleanup_check_reuse(સંરચના k_thread *thread)
{
#ifdef CONFIG_THREAD_ABORT_NEED_CLEANUP
	k_thread_abort_cleanup_check_reuse(thread);
#નહીંતર
	ARG_UNUSED(thread);
#endif /* CONFIG_THREAD_ABORT_NEED_CLEANUP */
}

/* Register the thread with the object-core framework and its usage stats. */
સ્થિર અંતઃરેખ શૂન્ય init_thread_obj_core(સંરચના k_thread *thread)
{
#ifdef CONFIG_OBJ_CORE_THREAD
	k_obj_core_init_and_link(K_OBJ_CORE(thread), &obj_type_thread);
#ifdef CONFIG_OBJ_CORE_STATS_THREAD
	k_obj_core_stats_register(K_OBJ_CORE(thread), &thread->base.usage,
				  કદ(thread->base.usage));
#endif /* CONFIG_OBJ_CORE_STATS_THREAD */
#નહીંતર
	ARG_UNUSED(thread);
#endif /* CONFIG_OBJ_CORE_THREAD */
}

/*
 * Initialize userspace bookkeeping: register the thread and stack kernel
 * objects, record stack ownership, and grant the thread access to itself.
 */
સ્થિર અંતઃરેખ શૂન્ય init_thread_userspace(સંરચના k_thread *thread,
					 k_thread_ઢગલો_t *ઢગલો,
					 uint32_t options)
{
#ifdef CONFIG_USERSPACE
	__ASSERT((options & K_USER) == 0U || z_ઢગલો_is_user_capable(ઢગલો),
		 "user thread %p with kernel-only stack %p", thread, ઢગલો);
	k_object_init(thread);
	k_object_init(ઢગલો);
	thread->ઢગલો_obj = ઢગલો;
	thread->syscall_frame = ખાલી;

	/* Any given thread has access to itself */
	k_object_access_grant(thread, thread);
#નહીંતર
	ARG_UNUSED(thread);
	ARG_UNUSED(ઢગલો);
	ARG_UNUSED(options);
#endif /* CONFIG_USERSPACE */
}

/* Assert the thread object is cache-coherent while its stack stays cached. */
સ્થિર અંતઃરેખ શૂન્ય assert_thread_coherence(સંરચના k_thread *thread,
					   k_thread_ઢગલો_t *ઢગલો)
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
	__ASSERT_NO_MSG(!sys_cache_is_mem_coherent(ઢગલો));
#endif /* CONFIG_DYNAMIC_THREAD */
#endif /* CONFIG_KERNEL_COHERENCE */
	ARG_UNUSED(thread);
	ARG_UNUSED(ઢગલો);
}

/* Assert the arch layer populated switch_handle on USE_SWITCH builds. */
સ્થિર અંતઃરેખ શૂન્ય assert_switch_handle(સંરચના k_thread *thread)
{
#ifdef CONFIG_USE_SWITCH
	/* switch_handle must be non-null except when inside z_swap()
	 * for synchronization reasons.  Historically some notional
	 * USE_SWITCH architectures have actually ignored the field
	 */
	__ASSERT(thread->switch_handle != ખાલી,
		 "arch layer failed to initialize switch_handle");
#endif /* CONFIG_USE_SWITCH */
	ARG_UNUSED(thread);
}

/* Clear the opaque, kernel-managed per-thread custom data pointer. */
સ્થિર અંતઃરેખ શૂન્ય init_thread_custom_data(સંરચના k_thread *thread)
{
#ifdef CONFIG_THREAD_CUSTOM_DATA
	/* Initialize custom data field (value is opaque to kernel) */
	thread->custom_data = ખાલી;
#નહીંતર
	ARG_UNUSED(thread);
#endif /* CONFIG_THREAD_CUSTOM_DATA */
}

/*
 * Record the entry point and parameters, then link the thread into the
 * global thread-monitor list under the monitor spinlock.
 */
સ્થિર અંતઃરેખ શૂન્ય add_thread_to_monitor(સંરચના k_thread *thread,
					 k_thread_entry_t entry,
					 શૂન્ય *p1, શૂન્ય *p2, શૂન્ય *p3)
{
#ifdef CONFIG_THREAD_MONITOR
	thread->entry.pEntry = entry;
	thread->entry.parameter1 = p1;
	thread->entry.parameter2 = p2;
	thread->entry.parameter3 = p3;

	k_spinlock_ચાવી_t ચાવી = k_spin_lock(&z_thread_monitor_lock);

	thread->next_thread = _kernel.threads;
	_kernel.threads = thread;
	k_spin_unlock(&z_thread_monitor_lock, ચાવી);
#નહીંતર
	ARG_UNUSED(thread);
	ARG_UNUSED(entry);
	ARG_UNUSED(p1);
	ARG_UNUSED(p2);
	ARG_UNUSED(p3);
#endif /* CONFIG_THREAD_MONITOR */
}

/* Set the thread's name from the caller-supplied string (may be NULL). */
સ્થિર અંતઃરેખ શૂન્ય init_thread_name(સંરચના k_thread *thread, અચલ અક્ષર *name)
{
#ifdef CONFIG_THREAD_NAME
	set_thread_name(thread, name);
#નહીંતર
	ARG_UNUSED(thread);
	ARG_UNUSED(name);
#endif /* CONFIG_THREAD_NAME */
}

/* Set the thread's initial SMP CPU affinity mask. */
સ્થિર અંતઃરેખ શૂન્ય init_thread_cpu_mask(સંરચના k_thread *thread)
{
#ifdef CONFIG_SCHED_CPU_MASK
	જો (IS_ENABLED(CONFIG_SCHED_CPU_MASK_PIN_ONLY)) {
		thread->base.cpu_mask = 1; /* must specify only one cpu */
	} નહીંતર {
		thread->base.cpu_mask = -1; /* allow all cpus */
	}
#નહીંતર
	ARG_UNUSED(thread);
#endif /* CONFIG_SCHED_CPU_MASK */
}

/* Attach the thread to its memory domain and inherit caller permissions
 * when K_INHERIT_PERMS is requested.
 */
સ્થિર અંતઃરેખ શૂન્ય init_thread_userspace_perms(સંરચના k_thread *thread,
					       uint32_t options)
{
#ifdef CONFIG_USERSPACE
	z_mem_domain_init_thread(thread);

	જો ((options & K_INHERIT_PERMS) != 0U) {
		k_thread_perms_inherit(_current, thread);
	}
#નહીંતર
	ARG_UNUSED(thread);
	ARG_UNUSED(options);
#endif /* CONFIG_USERSPACE */
}

/* Clear the EDF scheduling deadline. */
સ્થિર અંતઃરેખ શૂન્ય init_thread_deadline(સંરચના k_thread *thread)
{
#ifdef CONFIG_SCHED_DEADLINE
	thread->base.prio_deadline = 0;
#નહીંતર
	ARG_UNUSED(thread);
#endif /* CONFIG_SCHED_DEADLINE */
}

/* Initialize the wait queue used to synchronize SMP thread halt/abort. */
સ્થિર અંતઃરેખ શૂન્ય init_thread_halt_queue(સંરચના k_thread *thread)
{
#ifdef CONFIG_SMP
	z_waitq_init(&thread->halt_queue);
#નહીંતર
	ARG_UNUSED(thread);
#endif /* CONFIG_SMP */
}

/* Initialize runtime CPU-usage tracking for the thread. */
સ્થિર અંતઃરેખ શૂન્ય init_thread_usage(સંરચના k_thread *thread)
{
#ifdef CONFIG_SCHED_THREAD_USAGE
	thread->base.usage = (સંરચના k_cycle_stats) {};
	thread->base.usage.track_usage = CONFIG_SCHED_THREAD_USAGE_AUTO_ENABLE;
#નહીંતર
	ARG_UNUSED(thread);
#endif /* CONFIG_SCHED_THREAD_USAGE */
}

/* Assert the thread wasn't already linked into the monitor list. */
સ્થિર અંતઃરેખ શૂન્ય assert_thread_not_reused(સંરચના k_thread *new_thread)
{
#ifdef CONFIG_THREAD_MONITOR
#ifdef CONFIG_ASSERT
	k_spinlock_ચાવી_t ચાવી = k_spin_lock(&z_thread_monitor_lock);
	bool reused = false;

	/* Check if the thread is already in the list */
	માટે (સંરચના k_thread *t = _kernel.threads; t; t = t->next_thread) {
		જો (t == new_thread) {
			reused = true;
			તોડો;
		}
	}

	k_spin_unlock(&z_thread_monitor_lock, ચાવી);

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
અક્ષર *z_setup_new_thread(સંરચના k_thread *new_thread,
			 k_thread_ઢગલો_t *ઢગલો, size_t ઢગલો_size,
			 k_thread_entry_t entry,
			 શૂન્ય *p1, શૂન્ય *p2, શૂન્ય *p3,
			 પૂર્ણાંક prio, uint32_t options, અચલ અક્ષર *name)
{
	અક્ષર *ઢગલો_ptr;

	Z_ASSERT_VALID_PRIO(prio, entry);

	assert_thread_not_reused(new_thread);

	thread_abort_cleanup_check_reuse(new_thread);
	init_thread_obj_core(new_thread);
	init_thread_userspace(new_thread, ઢગલો, options);

	z_waitq_init(&new_thread->join_queue);

	/* Initialize various struct k_thread members */
	z_init_thread_base(&new_thread->base, prio, _THREAD_SLEEPING, options);
	ઢગલો_ptr = setup_thread_ઢગલો(new_thread, ઢગલો, ઢગલો_size);

#જો Z_MUTEX_PI_ENABLED
	sys_slist_init(&new_thread->held_mutexes);
	new_thread->mutex_pended_on = ખાલી;
	new_thread->orig_prio = new_thread->base.prio;
#endif /* Z_MUTEX_PI_ENABLED */

	setup_shadow_ઢગલો(new_thread, ઢગલો);
	assert_thread_coherence(new_thread, ઢગલો);

	arch_new_thread(new_thread, ઢગલો, ઢગલો_ptr, entry, p1, p2, p3);

	/* static threads overwrite it afterwards with real value */
	new_thread->init_data = ખાલી;

	assert_switch_handle(new_thread);
	init_thread_custom_data(new_thread);
	add_thread_to_monitor(new_thread, entry, p1, p2, p3);
	init_thread_name(new_thread, name);
	init_thread_cpu_mask(new_thread);

	/* _current may be NULL if the dummy thread is not used */
	જો (IS_ENABLED(CONFIG_ARCH_HAS_CUSTOM_SWAP_TO_MAIN) &&
	    (_current == ખાલી)) {
		new_thread->resource_pool = ખાલી;
		પાછા ઢગલો_ptr;
	}

	init_thread_userspace_perms(new_thread, options);
	init_thread_deadline(new_thread);
	new_thread->resource_pool = _current->resource_pool;
	init_thread_halt_queue(new_thread);
	init_thread_usage(new_thread);

	SYS_PORT_TRACING_OBJ_FUNC(k_thread, create, new_thread);

	પાછા ઢગલો_ptr;
}

#ifdef CONFIG_THREAD_RUNTIME_ઢગલો_SAFETY
પૂર્ણાંક z_impl_k_thread_runtime_ઢગલો_unused_threshold_pct_set(સંરચના k_thread *thread,
							   uint32_t pct)
{
	size_t unused_threshold;

	જો (pct > 99) {
		પાછા -EINVAL;   /* 100% unused stack and up is invalid */
	}

	unused_threshold = (thread->ઢગલો_info.size * pct) / 100;

	thread->ઢગલો_info.usage.unused_threshold = unused_threshold;

	પાછા 0;
}

પૂર્ણાંક z_impl_k_thread_runtime_ઢગલો_unused_threshold_set(સંરચના k_thread *thread,
						       size_t threshold)
{
	જો (threshold > thread->ઢગલો_info.size) {
		પાછા -EINVAL;
	}

	thread->ઢગલો_info.usage.unused_threshold = threshold;

	પાછા 0;
}

size_t z_impl_k_thread_runtime_ઢગલો_unused_threshold_get(સંરચના k_thread *thread)
{
	પાછા thread->ઢગલો_info.usage.unused_threshold;
}

#ifdef CONFIG_USERSPACE
પૂર્ણાંક z_vrfy_k_thread_runtime_ઢગલો_unused_threshold_pct_set(સંરચના k_thread *thread,
							   uint32_t pct)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));

	પાછા z_impl_k_thread_runtime_ઢગલો_unused_threshold_pct_set(thread, pct);
}
#અન્તર્ભાવ <zephyr/syscalls/k_thread_runtime_stack_unused_threshold_pct_set_mrsh.c>

પૂર્ણાંક z_vrfy_k_thread_runtime_ઢગલો_unused_threshold_set(સંરચના k_thread *thread,
						       size_t threshold)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));

	પાછા z_impl_k_thread_runtime_ઢગલો_unused_threshold_set(thread, threshold);
}
#અન્તર્ભાવ <zephyr/syscalls/k_thread_runtime_stack_unused_threshold_set_mrsh.c>

size_t z_vrfy_k_thread_runtime_ઢગલો_unused_threshold_get(સંરચના k_thread *thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));

	પાછા z_impl_k_thread_runtime_ઢગલો_unused_threshold_get(thread);
}
#અન્તર્ભાવ <zephyr/syscalls/k_thread_runtime_stack_unused_threshold_get_mrsh.c>
#endif /* CONFIG_USERSPACE */
#endif /* CONFIG_THREAD_RUNTIME_STACK_SAFETY */

k_tid_t z_impl_k_thread_create(સંરચના k_thread *new_thread,
			      k_thread_ઢગલો_t *ઢગલો,
			      size_t ઢગલો_size, k_thread_entry_t entry,
			      શૂન્ય *p1, શૂન્ય *p2, શૂન્ય *p3,
			      પૂર્ણાંક prio, uint32_t options, k_timeout_t delay)
{
	__ASSERT(!arch_is_in_isr(), "Threads may not be created in ISRs");

	z_setup_new_thread(new_thread, ઢગલો, ઢગલો_size, entry, p1, p2, p3,
			  prio, options, ખાલી);

	thread_schedule_new(new_thread, delay);

	પાછા new_thread;
}

#ifdef CONFIG_USERSPACE
bool z_ઢગલો_is_user_capable(k_thread_ઢગલો_t *ઢગલો)
{
	પાછા k_object_find(ઢગલો) != ખાલી;
}

k_tid_t z_vrfy_k_thread_create(સંરચના k_thread *new_thread,
			       k_thread_ઢગલો_t *ઢગલો,
			       size_t ઢગલો_size, k_thread_entry_t entry,
			       શૂન્ય *p1, શૂન્ય *p2, શૂન્ય *p3,
			       પૂર્ણાંક prio, uint32_t options, k_timeout_t delay)
{
	size_t total_size, ઢગલો_obj_size;
	સંરચના k_object *ઢગલો_object;

	/* The thread and stack objects *must* be in an uninitialized state */
	K_OOPS(K_SYSCALL_OBJ_NEVER_INIT(new_thread, K_OBJ_THREAD));

	/* No need to check z_stack_is_user_capable(), it won't be in the
	 * object table if it isn't
	 */
	ઢગલો_object = k_object_find(ઢગલો);
	K_OOPS(K_SYSCALL_VERIFY_સંદેશ(k_object_validation_check(ઢગલો_object, ઢગલો,
						K_OBJ_THREAD_ઢગલો_ELEMENT,
						_OBJ_INIT_FALSE) == 0,
				    "bad stack object"));

	/* Verify that the stack size passed in is OK by computing the total
	 * size and comparing it with the size value in the object metadata
	 */
	K_OOPS(K_SYSCALL_VERIFY_સંદેશ(!size_add_overflow(K_THREAD_ઢગલો_RESERVED,
						       ઢગલો_size, &total_size),
				    "stack size overflow (%zu+%zu)",
				    ઢગલો_size,
				    K_THREAD_ઢગલો_RESERVED));

	/* Testing less-than-or-equal since additional room may have been
	 * allocated for alignment constraints
	 */
#ifdef CONFIG_GEN_PRIV_STACKS
	ઢગલો_obj_size = ઢગલો_object->data.ઢગલો_data->size;
#નહીંતર
	ઢગલો_obj_size = ઢગલો_object->data.ઢગલો_size;
#endif /* CONFIG_GEN_PRIV_STACKS */
	K_OOPS(K_SYSCALL_VERIFY_સંદેશ(total_size <= ઢગલો_obj_size,
				    "stack size %zu is too big, max is %zu",
				    total_size, ઢગલો_obj_size));

	/* User threads may only create other user threads and they can't
	 * be marked as essential
	 */
	K_OOPS(K_SYSCALL_VERIFY(options & K_USER));
	K_OOPS(K_SYSCALL_VERIFY(!(options & K_ESSENTIAL)));

	/* Check validity of prio argument; must be the same or worse priority
	 * than the caller
	 */
	K_OOPS(K_SYSCALL_VERIFY(_is_valid_prio(prio, ખાલી)));
	K_OOPS(K_SYSCALL_VERIFY(z_is_prio_lower_or_equal(prio,
							_current->base.prio)));

	z_setup_new_thread(new_thread, ઢગલો, ઢગલો_size,
			   entry, p1, p2, p3, prio, options, ખાલી);

	thread_schedule_new(new_thread, delay);

	પાછા new_thread;
}
#અન્તર્ભાવ <zephyr/syscalls/k_thread_create_mrsh.c>
#endif /* CONFIG_USERSPACE */

શૂન્ય z_init_thread_base(સંરચના _thread_base *thread_base, પૂર્ણાંક priority,
		       uint32_t initial_state, unsigned પૂર્ણાંક options)
{
	/* k_q_node is initialized upon first insertion in a list */
	thread_base->pended_on = ખાલી;
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
	thread_base->slice_expired = ખાલી;
#endif /* CONFIG_TIMESLICE_PER_THREAD */

#ifdef CONFIG_SPIN_VALIDATE
	thread_base->swap_data = ખાલી;
	/* otherwise swap_data does not need to be initialized */
#endif

	z_init_thread_timeout(thread_base);
}

FUNC_NORETURN શૂન્ય k_thread_user_mode_enter(k_thread_entry_t entry,
					    શૂન્ય *p1, શૂન્ય *p2, શૂન્ય *p3)
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
	__ASSERT(z_ઢગલો_is_user_capable(_current->ઢગલો_obj),
		 "dropping to user mode with kernel-only stack object");
#ifdef CONFIG_THREAD_USERSPACE_LOCAL_DATA
	memset(_current->userspace_local_data, 0,
	       કદ(સંરચના _thread_userspace_local_data));
#endif /* CONFIG_THREAD_USERSPACE_LOCAL_DATA */
#ifdef CONFIG_THREAD_LOCAL_STORAGE
	arch_tls_ઢગલો_setup(_current,
			     (અક્ષર *)(_current->ઢગલો_info.start +
				      _current->ઢગલો_info.size));
#endif /* CONFIG_THREAD_LOCAL_STORAGE */
	arch_user_mode_enter(entry, p1, p2, p3);
#નહીંતર
	/* XXX In this case we do not reset the stack */
	z_thread_entry(entry, p1, p2, p3);
#endif /* CONFIG_USERSPACE */
}

#જો defined(CONFIG_INIT_STACKS) && defined(CONFIG_THREAD_ઢગલો_INFO)
#ifdef CONFIG_ઢગલો_GROWS_UP
#error "Unsupported configuration for stack analysis"
#endif /* CONFIG_STACK_GROWS_UP */

પૂર્ણાંક z_ઢગલો_space_get(અચલ uint8_t *ઢગલો_start, size_t size, size_t *unused_ptr)
{
	size_t unused = 0;
	અચલ uint8_t *checked_ઢગલો = ઢગલો_start;
	/* Take the address of any local variable as a shallow bound for the
	 * stack pointer.  Addresses above it are guaranteed to be
	 * accessible.
	 */
	અચલ uint8_t *ઢગલો_pointer = (અચલ uint8_t *)&ઢગલો_start;

	/* If we are currently running on the stack being analyzed, some
	 * memory management hardware will generate an exception if we
	 * read unused stack memory.
	 *
	 * This never happens when invoked from user mode, as user mode
	 * will always run this function on the privilege elevation stack.
	 */
	જો ((ઢગલો_pointer > ઢગલો_start) && (ઢગલો_pointer <= (ઢગલો_start + size)) &&
	    IS_ENABLED(CONFIG_NO_UNUSED_ઢગલો_INSPECTION)) {
		/* TODO: We could add an arch_ API call to temporarily
		 * disable the stack checking in the CPU, but this would
		 * need to be properly managed wrt context switches/interrupts
		 */
		પાછા -ENOTSUP;
	}

	જો (IS_ENABLED(CONFIG_ઢગલો_SENTINEL)) {
		/* First 4 bytes of the stack buffer reserved for the
		 * sentinel value, it won't be 0xAAAAAAAA for thread
		 * stacks.
		 *
		 * FIXME: thread->stack_info.start ought to reflect
		 * this!
		 */
		checked_ઢગલો += 4;
		size -= 4;
	}

	માટે (size_t i = 0; i < size; i++) {
		જો ((checked_ઢગલો[i]) == 0xaaU) {
			unused++;
		} નહીંતર {
			તોડો;
		}
	}

	*unused_ptr = unused;

	પાછા 0;
}

#ifdef CONFIG_THREAD_RUNTIME_ઢગલો_SAFETY
પૂર્ણાંક k_thread_runtime_ઢગલો_safety_full_check(અચલ સંરચના k_thread *thread,
					     size_t *unused_ptr,
					     k_thread_ઢગલો_safety_handler_t handler,
					     શૂન્ય *arg)
{
	પૂર્ણાંક    rv;
	size_t unused_space;

	__ASSERT_NO_MSG(thread != ખાલી);

	rv = z_ઢગલો_space_get((અચલ uint8_t *)thread->ઢગલો_info.start,
			       thread->ઢગલો_info.size, &unused_space);

	જો (rv != 0) {
		પાછા rv;
	}

	જો (unused_ptr != ખાલી) {
		*unused_ptr = unused_space;
	}

	જો ((unused_space < thread->ઢગલો_info.usage.unused_threshold) &&
	    (handler != ખાલી)) {
		handler(thread, unused_space, arg);
	}

	પાછા 0;
}

પૂર્ણાંક k_thread_runtime_ઢગલો_safety_threshold_check(અચલ સંરચના k_thread *thread,
						  size_t *unused_ptr,
						  k_thread_ઢગલો_safety_handler_t handler,
						  શૂન્ય *arg)
{
	પૂર્ણાંક    rv;
	size_t unused_space;
	size_t threshold;
	size_t scan_size;

	__ASSERT_NO_MSG(thread != ખાલી);

	threshold = thread->ઢગલો_info.usage.unused_threshold;
	scan_size = threshold;

	/* z_stack_space_get() reserves the first bytes of the stack buffer for
	 * the stack sentinel and shrinks its scan window by that amount. Grow
	 * the requested window by the same reservation so that up to
	 * 'threshold' bytes of the usable stack are actually examined;
	 * otherwise a fully unused window would report fewer than 'threshold'
	 * unused bytes and spuriously trip the threshold.
	 */
	જો (IS_ENABLED(CONFIG_ઢગલો_SENTINEL) && (threshold != 0U)) {
		scan_size += કદ(uint32_t);
	}

	rv = z_ઢગલો_space_get((અચલ uint8_t *)thread->ઢગલો_info.start,
			       scan_size, &unused_space);

	જો (rv != 0) {
		પાછા rv;
	}

	જો (unused_ptr != ખાલી) {
		*unused_ptr = unused_space;
	}

	જો ((unused_space < threshold) && (handler != ખાલી)) {
		handler(thread, unused_space, arg);
	}

	પાછા 0;
}
#endif /* CONFIG_THREAD_RUNTIME_STACK_SAFETY */

પૂર્ણાંક z_impl_k_thread_ઢગલો_space_get(અચલ સંરચના k_thread *thread,
				    size_t *unused_ptr)
{
#ifdef CONFIG_THREAD_ઢગલો_MEM_MAPPED
	જો (thread->ઢગલો_info.mapped.addr == ખાલી) {
		પાછા -EINVAL;
	}
#endif /* CONFIG_THREAD_STACK_MEM_MAPPED */

	પાછા z_ઢગલો_space_get((અચલ uint8_t *)thread->ઢગલો_info.start,
				 thread->ઢગલો_info.size, unused_ptr);
}

#ifdef CONFIG_USERSPACE
પૂર્ણાંક z_vrfy_k_thread_ઢગલો_space_get(અચલ સંરચના k_thread *thread,
				    size_t *unused_ptr)
{
	size_t unused;
	પૂર્ણાંક ret;

	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));

	ret = z_impl_k_thread_ઢગલો_space_get(thread, &unused);
	CHECKIF(ret != 0) {
		પાછા ret;
	}

	ret = k_usermode_to_copy(unused_ptr, &unused, કદ(size_t));
	CHECKIF(ret != 0) {
		પાછા ret;
	}

	પાછા 0;
}
#અન્તર્ભાવ <zephyr/syscalls/k_thread_stack_space_get_mrsh.c>
#endif /* CONFIG_USERSPACE */
#endif /* CONFIG_INIT_STACKS && CONFIG_THREAD_STACK_INFO */

#ifdef CONFIG_USERSPACE
સ્થિર અંતઃરેખ k_ticks_t z_vrfy_k_thread_timeout_remaining_ticks(
						    અચલ સંરચના k_thread *thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	પાછા z_impl_k_thread_timeout_remaining_ticks(thread);
}
#અન્તર્ભાવ <zephyr/syscalls/k_thread_timeout_remaining_ticks_mrsh.c>

સ્થિર અંતઃરેખ k_ticks_t z_vrfy_k_thread_timeout_expires_ticks(
						  અચલ સંરચના k_thread *thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	પાછા z_impl_k_thread_timeout_expires_ticks(thread);
}
#અન્તર્ભાવ <zephyr/syscalls/k_thread_timeout_expires_ticks_mrsh.c>
#endif /* CONFIG_USERSPACE */

#ifdef CONFIG_INSTRUMENT_THREAD_SWITCHING
શૂન્ય z_thread_mark_switched_in(શૂન્ય)
{
#જો defined(CONFIG_SCHED_THREAD_USAGE) && !defined(CONFIG_USE_SWITCH)
	z_sched_usage_start(_current);
#endif /* CONFIG_SCHED_THREAD_USAGE && !CONFIG_USE_SWITCH */

#ifdef CONFIG_TRACING
	SYS_PORT_TRACING_FUNC(k_thread, switched_in);
#endif /* CONFIG_TRACING */
}

શૂન્ય z_thread_mark_switched_out(શૂન્ય)
{
#જો defined(CONFIG_SCHED_THREAD_USAGE) && !defined(CONFIG_USE_SWITCH)
	z_sched_usage_stop();
#endif /*CONFIG_SCHED_THREAD_USAGE && !CONFIG_USE_SWITCH */

#ifdef CONFIG_TRACING
#ifdef CONFIG_THREAD_LOCAL_STORAGE
	/* Dummy thread won't have TLS set up to run arbitrary code */
	જો (!_current ||
	    (_current->base.thread_state & _THREAD_DUMMY) != 0) {
		પાછા;
	}
#endif /* CONFIG_THREAD_LOCAL_STORAGE */
	SYS_PORT_TRACING_FUNC(k_thread, switched_out);
#endif /* CONFIG_TRACING */
}
#endif /* CONFIG_INSTRUMENT_THREAD_SWITCHING */

પૂર્ણાંક k_thread_runtime_stats_get(k_tid_t thread,
			       k_thread_runtime_stats_t *stats)
{
	જો ((thread == ખાલી) || (stats == ખાલી)) {
		પાછા -EINVAL;
	}

#ifdef CONFIG_SCHED_THREAD_USAGE
	z_sched_thread_usage(thread, stats);
#નહીંતર
	*stats = (k_thread_runtime_stats_t) {};
#endif /* CONFIG_SCHED_THREAD_USAGE */

	પાછા 0;
}

પૂર્ણાંક k_thread_runtime_stats_all_get(k_thread_runtime_stats_t *stats)
{
#ifdef CONFIG_SCHED_THREAD_USAGE_ALL
	k_thread_runtime_stats_t  tmp_stats;
#endif /* CONFIG_SCHED_THREAD_USAGE_ALL */

	જો (stats == ખાલી) {
		પાછા -EINVAL;
	}

	*stats = (k_thread_runtime_stats_t) {};

#ifdef CONFIG_SCHED_THREAD_USAGE_ALL
	/* Retrieve the usage stats for each core and amalgamate them. */

	unsigned પૂર્ણાંક અંક_cpus = arch_અંક_cpus();

	માટે (unsigned પૂર્ણાંક i = 0; i < અંક_cpus; i++) {
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

	પાછા 0;
}

પૂર્ણાંક k_thread_runtime_stats_cpu_get(પૂર્ણાંક cpu, k_thread_runtime_stats_t *stats)
{
	જો (stats == ખાલી) {
		પાછા -EINVAL;
	}

	*stats = (k_thread_runtime_stats_t) {};

#ifdef CONFIG_SCHED_THREAD_USAGE_ALL
	CHECKIF((cpu < 0) || ((unsigned પૂર્ણાંક)cpu >= arch_અંક_cpus())) {
		પાછા -EINVAL;
	}

	z_sched_cpu_usage(cpu, stats);
#નહીંતર
	ARG_UNUSED(cpu);
#endif

	પાછા 0;
}

#ifdef CONFIG_THREAD_ABORT_NEED_CLEANUP
/** Pointer to thread which needs to be cleaned up. */
સ્થિર સંરચના k_thread *thread_to_cleanup;

/** Spinlock for thread abort cleanup. */
સ્થિર સંરચના k_spinlock thread_cleanup_lock;

#ifdef CONFIG_THREAD_ઢગલો_MEM_MAPPED
સ્થિર શૂન્ય *thread_cleanup_ઢગલો_addr;
સ્થિર size_t thread_cleanup_ઢગલો_sz;

સ્થિર શૂન્ય unmap_thread_ઢગલો(શૂન્ય *addr, size_t sz)
{
	જો (addr != ખાલી) {
		k_mem_unmap_phys_guard(addr, sz, false);
	}
}
#endif /* CONFIG_THREAD_STACK_MEM_MAPPED */

શૂન્ય defer_thread_cleanup(સંરચના k_thread *thread)
{
	/* Note when adding new deferred cleanup steps:
	 * - The thread object may have been overwritten by the time
	 *   the actual cleanup is being done (e.g. thread object
	 *   allocated on a stack). So stash any necessary data here
	 *   that will be used in the actual cleanup steps.
	 */
	thread_to_cleanup = thread;

#ifdef CONFIG_THREAD_ઢગલો_MEM_MAPPED
	/* Note that the permission of the stack should have been
	 * stripped of user thread access due to the thread having
	 * already exited from a memory domain. That is done via
	 * k_thread_abort().
	 */

	/* Stash the address and size so the region can be unmapped
	 * later.
	 */
	thread_cleanup_ઢગલો_addr = thread->ઢગલો_info.mapped.addr;
	thread_cleanup_ઢગલો_sz = thread->ઢગલો_info.mapped.sz;

	/* The stack is now considered un-usable. This should prevent any functions
	 * from looking directly into the mapped stack if they are made to be aware
	 * of memory mapped stacks, e.g., z_stack_space_get().
	 */
	thread->ઢગલો_info.mapped.addr = ખાલી;
	thread->ઢગલો_info.mapped.sz = 0;
#endif /* CONFIG_THREAD_STACK_MEM_MAPPED */
}

શૂન્ય do_deferred_thread_cleanup(શૂન્ય)
{
	/* Note when adding new actual cleanup steps:
	 * - The thread object may have been overwritten when this is
	 *   called. So only data stashed by defer_thread_cleanup() can
	 *   be used here, never anything from the thread object.
	 */

#ifdef CONFIG_THREAD_ઢગલો_MEM_MAPPED
	unmap_thread_ઢગલો(thread_cleanup_ઢગલો_addr, thread_cleanup_ઢગલો_sz);

	thread_cleanup_ઢગલો_addr = ખાલી;
	thread_cleanup_ઢગલો_sz = 0;
#endif /* CONFIG_THREAD_STACK_MEM_MAPPED */
}

શૂન્ય do_thread_cleanup(સંરચના k_thread *thread)
{
	/* This is only for threads which are no longer running and are not
	 * the current thread, so the thread object is still valid here and
	 * the cleanup can be driven from it directly.
	 */

#ifdef CONFIG_THREAD_ઢગલો_MEM_MAPPED
	unmap_thread_ઢગલો(thread->ઢગલો_info.mapped.addr,
			   thread->ઢગલો_info.mapped.sz);

	/* The stack is now unmapped and thus un-usable. Clear the mapping
	 * info so nothing looks into the stack afterwards, and so that
	 * a repeated cleanup of the same thread object is a no-op,
	 * e.g., z_stack_space_get().
	 */
	thread->ઢગલો_info.mapped.addr = ખાલી;
	thread->ઢગલો_info.mapped.sz = 0;
#નહીંતર /* CONFIG_THREAD_STACK_MEM_MAPPED */
	ARG_UNUSED(thread);
#endif /* CONFIG_THREAD_STACK_MEM_MAPPED */
}

શૂન્ય k_thread_abort_cleanup(સંરચના k_thread *thread)
{
	K_SPINLOCK(&thread_cleanup_lock) {
		જો (thread_to_cleanup != ખાલી) {
			/* Finish the pending one first. */
			do_deferred_thread_cleanup();
			thread_to_cleanup = ખાલી;
		}

		જો (thread == _current) {
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
		} નહીંતર {
			/* Not the current running thread, so we are safe to do
			 * cleanups.
			 */
			do_thread_cleanup(thread);
		}
	}
}

શૂન્ય k_thread_abort_cleanup_check_reuse(સંરચના k_thread *thread)
{
	K_SPINLOCK(&thread_cleanup_lock) {
		/* This is to guard reuse of the same thread object and make sure
		 * any pending cleanups of it needs to be finished before the thread
		 * object can be reused.
		 */
		જો (thread_to_cleanup == thread) {
			do_deferred_thread_cleanup();
			thread_to_cleanup = ખાલી;
		}
	}
}

#endif /* CONFIG_THREAD_ABORT_NEED_CLEANUP */

શૂન્ય z_dummy_thread_init(સંરચના k_thread *dummy_thread)
{
	dummy_thread->base.thread_state = _THREAD_DUMMY;
#ifdef CONFIG_SCHED_CPU_MASK
	dummy_thread->base.cpu_mask = -1;
#endif /* CONFIG_SCHED_CPU_MASK */
	dummy_thread->base.user_options = K_ESSENTIAL;
#જો Z_MUTEX_PI_ENABLED
	sys_slist_init(&dummy_thread->held_mutexes);
	dummy_thread->mutex_pended_on = ખાલી;
	dummy_thread->orig_prio = dummy_thread->base.prio;
#endif /* Z_MUTEX_PI_ENABLED */
#ifdef CONFIG_THREAD_ઢગલો_INFO
	dummy_thread->ઢગલો_info.start = 0U;
	dummy_thread->ઢગલો_info.size = 0U;
#endif /* CONFIG_THREAD_STACK_INFO */
#ifdef CONFIG_USERSPACE
	dummy_thread->mem_domain_info.mem_domain = &k_mem_domain_default;
#endif /* CONFIG_USERSPACE */
#જો (K_HEAP_MEM_POOL_SIZE > 0)
	k_thread_system_pool_assign(dummy_thread);
#નહીંતર
	dummy_thread->resource_pool = ખાલી;
#endif /* K_HEAP_MEM_POOL_SIZE */

#ifdef CONFIG_USE_SWITCH
	dummy_thread->switch_handle = ખાલી;
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

શૂન્ય z_impl_k_thread_priority_set(k_tid_t thread, પૂર્ણાંક prio)
{
	/*
	 * Use NULL, since we cannot know what the entry point is (we do not
	 * keep track of it) and idle cannot change its priority.
	 */
	Z_ASSERT_VALID_PRIO(prio, ખાલી);

	bool need_sched = z_thread_prio_set((સંરચના k_thread *)thread, prio);

	જો ((need_sched) && (IS_ENABLED(CONFIG_SMP) ||
			     (_current->base.sched_locked == 0U))) {
		z_reschedule_unlocked();
	}
}

#ifdef CONFIG_USERSPACE
સ્થિર અંતઃરેખ શૂન્ય z_vrfy_k_thread_priority_set(k_tid_t thread, પૂર્ણાંક prio)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	K_OOPS(K_SYSCALL_VERIFY_સંદેશ(_is_valid_prio(prio, ખાલી),
				    "invalid thread priority %d", prio));
#ifndef CONFIG_USERSPACE_THREAD_MAY_RAISE_PRIORITY
	K_OOPS(K_SYSCALL_VERIFY_સંદેશ((int8_t)prio >= thread->base.prio,
				    "thread priority may only be downgraded (%d < %d)",
				    prio, thread->base.prio));
#endif /* CONFIG_USERSPACE_THREAD_MAY_RAISE_PRIORITY */
	z_impl_k_thread_priority_set(thread, prio);
}
#અન્તર્ભાવ <zephyr/syscalls/k_thread_priority_set_mrsh.c>
#endif /* CONFIG_USERSPACE */

શૂન્ય z_impl_k_thread_suspend(k_tid_t thread)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_thread, suspend, thread);

	/* Special case "suspend the current thread" as it doesn't
	 * need the async complexity below.
	 */
	જો (!IS_ENABLED(CONFIG_SMP) &&
	    likely((thread == _current) && !arch_is_in_isr())) {
		z_thread_suspend_current(thread);
		પાછા;
	}

	k_spinlock_ચાવી_t ચાવી = z_sched_spinlock_lock();

	જો (unlikely(z_is_thread_suspended(thread))) {

		/* The target thread is already suspended. Nothing to do. */

		z_sched_spinlock_unlock(ચાવી);
		પાછા;
	}

	z_thread_halt(thread, ચાવી, false);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_thread, suspend, thread);
}

#ifdef CONFIG_USERSPACE
સ્થિર અંતઃરેખ શૂન્ય z_vrfy_k_thread_suspend(k_tid_t thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	z_impl_k_thread_suspend(thread);
}
#અન્તર્ભાવ <zephyr/syscalls/k_thread_suspend_mrsh.c>
#endif /* CONFIG_USERSPACE */

શૂન્ય z_impl_k_thread_resume(k_tid_t thread)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_thread, resume, thread);

	k_spinlock_ચાવી_t ચાવી = z_sched_spinlock_lock();

	/* Do not try to resume a thread that was not suspended */
	જો (unlikely(!z_is_thread_suspended(thread))) {
		z_sched_spinlock_unlock(ચાવી);
		પાછા;
	}

	z_mark_thread_as_not_suspended(thread);
	z_sched_ready_locked(thread);

	z_reschedule_locked(ચાવી);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_thread, resume, thread);
}

#ifdef CONFIG_USERSPACE
સ્થિર અંતઃરેખ શૂન્ય z_vrfy_k_thread_resume(k_tid_t thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	z_impl_k_thread_resume(thread);
}
#અન્તર્ભાવ <zephyr/syscalls/k_thread_resume_mrsh.c>
#endif /* CONFIG_USERSPACE */

શૂન્ય z_impl_k_wakeup(k_tid_t thread)
{
	SYS_PORT_TRACING_OBJ_FUNC(k_thread, wakeup, thread);

	k_spinlock_ચાવી_t ચાવી = z_sched_spinlock_lock();

	જો (z_is_thread_sleeping(thread)) {
		(શૂન્ય)z_try_abort_thread_timeout(thread);
		z_mark_thread_as_not_sleeping(thread);
		z_sched_ready_locked(thread);
		z_reschedule_locked(ચાવી);
	} નહીંતર {
		z_sched_spinlock_unlock(ચાવી);
	}
}

#ifdef CONFIG_USERSPACE
સ્થિર અંતઃરેખ શૂન્ય z_vrfy_k_wakeup(k_tid_t thread)
{
	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	z_impl_k_wakeup(thread);
}
#અન્તર્ભાવ <zephyr/syscalls/k_wakeup_mrsh.c>
#endif /* CONFIG_USERSPACE */

શૂન્ય z_thread_abort(સંરચના k_thread *thread)
{
	bool essential = z_is_thread_essential(thread);
	k_spinlock_ચાવી_t ચાવી = z_sched_spinlock_lock();

	જો (z_is_thread_dead(thread)) {
		z_sched_spinlock_unlock(ચાવી);
		પાછા;
	}

	z_thread_halt(thread, ચાવી, true);

	જો (essential) {
		__ASSERT(!essential, "aborted essential thread %p", thread);
		k_panic();
	}
}

#જો !defined(CONFIG_ARCH_HAS_THREAD_ABORT)
શૂન્ય z_impl_k_thread_abort(k_tid_t thread)
{
	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_thread, abort, thread);

	z_thread_abort(thread);

	__ASSERT_NO_MSG(z_is_thread_dead(thread));

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_thread, abort, thread);
}
#endif /* !CONFIG_ARCH_HAS_THREAD_ABORT */

પૂર્ણાંક z_impl_k_thread_join(સંરચના k_thread *thread, k_timeout_t timeout)
{
	k_spinlock_ચાવી_t ચાવી = z_sched_spinlock_lock();
	પૂર્ણાંક ret;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_thread, join, thread, timeout);

	જો (z_is_thread_dead(thread)) {
		z_sched_switch_spin(thread);
		ret = 0;
	} નહીંતર જો (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		ret = -EBUSY;
	} નહીંતર જો ((thread == _current) ||
		   (thread->base.pended_on == &_current->join_queue)) {
		ret = -EDEADLK;
	} નહીંતર {
		__ASSERT(!arch_is_in_isr(), "cannot join in ISR");
		z_sched_add_to_waitq_locked(_current, &thread->join_queue);
		z_add_thread_timeout(_current, timeout);

		SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_thread, join, thread, timeout);
		ret = z_swap_locked(ચાવી);
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_thread, join, thread, timeout, ret);

		પાછા ret;
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_thread, join, thread, timeout, ret);

	z_sched_spinlock_unlock(ચાવી);
	પાછા ret;
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
સ્થિર bool thread_obj_validate(સંરચના k_thread *thread)
{
	સંરચના k_object *ko = k_object_find(thread);
	પૂર્ણાંક ret = k_object_validate(ko, K_OBJ_THREAD, _OBJ_INIT_TRUE);

	પસંદગી (ret) {
	સ્થિતિ 0:
		પાછા false;
	સ્થિતિ -EINVAL:
		પાછા true;
	મૂળભૂત:
#ifdef CONFIG_LOG
		k_object_dump_error(ret, thread, ko, K_OBJ_THREAD);
#endif /* CONFIG_LOG */
		/* ret is a non-zero error code here (the 0 and -EINVAL cases
		 * are handled above), so this branch must always oops. Passing
		 * ret as the "verify" expression would treat the failure code as
		 * success and fall through to CODE_UNREACHABLE; verify ret == 0
		 * so the oops is actually raised.
		 */
		K_OOPS(K_SYSCALL_VERIFY_સંદેશ(ret == 0, "access denied"));
	}
	CODE_UNREACHABLE; /* LCOV_EXCL_LINE */
}

સ્થિર અંતઃરેખ પૂર્ણાંક z_vrfy_k_thread_join(સંરચના k_thread *thread,
				       k_timeout_t timeout)
{
	જો (thread_obj_validate(thread)) {
		પાછા 0;
	}

	પાછા z_impl_k_thread_join(thread, timeout);
}
#અન્તર્ભાવ <zephyr/syscalls/k_thread_join_mrsh.c>

સ્થિર અંતઃરેખ શૂન્ય z_vrfy_k_thread_abort(k_tid_t thread)
{
	જો (thread_obj_validate(thread)) {
		પાછા;
	}

	K_OOPS(K_SYSCALL_VERIFY_સંદેશ(!z_is_thread_essential(thread),
				    "aborting essential thread %p", thread));

	z_impl_k_thread_abort((સંરચના k_thread *)thread);
}
#અન્તર્ભાવ <zephyr/syscalls/k_thread_abort_mrsh.c>
#endif /* CONFIG_USERSPACE */

bool k_can_yield(શૂન્ય)
{
	પાછા !(k_is_pre_kernel() || k_is_in_isr() || !arch_cpu_irqs_are_enabled() ||
		 z_is_idle_thread_object(_current));
}

શૂન્ય z_impl_k_yield(શૂન્ય)
{
	__ASSERT(!arch_is_in_isr(), "");

	SYS_PORT_TRACING_FUNC(k_thread, yield);

	z_sched_yield();
}

#ifdef CONFIG_USERSPACE
સ્થિર અંતઃરેખ શૂન્ય z_vrfy_k_yield(શૂન્ય)
{
	z_impl_k_yield();
}
#અન્તર્ભાવ <zephyr/syscalls/k_yield_mrsh.c>
#endif /* CONFIG_USERSPACE */
